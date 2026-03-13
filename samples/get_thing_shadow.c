// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: Get a thing shadow

#include <gg/buffer.h>
#include <gg/error.h>
#include <gg/ipc/client.h>
#include <gg/sdk.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    gg_sdk_init();

    GgError err = ggipc_connect();
    if (err != GG_ERR_OK) {
        fprintf(stderr, "Failed to establish IPC connection.\n");
        exit(-1);
    }

    GgBuffer thing_name
        = gg_buffer_from_null_term(getenv("AWS_IOT_THING_NAME"));
    GgBuffer shadow_name = GG_STR("<define_your_own_shadowName>");

    uint8_t shadow_buf[8192];
    GgBuffer payload = GG_BUF(shadow_buf);

    err = ggipc_get_thing_shadow(thing_name, &shadow_name, &payload);
    if (err != GG_ERR_OK) {
        fprintf(
            stderr,
            "Failed to get thing shadow %.*s from the thing %.*s.\n",
            (int) shadow_name.len,
            shadow_name.data,
            (int) thing_name.len,
            thing_name.data
        );
        exit(-1);
    }

    printf("Shadow document:\n%.*s\n", (int) payload.len, payload.data);
}
