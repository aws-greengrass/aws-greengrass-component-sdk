// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include <gg/buffer.h>
#include <gg/error.h>
#include <gg/ipc/client.h>
#include <gg/ipc/client_raw.h>
#include <gg/log.h>
#include <gg/map.h>
#include <gg/object.h>
#include <stddef.h>

static GgError error_handler(void *ctx, GgBuffer error_code, GgBuffer message) {
    (void) ctx;

    GG_LOGE(
        "Received DeleteThingShadow error %.*s: %.*s.",
        (int) error_code.len,
        error_code.data,
        (int) message.len,
        message.data
    );

    if (gg_buffer_eq(error_code, GG_STR("InvalidArgumentsError"))) {
        return GG_ERR_INVALID;
    }
    if (gg_buffer_eq(error_code, GG_STR("ResourceNotFoundError"))) {
        return GG_ERR_NOENTRY;
    }
    if (gg_buffer_eq(error_code, GG_STR("ServiceError"))) {
        return GG_ERR_RETRY;
    }
    if (gg_buffer_eq(error_code, GG_STR("UnauthorizedError"))) {
        return GG_ERR_UNAUTHORIZED;
    }

    return GG_ERR_FAILURE;
}

static GgError response_handler(void *ctx, GgMap result) {
    (void) ctx;

    GgObject *payload_obj;
    GgError ret = gg_map_validate(
        result,
        GG_MAP_SCHEMA(
            { GG_STR("payload"), GG_OPTIONAL, GG_TYPE_BUF, &payload_obj }
        )
    );
    if (ret != GG_ERR_OK) {
        GG_LOGE("Failed validating DeleteThingShadow response.");
        return GG_ERR_INVALID;
    }

    return GG_ERR_OK;
}

GgError ggipc_delete_thing_shadow(
    GgBuffer thing_name, GgBuffer shadow_name, GgBuffer *payload
) {
    GgMap args = GG_MAP(
        gg_kv(GG_STR("thingName"), gg_obj_buf(thing_name)),
        gg_kv(GG_STR("shadowName"), gg_obj_buf(shadow_name))
    );

    return ggipc_call(
        GG_STR("aws.greengrass#DeleteThingShadow"),
        GG_STR("aws.greengrass#DeleteThingShadowRequest"),
        args,
        &response_handler,
        &error_handler,
        payload
    );
}
