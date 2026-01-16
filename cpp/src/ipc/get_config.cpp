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
#include <gg/arena.h>
}

namespace gg::ipc {

std::error_code Client::get_config(
    std::span<const Buffer> key_path,
    std::optional<std::string_view> component_name,
    std::span<std::byte> value_mem,
    std::string_view &value
) noexcept {
    Buffer comp_name_buf { component_name.value_or(std::string_view {}) };
    GgBuffer result = { .data = reinterpret_cast<uint8_t *>(value_mem.data()),
                        .len = value_mem.size() };
    GgError ret = ggipc_get_config_str(
        { .bufs = const_cast<Buffer *>(key_path.data()),
          .len = key_path.size() },
        component_name.has_value() ? &comp_name_buf : nullptr,
        &result
    );
    if (ret != GgError::GG_ERR_OK) {
        return ret;
    }

    value = std::string_view { reinterpret_cast<char *>(result.data),
                               result.len };
    return {};
}

std::error_code Client::get_config(
    std::span<const Buffer> key_path,
    std::optional<std::string_view> component_name,
    std::span<std::byte> value_mem,
    Object &value
) noexcept {
    Buffer comp_name_buf { component_name.value_or(std::string_view {}) };
    GgArena arena = gg_arena_init(
        { .data = reinterpret_cast<uint8_t *>(value_mem.data()),
          .len = value_mem.size() }
    );
    GgObject result;
    GgError ret = ggipc_get_config(
        { .bufs = const_cast<Buffer *>(key_path.data()),
          .len = key_path.size() },
        component_name.has_value() ? &comp_name_buf : nullptr,
        &arena,
        &result
    );
    if (ret != GgError::GG_ERR_OK) {
        return ret;
    }

    value = result;
    return {};
}

namespace {

    template <typename T>
    std::error_code get_config_scalar(
        Client &client,
        std::span<const Buffer> key_path,
        std::optional<std::string_view> component_name,
        T &value
    ) noexcept {
        std::byte mem[sizeof(Object)];
        Object result;
        std::error_code ret = client.get_config(
            key_path, component_name, std::span(mem), result
        );
        if (ret) {
            return ret;
        }
        auto var = result.to_variant();
        auto *val = std::get_if<T>(&var);
        if (val == nullptr) {
            return GgError::GG_ERR_INVALID;
        }
        value = *val;
        return {};
    }

}

std::error_code Client::get_config(
    std::span<const Buffer> key_path,
    std::optional<std::string_view> component_name,
    std::int64_t &value
) noexcept {
    return get_config_scalar(*this, key_path, component_name, value);
}

std::error_code Client::get_config(
    std::span<const Buffer> key_path,
    std::optional<std::string_view> component_name,
    double &value
) noexcept {
    return get_config_scalar(*this, key_path, component_name, value);
}

std::error_code Client::get_config(
    std::span<const Buffer> key_path,
    std::optional<std::string_view> component_name,
    bool &value
) noexcept {
    return get_config_scalar(*this, key_path, component_name, value);
}

}
