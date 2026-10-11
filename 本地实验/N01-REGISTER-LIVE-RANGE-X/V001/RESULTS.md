# N01 V001 Results

ROUTE: N01-REGISTER-LIVE-RANGE-X
REVISION: V001
PARENT: `1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f`
SOURCE_SHA256: `3692a6a6fc67a7ec7a192fa41e2afdd35aa01893cea8e167562f60a2cd680184`
SOURCE_COMMIT: `a3661ee74a72e3bf3890ff7517488b8e8fb746de`

## Kernel change

The wide-row low-precision path changed two temporary scalar declarations from
const initialization to a declaration followed by assignment. Arithmetic,
addresses, memory transfers, synchronization, core allocation, and cache policy
were unchanged.

## Results

- Compile: PASS, final return code `0`, CANN `8.5.0.alpha002`, `dav-c220` / `dav-2201`.
- Correctness: PASS, 4/4 cases; FP16 and BF16 at D=12288 and D=32768. The
  first runtime attempt and its dynamic-library failure are retained in the raw
  records; the retry with the system C++ runtime passed.
- Device code: Parent and Candidate functions are byte-identical for FP16,
  BF16, and FP32. Sizes are `13792`, `14428`, and `12840` bytes respectively.
- Local: raw paired samples retained, 21 pairs per case across the same 4 cases.
  `LOCAL_SCORE=NONE` because the generated device code is identical; the raw
  timing data is evidence only and is not used as a performance claim.
- Device: d7 / `hwnput3`; pre-Local HBM usage `43%`, AICore `0%`, AIVector
  `2%`; post-Local HBM usage `43%`, AICore `8%`, AIVector `19%`. Existing
  processes were observed and left unchanged.

## Remote run

`~/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V001/a3661ee74a72e3bf3890ff7517488b8e8fb746de`

The remote Candidate SHA256 equals the local V001 source SHA256. No remote
temporary directory was reused or removed.

## Evidence

The `results/` directory contains the original command records, Compile logs,
Correctness logs, Local raw output, pre/post device snapshots, and the six
Parent/Candidate device-code files used by `codegen-compare.txt`. Host runner
executables and object files remain local and are intentionally excluded from
the Route commit.
