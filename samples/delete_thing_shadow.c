// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: Delete a thing shadow

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
    GgBuffer shadow_name = GG_STR("my-shadow");

    err = ggipc_delete_thing_shadow(thing_name, &shadow_name);
    if (err != GG_ERR_OK) {
        fprintf(stderr, "Failed to delete shadow.\n");
        exit(-1);
    }

    printf("Shadow deleted successfully.\n");
}
