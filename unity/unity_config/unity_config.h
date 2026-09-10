#ifndef GG_UNITY_CONFIG_H
#define GG_UNITY_CONFIG_H

#include "unity_handlers.h" // IWYU pragma: export

#define UNITY_TEST_PROTECT() gg_test_protect()
#define UNITY_TEST_ABORT() gg_test_abort()

#define UNITY_INCLUDE_DOUBLE
#define UNITY_INCLUDE_FLOAT

// Enable 64-bit assertion support unconditionally. Unity only auto-enables
// 64-bit support (and thus defines UNITY_DISPLAY_STYLE_INT64 and
// UNITY_DISPLAY_STYLE_UINT64) when it detects a 64-bit target. The gg_test
// helpers reference those display styles (e.g. gg_test_assert_int64_equal,
// and the size_t length comparisons in equal.c), so they must exist on
// 32-bit targets too. Without this, 32-bit ARM builds fail to compile with
// "'UNITY_DISPLAY_STYLE_INT64' undeclared".
#define UNITY_SUPPORT_64

#endif
