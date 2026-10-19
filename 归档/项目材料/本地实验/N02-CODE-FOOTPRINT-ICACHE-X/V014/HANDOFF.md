# N02 V014 Handoff

- Candidate: `本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V014/submission.asc`
- Source commit: `eba13c32d`; SHA-256: `697b2f181763300a05207df7e9bff373e86e5844b272a99a74fe67a6541b44d6`.
- Parent Candidate: V013 source commit `3b0593944`.
- Compile: Candidate full-link RC=0. Initial paired Runner build RC=2; existing V016 Runner build retry 05 RC=0 and was used without further rebuilding.
- Runner identity: V016, SHA-256 `fc161b75687af8a5ec65318b59a5331459677358b963005dcf901fda341d8093`; dependency record includes Parent V011 and Candidate V014.
- Device artifacts: Candidate object 346016 B / `.alink` 191704 B; both differ from Parent V011 and V013.
- Correctness: 4/4 Parent/Candidate cases PASS, RC=0, attempt 03. Earlier missing-directory and loader failures are retained and did not execute kernels.
- Same-binary: V011 control RC=0, 62 samples per case; robust MAD/drift classification PASS in all four cases. Raw event times still contain isolated spikes up to 269.580 us.
- Paired Local: 4 x 21 alternating pairs, RC=0. Large unexplained device-event tails, including values up to hundreds of microseconds, make the small median deltas non-attributable. `LOCAL_SCORE=NONE`, `LOCAL_DELTA=NONE`; all raw samples retained.
- d6 load: snapshots before each stage and after Local report HBM 91%, AICore/HBM bandwidth 0%, VLLMWorker_TP PID 3357697 using 56355 MB, and no `fuser` output for `/dev/davinci6`. The process was not changed.
- Case identity: V016 labels identify four local shapes (FP16/BF16 at D=12288 and D=32768). Mapping to Official Case 2/4/15 is unknown; do not infer a score from these local samples.
- Full outcomes and paths: `本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V014/RESULTS.md` and `results/`.

V014 changes the wide low-precision pass-2 loop from V013's explicit two-tile body to a tile-zero prologue followed by a uniform one-tile interior loop. The next research step should compare the available Case score evidence only after the case-to-shape mapping is established, then select a distinct code-layout/control-flow hypothesis. No V015 source or device run is part of this record.
