// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef GG_OBJECT_HPP
#define GG_OBJECT_HPP

#include <gg/buffer.hpp>
#include <gg/error.hpp>
#include <gg/list.hpp>
#include <gg/map.hpp>
#include <gg/utility.hpp>
#include <cstdint>
#include <cstdlib>
#include <concepts>
#include <optional>
#include <span>
#include <string_view>
#include <type_traits>
#include <variant>

extern "C" {
#include <gg/types.h>
}

namespace gg {

template <class T>
using is_object_alternative = std::disjunction<
    std::is_same<T, std::monostate>,
    std::is_same<T, bool>,
    std::is_integral<T>,
    std::is_floating_point<T>,
    std::is_same<T, std::span<uint8_t>>,
    std::is_same<std::string_view, T>,
    std::is_same<T, List>,
    std::is_same<T, Map>>;

template <class T>
constexpr bool is_object_alternative_v = is_object_alternative<T>::value;

class [[nodiscard]] Object : public GgObject {
public:
    using Variant = ::std::variant<
        ::std::monostate,
        bool,
        ::std::int64_t,
        double,
        ::std::span<uint8_t>,
        ::gg::List,
        ::gg::Map>;

    constexpr Object() noexcept = default;

    constexpr Object(GgObject obj) noexcept
        : GgObject { obj } {
    }

    static Object to_object(const Variant &variant) {
        if (variant.valueless_by_exception()) {
            return {};
        }
        return std::visit(
            [](auto &&value) noexcept -> Object { return Object { value }; },
            variant
        );
    }

    Variant to_variant() const noexcept;

    explicit Object(const Variant &variant) noexcept
        : Object { to_object(variant) } {
    }

    constexpr Object(const Object &) noexcept = default;
    constexpr Object &operator=(const Object &) noexcept = default;

    Object(std::monostate /*unused*/) noexcept { };

    explicit Object(bool boolean) noexcept;

    Object(std::integral auto i64) noexcept
        requires(!std::is_same_v<bool, std::remove_cv_t<decltype(i64)>>)
        : Object { static_cast<int64_t>(i64) } {
    }

    Object(int64_t i64) noexcept;

    Object(std::floating_point auto f64) noexcept
        : Object { static_cast<double>(f64) } {
    }

    Object(double f64) noexcept;

    Object(gg::Buffer buffer) noexcept;

    Object(std::span<uint8_t> buffer) noexcept
        : Object { Buffer { buffer } } {
    }

    Object(std::string_view str) noexcept
        : Object { Buffer { str } } {
    }

    template <size_t N>
    Object(const char (&arr)[N]) noexcept
        : Object { std::string_view { arr } } {
    }

    Object(List list) noexcept;

    Object(Map map) noexcept;

    template <class T>
    Object &operator=(const T &value) noexcept
        requires(std::is_constructible_v<Object, T>)
    {
        return *this = Object { value };
    }

    GgObjectType index() const noexcept;

    // Assumes Object is a Map
    Object &operator[](std::string_view key) const;

    // Assumes Object is a List
    Object &operator[](std::size_t idx) const;
};

template <class T>
concept ObjectType = std::conjunction_v<
    std::negation<std::is_pointer<T>>,
    std::is_constructible<Object, T>>;

template <class T>
using is_buffer_type = std::disjunction<
    std::is_same<T, Buffer>,
    std::is_same<T, std::span<uint8_t>>,
    std::is_same<T, std::string_view>>;

template <ObjectType T>
std::optional<T> get_if(Object obj) noexcept {
    using Type = std::remove_cvref_t<T>;
    if constexpr (std::is_integral_v<Type>) {
        return get_if<int64_t>(obj);
    } else if constexpr (std::is_floating_point_v<Type>) {
        return get_if<double>(obj);
    } else if constexpr (is_buffer_type<Type>::value) {
        if (auto buf = get_if<std::span<uint8_t>>(obj); buf.has_value()) {
            return T {
                reinterpret_cast<typename T::pointer>(buf.value().data()),
                buf.value().size()
            };
        }
        return std::nullopt;
    } else {
        static_assert(false, "should be unreachable");
    }
}

template <>
std::optional<std::monostate> get_if(Object obj) noexcept;
template <>
std::optional<bool> get_if(Object obj) noexcept;
template <>
std::optional<int64_t> get_if(Object obj) noexcept;
template <>
std::optional<double> get_if(Object obj) noexcept;
template <>
std::optional<std::span<uint8_t>> get_if(Object obj) noexcept;
template <>
std::optional<List> get_if(Object obj) noexcept;
template <>
std::optional<Map> get_if(Object obj) noexcept;

template <ObjectType T>
T get(Object obj) {
    auto value = get_if<T>(obj);
    if (!value) {
        GG_THROW_OR_ABORT(
            Exception { GG_ERR_INVALID, "get: Bad index for object." }
        );
    }
    return *value;
}

constexpr Object nullobj {};

static_assert(
    std::is_standard_layout_v<gg::Object>,
    "gg::Object must not declare any virtual functions or data members."
);
}

#endif
