// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: Subscribe to local publish/subscribe messages

#include <assert.h>
#include <gg/error.h>
#include <gg/ipc/client.h>
#include <gg/object.h>
#include <gg/sdk.h>
#include <gg/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static void on_subscription_response(
    void *ctx, GgBuffer topic, GgObject payload, GgIpcSubscriptionHandle handle
) {
    (void) ctx;
    (void) handle;

    if (gg_obj_type(payload) == GG_TYPE_BUF) {
        GgBuffer message = gg_obj_into_buf(payload);
        printf(
            "Received new message on topic %.*s: %.*s\n",
            (int) topic.len,
            topic.data,
            (int) message.len,
            message.data
        );
    } else {
        assert(gg_obj_type(payload) == GG_TYPE_MAP);
        printf(
            "Received new message on topic %.*s: (JSON message)\n",
            (int) topic.len,
            topic.data
        );
    }
}

int main(void) {
    gg_sdk_init();

    GgError err = ggipc_connect();
    if (err != GG_ERR_OK) {
        fprintf(stderr, "Failed to establish IPC connection.\n");
        exit(-1);
    }

    GgBuffer topic = GG_STR("my/topic");

    GgIpcSubscriptionHandle handle;
    err = ggipc_subscribe_to_topic(
        topic, on_subscription_response, NULL, &handle
    );
    if (err != GG_ERR_OK) {
        fprintf(
            stderr,
            "Failed to subscribe to topic: %.*s\n",
            (int) topic.len,
            topic.data
        );
        exit(-1);
    }

    printf(
        "Successfully subscribed to topic: %.*s\n", (int) topic.len, topic.data
    );

    // Keep the main thread alive, or the process will exit.
    while (1) {
        sleep(10);
    }

    // To stop subscribing, close the stream.
    ggipc_close_subscription(handle);
}
