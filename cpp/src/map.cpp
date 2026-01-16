// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include <gg/buffer.hpp>
#include <gg/map.hpp>
#include <gg/object.hpp>

extern "C" {
#include <gg/map.h>
}

namespace gg {

std::string_view KV::key() const noexcept {
    GgBuffer key = gg_kv_key(*this);
    return std::string_view { reinterpret_cast<char *>(key.data), key.len };
}

Object *KV::value() noexcept {
    return static_cast<Object *>(gg_kv_val(this));
}

KV::KV(Buffer key, const Object &value) noexcept
    : GgKV { gg_kv(key, value) } {
}

void KV::key(std::string_view str) noexcept {
    gg_kv_set_key(this, Buffer { str });
}

void KV::key(std::span<uint8_t> key) noexcept {
    gg_kv_set_key(this, Buffer { key });
}

void KV::value(GgObject obj) noexcept {
    *gg_kv_val(this) = obj;
}

}
