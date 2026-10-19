# N01 V002 Results

ROUTE: N01-REGISTER-LIVE-RANGE-X
REVISION: V002
PARENT_REVISION_COMMIT: `a3661ee74a72e3bf3890ff7517488b8e8fb746de`
SOURCE_COMMIT: `737143f3e5b0780a0cd96ec31ddfc2f5476e50e5`
SOURCE_SHA256: `f26bd3160c34039b535014c7b475ccbe3b6b5ed447d8efc37caee280288b345e`
PAIRED_PARENT: R31B V011, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`

## Kernel change

In `ProcessWideLowPrecision`, FP16 first-pass `valueFp32` is squared into
`xFp32`. Since `valueFp32` has no further use before the reduction, V002 passes
it as the `ReduceSum` FP32 scratch tensor. Math, addresses, memory transfers,
synchronization, core allocation, and cache policy are unchanged.

## Results

- Compile: CANN `8.5.0.alpha002`, `dav-c220` / `dav-2201`. Configure PASS.
  Initial build returned `2` because the host registration compile could not
  find `<vector>`. The retry supplied the existing system C++ include paths and
  returned `0`.
- Correctness: PASS, 4/4 cases: FP16/BF16 at D=12288 and D=32768. The runner's
  output label remains `V001`; the actual executable and Candidate source were
  built from this V002 run directory and the command record carries V002 SHA.
- Device code: extracted from the V002 linked `aicore_binary` ELF. Parent and
  Candidate function bytes differ for all three dtypes. Sizes: FP32
  `12840 -> 12792`, FP16 `13792 -> 13804`, BF16 `14428 -> 14476` bytes.
  The supplied `llvm-objdump` reports the target format as `elf64-hiipu` and
  emits `<not available>` for instruction words, so no instruction-level or
  register-count claim is made.
- Local: paired run PASS, 4 cases, 21 pairs per case, raw samples retained.
  `LOCAL_SCORE=NONE`: the pre-Local snapshot recorded AIVector usage at 91%, so
  the timing samples are kept as evidence without a performance conclusion.
- Device: d7 / `hwnput3`. Pre-Compile HBM usage 43%, AICore 1%, AIVector 11%;
  pre-Correctness HBM usage 37%, AICore 0%, AIVector 90%; pre-Local HBM usage
  37%, AICore 0%, AIVector 91%; post-Local AIVector usage 2%. Existing device
  processes were observed and left unchanged. No paired runner remained after
  Local.

## Remote run

`~/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V002/737143f3e5b0780a0cd96ec31ddfc2f5476e50e5`

The Parent, Candidate, CMake, paired Runner, and ABI files were uploaded to this
newly absent run directory and matched their local SHA256 values. V002 source
was compiled only after its source commit.

## Evidence

`results/` retains configure/build attempts and return codes, Correctness and
Local command/output records, device snapshots, linked device object/text,
function byte extracts, byte-difference output, and the raw disassembler output.
Host runner binaries and `.asc.o` build intermediates are not included.
