// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include <gg/arena.h>
#include <gg/buffer.h>
#include <gg/error.h>
#include <gg/ipc/client.h>
#include <gg/ipc/client_raw.h>
#include <gg/log.h>
#include <gg/map.h>
#include <gg/object.h>

static GgError error_handler(void *ctx, GgBuffer error_code, GgBuffer message) {
    (void) ctx;
    GG_LOGE(
        "Received CreateLocalDeployment error %.*s: %.*s.",
        (int) error_code.len,
        error_code.data,
        (int) message.len,
        message.data
    );
    return GG_ERR_FAILURE;
}

typedef struct {
    GgBuffer *value;
} CopyDeploymentIdCtx;

static GgError copy_deployment_id(void *ctx, GgMap result) {
    CopyDeploymentIdCtx *resp_ctx = ctx;

    GgObject *value = NULL;
    GgError ret = gg_map_validate(
        result,
        GG_MAP_SCHEMA(
            {
                GG_STR("deploymentId"),
                GG_REQUIRED,
                GG_TYPE_BUF,
                &value,
            }
        )
    );
    if (ret != GG_ERR_OK) {
        GG_LOGE("Failed validating CreateLocalDeployment response.");
        return GG_ERR_INVALID;
    }

    if (resp_ctx->value != NULL) {
        GgBuffer deployment_id = gg_obj_into_buf(*value);
        GgArena alloc = gg_arena_init(*resp_ctx->value);
        ret = gg_arena_claim_buf(&deployment_id, &alloc);
        if (ret != GG_ERR_OK) {
            GG_LOGE("Insufficient memory provided for deployment id.");
            return ret;
        }
        *resp_ctx->value = deployment_id;
    }
    return GG_ERR_OK;
}

GgError ggipc_create_local_deployment(
    const GgCreateLocalDeploymentArgs *args, GgBuffer *deployment_id
) {
    GgKV kv_entries[6];
    size_t kv_count = 0;

    if (args->component_to_configuration.len > 0) {
        kv_entries[kv_count++] = gg_kv(
            GG_STR("componentToConfiguration"),
            gg_obj_map(args->component_to_configuration)
        );
    }

    if (args->root_component_versions_to_add.len > 0) {
        kv_entries[kv_count++] = gg_kv(
            GG_STR("rootComponentVersionsToAdd"),
            gg_obj_map(args->root_component_versions_to_add)
        );
    }

    if (args->root_components_to_remove.len > 0) {
        kv_entries[kv_count++] = gg_kv(
            GG_STR("rootComponentsToRemove"),
            gg_obj_list(args->root_components_to_remove)
        );
    }

    if (args->recipe_directory_path.data != NULL
        && args->recipe_directory_path.len > 0) {
        kv_entries[kv_count++] = gg_kv(
            GG_STR("recipeDirectoryPath"),
            gg_obj_buf(args->recipe_directory_path)
        );
    }

    if (args->artifacts_directory_path.data != NULL
        && args->artifacts_directory_path.len > 0) {
        kv_entries[kv_count++] = gg_kv(
            GG_STR("artifactsDirectoryPath"),
            gg_obj_buf(args->artifacts_directory_path)
        );
    }

    if (args->failure_handling_policy.data != NULL
        && args->failure_handling_policy.len > 0) {
        kv_entries[kv_count++] = gg_kv(
            GG_STR("failureHandlingPolicy"),
            gg_obj_buf(args->failure_handling_policy)
        );
    }

    GgMap args_map = { .pairs = kv_entries, .len = kv_count };

    if (deployment_id != NULL) {
        CopyDeploymentIdCtx resp_ctx = { .value = deployment_id };
        return ggipc_call(
            GG_STR("aws.greengrass#CreateLocalDeployment"),
            GG_STR("aws.greengrass#CreateLocalDeploymentRequest"),
            args_map,
            &copy_deployment_id,
            &error_handler,
            &resp_ctx
        );
    }

    return ggipc_call(
        GG_STR("aws.greengrass#CreateLocalDeployment"),
        GG_STR("aws.greengrass#CreateLocalDeploymentRequest"),
        args_map,
        NULL,
        &error_handler,
        NULL
    );
}
