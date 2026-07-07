// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "get_config_common.h"
#include <gg/buffer.h>
#include <gg/error.h>
#include <gg/io.h>
#include <gg/ipc/client_priv.h>
#include <gg/json_encode.h>
#include <gg/log.h>
#include <gg/object.h>
#include <gg/types.h>

typedef struct {
    GgObjectReceiver receiver;
    GgBuffer *final_key;
} ForwardConfigCtx;

static GgError forward_config_object(void *ctx, GgMap result) {
    ForwardConfigCtx *fwd_ctx = ctx;

    GgObject *value;
    GgError ret
        = ggipc_get_config_resp_value(result, &value, fwd_ctx->final_key);
    if (ret != GG_ERR_OK) {
        return GG_ERR_INVALID;
    }

    return gg_object_receiver_submit(fwd_ctx->receiver, *value);
}

GgError ggipc_get_config_receive(
    GgBufList key_path,
    const GgBuffer *component_name,
    GgObjectReceiver receiver
) {
    ForwardConfigCtx write_ctx = {
        .receiver = receiver,
        .final_key
        = (key_path.len == 0) ? NULL : &key_path.bufs[key_path.len - 1],
    };

    return ggipc_get_config_common(
        key_path, component_name, &forward_config_object, &write_ctx
    );
}
