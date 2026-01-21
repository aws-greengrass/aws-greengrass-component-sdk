// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "b64_encode_mem.h"
#include <gg/arena.h>
#include <gg/base64.h>
#include <gg/buffer.h>
#include <gg/cleanup.h>
#include <gg/error.h>
#include <gg/ipc/client.h>
#include <gg/ipc/client_raw.h>
#include <gg/log.h>
#include <gg/map.h>
#include <gg/object.h>
#include <string.h>
#include <stdint.h>

static GgError error_handler(void *ctx, GgBuffer error_code, GgBuffer message) {
    (void) ctx;

    GG_LOGE(
        "Received PublishToIoTCore error %.*s: %.*s.",
        (int) error_code.len,
        error_code.data,
        (int) message.len,
        message.data
    );

    if (gg_buffer_eq(error_code, GG_STR("UnauthorizedError"))) {
        return GG_ERR_UNSUPPORTED;
    }
    return GG_ERR_FAILURE;
}

GgError ggipc_publish_to_iot_core(
    GgBuffer topic_name, GgBuffer payload, uint8_t qos
) {
    GG_MTX_SCOPE_GUARD(&gg_ipc_b64_encode_mtx);
    GgArena arena = gg_arena_init(GG_BUF(gg_ipc_b64_encode_mem));

    GgBuffer b64_payload;
    GgError ret = gg_base64_encode(payload, &arena, &b64_payload);
    if (ret != GG_ERR_OK) {
        GG_LOGE(
            "Insufficient memory provided to base64 encode PublishToIoTCore payload (required %zu, available %" PRIu32
            ").",
            ((payload.len + 2) / 3) * 4,
            arena.capacity - arena.index
        );
        return ret;
    }

    GgBuffer qos_buffer = GG_BUF((uint8_t[1]) { qos + (uint8_t) '0' });
    GgMap args = GG_MAP(
        gg_kv(GG_STR("topicName"), gg_obj_buf(topic_name)),
        gg_kv(GG_STR("payload"), gg_obj_buf(b64_payload)),
        gg_kv(GG_STR("qos"), gg_obj_buf(qos_buffer))
    );

    return ggipc_call(
        GG_STR("aws.greengrass#PublishToIoTCore"),
        GG_STR("aws.greengrass#PublishToIoTCoreRequest"),
        args,
        NULL,
        &error_handler,
        NULL
    );
}
