# ROW-OCC-H1 V001 Correctness Evidence

CORRECTNESS_STATUS=PASS
PASS_COUNT=7/7
DEVICE=4
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE_SHA=7757274fe51507a2f8bf599c5771a9f1e9b21dce68c66a0b2b97934d7f4d1315
PARENT_EXE_SHA=815fd5ce4f524f98e9474e496f9b914f1e9ce176be33e93e108f11db1a58c1df
CANDIDATE_EXE_SHA=e56cc2bb020136a695c1aba17ca3a9e71dc383cd4b8653fc4a43527ecb333813
HARNESS_SHA=2aa1aa3b2801a9202de01946ff079ec3b580c6ee7218dc03a772a87a4a4467e7

The exact same canonical reference runner and input generator were used for
parent and candidate. Every cell returned `RC=0` and `bad=0` for both sides:

```text
C12  8x8192   BF16
C13  128x16384 FP16
C14  128x16384 BF16
C16  9x32768  BF16
C01  1x128    FP16
C08  8x2048   BF16
C11  1x8192   FP32
```

The full raw output, device stats, and host/NPU snapshots are retained under
`correctness/`. Existing processes were observed only; none was killed,
paused, or migrated.
