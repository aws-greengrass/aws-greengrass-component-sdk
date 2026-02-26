// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: Get a thing shadow

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

    uint8_t shadow_buf[8192];
    GgBuffer payload = GG_BUF(shadow_buf);

    err = ggipc_get_thing_shadow(
        gg_buffer_from_null_term(THING_NAME),
        gg_buffer_from_null_term(SHADOW_NAME),
        &payload
    );
    if (err != GG_ERR_OK) {
        fprintf(
            stderr,
            "Failed to get thing shadow %s from the thing %s.\n",
            SHADOW_NAME,
            THING_NAME
        );
        exit(-1);
    }

    printf("Shadow document:\n%.*s\n", (int) payload.len, payload.data);
}
