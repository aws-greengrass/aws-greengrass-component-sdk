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

/// Snapshot of a thread's trail context. Returned by the subspan begin call
/// and consumed by the scope-exit cleanup to restore the caller's context.
typedef struct {
    uint16_t trace_id;
    uint16_t span_id;
    uint16_t parent_span_id;
} GgLogTrailState;

/// Begin a new sub-span within the current trace and log a subspan_start line.
/// Preserves trace_id, generates a fresh span_id, and makes the caller's
/// span_id the new parent_span_id. Returns the caller's previous state so the
/// scope-cleanup can restore it. No-op if no trace is active.
GgLogTrailState gg_log_trail_subspan_begin(const char *kind);

// Internal: cleanup callback for GG_LOG_TRAIL_SUBSPAN_SCOPE.
static inline void gg_log_trail_subspan_end_(const GgLogTrailState *saved) {
    gg_log_set_trail(saved->trace_id, saved->span_id, saved->parent_span_id);
}

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

/// Begin a sub-span keyed off GG_MODULE and restore the parent context on
/// scope exit. trace_id is preserved; a fresh span_id is generated; the
/// previous span_id becomes parent_span_id. No-op if no trace is active.
/// NOTE: declares a scope-guard variable, so it must be used inside a brace
/// block.
#define GG_LOG_TRAIL_SUBSPAN_SCOPE() \
    __attribute__((cleanup(gg_log_trail_subspan_end_))) const GgLogTrailState \
    GG_MACRO_PASTE(gg_log_trail_subspan_, __LINE__) \
        = gg_log_trail_subspan_begin(GG_MODULE)

#else

#define GG_LOG_TRAIL_SCOPE_GUARD()
#define GG_LOG_TRAIL_ROOT_SCOPE(...)
#define GG_LOG_TRAIL_INHERIT_SCOPE(headers)
#define GG_LOG_TRAIL_SUBSPAN_SCOPE()

#endif // GG_LOG_TRAIL_ENABLED

#endif
