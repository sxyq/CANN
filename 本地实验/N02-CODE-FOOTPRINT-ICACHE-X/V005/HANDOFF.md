# N02 V005 Handoff

- Candidate source: `本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V005/submission.asc`
- Candidate source commit: `2f0316b103f92978b9be07cce42c45f0566b4e30`
- Paired-entry binding commit: `87eaba22cd8efe1fd0d74e507207e0f7f627e26b`
- Parent source: `线上结果/R31B/V011/submission.asc`
- Compile: Parent/Candidate `full_link` and existing paired runner build passed under CANN 8.5.0.alpha002.
- Correctness: Parent and Candidate passed 4/4 cases on d6.
- Local: 4 cases x 21 pairs returned 0. Outliers and the active d6 VLLM load make the medians diagnostic only; `LOCAL_SCORE=NONE` and `LOCAL_DELTA=NONE`.
- Device object size: 334040 B to 330792 B. Both dtype hot symbols shortened by 0x460 B; `.alink` size stayed 183144 B while its contents differed.
- Remote run: `/home/data4t2/lelinfeng/cann/server_runs/N02-CODE-FOOTPRINT-ICACHE-X-RESET-20261011/V005/2f0316b103f92978b9be07cce42c45f0566b4e30`
- Complete raw evidence and device artifacts are under `V005/results/`.
- Next: V006 should use a distinct device-side structure change that produces a materially different instruction layout; avoid another inline/noinline attribute variant and keep host dispatch unchanged.
