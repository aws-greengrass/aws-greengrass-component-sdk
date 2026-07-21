// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include <gg/buffer.h>
#include <gg/error.h>
#include <gg/flags.h>
#include <gg/ipc/client.h>
#include <gg/ipc/client_raw.h>
#include <gg/log.h>
#include <gg/map.h>
#include <gg/object.h>

static GgError subscribe_to_iot_core_connection_status_resp_handler(
    void *ctx,
    void *aux_ctx,
    GgIpcSubscriptionHandle handle,
    GgBuffer service_model_type,
    GgMap data
) {
    GgIpcSubscribeToIotCoreConnectionStatusCallback *callback = ctx;

    if (!gg_buffer_eq(
            service_model_type,
            GG_STR("aws.greengrass#IoTCoreConnectionStatusEvent")
        )) {
        GG_LOGE("Unexpected service-model-type received.");
        return GG_ERR_INVALID;
    }

    GgObject *connection_status_event_obj;
    GgError ret = gg_map_validate(
        data,
        GG_MAP_SCHEMA(
            { GG_STR("connectionStatusEvent"),
              GG_REQUIRED,
              GG_TYPE_MAP,
              &connection_status_event_obj },
        )
    );
    if (ret != GG_ERR_OK) {
        GG_LOGE("Received invalid IoT Core connection status event payload.");
        return GG_ERR_INVALID;
    }

    GgObject *status_obj;
    ret = gg_map_validate(
        gg_obj_into_map(*connection_status_event_obj),
        GG_MAP_SCHEMA(
            { GG_STR("status"), GG_REQUIRED, GG_TYPE_BUF, &status_obj },
        )
    );
    if (ret != GG_ERR_OK) {
        GG_LOGE("Received invalid IoT Core connection status event.");
        return GG_ERR_INVALID;
    }

    GgBuffer status = gg_obj_into_buf(*status_obj);

    bool connected;
    if (gg_buffer_eq(status, GG_STR("CONNECTED"))) {
        connected = true;
    } else if (gg_buffer_eq(status, GG_STR("DISCONNECTED"))) {
        connected = false;
    } else {
        GG_LOGW(
            "Unknown IoT Core connection status: %.*s; ignoring.",
            (int) status.len,
            status.data
        );
        return GG_ERR_OK;
    }

    callback(aux_ctx, connected, handle);
    return GG_ERR_OK;
}

static GgError error_handler(void *ctx, GgBuffer error_code, GgBuffer message) {
    (void) ctx;

    GG_LOGE(
        "Received SubscribeToIoTCoreConnectionStatus error %.*s: %.*s.",
        (int) error_code.len,
        error_code.data,
        (int) message.len,
        message.data
    );

    return GG_ERR_FAILURE;
}

GgError ggipc_subscribe_to_iot_core_connection_status(
    GgIpcSubscribeToIotCoreConnectionStatusCallback *callback,
    void *ctx,
    GgIpcSubscriptionHandle *handle
) {
    GgMap args = (GgMap) { 0 };

    return ggipc_subscribe(
        GG_STR("aws.greengrass#SubscribeToIoTCoreConnectionStatus"),
        GG_STR("aws.greengrass#SubscribeToIoTCoreConnectionStatusResponse"),
        args,
        NULL,
        &error_handler,
        NULL,
        &subscribe_to_iot_core_connection_status_resp_handler,
        callback,
        ctx,
        handle
    );
}
