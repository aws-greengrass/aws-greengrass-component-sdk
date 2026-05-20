// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include <gg/ipc/client.hpp>
#include <cstddef>
#include <span>
#include <string_view>
#include <system_error>

extern "C" {
#include <gg/ipc/client.h>
}

namespace gg::ipc {

std::error_code Client::create_local_deployment(
    const GgCreateLocalDeploymentArgs &args,
    std::span<std::byte> deployment_id_mem,
    std::string_view *deployment_id
) noexcept {
    GgBuffer id_buf = { reinterpret_cast<uint8_t *>(deployment_id_mem.data()),
                        deployment_id_mem.size() };
    auto err = ggipc_create_local_deployment(
        &args, deployment_id != nullptr ? &id_buf : nullptr
    );
    if ((deployment_id != nullptr) && (err == GG_ERR_OK)) {
        *deployment_id
            = { reinterpret_cast<const char *>(id_buf.data), id_buf.len };
    }
    return err;
}

}
