#include <string.h>
#include <stdio.h>
#include <QDebug>
#include "ble_ipsp.h"
#include "ble_srv_common.h"

ble_ipsp::ble_ipsp()
{

}

// ================================================================================================
#define BLE_IPSP_TRC                  qDebug                                                        /**< Used for getting trace of execution in the module. */
#define BLE_IPSP_ERR                  qDebug                                                        /**< Used for logging errors in the module. */
#define IPSP_ANY_CID                  0xFFFE                                                        /**< Identifier for any channel. Usage: Search for existing channel on a connection handle. */

#define IPSP_MAX_CONNECTED_DEVICES    BLE_IPSP_MAX_CHANNELS                                         /**< Table for maximum number of connected devices the module will keep track of. */
#define RX_BUFFER_TOTAL_SIZE          (BLE_IPSP_RX_BUFFER_SIZE * BLE_IPSP_RX_BUFFER_COUNT)          /**< Total receive buffer size reserved for each IPSP channel. */
#define MAX_L2CAP_RX_BUFFER           (RX_BUFFER_TOTAL_SIZE * BLE_IPSP_MAX_CHANNELS)                /**< Total receive buffer received for all channels. */
#define INVALID_CHANNEL_INSTANCE      0xFF                                                          /**< Indicates channel instance is invalid. */

/**@brief IPSP Channel States. */
typedef enum
{
    CHANNEL_IDLE,                                                                                   /**< Indicates the channel is free and not in use. */
    CHANNEL_CONNECTING,                                                                             /**< Indicates the channel creation is requested and is awaiting a response. */
    CHANNEL_CONNECTED,                                                                              /**< Indicates the channel is connected and ready for data exchange. */
    CHANNEL_DISCONNECTING                                                                           /**< Indicates the channel is in the process of being disconnected. */
} channel_state_t;

/**@brief Possible response actions for an incoming channel. Default is to accept. */
typedef enum
{
    INCOMING_CHANNEL_ACCEPT,                                                                       /**< Indicates that the incoming channel should be accepted if all other criteria are met. */
    INCOMING_CHANNEL_REJECT                                                                        /**< Indicates that the incoming channel for IPSP PSM should be rejected regardless of the other criteria. */
} incoming_channel_action_t;

/**@brief Data type for book keeping connected devices.
 *
 * @note Not all connected devices establish an L2CAP connection.
 */
typedef struct
{
    volatile incoming_channel_action_t  response;                                                   /**< Indicator if the incoming channel should be accepted or rejected. */
    ble_gap_addr_t                      ble_addr;                                                   /**< Bluetooth device address of the peer. */
    uint16_t                            conn_handle;                                                /**< Connection handle identifying the link with the peer. */
} peer_connection_t;

/**@brief IPSP Channel Information. */
typedef struct
{
    uint16_t     conn_handle;                                                                       /**< Identifies the BLE link on which channel is established. BLE_CONN_HANDLE_INVALID if channel is unassigned. */
    uint16_t     cid;                                                                               /**< L2CAP channel identifier needed to manage the channel once established. BLE_L2CAP_CID_INVALID if channel is unassigned. */
    uint16_t     rx_buffer_status;                                                                  /**< Usage status of RX buffers. */
    uint8_t      state;                                                                             /**< State information for the channel. See @ref channel_state_t for details. */
    uint8_t    * p_rx_buffer;                                                                       /**< Receive buffer for the channel. */
} channel_t;

static ble_ipsp_evt_handler_t m_evt_handler = NULL;                                                 /**< Asynchronous event notification callback registered with the module. */
static channel_t              m_channel[BLE_IPSP_MAX_CHANNELS];                                     /**< Table of channels managed by the module. */
static uint8_t                m_rx_buffer[MAX_L2CAP_RX_BUFFER];                                     /**< Receive buffer reserved for all channels to receive data on the L2CAP IPSP channel. */
static peer_connection_t      m_connected_device[IPSP_MAX_CONNECTED_DEVICES];                       /**< Table maintaining list of peer devices and the connection handle.

/**@brief Initialize the peer connected device in the list.
 *
 * @param[in] index Identifies the list element to be initialized.
 */
static __INLINE void connected_device_init(uint32_t index)
{
    memset (&m_connected_device[index].ble_addr, 0, sizeof(ble_gap_addr_t));
    m_connected_device[index].conn_handle = BLE_CONN_HANDLE_INVALID;
    m_connected_device[index].response    = INCOMING_CHANNEL_ACCEPT;
}

/**@brief Allocate an entry for the peer connected device in the list.
 *
 * @param[in] p_peer_addr Pointer to peer's device address.
 * @param[in] conn_handle Connection handle identifying the link with the peer.
 */
static __INLINE void connected_device_allocate(ble_gap_addr_t const * p_peer_addr,
                                               uint16_t               conn_handle)
{
    for (uint32_t index = 0; index < IPSP_MAX_CONNECTED_DEVICES; index++)
    {
        if (m_connected_device[index].conn_handle == BLE_CONN_HANDLE_INVALID)
        {
            m_connected_device[index].conn_handle = conn_handle;
            memcpy(m_connected_device[index].ble_addr.addr, p_peer_addr->addr, BLE_GAP_ADDR_LEN);
            break;
        }
    }
}

/**@brief Initialize channel.
 *
 * @param[in] ch_id Identifies the IPSP channel on which the procedure is requested.
 */
static __INLINE void channel_init(uint8_t ch_id)
{
    m_channel[ch_id].conn_handle      = BLE_CONN_HANDLE_INVALID;
    m_channel[ch_id].cid              = BLE_L2CAP_CID_INVALID;
    m_channel[ch_id].rx_buffer_status = 0;
    m_channel[ch_id].state            = CHANNEL_IDLE;
    m_channel[ch_id].p_rx_buffer      = &m_rx_buffer[ch_id*RX_BUFFER_TOTAL_SIZE];
}

/**@brief Free channel.
 *
 * @param[in] ch_id Identifies the IPSP channel on which the procedure is requested.
 */
static __INLINE void channel_free(uint8_t ch_id)
{
    BLE_IPSP_TRC("[Index 0x%02X]:[Conn Handle 0x%04X]:[CID 0x%04X]: Freeing channel",
             ch_id, m_channel[ch_id].conn_handle, m_channel[ch_id].cid);

    channel_init(ch_id);
}


/**@brief Searches the IPSP channel based on connection handle and local L2CAP channel identifier.
 *
 * @param[in]  conn_handle The connection handle, identifying the peer device.
 * @param[in]  l2cap_cid   The local L2CAP channel identifier, identifying the L2CAP channel.
 * @param[out] p_ch_id     The IPSP channel identifier, if the search succeeded, else,
 *                         BLE_IPSP_MAX_CHANNELS indicating no IPSP channel was found.
 */
static __INLINE uint32_t channel_search(uint16_t conn_handle, uint16_t l2cap_cid, uint8_t * p_ch_id)
{
    BLE_IPSP_TRC("[Conn Handle 0x%04X]:[CID 0x%04X]: channel_search",
             conn_handle, l2cap_cid);

    for (int i = 0; i < BLE_IPSP_MAX_CHANNELS; i++)
    {
        BLE_IPSP_TRC("[@ Index 0x%02X] ==> Conn Handle: 0x%04X"
                 "                            CID        : 0x%04X",
                 i, m_channel[i].conn_handle, m_channel[i].cid);

        if (m_channel[i].conn_handle == conn_handle)
        {
            if ((l2cap_cid == IPSP_ANY_CID) || (m_channel[i].cid == l2cap_cid))
            {
                BLE_IPSP_TRC("channel_search succeeded, index 0x%04X", i);

                *p_ch_id = (uint8_t)i;
                return NRF_SUCCESS;
            }
        }
    }

    BLE_IPSP_TRC("No matching channel found!");
    return (NRF_ERROR_BLE_IPSP_ERR_BASE + NRF_ERROR_NOT_FOUND);
}

void ble_ipsp_evt_handler(ble_evt_t const * p_evt)
{
    //TODO: unimplemented
    if (m_evt_handler == NULL) return;

    ble_ipsp_handle_t    handle;
    ble_ipsp_evt_t       ipsp_event;
    uint32_t             retval;
    uint8_t              ch_id;
    bool                 notify_event;
    bool                 submit_rx_buffer;

    ch_id                 = INVALID_CHANNEL_INSTANCE;
    notify_event          = false;
    submit_rx_buffer      = false;
    retval                = NRF_SUCCESS;
    ipsp_event.evt_result = NRF_SUCCESS;
    handle.conn_handle    = BLE_CONN_HANDLE_INVALID;
    handle.cid            = BLE_L2CAP_CID_INVALID;

    BLE_IPSP_TRC(">> Received BLE Event 0x%04X",p_evt->header.evt_id);

    switch (p_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
        {
            printf("Received BLE Event BLE_GAP_EVT_CONNECTED\n");
            // Create an entry in the connected devices table.
            // This is needed to be able to fetch the peer address on IPSP channel establishment.
            connected_device_allocate(&p_evt->evt.gap_evt.params.connected.peer_addr,
                                      p_evt->evt.gap_evt.conn_handle);
            break;
        }
        case BLE_L2CAP_EVT_CH_SETUP_REQUEST:
        {
            printf("Received BLE Event BLE_L2CAP_EVT_CH_SETUP_REQUEST\n");
            break;
        }
        case BLE_L2CAP_EVT_CH_SETUP:
        {
            printf("Received BLE Event BLE_L2CAP_EVT_CH_SETUP\n");
            break;
        }
        case BLE_L2CAP_EVT_CH_SETUP_REFUSED:
        {
            printf("Received BLE Event BLE_L2CAP_EVT_CH_SETUP_REFUSED\n");
            break;
        }
        case BLE_L2CAP_EVT_CH_RELEASED:
        {
            printf("Received BLE Event BLE_L2CAP_EVT_CH_RELEASED\n");
            break;
        }
        case BLE_L2CAP_EVT_CH_RX:
        {
            printf("Received BLE Event BLE_L2CAP_EVT_CH_RX\n");
            break;
        }
        case BLE_L2CAP_EVT_CH_TX:
        {
            printf("Received BLE Event BLE_L2CAP_EVT_CH_TX\n");
            break;
        }
        case BLE_L2CAP_EVT_CH_SDU_BUF_RELEASED:
        {
            printf("Received BLE Event BLE_L2CAP_EVT_CH_SDU_BUF_RELEASED\n");
            break;
        }
        case BLE_GAP_EVT_DISCONNECTED:
        {
            printf("Received BLE Event BLE_GAP_EVT_DISCONNECTED\n");
            break;
        }
        default:
            printf("Received BLE Event default\n");
            break;
    }
}

uint32_t ble_ipsp_init(adapter_t *m_adapter, const ble_ipsp_init_t * p_init)
{
    ble_uuid_t ble_uuid;
    uint32_t err_code;
    uint16_t handle;

    if (p_init == nullptr) return NRF_ERROR_NULL;
    if (p_init->evt_handler == nullptr) return NRF_ERROR_NULL;

    // Add service to indicate IPSP support.
    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_IPSP_SERVICE);

    err_code = sd_ble_gatts_service_add(m_adapter, BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    m_evt_handler = p_init->evt_handler;

    // Initialize the channel.
    for (int i = 0; i < BLE_IPSP_MAX_CHANNELS; i++)
    {
        channel_init(i);
    }

    // Initialize the connected peer device table.
    for (int i = 0; i < IPSP_MAX_CONNECTED_DEVICES; i++)
    {
        connected_device_init(i);
    }

    return NRF_SUCCESS;
}

uint32_t ble_ipsp_connect(adapter_t *m_adapter, const ble_ipsp_handle_t * p_handle)
{
    if (m_evt_handler == nullptr) return NRF_ERROR_NULL;
    if (p_handle == nullptr) return NRF_ERROR_NULL;
    if (p_handle->conn_handle == BLE_CONN_HANDLE_INVALID) return NRF_ERROR_NULL;

    uint32_t err_code;
    uint8_t  ch_id  = INVALID_CHANNEL_INSTANCE;

    BLE_IPSP_TRC("[Conn Handle 0x%04X]: >> ble_ipsp_connect", p_handle->conn_handle);

    // Check if channel already exists with the peer.
    err_code = channel_search(p_handle->conn_handle, IPSP_ANY_CID, &ch_id);

    if (err_code == NRF_SUCCESS)
    {
        // IPSP channel already exists.
        err_code = NRF_ERROR_BLE_IPSP_CHANNEL_ALREADY_EXISTS;
    }
    else
    {
        // Search for a free channel.
        err_code = channel_search(BLE_CONN_HANDLE_INVALID, BLE_L2CAP_CID_INVALID, &ch_id);
        BLE_IPSP_TRC("2 channel_search result %08X", err_code);

        if (err_code == NRF_SUCCESS)
        {
            m_channel[ch_id].state = CHANNEL_CONNECTING;

            ble_l2cap_ch_setup_params_t param =
            {
                {             // rx_params
                    BLE_IPSP_MTU,    //rx_mtu
                    BLE_IPSP_RX_MPS, //rx_mps
                    {                //sdu_buf
                        nullptr, // p_data
                        0        // len
                    }
                },
                BLE_IPSP_PSM, // le_psm
                0             // status
            };
            BLE_IPSP_TRC("Requesting sd_ble_l2cap_ch_setup");

            err_code = sd_ble_l2cap_ch_setup(m_adapter, p_handle->conn_handle,
                                             &m_channel[ch_id].cid, &param);
            if (err_code != NRF_SUCCESS)
            {
                BLE_IPSP_ERR("sd_ble_l2cap_ch_conn_request failed, reason %08lX", err_code);
                channel_free(ch_id);
            }
            else
            {
                BLE_IPSP_TRC("Local channel id from SD %04X.", m_channel[ch_id].cid);
                m_channel[ch_id].conn_handle = p_handle->conn_handle;
            }
        }
        else
        {
            err_code = (NRF_ERROR_BLE_IPSP_ERR_BASE + NRF_ERROR_NO_MEM);
        }
    }
    return err_code;
}

uint32_t ble_ipsp_send(ble_ipsp_handle_t const * p_handle,
                       uint8_t const           * p_data,
                       uint16_t                  data_len)
{
    uint32_t err_code = NRF_ERROR_NOT_SUPPORTED;
    //TODO: unimplemented
    printf("(unimplemented) ble_ipsp_send\n");
    return err_code;
}
// ================================================================================================
