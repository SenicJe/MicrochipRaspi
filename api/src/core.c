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
#include "impl.h"

#if HMI_IO == HMI_IO_CDC_SERIAL
/* Forward declaration of hmi3d_init_msg_extract */
void hmi3d_init_msg_extract(hmi_t *hmi);
#endif

void hmi_initialize(hmi_t *hmi) {
    HMI_ASSERT(hmi);

    HMI_MEMSET(hmi, 0, sizeof(hmi_t));

#if HMI_IO == HMI_IO_HID_3DTOUCHPAD
    hmi->io.vendor_id = 0x4d8;
    hmi->io.product_id = 0x09d3;
    hmi->io.report_id = 4;
    hmi->io.iface = 1;
#elif HMI_IO == HMI_IO_CDC_SERIAL
    /* Initialize extraction of messages from serial stream */
    hmi3d_init_msg_extract(hmi);
#endif

#if defined(HMI_SYNC_INTERRUPT) || defined(HMI_SYNC_THREADING)
    HMI_SYNC_INIT(hmi->io_sync);
#endif
}

void hmi_cleanup(hmi_t *hmi) {
    HMI_ASSERT(hmi);

#if defined(HMI_SYNC_INTERRUPT) || defined(HMI_SYNC_THREADING)
    HMI_SYNC_RELEASE(hmi->io_sync);
#endif
}

#ifndef HMI_NO_LOGGING

int hmi_log(hmi_t *hmi, const char *fmt, ...)
{
    va_list vlist;
    int result = 0;

    if(hmi->logging.logger) {
        va_start(vlist, fmt);
        result = hmi->logging.logger(hmi->logging.opaque, fmt, vlist);
        va_end(vlist);
    }

    return result;
}

int hmi_set_logger(hmi_t *hmi,
                   hmi_logger_t logger,
                   void *opaque)
{
    hmi->logging.logger = logger;
    hmi->logging.opaque = opaque;
    return HMI_NO_ERROR;
}

#endif
