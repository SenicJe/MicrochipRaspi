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
#include "console.h"

#include <stdlib.h>

/* Human readable strings for the gestures */
static const char *gestures[] = {
    "-                       ",
    "Flick West > East       ",
    "Flick East > West       ",
    "Flick South > North     ",
    "Flick North > South     ",
    "Circle clockwise        ",
    "Circle counter-clockwise"
};

/* Human readable button strings */
static const char *buttons[]= {
    "no button            ",
    "left                 ",
    "right                ",
    "left + right         ",
    "middle               ",
    "left + middle        ",
    "right + middle       ",
    "left + right + middle"
};

void init_device(data_t *data)
{
    /* Bitmask later used for starting a stream with
     * SD-, position- and gesture-data
     */
    const int stream_flags = hmi3d_DataOutConfigMask_DSPStatus |
            hmi3d_DataOutConfigMask_GestureInfo |
            hmi3d_DataOutConfigMask_TouchInfo |
            hmi3d_DataOutConfigMask_AirWheelInfo |
            hmi3d_DataOutConfigMask_xyzPosition |
            hmi3d_DataOutConfigMask_SDData;
    const int com_flags = hmi2d_com_3d_messages |
            hmi2d_com_finger_positions |
            hmi2d_com_mouse_buttons;

    /* Allocate a new hmi_t-instance */
    data->hmi = hmi_create();

    /* Initialize the hmi_t-instance */
    hmi_initialize(data->hmi);

    /* Open a connection to the device */
    if(hmi_open(data->hmi) < 0) {
        mvprintw(3, 0, "Could not open connection to device.\n");
        refresh();
        getch();
        exit(-1);
    }

    /* Get pointers to the result-buffers */
    data->out_pos = hmi3d_get_position(data->hmi);
    data->out_sd = hmi3d_get_sd(data->hmi);
    data->out_calib = hmi3d_get_calibration(data->hmi);
    data->out_gesture = hmi3d_get_gesture(data->hmi);
    data->out_touch = hmi3d_get_touch(data->hmi);
    data->out_air_wheel = hmi3d_get_air_wheel(data->hmi);
    data->fingers = hmi2d_get_finger_positions(data->hmi);
    data->mouse = hmi2d_get_mouse(data->hmi);

    /* Set required communication mode */
    if(hmi2d_set_com_mask(data->hmi, com_flags, hmi2d_com_all) != HMI_NO_ERROR)
    {
        mvprintw(3, 0, "Could not set communication mode.\n");
        refresh();
        getch();
        exit(-1);
    }

    /* Set required active features */
    if(hmi2d_set_active_mask(data->hmi, hmi2d_active_gesture_recognition,
                             hmi2d_active_all) != HMI_NO_ERROR)
    {
        mvprintw(3, 0, "Could not set active mask.\n");
        refresh();
        getch();
        exit(-1);
    }

    /* Set 2D/3D mixed operation mode */
    if(hmi2d_set_operation_mode(data->hmi, hmi2d_mixed_mode) != HMI_NO_ERROR) {
        mvprintw(3, 0, "Could not set 2D/3D mixed operation mode.\n");
        refresh();
        getch();
        exit(-1);
    }

    /* Reset the device to the default state:
     * - Automatic calibration enabled
     * - All frequencies allowed
     * - Approach detection enabled for power saving
     * - Enable all gestures ( bit 1 to 6 set to 1 = 126 )
     */
    data->air_wheel = 1;
    data->touch_detect = 1;
    if(hmi3d_set_auto_calibration(data->hmi, data->auto_calib) < 0 ||
       hmi3d_select_frequencies(data->hmi, hmi3d_all_freq) < 0 ||
       hmi3d_set_approach_detection(data->hmi, 1) < 0 ||
       hmi3d_set_air_wheel_enabled(data->hmi, data->air_wheel) < 0 ||
       hmi3d_set_touch_detection(data->hmi, data->touch_detect) < 0 ||
       hmi3d_set_enabled_gestures(data->hmi, 126) < 0)
    {
        mvprintw(3, 0, "Could not reset device to default state.\n");
        refresh();
        getch();
        exit(-1);
    }

    /* Set output-mask to the bitmask defined above and output only changes */
    if(hmi3d_set_output_enable_mask(data->hmi, stream_flags, 0,
                                    hmi3d_DataOutConfigMask_OutputAll) < 0)
    {
        mvprintw(3, 0, "Could not set data-output of device.\n");
        refresh();
        getch();
        exit(-1);
    }
}

void free_device(data_t *data)
{
    /* Reset default communication mode */
    if(hmi2d_set_com_mask(data->hmi, hmi2d_com_default, hmi2d_com_all)
            != HMI_NO_ERROR)
    {
        fprintf(stderr, "Could not reset default communication mode.\n");
    }

    /* Reset default features */
    if(hmi2d_set_active_mask(data->hmi, hmi2d_active_default, hmi2d_active_all)
            != HMI_NO_ERROR)
    {
        fprintf(stderr, "Could not reset default active mask.\n");
    }

    /* Close the connection to the device */
    hmi_close(data->hmi);

    /* Release resources that were associated with the hmi_t-instance */
    hmi_cleanup(data->hmi);

    /* Release the hmi_t-instance itself */
    hmi_free(data->hmi);
}

void update_device(data_t *data)
{
    int i;

    /* Abbreviations for the electrodes */
    const char source[] = { 'S', 'W', 'N', 'E', 'C' };

    /* Retrieve all 3D events and states */
    while(hmi3d_retrieve_data(data->hmi, NULL) == 0)
    {
        if(data->out_gesture != NULL) {
            /* Remember last gesture and reset it after 1s = 200 updates */
            if(data->out_gesture->gesture > 0 && data->out_gesture->gesture <= 6)
                data->last_gesture = data->out_gesture->gesture;
            else if(data->out_gesture->last_event > 200)
                data->last_gesture = 0;
        }
    }

    /* Retrieve all 2D events and states */
    while(hmi2d_retrieve_data(data->hmi) == HMI_NO_ERROR)
    {
        if(data->mouse->press_event) {
            /* Remember last button_press state */
            data->last_mouse = data->mouse->press_event;
            data->mouse_countdown = 100;
        }
    }

    /* Print update of the position */
    if(data->out_pos != NULL) {
        mvprintw(4, 0, "Position:         X %5d   Y %5d   Z %5d   ",
                 data->out_pos->x, data->out_pos->y,
                 data->out_pos->z);
    } else {
        mvprintw(4, 0, "Position:         Not available");
    }

    /* Print update of gestures */
    if(data->out_gesture != NULL) {
        mvprintw(5, 0, "Gesture:          %s ",
                 gestures[data->last_gesture]);
    } else {
        mvprintw(5, 0, "Gesture:          Not available");
    }

    /* Print update of touch */
    mvprintw(6, 0, "Touch:            ");
    for(i = 0; i < 5; ++i) {
        if(data->out_touch->touch_flags & (1<<i))
            printw("%c ", source[i]);
    }
    printw("           ");

    /* Print update of AirWheel */
    if(data->out_air_wheel != NULL) {
        mvprintw(7, 0, "Air Wheel:        %d   ",
                 data->out_air_wheel->counter);
    } else {
        mvprintw(7, 0, "Air Wheel:        Not available");
    }

    /* Print update of Signal Deviation */
    if(data->out_sd != NULL) {
        mvprintw(8, 0, "Signal Deviation:");
        for(i = 0; i < 5; ++i)
            printw(" %c %5.0f  ", source[i], data->out_sd->channel[i]);

        if(data->out_calib != NULL)
            mvprintw(9, 0, "Last Calibration: %7d",
                     data->out_calib->last_event);
    } else {
        mvprintw(8, 0, "Signal Deviation: Not available");
    }

    /* Print update of fingers touch */
    if(data->fingers != NULL) {
        mvprintw(12, 0, "Finger ID:   ");
        for(i = 0; i < data->fingers->count; ++i)
            printw("%5d ", data->fingers->entry[i].finger_id);
        for(i = data->fingers->count; i < 10; ++i)
            printw("  --- ");
        mvprintw(13, 0, "Pos X:       ");
        for(i = 0; i < data->fingers->count; ++i)
            printw("%5d ", data->fingers->entry[i].x);
        for(i = data->fingers->count; i < 10; ++i)
            printw("  --- ");
        mvprintw(14, 0, "Pos Y:       ");
        for(i = 0; i < data->fingers->count; ++i)
            printw("%5d ", data->fingers->entry[i].y);
        for(i = data->fingers->count; i < 10; ++i)
            printw("  --- ");
    } else {
        mvprintw(12, 0, "Fingers:     Not available");
    }

    /* Print update of mouse emulation */
    if(data->mouse != NULL) {
        mvprintw(15, 0, "Mouse Press: %s ", buttons[data->last_mouse]);
    } else {
        mvprintw(15, 0, "Mouse Press: Not available");
    }

    /* Reset output after countdown */
    if(data->mouse_countdown)
        data->mouse_countdown--;
    else
        data->last_mouse = 0;
}

void switch_auto_calib(data_t *data)
{
    /* Try to swith autocalibration */
    int new_mode = !data->auto_calib;
    if(hmi3d_set_auto_calibration(data->hmi, new_mode) != 0)
        return;
    data->auto_calib = new_mode;
}

void calibrate_now(data_t *data)
{
    /* Try to force calibration */
    hmi3d_force_calibration(data->hmi);
}

void switch_touch_detect(data_t *data) {
    int new_mode = !data->touch_detect;
    if(hmi3d_set_touch_detection(data->hmi, new_mode) != 0)
        return;
    data->touch_detect = new_mode;
}

void switch_air_wheel(data_t *data) {
    int new_mode = !data->air_wheel;
    if(hmi3d_set_air_wheel_enabled(data->hmi, new_mode) != 0)
        return;
    data->air_wheel = new_mode;
}
