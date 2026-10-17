# SYNC-TOPOLOGY-CHAMPION-X V001 Build/Link Failure

## Attempt identity

- `RUN_ID`: `SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-e2d3d8c3-20261002T220501Z`
- Route branch: `w2/m1/sync-topology`
- Harness source commit: `e2d3d8c30083a522f504c5472bfaf2068053b25b`
- Candidate source SHA256: `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec`
- Candidate sidecar: matched Candidate source SHA256; identity verification passed.
- Direct Parent: `R31B V011`
- Parent source SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Candidate, sidecar, and Parent identity verification: `PASS`.

## Configure and Build/Link

- Configure: `PASS`.
- Build/Link: `FAIL`, return code `2`.
- Parent and Candidate ASC host compilation both reported `unknown type name TensorGroupInfo` at `submission.asc:3479`.
- No new timing or correctness executable was produced.
- Correctness: `NOT_RUN`.
- Timing: `NOT_RUN`.

## Resource snapshot

- Device: `d4`.
- HBM used: `59190/65536 MB`.
- AICore: `0%`.
- Resident process: `VLLMEngineCor`, PID `2999855`.
- Available disk: `616 GB`.

## Server evidence

- Configure and Build/Link log: `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/results/SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-e2d3d8c3-20261002T220501Z/build-link.log`
- The server log remains at its original path and was not copied over.

No score or shared scheduling record was changed by this attempt.
