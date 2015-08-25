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
#include "2d.h"

#ifndef HMI_NO_DATA_RETRIEVAL

void hmi2d_handle_data_row(hmi_t *hmi,
                           hmi2d_row_t *row,
                           const unsigned char *msg)
{
    int size = msg[1];
    const unsigned char *data = msg + 2;
    unsigned short mask = (size >= 2) ? GET_U16(data) : 0;
    const unsigned char *cursor = data + 2;
    int i;

#ifdef HMI3D_SYNC_THREADING
    /* Synchronize against hmi2d_retrieve_data calls from application */
    HMI_SYNC_LOCK(hmi->io_sync);
#endif

    hmi->internal2d.msg_counter++;

    for(i = 0; i < 16; ++i) {
        if(mask & (1 << i)) {
            if(cursor + 1 - data >= size) {
                HMI_BAD_DATA("hmi2d_handle_mutual_raw",
                             "Mask doesn't fit available entries",
                             mask, (size-2) / 2);
                break;
            }
            (*row)[i] = GET_U16(cursor);
            cursor += 2;
        } else {
            (*row)[i] = 0;
        }
    }

#ifdef HMI3D_SYNC_THREADING
    /* Release synchronization against hmi2d_retrieve_data */
    HMI_SYNC_UNLOCK(hmi->io_sync);
#endif
}

void hmi2d_handle_finger_pos(hmi_t *hmi, const unsigned char *msg)
{
    int size = msg[1];
    int i;
    int count = size / 4;

#ifdef HMI3D_SYNC_THREADING
    /* Synchronize against hmi2d_retrieve_data calls from application */
    HMI_SYNC_LOCK(hmi->io_sync);
#endif

    hmi->internal2d.msg_counter++;

    if(count > 10) {
        HMI_BAD_DATA("hmi2d_handle_finger_pos",
                     "Message contains more than 10 finger positions",
                     count, 0);
        count = 10;
    }

    for(i = 0; i < count; ++i) {
        unsigned int v = GET_U32(msg + 2 + 4*i);
        hmi->internal2d.fingers.entry[i].finger_id = v & 0xFF;
        hmi->internal2d.fingers.entry[i].x = (v >> 20) & 0xFFF;
        hmi->internal2d.fingers.entry[i].y = (v >> 8) & 0xFFF;
    }
    hmi->internal2d.fingers.count = count;

#ifdef HMI3D_SYNC_THREADING
    /* Release synchronization against hmi2d_retrieve_data */
    HMI_SYNC_UNLOCK(hmi->io_sync);
#endif
}

void hmi2d_handle_mouse_btns(hmi_t *hmi, const unsigned char *msg)
{
    int size = msg[1];
    int state, old_state;

    if(size != 1) {
        HMI_BAD_DATA("hmi2d_handle_mouse_btns",
                     "Expected message of 1 byte",
                     size, 0);
        return;
    }

#ifdef HMI3D_SYNC_THREADING
    /* Synchronize against hmi2d_retrieve_data calls from application */
    HMI_SYNC_LOCK(hmi->io_sync);
#endif

    hmi->internal2d.msg_counter++;

    state = GET_U8(msg + 2);
    old_state = hmi->internal2d.mouse.button_state;
    hmi->internal2d.mouse.button_state = state;
    hmi->internal2d.mouse.press_event |= state & ~old_state;
    hmi->internal2d.mouse.release_event |= old_state & ~state;

#ifdef HMI3D_SYNC_THREADING
    /* Release synchronization against hmi2d_retrieve_data */
    HMI_SYNC_UNLOCK(hmi->io_sync);
#endif
}

void hmi2d_handle_gesture(hmi_t *hmi, const unsigned char *msg)
{
    int size = msg[1];

    if(size != 1) {
        HMI_BAD_DATA("hmi2d_handle_gesture",
                     "Expected message of 1 byte",
                     size, 0);
        return;
    }

#ifdef HMI3D_SYNC_THREADING
    /* Synchronize against hmi2d_retrieve_data calls from application */
    HMI_SYNC_LOCK(hmi->io_sync);
#endif

    hmi->internal2d.msg_counter++;

    hmi->internal2d.gesture.gesture = GET_U8(msg + 2);
    hmi->internal2d.last_gesture = hmi->internal2d.msg_counter;

#ifdef HMI3D_SYNC_THREADING
    /* Release synchronization against hmi2d_retrieve_data */
    HMI_SYNC_UNLOCK(hmi->io_sync);
#endif
}

int hmi2d_retrieve_data(hmi_t *hmi)
{
    int count;
    int result = HMI_NO_DATA;
    int last_counter, current_counter;

    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    last_counter = hmi->result2d.msg_counter;

#if defined(HMI3D_SYNC_INTERRUPT) || defined(HMI3D_SYNC_THREADING)
    /* Synchronize against changes of internal buffer from message-handlers */
    HMI_SYNC_LOCK(hmi->io_sync);
#endif

    for(;;) {
        /* Check whether new data are available */
        current_counter = hmi->internal2d.msg_counter;
        count = current_counter - last_counter;
        if(count > 0)
            break;

#if defined(HMI3D_SYNC_INTERRUPT) || defined(HMI3D_SYNC_THREADING)
        /* Temporarily release synchronization */
        HMI_SYNC_UNLOCK(hmi->io_sync);
#endif

        /* Receive and handle message */
        result = hmi_message_receive(hmi, NULL);

#if defined(HMI3D_SYNC_INTERRUPT) || defined(HMI3D_SYNC_THREADING)
        /* Relock after message handling */
        HMI_SYNC_LOCK(hmi->io_sync);
#endif

        if(result != HMI_NO_ERROR)
            break;
    }

    if(count > 0) {
        hmi->result2d = hmi->internal2d;

        /* Reset button-events */
        hmi->internal2d.mouse.press_event = 0;
        hmi->internal2d.mouse.release_event = 0;

        /* Reset gestures */
        if(hmi->result2d.last_gesture <= last_counter)
            hmi->result2d.gesture.gesture = 0;

        result = HMI_NO_ERROR;
    }

#if defined(HMI3D_SYNC_INTERRUPT) || defined(HMI3D_SYNC_THREADING)
    /* Release synchronization against message-handlers */
    HMI_SYNC_UNLOCK(hmi->io_sync);
#endif

    return result;
}

#endif
