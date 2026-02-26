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
    GgList *results;
    GgBuffer *next_token;
} ListShadowsCtx;

static GgError response_handler(void *ctx, GgMap result) {
    ListShadowsCtx *list_ctx = ctx;

    GgObject *results_obj;
    GgObject *next_token_obj;
    GgError ret = gg_map_validate(
        result,
        GG_MAP_SCHEMA(
            { GG_STR("results"), GG_REQUIRED, GG_TYPE_LIST, &results_obj },
            { GG_STR("nextToken"), GG_OPTIONAL, GG_TYPE_BUF, &next_token_obj }
        )
    );
    if (ret != GG_ERR_OK) {
        GG_LOGE("Failed validating ListNamedShadowsForThing response.");
        return GG_ERR_INVALID;
    }

    *list_ctx->results = gg_obj_into_list(*results_obj);

    if (next_token_obj != NULL && list_ctx->next_token != NULL) {
        *list_ctx->next_token = gg_obj_into_buf(*next_token_obj);
    }

    return GG_ERR_OK;
}

GgError ggipc_list_named_shadows_for_thing(
    GgBuffer thing_name,
    GgBuffer page_token,
    GgList *results,
    GgBuffer *next_token
) {
    GgMap args;
    if (page_token.len > 0) {
        args = GG_MAP(
            gg_kv(GG_STR("thingName"), gg_obj_buf(thing_name)),
            gg_kv(GG_STR("nextToken"), gg_obj_buf(page_token))
        );
    } else {
        args = GG_MAP(gg_kv(GG_STR("thingName"), gg_obj_buf(thing_name)));
    }

    ListShadowsCtx list_ctx = { .results = results, .next_token = next_token };

    return ggipc_call(
        GG_STR("aws.greengrass#ListNamedShadowsForThing"),
        GG_STR("aws.greengrass#ListNamedShadowsForThingRequest"),
        args,
        &response_handler,
        &error_handler,
        &list_ctx
    );
}
