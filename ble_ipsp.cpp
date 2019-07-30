#include <string.h>
#include <stdio.h>
#include "ble_ipsp.h"
#include "ble_srv_common.h"

ble_ipsp::ble_ipsp()
{

}

// ================================================================================================
#define IPSP_MAX_CONNECTED_DEVICES    BLE_IPSP_MAX_CHANNELS                                         /**< Table for maximum number of connected devices the module will keep track of. */
#define RX_BUFFER_TOTAL_SIZE          (BLE_IPSP_RX_BUFFER_SIZE * BLE_IPSP_RX_BUFFER_COUNT)          /**< Total receive buffer size reserved for each IPSP channel. */
#define MAX_L2CAP_RX_BUFFER           (RX_BUFFER_TOTAL_SIZE * BLE_IPSP_MAX_CHANNELS)                /**< Total receive buffer received for all channels. */

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

void ble_ipsp_evt_handler(ble_evt_t const * p_evt)
{
    //TODO: unimplemented
    printf("(unimplemented) ble_ipsp_evt_handler\n");
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
