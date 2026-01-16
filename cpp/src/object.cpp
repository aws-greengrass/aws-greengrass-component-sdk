// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include <gg/error.hpp>
#include <gg/object.hpp>
#include <system_error>

namespace gg {
Object Object::operator[](std::string_view key) const {
    if (index() != GG_TYPE_MAP) {
        GG_THROW_OR_ABORT(
            Exception { GG_ERR_PARSE, "gg::Object is not gg::Map" }
        );
    }

    return *Map { gg_obj_into_map(*this) }[key];
}

Object Object::operator[](std::size_t idx) const {
    if (index() != GG_TYPE_LIST) {
        GG_THROW_OR_ABORT(
            Exception { GG_ERR_PARSE, "gg::Object is not gg::List" }
        );
    }
    return List { gg_obj_into_list(*this) }[idx];
}

}
