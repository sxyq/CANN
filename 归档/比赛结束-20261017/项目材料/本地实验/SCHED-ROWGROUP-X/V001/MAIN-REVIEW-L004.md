# L004 controlled result — SCHED-ROWGROUP-X V001

Lease: L004 exclusive device 4
Released: 2026-09-24T04:18:38+00:00
SHA: 0fae0a42e3942356fe3477cd518b17180b6104d57350cee705cba37a5895e65c

## Preflight
- TIMESTAMP: 2026-09-24T04:16:41+00:00
- DEVICE_ID: 4
- AICORE: 0
- FREE_HBM: 6350 (pages as Main survey MB)
- VISIBLE: VLLMEngineCor residual only on d4; no other next6
- other_next6: NONE

## Protocol
- Interleave: PC, PC, CP, CP
- Shapes: 3 unaligned + 1 aligned control (17x256)
- Parent + Candidate both on d4, adjacent window

## Results
| pair | shape | order | parent_us | v001_us | delta% | parent_stdev | parent_sofm |
|---|---|---|---:|---:|---:|---:|---:|
| 01 | rows=7 width=65 dtype=0 | PC | 215.432 | 153.904 | -28.56 | 65.7641 | 0.305266 |
| 02 | rows=33 width=100 dtype=0 | PC | 247.371 | 123.246 | -50.178 | 53.6317 | 0.216807 |
| 03 | rows=17 width=257 dtype=0 | CP | 99.977 | 117.456 | 17.483 | 12.2719 | 0.122747 |
| 04 | rows=17 width=256 dtype=0 | CP | 101.596 | 112.506 | 10.739 | 16.6742 | 0.164123 |

PARENT_TIMES: [215.432, 247.371, 99.977, 101.596]
CANDIDATE_TIMES: [153.904, 123.246, 117.456, 112.506]
PARENT_JITTER max sofm: 0.3053, max range: 221.0us
MEDIAN_DELTA: -8.91%
WORST_DELTA: 17.483%
DIRECTIONAL_CONSISTENCY: 0.5
Aligned control: 10.739%
LOAD_QUALITY (parent jitter only): **LOAD_CONTAMINATED**
CORRECTNESS: PASS

## Decision
**NEEDS_ONE_MORE_LOCAL**

L004 controlled d4: LOAD_QUALITY=LOAD_CONTAMINATED from parent jitter only (max stdev/median=0.305, max range=221.0us). Per protocol, parent wild → do not judge candidate. Raw deltas [-28.56, -50.178, 17.483, 10.739]% median -8.910%, directional 0.50, aligned control 10.739%. Correctness PASS. Need cleaner parent jitter before ONLINE/REJECT.

d4 released. No V002. No CANNJudge. set1-3 + L004 preserved.
