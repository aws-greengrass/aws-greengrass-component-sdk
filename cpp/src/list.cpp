// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include <gg/error.hpp>
#include <gg/list.hpp>
#include <gg/object.hpp>
#include <system_error>

extern "C" {
#include <gg/list.h>
}

namespace gg {

List::reference List::at(size_type pos) const {
    if (pos >= size()) {
        GG_THROW_OR_ABORT(
            gg::Exception(GG_ERR_RANGE, "gg::List::at: out of range")
        );
    }

    return operator[](pos);
}

List::reference List::operator[](size_type pos) const noexcept {
    return data()[pos];
}

List::pointer List::data() const noexcept {
    return static_cast<Object *>(items);
}

std::error_code List::type_check(GgObjectType type) const noexcept {
    return gg_list_type_check(*this, type);
}

}
