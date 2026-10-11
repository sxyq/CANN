# N01 V004 Repeat 01

This follow-up used the fixed V004 source and already-built Runner. No source
was changed and Compile was not repeated.

## Identity

- Branch source commit: `e833b161dd2ffede6ba8fda34ae0073d67f85ac9`
- Candidate SHA256: `9e19b6ab1a9ac71ceee22b92587f8deb879ab32e1b317b3ed6671ddd5cbc8a29`
- Parent Champion SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Runner SHA256: `3820f72961e18c2092ceb4a853c424e6cdd82044cb9fbb44bdca7742e25c7faa`
- Device: d7 / `hwnput3`
- Remote run: `~/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V004/e833b161`
- The Runner's display label is V016/V001 depending on mode. Command identity and the verified Candidate SHA above identify this fixed V004 build.

## Same-binary Parent control

The existing `--qualify-v011` mode ran V011 alone for 2 blocks x 31 samples
per case, 62 samples per case and 248 sample lines total. Same-binary output
correctness passed all 4 cases and the Runner exited `0`. The existing
qualification rule reports blocked when MAD/median or block drift exceeds
0.25; all four cases were blocked by MAD/median:

| Case | Samples | Median (us) | MAD/median | Block drift | Result |
|---|---:|---:|---:|---:|---|
| FP16 D=12288 | 62 | 10.200 | 0.42157 | 0.06275 | `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE` |
| FP16 D=32768 | 62 | 23.590 | 0.26282 | 0.08309 | `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE` |
| BF16 D=12288 | 62 | 18.270 | 0.28845 | 0.14012 | `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE` |
| BF16 D=32768 | 62 | 25.150 | 0.28350 | 0.10020 | `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE` |

Raw and command files: `results/qualify-v011-v004.raw.log` and
`results/qualify-v011-v004.command`. Device observations before/after:
`results/pre-qualify-v004-device.txt` and
`results/post-qualify-v011-device.txt`.

## Paired repeat

The existing `--paired` mode ran 21 interleaved Parent/Candidate pairs for
each of the four cases: 84 complete paired samples total. Both outputs passed
correctness in every case; Runner exit code `0`.

| Case | Parent median (us) | Candidate median (us) | Paired delta median (us) | Pairs | Delta range (us) |
|---|---:|---:|---:|---:|---:|
| FP16 D=12288 | 7.340 | 16.840 | +2.840 | 21 | -12.140 to 14.720 |
| FP16 D=32768 | 11.900 | 13.500 | 0.000 | 21 | -13.140 to 14.280 |
| BF16 D=12288 | 17.960 | 17.820 | -1.760 | 21 | -209.660 to 21.320 |
| BF16 D=32768 | 18.660 | 19.920 | +1.240 | 21 | -21.840 to 27.760 |

`LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`. The same-binary control failed
repeatability for all four shapes, and the paired run contains large opposing
device-event excursions, including Parent `229.320 us` for BF16 D=12288.
These medians are descriptive raw-run statistics only; no aggregate score or
performance conclusion is assigned.

Raw and command files: `results/paired-v004-repeat-01.raw.log` and
`results/paired-v004-repeat-01.command`. Device snapshots before/after:
`results/pre-paired-v004-repeat-01-device.txt` and
`results/post-paired-v004-repeat-01-device.txt`.

## Device state

Before qualification, HBM usage was 37%, AICore 0%, AIVector 45%; an existing
Python process used 21260 MB. After qualification, HBM remained 37%, AICore
5%, AIVector 4%; existing `clx_ref_candida` and Python processes were recorded
and left untouched. Before the paired repeat, HBM was 37%, AICore 0%, AIVector
8%, and HBM bandwidth 25%; only the existing Python process was listed. After
the paired repeat, HBM remained 37%, AICore/AIVector were 0%/0%, HBM bandwidth
22%, and no paired Runner remained. HBM capacity was 65536 MB, giving about
41288 MB free based on the rounded usage percentage. There was no OOM, resource
error, or low-HBM condition.

`results/pre-local-observed-device.txt` is an earlier snapshot from before
this control/repeat sequence and is retained unchanged. No other process was
stopped or modified.

NEXT_ACTION: preserve this fixed-V004 evidence; no later source revision was
started.
