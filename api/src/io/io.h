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
#ifndef HMI_IO_H
#define HMI_IO_H

#include "../impl.h"

#if HMI_IO == HMI_IO_CDC_SERIAL

/* ======== Internal 3D Serial IO ======== */

/* Function: hmi3d_serial_read
 *
 * Tries to read at max maxsize data from the device to buffer.
 * Returns the amount of read data.
 */
int hmi3d_serial_read(hmi_t *hmi, void *buffer, int maxsize);

/* Function: hmi3d_serial_write
 *
 * Writes size bytes from buffer to the device.
 * Returns size on success or a negative error code on failure.
 */
int hmi3d_serial_write(hmi_t *hmi, void *buffer, int size);

/* ======== Internal Message Extraction for Serial IO ======== */

/* Function: hmi3d_init_msg_extract
 *
 * Initializes the state-machine of <message_extract>.
 *
 * This function is called by <hmi_initialize>.
 */
void hmi3d_init_msg_extract(hmi_t *hmi);

#endif /* HMI_IO == HMI_IO_CDC_SERIAL */

#endif /* HMI_IO_H */
