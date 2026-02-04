// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: Restart a component

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

    GgBuffer component_name = GG_STR("com.example.HelloWorld");

    err = ggipc_restart_component(component_name);
    if (err != GG_ERR_OK) {
        fprintf(
            stderr,
            "Failed to restart component: %.*s\n",
            (int) component_name.len,
            component_name.data
        );
        exit(-1);
    }

    printf(
        "Successfully requested restart for component: %.*s\n",
        (int) component_name.len,
        component_name.data
    );
}
