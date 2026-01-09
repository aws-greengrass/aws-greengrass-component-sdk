// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include <gg/error.hpp>
#include <gg/object.hpp>
#include <cstdint>
#include <optional>
#include <system_error>

extern "C" {
#include <gg/object.h>
#include <gg/types.h>
}

namespace gg {

Object::Variant Object::to_variant() const noexcept {
    switch (index()) {
    case GG_TYPE_NULL:
        return {};
    case GG_TYPE_BOOLEAN:
        return gg_obj_into_bool(*this);
    case GG_TYPE_I64:
        return gg_obj_into_i64(*this);
    case GG_TYPE_F64:
        return gg_obj_into_f64(*this);
    case GG_TYPE_BUF: {
        GgBuffer buf = gg_obj_into_buf(*this);
        return std::span { buf.data, buf.len };
    }
    case GG_TYPE_LIST:
        return gg_obj_into_list(*this);
    case GG_TYPE_MAP:
        return gg_obj_into_map(*this);
    }
    abort();
}

Object::Object(bool boolean) noexcept
    : GgObject { gg_obj_bool(boolean) } {
}

Object::Object(int64_t i64) noexcept
    : GgObject { gg_obj_i64(i64) } {
}

Object::Object(double f64) noexcept
    : GgObject { gg_obj_f64(f64) } {
}

Object::Object(gg::Buffer buffer) noexcept
    : GgObject { gg_obj_buf(buffer) } {
}

Object::Object(List list) noexcept
    : GgObject { gg_obj_list(list) } {
}

Object::Object(Map map) noexcept
    : GgObject { gg_obj_map(map) } {
}

GgObjectType Object::index() const noexcept {
    return gg_obj_type(*this);
}

Object &Object::operator[](std::string_view key) const {
    if (index() != GG_TYPE_MAP) {
        GG_THROW_OR_ABORT(
            Exception { GG_ERR_PARSE, "gg::Object is not gg::Map" }
        );
    }

    return Map { gg_obj_into_map(*this) }[key];
}

Object &Object::operator[](std::size_t idx) const {
    if (index() != GG_TYPE_LIST) {
        GG_THROW_OR_ABORT(
            Exception { GG_ERR_PARSE, "gg::Object is not gg::List" }
        );
    }
    return List { gg_obj_into_list(*this) }[idx];
}

template <>
std::optional<std::monostate> get_if(Object obj) noexcept {
    if (gg_obj_type(obj) == GG_TYPE_NULL) {
        return std::monostate {};
    }
    return std::nullopt;
}

template <>
std::optional<bool> get_if(Object obj) noexcept {
    if (gg_obj_type(obj) == GG_TYPE_BOOLEAN) {
        return gg_obj_into_bool(obj);
    }
    return std::nullopt;
}

template <>
std::optional<int64_t> get_if(Object obj) noexcept {
    if (gg_obj_type(obj) == GG_TYPE_I64) {
        return gg_obj_into_i64(obj);
    }
    return std::nullopt;
}

template <>
std::optional<double> get_if(Object obj) noexcept {
    if (gg_obj_type(obj) == GG_TYPE_F64) {
        return gg_obj_into_f64(obj);
    }
    return std::nullopt;
}

template <>
std::optional<std::span<uint8_t>> get_if(Object obj) noexcept {
    if (gg_obj_type(obj) == GG_TYPE_BUF) {
        GgBuffer buf = gg_obj_into_buf(obj);
        return std::span { buf.data, buf.len };
    }
    return std::nullopt;
}

template <>
std::optional<List> get_if(Object obj) noexcept {
    if (gg_obj_type(obj) == GG_TYPE_LIST) {
        return gg_obj_into_list(obj);
    }
    return std::nullopt;
}

template <>
std::optional<Map> get_if(Object obj) noexcept {
    if (gg_obj_type(obj) == GG_TYPE_MAP) {
        return gg_obj_into_map(obj);
    }
    return std::nullopt;
}
}
