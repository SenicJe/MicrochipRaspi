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

#if HMI_IO == HMI_IO_HID_3DTOUCHPAD

#include "hidapi.h"

void hmi_hid_set_details(hmi_t *hmi,
                         int vendor_id,
                         int product_id,
                         int report_id,
                         int iface)
{
    hmi->io.vendor_id = vendor_id;
    hmi->io.product_id = product_id;
    hmi->io.report_id = report_id;
    hmi->io.iface = iface;
}

int hmi_open(hmi_t *hmi)
{
    struct hid_device_info *devs, *cur_dev;
    const char *path_to_open = 0;

    HMI_ASSERT(hmi);
    HMI_ASSERT(!HMI_CONNECTED(hmi));

    hid_init();

    devs = hid_enumerate(hmi->io.vendor_id, hmi->io.product_id);

    for(cur_dev = devs; cur_dev; cur_dev = cur_dev->next) {
        if(cur_dev->vendor_id != hmi->io.vendor_id ||
                cur_dev->product_id != hmi->io.product_id)
            continue;
#ifdef __linux__
        if(cur_dev->interface_number != hmi->io.iface)
            continue;
#endif
        path_to_open = cur_dev->path;
        break;
    }

    if(path_to_open)
        hmi->io.handle = hid_open_path(path_to_open);

    hid_free_enumeration(devs);

    if(!hmi->io.handle)
        return HMI_IO_OPEN_ERROR;

    hid_set_nonblocking(hmi->io.handle, 1);

    return HMI_NO_ERROR;
}

void hmi_close(hmi_t *hmi)
{
    HMI_ASSERT(hmi);
    HMI_ASSERT(HMI_CONNECTED(hmi));

    hid_close(hmi->io.handle);
    hmi->io.handle = 0;

    /* TODO Maybe call hid_exit(), but might be problematic when using multiple
     * devices in one application
     */
}

static int hmi_hid_fetch(hmi_t *hmi, int timeout)
{
    /* Tries to fetch another message from the incomming packets
     * The protocol defines the 64 byte packet with the following content:
     *
     * report_id | 1 byte  | The report id of the packet
     * length    | 1 byte  | The length of the data in the packet
     * data      | n bytes | A sequence of chunks with n = length
     * padding   | m bytes | Unused padding with m = 62 - length
     *
     * Data consits of a sequence of chunks with one chunk having
     * the following content:
     *
     * msg_id | 1 byte  | The ID of the message this chunk belongs to
     * flags  | 1 byte  | The flags of the chunk
     *        |         | Bit 0-5: length of data = k
     *        |         | Bit 6:   0 = Msg complete, 1 = Msg incomplete
     *        |         | Bit 7:   0 = Continues msg, 1 = Starts new message
     * data   | k bytes | The data of the chunk (see flags)
     */

    int result = HMI_NO_DATA;
    for(;;) {
        int id, flen, len, incomplete, continued;

        /* Read new packet if needed */
        if(!hmi->io.cursor) {
            int size = hid_read_timeout(hmi->io.handle, hmi->io.packet, 64, timeout);
            if(!size)
                break;
            /* Check report id */
            if(hmi->io.packet[0] != hmi->io.report_id)
                continue;
            /* Check packet length */
            if(hmi->io.packet[1] > 62) {
                HMI_BAD_DATA("hmi_hid_fetch",
                             "reported data size exceeds capacity of 62 bytes",
                             hmi->io.packet[1], 0);
                continue;
            }
            /* Set cursor to the begin of the data-block */
            hmi->io.cursor = 2;
        }

        /* Check whether packet has space for another chunk
         * NOTE The size of chunk header is same as the size of package header
         */
        if(hmi->io.cursor > hmi->io.packet[1]) {
            HMI_BAD_DATA("hmi_hid_fetch",
                         "chunk end doesn't match packet end",
                         hmi->io.cursor, 2 + hmi->io.packet[1]);
            hmi->io.cursor = 0;
            hmi->io.offset = 0;
            continue;
        }

        /* Interpret chunk header */
        id = hmi->io.packet[hmi->io.cursor];
        flen = hmi->io.packet[hmi->io.cursor+1];
        len = flen & 0x3F;
        incomplete = (flen & 0x40) != 0;
        continued = (flen & 0x80) != 0;

        /* Check whether chunk fits to the last processed chunk */
        if(hmi->io.offset) {
            /* There is already a message that the chunk should continue */
            if(!continued) {
                HMI_BAD_DATA("hmi_hid_fetch",
                             "Chunk starts new message while there is still "
                             "an incomplete message", 0, 0);
                hmi->io.cursor += 2 + len;
                hmi->io.offset = 0;
                continue;
            }
            /* Check whether chunk id fits message id */
            if(id != hmi->io.accum[0]) {
                HMI_BAD_DATA("hmi_hid_fetch",
                             "Chunk has an different id than the message "
                             "it belongs to",
                             hmi->io.accum[0], id);
                hmi->io.cursor += 2 + len;
                hmi->io.offset = 0;
                continue;
            }
        } else {
            /* The chunk should start a new message */
            if(continued) {
                HMI_BAD_DATA("hmi_hid_fetch",
                             "Chunk continues already completed message", 0, 0);
                hmi->io.cursor += 2 + len;
                hmi->io.offset = 0;
                continue;
            }
            /* Start a new message */
            hmi->io.accum[0] = id;
            hmi->io.offset = 2;
        }

        /* Check whether chunk fits into packet
         * NOTE The size of chunk header is same as the size of package header
         */
        if(hmi->io.cursor + len > hmi->io.packet[1]) {
            HMI_BAD_DATA("hmi_hid_fetch",
                         "Chunk data end exceeds packet length",
                         hmi->io.cursor + len, hmi->io.packet[1]);
            hmi->io.offset = 0;
            hmi->io.cursor = 0;
            continue;
        }

        /* Check whether complete message fits into the capacity */
        if(hmi->io.offset + len > 256) {
            HMI_BAD_DATA("hmi_hid_fetch",
                         "Overall message size exceeds capacity of 256 bytes",
                         hmi->io.offset + len, 0);
            if(incomplete)
                hmi->io.offset += len;
            else
                hmi->io.offset = 0;
            continue;
        }

        /* Copy from packet to accumulation buffer */
        HMI_MEMCPY(hmi->io.accum + hmi->io.offset,
                   hmi->io.packet + hmi->io.cursor + 2, len);

        /* Advance cursors */
        hmi->io.cursor += 2 + len;
        hmi->io.offset += len;

        /* Check if packet was completely processed */
        if(hmi->io.cursor == 2 + hmi->io.packet[1])
            hmi->io.cursor = 0;

        /* Check if chunk completed the message */
        if(!incomplete) {
            hmi->io.accum[1] = hmi->io.offset - 2;
            hmi->io.offset = 0;
            result = HMI_NO_ERROR;
            break;
        }
    }

    return result;
}

int hmi_message_receive(hmi_t *hmi, int *timeout)
{
    int result = HMI_NO_DATA;
    for(;;) {
        /* Fetch another message from incoming packets */
        result = hmi_hid_fetch(hmi, 0);

        if(result == HMI_NO_DATA) {
            if(!timeout)
                break;

            if(*timeout <= 0)
                break;

            HMI_SLEEP(20);
            *timeout -= 20;
            continue;
        }

        if(result != HMI_NO_ERROR)
            break;

        /* Handle received message */
        if(hmi->io.accum[0] == 0xFE) {
            /* HMI3D packets include the size as the first data byte */
            hmi->io.accum[1] += 1;
            /* Forward HMI3D data only */
            hmi3d_message_handle(hmi, hmi->io.accum + 1, hmi->io.accum[1]);
        } else {
            hmi2d_message_handle(hmi, hmi->io.accum);
        }
        break;
    }
    return result;
}

int hmi2d_message_write(hmi_t *hmi, int id, int size, unsigned char *msg) {
    /* Uses the same protocol as hmi_hid_fetch but only splits big messages
     * and doesn't pack mulitple messages into one packet
     */
    int result = HMI_NO_ERROR;
    unsigned char packet[64];
    int remaining = size;

    packet[0] = hmi->io.report_id;
    packet[2] = id;

    /* Send packets of up to 60 data bytes until complete message was sent */
    while(remaining) {
        int offset = size - remaining;
        int dataLength = (remaining > 60) ? 60 : remaining;

        packet[1] = 2 + dataLength;
        packet[3] = dataLength
                | ((remaining > dataLength) ? 0x40 : 0)
                | ((offset != 0) ? 0x80 : 0);

        HMI_MEMCPY(packet + 4, msg + offset, dataLength);

        if(hid_write(hmi->io.handle, packet, 64) != 64) {
            result = HMI_IO_ERROR;
            break;
        }

        remaining -= dataLength;
    }

    /* Send message without data as this is not handled by the while loop */
    if(size == 0) {
        packet[1] = 2;
        packet[3] = 0;
        if(hid_write(hmi->io.handle, packet, 64) != 64)
            result = HMI_IO_ERROR;
    }

    return result;
}

int hmi3d_message_write(hmi_t *hmi, void *msg, int size)
{
    unsigned char *data = (unsigned char*)msg;
    return hmi2d_message_write(hmi, hmi2d_msg_t_3d_subsystem,
                               size - 1, data + 1);
}

int hmi3d_reset(hmi_t *hmi)
{
    unsigned char msg[] = { 0x11, 0x00, 0x00, 0x00, 0x00, 0x00 };

    HMI_ASSERT(hmi);
    HMI_ASSERT(HMI_CONNECTED(hmi));

    return hmi2d_message_write(hmi, hmi2d_msg_t_3d_reset, sizeof(msg), msg);
}

#endif
