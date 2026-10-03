# Repeat-batch support build attempt 005

```text
UTC=2026-10-03T07:58Z
SUPPORT_COMMIT=c8f10e8e
RUNNER_SOURCE_SHA256=c3cd60fbae5956ceed65d561ec2f77a1fef8641626a105479ffc24877b21358e
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
CMAKE_RC=0
BUILD_RC=2
```

The first support-only build configured successfully but inherited
`ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/latest` while the compiler
was selected from `8.5.0.alpha002`; Ascend C then failed to find `<cstdint>` in
both wrapper translations. No executable was used for measurement. The full
log is `build-005.log`; the failed build directory is retained and is not
cleaned.

The retry changes only the build environment to pin both CMake and the loader
to `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`.
