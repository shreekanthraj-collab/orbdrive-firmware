/**
 * @file nfw_assert.h
 * @brief Orb Drive Framework assertion support.
 */

#ifndef NFW_ASSERT_H
#define NFW_ASSERT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief Handle a framework assertion failure.
 *
 * @param expr Assertion expression.
 * @param file Source file where the assertion failed.
 * @param line Source line where the assertion failed.
 */
void nfwAssertFailure(
    const char *expr,
    const char *file,
    uint32_t line
);

#ifdef __cplusplus
}
#endif

#endif /* NFW_ASSERT_H */
