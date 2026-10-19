# N01 V003 Results

ROUTE: N01-REGISTER-LIVE-RANGE-X
REVISION: V003
PARENT_REVISION_COMMIT: `737143f3e5b0780a0cd96ec31ddfc2f5476e50e5`
SOURCE_COMMIT: `c72be019d2066fb90c6a803e1640af4c12f15a72`
SOURCE_SHA256: `ee8a2adf42b858921f8dab97cabf734c654bee1b7a480b713df285d043b9af60`
PAIRED_PARENT: R31B V011, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
FULL_SOURCE_SIMILARITY: `0.982-0.999` across the ten route comparisons reported by Main. This remains above the 80% target because the Champion body is shared; no name, comment, formatting, or meaningless wrapper change is used to lower it. V003 contains a real executable temporary-placement change, while its produced device bytes remain identical to V002.

## Kernel change

In the FP16 first pass of `ProcessWideLowPrecision`, V003 keeps the squared
value in `valueFp32` and passes `xFp32` as the `ReduceSum` scratch tensor. This
changes the temporary placement after conversion while preserving arithmetic,
addresses, memory transfers, synchronization, core allocation, and cache
policy.

## Results

- Compile: PASS, CANN `8.5.0.alpha002`, `dav-c220` / `dav-2201`, return code `0`.
- Correctness: PASS, 4/4 cases: FP16/BF16 at D=12288 and D=32768. The existing
  runner output label remains `V001`; the V003 command and Candidate SHA identify
  the actual tested binary.
- Device code: Parent/Candidate function bytes differ for FP32, FP16, and BF16,
  with sizes `12840 -> 12792`, `13792 -> 13804`, and `14428 -> 14476` bytes.
  V003 Candidate bytes are byte-identical to V002 Candidate bytes for all three
  dtypes, so this placement change produced no incremental device-code change.
  The target disassembler still reports `<not available>` for instruction words;
  no register-count claim is made.
- Local: PASS, 4 cases with 21 pairs each; raw samples and per-case statistics
  are retained. Paired device deltas are FP16 D12288 `+1.600 us`, FP16 D32768
  `-0.820 us`, BF16 D12288 `+0.120 us`, and BF16 D32768 `+2.620 us` median.
  `LOCAL_SCORE=VALID_PER_CASE`; no combined score is claimed.
- Device: d7 / `hwnput3`. Pre-Local HBM usage `37%`, AICore `0%`, AIVector
  `0%`; post-Local HBM usage `37%`, AICore `7%`, AIVector `16%`. The existing
  Python process was observed and left unchanged. No paired runner remained
  after Local.

## Remote run

`~/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V003/c72be019`

The Parent, Candidate, CMake, paired Runner, and ABI files matched their local
SHA256 values in this newly created run directory.

## Evidence

`results/` retains Compile, Correctness, Local, device snapshots, linked device
ELF/text, function byte extracts, V003 byte comparisons, and the raw target
disassembler output. Host runner binaries and `.asc.o` build intermediates are
excluded.
