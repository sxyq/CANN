# SYNC-TOPOLOGY-CHAMPION-X V001 Build/Link Failure

## Attempt identity

- `RUN_ID`: `SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-9ed79938-20261002T230002Z`
- Route branch: `w2/m1/sync-topology`
- Harness source commit: `9ed799384956d1a747a632ecbd8321730c6882c3`
- Candidate source SHA256: `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec`
- Candidate sidecar: `PASS`, matched Candidate source SHA256.
- Direct Parent: `R31B V011`
- Parent source SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Eight support-file SHA256 values: all matched the route worktree.

## Configure and Build/Link

- Configure: `PASS`.
- Build/Link: `FAIL`, return code `2`.
- Verbose compiler command: both ASC and host sections contain complete `-include` options with their corresponding metadata header path; metadata declarations are available and the `TensorInfo` / `TensorGroupInfo` errors are cleared.
- Remaining Parent and Candidate diagnostic: `aclrtStream` is undeclared in the ASC source parser section. The ACL header is limited to the host section; CANN `acl_base.h:48` defines `aclrtStream` as `typedef void *aclrtStream`.
- No timing or correctness executable was produced. A Candidate shared library already on disk had a modification time before this RUN_ID and is not an output of this attempt.
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

- Configure and Build/Link log: `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/results/SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-9ed79938-20261002T230002Z/build-link.log`
- The server log remains at its original path and was not copied over.

No Candidate or Parent source, performance behavior, Revision metadata, score, or shared scheduling record was changed for this attempt.
