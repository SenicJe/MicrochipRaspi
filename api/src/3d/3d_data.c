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
#include "3d.h"

#ifndef HMI3D_NO_DATA_RETRIEVAL

unsigned char systemModeElectrodes[] = { 4, 5 };

void hmi3d_handle_data_output(hmi_t *hmi,
                              const unsigned char *data)
{
    int dataOutputConfig = GET_U16(data + 4);
    unsigned char timestamp = GET_U8(data + 6);
    int systemInfo = GET_U8(data + 7);
    const unsigned char *cursor = data + 8;
    int systemMode = (dataOutputConfig & hmi3d_DataOutConfigMask_ElectrodeConfiguration) >> 8;
    int electrodeCount = systemModeElectrodes[systemMode & 0x1];
    int increment;

    hmi3d_input_data_t *dest = &hmi->internal;

    int airWheelActive = (systemInfo & hmi3d_SystemInfo_AirWheelValid) ? 1 : 0;

#ifdef HMI_SYNC_THREADING
    /* Synchronize against hmi3d_retrieve_data calls by application */
    HMI_SYNC_LOCK(hmi->io_sync);
#endif

    /* NOTE Overflows should not be a problem as long as more
     * than one message per 256 samples is received.
     * Otherwise this algorithm will loose precision but should
     * still work as counts are only compared for equality
     * or via substraction.
     */
    increment = (unsigned char)(timestamp -
                                hmi->last_time_stamp);
    dest->frame_counter += increment ? increment : 1;
    hmi->last_time_stamp = timestamp;

    if(dataOutputConfig & hmi3d_DataOutConfigMask_DSPStatus) {
        int calibration = GET_U8(cursor);
        int frequency = GET_U8(cursor + 1);
        if(calibration != 0) {
            dest->calib.reason = calibration;
            dest->calib.last_event = dest->frame_counter;
        }
        if(frequency != dest->frequency.frequency) {
            dest->frequency.frequency = frequency;
            dest->frequency.freq_changed = 1;
            dest->frequency.last_event = dest->frame_counter;
        }
        cursor += 2;
    }
    if(dataOutputConfig & hmi3d_DataOutConfigMask_GestureInfo) {
        int gestureInfo = GET_U32(cursor);
        int raw = gestureInfo & 0xFF;
        int gesture = raw > 1 ? raw - 1 : 0;
        if(gesture) {
            dest->gesture.gesture = gesture;
            dest->gesture.flags = gestureInfo & hmi3d_gesture_flags_mask;
            dest->gesture.last_event = dest->frame_counter;
        }
        cursor += 4;
    }
    if(dataOutputConfig & hmi3d_DataOutConfigMask_TouchInfo) {
        int info = GET_U32(cursor);
        int touch = info & hmi3d_touch_mask;
        int tap = info & hmi3d_tap_mask;
        if((hmi3d_touch_flags_t)touch != dest->touch.touch_flags) {
            dest->touch.touch_flags = touch;
            dest->touch.last_touch_event = dest->frame_counter;
            dest->touch.last_touch_event_start = dest->frame_counter - ((info & 0xFF0000) >> 16);
        }
        if(tap) {
            dest->touch.tap_flags = tap;
            dest->touch.last_tap_event = dest->frame_counter;
        }
        cursor += 4;
    }
    if(dataOutputConfig & hmi3d_DataOutConfigMask_AirWheelInfo) {
        if(airWheelActive) {
            int counter = GET_U8(cursor);
            if(counter != dest->air_wheel.counter)
                dest->air_wheel.counter = counter;
        }
        cursor += 2;
    }
    if(airWheelActive != dest->air_wheel.active) {
        dest->air_wheel.active = airWheelActive;
        dest->air_wheel.last_event = dest->frame_counter;
    }
    if(dataOutputConfig & hmi3d_DataOutConfigMask_xyzPosition) {
        if(systemInfo & hmi3d_SystemInfo_PositionValid) {
            dest->pos.x = GET_U16(cursor);
            dest->pos.y = GET_U16(cursor+2);
            dest->pos.z = GET_U16(cursor+4);
        }
        cursor += 6;
    }
    dest->noise_power.valid = 0;
    if(dataOutputConfig & hmi3d_DataOutConfigMask_NoisePower) {
        if(systemInfo & hmi3d_SystemInfo_NoisePowerValid) {
            dest->noise_power.value = GET_F32(cursor);
            dest->noise_power.valid = 1;
        }
        cursor += 4;
    }
    if(dataOutputConfig & hmi3d_DataOutConfigMask_CICData) {
        if(systemInfo & hmi3d_SystemInfo_RawDataValid) {
            int i;
            for(i = 0; i < electrodeCount; ++i)
                dest->cic.channel[i] = GET_F32(cursor + 4*i);
            for(i = electrodeCount; i < 5; ++i)
                dest->cic.channel[i] = HMI_UNDEFINED_VALUE;
        }
        cursor += electrodeCount * 4;
    }
    if(dataOutputConfig & hmi3d_DataOutConfigMask_SDData) {
        if(systemInfo & hmi3d_SystemInfo_RawDataValid) {
            int i;
            for(i = 0; i < electrodeCount; ++i)
                dest->sd.channel[i] = GET_F32(cursor + 4*i);
            for(i = electrodeCount; i < 5; ++i)
                dest->sd.channel[i] = HMI_UNDEFINED_VALUE;
        }
        cursor += electrodeCount * 4;
    }

#ifdef HMI_SYNC_THREADING
    /* Release synchronization against hmi3d_retrieve_data */
    HMI_SYNC_UNLOCK(hmi->io_sync);
#endif
}

int hmi3d_retrieve_data(hmi_t *hmi, int *skipped) {
    int count;
    int error = HMI_NO_DATA;
    int last_counter, current_counter;

    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    last_counter = hmi->result.frame_counter;

#if defined(HMI_SYNC_INTERRUPT) || defined(HMI_SYNC_THREADING)
    /* Synchronize against changes of internal buffer from message-handler */
    HMI_SYNC_LOCK(hmi->io_sync);
#endif

    for(;;) {
        /* Check whether a full data-frame is available */
        current_counter = hmi->internal.frame_counter;
        count = current_counter - last_counter;
        if(count > 0)
            break;

#if defined(HMI_SYNC_INTERRUPT) || defined(HMI_SYNC_THREADING)
        /* Temporarily release synchronization */
        HMI_SYNC_UNLOCK(hmi->io_sync);
#endif

        /* Receive and handle message */
        error = hmi_message_receive(hmi, NULL);

#if defined(HMI_SYNC_INTERRUPT) || defined(HMI_SYNC_THREADING)
        /* Relock after message handling */
        HMI_SYNC_LOCK(hmi->io_sync);
#endif

        if(error != HMI_NO_ERROR)
            break;
    }

    if(count > 0) {
        hmi->result = hmi->internal;

        if(hmi->result.gesture.last_event <= last_counter) {
            hmi->result.gesture.gesture = 0;
            /* Reset flags except for the in-progress flag */
            hmi->result.gesture.flags &= hmi3d_gesture_in_progress;
        }
        hmi->result.gesture.last_event = current_counter -
                hmi->result.gesture.last_event;

        hmi->result.touch.last_touch_event = current_counter -
                hmi->result.touch.last_touch_event;
        if(hmi->result.touch.last_tap_event <= last_counter)
            hmi->result.touch.tap_flags = 0;
        hmi->result.touch.last_tap_event = current_counter -
                hmi->result.touch.last_tap_event;
        hmi->result.touch.last_touch_event_start = current_counter -
                hmi->result.touch.last_touch_event_start;

        hmi->result.air_wheel.last_event = current_counter -
                hmi->result.air_wheel.last_event;

        if(hmi->result.calib.last_event <= last_counter)
            hmi->result.calib.reason = 0;
        hmi->result.calib.last_event = current_counter -
                hmi->result.calib.last_event;

        if(hmi->result.frequency.last_event <= last_counter)
            hmi->result.frequency.freq_changed = 0;
        hmi->result.frequency.last_event = current_counter -
                hmi->result.frequency.last_event;

        if(skipped)
            *skipped = count - 1;

        error = HMI_NO_ERROR;
    }

#if defined(HMI_SYNC_INTERRUPT) || defined(HMI_SYNC_THREADING)
    /* Release synchronization against Hardware-Layer */
    HMI_SYNC_UNLOCK(hmi->io_sync);
#endif

    return error;
}

#endif
