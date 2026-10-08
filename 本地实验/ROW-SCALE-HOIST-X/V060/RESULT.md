# ROW-SCALE-HOIST-X V060 Result

- Parent: V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`).
- Candidate source: `144e93799f98ee8595c52c7bbbae1323ab46a4139c89f2712f22253ef18c2df8`.
- Single change: FP16 wide `ProcessSmallLowPrecisionContiguousBatched` row-scale placement, after native FP16 gamma and before bias.
- Compile: PASS (`device`, `submission`).
- Correctness: PASS for Parent and Candidate on FP16 `[128,256]`; candidate delta executed in the intended wide branch.
- Local: 4 interleaved blocks, 32 device-event samples per arm per block, plus one Parent stability invocation; 128 pooled samples per arm. Parent/Candidate pooled medians are `19.55 us` / `18.98 us`.
- Numeric descriptive Local score: `103.0031612223`; median Local delta: `-2.9156010230%`; median throughput: `1,676,112,531.969` / `1,726,448,893.572` elements/s; throughput delta `+2.9156010230%`.
- Paired direction is `3/4`, but cross-block variation and concurrent device load make the result `MEASUREMENT_BLOCKED`. All raw timings and snapshots are retained under `local/`.
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; no Online candidate. V060 is not promoted and Current Local Best remains V026.
- Next: V061 must use exact V026 as Direct Parent and test one independent FP32 panel-resident aligned-repeat row-scale placement change.

