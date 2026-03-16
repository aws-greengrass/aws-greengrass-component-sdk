// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include <gg/base64.h>
#include <gg/buffer.h>
#include <gg/error.h>
#include <gg/ipc/client.h>
#include <gg/ipc/client_raw.h>
#include <gg/log.h>
#include <gg/map.h>
#include <gg/object.h>
#include <gg/vector.h>
#include <stddef.h>

static GgError error_handler(void *ctx, GgBuffer error_code, GgBuffer message) {
    (void) ctx;

    GG_LOGE(
        "Received GetThingShadow error %.*s: %.*s.",
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
    GgBuffer *payload = ctx;

    GgObject *payload_obj;
    GgError ret = gg_map_validate(
        result,
        GG_MAP_SCHEMA(
            { GG_STR("payload"), GG_REQUIRED, GG_TYPE_BUF, &payload_obj }
        )
    );
    if (ret != GG_ERR_OK) {
        GG_LOGE("Failed validating GetThingShadow response.");
        return GG_ERR_INVALID;
    }

    GgBuffer b64_payload = gg_obj_into_buf(*payload_obj);
    if ((payload != NULL) && !gg_base64_decode(b64_payload, payload)) {
        GG_LOGE("Failed to decode shadow payload.");
        return GG_ERR_PARSE;
    }

    return GG_ERR_OK;
}

GgError ggipc_get_thing_shadow(
    GgBuffer thing_name, const GgBuffer *shadow_name, GgBuffer *payload
) {
    GgKVVec args = GG_KV_VEC((GgKV[2]) { 0 });
    (void) gg_kv_vec_push(
        &args, gg_kv(GG_STR("thingName"), gg_obj_buf(thing_name))
    );
    if (shadow_name != NULL) {
        (void) gg_kv_vec_push(
            &args, gg_kv(GG_STR("shadowName"), gg_obj_buf(*shadow_name))
        );
    }

    return ggipc_call(
        GG_STR("aws.greengrass#GetThingShadow"),
        GG_STR("aws.greengrass#GetThingShadowRequest"),
        args.map,
        &response_handler,
        &error_handler,
        payload
    );
}
