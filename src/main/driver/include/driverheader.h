#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Demo entry point exported from the driver library.
 *
 * This is a placeholder that exercises the JNI load path and verifies that the
 * driver shared library is correctly linked and loaded at runtime.
 */
void c_doThing();

#ifdef __cplusplus
}  // extern "C"
#endif
