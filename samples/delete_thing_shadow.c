// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: Delete a thing shadow

#include <gg/buffer.h>
#include <gg/error.h>
#include <gg/ipc/client.h>
#include <gg/sdk.h>
#include <stdio.h>
#include <stdlib.h>

#define THING_NAME "<define_your_own_thingName>"
#define SHADOW_NAME "<define_your_own_shadowName>"

int main(void) {
    gg_sdk_init();

    GgError err = ggipc_connect();
    if (err != GG_ERR_OK) {
        fprintf(stderr, "Failed to establish IPC connection.\n");
        exit(-1);
    }

    uint8_t response_buf[8192];
    GgBuffer response = GG_BUF(response_buf);

    err = ggipc_delete_thing_shadow(
        GG_STR(THING_NAME), GG_STR(SHADOW_NAME), &response
    );
    if (err != GG_ERR_OK) {
        fprintf(
            stderr,
            "Failed to delete thing shadow %s for thing %s.\n",
            SHADOW_NAME,
            THING_NAME
        );
        exit(-1);
    }

    printf("Shadow %s deleted successfully.\n", SHADOW_NAME);
}
