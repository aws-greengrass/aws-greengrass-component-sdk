// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef GG_ERROR_HPP
#define GG_ERROR_HPP

#include <string>
#include <system_error>
#include <type_traits>

//! GG error codes

extern "C" {
#include <gg/error.h>
}

namespace gg {
// for differentiation between standard error categories
const std::error_category &category() noexcept;
}

// Allow implicit conversion from GgError to std::error_code and comparisons
// between GgError and std::error_code
namespace std {
template <>
struct is_error_code_enum<GgError> : true_type { };
}

// implicit conversion uses make_error_code
inline std::error_code make_error_code(GgError gg_error_value) noexcept {
    return { static_cast<int>(gg_error_value), gg::category() };
}

namespace gg {
class Exception : public std::system_error {
public:
    Exception(GgError code)
        : std::system_error { code } {
    }

    Exception(GgError code, const std::string &what)
        : std::system_error { code, what } {
    }

    Exception(GgError code, const char *what)
        : std::system_error { code, what } {
    }
};

}

#endif
