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
#include <hmi_api.h>
#include <stdio.h>
#include <unistd.h>

/* This demo shows how to use hmi_t directly.
 * This may be used to avoid the allocation on the heap.
 */

int main() {
    int i;

    /* Create HMI-Instance */
    hmi_t *hmi = hmi_create();

    /* Bitmask later used for starting a stream with SD- and position-data */
    const int stream_flags = hmi3d_DataOutConfigMask_xyzPosition |
            hmi3d_DataOutConfigMask_SDData;

    /* Pointer to the internal data-buffers */
    const hmi3d_signal_t * sd = NULL;
    const hmi3d_position_t * pos = NULL;

    printf("HMI 3D Data Retrieval Demo %s\n", hmi_version_str());

    /* Initialize all variables and required resources of hmi */
    hmi_initialize(hmi);

    /* Aquire the pointers to the data-buffers */
    sd = hmi3d_get_sd(hmi);
    pos = hmi3d_get_position(hmi);

    /* Open connection to the device */
    if(hmi_open(hmi) < 0) {
        fprintf(stderr, "Could not open connection to device.\n");
        return -1;
    }

    /* Enable 3D message communication only */
    if(hmi2d_set_com_mask(hmi, hmi2d_com_3d_messages, hmi2d_com_all)
            != HMI_NO_ERROR)
    {
        fprintf(stderr, "Could not set communication mode.\n");
        return -1;
    }

    /* Disable all active features */
    if(hmi2d_set_active_mask(hmi, 0, hmi2d_active_all) != HMI_NO_ERROR) {
        fprintf(stderr, "Could not set active mask.\n");
        return -1;
    }

    /* Set 3D only operation mode */
    if(hmi2d_set_operation_mode(hmi, hmi2d_3d_mode) != HMI_NO_ERROR) {
        fprintf(stderr, "Could not set 3D only operation mode.\n");
        return -1;
    }

    /* Try to reset the device to the default state:
     * - Automatic calibration enabled
     * - All frequencies allowed
     * - Approach detection disabled
     */
    if(hmi3d_set_auto_calibration(hmi, 1) < 0 ||
       hmi3d_select_frequencies(hmi, hmi3d_all_freq) < 0 ||
       hmi3d_set_approach_detection(hmi, 0) < 0)
    {
        fprintf(stderr, "Could not reset device to default state.\n");
        return -1;
    }

    /* Set output-mask to the bitmask defined above and stream all data */
    if(hmi3d_set_output_enable_mask(hmi, stream_flags, stream_flags,
                                    hmi3d_DataOutConfigMask_OutputAll) < 0)
    {
        fprintf(stderr, "Could not set output-mask for streaming.\n");
        return -1;
    }

    /* Listens for about 10 seconds to incoming messages */
    for(i = 0; i < 200; ++i) {
        /* Try to fetch stream-data until no more messages are available*/
        while(!hmi3d_retrieve_data(hmi, 0)) {
            /* Output the position */
            printf("Position: %5d %5d %5d\n", pos->x, pos->y, pos->z);

            /* Output the SD-data */
            printf("SD-Data:  %5.0f %5.0f %5.0f %5.0f %5.0f\n",
                   sd->channel[0], sd->channel[1], sd->channel[2],
                   sd->channel[3], sd->channel[4]);
        }
        usleep(50000);
    }

    /* Reset default communication mode */
    if(hmi2d_set_com_mask(hmi, hmi2d_com_default, hmi2d_com_all)
            != HMI_NO_ERROR)
    {
        fprintf(stderr, "Could not reset default communication mode.\n");
    }

    /* Reset default features */
    if(hmi2d_set_active_mask(hmi, hmi2d_active_default, hmi2d_active_all)
            != HMI_NO_ERROR)
    {
        fprintf(stderr, "Could not reset default active mask.\n");
    }

    /* Reset to mixed operation mode */
    if(hmi2d_set_operation_mode(hmi, hmi2d_mixed_mode) != HMI_NO_ERROR) {
        fprintf(stderr, "Could not restore mixed operation mode.\n");
    }

    /* Close connection to device */
    hmi_close(hmi);

    /* Release further resources that were used by hmi */
    hmi_cleanup(hmi);
    hmi_free(hmi);

    return 0;
}
