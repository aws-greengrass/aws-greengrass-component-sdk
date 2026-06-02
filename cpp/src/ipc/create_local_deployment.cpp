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
    const CreateLocalDeploymentArgs &args,
    std::span<std::byte> deployment_id_mem,
    std::string_view *deployment_id
) noexcept {
    auto to_buf = [](std::string_view sv) -> GgBuffer {
        return {
            const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(sv.data())),
            sv.size()
        };
    };

    GgCreateLocalDeploymentArgs c_args = {
        .recipe_directory_path = to_buf(args.recipe_directory_path),
        .artifacts_directory_path = to_buf(args.artifacts_directory_path),
        .root_component_versions_to_add = args.root_component_versions_to_add,
        .root_components_to_remove = args.root_components_to_remove,
        .component_to_configuration = args.component_to_configuration,
        .component_to_run_with_info = args.component_to_run_with_info,
        .group_name = to_buf(args.group_name),
        .failure_handling_policy = args.failure_handling_policy,
    };

    GgBuffer id_buf = { reinterpret_cast<uint8_t *>(deployment_id_mem.data()),
                        deployment_id_mem.size() };
    auto err = ggipc_create_local_deployment(
        &c_args, deployment_id != nullptr ? &id_buf : nullptr
    );
    if ((deployment_id != nullptr) && (err == GG_ERR_OK)) {
        *deployment_id
            = { reinterpret_cast<const char *>(id_buf.data), id_buf.len };
    }
    return err;
}

}
