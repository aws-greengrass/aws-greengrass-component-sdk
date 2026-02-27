// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: List named shadows for a thing

#include <gg/buffer.h>
#include <gg/error.h>
#include <gg/ipc/client.h>
#include <gg/object.h>
#include <gg/sdk.h>
#include <stdio.h>
#include <stdlib.h>

#define THING_NAME "lite-shadow-thing"

int main(void) {
    gg_sdk_init();

    GgError err = ggipc_connect();
    if (err != GG_ERR_OK) {
        fprintf(stderr, "Failed to establish IPC connection.\n");
        exit(-1);
    }

    GgList results;
    uint8_t next_token_buf[256];
    GgBuffer next_token = GG_BUF(next_token_buf);

    err = ggipc_list_named_shadows_for_thing(
        gg_buffer_from_null_term((char *) THING_NAME),
        GG_STR(""),
        &results,
        &next_token
    );
    if (err != GG_ERR_OK) {
        fprintf(
            stderr, "Failed to list named shadows for thing %s.\n", THING_NAME
        );
        exit(-1);
    }

    printf("Named shadows for thing %s:\n", THING_NAME);
    for (size_t i = 0; i < results.len; i++) {
        GgObject shadow_name_obj = results.items[i];
        if (gg_obj_type(shadow_name_obj) == GG_TYPE_BUF) {
            GgBuffer shadow_name = gg_obj_into_buf(shadow_name_obj);
            printf("  - %.*s\n", (int) shadow_name.len, shadow_name.data);
        }
    }

    if (next_token.len > 0) {
        printf("Next token: %.*s\n", (int) next_token.len, next_token.data);
    }
}
