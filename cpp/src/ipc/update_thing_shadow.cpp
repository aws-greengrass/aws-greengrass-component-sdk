// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include <gg/buffer.hpp>
#include <gg/error.hpp>
#include <gg/ipc/client.hpp>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <system_error>

extern "C" {
#include <gg/ipc/client.h>
}

namespace gg::ipc {

std::error_code Client::update_thing_shadow(
    std::string_view thing_name,
    std::optional<std::string_view> shadow_name,
    std::string_view payload,
    std::span<std::byte> response_mem,
    std::string_view *response
) noexcept {
    Buffer shadow_buf { shadow_name.value_or(std::string_view {}) };
    GgBuffer resp = { .data = reinterpret_cast<uint8_t *>(response_mem.data()),
                      .len = response_mem.size() };
    GgError ret = ggipc_update_thing_shadow(
        Buffer { thing_name },
        shadow_name.has_value() ? &shadow_buf : nullptr,
        Buffer { payload },
        response != nullptr ? &resp : nullptr
    );
    if (ret != GgError::GG_ERR_OK) {
        return ret;
    }
    if (response != nullptr) {
        *response = std::string_view { reinterpret_cast<char *>(resp.data),
                                       resp.len };
    }
    return {};
}

}
