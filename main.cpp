#include <QCoreApplication>
#include <QSerialPort>
#include <QDebug>

#include "qsoftdevices.h"

/** @file
 *
 * @defgroup ble_sdk_6lowpan_eval_main 6LoWPAN Adaptation Layer.
 * @{
 * @ingroup ble_sdk_6lowpan_eval
 * @brief 6LoWPAN Adaptation Layer Example.
 */

/** Includes */
#include "ble.h"
#include "sd_rpc.h"
#include "ble_ipsp.h"
#include "ble_srv_common.h"

/** Definitions */
#define DEFAULT_BAUD_RATE 1000000 /**< The baud rate to be used for serial communication with nRF5 device. */

#ifdef _WIN32
#define DEFAULT_UART_PORT_NAME "COM1"
#endif
#ifdef __APPLE__
#define DEFAULT_UART_PORT_NAME "/dev/tty.usbmodem00000"
#endif
#ifdef __linux__
#define DEFAULT_UART_PORT_NAME "/dev/ttyACM0"
#endif

/** Global variables */
static adapter_t *m_adapter = NULL;

// ==============================================================================
//TODO: app_error.h
#define APP_ERROR_CHECK(err_code)
//TODO: nff_log.h
#define NRF_LOG_INFO printf
// ==============================================================================

#define APP_IPSP_TAG                        35                                                      /**< Identifier for L2CAP configuration with the softdevice. */
#define APP_IPSP_INITIATOR_PRIO             1                                                       /**< Priority with the SDH on receiving events from the softdevice. */
#define SCANNING_LED                        BSP_LED_0_MASK                                          /**< Is on when device is scanning. */
#define CONNECTED_LED                       BSP_LED_1_MASK                                          /**< Is on when device is connected. */
#define BUTTON_DETECTION_DELAY              APP_TIMER_TICKS(50)
#define SCAN_INTERVAL                       0x00A0                                                  /**< Determines scan interval in units of 0.625 millisecond. */
#define SCAN_WINDOW                         0x0050                                                  /**< Determines scan window in units of 0.625 millisecond. */
#define MIN_CONNECTION_INTERVAL             MSEC_TO_UNITS(30, UNIT_1_25_MS)                         /**< Determines maximum connection interval in millisecond. */
#define MAX_CONNECTION_INTERVAL             MSEC_TO_UNITS(60, UNIT_1_25_MS)                         /**< Determines maximum connection interval in millisecond. */
#define SLAVE_LATENCY                       0                                                       /**< Determines slave latency in counts of connection events. */
#define SUPERVISION_TIMEOUT                 MSEC_TO_UNITS(4000, UNIT_10_MS)                         /**< Determines supervision time-out in units of 10 millisecond. */
#define DEAD_BEEF                           0xDEADBEEF                                              /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define APP_ENABLE_LOGS                 1                                                           /**< Enable logs in the application. */

#if (APP_ENABLE_LOGS == 1)

#define APPL_LOG  NRF_LOG_INFO
#define APPL_DUMP NRF_LOG_RAW_HEXDUMP_INFO
#define APPL_ADDR IPV6_ADDRESS_LOG

#else // APP_ENABLE_LOGS

#define APPL_LOG(...)
#define APPL_DUMP(...)
#define APPL_ADDR(...)

#endif // APP_ENABLE_LOGS

static ble_ipsp_handle_t    m_handle;
static uint16_t             m_conn_handle = BLE_CONN_HANDLE_INVALID;
static const ble_gap_addr_t m_peer_addr =
{
    0,                                   // addr_id_peer
    BLE_GAP_ADDR_TYPE_PUBLIC,            // addr_type
    {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x00} // addr
};


/**
 * @brief Scan parameters requested for scanning and connection.
 */
static const ble_gap_scan_params_t m_scan_param =
{
    0,                          // extended
    0,                          // report_incomplete_evts
    0,                          // Passive scanning.
    BLE_GAP_SCAN_FP_ACCEPT_ALL, // Do not use whitelist.
    BLE_GAP_PHY_AUTO,           // Automatic PHY selection.
    (uint16_t)SCAN_INTERVAL,    // Scan interval.
    (uint16_t)SCAN_WINDOW,      // Scan window.
    0,                          // Never stop scanning unless explicit asked to.
    {0}                         // channel_mask
};


/**
 * @brief Connection parameters requested for connection.
 */
static const ble_gap_conn_params_t m_connection_param =
{
    .min_conn_interval = (uint16_t)MIN_CONNECTION_INTERVAL,   // Minimum connection
    .max_conn_interval = (uint16_t)MAX_CONNECTION_INTERVAL,   // Maximum connection
    .slave_latency     = 0,                                   // Slave latency
    .conn_sup_timeout  = (uint16_t)SUPERVISION_TIMEOUT        // Supervision time-out
};

/**@brief Function for the LEDs initialization.
 *
 * @details Initializes all LEDs used by this application.
 */
static void leds_init(void)
{
    // Configure LEDs.
    //TODO: LEDS_CONFIGURE((SCANNING_LED | CONNECTED_LED));

    // Set LEDs off.
    //TODO: LEDS_OFF((SCANNING_LED | CONNECTED_LED));
}

/**@brief Function for handling button events.
 *
 * @param[in]   pin_no         The pin number of the button pressed.
 * @param[in]   button_action  The action performed on button.
 */
static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    uint32_t err_code;
#if 0
    //TODO: button
    if (button_action == APP_BUTTON_PUSH)
    {
        switch (pin_no)
        {
            case BSP_BUTTON_0:
            {
                if (m_conn_handle == BLE_CONN_HANDLE_INVALID)
                {
                    err_code = sd_ble_gap_connect(&m_peer_addr,
                                                  &m_scan_param,
                                                  &m_connection_param,
                                                  APP_IPSP_TAG);

                    if (err_code != NRF_SUCCESS)
                    {
                        APPL_LOG("Connection Request Failed, reason 0x%08lX", err_code);
                    }
                    APP_ERROR_CHECK(err_code);
                }
                else
                {
                     APPL_LOG("Connection exists with peer");
                }

                break;
            }
            case BSP_BUTTON_1:
            {
                if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
                {
                    ble_ipsp_handle_t ipsp_handle;
                    ipsp_handle.conn_handle = m_conn_handle;
                    err_code = ble_ipsp_connect(&ipsp_handle);
                    APP_ERROR_CHECK_BOOL((err_code == NRF_SUCCESS) ||
                                         (err_code == NRF_ERROR_BLE_IPSP_CHANNEL_ALREADY_EXISTS));
                }
                else
                {
                     APPL_LOG("No physical link exists with peer");
                }
                break;
            }
            case BSP_BUTTON_2:
            {
                err_code = ble_ipsp_disconnect(&m_handle);
                APPL_LOG("ble_ipsp_disconnect result %08lX", err_code);
                break;
            }
            case BSP_BUTTON_3:
            {
                if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
                {
                    err_code = sd_ble_gap_disconnect(m_conn_handle, 0x13);
                    APPL_LOG("sd_ble_gap_disconnect result %08lX", err_code);
                }
                else
                {
                    APPL_LOG("No physical link exists with peer");
                }
                break;
            }
            default:
                break;
        }
    }
#endif
}


/**@brief Function for the Button initialization.
 *
 * @details Initializes all Buttons used by this application.
 */
static void buttons_init(void)
{
    uint32_t err_code;
#if 0
    //TODO: button
    static app_button_cfg_t buttons[] =
    {
        {BSP_BUTTON_0, false, BUTTON_PULL, button_event_handler},
        {BSP_BUTTON_1, false, BUTTON_PULL, button_event_handler},
        {BSP_BUTTON_2, false, BUTTON_PULL, button_event_handler},
        {BSP_BUTTON_3, false, BUTTON_PULL, button_event_handler}
    };

    err_code = app_button_init(buttons, ARRAY_SIZE(buttons), BUTTON_DETECTION_DELAY);
    APP_ERROR_CHECK(err_code);

    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);
#endif
}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module.
 */
static void timers_init(void)
{
    // Initialize timer module.
#if 0
    //TODO: timer
    APP_ERROR_CHECK(app_timer_init());
#endif

}


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t const * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            APPL_LOG("Connected, handle 0x%04X.", p_ble_evt->evt.gap_evt.conn_handle);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

            //TODO: LEDS_ON(CONNECTED_LED);
            //TODO: LEDS_OFF(SCANNING_LED);

            break;
        case BLE_GAP_EVT_DISCONNECTED:
            APPL_LOG("Disconnected.");

            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            //TODO: LEDS_OFF(CONNECTED_LED);
            break;
        default:
            break;
    }
}

/**@brief IPSP event handler.
 * @param[in] p_handle Identifies the connection and channel on which the event occurred.
 * @param[in] p_evt    Event and related parameters (if any).
 *
 * @returns    NRF_SUCCESS.
 */
static uint32_t app_ipsp_event_handler(ble_ipsp_handle_t const * p_handle,
                                       ble_ipsp_evt_t const    * p_evt)
{
    switch (p_evt->evt_id)
    {
        case BLE_IPSP_EVT_CHANNEL_CONNECTED:
        {
            APPL_LOG("[0x%04X]:[0x%04X] BLE_IPSP_EVT_CHANNEL_CONNECTED Event. Result "
                     "0x%08lX", p_handle->conn_handle, p_handle->cid, p_evt->evt_result);
            if (p_evt->evt_result == NRF_SUCCESS)
            {
                m_handle = (*p_handle);
            }
            break;
        }
        case BLE_IPSP_EVT_CHANNEL_DISCONNECTED:
        {
            APPL_LOG("[0x%04X]:[0x%04X] BLE_IPSP_EVT_CHANNEL_DISCONNECTED Event. Result "
                     "0x%08lX", p_handle->conn_handle, p_handle->cid, p_evt->evt_result);
            break;
        }
        case BLE_IPSP_EVT_CHANNEL_DATA_RX:
        {
            const uint16_t received_sdu_length = MIN(p_evt->p_evt_param->p_l2cap_evt->params.rx.sdu_buf.len,
                                                     p_evt->p_evt_param->p_l2cap_evt->params.rx.sdu_len);

            APPL_LOG("[0x%04X]:[0x%04X] BLE_IPSP_EVT_CHANNEL_DATA_RX Event. Result 0x%08lX,"
                     " Data Length 0x%04X", p_handle->conn_handle, p_handle->cid,
                     p_evt->evt_result,  p_evt->p_evt_param->p_l2cap_evt->params.rx.sdu_buf.len);

            //uint8_t * p_send_buffer = nrf_malloc(received_sdu_length);
            uint8_t * p_send_buffer = (uint8_t*) malloc(received_sdu_length);

            if (p_send_buffer != NULL)
            {
                // Make a copy of the buffer as this buffer is recycled once the callback returns.
                // ble_ipsp_send requires the memory to be resident until the event
                // BLE_IPSP_EVT_CHANNEL_DATA_TX_COMPLETE is notified with the buffer.
                memcpy (p_send_buffer,
                        p_evt->p_evt_param->p_l2cap_evt->params.rx.sdu_buf.p_data,
                        received_sdu_length);

                //Echo back the data received.
                uint32_t err_code = ble_ipsp_send(p_handle,
                                                  p_send_buffer,
                                                  received_sdu_length);

                APPL_LOG("[0x%04X]:[0x%04X]:[%p] Echo data back. Result %08lX",
                         p_handle->conn_handle, p_handle->cid, p_send_buffer, err_code);
            }
            else
            {
                 APPL_LOG("[0x%04X]:[0x%04X] No memory to echo data back.",
                          p_handle->conn_handle, p_handle->cid);
            }

            break;
        }
        case BLE_IPSP_EVT_CHANNEL_DATA_TX_COMPLETE:
        {
            APPL_LOG ("[0x%04X]:[0x%04X]:[%p] BLE_IPSP_EVT_CHANNEL_DATA_TX_COMPLETE Event."
                      "Result 0x%08lX\r\n", p_handle->conn_handle, p_handle->cid,
                      p_evt->p_evt_param->p_l2cap_evt->params.tx.sdu_buf.p_data, p_evt->evt_result);
            //nrf_free(p_evt->p_evt_param->p_l2cap_evt->params.tx.sdu_buf.p_data);
            free(p_evt->p_evt_param->p_l2cap_evt->params.tx.sdu_buf.p_data);
            break;
        }
        default:
        {
            APPL_LOG("[0x%04X]:[0x%04X] Unknown Event 0x%04X. Result 0x%08lX",
                     p_evt->evt_id, p_handle->conn_handle, p_handle->cid, p_evt->evt_result);
            break;
        }
    }
    return NRF_SUCCESS;
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static uint32_t ble_stack_init(void)
{
    uint32_t     ram_start = 0; // Value is not used by ble-driver
    uint32_t     err_code;
    ble_cfg_t    ble_cfg;

    // Configure the maximum number of connections.
    memset(&ble_cfg, 0, sizeof(ble_cfg));
    ble_cfg.gap_cfg.role_count_cfg.periph_role_count  = 0;
    ble_cfg.gap_cfg.role_count_cfg.central_role_count = BLE_IPSP_MAX_CHANNELS;
    ble_cfg.gap_cfg.role_count_cfg.central_sec_count  = 0;
    err_code = sd_ble_cfg_set(m_adapter, BLE_GAP_CFG_ROLE_COUNT, &ble_cfg, ram_start);

    if (err_code == NRF_SUCCESS)
    {
        memset(&ble_cfg, 0, sizeof(ble_cfg));

        // Configure total number of connections.
        ble_cfg.conn_cfg.conn_cfg_tag                     = APP_IPSP_TAG;
        ble_cfg.conn_cfg.params.gap_conn_cfg.conn_count   = BLE_IPSP_MAX_CHANNELS;
        ble_cfg.conn_cfg.params.gap_conn_cfg.event_length = BLE_GAP_EVENT_LENGTH_DEFAULT;
        err_code = sd_ble_cfg_set(m_adapter, BLE_CONN_CFG_GAP, &ble_cfg, ram_start);

    }

    if (err_code ==  NRF_SUCCESS)
    {
        memset(&ble_cfg, 0, sizeof(ble_cfg));

         // Configure the number of custom UUIDS.
        ble_cfg.common_cfg.vs_uuid_cfg.vs_uuid_count = 0;
        err_code = sd_ble_cfg_set(m_adapter, BLE_COMMON_CFG_VS_UUID, &ble_cfg, ram_start);
    }

    if (err_code == NRF_SUCCESS)
    {
        memset(&ble_cfg, 0, sizeof(ble_cfg));

        // Set L2CAP channel configuration

        // @note The TX MPS and RX MPS of initiator and acceptor are not symmetrically set.
        // This will result in effective MPS of 50 in reach direction when using the initiator and
        // acceptor example against each other. In the IPSP, the TX MPS is set to a higher value
        // as Linux which is the border router for 6LoWPAN examples uses default RX MPS of 230
        // bytes. Setting TX MPS of 212 in place of 50 results in better credit and hence bandwidth
        // utilization.
        ble_cfg.conn_cfg.conn_cfg_tag                        = APP_IPSP_TAG;
        ble_cfg.conn_cfg.params.l2cap_conn_cfg.rx_mps        = BLE_IPSP_RX_MPS;
        ble_cfg.conn_cfg.params.l2cap_conn_cfg.rx_queue_size = BLE_IPSP_RX_BUFFER_COUNT;
        ble_cfg.conn_cfg.params.l2cap_conn_cfg.tx_mps        = BLE_IPSP_TX_MPS;
        ble_cfg.conn_cfg.params.l2cap_conn_cfg.tx_queue_size = 1;
        ble_cfg.conn_cfg.params.l2cap_conn_cfg.ch_count      = 1; // One L2CAP channel per link.
        err_code = sd_ble_cfg_set(m_adapter, BLE_CONN_CFG_L2CAP, &ble_cfg, ram_start);
    }

    if (err_code == NRF_SUCCESS)
    {
        memset(&ble_cfg, 0, sizeof(ble_cfg));

        // Set the ATT table size.
        ble_cfg.gatts_cfg.attr_tab_size.attr_tab_size = 256;
        err_code = sd_ble_cfg_set(m_adapter, BLE_GATTS_CFG_ATTR_TAB_SIZE, &ble_cfg, ram_start);
    }


    if (err_code ==  NRF_SUCCESS)
    {
        err_code = sd_ble_enable(m_adapter, &ram_start);

        switch (err_code) {
            case NRF_SUCCESS:
                break;
            case NRF_ERROR_INVALID_STATE:
                printf("BLE stack already enabled\n");
                fflush(stdout);
                break;
            default:
                printf("Failed to enable BLE stack. Error code: %d\n", err_code);
                fflush(stdout);
                break;
        }
    }

    return err_code;
}


static void services_init()
{
    ble_ipsp_init_t init_param;
    init_param.evt_handler = app_ipsp_event_handler;

    uint32_t err_code = ble_ipsp_init(&init_param);
    APP_ERROR_CHECK(err_code);

    ble_gap_addr_t m_my_addr;

    m_my_addr.addr[5]   = 0x00;
    m_my_addr.addr[4]   = 0x11;
    m_my_addr.addr[3]   = 0x22;
    m_my_addr.addr[2]   = 0x33;
    m_my_addr.addr[1]   = 0x44;
    m_my_addr.addr[0]   = 0x55;
    m_my_addr.addr_type = BLE_GAP_ADDR_TYPE_PUBLIC;

    err_code = sd_ble_gap_addr_set(m_adapter, &m_my_addr);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
#if 0
//TODO: mgmt
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
#endif
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details Handle any pending log operation(s), then sleep until the next event occurs.
 */
static void idle_state_handle(void)
{
#if 0
//TODO: mgmt
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
#endif
}


/**@brief Function for handling error message events from sd_rpc.
 *
 * @param[in] adapter The transport adapter.
 * @param[in] code Error code that the error message is associated with.
 * @param[in] message The error message that the callback is associated with.
 */
static void status_handler(adapter_t * adapter, sd_rpc_app_status_t code, const char * message)
{
    printf("Status: %d, message: %s\n", (uint32_t)code, message);
    fflush(stdout);
}


/**@brief Function for handling the log message events from sd_rpc.
 *
 * @param[in] adapter The transport adapter.
 * @param[in] severity Level of severity that the log message is associated with.
 * @param[in] message The log message that the callback is associated with.
 */
static void log_handler(adapter_t * adapter, sd_rpc_log_severity_t severity, const char * message)
{
    switch (severity)
    {
        case SD_RPC_LOG_ERROR:
            printf("Error: %s\n", message);
            fflush(stdout);
            break;

        case SD_RPC_LOG_WARNING:
            printf("Warning: %s\n", message);
            fflush(stdout);
            break;

        case SD_RPC_LOG_INFO:
            printf("Info: %s\n", message);
            fflush(stdout);
            break;

        default:
            printf("Log: %s\n", message);
            fflush(stdout);
            break;
    }
}


/**@brief Function for initializing serial communication with the target nRF5 Bluetooth slave.
 *
 * @param[in] serial_port The serial port the target nRF5 device is connected to.
 *
 * @return The new transport adapter.
 */
static adapter_t * adapter_init(char * serial_port, uint32_t baud_rate)
{
    physical_layer_t  * phy;
    data_link_layer_t * data_link_layer;
    transport_layer_t * transport_layer;

    phy = sd_rpc_physical_layer_create_uart(serial_port,
                                            baud_rate,
                                            SD_RPC_FLOW_CONTROL_NONE,
                                            SD_RPC_PARITY_NONE);
    data_link_layer = sd_rpc_data_link_layer_create_bt_three_wire(phy, 250);
    transport_layer = sd_rpc_transport_layer_create(data_link_layer, 1500);
    return sd_rpc_adapter_create(transport_layer);
}


/** Event dispatcher */

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in] adapter The transport adapter.
 * @param[in] p_ble_evt Bluetooth stack event.
 */
static void ble_evt_dispatch(adapter_t * adapter, ble_evt_t * p_ble_evt)
{
    if (p_ble_evt == NULL)
    {
        printf("Received an empty BLE event\n");
        fflush(stdout);
        return;
    }

    ble_ipsp_evt_handler(p_ble_evt);
    on_ble_evt(p_ble_evt);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
//============================================================================================================================================
    uint32_t error_code;
    char *   serial_port = DEFAULT_UART_PORT_NAME;
    uint32_t baud_rate = DEFAULT_BAUD_RATE;

    if (argc > 1)
    {
        serial_port = argv[1];
    }

    printf("Serial port used: %s\n", serial_port);
    printf("Baud rate used: %d\n", baud_rate);
    fflush(stdout);

    m_adapter =  adapter_init(serial_port, baud_rate);
    sd_rpc_log_handler_severity_filter_set(m_adapter, SD_RPC_LOG_INFO);
    error_code = sd_rpc_open(m_adapter, status_handler, ble_evt_dispatch, log_handler);

    if (error_code != NRF_SUCCESS)
    {
        printf("Failed to open nRF BLE Driver. Error code: 0x%02X\n", error_code);
        fflush(stdout);
        return error_code;
    }

    //TODO: timers_init();
    //TODO: buttons_init();
    //TODO: leds_init();
    //TODO: power_management_init();

    error_code = ble_stack_init();

    if (error_code != NRF_SUCCESS)
    {
        return error_code;
    }

    services_init();

    //TODO:
    // Enter main loop.
    //for (;;)
    //{
    //    idle_state_handle();
    //}

//============================================================================================================================================
#if 0
    QSerialPort serialPort;
    serialPort.setPortName("/dev/ttyACM0");
    //serialPort.setPortName("COM10");
    serialPort.setBaudRate(QSerialPort::Baud115200);
    serialPort.setDataBits(QSerialPort::Data8);
    serialPort.setParity(QSerialPort::NoParity);
    serialPort.setStopBits(QSerialPort::OneStop);
    serialPort.setFlowControl(QSerialPort::NoFlowControl);

    if (!serialPort.open(QIODevice::ReadWrite)) {
        qDebug() << "Failed to open";
        return 1;
    }

    QSoftDevices softDevice(&serialPort);
    QObject::connect(
        &softDevice, &QSoftDevices::readyRead,
        [&softDevice]() {
            QByteArray event = softDevice.read(softDevice.bytesAvailable());
            qDebug() << "QSoftDevices::receivedEvent" << event.toHex() << event.size();
        }
    );

    QVarLengthArray<char> varLenArray{0x4c, 0x00};
    softDevice.writeCommand(varLenArray);
    softDevice.writeCommand({0x4c, 0x00});
    // NOTE: can't get events after invalid commands
    //softDevice.writeCommand({0x4c}); // invalid command
    //softDevice.writeCommand({0x4c, 0x00, 0x00}); // invalid command
    //softDevice.writeCommand({0x4c, 0x00, 0x00, 0x00}); // invalid command

    QByteArray byteArray(std::begin<char>({0x4c, 0x00}), 2);
    softDevice.writeCommand({byteArray.begin(), byteArray.end()});
    softDevice.writeCommand(QVarLengthArray<char>(byteArray.begin(), byteArray.end()));

    //echo -e '\x06\x00\x00\x02\x03\x04\x05\x06' > /dev/ttyACM0
    //cmd: 0300 00 4c 00
    //evt: 0700 01 4c 00 00000000
    QByteArray command(std::begin<char>({0x4c, 0x00}), 2);
    softDevice.write(command);
#endif
    qDebug() << "QtSoftDevice Run";
    return a.exec();
}
