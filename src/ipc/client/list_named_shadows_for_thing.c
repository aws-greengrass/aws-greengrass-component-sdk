// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include <gg/buffer.h>
#include <gg/error.h>
#include <gg/ipc/client.h>
#include <gg/ipc/client_raw.h>
#include <gg/list.h>
#include <gg/log.h>
#include <gg/map.h>
#include <gg/object.h>
#include <gg/vector.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>

static GgError error_handler(void *ctx, GgBuffer error_code, GgBuffer message) {
    (void) ctx;

    GG_LOGE(
        "Received ListNamedShadowsForThing error %.*s: %.*s.",
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

typedef struct {
    GgIpcListNamedShadowsCallback *callback;
    void *ctx;
    uint8_t next_token_buf[256];
    GgBuffer next_token;
    bool has_next;
} ListShadowsCtx;

static GgError response_handler(void *ctx, GgMap result) {
    ListShadowsCtx *list_ctx = ctx;

    GgObject *results_obj;
    GgObject *next_token_obj;
    GgError ret = gg_map_validate(
        result,
        GG_MAP_SCHEMA(
            { GG_STR("results"), GG_REQUIRED, GG_TYPE_LIST, &results_obj },
            { GG_STR("nextToken"), GG_OPTIONAL, GG_TYPE_BUF, &next_token_obj },
            { GG_STR("timestamp"), GG_OPTIONAL, GG_TYPE_F64, NULL }
        )
    );
    if (ret != GG_ERR_OK) {
        GG_LOGE("Failed validating ListNamedShadowsForThing response.");
        return GG_ERR_INVALID;
    }

    // Call callback for each shadow name
    GgList list = gg_obj_into_list(*results_obj);
    GG_LIST_FOREACH (item, list) {
        if (gg_obj_type(*item) == GG_TYPE_BUF) {
            list_ctx->callback(list_ctx->ctx, gg_obj_into_buf(*item));
        }
    }

    // Handle nextToken for pagination
    if (next_token_obj != NULL) {
        GgBuffer token = gg_obj_into_buf(*next_token_obj);
        if (token.len > 0) {
            if (token.len > sizeof(list_ctx->next_token_buf)) {
                GG_LOGW(
                    "NextToken length %zu exceeds buffer, stopping pagination.",
                    (size_t) token.len
                );
                list_ctx->has_next = false;
            } else {
                memcpy(list_ctx->next_token_buf, token.data, token.len);
                list_ctx->next_token
                    = (GgBuffer) { .data = list_ctx->next_token_buf,
                                   .len = token.len };
                list_ctx->has_next = true;
            }
        } else {
            list_ctx->has_next = false;
        }
    } else {
        list_ctx->has_next = false;
    }

    return GG_ERR_OK;
}

GgError ggipc_list_named_shadows_for_thing(
    GgBuffer thing_name,
    uint32_t page_size,
    GgIpcListNamedShadowsCallback *callback,
    void *ctx
) {
    ListShadowsCtx list_ctx = {
        .callback = callback,
        .ctx = ctx,
        .next_token = { 0 },
        .has_next = false,
    };

    do {
        list_ctx.has_next = false;

        GgKVVec args = GG_KV_VEC((GgKV[3]) { 0 });

        // Required argument: thingName
        (void) gg_kv_vec_push(
            &args, gg_kv(GG_STR("thingName"), gg_obj_buf(thing_name))
        );

        // Optional: nextToken for pagination
        if (list_ctx.next_token.len > 0) {
            (void) gg_kv_vec_push(
                &args,
                gg_kv(GG_STR("nextToken"), gg_obj_buf(list_ctx.next_token))
            );
        }

        // Optional: pageSize
        if (page_size > 0) {
            (void) gg_kv_vec_push(
                &args,
                gg_kv(GG_STR("pageSize"), gg_obj_i64((int64_t) page_size))
            );
        }

        GgError ret = ggipc_call(
            GG_STR("aws.greengrass#ListNamedShadowsForThing"),
            GG_STR("aws.greengrass#ListNamedShadowsForThingRequest"),
            args.map,
            &response_handler,
            &error_handler,
            &list_ctx
        );

        if (ret != GG_ERR_OK) {
            return ret;
        }
    } while (list_ctx.has_next);

    return GG_ERR_OK;
}
