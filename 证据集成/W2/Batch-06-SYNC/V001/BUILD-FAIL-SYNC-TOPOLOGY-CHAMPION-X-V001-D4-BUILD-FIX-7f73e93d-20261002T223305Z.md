# SYNC-TOPOLOGY-CHAMPION-X V001 Build/Link Failure

## Attempt identity

- `RUN_ID`: `SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-7f73e93d-20261002T223305Z`
- Route branch: `w2/m1/sync-topology`
- Harness source commit: `7f73e93d7232ab3bbcc2ac2974efa5f888ffdbf9`
- Candidate source SHA256: `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec`
- Candidate sidecar and source-meta identity: `PASS`; sidecar matched the Candidate source.
- Direct Parent: `R31B V011`
- Parent source SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- All eight support-file identities: `MATCH`.

## Configure and Build/Link

- Configure: `PASS`.
- Build/Link: `FAIL`, return code `2`.
- Parent and Candidate host compilation reported `TensorGroupInfo`, `TensorInfo`, and `aclrtStream` as undeclared at the `run_kernel` declaration in `submission.asc`.
- The compiler command placed `local_tensor_metadata.h` within the `-Xaicore-start` / `-Xaicore-end` segment. In the host segment, the `local_abi_shim.h` path appeared without its `-include` option.
- No new executable was produced.
- Correctness: `NOT_RUN`.
- Timing: `NOT_RUN`.

## Resource snapshots

| Resource | Before Build | After Build |
|---|---:|---:|
| d4 HBM used | 59190/65536 MB | 59190/65536 MB |
| d4 AICore | 0% | 0% |
| `VLLMEngineCor` PID | 2999855 | 2999855 |
| Available disk | 616 GB | 616 GB |
| SYNC process | Not reported before | None after Build |

## Server evidence

- Configure and Build/Link log: `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/results/SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-7f73e93d-20261002T223305Z/build-link.log`
- The server log remains at its original path and was not copied over.

No Candidate or Parent source, performance behavior, Revision metadata, score, or shared scheduling record was changed for this attempt.
