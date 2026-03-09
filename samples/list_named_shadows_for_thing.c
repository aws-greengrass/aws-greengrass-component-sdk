// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: List named shadows for a thing

#include <gg/buffer.h>
#include <gg/error.h>
#include <gg/ipc/client.h>
#include <gg/sdk.h>
#include <stdio.h>
#include <stdlib.h>

#define THING_NAME "<define_your_own_thingName>"

static void print_shadow(void *ctx, GgBuffer shadow_name) {
    (void) ctx;
    printf("  - %.*s\n", (int) shadow_name.len, shadow_name.data);
}

int main(void) {
    gg_sdk_init();

    GgError err = ggipc_connect();
    if (err != GG_ERR_OK) {
        fprintf(stderr, "Failed to establish IPC connection.\n");
        exit(-1);
    }

    printf("Named shadows for thing %s:\n", THING_NAME);

    err = ggipc_list_named_shadows_for_thing(
        GG_STR(THING_NAME), 0, &print_shadow, NULL
    );
    if (err != GG_ERR_OK) {
        fprintf(
            stderr, "Failed to list named shadows for thing %s.\n", THING_NAME
        );
        exit(-1);
    }
}
