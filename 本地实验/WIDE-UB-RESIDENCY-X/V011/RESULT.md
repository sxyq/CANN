# WIDE-UB-RESIDENCY-X V011

ROUTE: WIDE-UB-RESIDENCY-X
REVISION: V011
PARENT_SOURCE: `本地实验/WIDE-UB-RESIDENCY-X/V007/submission.asc`
PARENT_SOURCE_COMMIT: `bd2e5f7f7c8f475f8057c16211f52959d3b7a33`
SOURCE_COMMIT: `e591b3ffe1f83f1e47b169be7cb03cda7f77d999`
WORKTREE_BASE: `c28bb58531b771d247f78db10382eaff1c534273` (V010，保留)

## HYPOTHESIS

FP16 wide path keeps the complete native-half `y` row in UB, so Pass2 does not
reload `x` or `residual`. At D=32768 the current 4096-element tile runs eight
times per row. Starting at 5120 elements should reduce that to seven runs,
cutting per-tile load/store and event setup while retaining the same full-row
`y`, arithmetic order, and two-deep input/parameter staging. Logical GM payload
is unchanged. This is a source-level performance hypothesis, not a measured
gain.

## SOURCE EVIDENCE

- V007 `Init()` allocates FP16 input staging as two tiles each for `x` and
  `residual`, a separate output tile, three FP32 work tiles, the complete
  half-precision `y` rows, and reduction partials.
- V007's tile chooser starts at 4096. Its FP16 call counted the four input
  tiles but omitted the separate output tile. V011 counts five FP16 I/O tiles
  and starts the FP16 chooser at 5120; FP32 and BF16 still start at 4096.
- For FP16 D=32768, rows=2 and two blocks, one row is handled per block. At
  tile=5120, the accounted UB use per block is 65,536 B (`y`) + 51,200 B
  (four input tiles and one output tile) + 61,440 B (three FP32 work tiles) +
  64 B (reduction) = 178,240 B. The 176 KiB planner budget is 180,224 B, so
  the estimate is 1,984 B below that budget.
- At D=12288, three retained rows would require 186,560 B with tile=5120; two
  require 161,920 B. The existing paired case has two rows and two blocks, so
  each block still handles one row. No block assignment changes.
- For D=32768, the source-level `Load`/`Store` call count per row changes from
  16 Pass1 input loads + 16 Pass2 parameter loads + 8 output stores (40) to
  14 + 14 + 7 (35). Across two rows that is 80 to 70 calls. Logical GM bytes
  remain 655,360 B per launch: 262,144 B of `x`/`residual`, 262,144 B of
  per-block `gamma`/`bias`, and 131,072 B of output. These counts are source
  estimates, not physical transaction counts or device timings.
- Earlier FP16 experiments that reloaded/recomputed `y` or kept only one tile
  were correct but slower at their reported medians (V001 D=32768: +1.960 us;
  V003 D=32768: +1.920 us). V011 retains the complete `y` row and does not
  repeat those mechanisms.
- V007 BF16 D=32768 had paired delta median −1.500 us with P10–P90
  −7.160..+2.280 us; that interval crosses zero. It does not establish a
  wider-tile gain. V011 isolates the FP16 path, whose full-row footprint is
  smaller.

## CHANGE

- Set the FP16 wide-path starting tile to 5120 elements.
- Include the separate FP16 output tile in the existing UB row planner's I/O
  tile count.
- Let the existing chooser retain the caller-selected starting tile; the
  FP32/BF16 callers continue to pass 4096.
- Pass1/Pass2 arithmetic, data flow, synchronization, and block assignment are
  unchanged. No Runner, build entry, or shared record was changed.

## COMPILE / DEVICE

COMPILE: PASS after one recorded environment failure.
COMPILE_CONFIGURE_RETURN_CODE: 0
COMPILE_BUILD_RETURN_CODE: 0
COMPILE_ENV: CANN `/usr/local/Ascend/ascend-toolkit/set_env.sh`; CPLUS_INCLUDE_PATH restored from the successful V010 transaction.
COMPILE_BINDING: existing `paired_runner_v016` includes V011 and CMake cache `R31B_V016_SOURCE_DIR` points to V011.
COMPILE_LOG_FIRST_FAILURE: `本地实验/WIDE-UB-RESIDENCY-X/V011/server-transaction-20261017/compile-rerun-20261016T192350Z.log`; `fatal error: 'vector' file not found`.
COMPILE_LOG_FINAL: `本地实验/WIDE-UB-RESIDENCY-X/V011/server-transaction-20261017/compile-rerun-20261016T192500Z-envfix.log`.
CORRECTNESS: PASS; Parent and Candidate passed all 4/4 paired cases; runner return code 0.
LOCAL: COMPLETED; 4 cases, 45 warmups, 21 paired samples per case.
LOCAL_LOG: `本地实验/WIDE-UB-RESIDENCY-X/V011/server-transaction-20261017/paired-run-rerun-20261016T192650Z-envfix.log`.
HOST: `hwnput3`
SOC_VERSION: `Ascend910B3`
DEVICE_ID: 2
PREFLIGHT_HBM_USAGE: 5%; `npu-smi info -t proc-mem -i 2` reported no process.
POSTFLIGHT_HBM_USAGE: 5%; `npu-smi info -t proc-mem -i 2` reported no process.
DEVICE_RESET: NO

## SERVER EVIDENCE

REMOTE_SOURCE_PATH: `/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/project/本地实验/WIDE-UB-RESIDENCY-X/V011/submission.asc`
SOURCE_SHA256: `f11d1399be4a00c8ebecd7c21cbfc057f26ec6c0b244995793f4beaad2e5e0b5`
CANDIDATE_COMMIT: `e591b3ffe1f83f1e47b169be7cb03cda7f77d999`
REMOTE_COMPILE_LOG_FIRST_FAILURE: `/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/V011/compile-rerun-20261016T192350Z.log`
REMOTE_COMPILE_LOG_FINAL: `/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/V011/compile-rerun-20261016T192500Z-envfix.log`
REMOTE_RUN_LOG: `/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/V011/paired-run-rerun-20261016T192650Z-envfix.log`

## LOCAL SUMMARY

| case | Parent median us | Candidate median us | paired delta median us | P10-P90 us |
|---|---:|---:|---:|---:|
| fp16-tail-d12288 | 6.640 | 7.240 | 0.020 | -1.920..14.480 |
| fp16-wide-d32768 | 10.940 | 10.180 | -0.620 | -5.660..3.860 |
| bf16-tail-d12288 | 8.020 | 7.920 | -0.160 | -1.520..0.980 |
| bf16-wide-d32768 | 14.480 | 12.920 | -1.520 | -2.100..13.900 |

LOCAL_SCORE: NONE
LOCAL_DELTA: NONE; all four P10-P90 intervals cross zero.
ONLINE_RECOMMENDATION: NONE; V011 has no stable local improvement, and no online submission was made.

## NEXT DEVICE ENTRY

V011 Compile, Correctness, and Local are complete. No next version is started;
the existing paired runner and V011 server evidence remain available for later
read-only comparison with a separately defined mechanism.

STATUS: SERVER_VALIDATED_LOCAL_NO_CLEAR_GAIN
