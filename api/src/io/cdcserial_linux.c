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
#include "io.h"

#if (HMI_IO == HMI_IO_CDC_SERIAL) && defined(__linux__)

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>

#define DEVICE "/dev/ttyACM0"

int hmi_open(hmi_t *hmi) {
    int error = HMI_NO_ERROR;
    int device;

    device = open(DEVICE, O_RDWR | O_NOCTTY | O_NDELAY);
    if(device == -1)
        error = HMI_IO_OPEN_ERROR;

    /* Set terminal parameters */
    if(!error) {
        struct termios io;
        int iFlags;

        memset(&io, 0, sizeof(io));

        if(tcgetattr(device, &io))
            error = HMI_IO_CTL_ERROR;

        if(!error) {
            io.c_iflag = 0;
            io.c_oflag = 0;
            io.c_lflag = 0;
            io.c_cflag = CS8;
            io.c_cc[VMIN] = 1;
            io.c_cc[VTIME] = 0;
            if(tcsetattr(device, TCSANOW, &io))
                error = HMI_IO_CTL_ERROR;
        }

        /* Turn on DTR */
        if(!error) {
            iFlags = TIOCM_DTR;
            if(ioctl(device, TIOCMBIS, &iFlags))
                error = HMI_IO_CTL_ERROR;
        }
    }

    if(error) {
        if(device != -1)
            close(device);
    } else {
        hmi->io.cdc_serial = (void*)device;
    }

    return error;
}

void hmi_close(hmi_t *hmi) {
    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    close((int)hmi->io.cdc_serial);
    hmi->io.cdc_serial = 0;
}

int hmi3d_reset(hmi_t *hmi) {
    const unsigned char reset_msg[] = {
        0xFE, 0xFF, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00
    };
    int device = -1;
    int error = HMI_NO_ERROR;

    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    device = (int)hmi->io.cdc_serial;
    if(write(device, reset_msg, sizeof(reset_msg)) != sizeof(reset_msg))
        error = HMI_IO_ERROR;

    return error;
}

int hmi3d_serial_read(hmi_t *hmi, void *buffer, int maxsize) {
    int result;
    int device = -1;

    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    device = (int)hmi->io.cdc_serial;
    result = read(device, buffer, maxsize);
    if(result <= 0)
        result = HMI_IO_ERROR;

    return result;
}

int hmi3d_serial_write(hmi_t *hmi, void *buffer, int size) {
    int result;
    int device = -1;

    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    device = (int)hmi->io.cdc_serial;
    result = write(device, buffer, size);
    if(result <= 0)
        result = HMI_IO_ERROR;

    return result;
}

#endif /* (HMI_IO == HMI_IO_CDC_SERIAL) && defined(__linux__) */
