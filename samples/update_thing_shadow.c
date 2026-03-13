// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: Update a thing shadow

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
    GgBuffer payload = GG_STR("<define_your_own_payload>");

    err = ggipc_update_thing_shadow(thing_name, &shadow_name, payload, NULL);
    if (err != GG_ERR_OK) {
        fprintf(
            stderr,
            "Failed to update thing shadow %.*s for thing %.*s.\n",
            (int) shadow_name.len,
            shadow_name.data,
            (int) thing_name.len,
            thing_name.data
        );
        exit(-1);
    }

    printf("Shadow updated successfully.\n");
}
