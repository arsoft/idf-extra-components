/*
 * SPDX-FileCopyrightText: 2019-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <protocomm.h>

#include "esp_event.h"
#ifdef CONFIG_NETWORK_PROV_NETWORK_TYPE_WIFI
#include "esp_wifi_types.h"
#endif
#ifdef CONFIG_NETWORK_PROV_NETWORK_TYPE_THREAD
#include "openthread/dataset.h"
#endif
#include "network_provisioning/network_config.h"

#ifdef __cplusplus
extern "C" {
#endif

ESP_EVENT_DECLARE_BASE(NETWORK_PROV_EVENT);

/**
 * @brief   Events generated by manager
 *
 * These events are generated in order of declaration and, for the
 * stretch of time between initialization and de-initialization of
 * the manager, each event is signaled only once
 */
typedef enum {
    /**
     * Emitted when the manager is initialized
     */
    NETWORK_PROV_INIT,

    /**
     * Indicates that provisioning has started
     */
    NETWORK_PROV_START,
#ifdef CONFIG_NETWORK_PROV_NETWORK_TYPE_WIFI
    /**
     * Emitted when Wi-Fi AP credentials are received via `protocomm`
     * endpoint `network_config`. The event data in this case is a pointer
     * to the corresponding `wifi_sta_config_t` structure
     */
    NETWORK_PROV_WIFI_CRED_RECV,
#endif // CONFIG_NETWORK_PROV_NETWORK_TYPE_WIFI
#ifdef CONFIG_NETWORK_PROV_NETWORK_TYPE_THREAD
    /**
     * Emitted when Thread Dataset is received via `protocomm` endpoint
     * `network_config`, The event data in this case is a pointer to the
     * corresponding `otOperationalDatasetTlvs` structure
     */
    NETWORK_PROV_THREAD_DATASET_RECV,
#endif // CONFIG_NETWORK_PROV_NETWORK_TYPE_THREAD
#ifdef CONFIG_NETWORK_PROV_NETWORK_TYPE_WIFI
    /**
     * Emitted when device fails to connect to the AP of which the
     * credentials were received earlier on event `NETWORK_PROV_WIFI_CRED_RECV`.
     * The event data in this case is a pointer to the disconnection
     * reason code with type `network_prov_wifi_sta_fail_reason_t`
     */
    NETWORK_PROV_WIFI_CRED_FAIL,
#endif // CONFIG_NETWORK_PROV_NETWORK_TYPE_WIFI
#ifdef CONFIG_NETWORK_PROV_NETWORK_TYPE_THREAD
    /**
     * Emitted when device fails to connect to the Thread network of which
     * dataset was received earlier on event `NETWORK_PROv_THREAD_DATASET_RECV`.
     * The event data in this case is a pointer to the disconnection
     * reason code with type `network_prov_thread_fail_reason_t`
     */
    NETWORK_PROV_THREAD_DATASET_FAIL,
#endif // CONFIG_NETWORK_PROV_NETWORK_TYPE_THREAD
#ifdef CONFIG_NETWORK_PROV_NETWORK_TYPE_WIFI
    /**
     * Emitted when device successfully connects to the AP of which the
     * credentials were received earlier on event `NETWORK_PROV_WIFI_CRED_RECV`
     */
    NETWORK_PROV_WIFI_CRED_SUCCESS,
#endif // CONFIG_NETWORK_PROV_NETWORK_TYPE_WIFI
#ifdef CONFIG_NETWORK_PROV_NETWORK_TYPE_THREAD
    /**
     * Emitted when device successfully connects to the Thread etwork of
     * which the dataset was received earlier on event
     * `NETWORK_PROV_THREAD_DATASET_RECV`
     */
    NETWORK_PROV_THREAD_DATASET_SUCCESS,
#endif // CONFIG_NETWORK_PROV_NETWORK_TYPE_THREAD

    /**
     * Signals that provisioning service has stopped
     */
    NETWORK_PROV_END,

    /**
     * Signals that manager has been de-initialized
     */
    NETWORK_PROV_DEINIT,
} network_prov_cb_event_t;

typedef void (*network_prov_cb_func_t)(void *user_data, network_prov_cb_event_t event, void *event_data);

/**
 * @brief   Event handler that is used by the manager while
 *          provisioning service is active
 */
typedef struct {
    /**
     * Callback function to be executed on provisioning events
     */
    network_prov_cb_func_t event_cb;

    /**
     * User context data to pass as parameter to callback function
     */
    void *user_data;
} network_prov_event_handler_t;

/**
 * @brief Event handler can be set to none if not used
 */
#define NETWORK_PROV_EVENT_HANDLER_NONE { \
    .event_cb  = NULL,                    \
    .user_data = NULL                     \
}

/**
 * @brief   Structure for specifying the provisioning scheme to be
 *          followed by the manager
 *
 * @note    Ready to use schemes are available:
 *              - network_prov_scheme_ble     : for provisioning over BLE transport + GATT server
 *              - network_prov_scheme_softap  : for provisioning over SoftAP transport + HTTP server
 *              - network_prov_scheme_console : for provisioning over Serial UART transport + Console (for debugging)
 */
typedef struct network_prov_scheme {
    /**
     * Function which is to be called by the manager when it is to
     * start the provisioning service associated with a protocomm instance
     * and a scheme specific configuration
     */
    esp_err_t (*prov_start) (protocomm_t *pc, void *config);

    /**
     * Function which is to be called by the manager to stop the
     * provisioning service previously associated with a protocomm instance
     */
    esp_err_t (*prov_stop) (protocomm_t *pc);

    /**
     * Function which is to be called by the manager to generate
     * a new configuration for the provisioning service, that is
     * to be passed to prov_start()
     */
    void *(*new_config) (void);

    /**
     * Function which is to be called by the manager to delete a
     * configuration generated using new_config()
     */
    void (*delete_config) (void *config);

    /**
     * Function which is to be called by the manager to set the
     * service name and key values in the configuration structure
     */
    esp_err_t (*set_config_service) (void *config, const char *service_name, const char *service_key);

    /**
     * Function which is to be called by the manager to set a protocomm endpoint
     * with an identifying name and UUID in the configuration structure
     */
    esp_err_t (*set_config_endpoint) (void *config, const char *endpoint_name, uint16_t uuid);

#ifdef CONFIG_NETWORK_PROV_NETWORK_TYPE_WIFI
    /**
     * Sets mode of operation of Wi-Fi during provisioning
     * This is set to :
     * - WIFI_MODE_APSTA for SoftAP transport
     * - WIFI_MODE_STA for BLE transport
     */
    wifi_mode_t wifi_mode;
#endif // CONFIG_NETWORK_PROV_NETWORK_TYPE_WIFI
} network_prov_scheme_t;

/**
 * @brief   Structure for specifying the manager configuration
 */
typedef struct {
    /**
     * Provisioning scheme to use. Following schemes are already available:
     *     - network_prov_scheme_ble     : for provisioning over BLE transport + GATT server
     *     - network_prov_scheme_softap  : for provisioning over SoftAP transport + HTTP server + mDNS (optional)
     *     - network_prov_scheme_console : for provisioning over Serial UART transport + Console (for debugging)
     */
    network_prov_scheme_t scheme;

    /**
     * Event handler required by the scheme for incorporating scheme specific
     * behavior while provisioning manager is running. Various options may be
     * provided by the scheme for setting this field. Use NETWORK_PROV_EVENT_HANDLER_NONE
     * when not used. When using scheme network_prov_scheme_ble, the following
     * options are available:
     *     - NETWORK_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM
     *     - NETWORK_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BLE
     *     - NETWORK_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BT
     */
    network_prov_event_handler_t scheme_event_handler;

    /**
     * Event handler that can be set for the purpose of incorporating application
     * specific behavior. Use NETWORK_PROV_EVENT_HANDLER_NONE when not used.
     */
    network_prov_event_handler_t app_event_handler;
} network_prov_mgr_config_t;

/**
 * @brief   Security modes supported by the Provisioning Manager.
 *
 * These are same as the security modes provided by protocomm
 */
typedef enum network_prov_security {
#ifdef CONFIG_ESP_PROTOCOMM_SUPPORT_SECURITY_VERSION_0
    /**
     * No security (plain-text communication)
     */
    NETWORK_PROV_SECURITY_0 = 0,
#endif
#ifdef CONFIG_ESP_PROTOCOMM_SUPPORT_SECURITY_VERSION_1
    /**
     * This secure communication mode consists of
     *   X25519 key exchange
     * + proof of possession (pop) based authentication
     * + AES-CTR encryption
     */
    NETWORK_PROV_SECURITY_1 = 1,
#endif
#ifdef CONFIG_ESP_PROTOCOMM_SUPPORT_SECURITY_VERSION_2
    /**
     * This secure communication mode consists of
     *  SRP6a based authentication and key exchange
     *  + AES-GCM encryption/decryption
     */
    NETWORK_PROV_SECURITY_2 = 2
#endif
} network_prov_security_t;

/**
 * @brief  Security 1 params structure
 *         This needs to be passed when using NETWORK_PROV_SECURITY_1
 */
typedef const char network_prov_security1_params_t;

/**
 * @brief  Security 2 params structure
 *         This needs to be passed when using NETWORK_PROV_SECURITY_2
 */
typedef protocomm_security2_params_t network_prov_security2_params_t;

/**
 * @brief   Initialize provisioning manager instance
 *
 * Configures the manager and allocates internal resources
 *
 * Configuration specifies the provisioning scheme (transport)
 * and event handlers
 *
 * Event NETWORK_PROV_INIT is emitted right after initialization
 * is complete
 *
 * @param[in] config Configuration structure
 *
 * @return
 *  - ESP_OK      : Success
 *  - ESP_FAIL    : Fail
 */
esp_err_t network_prov_mgr_init(network_prov_mgr_config_t config);

/**
 * @brief   Stop provisioning (if running) and release
 *          resource used by the manager
 *
 * Event NETWORK_PROV_DEINIT is emitted right after de-initialization
 * is finished
 *
 * If provisioning service is  still active when this API is called,
 * it first stops the service, hence emitting NETWORK_PROV_END, and
 * then performs the de-initialization
 */
void network_prov_mgr_deinit(void);

#ifdef CONFIG_NETWORK_PROV_NETWORK_TYPE_WIFI
/**
 * @brief   Checks if device is provisioned
 *
 * This checks if Wi-Fi credentials are present on the NVS
 *
 * The Wi-Fi credentials are assumed to be kept in the same
 * NVS namespace as used by esp_wifi component
 *
 * If one were to call esp_wifi_set_config() directly instead
 * of going through the provisioning process, this function will
 * still yield true (i.e. device will be found to be provisioned)
 *
 * @note    Calling network_prov_mgr_start_provisioning() automatically
 *          resets the provision state, irrespective of what the
 *          state was prior to making the call.
 *
 * @param[out] provisioned  True if provisioned, else false
 *
 * @return
 *  - ESP_OK      : Retrieved provision state successfully
 *  - ESP_FAIL    : Wi-Fi not initialized
 *  - ESP_ERR_INVALID_ARG   : Null argument supplied
 */
esp_err_t network_prov_mgr_is_wifi_provisioned(bool *provisioned);
#endif // CONFIG_NETWORK_PROV_NETWORK_TYPE_WIFI

#ifdef CONFIG_NETWORK_PROV_NETWORK_TYPE_THREAD
/**
 * @brief   Checks if device is provisioned
 *
 * This checks if there is active dataset for Thread device
 *
 * @note    Calling network_prov_mgr_start_provisioning() automatically
 *          resets the provision state, irrespective of what the
 *          state was prior to making the call.
 *
 * @param[out] provisioned  True if provisioned, else false
 *
 * @return
 *  - ESP_OK      : Retrieved provision state successfully
 *  - ESP_FAIL    : Not initialized
 *  - ESP_ERR_INVALID_ARG   : Null argument supplied
 */
esp_err_t network_prov_mgr_is_thread_provisioned(bool *provisioned);
#endif // CONFIG_NETWORK_PROV_NETWORK_TYPE_THREAD

/**
 * @brief   Checks whether the provisioning state machine is idle
 *
 * @return  True if state machine is idle, else false
 */
bool network_prov_mgr_is_sm_idle(void);

/**
 * @brief   Start provisioning service
 *
 * This starts the provisioning service according to the scheme
 * configured at the time of initialization. For scheme :
 * - network_prov_scheme_ble : This starts protocomm_ble, which internally initializes
 *                          BLE transport and starts GATT server for handling
 *                          provisioning requests
 * - network_prov_scheme_softap : This activates SoftAP mode of Wi-Fi and starts
 *                          protocomm_httpd, which internally starts an HTTP
 *                          server for handling provisioning requests (If mDNS is
 *                          active it also starts advertising service with type
 *                          _esp_wifi_prov._tcp)
 *
 * Event NETWORK_PROV_START is emitted right after provisioning starts without failure
 *
 * @note   This API will start provisioning service even if device is found to be
 *         already provisioned, i.e. network_prov_mgr_is_wifi_provisioned() yields true
 *
 * @param[in] security      Specify which protocomm security scheme to use :
 *                              - NETWORK_PROV_SECURITY_0 : For no security
 *                              - NETWORK_PROV_SECURITY_1 : x25519 secure handshake for session
 *                                establishment followed by AES-CTR encryption of provisioning messages
 *                              - NETWORK_PROV_SECURITY_2:  SRP6a based authentication and key exchange
 *                                followed by AES-GCM encryption/decryption of provisioning messages
 * @param[in] network_prov_sec_params
 *                          Pointer to security params (NULL if not needed).
 *                          This is not needed for protocomm security 0
 *                          This pointer should hold the struct of type
 *                          network_prov_security1_params_t for protocomm security 1
 *                          and network_prov_security2_params_t for protocomm security 2 respectively.
 *                          This pointer and its contents should be valid till the provisioning service is
 *                          running and has not been stopped or de-inited.
 * @param[in] service_name  Unique name of the service. This translates to:
 *                              - Wi-Fi SSID when provisioning mode is softAP
 *                              - Device name when provisioning mode is BLE
 * @param[in] service_key   Key required by client to access the service (NULL if not needed).
 *                          This translates to:
 *                              - Wi-Fi password when provisioning mode is softAP
 *                              - ignored when provisioning mode is BLE
 *
 * @return
 *  - ESP_OK      : Provisioning started successfully
 *  - ESP_FAIL    : Failed to start provisioning service
 *  - ESP_ERR_INVALID_STATE : Provisioning manager not initialized or already started
 */
esp_err_t network_prov_mgr_start_provisioning(network_prov_security_t security, const void *network_prov_sec_params, const char *service_name, const char *service_key);

/**
 * @brief   Stop provisioning service
 *
 * If provisioning service is active, this API will initiate a process to stop
 * the service and return. Once the service actually stops, the event NETWORK_PROV_END
 * will be emitted.
 *
 * If network_prov_mgr_deinit() is called without calling this API first, it will
 * automatically stop the provisioning service and emit the NETWORK_PROV_END, followed
 * by NETWORK_PROV_DEINIT, before returning.
 *
 * This API will generally be used along with network_prov_mgr_disable_auto_stop()
 * in the scenario when the main application has registered its own endpoints,
 * and wishes that the provisioning service is stopped only when some protocomm
 * command from the client side application is received.
 *
 * Calling this API inside an endpoint handler, with sufficient cleanup_delay,
 * will allow the response / acknowledgment to be sent successfully before the
 * underlying protocomm service is stopped.
 *
 * Cleaup_delay is set when calling network_prov_mgr_disable_auto_stop().
 * If not specified, it defaults to 1000ms.
 *
 * For straightforward cases, using this API is usually not necessary as
 * provisioning is stopped automatically once NETWORK_PROV_CRED_SUCCESS is emitted.
 * Stopping is delayed (maximum 30 seconds) thus allowing the client side
 * application to query for network state, i.e. after receiving the first query
 * and sending `network state connected` response the service is stopped immediately.
 */
void network_prov_mgr_stop_provisioning(void);

/**
 * @brief   Wait for provisioning service to finish
 *
 * Calling this API will block until provisioning service is stopped
 * i.e. till event NETWORK_PROV_END is emitted.
 *
 * This will not block if provisioning is not started or not initialized.
 */
void network_prov_mgr_wait(void);

/**
 * @brief   Disable auto stopping of provisioning service upon completion
 *
 * By default, once provisioning is complete, the provisioning service is automatically
 * stopped, and all endpoints (along with those registered by main application) are
 * deactivated.
 *
 * This API is useful in the case when main application wishes to close provisioning service
 * only after it receives some protocomm command from the client side app. For example, after
 * connecting to network, the device may want to connect to the cloud, and only once that is
 * successfully, the device is said to be fully configured. But, then it is upto the main
 * application to explicitly call network_prov_mgr_stop_provisioning() later when the device is
 * fully configured and the provisioning service is no longer required.
 *
 * @note    This must be called before executing network_prov_mgr_start_provisioning()
 *
 * @param[in] cleanup_delay Sets the delay after which the actual cleanup of transport related
 *                          resources is done after a call to network_prov_mgr_stop_provisioning()
 *                          returns. Minimum allowed value is 100ms. If not specified, this will
 *                          default to 1000ms.
 *
 * @return
 *  - ESP_OK : Success
 *  - ESP_ERR_INVALID_STATE : Manager not initialized or
 *                            provisioning service already started
 */
esp_err_t network_prov_mgr_disable_auto_stop(uint32_t cleanup_delay);

/**
 * @brief   Set application version and capabilities in the JSON data returned by
 *          proto-ver endpoint
 *
 * This function can be called multiple times, to specify information about the various
 * application specific services running on the device, identified by unique labels.
 *
 * The provisioning service itself registers an entry in the JSON data, by the label "prov",
 * containing only provisioning service version and capabilities. Application services should
 * use a label other than "prov" so as not to overwrite this.
 *
 * @note    This must be called before executing network_prov_mgr_start_provisioning()
 *
 * @param[in] label   String indicating the application name.
 *
 * @param[in] version String indicating the application version.
 *                    There is no constraint on format.
 *
 * @param[in] capabilities  Array of strings with capabilities.
 *                          These could be used by the client side app to know
 *                          the application registered endpoint capabilities
 *
 * @param[in] total_capabilities  Size of capabilities array
 *
 * @return
 *  - ESP_OK : Success
 *  - ESP_ERR_INVALID_STATE : Manager not initialized or
 *                            provisioning service already started
 *  - ESP_ERR_NO_MEM : Failed to allocate memory for version string
 *  - ESP_ERR_INVALID_ARG : Null argument
 */
esp_err_t network_prov_mgr_set_app_info(const char *label, const char *version,
                                        const char **capabilities, size_t total_capabilities);

/**
 * @brief   Create an additional endpoint and allocate internal resources for it
 *
 * This API is to be called by the application if it wants to create an additional
 * endpoint. All additional endpoints will be assigned UUIDs starting from 0xFF54
 * and so on in the order of execution.
 *
 * protocomm handler for the created endpoint is to be registered later using
 * network_prov_mgr_endpoint_register() after provisioning has started.
 *
 * @note    This API can only be called BEFORE provisioning is started
 *
 * @note    Additional endpoints can be used for configuring client provided
 *          parameters other than Wi-Fi credentials or Thread dataset, that are necessary
 *          for the main application and hence must be set prior to starting the application
 *
 * @note    After session establishment, the additional endpoints must be targeted
 *          first by the client side application before sending Wi-Fi/Thread configuration,
 *          because once Wi-Fi/Thread configuration finishes the provisioning service is
 *          stopped and hence all endpoints are unregistered
 *
 * @param[in] ep_name  unique name of the endpoint
 *
 * @return
 *  - ESP_OK      : Success
 *  - ESP_FAIL    : Failure
 */
esp_err_t network_prov_mgr_endpoint_create(const char *ep_name);

/**
 * @brief   Register a handler for the previously created endpoint
 *
 * This API can be called by the application to register a protocomm handler
 * to any endpoint that was created using network_prov_mgr_endpoint_create().
 *
 * @note    This API can only be called AFTER provisioning has started
 *
 * @note    Additional endpoints can be used for configuring client provided
 *          parameters other than Wi-Fi credentials or Thread dataset, that are necessary
 *          for the main application and hence must be set prior to starting the application
 *
 * @note    After session establishment, the additional endpoints must be targeted
 *          first by the client side application before sending Wi-Fi/Thread configuration,
 *          because once Wi-Fi/Thread configuration finishes the provisioning service is
 *          stopped and hence all endpoints are unregistered
 *
 * @param[in] ep_name   Name of the endpoint
 * @param[in] handler   Endpoint handler function
 * @param[in] user_ctx  User data
 *
 * @return
 *  - ESP_OK      : Success
 *  - ESP_FAIL    : Failure
 */
esp_err_t network_prov_mgr_endpoint_register(const char *ep_name,
        protocomm_req_handler_t handler,
        void *user_ctx);

/**
 * @brief   Unregister the handler for an endpoint
 *
 * This API can be called if the application wants to selectively
 * unregister the handler of an endpoint while the provisioning
 * is still in progress.
 *
 * All the endpoint handlers are unregistered automatically when
 * the provisioning stops.
 *
 * @param[in] ep_name  Name of the endpoint
 */
void network_prov_mgr_endpoint_unregister(const char *ep_name);

#ifdef CONFIG_NETWORK_PROV_NETWORK_TYPE_WIFI
/**
 * @brief   Get state of Wi-Fi Station during provisioning
 *
 * @param[out] state    Pointer to network_prov_wifi_sta_state_t
 *                      variable to be filled
 *
 * @return
 *  - ESP_OK    : Successfully retrieved Wi-Fi state
 *  - ESP_FAIL  : Provisioning app not running
 */
esp_err_t network_prov_mgr_get_wifi_state(network_prov_wifi_sta_state_t *state);

/**
 * @brief   Get reason code in case of Wi-Fi station
 *          disconnection during provisioning
 *
* @param[out] reason    Pointer to network_prov_wifi_sta_fail_reason_t
*                       variable to be filled
 *
 * @return
 *  - ESP_OK    : Successfully retrieved Wi-Fi disconnect reason
 *  - ESP_FAIL  : Provisioning app not running
 */
esp_err_t network_prov_mgr_get_wifi_disconnect_reason(network_prov_wifi_sta_fail_reason_t *reason);

/**
 * @brief   Runs Wi-Fi as Station with the supplied configuration
 *
 * Configures the Wi-Fi station mode to connect to the AP with
 * SSID and password specified in config structure and sets
 * Wi-Fi to run as station.
 *
 * This is automatically called by provisioning service upon
 * receiving new credentials.
 *
 * If credentials are to be supplied to the manager via a
 * different mode other than through protocomm, then this
 * API needs to be called.
 *
 * Event NETWORK_PROV_CRED_RECV is emitted after credentials have
 * been applied and Wi-Fi station started
 *
 * @param[in] wifi_cfg  Pointer to Wi-Fi configuration structure
 *
 * @return
 *  - ESP_OK      : Wi-Fi configured and started successfully
 *  - ESP_FAIL    : Failed to set configuration
 */
esp_err_t network_prov_mgr_configure_wifi_sta(wifi_config_t *wifi_cfg);

/**
 * @brief   Reset Wi-Fi provisioning config
 *
 * Calling this API will restore WiFi stack persistent settings to default values.
 *
 * @return
 *  - ESP_OK      : Reset provisioning config successfully
 *  - ESP_FAIL    : Failed to reset provisioning config
 */
esp_err_t network_prov_mgr_reset_wifi_provisioning(void);

/**
 * @brief   Reset internal state machine and clear provisioned credentials.
 *
 * This API should be used to restart provisioning ONLY in the case
 * of provisioning failures without rebooting the device.
 *
 * @return
 *  - ESP_OK      : Reset provisioning state machine successfully
 *  - ESP_FAIL    : Failed to reset provisioning state machine
 *  - ESP_ERR_INVALID_STATE : Manager not initialized
 */
esp_err_t network_prov_mgr_reset_wifi_sm_state_on_failure(void);

/**
 * @brief   Reset internal state machine and clear provisioned credentials.
 *
 * This API can be used to restart provisioning ONLY in case the device is
 * to be provisioned again for new credentials after a previous successful
 * provisioning without rebooting the device.
 *
 * @note   This API can be used only if provisioning auto-stop has been
 *         disabled using network_prov_mgr_disable_auto_stop()
 *
 * @return
 *  - ESP_OK      : Reset provisioning state machine successfully
 *  - ESP_FAIL    : Failed to reset provisioning state machine
 *  - ESP_ERR_INVALID_STATE : Manager not initialized
 */
esp_err_t network_prov_mgr_reset_wifi_sm_state_for_reprovision(void);
#endif // CONFIG_NETWORK_PROV_NETWORK_TYPE_WIFI

#ifdef CONFIG_NETWORK_PROV_NETWORK_TYPE_THREAD
/**
 * @brief   Reset Thread provisioning config
 *
 * Calling this API will restore Thread stack persistent settings to default values.
 *
 * @return
 *  - ESP_OK      : Reset provisioning config successfully
 *  - ESP_FAIL    : Failed to reset provisioning config
 */
esp_err_t network_prov_mgr_reset_thread_provisioning(void);

/**
 * @brief   Get state of Thread during provisioning
 *
 * @param[out] state    Pointer to network_prov_thread_state_t
 *                      variable to be filled
 *
 * @return
 *  - ESP_OK    : Successfully retrieved Thread state
 *  - ESP_FAIL  : Provisioning app not running
 */
esp_err_t network_prov_mgr_get_thread_state(network_prov_thread_state_t *state);

/**
 * @brief   Get reason code in case of thread detached during provisioning
 *
* @param[out] reason    Pointer to network_prov_thread_fail_reason_t
*                       variable to be filled
 *
 * @return
 *  - ESP_OK    : Successfully retrieved thread detached reason
 *  - ESP_FAIL  : Provisioning app not running
 */
esp_err_t network_prov_mgr_get_thread_detached_reason(network_prov_thread_fail_reason_t *reason);

/**
 * @brief   Runs Thread with the supplied configuration
 *
 * Configures the Thread Dataset so that the device can be attached
 * to a specific Thread network
 *
 * This is automatically called by provisioning service upon
 * receiving new Thread dataset.
 *
 * If the dataset is to be supplied to the manager via a
 * different mode other than through protocomm, then this
 * API needs to be called.
 *
 * Event THREAD_PROV_CRED_RECV is emitted after credentials have
 * been applied and Thread started
 *
 * @param[in] thread_dataset  Pointer to Dataset Tlvs structure
 *
 * @return
 *  - ESP_OK      : Thread dataset configured and started successfully
 *  - ESP_FAIL    : Failed to set configuration
 */
esp_err_t network_prov_mgr_configure_thread_dataset(otOperationalDatasetTlvs *thread_dataset);

/**
 * @brief   Reset internal state machine and clear provisioned credentials.
 *
 * This API should be used to restart provisioning ONLY in the case
 * of provisioning failures without rebooting the device.
 *
 * @return
 *  - ESP_OK      : Reset provisioning state machine successfully
 *  - ESP_FAIL    : Failed to reset provisioning state machine
 *  - ESP_ERR_INVALID_STATE : Manager not initialized
 */
esp_err_t network_prov_mgr_reset_thread_sm_state_on_failure(void);

/**
 * @brief   Reset internal state machine and clear provisioned credentials.
 *
 * This API can be used to restart provisioning ONLY in case the device is
 * to be provisioned again for new credentials after a previous successful
 * provisioning without rebooting the device.
 *
 * @note   This API can be used only if provisioning auto-stop has been
 *         disabled using network_prov_mgr_disable_auto_stop()
 *
 * @return
 *  - ESP_OK      : Reset provisioning state machine successfully
 *  - ESP_FAIL    : Failed to reset provisioning state machine
 *  - ESP_ERR_INVALID_STATE : Manager not initialized
 */
esp_err_t network_prov_mgr_reset_thread_sm_state_for_reprovision(void);

#endif // CONFIG_NETWORK_PROV_NETWORK_TYPE_THREAD

#ifdef __cplusplus
}
#endif
