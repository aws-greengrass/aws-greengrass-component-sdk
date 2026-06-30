// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef GG_LOG_TRAIL_H
#define GG_LOG_TRAIL_H

#ifdef GG_LOG_TRAIL_ENABLED

#include <gg/attr.h>
#include <gg/eventstream/decode.h>
#include <gg/eventstream/types.h>
#include <gg/log.h>
#include <gg/macro_util.h>
#include <stdbool.h>
#include <stddef.h>

/// Start a root trace if none is active on this thread (idempotent).
/// Generates fresh trace_id and span_id, sets TLS, emits one INFO log line.
FORMAT(printf, 2, 3)
void gg_log_trail_root_begin(const char *kind, const char *fmt, ...);

/// Attach T/S/P trace headers to an outbound EventStream frame.
/// Returns 3 on success, 0 if no trace active or headers_capacity < 3.
size_t gg_log_trail_attach_headers(
    EventStreamHeader *headers, size_t headers_capacity
);

/// Extract T/S/P from inbound header iterator and set TLS for the new span.
/// Returns true if trace context was found and applied; false otherwise.
bool gg_log_trail_extract_and_apply(EventStreamHeaderIter headers);

// Internal: cleanup callback for GG_LOG_TRAIL_SCOPE_GUARD.
static inline void gg_log_trail_scope_clear_(const int *guard) {
    (void) guard;
    gg_log_clear_trail();
}

/// Clear the thread trace context automatically when the enclosing scope exits.
/// Place after gg_log_trail_root_begin / gg_log_trail_extract_and_apply so the
/// trace is cleared on every normal return path (replaces a manual
/// gg_log_clear_trail()).
#define GG_LOG_TRAIL_SCOPE_GUARD() \
    __attribute__((cleanup(gg_log_trail_scope_clear_))) const int \
    GG_MACRO_PASTE(gg_log_trail_guard_, __LINE__) \
        = 0

/// Begin a root trace and clear it automatically on scope exit.
/// Forwards all args to gg_log_trail_root_begin(kind, fmt, ...).
/// NOTE: declares a scope-guard variable, so it must be used inside a brace
/// block.
#define GG_LOG_TRAIL_ROOT_SCOPE(...) \
    gg_log_clear_trail(); \
    gg_log_trail_root_begin(__VA_ARGS__); \
    GG_LOG_TRAIL_SCOPE_GUARD()

/// Inherit trace context from inbound EventStream headers and clear it on scope
/// exit. NOTE: declares a scope-guard variable, so it must be used inside a
/// brace block.
#define GG_LOG_TRAIL_INHERIT_SCOPE(headers) \
    gg_log_clear_trail(); \
    gg_log_trail_extract_and_apply(headers); \
    GG_LOG_TRAIL_SCOPE_GUARD()

#else

#define GG_LOG_TRAIL_SCOPE_GUARD()
#define GG_LOG_TRAIL_ROOT_SCOPE(...)
#define GG_LOG_TRAIL_INHERIT_SCOPE(headers)

#endif // GG_LOG_TRAIL_ENABLED

#endif
