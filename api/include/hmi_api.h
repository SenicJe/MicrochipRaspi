/******************************************************************************
 *
 * Copyright (C) 2014 Microchip Technology Inc. and its
 *                    subsidiaries ("Microchip").
 *
 * All rights reserved.
 *
 * You are permitted to use the Aurea software, 3DTouchPad SDK, and other
 * accompanying software with Microchip products.  Refer to the license
 * agreement accompanying this software, if any, for additional info regarding
 * your rights and obligations.
 *
 * SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
 * MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR
 * PURPOSE. IN NO EVENT SHALL MICROCHIP, SMSC, OR ITS LICENSORS BE LIABLE OR
 * OBLIGATED UNDER CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH
 * OF WARRANTY, OR OTHER LEGAL EQUITABLE THEORY FOR ANY DIRECT OR INDIRECT
 * DAMAGES OR EXPENSES INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES, OR OTHER SIMILAR COSTS.
 *
 ******************************************************************************/
#ifndef HMI_API_H
#define HMI_API_H


/* ======== API-Revision ======== */

#define HMI_API_REV 1

/* ======== IO-Types ======== */

/* Possible IO-types */
#define HMI_IO_CUSTOM 0
#define HMI_IO_CDC_SERIAL 1
#define HMI_IO_HID_3DTOUCHPAD 2

/* ======== Declaration of hmi_t ======== */

#ifndef HMI_NO_HMI_TYPEDEF
typedef struct hmi_struct hmi_t;
#endif

/* hmi_custom.h allows to provide extra configuration and functionality
 * on a per project-basis.
 * This is mainly used by projects for embedded devices that provide their
 * IO-functionality and configuration this way.
 */
#ifdef HMI_CUSTOM
#include "hmi_custom.h"
#endif

#if defined(HMI_API_DYNAMIC) && defined(_WIN32)
#   ifdef HMI_API_EXPORT
#       define HMI_API __declspec(dllexport)
#   else
#       define HMI_API __declspec(dllimport)
#   endif
#else
#   define HMI_API
#endif

#ifndef CDECL
#   ifdef _WIN32
#       define CDECL __cdecl
#   else
#       define CDECL
#   endif
#endif

#ifndef HMI_UNDEFINED_VALUE
#   define HMI_UNDEFINED_VALUE 0
#endif

/* ======== IO-related configuration ======== */

/* Default IO-implementation */
#ifndef HMI_IO
#if defined(_WIN32) || defined(__linux__)
#   define HMI_IO HMI_IO_HID_3DTOUCHPAD
#else
#   error "No IO-implementation selected"
#endif
#endif

#if HMI_IO != HMI_IO_CUSTOM && HMI_IO != HMI_IO_HID_3DTOUCHPAD && \
    HMI_IO != HMI_IO_CDC_SERIAL
#   error "Unknown IO implementation selected"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ======== Error Codes ======== */

/* Enum: hmi_error_t
 *
 * Error codes as returned by functions of the HMI API.
 *
 * HMI_NO_ERROR                - No Error Occured
 * HMI_NO_DATA                 - <hmi3d_retrieve_data> could not retrieve new data.
 *                               This code does not necessarily mark an error.
 * HMI_3D_SYSTEM_ERROR         - Last System_Status message contained an error state
 *                               See <hmi3d_get_system_error> and <hmi3d_system_error_t> for details.
 * HMI_NO_RESPONSE_ERROR       - Could not receive an response for the last sent instruction
 * HMI_MSG_MISSING_ERROR       - Request for message was acknowledged but no message or requested type
 *                               was received.
 * HMI_2D_BOOTLOADER_ERROR     - Last response to an 2D update function contained an error state
 *                               See <hmi2d_get_bootloader_error> and <hmi2d_bootloader_error_t> for details.
 * HMI_IO_ERROR                - There was some unspecified error during communication
 * HMI_IO_CTL_ERROR            - Configuration of the connection to the device failed with an error
 * HMI_IO_OPEN_ERROR           - Opening the connection to the device failed
 * HMI_IO_ENUM_ERROR           - Error during automatic device detection.
 *                               This error is specific to systems that provide automatic
 *                               device detection
 * HMI_BAD_PARAM_ERROR         - Parameter of a function call was invalid in that context
 * HMI_NO_IMPLEMENTATION_ERROR - The implementation of the called function is missing or incomplete
 */

typedef enum {
    HMI_NO_ERROR = 0,
    HMI_NO_DATA = -1,
    HMI_3D_SYSTEM_ERROR = -8,
    HMI_NO_RESPONSE_ERROR = -9,
    HMI_MSG_MISSING_ERROR = -10,
    HMI_2D_BOOTLOADER_ERROR = -11,
    HMI_IO_ERROR = -16,
    HMI_IO_CTL_ERROR = -17,
    HMI_IO_OPEN_ERROR = -18,
    HMI_IO_ENUM_ERROR = -19,
    HMI_BAD_PARAM_ERROR = -32,
    HMI_NO_IMPLEMENTATION_ERROR = -48
} hmi_error_t;

/* Enum: hmi3d_system_error_t
 *
 * Error codes as returned from the MGC3130 system.
 *
 * hmi3d_system_NoError              - No error occured
 * hmi3d_system_UnknownCommand       - Message ID is unknown
 * hmi3d_system_InvalidSessionid     - Session ID is invalid or does not match (<3D Firmware Update>)
 * hmi3d_system_InvalidCrc           - CRC is invalid (<3D Firmware Update>)
 * hmi3d_system_InvalidLength        - Length is invalid (<3D Firmware Update>)
 * hmi3d_system_InvalidAddress       - Address is invalid (<3D Firmware Update>)
 * hmi3d_system_InvalidFunction      - Function id is invalid (<3D Firmware Update>)
 * hmi3d_system_ContentMismatch      - <hmi3d_UpdateFunction_VerifyOnly> found mismatch between
 *                                     sent data block and flash memory (<3D Firmware Update>)
 * hmi3d_system_WrongParameterAddr   - Parameter start address does not match
 *                                     Library Loader assumptions (<3D Firmware Update>)
 * hmi3d_system_WrongParameterValue  - Parameter Value is invalid
 * hmi3d_system_UnknownParameterID   - Parameter ID is invalid
 * hmi3d_system_WakeupHappened       - Device was sleeping and therefore could not
 *                                     process the request
 * hmi3d_system_LoaderUpdateStarted  - Library Loader Update has started (<3D Firmware Update>)
 * hmi3d_system_LoaderUpdateFinished - Library Loader Update has finished (<3D Firmware Update>)
 */
typedef enum {
    hmi3d_system_NoError = 0,
    hmi3d_system_UnknownCommand = 0x01,
    hmi3d_system_InvalidSessionid = 0x02,
    hmi3d_system_InvalidCrc = 0x03,
    hmi3d_system_InvalidLength = 0x04,
    hmi3d_system_InvalidAddress = 0x05,
    hmi3d_system_InvalidFunction = 0x06,
    hmi3d_system_ContentMismatch = 0x08,
    hmi3d_system_WrongParameterAddr = 0x0B,
    hmi3d_system_WrongParameterValue = 0x14,
    hmi3d_system_UnknownParameterID = 0x15,
    hmi3d_system_WakeupHappened = 0x1A,
    hmi3d_system_LoaderUpdateStarted = 0x80,
    hmi3d_system_LoaderUpdateFinished = 0x81
} hmi3d_system_error_t;


/* ======== Common Functions ======== */

/* Function: hmi_initialize
 *
 * Initializes hmi for further use. Resources that were aquired with
 * <hmi_initialize> have to be released with <hmi_cleanup> once they are
 * no longer needed.
 *
 * hmi - The <hmi_t>-instance to be initialized.
 *          New instances could be aquired with <hmi_create>.
 *
 * See also:
 *    <hmi_create>, <hmi_free>, <hmi_cleanup>
 */
HMI_API void CDECL hmi_initialize(hmi_t *hmi);

/* Function: hmi_cleanup
 *
 * Releases resources that were aquired by <hmi_initialize>.
 * 
 * See also:
 *    <hmi_initialize>
 */
HMI_API void CDECL hmi_cleanup(hmi_t *hmi);

/* ======== Logging ======== */

#ifndef HMI_NO_LOGGING

#include <stdarg.h>

/* Function: hmi_log
 *
 * Calls the logger set with <hmi_set_logger> to do printf like output.
 *
 * fmt - The format string
 * ... - The extra parameters that are forwarded via va_list
 *
 * If not logger is set this function returns 0. Otherwise it returns
 * the return-value of the call to the logger-function.
 *
 * See also:
 *    <hmi_set_logger>
 */
HMI_API int CDECL hmi_log(hmi_t *hmi, const char *fmt, ...);

/* Typedef: hmi_logger_t
 *
 * Definition of the signature for logger functions.
 *
 * opaque - <hmi_log> will pass the opaque pointer provided with
 *          <hmi_set_logger>
 * fmt    - The format string
 * vlist  - List of the variadic arguments
 *
 * See also:
 *    <hmi_set_logger>
 */
typedef int (CDECL* hmi_logger_t)(void *opaque,
                                  const char *fmt,
                                  va_list vlist);

/* Function: hmi_set_logger
 *
 * Sets the logger implementation that is called by <hmi_log>.
 *
 * logger - A logger function accepting the opaque-pointer, format string
 *          and the variadic argument list as parameters
 * opaque - An opaque pointer that is provided on calls of logger as the
 *          first argument
 *
 * This function always return 0.
 *
 * An example of valid functions for logger is vfprintf.
 *
 * See also:
 *    <hmi_log>, <hmi_logger_t>
 */
HMI_API int CDECL hmi_set_logger(hmi_t *hmi,
                                    hmi_logger_t logger,
                                    void *opaque);

#endif

/* ======== Connection Handling ======== */

#if HMI_IO == HMI_IO_HID_3DTOUCHPAD

/* Function: hmi_hid_set_details
 *
 * Sets the Vendor ID, Product ID, Report ID and Interface that is used by
 * the HID connection.
 *
 * vendor_id  - The Vendor ID of the device to open
 * product_id - The Product ID of the device to open
 * report_id  - The report ID to be used in the communication
 * iface      - The interface of the device to open (not needed on Windows)
 *
 * This function is only defined when compiled for communication with the
 * 3DTouchPad.
 *
 * It should be used when connection with different IDs than the default
 * is required. This is the case for example when connecting to the
 * 2D bootloader device during <2D Firmware Update>.
 */
HMI_API void CDECL hmi_hid_set_details(hmi_t *hmi,
                                       int vendor_id,
                                       int product_id,
                                       int report_id,
                                       int iface);

#endif

/* Function: hmi_open
 *
 * Opens a connection to the physical device and associates it with hmi
 * that was already initialized with <hmi_initialize>.
 *
 * Returns 0 on success or a negative value on error.
 *
 * The connection has to be closed with <hmi_close> after use.
 *
 * See also:
 *    <hmi_initialize>, <hmi_close>
 */
HMI_API int CDECL hmi_open(hmi_t *hmi);

/* Function: hmi_close
 *
 * Closes the connection to the device associated with hmi that was
 * established with <hmi_open>.
 */
HMI_API void CDECL hmi_close(hmi_t *hmi);

/* Function: hmi3d_reset
 *
 * Tries to reset the 3D-device if this is supported by the system setup.
 *
 * Returns 0 on success or a negative value on error.
 *
 * Resetting the hardware is done by signaling the reset line of the chip.
 * This requires support by the hardware connection.
 */
HMI_API int CDECL hmi3d_reset(hmi_t *hmi);

/* ======== 3D Low Level Communication ======== */

/* Enum: hmi3d_message_id_t
 *
 * Enumeration of message IDs sent from or to device.
 *
 * hmi3d_msg_System_Status         - Response to other messages
 * hmi3d_msg_Request_Message       - Request for a message sent by host
 * hmi3d_msg_Fw_Update_Start       - Starts <3D Firmware Update>
 * hmi3d_msg_Fw_Update_Block       - Content during <3D Firmware Update>
 * hmi3d_msg_Fw_Update_Completed   - Finishes <3D Firmware Update>
 * hmi3d_msg_Fw_Version_Info       - Contains FW Version data
 * hmi3d_msg_Sensor_Data_Output    - Contains different sensor data
 * hmi3d_msg_Set_Runtime_Parameter - Update of runtime parameter if from host
 *                                   Requested parameter value if from device
 *
 * Some of the message IDs are used for messages in both directions with a
 * different meaning.
 */
typedef enum {
    hmi3d_msg_System_Status = 0x15,
    hmi3d_msg_Request_Message = 0x06,
    hmi3d_msg_Fw_Update_Start = 0x80,
    hmi3d_msg_Fw_Update_Block = 0x81,
    hmi3d_msg_Fw_Update_Completed = 0x82,
    hmi3d_msg_Fw_Version_Info = 0x83,
    hmi3d_msg_Sensor_Data_Output = 0x91,
    hmi3d_msg_Set_Runtime_Parameter = 0xA2,
} hmi3d_message_id_t;

/* Enum: hmi3d_parameter_id_t
 *
 * Enumeration of possible parameter ids.
 * For the correct arguments refer to the MGC3130 GestIC Library
 * Interface Description User's Guids or use the
 * <3D Real Time Control> functions.
 *
 * hmi3d_param_trigger                     - Trigger an action.
 *                                           See <hmi3d_trigger_action>
 * hmi3d_param_makePersistent              - Make changes persistent.
 * hmi3d_param_afeRxAtt_S                  - Signal Matching for South electrode
 * hmi3d_param_afeRxAtt_W                  - Signal Matching for West electrode
 * hmi3d_param_afeRxAtt_N                  - Signal Matching for North electrode
 * hmi3d_param_afeRxAtt_E                  - Signal Matching for East electrode
 * hmi3d_param_afeRxAtt_C                  - Signal Matching for Center electrode
 * hmi3d_param_channelmapping_S            - Physical channel for South electrode
 * hmi3d_param_channelmapping_W            - Physical channel for West electrode
 * hmi3d_param_channelmapping_N            - Physical channel for North electrode
 * hmi3d_param_channelmapping_E            - Physical channel for East electrode
 * hmi3d_param_channelmapping_C            - Physical channel for Center electrode
 * hmi3d_param_dspCalOpMode                - Calibration Operation Mode Flags.
 *                                           See <hmi3d_set_auto_calibration>
 * hmi3d_param_transFreqSelect             - Transmit Frequency Selection.
 *                                           See <hmi3d_select_frequencies>
 * hmi3d_param_dspGestureMask              - Mask for Gesture Processing.
 *                                           See <hmi3d_set_enabled_gestures>
 * hmi3d_param_dspAirWheelConfig           - Enables/Disables AirWheel.
 *                                           See <hmi3d_set_air_wheel_enabled>
 * hmi3d_param_dspTouchConfig              - Enables/Disables Touch Detection.
 *                                           See <hmi3d_set_touch_detection>
 * hmi3d_param_dspApproachDetectionMode    - Enables/Disables Approach Detection.
 *                                           See <hmi3d_set_approach_detection>
 * hmi3d_param_dataOutputEnableMask        - Determines the continuous data output of the device.
 *                                           See <hmi3d_set_output_enable_mask>
 * hmi3d_param_dataOutputLockMask          - Determines the continuous data output of the device.
 *                                           See <hmi3d_set_output_enable_mask>
 * hmi3d_param_dataOutputRequestMask       - Determines the data output for next message.
 * hmi3d_param_dataOutputGestureInProgress - Enables/Disables output of
 *                                           gestures in progress
 */
typedef enum {
    hmi3d_param_trigger = 0x1000,
    hmi3d_param_makePersistent = 0xFF00,
    hmi3d_param_afeRxAtt_S = 0x50,
    hmi3d_param_afeRxAtt_W = 0x51,
    hmi3d_param_afeRxAtt_N = 0x52,
    hmi3d_param_afeRxAtt_E = 0x53,
    hmi3d_param_afeRxAtt_C = 0x54,
    hmi3d_param_channelmapping_S = 0x65,
    hmi3d_param_channelmapping_W = 0x66,
    hmi3d_param_channelmapping_N = 0x67,
    hmi3d_param_channelmapping_E = 0x68,
    hmi3d_param_channelmapping_C = 0x69,
    hmi3d_param_dspCalOpMode = 0x80,
    hmi3d_param_transFreqSelect = 0x82,
    hmi3d_param_dspGestureMask = 0x85,
    hmi3d_param_dspAirWheelConfig = 0x90,
    hmi3d_param_dspTouchConfig = 0x97,
    hmi3d_param_dspApproachDetectionMode = 0x97,
    hmi3d_param_dataOutputEnableMask = 0xA0,
    hmi3d_param_dataOutputLockMask = 0xA1,
    hmi3d_param_dataOutputRequestMask = 0xA2,
    hmi3d_param_dataOutputGestureInProgress = 0xA3
} hmi3d_parameter_id_t;

/* Enum: hmi3d_trigger_id_t
 *
 * Actions that could be triggered with <hmi3d_trigger_action>.
 *
 * hmi3d_trigger_calibration     - Triggers manual calibration
 *                                 (see also <hmi3d_force_calibration>).
 * hmi3d_trigger_enterDeepSleep1 - Enters deep sleep mode 1. Device will wake up
 *                                 at incoming messages or hardware reset
 *                                 (see also <hmi3d_reset>).
 * hmi3d_trigger_enterDeepSleep2 - Enters deep sleep mode 2. Device will wake up
 *                                 at falling edge on External Interrupt (IRQ0)
 *                                 or hardware reset (see also <hmi3d_reset>).
 *
 * See also:
 *    <hmi3d_trigger_action>, <hmi3d_param_trigger>
 */
typedef enum {
    hmi3d_trigger_calibration = 0x00,
    hmi3d_trigger_enterDeepSleep1 = 0x01,
    hmi3d_trigger_enterDeepSleep2 = 0x02
} hmi3d_trigger_id_t;

/* Enum: hmi3d_DataOutConfigMask_t
 *
 * Enumeration of flags for possible sensor data output.
 *
 * This enumeration is used for selecting what data to communicate
 * via <hmi3d_set_output_enable_mask> and in <hmi3d_msg_Sensor_Data_Output>
 * to mark available fields and the format of the raw sensor data.
 *
 * hmi3d_DataOutConfigMask_DSPStatus    - DSP related information like
 *                                        calibration events amnd frequency
 *                                        (accessed via <hmi3d_get_calibration>
 *                                        and <hmi3d_get_frequency>)
 * hmi3d_DataOutConfigMask_GestureInfo  - Gesture related data
 *                                        (accessible via <hmi3d_get_gesture>)
 * hmi3d_DataOutConfigMask_TouchInfo    - Touch related data
 *                                        (accessible via <hmi3d_get_touch>)
 * hmi3d_DataOutConfigMask_AirWheelInfo - AirWheel related data
 *                                        (accessible via <hmi3d_get_air_wheel>)
 * hmi3d_DataOutConfigMask_xyzPosition  - Position data (accessible via
 *                                        <hmi3d_get_position>)
 * hmi3d_DataOutConfigMask_NoisePower   - Noise power data (accessible via
 *                                        <hmi3d_get_noise_power>)
 * hmi3d_DataOutConfigMask_CICData      - Raw CIC sensor data (accessible via
 *                                        <hmi3d_get_cic>)
 * hmi3d_DataOutConfigMask_SDData       - SD sensor data (accessible via
 *                                        <hmi3d_get_sd>)
 *
 * The following are masks that are not directly related to data output.
 *
 * hmi3d_DataOutConfigMask_ElectrodeConfiguration - Mask for the electrode
 *                                        configuration. Only relevant for
 *                                        <hmi3d_msg_Sensor_Data_Output>
 *                                        message-handling.
 * hmi3d_DataOutConfigMask_OutputAll    - Mask for all data flags
 */
typedef enum {
    hmi3d_DataOutConfigMask_DSPStatus = 0x0001,
    hmi3d_DataOutConfigMask_GestureInfo = 0x0002,
    hmi3d_DataOutConfigMask_TouchInfo = 0x0004,
    hmi3d_DataOutConfigMask_AirWheelInfo = 0x0008,
    hmi3d_DataOutConfigMask_xyzPosition = 0x0010,
    hmi3d_DataOutConfigMask_NoisePower = 0x0020,
    hmi3d_DataOutConfigMask_CICData = 0x0800,
    hmi3d_DataOutConfigMask_SDData = 0x1000,
    /* Masks */
    hmi3d_DataOutConfigMask_ElectrodeConfiguration = 0x0700,
    hmi3d_DataOutConfigMask_OutputAll = 0x183F
} hmi3d_DataOutConfigMask_t;

/* Enum: hmi3d_SystemInfo_t
 *
 * Contains information about the system state and the content of the
 * containing <hmi3d_msg_Sensor_Data_Output> message.
 *
 * hmi3d_SystemInfo_PositionValid      - Contained position data is valid
 * hmi3d_SystemInfo_AirWheelValid      - Contained AirWheel data is valid
 * hmi3d_SystemInfo_RawDataValid       - Contained RawData fields are valid
 * hmi3d_SystemInfo_NoisePowerValid    - Contained noise-power data is valid
 * hmi3d_SystemInfo_EnvironmentalNoise - Environment noise was detected
 * hmi3d_SystemInfo_Clipping           - Sensor data values are clipped
 * hmi3d_SystemInfo_DSPRunning         - The signal processing (DSP) is running
 */
typedef enum {
    hmi3d_SystemInfo_PositionValid = 0x01,
    hmi3d_SystemInfo_AirWheelValid = 0x02,
    hmi3d_SystemInfo_RawDataValid = 0x04,
    hmi3d_SystemInfo_NoisePowerValid = 0x08,
    hmi3d_SystemInfo_EnvironmentalNoise = 0x10,
    hmi3d_SystemInfo_Clipping = 0x20,
    hmi3d_SystemInfo_DSPRunning = 0x80
} hmi3d_SystemInfo_t;

/* Enum: hmi3d_param_category
 *
 * Enumeration of possible categories for use with <hmi3d_make_persistent>.
 *
 * hmi3d_afe_category    - AFE Category
 * hmi3d_dsp_category    - DSP Category
 * hmi3d_system_category - System Category
 */
typedef enum {
    hmi3d_afe_category = 0,
    hmi3d_dsp_category = 1,
    hmi3d_system_category = 2
} hmi3d_param_category;

/* Function: hmi3d_send_message
 *
 * Sends a message to the device and waits for a response.
 *
 * msg     - Pointer to the message to send
 * size    - The size of the message as set in msg
 * timeout - Timeout in milliseconds to wait for a response
 *
 * Returns 0 on success or a negative value when the request failed.
 *
 * This function calls <wait_response> to wait for an response.
 * If <wait_response> returns an error this function retries up to three times
 * before it in turn returns an error to the caller.
 *
 * See also:
 *    <wait_response>, <hmi3d_set_param>, <hmi3d_trigger_action>
 */
HMI_API int CDECL hmi3d_send_message(hmi_t *hmi,
                                        void *msg,
                                        int size,
                                        int timeout);

/* Function: hmi3d_set_param
 *
 * Sends the instruction for updating a runtime-parameter to the device.
 *
 * param   - The code for the parameter to update
 * arg0    - First parameter specific argument
 * arg1    - Second parameter specific argument
 *
 * Returns 0 on success or a negative value when the request failed.
 *
 * After a failure or no response within the timeout the request gets resend
 * up to three times.
 *
 * See also:
 *    <hmi3d_send_message>, <hmi3d_set_auto_calibration>,
 *    <hmi3d_select_frequencies>, <hmi3d_set_approach_detection>,
 *    <hmi3d_set_enabled_gestures>, <hmi3d_get_param>
 */
HMI_API int CDECL hmi3d_set_param(hmi_t *hmi,
                                     unsigned short param,
                                     unsigned int arg0,
                                     unsigned int arg1);

/* Function: hmi3d_get_param
 *
 * Reads back a parameter from the device.
 *
 * param   - The code for the parameter to read
 * arg0    - Pointer where to store the first argument, could be NULL
 * arg1    - Pointer where to store the second argument, could be NULL
 *
 * Returns 0 on success or a negative value when the request failed.
 *
 * The read parameter arguments are stored in arg0 and arg1 if those pointers
 * are valid.
 *
 * After a failure or no response within the timeout the request gets resend
 * up to three times.
 *
 * See also:
 *    <hmi3d_set_param>
 */
HMI_API int CDECL hmi3d_get_param(hmi_t *hmi,
                                     unsigned short param,
                                     unsigned int *arg0,
                                     unsigned int *arg1);

/* Function: hmi3d_trigger_action
 *
 * Sends the instruction for a specific action to the device.
 *
 * action  - The firmware-specific code for the action
 *
 * Returns 0 on success or a negative value when the request failed.
 *
 * After a failure or no response within the timeout the request gets resend
 * up to three times.
 *
 * The enumeration hmi3d_trigger_id_t provides the possible actions.
 *
 * See also:
 *    <hmi3d_send_message>, <hmi3d_force_calibration>
 */
HMI_API int CDECL hmi3d_trigger_action(hmi_t *hmi,
                                          unsigned short action);


/* ======== 3D Data Retrieval ======== */

#ifndef HMI3D_NO_DATA_RETRIEVAL

/* Enumeration: hmi3d_gestures_t
 *
 * The gestures supported by the API
 *
 * hmi3d_gest_none  - No gesture detected
 * hmi3d_flick_w2e  - Flick from west to east detected
 * hmi3d_flick_e2w  - Flick from east to west was detected
 * hmi3d_flick_s2n  - Flick from south to north was detected
 * hmi3d_flick_n2s  - Flick from north to south was detected
 * hmi3d_circle_cw  - Clock-wise circle detected
 * hmi3d_circle_ccw - Counter-clock-wise circle detected
 */
typedef enum {
    hmi3d_gest_none    = 0x00,
    hmi3d_flick_w2e    = 0x01,
    hmi3d_flick_e2w    = 0x02,
    hmi3d_flick_s2n    = 0x03,
    hmi3d_flick_n2s    = 0x04,
    hmi3d_circle_cw    = 0x05,
    hmi3d_circle_ccw   = 0x06
} hmi3d_gestures_t;

/* Enumeration: hmi3d_gesture_flags_t
 *
 * Bitmask of gesture related flags:
 *
 * hmi3d_gesture_edge_flick  - The flick was actually an edge flick
 * hmi3d_gesture_in_progress - Gesture recognition is in progress.
 *                             Only set while recognizer is active and
 *                             gets reset once gesture got recognized.
 * hmi3d_gesture_flags_mask  - Mask of possible flags.
 */
typedef enum {
    hmi3d_gesture_edge_flick = 0x00010000,
    hmi3d_gesture_in_progress = 0x80000000,
    hmi3d_gesture_flags_mask = 0x80010000
} hmi3d_gesture_flags_t;

/* Enum: hmi3d_touch_flags_t
 *
 * Bitmask of gesture related flags
 *
 * hmi3d_touch_north  - Touch at north electrode detected
 * hmi3d_touch_east   - Touch at east electrode detected
 * hmi3d_touch_south  - Touch at south electrode detected
 * hmi3d_touch_west   - Touch at west electrode detected
 * hmi3d_touch_center - Touch at center electrode detected
 * hmi3d_touch_mask   - Mask for detected touchs
 */
typedef enum {
    hmi3d_touch_north = 0x04,
    hmi3d_touch_east = 0x08,
    hmi3d_touch_south = 0x01,
    hmi3d_touch_west = 0x02,
    hmi3d_touch_center = 0x10,
    hmi3d_touch_mask = 0x1F
} hmi3d_touch_flags_t;

/* Enum: hmi3d_tap_flags_t
 *
 * Flags for the different tap events as they are communicated by the
 * <hmi3d_msg_Sensor_Data_Output> message.
 *
 * hmi3d_tap_north         - Tap on north electrode
 * hmi3d_tap_east          - Tap on east electrode
 * hmi3d_tap_south         - Tap on south electrode
 * hmi3d_tap_west          - Tap on west electrode
 * hmi3d_tap_center        - Tap on center electrode
 * hmi3d_double_tap_north  - Double tap on north electrode
 * hmi3d_double_tap_east   - Double tap on east electrode
 * hmi3d_double_tap_south  - Double tap on south electrode
 * hmi3d_double_tap_west   - Double tap on west electrode
 * hmi3d_double_tap_center - Double tap on center electrode
 * hmi3d_single_tap_mask   - Mask for tap events
 * hmi3d_double_tap_mask   - Mask for double tap events
 * hmi3d_tap_mask          - Mask for tap and double tap events
 *
 * Each tap results in the corresponding tap event while two consecutive taps
 * result in an additional double-tap event.
 */
typedef enum {
    hmi3d_tap_north = 0x0080,
    hmi3d_tap_east = 0x0100,
    hmi3d_tap_south = 0x0020,
    hmi3d_tap_west = 0x0040,
    hmi3d_tap_center = 0x0200,
    hmi3d_double_tap_north = 0x1000,
    hmi3d_double_tap_east = 0x2000,
    hmi3d_double_tap_south = 0x0400,
    hmi3d_double_tap_west = 0x0800,
    hmi3d_double_tap_center = 0x4000,
    hmi3d_single_tap_mask = 0x03E0,
    hmi3d_double_tap_mask = 0x7C00,
    hmi3d_tap_mask = 0x7FE0
} hmi3d_tap_flags_t;

/* Enumeration: hmi3d_calib_reason_t
 *
 * Bitmask of possible reasons for calibrations
 *
 * hmi3d_forced_calib     - Enforced with <hmi3d_force_calibration>
 * hmi3d_startup_calib    - Triggered during startup
 * hmi3d_gesture_calib    - Triggered after gesture
 * hmi3d_negative_calib   - Triggered to compensate negative values
 * hmi3d_idle_calib       - Triggered to compensate background noise
 * hmi3d_invalidity_calib - Triggered as signals exceeded their valid range
 * hmi3d_dsp_forced_calib - Internally triggered calibration
 */
typedef enum {
    hmi3d_forced_calib = 0x0002,
    hmi3d_startup_calib = 0x0004,
    hmi3d_gesture_calib = 0x0008,
    hmi3d_negative_calib = 0x0010,
    hmi3d_idle_calib = 0x0020,
    hmi3d_invalidity_calib = 0x0040,
    hmi3d_dsp_forced_calib = 0x0080,
} hmi3d_calib_reason_t;

/* Structure: hmi3d_signal_t
 *
 * Contains data for all channels of one type of signal.
 *
 * channel - The indvidual channels
 *
 * The channels are typically in the order
 * (south, west, north, east, center).
 *
 * See also:
 *    <hmi3d_get_cic>, <hmi3d_get_sd>
 */
typedef struct {
    float channel[5];
} hmi3d_signal_t;

/* Structure: hmi3d_position_t
 *
 * Contains position data that could be accessed with <hmi3d_get_position>.
 *
 * x - Ranges from 0 (west) to 65535 (east)
 * y - Ranges from 0 (south) to 65535 (north)
 * z - Ranges from 0 (touching) to 65535 (hand away)
 */
typedef struct {
    int x, y, z;
} hmi3d_position_t;

/* Structure: hmi3d_gesture_t
 *
 * Contains information about gestures
 *
 * gesture    - The type of the gesture (See <hmi3d_gesture_t>)
 * flags      - Flags for the gesture (See <hmi3d_gesture_flags_t>)
 * last_event - Count of samples since the last detected gesture
 *
 * gesture will be set only once. It will be reset to <hmi3d_gest_none> by the
 * next call to <hmi3d_retrieve_data> if no subsequent gesture occured.
 *
 * Gesture data could be accessed with <hmi3d_get_gesture>.
 *
 * Note:
 *    last_event is based on the TimeStamp field of SENSOR_DATA_OUTPUT message.
 *    To be accurate the device isn't allowed to sleep and has to send at least
 *    one such message per second.
 *
 * Note:
 *    internally last_event is set to the count of samples between start-up
 *    and last gesture.
 */
typedef struct {
    hmi3d_gestures_t gesture;
    hmi3d_gesture_flags_t flags;
    int last_event;
} hmi3d_gesture_t;

/* Struct: hmi3d_touch_t
 *
 * Contains information about touch events.
 *
 * flags                  - Flags for the touch (See <hmi3d_touch_flags_t>)
 * last_event             - Count of samples since last touch event
 * tap_flags              - Flags for tap events
 * last_tap_event         - Count of samples since last tap event
 * last_touch_event_start - Count of samples since last touch event started
 *
 * Note:
 *    The *last_event fields are based on the TimeStamp field of
 *    SENSOR_DATA_OUTPUT message.
 *    To be accurate the device isn't allowed to sleep and has to send at least
 *    one such message per second.
 *
 * Note:
 *    internally the *last_event fiels are set to the count of samples between
 *    start-up and last gesture.
 */
typedef struct {
    hmi3d_touch_flags_t touch_flags;
    int last_touch_event;
    int last_touch_event_start;
    hmi3d_tap_flags_t tap_flags;
    int last_tap_event;
} hmi3d_touch_t;

/* Struct: hmi3d_air_wheel_t
 *
 * Contains information about AirWheel events.
 *
 * counter    - Counter which indicates how far the AirWheel rotation has
 *              progressed. Incrementing values indicate a clockwise rotation.
 *              Decrementing values indicate counter clockwise rotation.
 *              An increment of 32 approximates one full rotation.
 * active     - Boolean value indicating whether AirWheel is currently detected
 * last_event - Count of samples since last change of active
 *
 * Note:
 *    last_event is based on the TimeStamp field of SENSOR_DATA_OUTPUT message.
 *    To be accurate the device isn't allowed to sleep and has to send at least
 *    one such message per second.
 *
 * Note:
 *    internally last_event is set to the count of samples between start-up
 *    and last gesture.
 */
typedef struct {
    int counter;
    int active;
    int last_event;
} hmi3d_air_wheel_t;

/* Structure: hmi3d_calib_t
 *
 * Contains information about calibrations.
 *
 * reason     - Reason of the calibration as OR-combination of
 *              <hmi3d_calib_reason_t>.
 * last_event - Count of samples since last calibration
 *
 * reason will be set only once. It will be reset to 0 by subsequent calls to
 * <hmi3d_retrieve_data> when no further calibration occurs.
 *
 * Calibration data could be accessed with <hmi3d_get_calibration>.
 * Calibrations are updated when <hmi3d_DataOutConfigMask_SDData> is set.
 *
 * See also:
 *    <hmi3d_set_auto_calibration>, <hmi3d_force_calibration>
 *
 * Note:
 *    last_event is based on the TimeStamp field of SENSOR_DATA_OUTPUT message.
 *    To be accurate the device isn't allowed to sleep and has to send at least
 *    one such message per second.
 *
 * Note:
 *    internally last_event is set to the count of samples between start-up
 *    and last gesture.
 */
typedef struct {
    hmi3d_calib_reason_t reason;
    int last_event;
} hmi3d_calib_t;

/* Structure: hmi3d_freq_t
 *
 * Contains information about the working frequency.
 *
 * frequency    - The currently active frequency in kHz
 * freq_changed - Boolean value set to 1 when last frequency changed during
 *                last call to <hmi3d_retrieve_data>
 * last_event   - Count of samples since last calibration
 *
 * Frequency data could be accessed with <hmi3d_get_frequency>.
 * Frequency is always updated when any data is retrieved.
 *
 * See also:
 *    <hmi3d_select_frequencies>
 *
 * Note:
 *    last_event is based on the TimeStamp field of SENSOR_DATA_OUTPUT message.
 *    To be accurate the device isn't allowed to sleep and has to send at least
 *    one such message per second.
 *
 * Note:
 *    internally last_event is set to the count of samples between start-up
 *    and last gesture.
 */
typedef struct {
    int frequency;
    int freq_changed;
    int last_event;
} hmi3d_freq_t;

/* Structure: hmi3d_noise_power_t
 *
 * Contains current noise value
 *
 * value - The value of the noise power
 * valid - Not zero when value is valid or zero otherwise
 *
 * Noise power data is accessed with <hmi3d_get_noise_power>.
 * Or directly via hmi->results.noise_power when using the
 * static API.
 */
typedef struct {
    float value;
    int valid;
} hmi3d_noise_power_t;

#endif

#ifndef HMI3D_NO_DATA_RETRIEVAL

/* Function: hmi3d_retrieve_data
 *
 * Retrieves the availabe data output from the device and updates the result
 * buffer.
 *
 * skipped - If a pointer to an integer is provided it will be updated with the
 *           count of skipped data-sets.
 *
 * Returns 0 on success or a negative <hmi_error_t> code if no new data is
 * available or the communication is broken.
 *
 * It is possible that multiple updates were received during calls to different
 * functions. In this case the buffer will contain an accumulation of received
 * data after this call and skipped will be set to the amount of skipped
 * data-sets.
 *
 * The fetched data could be accessed with the hmi3d_get_* functions.
 *
 * Note:
 *    <hmi3d_retrieve_data> may process only fragments of the incoming
 *    data. It is therefore usefull to call it repeatedly until no more data are
 *    available to keep it in sync with the device.
 *
 * See also:
 *    <hmi3d_get_cic>, <hmi3d_get_sd>,
 *    <hmi3d_get_position>, <hmi3d_get_gesture>, <hmi3d_get_calibration>
 */
HMI_API int CDECL hmi3d_retrieve_data(hmi_t *hmi, int *skipped);

#endif

/* ======== 3D Firmware Version ======== */

#ifndef HMI3D_NO_FW_VERSION

/* Function: hmi3d_query_fw_version
 *
 * Reads the library version information.
 *
 * version  - Pointer to a buffer receiving the version string
 * v_length - Maximal length of version in bytes including the terminating 0.
 *
 * Note:
 *    The complete firmware-version-string has a maximum size of 120
 *    characters.
 *
 * Returns 0 on success or a negative value when the request failed.
 *
 * After a failure or no response within the timeout the request gets resend
 * up to three times.
 */
HMI_API int CDECL hmi3d_query_fw_version(hmi_t *hmi,
                                            char *version,
                                            int v_length);

#endif

/* ======== 3D Real time control (RTC) ======== */

#ifndef HMI3D_NO_DATA_RETRIEVAL

/* Function: hmi3d_set_output_enable_mask
 *
 * Changes the data output of the 3D part of the device.
 *
 * flags   - Which data to be streamed as a combination of
 *           <hmi3d_DataOutConfigMask_t>-values
 * locked  - Which of the streamed data have to be always included in the
 *           message even when they have no valid data.
 * mask    - The mask of which data should get configured by this call
 *
 * Returns 0 on success or a negative value when the request failed.
 *
 * After a failure or no response within the timeout the request gets resend
 * up to three times.
 *
 * See also:
 *    <hmi3d_get_output_enable_mask>
 */
HMI_API int CDECL hmi3d_set_output_enable_mask(hmi_t *hmi,
                                               hmi3d_DataOutConfigMask_t flags,
                                               hmi3d_DataOutConfigMask_t locked,
                                               hmi3d_DataOutConfigMask_t mask);

/* Function: hmi3d_get_output_enable_mask
 *
 * Reads back the mask of data output set in the 3D part of the device.
 *
 * flags   - Pointer to variable receiving which data is to be streamed as a
 *           combination of <hmi3d_DataOutConfigMask_t>-values. May be 0.
 * locked  - Pointer to variable receiving which of the streamed data has to
 *           be included always even though they are not valid. May be 0.
 *
 * Returns 0 on success or a negative value when the request failed.
 *
 * If the request failed the content of flags and locked is not defined.
 *
 * After a failure or no response within the timeout the request gets resend
 * up to three times.
 *
 * See also:
 *    <hmi3d_set_output_enable_mask>
 */
HMI_API int CDECL hmi3d_get_output_enable_mask(hmi_t *hmi,
                                               hmi3d_DataOutConfigMask_t *flags,
                                               hmi3d_DataOutConfigMask_t *locked);

#endif

#ifndef HMI3D_NO_RTC

/* Enumeration: hmi3d_frequencies_t
 *
 * Bitmask for working frequencies that could be enabled.
 * The actual frequencies depend on the library version in place and are listed
 * in the libraries manual.
 * The currently selected frequency may be fetched with <hmi3d_get_frequency>.
 *
 * hmi3d_freq1    - Frequency 1 enabled (highest)
 * hmi3d_freq2    - Frequency 2 enabled
 * hmi3d_freq3    - Frequency 3 enabled
 * hmi3d_freq4    - Frequency 4 enabled
 * hmi3d_freq5    - Frequency 5 enabled (lowest)
 * hmi3d_all_freq - All frequencies enabled
 *
 * See also:
 *    <hmi3d_select_frequencies>
 */
typedef enum {
    hmi3d_freq1 = 0x01,
    hmi3d_freq2 = 0x02,
    hmi3d_freq3 = 0x04,
    hmi3d_freq4 = 0x08,
    hmi3d_freq5 = 0x10,
    hmi3d_all_freq = 0x1F
} hmi3d_frequencies_t;

/* Function: hmi3d_set_auto_calibration
 *
 * Enables automatic calibration if enabled is set to a value other than 0.
 *
 * enabled - Boolean value whether calibration is enabled
 *
 * Returns 0 if the change was succesfull or a negative value on failure.
 *
 * After a failure or no response within the timeout the request gets resend
 * up to three times.
 *
 * See also:
 *    <hmi3d_get_auto_calibration>, <hmi3d_force_calibration>,
 *    <hmi3d_get_calibration>
 */
HMI_API int CDECL hmi3d_set_auto_calibration(hmi_t *hmi,
                                             int enabled);

/* Function: hmi3d_get_auto_calibration
 *
 * Reads back whether automatic calibration is enabled.
 *
 * enabled - Pointer to variable that receives the state of auto-calibration
 *
 * Returns 0 on success or a negative value when the request failed.
 *
 * If the request failed the content of enabled will not be changed.
 *
 * After a failure or no response within the timeout the request gets resend
 * up to three times.
 *
 * See also:
 *    <hmi3d_set_auto_calibration>, <hmi3d_force_calibration>
 */
HMI_API int CDECL hmi3d_get_auto_calibration(hmi_t *hmi,
                                             int *enabled);

/* Function: hmi3d_force_calibration
 *
 * Enforces a immediate calibration of the device.
 *
 * Returns 0 if the device acknoledged the calibration-request.
 *
 * After a failure or no response within the timeout the request gets resend
 * up to three times.
 *
 * See also:
 *    <hmi3d_set_auto_calibration>
 */
HMI_API int CDECL hmi3d_force_calibration(hmi_t *hmi);

/* Function: hmi3d_select_frequencies
 *
 * Selects which working frequencies are allowed.
 *
 * frequencies - Allowed frequencies as a combination of <hmi3d_frequencies_t>
 *
 * Selects the allowed working frequencies for the device. The allowed
 * frequencies are passed in the argument frequencies as a bitwise-or
 * combination of <hmi3d_frequencies_t>-values.
 *
 * Returns 0 if the change was successfull.
 *
 * After a failure or no response within the timeout the request gets resend
 * up to three times.
 */
HMI_API int CDECL hmi3d_select_frequencies(hmi_t *hmi,
                                           hmi3d_frequencies_t frequencies);

/* Function: hmi3d_set_approach_detection
 *
 * Enables approach detection for power saving.
 *
 * enabled - Boolean value whether approach detection is enabled
 *
 * Returns 0 if the change was successfull.
 *
 * After a failure or no response within the timeout the request gets resend
 * up to three times.
 *
 * See also:
 *    <hmi3d_get_approach_detection>
 */
HMI_API int CDECL hmi3d_set_approach_detection(hmi_t *hmi, int enabled);

/* Function: hmi3d_get_approach_detection
 *
 * Reads back whether approach detection is enabled.
 *
 * enabled - Pointer to a variable receiving the state of the approach detection
 *
 * Returns 0 on success or a negative value when the request failed.
 *
 * If the request failed the content of enabled will not be changed.
 *
 * After a failure or no response within the timeout the request gets resend
 * up to three times.
 *
 * See also:
 *    <hmi3d_set_approach_detection>
 */
HMI_API int CDECL hmi3d_get_approach_detection(hmi_t *hmi, int *enabled);

/* Function: hmi3d_set_enabled_gestures
 *
 * Makes certain gestures available for gesture recognition.
 *
 * gestures - Bitmask of gestures that are to be enabled
 *
 * Returns 0 if the enabled gestures was successfully changed.
 *
 * The individual gestures are represented as 1 shifted by the value of the
 * according <hmi3d_gestures_t>-constant. E.g. Horizontal flicks are
 * represented by (1 << hmi3d_flick_w2e) | (1 << hmi3d_flick_e2w)
 *
 * After a failure or no response within the timeout the request gest resend
 * up to three times.
 *
 * See also:
 *    <hmi3d_get_enabled_gestures>, <hmi3d_get_gesture>
 */
HMI_API int CDECL hmi3d_set_enabled_gestures(hmi_t *hmi, int gestures);

/* Function: hmi3d_get_enabled_gestures
 *
 * Reads back which gestures are currently available for gesture detection.
 *
 * gestures - Pointer to a variable receiving the bitmask of enabled gestures
 *
 * Returns 0 on success or a negative value when the request failed.
 *
 * If the request failed the content of gestures will not be changed.
 *
 * After a failure or no response within the timeout the request gets resend
 * up to three times.
 *
 * See also:
 *    <hmi3d_set_enabled_gestures>
 */
HMI_API int CDECL hmi3d_get_enabled_gestures(hmi_t *hmi, int *gestures);

/* Function: hmi3d_set_touch_detection
 *
 * En-/Disables touch detection.
 *
 * enabled - Boolean value whether touch detection is enabled
 *
 * Returns 0 if the change was successfull.
 *
 * After a failure or no response within the timeout the request gets resend
 * up to three times.
 *
 * See also:
 *    <hmi3d_get_touch_detection>, <hmi3d_get_touch>
 */
HMI_API int CDECL hmi3d_set_touch_detection(hmi_t *hmi, int enabled);

/* Function: hmi3d_get_touch_detection
 *
 * Reads back whether touch detection is enabled
 *
 * enabled - Pointer to a variable receiving the state of touch-detection
 *
 * Returns 0 on success or a negative value when the request failed.
 *
 * If the request failed the content of enabled will not be changed.
 *
 * After a failure or no response within the timeout the request gets resend
 * up to three times.
 *
 * See also:
 *    <hmi3d_set_touch_detection>
 */
HMI_API int CDECL hmi3d_get_touch_detection(hmi_t *hmi, int *enabled);

/* Function: hmi3d_set_air_wheel_enabled
 *
 * En-/Disables AirWheel.
 *
 * enabled - Boolean value whether AirWheel is enabled
 *
 * Returns 0 if the change was successfull.
 *
 * If AirWheel is enabled no circle-gestures could be detected.
 *
 * After a failure or no response within the timeout the request gets resend
 * up to three times.
 *
 * See also:
 *    <hmi3d_get_air_wheel_enabled>, <hmi3d_get_air_wheel>
 */
HMI_API int CDECL hmi3d_set_air_wheel_enabled(hmi_t *hmi, int enabled);

/* Function: hmi3d_get_air_wheel_enabled
 *
 * Reads back whether the AirWheel-feature is enabled
 *
 * enabled - Pointer to a variable receiving the state of AirWheel-feature
 *
 * Returns 0 on success or a negative value when the request failed.
 *
 * If the request failed the content of enabled will not be changed.
 *
 * After a failure or no response within the timeout the request gets resend
 * up to three times.
 *
 * See also:
 *    <hmi3d_set_air_wheel_enabled>
 */
HMI_API int CDECL hmi3d_get_air_wheel_enabled(hmi_t *hmi, int *enabled);

/* Function: hmi3d_make_persistent
 *
 * Stores current value for all parameters of a category to the flash.
 *
 * category - Parameter category which should be stored
 *
 * Returns 0 if parameters where successfully stored or
 * the negative error-code on failure.
 *
 * After a failure or no response within the timeout the request gets
 * resend up to three times.
 *
 * See also:
 *    <hmi3d_set_param>, <hmi3d_param_category>
 */
HMI_API int CDECL hmi3d_make_persistent(hmi_t *hmi,
                                        hmi3d_param_category category);

#endif

/* ======== 3D Firmware Update Functions ======== */

#ifndef HMI3D_NO_UPDATE

/* Enumeration: hmi3d_UpdateFunction_t
 *
 * The mode of the update-session
 *
 * hmi3d_UpdateFunction_ProgramFlash - Records will be written to the flash
 * hmi3d_UpdateFunction_VerifyOnly   - Code will only be verified but not
 *                                     changed in flash.
 * hmi3d_UpdateFunction_Restart      - Only used internally for final restart.
 *                                     Don't provide this to the hmi3d_update_*
 *                                     functions
 *
 * If a session gets started by calling <hmi3d_update_begin> with
 * <hmi3d_UpdateFunction_VerifyOnly> no other value is allowed in subsequent
 * <hmi3d_update_write> calls.
 *
 * If a session gets started with <hmi3d_UpdateFunction_ProgramFlash> it has to
 * be finished with <hmi3d_UpdateFunction_ProgramFlash> too in order to have a
 * valid firmware.
 *
 * See also:
 *    <3D Firmware Update>, <hmi3d_update_image>, <hmi3d_update_begin>,
 *    <hmi3d_update_write>
 */
typedef enum {
    hmi3d_UpdateFunction_ProgramFlash = 0,
    hmi3d_UpdateFunction_VerifyOnly = 1,
    hmi3d_UpdateFunction_Restart = 3
} hmi3d_UpdateFunction_t;

/* Structure: hmi3d_update_record_t
 *
 * Contains one record of a firmware library image.
 *
 * address - The address of the record aligned to 128 bytes
 * length  - The length of the data packed in the record.
 *           This is NOT the size of the record itself.
 * record  - The 128-byte long content of the record.
 *
 * See also:
 *    <3D Firmware Update>, <hmi3d_update_image>, <hmi3d_update_image_t>
 */
typedef struct {
    unsigned short address;
    unsigned char length;
    unsigned char data[128];
} hmi3d_update_record_t;

/* Structure: hmi3d_update_image_t
 *
 * Contains the data of a library image as required by <hmi3d_update_image>
 *
 * record_count - The actual count of records
 * iv           - The initialization vector.
 * fw_version   - The version containing an ASCII-string
 * data         - The first of record_count records
 *
 * For instructions on how to obtain an hmi3d_update_image_t instance see
 * <3D Firmware Update>.
 *
 * See also:
 *    <3D Firmware Update>, <hmi3d_update_image>
 */
typedef struct {
    int record_count;
    unsigned char iv[14];
    unsigned char fw_version[120];
    hmi3d_update_record_t data[1];
} hmi3d_update_image_t;

/* Function: hmi3d_update_begin
 *
 * Starts a flash session to update the contained firmware-library
 *
 * session_id - A random session-id. Can be any value except 0.
 * iv         - The 14-byte long initialization vector.
 *              It is provided as part of the library-image.
 * mode       - The mode for this session
 *
 * Returns 0 on success or a negative value when the request failed.
 *
 * This function resets the device and starts an session for updating the
 * contained firmware-library. The session has to be finished with
 * hmi3d_update_end or another reset to cancel it.
 *
 * Note:
 *    If the mode is <hmi3d_UpdateFunction_VerifyOnly> subsequent calls to
 *    <hmi3d_update_write> have to be called with the same mode.
 *    <hmi3d_update_end> implicitely uses the same mode to finish the session.
 *
 * See also:
 *    <3D Firmware Update>, <hmi3d_update_image>, <hmi3d_update_write>,
 *    <hmi3d_update_end>
 */
HMI_API int CDECL hmi3d_update_begin(hmi_t *hmi,
                                     unsigned int session_id,
                                     void *iv,
                                     hmi3d_UpdateFunction_t mode);

/* Function: hmi3d_update_write
 *
 * Writes one record during a flash session
 *
 * address - The address of the record, This is part of the library-image
 * length  - The size of the data packed in the record.
 *           This is NOT the size of the record
 * record  - The 128-byte long data of the record
 * mode    - The update-mode for this record.
 *
 * Returns 0 on success or a negative value when the request failed.
 *
 * The address is aligned to 128 bytes and the record is always 128-bytes long
 * independant of size-argument. If the provided record is less than 128-bytes
 * the rest of the record has to be filled with zeros.
 *
 * After a failure or no response within the timeout the request gets resend
 * up to three times.
 *
 * Note:
 *    If <hmi3d_update_begin> was called with <hmi3d_UpdateFunction_VerifyOnly>
 *    this mode has to be used in the <hmi3d_update_write> too.
 *
 * See also:
 *    <3D Firmware Update>, <hmi3d_update_image>, <hmi3d_update_begin>,
 *    <hmi3d_update_end>
 */
HMI_API int CDECL hmi3d_update_write(hmi_t *hmi,
                                     unsigned short address,
                                     unsigned char length,
                                     unsigned char *record,
                                     hmi3d_UpdateFunction_t mode);

/* Function: hmi3d_update_end
 *
 * Finishes the flash process and ends the flash session.
 *
 * version - The 120-byte long library-version containing an ASCII-string
 *           It is provided as part of the the firmware image
 *
 * Returns 0 on success or a negative value when the request failed.
 *
 * This function ends the flash session by finally writing the version-string
 * and resetting the device. After this function returns the device
 * needs some time until it finishes the reset and subsequent start-up
 * procedure. <hmi3d_wait_for_version_info> waits until the
 * wanted start-up procedure sends the firmware version message which
 * marks that the library loader is ready (first call) or the library itself
 * is ready (second call).
 *
 * After a failure or no response within the timeout the request gets resend
 * up to three times.
 *
 * Note:
 *    The final reset triggered by <hmi3d_update_end> may be delayed by
 *    the firmware for a few seconds. This could be shortened by a manual
 *    <hmi3d_reset> about 200 milliseconds after <hmi3d_update_end> succeeded.
 *
 * Important:
 *    After flashing of loader-updates the chip requires some time
 *    (about 20-25 seconds) to complete the update before it is ready for
 *    flashing the firmware. <hmi3d_update_wait_loader_done> does the
 *    required check.
 *
 * See also:
 *    <3D Firmware Update>, <hmi3d_update_image>, <hmi3d_update_begin>,
 *    <hmi3d_update_write>
 */
HMI_API int CDECL hmi3d_update_end(hmi_t *hmi,
                                   unsigned char *version);

/* Function: hmi3d_update_image
 *
 * Does the whole process of writing the image to device including the resets.
 *
 * session_id - A random session-id. Can be any value except 0.
 * image      - Ptr to a <hmi3d_update_image_t>-structure containing the image.
 *
 * Returns 0 on success or a negative value when the request failed.
 *
 * Runs the whole process of flashing the firmware library on the provided
 * image. This includeds the initial and final restarts of the device.
 * If the image is not available in a <hmi3d_update_image_t>-structure the
 * functions <hmi3d_update_begin>, <hmi3d_update_write> and <hmi3d_update_end>
 * could be used directly.
 *
 * After a failure or no response within the timeout the request gets resend
 * up to three times.
 *
 * Important:
 *    After flashing of loader-updates the chip requires some time to
 *    complete the update before it is ready for flashing the firmware.
 *    <hmi3d_update_wait_loader_done> does the required check.
 *
 * See also:
 *    <3D Firmware Update>, <hmi3d_update_begin>, <hmi3d_update_write>,
 *    <hmi3d_update_end>
 */
HMI_API int CDECL hmi3d_update_image(hmi_t *hmi,
                                     unsigned int session_id,
                                     hmi3d_update_image_t *image,
                                     hmi3d_UpdateFunction_t mode);

/* Function: hmi3d_update_wait_loader_done
 *
 * Waits until loader update is finished.
 *
 * Return 0 on success or a negative value when request failed.
 *
 * Call this function after a loader got uploaded to the chip to ensure
 * that it was completely processed and stored in flash before starting
 * the firmware-update.
 *
 * Note:
 *    An bootloader update can take up to 20 seconds.
 *
 * Important:
 *    Only call this function after a loader-upload.
 *    Otherwise it will fail and return an error.
 *
 * See also:
 *    <3D Firmware Update>, <hmi3d_update_image>, <hmi3d_update_end>
 */
HMI_API int CDECL hmi3d_update_wait_loader_done(hmi_t *hmi);

/* Function: hmi3d_wait_for_version_info
 *
 * After a reset waits until version info message was received.
 *
 * Return 0 on success or the negative <hmi_error_t> code
 * when request failed.
 *
 * This function waits after a reset until the library loader
 * version info was received. At this point the loader is ready
 * to receive instructions.
 *
 * Note:
 *    <hmi3d_update_begin> already contains the reset and waiting
 *    until the loader is ready. There is no need to do an extra
 *    call to <hmi3d_reset> and <hmi3d_wait_for_version_info>
 *
 * A second subsequent call will wait until the library version
 * information was received. At that point the GestIC library is
 * completely operational.
 *
 * Note:
 *    This function is only intended to be used for waiting until
 *    the different start-up states are finished.
 *
 * See also:
 *    <3D Firmware Update>, <hmi3d_reset>
 */
HMI_API int CDECL hmi3d_wait_for_version_info(hmi_t *hmi);
#endif

/* ======== 2D Low Level Communication ======== */

/* Enum: hmi2d_msg_receive_id_t
 *
 * IDs for incoming messages from the 2D subsystem as processed by this library
 *
 * hmi2d_msg_r_update       - Response to <hmi2d_msg_t_update>
 * hmi2d_msg_r_parameter    - One parameter entry as response to
 *                            <hmi2d_msg_t_get_param>
 * hmi2d_msg_r_mutual_raw_0 - First row of raw sensor data
 * hmi2d_msg_r_mutual_raw_f - Last row of raw sensor data
 * hmi2d_msg_r_mutual_cal_0 - First row of calibrated sensor data
 * hmi2d_msg_r_mutual_cal_f - Last row of calibrated sensor data
 * hmi2d_msg_r_ack          - Acknowledge of command
 * hmi2d_msg_r_mouse_btns   - State of the emulated mouse-buttons
 * hmi2d_msg_r_gesture      - Gestures as used for keyboard-emulation
 * hmi2d_msg_r_finger_pos   - List of finger positions
 * hmi2d_msg_r_self_raw     - Raw self scan sensor data
 * hmi2d_msg_r_self_measure - Calibrated self scan sensor data
 * hmi2d_msg_r_3d_subsystem - Message that was forwarded from 3D subsystem
 * hmi2d_msg_r_fw_version   - FW Version as response to <hmi2d_msg_t_fw_version>
 */
typedef enum {
    hmi2d_msg_r_update = 0x55,
    hmi2d_msg_r_parameter = 0xCF,
    hmi2d_msg_r_mutual_raw_0 = 0xD0,
    hmi2d_msg_r_mutual_raw_f = 0xDF,
    hmi2d_msg_r_mutual_cal_0 = 0xE0,
    hmi2d_msg_r_mutual_cal_f = 0xEF,
    hmi2d_msg_r_ack = 0xF0,
    hmi2d_msg_r_mouse_btns = 0xF6,
    hmi2d_msg_r_gesture = 0xF7,
    hmi2d_msg_r_finger_pos = 0xF8,
    hmi2d_msg_r_self_raw = 0xFA,
    hmi2d_msg_r_self_measure = 0xFD,
    hmi2d_msg_r_3d_subsystem = 0xFE,
    hmi2d_msg_r_fw_version = 0xFF
} hmi2d_msg_receive_id_t;

/* Enum: hmi2d_msg_transmit_id_t
 *
 * IDs for outgoing messages for the 2D-subsystem as sent by this library
 *
 * hmi2d_msg_t_update       - <2D Firmware Update> related messages.
 *                            The exact meaning depends on the data of the
 *                            message.
 * hmi2d_msg_t_set_param    - Command to change the value of a parameter.
 *                            The related API function is <hmi2d_set_param>.
 * hmi2d_msg_t_get_param    - Query for the current value of a parameter.
 *                            The related API function is <hmi2d_get_param>.
 * hmi2d_msg_t_3d_reset     - Command to do a hard reset of the
 *                            3D subsystem. It gets sent by <hmi3d_reset> when
 *                            compiled for communication with the 3DTouchPad.
 * hmi2d_msg_t_3d_subsystem - Command that are addressed to the 3D subsystem.
 *                            Gets sent by <hmi3d_message_write>
 *                            when compiled for use with the 3DTouchPad.
 * hmi2d_msg_t_fw_version   - Query for the version of the 2D firmware.
 *                            The related API function is
 *                            <hmi2d_query_fw_version>.
 */
typedef enum {
    hmi2d_msg_t_update = 0x55,
    hmi2d_msg_t_set_param = 0xE0,
    hmi2d_msg_t_get_param = 0xE1,
    hmi2d_msg_t_3d_reset = 0xFC,
    hmi2d_msg_t_3d_subsystem = 0xFD,
    hmi2d_msg_t_fw_version = 0xFF
} hmi2d_msg_transmit_id_t;

/* Enum: hmi2d_parameter_id_t
 *
 * IDs of parameters for the 2D-subsystem that are supported by this library
 *
 * hmi2d_param_com_mask             - Mask for communicated messages as set by
 *                                    <hmi2d_set_com_mask>
 * hmi2d_param_active_mask          - Mask of active features as set by
 *                                    <hmi2d_set_active_mask>
 * hmi2d_param_operation_mode       - 2D/3D operation mode as set by
 *                                    <hmi2d_set_operation_mode>
 *
 * hmi2d_param_swipeBorderLeft      - Left border of of swipe region.
 *                                    Swipes have to start outside the region.
 *                                    The whole board has range [0,0]-[512,384]
 * hmi2d_param_swipeBorderRight     - Right border of swipe region.
 * hmi2d_param_swipeBorderTop       - Top border of swipe region.
 * hmi2d_param_swipeBorderBottom    - Bottom border of swipe region.
 * hmi2d_param_clickBorderLeft      - Left border of click region.
 *                                    Clicks will only be detected inside.
 *                                    The whole board has range [0,0]-[512,384]
 * hmi2d_param_clickBorderRight     - Right border of swipe region.
 * hmi2d_param_clickBorderTop       - Top border of swipe region.
 * hmi2d_param_clickBorderBottom    - Bottom border of swipe region.
 * hmi2d_param_autoMovBorderHorz    - Horizontal border for automatic movement
 *                                    at edges (0 = no auto-mov region,
 *                                    512/2 = the border takes the whole board)
 * hmi2d_param_autoMovBorderVert    - Vertical border for automatic movement at
 *                                    edges (0 = no auto-mov region,
 *                                    512/2 = the border takes the whole board)
 * hmi2d_param_autoMovSpeedX        - Speed of horizontal automatic movement
 * hmi2d_param_autoMovSpeedY        - Speed of vertical automatic movement
 * hmi2d_param_autoMovDragSpeedX    - Speed of horizontal movement while dragging
 * hmi2d_param_autoMovDragSpeedY    - Speed of vertical movement while dragging
 * hmi2d_param_autoMovScrollSpeedX  - Speed of automatic horizontal scrolling
 * hmi2d_param_autoMovScrollSpeedY  - Speed of automatic Vertical scrolling
 * hmi2d_param_scrollSpeedX         - Horizontal scroll speed
 * hmi2d_param_scrollSpeedY         - Vertical scroll speed
 * hmi2d_param_scrollStartThreshold - Minimal movement to start scrolling
 * hmi2d_param_scrollInertiaX       - To what degree horizontal scrolling
 *                                    continues after fingers were raised
 * hmi2d_param_scrollInertiaY       - To what degree vertical scrolling
 *                                    continues after fingers were raised
 *
 * There are parameters for the key combinations that are sent to the host in
 * case of an detected gesture. Each of those key-combos consists of four
 * parameters:
 *
 * send - Defines under which conditions the key-combo should be emitted
 * key1 - The first key of the combo
 * key2 - The second key of the combo
 * key3 - The third and last key of the combo
 *
 * The sends parameter is the one of the key-combo parameters that is used by
 * <hmi2d_set_key_combo> to identify the parameters for the key-combo.
 * The send parameters are:
 *
 * hmi2d_param_outkeyFlickL_send   - Flick right to left key-combo
 * hmi2d_param_outkeyFlickR_send   - Flick left to right key-combo
 * hmi2d_param_outkeyFlickU_send   - Flick bottom to top key-combo
 * hmi2d_param_outkeyFlickD_send   - Flick top to bottom key-combo
 * hmi2d_param_outkeySwipe1FL_send - Swipe from right edge key-combo
 * hmi2d_param_outkeySwipe1FR_send - Swipe from left edge key-combo
 * hmi2d_param_outkeySwipe1FU_send - Swipe from bottom edge key-combo
 * hmi2d_param_outkeySwipe1FD_send - Swipe from top edge key-combo
 *
 * See also:
 *    <hmi2d_set_param>, <hmi2d_get_param>, <2D Real Time Control>
 */
typedef enum {
    hmi2d_param_com_mask = 0x0080,
    hmi2d_param_active_mask = 0x0081,
    hmi2d_param_operation_mode = 0x0082,

    hmi2d_param_swipeBorderLeft = 0x0502,
    hmi2d_param_swipeBorderRight = 0x0503,
    hmi2d_param_swipeBorderTop = 0x0504,
    hmi2d_param_swipeBorderBottom = 0x0505,
    hmi2d_param_clickBorderLeft = 0x0540,
    hmi2d_param_clickBorderRight = 0x0541,
    hmi2d_param_clickBorderTop = 0x0542,
    hmi2d_param_clickBorderBottom = 0x0543,
    hmi2d_param_autoMovBorderHorz = 0x0600,
    hmi2d_param_autoMovBorderVert = 0x0601,
    hmi2d_param_autoMovSpeedX = 0x0602,
    hmi2d_param_autoMovSpeedY = 0x0603,
    hmi2d_param_autoMovDragSpeedX = 0x0604,
    hmi2d_param_autoMovDragSpeedY = 0x0605,
    hmi2d_param_autoMovScrollSpeedX = 0x0606,
    hmi2d_param_autoMovScrollSpeedY = 0x0607,
    hmi2d_param_scrollSpeedX = 0x06A0,
    hmi2d_param_scrollSpeedY = 0x06A1,
    hmi2d_param_scrollStartThreshold = 0x6A2,
    hmi2d_param_scrollInertiaX = 0x6A4,
    hmi2d_param_inertStartThresX = 0x6A5,
    hmi2d_param_inertStopThresX = 0x6A6,
    hmi2d_param_scrollInertiaY = 0x6A8,
    hmi2d_param_inertStartThresY = 0x6A9,
    hmi2d_param_inertStopThresY = 0x6AA,

    hmi2d_param_outkeyFlickL_send = 0x1000,
    hmi2d_param_outkeyFlickL_key1 = 0x1001,
    hmi2d_param_outkeyFlickL_key2 = 0x1002,
    hmi2d_param_outkeyFlickL_key3 = 0x1003,
    hmi2d_param_outkeyFlickR_send = 0x1010,
    hmi2d_param_outkeyFlickR_key1 = 0x1011,
    hmi2d_param_outkeyFlickR_key2 = 0x1012,
    hmi2d_param_outkeyFlickR_key3 = 0x1013,
    hmi2d_param_outkeyFlickU_send = 0x1020,
    hmi2d_param_outkeyFlickU_key1 = 0x1021,
    hmi2d_param_outkeyFlickU_key2 = 0x1022,
    hmi2d_param_outkeyFlickU_key3 = 0x1023,
    hmi2d_param_outkeyFlickD_send = 0x1030,
    hmi2d_param_outkeyFlickD_key1 = 0x1031,
    hmi2d_param_outkeyFlickD_key2 = 0x1032,
    hmi2d_param_outkeyFlickD_key3 = 0x1033,
    hmi2d_param_outkeySwipe1FL_send = 0x1100,
    hmi2d_param_outkeySwipe1FL_key1 = 0x1101,
    hmi2d_param_outkeySwipe1FL_key2 = 0x1102,
    hmi2d_param_outkeySwipe1FL_key3 = 0x1103,
    hmi2d_param_outkeySwipe1FR_send = 0x1110,
    hmi2d_param_outkeySwipe1FR_key1 = 0x1111,
    hmi2d_param_outkeySwipe1FR_key2 = 0x1112,
    hmi2d_param_outkeySwipe1FR_key3 = 0x1113,
    hmi2d_param_outkeySwipe1FU_send = 0x1120,
    hmi2d_param_outkeySwipe1FU_key1 = 0x1121,
    hmi2d_param_outkeySwipe1FU_key2 = 0x1122,
    hmi2d_param_outkeySwipe1FU_key3 = 0x1123,
    hmi2d_param_outkeySwipe1FD_send = 0x1130,
    hmi2d_param_outkeySwipe1FD_key1 = 0x1131,
    hmi2d_param_outkeySwipe1FD_key2 = 0x1132,
    hmi2d_param_outkeySwipe1FD_key3 = 0x1133,
    hmi2d_param_outkeyApproach_send = 0x1140,
    hmi2d_param_outkeyApproach_key1 = 0x1141,
    hmi2d_param_outkeyApproach_key2 = 0x1142,
    hmi2d_param_outkeyApproach_key3 = 0x1143,
} hmi2d_parameter_id_t;

/* Function: hmi2d_set_param
 *
 * This functions sets a parameter in the 2D subsystem.
 *
 * param - The id of the parameter to be set
 * arg0  - The first arg of the parameter.
 *         This is normaly the parameter value itself
 * arg1  - The second arg of the parameter. If not stated otherwise this is
 *         the mask of the bits of the parameter that have to be changed.
 *         To change all flags or to set a scalar parameter set this to ~0.
 *
 * Returns HMI_NO_ERROR on success or a negative error code.
 *
 * See also:
 *    <hmi2d_get_param>, <hmi2d_parameter_id_t>, <2D Real Time Control>
 */
HMI_API int CDECL hmi2d_set_param(hmi_t *hmi, hmi2d_parameter_id_t param,
                                  int arg0, int arg1);

/* Function: hmi2d_get_param
 *
 * This function fetches a parameter from the 2D subsystem
 *
 * param - The id of the parameter to fetch
 * arg0  - Pointer to an integer receiving the value of the parameter
 *
 * Returns HMI_NO_ERROR on success or a negative error code.
 *
 * See also:
 *    <hmi2d_set_param>, <hmi2d_parameter_id_t>
 */
HMI_API int CDECL hmi2d_get_param(hmi_t *hmi, hmi2d_parameter_id_t param,
                                  unsigned int *arg0);


/* ======== 2D Data Retrieval ======== */

#ifndef HMI2D_NO_DATA_RETRIEVAL

/* Enum: hmi2d_mouse_button_t
 *
 * Possible mouse buttons that could be emitted by the mouse emulation of the
 * 2D subsystem.
 *
 * hmi2d_mouse_left_btn   - Left mouse button clicked
 * hmi2d_mouse_right_btn  - Middle mouse button clicked
 * hmi2d_mouse_middle_btn - Right mouse button clicked
 */
typedef enum {
    hmi2d_mouse_left_btn   = 0x01,
    hmi2d_mouse_right_btn  = 0x02,
    hmi2d_mouse_middle_btn = 0x04
} hmi2d_mouse_button_t;

/* Enum: hmi2d_gesture_id_t
 *
 * hmi2d_no_gesture         - No gesture detected
 * hmi2d_edge_swipe1f_left  - Leftward swipe with one finger from the right edge
 * hmi2d_edge_swipe1f_right - Rightward swipe with one finger from the left edge
 * hmi2d_edge_swipe1f_up    - Upward swipe with one finger from the bottom edge
 * hmi2d_edge_swipe1f_down  - Downward swipe with one finger from the top edge
 * hmi2d_flick_left         - Leftward flick (from 3D subsystem)
 * hmi2d_flick_right        - Rightward flick (from 3D subsystem)
 * hmi2d_flick_up           - Upward flick (from 3D subsystem)
 * hmi2d_flick_down         - Downward flick (from 3D subsystem)
 * hmi2d_approach_detected  - Approach detected (from 3D subsystem)
 * hmi2d_pinch_narrowing    - Narrowing pinch
 * hmi2d_pinch_widening     - Widening pinch
 *
 */
typedef enum {
    hmi2d_no_gesture = 0x00,
    hmi2d_edge_swipe1f_left  = 0x10,
    hmi2d_edge_swipe1f_right = 0x11,
    hmi2d_edge_swipe1f_up    = 0x12,
    hmi2d_edge_swipe1f_down  = 0x13,
    hmi2d_flick_left         = 0x20,
    hmi2d_flick_right        = 0x21,
    hmi2d_flick_up           = 0x22,
    hmi2d_flick_down         = 0x23,
    hmi2d_approach_detected  = 0x30,
    hmi2d_pinch_narrowing    = 0x40,
    hmi2d_pinch_widening     = 0x41
} hmi2d_gesture_id_t;

/* Typedef: hmi2d_row_t
 *
 * One row of sensor data.
 */
typedef int hmi2d_row_t[16];

/* Typedef: hmi2d_block_t
 *
 * One block of sensor data.
 * Rows that were not update on the last <hmi2d_retrieve_data> call are set to
 * zero.
 */
typedef hmi2d_row_t hmi2d_block_t[16];

/* Struct: hmi2d_finger_pos_t
 *
 * Structure containig touch related data for one finger.
 *
 * x         - X-position of finger
 * y         - Y-position of finger
 * finger_id - An ID of the finger as assigned by the signal processing
 */
typedef struct {
    int x;
    int y;
    int finger_id;
} hmi2d_finger_pos_t;

/* Struct: hmi2d_finger_pos_list_t
 *
 * Structure containing touch related data for all fingers.
 *
 * count - The number of active entries in this list.
 * entry - The individual <hmi2d_finger_pos_t> -entries
 */
typedef struct {
    int count;
    hmi2d_finger_pos_t entry[10];
} hmi2d_finger_pos_list_t;

// NOTE This is for debugging purpose
/* Struct: hmi2d_mouse_t
 *
 * Structure containing data about the state of the mouse emulation.
 *
 * button_state  - Currently pressed buttons as <hmi2d_mouse_button_t> -flags
 * press_event   - Emulated mouse buttons pressed since the last update
 * release_event - Emulated mouse buttons release since the last update
 */
typedef struct {
    hmi2d_mouse_button_t button_state;
    hmi2d_mouse_button_t press_event;
    hmi2d_mouse_button_t release_event;
} hmi2d_mouse_t;

/* Struct: hmi2d_gesture_t
 *
 * Structure containing gesture data as used by the keyboard emulation.
 *
 * gesture - The id of the detected gesture (See <hmi2d_gesture_id_t>)
 */
typedef struct {
    hmi2d_gesture_id_t gesture;
} hmi2d_gesture_t;

/* Function: hmi2d_retrieve_data
 *
 * Retrieves incoming data from the 2D subsystem and updates result buffers.
 *
 * Returns HMI_NO_ERROR on success, HMI_NO_DATA if no data was available
 * for retrieval or a negative error code in case of an error.
 *
 * See also:
 *    <2D Data Retrieval>
 */
HMI_API int CDECL hmi2d_retrieve_data(hmi_t *hmi);

#endif

/* ======== 2D Real Time Control (RTC) ======== */

#ifndef HMI2D_NO_RTC

/* Enum: hmi2d_operation_mode_t
 *
 * hmi2d_mixed_mode  - Both 2D and 3D processing is enabled.
 * hmi2d_2d_mode     - Only 2D subsystem is enabled.
 * hmi2d_3d_mode     - Only 3D subsystem and 3D-based event generation
 *                     are enabled.
 * hmi2d_bridge_mode - 2D system is reduced to pure forwarding of messages.
 *                     No event generation is done.
 *
 * The default on device-startup is <hmi2d_mixed_mode>.
 *
 * Note:
 *    Communication with the 3D subsystem requires
 *    <hmi2d_com_3d_messages> to be set. Without it <hmi2d_3d_mode> is
 *    limited to gestures as accessible with <hmi2d_get_gesture> and
 *    <hmi2d_bridge_mode> will result in no data output at all.
 *
 * See also:
 *    <hmi2d_set_operation_mode>, <hmi2d_set_com_mask>,
 *    <hmi2d_com_3d_messages>
 */
typedef enum {
    hmi2d_mixed_mode = 0,
    hmi2d_2d_mode = 1,
    hmi2d_3d_mode = 2,
    hmi2d_bridge_mode = 3
} hmi2d_operation_mode_t;

/* Function: hmi2d_set_operation_mode
 *
 * Sets operation mode for 2D subsystem.
 *
 * mode - The new operation mode
 *
 * Note:
 *    Communication with the 3D subsystem requires
 *    <hmi2d_com_3d_messages> to be set. Without it <hmi2d_3d_mode> is
 *    limited to gestures as accessible with <hmi2d_get_gesture> and
 *    <hmi2d_bridge_mode> will result in no data output at all.
 *
 * See also:
 *    <hmi2d_get_operation_mode>, <hmi2d_operation_mode_t>, <hmi2d_set_param>,
 *    <hmi2d_set_com_mask>, <hmi2d_com_3d_messages>
 */
HMI_API int CDECL hmi2d_set_operation_mode(hmi_t *hmi,
                                           hmi2d_operation_mode_t mode);

/* Function: hmi2d_get_operation_mode
 *
 * Gets the currently active operation mode of the 2D subsystem.
 *
 * mode - Pointer <hmi2d_operation_mode_t> variable that receives the mode
 *
 * See also:
 *    <hmi2d_set_operation_mode>, <hmi2d_operation_mode_t>, <hmi2d_get_param>
 */
HMI_API int CDECL hmi2d_get_operation_mode(hmi_t *hmi,
                                           hmi2d_operation_mode_t *mode);

/* Enum: hmi2d_active_mask_t
 *
 * hmi2d_active_gesture_recognition     - Gesture processing is enabled
 * hmi2d_active_lock_scroll_dir         - Disables simultaneous scrolling in
 *                                        horizontal and vertical direction
 * hmi2d_active_full_mutual             - Full mutal scanning is enabled
 * hmi2d_active_mouse_buttons           - Emiting of emulated mouse buttons
 * hmi2d_active_mouse_movement          - Emiting of emulated mouse movement
 * hmi2d_active_flick_events            - Emiting of keys based on flick events
 * hmi2d_active_swipe_events            - Emiting of keys based on swipe events
 * hmi2d_active_air_wheel_scroll_events - Emiting of scroll events based on
 *                                        AirWheel
 * hmi2d_active_scroll_events           - Emiting of two-finger scroll events
 * hmi2d_active_pinch_events            - Emiting of pinch events
 * hmi2d_active_default                 - Default state after device startup
 * hmi2d_active_all                     - Mask of all defined flags
 *
 * The gesture based HID events (<hmi2d_active_mouse_buttons>,
 * <hmi2d_active_mouse_movement>, <hmi2d_active_flick_events>,
 * <hmi2d_active_swipe_events>, <hmi2d_active_air_wheel_scroll_events>,
 * <hmi2d_active_scroll_events>) also require <hmi2d_active_gesture_recognition>
 * in order to detect the events for emiting.
 *
 * See also:
 *    <hmi2d_set_active_mask>
 */
typedef enum {
    hmi2d_active_gesture_recognition = 0x0001,
    hmi2d_active_lock_scroll_dir = 0x0002,
    hmi2d_active_full_mutual = 0x0004,
    hmi2d_active_mouse_buttons = 0x0008,
    hmi2d_active_mouse_movement = 0x0010,
    hmi2d_active_flick_events = 0x0020,
    hmi2d_active_swipe_events = 0x0040,
    hmi2d_active_air_wheel_scroll_events = 0x0080,
    hmi2d_active_scroll_events = 0x0100,
    hmi2d_active_pinch_events = 0x0200,
    hmi2d_active_default = 0x03F9,
    hmi2d_active_all = 0x03FF
} hmi2d_active_mask_t;

/* Function: hmi2d_set_active_mask
 *
 * Sets/changes mask of activated processings.
 *
 * active - Bitmask of active events
 * mask   - Bitmask of which flags should be changed ~0 changes all flags
 *
 * Returns HMI_NO_ERROR on success or a negative error code.
 *
 * See also:
 *    <hmi2d_get_active_mask>, <hmi2d_active_mask_t>, <hmi2d_set_param>
 */
HMI_API int CDECL hmi2d_set_active_mask(hmi_t *hmi, hmi2d_active_mask_t active,
                                        hmi2d_active_mask_t mask);

/* Function: hmi2d_get_active_mask
 *
 * Gets the mask of activated processings.
 *
 * active - Pointer to <hmi2d_active_mask_t> variable that receives the mask
 *
 * Return HMI_NO_ERROR on success or a negative error code.
 *
 * See also:
 *    <hmi2d_set_active_mask>, <hmi2d_active_mask_t>, <hmi2d_get_param>
 */
HMI_API int CDECL hmi2d_get_active_mask(hmi_t *hmi,
                                        hmi2d_active_mask_t *active);

/* Function: hmi2d_set_full_mutual
 *
 * Enables/disables full-mutual mode.
 *
 * enabled - Boolean whether full-mutual is enabled
 *
 * Returns HMI_NO_ERROR on success or a negative error code.
 *
 * See also:
 *    <hmi2d_get_full_mutual>, <hmi2d_set_param>, <hmi2d_set_com_mask>
 */
HMI_API int CDECL hmi2d_set_full_mutual(hmi_t *hmi, int enabled);

/* Function: hmi2d_get_full_mutual
 *
 * Gets current state of mutual mode.
 *
 * enabled - Pointer to boolean variable receiving the state
 *
 * See also:
 *    <hmi2d_set_full_mutual>. <hmi2d_get_param>, <hmi2d_get_com_mask>
 */
HMI_API int CDECL hmi2d_get_full_mutual(hmi_t *hmi, int *enabled);

/* Enum: hmi2d_com_mask_t
 *
 * hmi2d_com_cal_self_measure   - Output of calibrated self-scan data
 *                                accessed via <hmi2d_get_self_cal>
 * hmi2d_com_cal_mutual_measure - Output of calibrated mutual-scan data
 *                                accessed via <hmi2d_get_mutual_cal>
 * hmi2d_com_raw_self_measure   - Output of raw self-scan data
 *                                accessed via <hmi2d_get_self_raw>
 * hmi2d_com_raw_mutual_measure - Output of raw mutual-scan data
 *                                accessed via <hmi2d_get_mutual_raw>
 * hmi2d_com_finger_positions   - Output of finger positions
 *                                accessed via <hmi2d_get_finger_positions>
 * hmi2d_com_gestures           - Output of gesture events
 *                                accessed via <hmi2d_get_gesture>
 *                                Requires <hmi2d_active_gesture_recognition>
 * hmi2d_com_mouse_buttons      - Output of mouse button state
 *                                accessed via <hmi2d_get_mouse>
 *                                Requires <hmi2d_active_gesture_recognition>
 * hmi2d_com_3d_messages        - Required for 3D communication
 * hmi2d_com_default            - Default value after device-startup
 * hmi2d_com_all                - Mask of all defined flags
 *
 * <hmi2d_com_3d_messages> is required for all 3D communication in
 * either direction. Without it the direct communication between the host and
 * the 3D subsystem is disabled. However events like flicks might still be
 * accessed via <hmi2d_get_gesture> when <hmi2d_com_gestures> is set.
 *
 * <hmi2d_com_gestures> and <hmi2d_com_mouse_buttons> require that gesture
 * recognition is activated (see <hmi2d_active_gesture_recognition> ) in order
 * to produce any events.
 *
 * See also:
 *    <hmi2d_set_com_mask>, <hmi2d_get_com_mask>
 */
typedef enum {
    hmi2d_com_cal_self_measure = 0x0001,
    hmi2d_com_cal_mutual_measure = 0x0002,
    hmi2d_com_raw_self_measure = 0x0004,
    hmi2d_com_raw_mutual_measure = 0x0008,
    hmi2d_com_finger_positions = 0x0800,
    hmi2d_com_gestures = 0x1000,
    hmi2d_com_mouse_buttons = 0x2000,
    hmi2d_com_3d_messages = 0x8000,
    hmi2d_com_default = 0x0000,
    hmi2d_com_all = 0xB80F,
} hmi2d_com_mask_t;

/* Function: hmi2d_set_com_mask
 *
 * Sets mask of what messages should be sent to the host.
 *
 * enabled - Bitmask containing flags for enabled messages
 * mask    - Bitmask of which flags should be affected by this change
 *
 * Returns HMI_NO_ERROR on success or a negative error code.
 *
 * Note:
 *    Always set <hmi2d_com_3d_messages> when direct communication
 *    with the 3D subsystem is required. Otherwise communication via functions
 *    with the hmi3d_-prefix will fail.
 *
 * See also:
 *    <hmi2d_get_com_mask>, <hmi2d_com_mask_t>, <hmi2d_set_param>,
 *    <hmi2d_set_full_mutual>, <hmi2d_set_operation_mode>
 */
HMI_API int CDECL hmi2d_set_com_mask(hmi_t *hmi,
                                     hmi2d_com_mask_t enabled,
                                     hmi2d_com_mask_t mask);

/* Function: hmi2d_get_com_mask
 *
 * Gets current mask of what messages should be sent to the host.
 *
 * enabled - Pointer to <hmi2d_com_mask_t> variable receiving the maks.
 *
 * Returns HMI_NO_ERROR on success or a negative error code.
 *
 * See also:
 *    <hmi2d_set_com_mask>, <hmi2d_com_mask_t>, <hmi2d_get_param>,
 *    <hmi2d_get_full_mutual>
 */
HMI_API int CDECL hmi2d_get_com_mask(hmi_t *hmi, hmi2d_com_mask_t *enabled);

/* Enum: hmi2d_event_cond_t
 *
 * Possible conditions for event generation
 *
 * hmi2d_event_disabled  - Event will never cause key-emulation or similar
 * hmi2d_event_on_single - Single event will cause key-emulation
 * hmi2d_event_on_double - Double event will cause key-emulation
 *
 * Note:
 *    <hmi2d_event_on_double> only has an effect for flicks. For other events
 *    will be processed as if <hmi2d_event_on_single> was selected.
 *
 * See also:
 *    <hmi2d_key_combo_t>, <hmi2d_set_key_combo>
 */
typedef enum {
    hmi2d_event_disabled = 0,
    hmi2d_event_on_single = 1,
    hmi2d_event_on_double = 2,
} hmi2d_event_cond_t;

/* Struct: hmi2d_key_combo_t
 *
 * cond - Condition for sending the associated key-combo
 * key  - The keys of the key combo
 *
 * The values of the keys are the same as specified in the USB HID Usage Tables.
 *
 * See also:
 *    <hmi2d_set_key_combo>, <hmi2d_event_cond_t>
 */
typedef struct {
    hmi2d_event_cond_t cond;
    int key[3];
} hmi2d_key_combo_t;

/* Function: hmi2d_set_key_combo
 *
 * Sets the parameters related to one key-combo-mapping
 *
 * param - The id of the send-parameter of the key-combo
 * combo - The combo itself
 *
 * Returns HMI_NO_ERROR on success or a negative error code.
 *
 * See also:
 *    <hmi2d_get_key_combo>, <hmi2d_key_combo_t>, <hmi2d_parameter_id_t>,
 *    <hmi2d_set_param>
 */
HMI_API int CDECL hmi2d_set_key_combo(hmi_t *hmi, int param,
                                      hmi2d_key_combo_t *combo);

/* Function: hmi2d_get_key_combo
 *
 * Gets the parameters related to one key-combo-mapping
 *
 * param - The id of the send-parameter of the key-combo
 * combo - Pointer to <hmi2d_key_combo_t> instance receiving the key-combo
 *
 * Return HMI_NO_ERROR on success or a negative error code.
 *
 * See aslo:
 *    <hmi2d_set_key_combo>, <hmi2d_key_combo_t>, <hmi2d_parameter_id_t>,
 *    <hmi2d_get_param>
 */
HMI_API int CDECL hmi2d_get_key_combo(hmi_t *hmi, int param,
                                      hmi2d_key_combo_t *combo);

#endif

/* ======== 2D Firmware Version ======== */

/* Struct: hmi2d_version_info_t
 *
 * Structure for the 2D firmware version.
 *
 * See also:
 *    <hmi2d_query_fw_version>
 */
typedef struct {
    unsigned short svn_revision;
    unsigned char wc_mixed;
    unsigned char wc_modified;
    unsigned short build_year;
    unsigned char build_month;
    unsigned char build_day;
    unsigned char build_hour;
    unsigned char build_minute;
    unsigned char build_second;
    char buildBy[9];
    char infoString[76];
    unsigned int checksum;
    unsigned int fw_app_id;
    unsigned char fw_version[4];
} hmi2d_version_info_t;

/* Function: hmi2d_query_fw_version
 *
 * Queries the version of the currently running 2D firmware and stores it
 * to the provided version argument.
 *
 * version - Pointer to a memory location where to store the retrieved
 *           firmware version
 *
 * Returns HMI_NO_ERROR on success or a negative error code.
 *
 * See also:
 *    <hmi2d_version_info_t>, <2D Firmware Update>
 */
HMI_API int CDECL hmi2d_query_fw_version(hmi_t *hmi,
                                         hmi2d_version_info_t *version);


/* ======== 2D Firmware Update Functions ======== */

#ifndef HMI2D_NO_UPDATE

/* Enum: hmi2d_bootloader_error_t
 *
 */
typedef enum {
    hmi2d_bl_no_error = 0,
    hmi2d_bl_flash_erase_error = 6,
    hmi2d_bl_checksum_mismatch_error = 7,
    hmi2d_bl_block_count_error = 8,
    hmi2d_bl_flash_write_error = 9,
    hmi2d_bl_out_of_ragne_address_error = 10,
    hmi2d_bl_bootloader_locked_error = 11,
    hmi2d_bl_unrecognized_command_error = 12,
    hmi2d_bl_number_of_bytes_error = 13,
    hmi2d_bl_hex_file_error = 14
} hmi2d_bootloader_error_t;

/* Enum: HMI_PROG
 *
 * A few values that are used in the <2D Firmware Update>.
 *
 * HMI2D_PROG_MEM_START    - Address of first byte of firmware
 * HMI2D_PROG_MEM_END      - Address of first byte after firmware
 * HMI2D_PROG_MEM_SIZE     - Size of firmware in bytes
 * HMI2D_PROG_PAGE_SIZE    - Size of memory pages (for erasing)
 * HMI2D_PROG_BLOCK_SIZE   - Size of single update block
 * HMI2D_PROG_BLOCK_COUNT  - Count of blocks in the firmware image
 * HMI2D_PROG_ERASED_VALUE - Value that is used in erased memory
 */
enum {
    HMI2D_PROG_MEM_START = 0x1d003000,
    HMI2D_PROG_MEM_END = 0x1d010000,
    HMI2D_PROG_MEM_SIZE = HMI2D_PROG_MEM_END - HMI2D_PROG_MEM_START,
    HMI2D_PROG_PAGE_SIZE = 0x1000,
    HMI2D_PROG_BLOCK_SIZE = 0x200,
    HMI2D_PROG_BLOCK_COUNT = HMI2D_PROG_MEM_SIZE / HMI2D_PROG_BLOCK_SIZE,
    HMI2D_PROG_ERASED_VALUE = 0xFFFFFFFF
};

/* Typedef: hmi2d_update_block_t
 *
 * Typedef for one single block of the firmware image.
 */
typedef char hmi2d_update_block_t[HMI2D_PROG_BLOCK_SIZE];

/* Typedef: hmi2d_update_image_t
 *
 * Typedef for the complete image.
 */
typedef hmi2d_update_block_t hmi2d_update_image_t[HMI2D_PROG_BLOCK_COUNT];

/* Function: hmi2d_update_enter_bootloader
 *
 * Enters the bootloader-mode for 2D firmware update.
 *
 * Returns HMI_NO_ERROR on success or a negative error code.
 *
 * This instruction has to be sent to 2D firmware in order to access
 * 2D bootloader device. It is not needed if already connected to the
 * 2D bootloader device.
 *
 * After a successfull call to hmi2d_update_enter_bootloader the connection
 * to the 2D device has to be closed and a connection to the
 * 2D bootloader device has to be opened. It might take some time until this
 * device is accessible as Windows has to install drivers first.
 *
 * The <Update2D Application> demonstrates this.
 *
 * See also:
 *    <Update2D Application>, <2D Firmware Update>,
 *    <hmi2d_update_exit_bootloader>
 */
HMI_API int CDECL hmi2d_update_enter_bootloader(hmi_t *hmi);

/* Function: hmi2d_update_unlock
 *
 * Unlocks the memory for the 2D firmware update.
 *
 * Returns HMI_NO_ERROR on success or a negative error code.
 *
 * This instruction has to be sent to the 2D bootloader device. If connected
 * to normal 2D device <hmi2d_update_enter_bootloader> has to be called first
 * with a subsequent reconnect to the 2D bootloader device.
 *
 * The next step affter hmi2d_update_unlock is a call to
 * <hmi2d_update_erase_memory>.
 *
 * See also:
 *    <Update2D Application>, <2D Firmware Update>
 */
HMI_API int CDECL hmi2d_update_unlock(hmi_t *hmi);

/* Function: hmi2d_update_erase_memory
 *
 * Erases the memory of the 2D firmware.
 *
 * Returns HMI_NO_ERROR on success or a negative error code.
 *
 * <hmi2d_update_erase_memory> has to be called while connected to the
 * 2D bootloader device after the memory was unlocked.
 * It is followed by programming the new image with either
 * <hmi2d_update_flash_memory> for the whole image or by individual calls
 * to <hmi2d_update_flash_block>.
 *
 * See also:
 *    <Update2D Application>, <2D Firmware Update>, <hmi2d_update_flash_memory>
 */
HMI_API int CDECL hmi2d_update_erase_memory(hmi_t *hmi);

/* Function: hmi2d_update_flash_block
 *
 * Writes one block of the firmware image.
 *
 * addr  - Address of the first byte of the block
 * block - The block to be written
 *
 * Returns HMI_NO_ERROR on success or a negative error code.
 *
 * An alternative to calling hmi2d_update_flash_block for each block of the
 * firmware image individually would be to call <hmi2d_update_flash_memory>
 * once for the whole image.
 *
 * The <Update2D Application> demonstrates the use of hmi2d_update_flash_block
 * in order to give detailed feedback on the progress of the update.
 *
 * Once the whole 2D firmware image was written the bootloader mode could
 * be left by calling <hmi2d_update_exit_bootloader>.
 *
 * See also:
 *    <Update2D Application>, <2D Firmware Update>, <hmi2d_update_flash_memory>
 */
HMI_API int CDECL hmi2d_update_flash_block(hmi_t *hmi,
                                           int addr,
                                           hmi2d_update_block_t *block);

/* Function: hmi2d_update_flash_memory
 *
 * Writes the whole 2D firmware image at once.
 *
 * image - The image to be written.
 *
 * Returns HMI_NO_ERROR on success or a negative error code.
 *
 * hmi2d_update_flash_memory has to be called after erasing the memory while
 * connected to the 2D bootloader device.
 *
 * If detailed feedback on the progress is needed <hmi2d_update_flash_block>
 * could be called for each block of the image instead of
 * <hmi2d_update_flash_memory>.
 *
 * Once the whole 2D firmware image was written the bootloader mode could
 * be left by calling <hmi2d_update_exit_bootloader>.
 *
 * See also:
 *    <Update2D Application>, <2D Firmware Update>, <hmi2d_update_flash_block>
 */
HMI_API int CDECL hmi2d_update_flash_memory(hmi_t *hmi,
                                            hmi2d_update_image_t *image);

/* Function: hmi2d_update_exit_bootloader
 *
 * Exits the 2D bootloader mode.
 *
 * Returns HMI_NO_ERROR on success or a negative error code.
 *
 * This instruction has to be sent to the 2D bootloader firmware in order to
 * start the normal operation of the 2D firmware. If a 2D firmware update was
 * already started it has to be completed in order to successfully leave the
 * 2D bootloader mode.
 *
 * After a successfull call to hmi2d_update_exit_bootloader the connection
 * to the 2D bootloader device has to be closed and a connection to the
 * normal 2D device has to be opened in order to be used.
 * It might take some time until this device is accessible as Windows might
 * have to install drivers first.
 *
 * The <Update2D Application> demonstrates how to do a complete 2D firmware
 * update.
 *
 * See also:
 *    <Update2D Application>, <2D Firmware Update>,
 *    <hmi2d_update_enter_bootloader>
 */
HMI_API int CDECL hmi2d_update_exit_bootloader(hmi_t *hmi);

#endif

/* Include the dynamic API that has to be used when accessing
 * the HMI-library via dynamic linking.
 * This is required because the exact configuration of the dynamic
 * library and therefore the structure of hmi_t is unknown.
 */
#if defined(HMI_API_DYNAMIC) || defined(HMI_HAS_DYNAMIC)
#include "hmi_dynamic.h"
#endif

/* Include the static part of the API that is not accessible
 * to applications linking dynamically against the HMI-library.
 */
#if defined(HMI_API_EXPORT) || !defined(HMI_API_DYNAMIC)
#include "hmi_static.h"
#endif

#ifdef __cplusplus
}
#endif

#endif /* HMI_API_H */
