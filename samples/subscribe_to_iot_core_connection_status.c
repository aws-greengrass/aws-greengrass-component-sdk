// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: Subscribe to IoT Core MQTT connection status changes

#include <gg/error.h>
#include <gg/ipc/client.h>
#include <gg/sdk.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static void on_connection_status(
    void *ctx, bool connected, GgIpcSubscriptionHandle handle
) {
    (void) ctx;
    (void) handle;

    printf(
        "IoT Core connection status changed: %s\n",
        connected ? "CONNECTED" : "DISCONNECTED"
    );
}

int main(void) {
    gg_sdk_init();

    GgError err = ggipc_connect();
    if (err != GG_ERR_OK) {
        fprintf(stderr, "Failed to establish IPC connection.\n");
        exit(-1);
    }

    GgIpcSubscriptionHandle handle;
    err = ggipc_subscribe_to_iot_core_connection_status(
        on_connection_status, NULL, &handle
    );
    if (err != GG_ERR_OK) {
        fprintf(stderr, "Failed to subscribe to IoT Core connection status.\n");
        exit(-1);
    }

    printf("Successfully subscribed to IoT Core connection status.\n");

    // Keep the main thread alive, or the process will exit.
    while (1) {
        sleep(10);
    }

    // To stop subscribing, close the subscription handle.
    ggipc_close_subscription(handle);
}
