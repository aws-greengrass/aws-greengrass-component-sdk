// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: List named shadows for a thing

#include <gg/buffer.h>
#include <gg/error.h>
#include <gg/ipc/client.h>
#include <gg/sdk.h>
#include <stdio.h>
#include <stdlib.h>

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

    GgBuffer thing_name
        = gg_buffer_from_null_term(getenv("AWS_IOT_THING_NAME"));

    printf(
        "Named shadows for thing %.*s:\n", (int) thing_name.len, thing_name.data
    );

    err = ggipc_list_named_shadows_for_thing(thing_name, &print_shadow, NULL);
    if (err != GG_ERR_OK) {
        fprintf(
            stderr,
            "Failed to list named shadows for thing %.*s.\n",
            (int) thing_name.len,
            thing_name.data
        );
        exit(-1);
    }
}
