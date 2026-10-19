# N01 V008 Handoff

V008 is committed as Candidate source `f03947035feab6674a1a558becc42475cb3cf788`, Source SHA256 `51bc4acda94997c76fdf5926371b120e5dd1efc0e955a95b6441f0a2089a3463`. Its only source change narrows the Pass 1 `xFp32` LocalTensor view to the active FP16/BF16 branch inside the tile loop.

- CANN 8.5 configure/build: PASS, exit `0`.
- Correctness: PASS, 4/4 on d5.
- Paired Local: 4 x 21 pairs on d5; raw values and outliers retained. `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`.
- Generated device ELF and paired Runner are retained with SHA256 identities. BF16 function size increased by 16 bytes versus V007; register count is unavailable.
- Result summary and evidence paths: `RESULTS.md` and `results/`.

NEXT_ACTION: proceed to V009 with a distinct, single-factor register-lifetime hypothesis; use d5 only.
