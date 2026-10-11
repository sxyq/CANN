# N01 V004 Results

ROUTE: N01-REGISTER-LIVE-RANGE-X
REVISION: V004
SOURCE_PARENT_COMMIT: `c72be019d2066fb90c6a803e1640af4c12f15a72`
SOURCE_COMMIT: `e833b161dd2ffede6ba8fda34ae0073d67f85ac9`
SOURCE_SHA256: `9e19b6ab1a9ac71ceee22b92587f8deb879ab32e1b317b3ed6671ddd5cbc8a29`
PAIRED_PARENT: R31B V011 Champion, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
FULL_SOURCE_SIMILARITY: `0.982-0.999` across the ten route comparisons reported by Main, above the 80% target because the Champion body is shared. No cosmetic edits were used to lower this value. Effective-change similarity was not measured in this Route.

## Kernel change

In the BF16 branch of `ProcessWideLowPrecision`, after the widened add, V004
uses `xFp32` for the square and passes `residualFp32` as `ReduceSum` scratch.
The add, square, reduction, addresses, memory transfers, synchronization,
core allocation, and cache policy retain their existing semantics; the change
is the temporary-value reuse point.

## Compile and identity

- Compile: PASS on CANN `8.5.0.alpha002`, `dav-c220` / `dav-2201`; configure,
  build, and SSH return codes are all `0`. No second compile was run.
- The local branch HEAD is the specified source commit. The uploaded Candidate
  SHA matches the local V004 source, and the remote Parent SHA matches the
  Champion source.
- The tested paired Runner SHA256 is
  `3820f72961e18c2092ceb4a853c424e6cdd82044cb9fbb44bdca7742e25c7faa`.
  The extracted device binary SHA256 is
  `f42a5d97cb13160c5e53e67f40adaec009f9257f50fea792ef527966306f192b`, matching
  the local saved artifact.

## Correctness

- PASS, 4/4 cases: FP16 and BF16 at D=12288 and D=32768; both Parent and
  Candidate pass each case. Runner exit code `0`.
- The existing runner's Candidate label is `V001`; the command record and
  verified Candidate SHA identify the actual V004 binary.

## Device code

Within this V004 linked device binary, Parent/Candidate function bytes differ
for all dtypes. Sizes are FP32 `12840/12840`, FP16 `13792/13796`, and BF16
`14428/14428` bytes. V003-to-V004 Candidate bytes also differ for all three
dtypes. However, V003-to-V004 Parent bytes differ for all three dtypes too, so
the cross-build changes cannot isolate the V004 source edit. The target
disassembler exits `0` but prints `<not available>` for instruction words; no
register-count or register-improvement claim is made.

## Local

The existing paired Runner completed all four cases with 21 interleaved pairs
per case: 84 paired samples total, with every raw Parent/Candidate sample
retained. Correctness during Local is PASS for all cases. Device-event summary:

| Case | Parent median (us) | Candidate median (us) | Paired delta median (us) | Delta range (us) | Pairs |
|---|---:|---:|---:|---:|---:|
| FP16 D=12288 | 17.900 | 6.920 | -3.220 | -13.760 to 15.220 | 21 |
| FP16 D=32768 | 17.400 | 16.320 | -2.180 | -13.500 to 3.620 | 21 |
| BF16 D=12288 | 15.780 | 18.080 | +1.960 | -18.600 to 18.940 | 21 |
| BF16 D=32768 | 17.620 | 18.260 | -0.160 | -14.980 to 10.100 | 21 |

`LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`. Paired-delta spread is `17.120` to
`37.540 us`, with alternating low/high device-event samples. Immediately before
Local, the d7 snapshot showed AICore/AIVector `0%/0%`, HBM bandwidth `23%`, and
an existing Python process using `21260 MB`. After Local, AIVector was `88%`
and HBM bandwidth `28%`; no process was changed. HBM capacity was `65536 MB`
and usage was `37%`, corresponding to about `41288 MB` free from the rounded
usage percentage. No paired Runner remained after Local.

The stage snapshots are `pre-compile-device.txt`, `pre-correctness-device.txt`,
`post-correctness-device.txt` (the pre-Local observation), and
`post-local-device.txt`. The existing Runner emits case samples but no
case-boundary device snapshots; no per-case snapshots are claimed.

## Evidence

`results/` retains the full paired raw log, correctness raw log, command and
return-code records, device snapshots, paired Runner executable, linked device
binary/text sections, six Parent/Candidate function extracts, byte comparisons,
and target disassembly output. Failed setup/extraction attempts are recorded in
`attempts-v004.log`. Host build intermediates are excluded.

Remote run: `~/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V004/e833b161`

NEXT_ACTION: keep V004 evidence committed; no later revision was started in
this fixed-V004 test run.
