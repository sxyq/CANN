# SYNC-TOPOLOGY-CHAMPION-X V001 Build Failure

## Attempt identity

- `RUN_ID`: `SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-6f0e9046-20261002T210858Z`
- Route branch: `w2/m1/sync-topology`
- Harness source commit: `6f0e904631fb2aabfbfdc31b4b38e0a0d9b49444`
- Server: `hwnput3` (`server3`)
- Candidate source SHA256: `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec`
- Direct Parent: `R31B V011`
- Parent source SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`

## Configure and Build/Link

- Configure: `PASS`, return code `0`.
- Build/Link targets: `sync_topology_v001_timing` and `sync_topology_v001_correctness`.
- Build/Link: `FAIL`, return code `2`.
- Candidate DSO: compile and link completed.
- Parent DSO: ASCPLUGIN reported `Unknown kernelInfo` at `线上结果/R31B/V011/submission.asc:3469`; the linker reported undefined `__origin__add_rms_norm_bias_custom<float>`, `__origin__add_rms_norm_bias_custom<half>`, and `__origin__add_rms_norm_bias_custom<bfloat16>` symbols.
- No timing or correctness executable was produced. Correctness and every timing mode were not run.

## Harness identities at build time

| File | SHA256 |
|---|---|
| `support/CMakeLists.txt` | `f3f9f9163e797763266c74c34809c14e1c7a6722debaa32b6ca5b16120502dd4` |
| `support/runner_main.inc` | `05214f4ddd948b047713dd02dd367c6c75ace9d74b1d53759425b82a7aa83ea1` |
| `support/timing_runner.cpp` | `40164d8e1012121928565547ebc31c20f242f380f3ae90f5d5b12a683c21b0d7` |
| `support/parent_module.asc` | `09da6a213f4037b4740c4065277eadc8fb0fd45c2658f05d7211f068077119d8` |
| `support/candidate_module.asc` | `62701989a596f273462aef6bbafef8aa1c425c2520e40f472e99a4070405424e` |
| `support/local_abi_shim.h` | `b2d86f36ba134926cad0fb591a0976ab85d295a2257ac457be290740838c51d2` |

## Server evidence

- Configure and Build/Link log: `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/results/SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-6f0e9046-20261002T210858Z/build-link.log`
- No log was copied over or replaced; this record indexes the existing server output.
