# HBH-09 V002 — restore original N=1 measurement runner

```text
RESTORE_COMMIT=b95b9bc5
REVERTED_SUPPORT_COMMIT=c8f10e8e
RUNNER_SOURCE_SHA256=86d6ad6df82ec7b2c6b414c2759a72a48567cf8f11a382a7fdcd003f45a955c0
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
REPEAT_BATCH_N=1 (fixed original behavior)
```

The N=8 support calibration failed to improve both same-binary MAD/median and
block drift. Its evidence remains under Window I. The support-only change was
explicitly reverted; this commit records the restored source identity. No
Candidate Kernel or measurement-result files were rewritten.
