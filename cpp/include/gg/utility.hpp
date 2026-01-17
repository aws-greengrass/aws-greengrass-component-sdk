// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef GG_UTILITY_HPP
#define GG_UTILITY_HPP

#include <algorithm>
#include <limits>
#include <type_traits>

#ifdef __cpp_exceptions
#define GG_THROW_OR_ABORT(...) throw(__VA_ARGS__)
#else
#include <cstdlib>
#define GG_THROW_OR_ABORT(...) std::abort()
#endif

#endif
