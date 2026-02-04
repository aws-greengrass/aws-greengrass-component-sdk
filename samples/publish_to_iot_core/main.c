// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: Publish a message to AWS IoT Core

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

    GgBuffer message = GG_STR("Hello, World");
    GgBuffer topic = GG_STR("my/topic");
    uint8_t qos = 1;

    err = ggipc_publish_to_iot_core(topic, message, qos);
    if (err != GG_ERR_OK) {
        fprintf(
            stderr,
            "Failed to publish to topic: %.*s\n",
            (int) topic.len,
            topic.data
        );
        exit(-1);
    }

    printf(
        "Successfully published to topic: %.*s\n", (int) topic.len, topic.data
    );
}
