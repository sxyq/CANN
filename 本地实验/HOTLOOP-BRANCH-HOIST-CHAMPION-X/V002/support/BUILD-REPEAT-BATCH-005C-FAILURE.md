# Repeat-batch support build attempt 005c

```text
UTC=2026-10-03T08:08Z
SUPPORT_COMMIT=c8f10e8e
BUILD_RC=2
```

This retry added the GCC target include directory to `CPLUS_INCLUDE_PATH` in
an attempt to resolve the standard C++ headers. It reached `arm_neon.h` but
failed with incompatible `__Int*`/`__Float*` vector type errors from the Ascend
C compiler. No executable was used for timing. The complete log is
`build-005c.log`; the failed build directory remains intact.
