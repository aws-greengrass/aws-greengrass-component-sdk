// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef GG_IPC_CLIENT_GET_CONFIG_COMMON_H
#define GG_IPC_CLIENT_GET_CONFIG_COMMON_H

#include <gg/attr.h>
#include <gg/buffer.h>
#include <gg/error.h>
#include <gg/ipc/client_raw.h>
#include <gg/object.h>
#include <gg/types.h>

/// Shared implementation for GetConfiguration IPC calls.
/// Sends the request and invokes the result_callback with the parsed response.
NONNULL(3) VISIBILITY(hidden)
GgError ggipc_get_config_common(
    GgBufList key_path,
    const GgBuffer *component_name,
    GgIpcResultCallback *result_callback,
    void *result_ctx
);

/// Extract the config value from the IPC response map.
/// If the response contains a single key matching final_key with a non-map
/// value, returns that value directly (classic behavior).
VISIBILITY(hidden)
GgError ggipc_get_config_resp_value(
    GgMap resp, GgObject **value, GgBuffer *final_key
);

#endif
