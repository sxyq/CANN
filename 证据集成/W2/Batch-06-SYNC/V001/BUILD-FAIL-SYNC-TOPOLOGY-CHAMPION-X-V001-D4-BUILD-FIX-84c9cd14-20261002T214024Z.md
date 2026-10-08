# SYNC-TOPOLOGY-CHAMPION-X V001 Build Failure

## Attempt identity

- `RUN_ID`: `SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-84c9cd14-20261002T214024Z`
- Route branch: `w2/m1/sync-topology`
- Harness source commit: `84c9cd14a8866bb2dd1c6aefee24cd3832d70c6a`
- Server: `hwnput3` (`server3`)
- Candidate source and sidecar SHA256: `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec`
- Direct Parent: `R31B V011`
- Parent source SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`

## Configure and Build/Link

- Configure: `PASS`.
- Build/Link targets: `sync_topology_v001_timing` and `sync_topology_v001_correctness`.
- Parent and Candidate ASC host compilation both failed at the `run_kernel` declaration with `unknown type name aclrtStream`.
- Numeric Build/Link return code was not included in the result provided by Main.
- No new timing or correctness executable was produced. Correctness and timing were not run.

## Support identities

Main reported that source, sidecar, Parent, and all seven support identities matched. Local support SHA256 values at commit `84c9cd14`:

| File | SHA256 |
|---|---|
| `support/CMakeLists.txt` | `8d9dad70a6490232d7706b286fcf0c98abd8f05941e3d9639615c7ed9b91aba3` |
| `support/npu_correctness.asc` | `b5ba81c58aa8b689d0bbc26626d8e9f2055cf74770938c988b9f4ac2e464df7a` |
| `support/local_abi_shim.h` | `b2d86f36ba134926cad0fb591a0976ab85d295a2257ac457be290740838c51d2` |
| `support/timing_runner.cpp` | `40164d8e1012121928565547ebc31c20f242f380f3ae90f5d5b12a683c21b0d7` |
| `support/runner_main.inc` | `05214f4ddd948b047713dd02dd367c6c75ace9d74b1d53759425b82a7aa83ea1` |
| `support/summarize_window.mjs` | `5b9acacb08817fc901c60474b272a744c577ff232762ce14069f6835a198a224` |
| `support/run_parent_window.sh` | `0a92ef218a7e298fc09f4784c1ddd9ec3fef54e9e08b31253dfc35ed9a74942a` |

## Server evidence

- Support identity log: `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/results/SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-84c9cd14-20261002T214024Z/runner-source-identity.log`
- Configure and Build/Link log: `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/results/SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-84c9cd14-20261002T214024Z/build-link.log`
- These server logs remain at their original paths and were not copied over.
