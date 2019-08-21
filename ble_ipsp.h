#ifndef BLE_IPSP_H
#define BLE_IPSP_H


class ble_ipsp
{
public:
    ble_ipsp();
};

// ================================================================================================
//TODO: sdk_errors.h
#define NRF_ERROR_BLE_IPSP_ERR_BASE         (0x8400)    /**< Base address for BLE IPSP related errors. */

/**
 * @defgroup ble_ipsp_errors IPSP codes
 * @brief Error and status codes specific to IPSP.
 * @{
 */
#define NRF_ERROR_BLE_IPSP_RX_PKT_TRUNCATED       (NRF_ERROR_BLE_IPSP_ERR_BASE + 0x0000)
#define NRF_ERROR_BLE_IPSP_CHANNEL_ALREADY_EXISTS (NRF_ERROR_BLE_IPSP_ERR_BASE + 0x0001)
#define NRF_ERROR_BLE_IPSP_LINK_DISCONNECTED      (NRF_ERROR_BLE_IPSP_ERR_BASE + 0x0002)
#define NRF_ERROR_BLE_IPSP_PEER_REJECTED          (NRF_ERROR_BLE_IPSP_ERR_BASE + 0x0003)
/* @} */

// ================================================================================================
#include <stdint.h>
#include "ble.h"

#ifdef __cplusplus
extern "C" {
#endif

/**@brief Maximum IPSP channels required to be supported. */
#define BLE_IPSP_MAX_CHANNELS                              1

/**@brief Maximum Transmit Unit on IPSP channel. */
#define BLE_IPSP_MTU                                       1280

/**@brief Receive MPS used by IPSP. */
#define BLE_IPSP_RX_MPS                                    50

/**@brief Transmission MPS used by IPSP.
 *
 * @note The actual MPS used is minimum of this value and the one requested by
 *       the peer during the channel setup. Here, the value used is
 *       (23 + 27 * 7).
 */
#define BLE_IPSP_TX_MPS                                    212

/**@brief Maximum data size that can be received.
 *
 * @details Maximum data size that can be received on the IPSP channel.
 *          Modify this values to intentionally set a receive size less
 *          than the MTU set on the channel.
 */
#define BLE_IPSP_RX_BUFFER_SIZE                            1280

/**@brief Maximum number of receive buffers.
 *
 * @details Maximum number of receive buffers to be used per IPSP channel.
 *          Each receive buffer is of size @ref BLE_IPSP_RX_BUFFER_SIZE.
 *          This configuration has implications on the number of SDUs that can
 *          be received while an SDU is being consumed by the application
 *          (6LoWPAN/IP Stack).
 */
#define BLE_IPSP_RX_BUFFER_COUNT                           4

/**@brief L2CAP Protocol Service Multiplexers number. */
#define BLE_IPSP_PSM                                       0x0023


/**@brief IPSP event identifier type. */
typedef enum
{
    BLE_IPSP_EVT_CHANNEL_CONNECTED,                                                                 /**< Channel connection event. */
    BLE_IPSP_EVT_CHANNEL_DISCONNECTED,                                                              /**< Channel disconnection event. */
    BLE_IPSP_EVT_CHANNEL_DATA_RX,                                                                   /**< Data received on channel event. */
    BLE_IPSP_EVT_CHANNEL_DATA_TX_COMPLETE                                                           /**< Requested data transmission complete on channel event. */
} ble_ipsp_evt_type_t;


/**@brief IPSP event parameter. */
typedef struct
{
    ble_l2cap_evt_t const   * p_l2cap_evt;                                                          /**< L2CAP event parameters. */
    ble_gap_addr_t  const   * p_peer;                                                               /**< Peer device address. */
} ble_ipsp_event_param_t;


/**@brief IPSP event and associated parameter type. */
typedef struct
{
    ble_ipsp_evt_type_t         evt_id;                                                             /**< Identifier event type. */
    ble_ipsp_event_param_t    * p_evt_param;                                                        /**< Parameters associated with the event. */
    uint32_t                    evt_result;                                                         /**< Result of the event.
                                                                                                      \n The event result is SDK_ERR_RX_PKT_TRUNCATED for @ref BLE_IPSP_EVT_CHANNEL_DATA_RX,
                                                                                                      \n implies that an incomplete SDU was received due to insufficient RX buffer size.
                                                                                                      \n The size determined by @ref BLE_IPSP_RX_BUFFER_SIZE. */
} ble_ipsp_evt_t;


/**@brief IPSP handle. */
typedef struct
{
    uint16_t    conn_handle;                                                                        /**< Identifies the link on which the IPSP channel is established. */
    uint16_t    cid;                                                                                /**< Identifies the IPSP logical channel. */
} ble_ipsp_handle_t;


/**@brief Profile event handler type.
 *
 * @param[in] p_handle Identifies the connection and channel on which the event occurred.
 * @param[in] p_evt    Event and related parameters (if any).
 *
 * @returns    Provision for the application to indicate if the event was successfully processed or
 *             not. Currently not used.
 */
typedef uint32_t (*ble_ipsp_evt_handler_t) (ble_ipsp_handle_t const * p_handle,
                                            ble_ipsp_evt_t    const * p_evt);


/**@brief IPSP initialization structure.
 *
 * @details IPSP initialization structure containing all options and data needed to
 *          initialize the profile.
 */
typedef struct
{
    ble_ipsp_evt_handler_t    evt_handler;                                                          /**< Event notification callback registered with the module to receive asynchronous events. */
} ble_ipsp_init_t;


/**@brief Function for initializing the Internet Protocol Support Profile.
 *
 * @param[in] p_init   Information needed to initialize the service.
 *
 * @retval NRF_SUCCESS If initialization of the service was successful, else,
 *                     an error code indicating reason for failure.
 */
uint32_t ble_ipsp_init(adapter_t *m_adapter, ble_ipsp_init_t const * p_init);


/**@brief Function for requesting a channel creation for the Internet Protocol Support Profile.
 *
 * @details Channel creation for Internet Protocol Support Profile (IPSP) is requested using this
 *          API. Connection handle provided in p_handle parameter identifies the peer with which
 *          the IPSP channel is being requested.
 *          NRF_SUCCESS return value by the API is only indicative of request procedure having
 *          succeeded. Result of channel establishment is known when the
 *          @ref BLE_IPSP_EVT_CHANNEL_CONNECTED event is notified.
 *          Therefore, the application must wait for @ref BLE_IPSP_EVT_CHANNEL_CONNECTED event on
 *          successful return of this API.
 *
 * @param[in] p_handle Indicates the connection handle on which IPSP channel is to be created.
 *
 * @retval NRF_SUCCESS If initialization of the service was successful, else,
 *                     an error code indicating reason for failure.
 */
uint32_t ble_ipsp_connect(adapter_t *m_adapter, ble_ipsp_handle_t const * p_handle);


/**@brief Function for sending IP data to peer.
 *
 * @param[in] p_handle Instance of the logical channel and peer for which the data is intended.
 * @param[in] p_data   Pointer to memory containing the data to be transmitted.
 *                     @note This memory must be resident and should not be freed unless
 *                     @ref BLE_IPSP_EVT_CHANNEL_DATA_TX_COMPLETE event is notified.
 * @param[in] data_len Length/size of data to be transferred.
 *
 * @retval NRF_SUCCESS If initialization of the service was successful, else,
 *                     an error code indicating reason for failure.
 */
uint32_t ble_ipsp_send(adapter_t *m_adapter,
                       ble_ipsp_handle_t const * p_handle,
                       uint8_t           const * p_data,
                       uint16_t                  data_len);


/**@brief Function for disconnecting IP transport.
 *
 * @param[in] p_handle Identifies IPSP transport.
 *
 * @retval NRF_SUCCESS If initialization of the service was successful, else,
 *                     an error code indicating reason for failure.
 */
uint32_t ble_ipsp_disconnect(ble_ipsp_handle_t const * p_handle);


/**@brief Function to accept incoming connections from a peer.
 *
 * @param[in] conn_handle Identifies the link with the peer.
 */
void ble_ipsp_incoming_channel_accept(uint16_t conn_handle);


/**@brief Function to reject incoming connections from a peer.
 *
 * @param[in] conn_handle Identifies the link with the peer.
 */
void ble_ipsp_incoming_channel_reject(uint16_t conn_handle);


/**@brief BLE event handler of the module.
 *
 * @param[in] p_evt BLE event to be handled.
 *
 * @retval NRF_SUCCESS If initialization of the service was successful, else,
 *                     an error code indicating reason for failure.
 */
void ble_ipsp_evt_handler(ble_evt_t const * p_evt);

#ifdef __cplusplus
}
#endif
// ================================================================================================

#endif // BLE_IPSP_H