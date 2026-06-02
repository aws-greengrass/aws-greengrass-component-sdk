// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: Create a local deployment that merges configuration into a component

#include <gg/error.h>
#include <gg/ipc/client.h>
#include <gg/map.h>
#include <gg/object.h>
#include <gg/sdk.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    gg_sdk_init();

    GgError err = ggipc_connect();
    if (err != GG_ERR_OK) {
        fprintf(stderr, "Failed to establish IPC connection.\n");
        exit(-1);
    }

    // Build componentToConfiguration:
    // {"com.example.MyComponent": {"MERGE": {"endpoint":
    // "https://example.com"}}}
    GgCreateLocalDeploymentArgs args = { 0 };
    args.component_to_configuration = GG_MAP(gg_kv(
        GG_STR("com.example.MyComponent"),
        gg_obj_map(GG_MAP(gg_kv(
            GG_STR("MERGE"),
            gg_obj_map(GG_MAP(gg_kv(
                GG_STR("endpoint"), gg_obj_buf(GG_STR("https://example.com"))
            )))
        )))
    ));

    uint8_t id_mem[64] = { 0 };
    GgBuffer id_buf = GG_BUF(id_mem);

    err = ggipc_create_local_deployment(&args, &id_buf);
    if (err != GG_ERR_OK) {
        fprintf(stderr, "Failed to create local deployment.\n");
        exit(-1);
    }

    printf("Deployment created: %.*s\n", (int) id_buf.len, id_buf.data);
}
