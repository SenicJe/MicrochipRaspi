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
#ifndef HMI_2D_H
#define HMI_2D_H

#include "../impl.h"

/* Function: hmi2d_send_message
 *
 * Sends message and waits for the acknowledge by <hmi2d_msg_r_ack>.
 *
 * id      - Id of message to be sent
 * size    - Size of message to be sent
 * msg     - Pointer to content of message
 * timeout - Timeout after which the message should be resent or aborted
 *
 * If no <hmi2d_msg_r_ack> was received within the given timeout the message
 * will be resent up to 5 times before this function returns with an error code.
 *
 * This function returns HMI_NO_ERROR on success, HMI_NO_RESPONSE_ERROR when
 * the device hasn't acknowledged the message or a different error code on
 * errors during transmission.
 *
 * See also:
 *    <hmi2d_handle_ack>
 */
int hmi2d_send_message(hmi_t *hmi, int id, int size, void *msg, int timeout);

/* Function: hmi2d_handle_ack
 *
 * Handles <hmi2d_msg_r_ack> messages.
 *
 * msg - Reference to the received message
 *
 * See also:
 *    <hmi_message_receive>, <hmi2d_send_message>
 */
void hmi2d_handle_ack(hmi_t *hmi, const unsigned char *msg);

#ifndef HMI2D_NO_DATA_RETRIEVAL

/* Function: hmi2d_handle_data_row
 *
 * Handles the different incoming messages of raw or calibrated sensor data.
 *
 * row - Pointer to the buffer of the row that receives the data
 * msg - Reference to the received message
 *
 * The different messages are <hmi2d_msg_r_self_raw>,
 * <hmi2d_msg_r_self_measure>, the messages between <hmi2d_msg_r_mutual_cal_0>
 * and <hmi2d_msg_r_mutual_cal_f> and the messages between
 * <hmi2d_msg_r_mutual_raw_0> and <hmi2d_msg_r_mutual_raw_f>.
 *
 * The correct row to store the data is already identified by
 * <hmi_message_receive>.
 *
 * See also:
 *    <hmi_message_receive>, <hmi2d_get_self_raw>, <hmi2d_get_self_cal>,
 *    <hmi2d_get_mutual_raw>, <hmi2d_get_mutual_cal>
 */
void hmi2d_handle_data_row(hmi_t *hmi,
                           hmi2d_row_t *row,
                           const unsigned char *msg);

/* Function: hmi2d_handle_finger_pos
 *
 * Handles incoming <hmi2d_msg_r_finger_pos> messages.
 *
 * msg - Reference to the received message
 *
 * This messages contain a list of all currently detected fingers with their
 * position.
 *
 * See also:
 *    <hmi_message_receive>, <hmi2d_get_finger_positions>
 */
void hmi2d_handle_finger_pos(hmi_t *hmi, const unsigned char *msg);

/* Function: hmi2d_handle_mouse_btns
 *
 * Handles incoming <hmi2d_msg_r_mouse_btns> messages.
 *
 * msg - Reference to the received message
 *
 * This messages contain the state of the buttons of the mouse emulation.
 *
 * See also:
 *    <hmi_message_receive>, <hmi2d_get_mouse>
 */
void hmi2d_handle_mouse_btns(hmi_t *hmi, const unsigned char *msg);

/* Function: hmi2d_handle_gesture
 *
 * Handles incoming <hmi2d_msg_r_gesture> messages.
 *
 * msg - Reference to the received message
 *
 * 2D gesture messages contain information about the different gestures that
 * are used for keyboard-emulation including 3D flicks.
 *
 * See also:
 *    <hmi_message_receive>, <hmi2d_get_gesture>
 */
void hmi2d_handle_gesture(hmi_t *hmi, const unsigned char *msg);

#endif

/* Function: hmi2d_handle_parameter
 *
 * Handles incoming <hmi2d_msg_r_parameter> messages as response to
 * parameter requests sent by <hmi2d_get_param>
 *
 * msg - Reference to the received message
 *
 * See also:
 *    <hmi_message_receive>, <hmi2d_get_param>
 */
void hmi2d_handle_parameter(hmi_t *hmi, const unsigned char *msg);

#ifndef HMI2D_NO_UPDATE
/* Function: hmi2d_handle_update_response
 *
 * Handles incoming <hmi2d_msg_r_update> messages as response to messages
 * sent during <2D Firmware Update>
 *
 * msg - Reference to the received message
 *
 * See also:
 *    <hmi_message_receive>, <2D Firmware Update>
 */
void hmi2d_handle_update_response(hmi_t *hmi, const unsigned char *msg);
#endif

/* Function: hmi2d_handle_fw_version
 *
 * Handles incoming <hmi2d_msg_r_fw_version> messages.
 *
 * msg - Reference to the received message
 *
 * See also:
 *    <hmi_message_receive>, <hmi2d_query_fw_version>
 */
void hmi2d_handle_fw_version(hmi_t *hmi, const unsigned char *msg);

#endif /* HMI_2D_H */
