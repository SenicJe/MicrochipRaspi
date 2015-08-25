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
#ifndef HMI_STATIC_H
#define HMI_STATIC_H

/* This file is included by hmi_api.h and should not be included directly.
 *
 * As most IDEs don't take definitions from the file that is including the
 * header into consideration when parsing a header this file in turn
 * includes hmi_api.h to help the parser find the correct definitions.
 */
#ifndef HMI_API_H
#include "hmi_api.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ======== Logger ======== */

#ifndef HMI_NO_LOGGING

typedef struct {
    void *opaque;
    hmi_logger_t logger;
} hmi_logging_t;

#endif

/* ======== Parameter Request ======== */

typedef struct {
    int param;
    unsigned int *arg0;
    unsigned int *arg1;
} hmi_param_request_t;

typedef struct {
    char *version;
    int size;
    int received;
} hmi3d_version_request_t;

typedef struct {
    hmi2d_version_info_t *version;
    int received;
} hmi2d_version_request_t;

/* ======== Structure containing retrieved 3D data ======== */

#ifndef HMI3D_NO_DATA_RETRIEVAL

typedef struct {
    hmi3d_signal_t cic;
    hmi3d_signal_t sd;
    hmi3d_position_t pos;
    hmi3d_gesture_t gesture;
    hmi3d_calib_t calib;
    hmi3d_touch_t touch;
    hmi3d_air_wheel_t air_wheel;
    hmi3d_freq_t frequency;
    hmi3d_noise_power_t noise_power;
    int frame_counter;
} hmi3d_input_data_t;

#endif

#ifndef HMI2D_NO_DATA_RETRIEVAL

typedef struct {
    hmi2d_row_t self_raw;
    hmi2d_row_t self_cal;
    hmi2d_block_t mutual_raw;
    hmi2d_block_t mutual_cal;
    hmi2d_finger_pos_list_t fingers;
    hmi2d_mouse_t mouse;
    hmi2d_gesture_t gesture;
    int last_gesture;
    int msg_counter;
} hmi2d_input_data_t;

#endif

/* ======== Message Extraction State ======== */

#if HMI_IO == HMI_IO_CDC_SERIAL
/* Message size is limited to what could be expressed with one byte */
#define HMI3D_MAX_MESSAGE_SIZE 255

/* The maximum of data that is read from the device at once */
#define HMI3D_INPUT_CAPACITY 1024

typedef struct {
    int state;
    int buffer_cursor;
    int buffer_size;
    unsigned char buffer[HMI3D_INPUT_CAPACITY];
    unsigned char msg[HMI3D_MAX_MESSAGE_SIZE];
} hmi3d_msg_extract_t;
#endif

/* ======== Flashing Libraries ======== */

#ifndef HMI3D_NO_UPDATE

typedef struct {
    unsigned int session_id;
    hmi3d_UpdateFunction_t session_mode;
    int crc_intialized;
    unsigned int crc_table[256];
} hmi3d_update_t;

#endif

/* ======== The hmi_io_t - Structure ======== */

#if HMI_IO == HMI_IO_CDC_SERIAL

typedef struct {
#if HMI_IO == HMI_IO_CDC_SERIAL
    void *cdc_serial;
    hmi3d_msg_extract_t msg_extract;
#endif
} hmi_io_t;

#elif HMI_IO == HMI_IO_HID_3DTOUCHPAD

typedef struct hid_device_ hid_device;
typedef struct {
    hid_device *handle;
    unsigned char accum[256];
    unsigned char packet[64];
    int cursor;
    int offset;

    int vendor_id;
    int product_id;
    int report_id;
    int iface;
} hmi_io_t;

#endif

/* ======== IO Assertion helpers ======== */

/* Macro: HMI_CONNECTED
 *
 * Returns whether the device was connected via <hmi_open>.
 *
 * This macro is used for debugging purpose to detect usage of functions that
 * require a connection even though it wasn't established.
 */
#ifndef HMI_CONNECTED
#   if HMI_IO == HMI_IO_CDC_SERIAL
#       define HMI_CONNECTED(HMI) ((HMI)->io.cdc_serial)
#   elif HMI_IO == HMI_IO_HID_3DTOUCHPAD
#       define HMI_CONNECTED(HMI) ((HMI)->io.handle)
#   else
#       error "Macro HMI_CONNECTED not defined"
#   endif
#endif

/* ======== The hmi_t - Structure ======== */

struct hmi_struct {
    volatile int resp_msg_id;
    volatile int resp_error_code;
    hmi_param_request_t * volatile param_request;
    hmi3d_version_request_t * volatile version_request;

#ifndef HMI3D_NO_DATA_RETRIEVAL
    /* Buffer for the result as fetched via <hmi3d_retrieve_data> */
    hmi3d_input_data_t result;
    /* Buffer that contains the state after the last received data-frame */
    hmi3d_input_data_t internal;
    unsigned char last_time_stamp;
#if defined(HMI_SYNC_INTERRUPT) || defined(HMI_SYNC_THREADING)
    /* Pointer to data required for synchronization (e.g. a mutex) */
    void *io_sync;
#endif
#endif

#ifndef HMI2D_NO_UPDATE
    int bootloader_2d_error_code;
#endif
    volatile int resp2d_msg_id;
    volatile int resp2d_ack;
    hmi_param_request_t * volatile param2d_request;
    hmi2d_version_request_t * volatile version2d_request;

#ifndef HMI2D_NO_DATA_RETRIEVAL
    hmi2d_input_data_t result2d;
    hmi2d_input_data_t internal2d;
#endif

#ifndef HMI_NO_LOGGING
    hmi_logging_t logging;
#endif
    hmi_io_t io;
#ifndef HMI3D_NO_UPDATE
    hmi3d_update_t flash;
    unsigned char fw_valid;
#endif
};

/* ======== Internal Communication Implementation ======== */

/* Function: hmi_message_receive
 *
 * Called when incoming messages should be received.
 *
 * timeout - Optional pointer to an integer variable containing the timeout.
 *           This function sets timeout to the remaining time when returning.
 *
 * This function returns a negative error code if an error occured.
 *
 * If timeout is not NULL this function might return at any time with
 * <HMI_NO_ERROR> even if no message was processed but has to
 * return <HMI_NO_DATA> once when the timeout expired.
 *
 * If timeout is NULL this function *might* return <HMI_NO_DATA> if
 * *definitely* no message was received and will return <HMI_NO_ERROR>
 * otherwise.
 *
 * Typical implementations will try to read one message and forward it to
 * <hmi3d_message_handle>. However in case of asynchronous message handling
 * this function might do nothing except returning after a short sleep with
 * <HMI_NO_ERROR> when timeout has a nonzero value.
 *
 * Note:
 *    Custom IO implementations have to provide their own implementation
 *    of this function.
 *
 * See also:
 *    <hmi3d_message_write>, <hmi3d_message_handle>
 */
int hmi_message_receive(hmi_t *hmi, int *timeout);

/* Function: hmi3d_message_write
 *
 * Writes a message to the device.
 *
 * msg   - The message to write to the device
 * size  - The size of the message in bytes
 *
 * Returns HMI_NO_ERROR on success.
 *
 * Note:
 *    Custom IO implementations have to provide their own implementation of
 *    this function.
 *
 * See also:
 *    <hmi_message_receive>
 */
int hmi3d_message_write(hmi_t *hmi, void *msg, int size);

/* Function: hmi2d_message_write.
 *
 * Sends a message to the 2D subsystem.
 *
 * id   - ID of the message to be sent
 * size - Size of the message
 * msg  - The content of the message
 *
 * Return HMI_NO_ERROR on success or the IO implementation specific error code
 * on failure.
 *
 * See also:
 *    <hmi_message_receive>, <hmi2d_send_message>
 */
int hmi2d_message_write(hmi_t *hmi, int id, int size, unsigned char *msg);

/* ========  Message Processing ======== */

/* Function: hmi2d_message_handle
 *
 * Does the evaluation of received messages for the 2D subsystem.
 *
 * msg  - Reference to the received message.
 *        The size of the message is stored as the second byte.
 *
 * This function identifies the messages and calls the appropriate handlers.
 *
 * See also:
 *    <hmi_message_receive>, <hmi2d_handle_ack>, <hmi2d_handle_data_row>,
 *    <hmi2d_handle_finger_pos>, <hmi2d_handle_mouse_btns>,
 *    <hmi2d_handle_gesture>, <hmi2d_handle_parameter>,
 *    <hmi2d_handle_update_response>, <hmi2d_handle_fw_version>
 */
void hmi2d_message_handle(hmi_t *hmi, const unsigned char *msg);

/* Function: hmi3d_message_handle
 *
 * Does the evaluation of received messages for the 3D subsystem.
 *
 * msg  - Reference to the received message
 * size - The size of the received message
 *
 * This function is the heart of processing of incoming messages.
 * It decodes the messages and calls the appropriate handlers.
 *
 * See also:
 *    <hmi_message_receive>, <hmi3d_handle_system_status>,
 *    <hmi3d_handle_version_info>, <hmi3d_handle_data_output>,
 *    <hmi3d_handle_runtime_parameter>
 */
void hmi3d_message_handle(hmi_t *hmi, const void *msg, int size);

/* ======== Version information ======== */

#define HMI_API_VERSION_MAJOR 0
#define HMI_API_VERSION_MINOR 9
#define HMI_API_VERSION_REVISION 1
#define HMI_API_VERSION_STRING "0.9.1"

#ifdef __cplusplus
}
#endif

#endif /* HMI_STATIC_H */
