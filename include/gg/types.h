// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef GG_TYPES_H
#define GG_TYPES_H

//! Public types used by the SDK.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/// A generic object.
typedef struct {
    // Used only with memcpy so no aliasing with contents
    uint8_t _private[(sizeof(void *) == 4) ? 9 : 11];
} GgObject;

/// Type tag for `GgObject`.
typedef enum {
    GG_TYPE_NULL = 0,
    GG_TYPE_BOOLEAN,
    GG_TYPE_I64,
    GG_TYPE_F64,
    GG_TYPE_BUF,
    GG_TYPE_LIST,
    GG_TYPE_MAP,
} GgObjectType;

/// An array of `GgObject`.
typedef struct {
    GgObject *items;
    size_t len;
} GgList;

/// A key-value pair used for `GgMap`.
/// `key` must be an UTF-8 encoded string.
typedef struct {
    // KVs alias with pointers to their value objects
    uint8_t _private[sizeof(void *) + 2 + sizeof(GgObject)];
} GgKV;

/// A map of UTF-8 strings to `GgObject`s.
typedef struct {
    GgKV *pairs;
    size_t len;
} GgMap;

/// A fixed buffer of bytes. Possibly a string.
typedef struct {
    uint8_t *data;
    size_t len;
} GgBuffer;

/// An array of `GgBuffer`.
typedef struct {
    GgBuffer *bufs;
    size_t len;
} GgBufList;

#endif
