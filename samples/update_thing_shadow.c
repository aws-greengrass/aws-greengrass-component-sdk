// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: Update a thing shadow

#include <gg/buffer.h>
#include <gg/error.h>
#include <gg/ipc/client.h>
#include <gg/sdk.h>
#include <stdio.h>
#include <stdlib.h>

#define THING_NAME "lite-shadow-thing"
#define SHADOW_NAME "bike_status"

int main(void) {
    gg_sdk_init();

    GgError err = ggipc_connect();
    if (err != GG_ERR_OK) {
        fprintf(stderr, "Failed to establish IPC connection.\n");
        exit(-1);
    }

    const char *update_doc = "{\"state\":{\"desired\":{\"speed\":25}}}";
    GgBuffer payload = gg_buffer_from_null_term(update_doc);

    uint8_t response_buf[8192];
    GgBuffer response = GG_BUF(response_buf);

    err = ggipc_update_thing_shadow(
        gg_buffer_from_null_term(THING_NAME),
        gg_buffer_from_null_term(SHADOW_NAME),
        payload,
        &response
    );
    if (err != GG_ERR_OK) {
        fprintf(
            stderr,
            "Failed to update thing shadow %s for thing %s.\n",
            SHADOW_NAME,
            THING_NAME
        );
        exit(-1);
    }

    printf(
        "Shadow updated successfully:\n%.*s\n",
        (int) response.len,
        response.data
    );
}
