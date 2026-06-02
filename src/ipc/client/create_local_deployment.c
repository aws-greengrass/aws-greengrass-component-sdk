// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include <assert.h>
#include <gg/arena.h>
#include <gg/buffer.h>
#include <gg/error.h>
#include <gg/ipc/client.h>
#include <gg/ipc/client_raw.h>
#include <gg/log.h>
#include <gg/map.h>
#include <gg/object.h>
#include <gg/vector.h>

static GgError error_handler(void *ctx, GgBuffer error_code, GgBuffer message) {
    (void) ctx;

    GG_LOGE(
        "Received CreateLocalDeployment error %.*s: %.*s.",
        (int) error_code.len,
        error_code.data,
        (int) message.len,
        message.data
    );

    if (gg_buffer_eq(error_code, GG_STR("InvalidArgumentsError"))) {
        return GG_ERR_INVALID;
    }
    if (gg_buffer_eq(error_code, GG_STR("InvalidRecipeDirectoryPathError"))) {
        return GG_ERR_INVALID;
    }
    if (gg_buffer_eq(
            error_code, GG_STR("InvalidArtifactsDirectoryPathError")
        )) {
        return GG_ERR_INVALID;
    }
    if (gg_buffer_eq(error_code, GG_STR("ServiceError"))) {
        return GG_ERR_FAILURE;
    }

    return GG_ERR_FAILURE;
}

static GgError response_handler(void *ctx, GgMap result) {
    GgBuffer *deployment_id = ctx;

    GgObject *value = NULL;
    GgError ret = gg_map_validate(
        result,
        GG_MAP_SCHEMA(
            { GG_STR("deploymentId"), GG_REQUIRED, GG_TYPE_BUF, &value }
        )
    );
    if (ret != GG_ERR_OK) {
        GG_LOGE("Failed validating CreateLocalDeployment response.");
        return GG_ERR_INVALID;
    }

    if (deployment_id != NULL) {
        GgBuffer id = gg_obj_into_buf(*value);
        GgArena alloc = gg_arena_init(*deployment_id);
        ret = gg_arena_claim_buf(&id, &alloc);
        if (ret != GG_ERR_OK) {
            GG_LOGE("Insufficient memory provided for deployment id.");
            return ret;
        }
        *deployment_id = id;
    }
    return GG_ERR_OK;
}

GgError ggipc_create_local_deployment(
    const GgCreateLocalDeploymentArgs *args, GgBuffer *deployment_id
) {
    GgKVVec kv_vec = GG_KV_VEC((GgKV[8]) { 0 });

    if (args->recipe_directory_path.data != NULL
        && args->recipe_directory_path.len > 0) {
        (void) gg_kv_vec_push(
            &kv_vec,
            gg_kv(
                GG_STR("recipeDirectoryPath"),
                gg_obj_buf(args->recipe_directory_path)
            )
        );
    }

    if (args->artifacts_directory_path.data != NULL
        && args->artifacts_directory_path.len > 0) {
        (void) gg_kv_vec_push(
            &kv_vec,
            gg_kv(
                GG_STR("artifactsDirectoryPath"),
                gg_obj_buf(args->artifacts_directory_path)
            )
        );
    }

    if (args->root_component_versions_to_add.len > 0) {
        (void) gg_kv_vec_push(
            &kv_vec,
            gg_kv(
                GG_STR("rootComponentVersionsToAdd"),
                gg_obj_map(args->root_component_versions_to_add)
            )
        );
    }

    if (args->root_components_to_remove.len > 0) {
        (void) gg_kv_vec_push(
            &kv_vec,
            gg_kv(
                GG_STR("rootComponentsToRemove"),
                gg_obj_list(args->root_components_to_remove)
            )
        );
    }

    if (args->component_to_configuration.len > 0) {
        (void) gg_kv_vec_push(
            &kv_vec,
            gg_kv(
                GG_STR("componentToConfiguration"),
                gg_obj_map(args->component_to_configuration)
            )
        );
    }

    if (args->component_to_run_with_info.len > 0) {
        (void) gg_kv_vec_push(
            &kv_vec,
            gg_kv(
                GG_STR("componentToRunWithInfo"),
                gg_obj_map(args->component_to_run_with_info)
            )
        );
    }

    if (args->group_name.data != NULL && args->group_name.len > 0) {
        (void) gg_kv_vec_push(
            &kv_vec, gg_kv(GG_STR("groupName"), gg_obj_buf(args->group_name))
        );
    }

    if (args->failure_handling_policy != GG_FAILURE_HANDLING_POLICY_NONE) {
        GgBuffer policy_str;
        switch (args->failure_handling_policy) {
        case GG_FAILURE_HANDLING_POLICY_ROLLBACK:
            policy_str = GG_STR("ROLLBACK");
            break;
        case GG_FAILURE_HANDLING_POLICY_DO_NOTHING:
            policy_str = GG_STR("DO_NOTHING");
            break;
        default:
            assert(false);
            return GG_ERR_INVALID;
        }
        (void) gg_kv_vec_push(
            &kv_vec,
            gg_kv(GG_STR("failureHandlingPolicy"), gg_obj_buf(policy_str))
        );
    }

    return ggipc_call(
        GG_STR("aws.greengrass#CreateLocalDeployment"),
        GG_STR("aws.greengrass#CreateLocalDeploymentRequest"),
        kv_vec.map,
        deployment_id != NULL ? &response_handler : NULL,
        &error_handler,
        deployment_id
    );
}
