# FULL V001 Online Coverage Report

日期：2026-09-19

完成条件：13 条规划中的 FULL route 均获得一次 CANNJudge 线上终态。PASS、Wrong Answer 和 Runtime Error 均计入线上覆盖；Compile Error 单独保留为过程记录。

| Route | Representative candidate | Git commit | Submission ID | Online result | Pass count | Official Score | Source SHA-256 | 15-case record |
| --- | --- | --- | --- | --- | ---: | ---: | --- | --- |
| FULL-R006-REDUCTION-ARCH | upstream snapshot | f27c029 | not recorded | Correctness Fail | 1/15 | — | 94ab0ef96a1a907fa187797b6c72361b2bdacfe6faf7a95d6471bd537a913266 | `实验/online/upstream-2026-09-18/results/FULL-R006-V001_RESULT.json` |
| FULL-R013-DOUBLE-BUFFER-PIPELINE | upstream exact candidate | 66a4f5a | 6aacb325b0477ec41e1707d2 | PASS | 15/15 | 18.76 | 8e00792c69af079f99f83bd57be1ac74c6c4c560c44ed2f0bf1b544ef88db9f7 | `实验/online/FULL-R013-DOUBLE-BUFFER-PIPELINE/V001/6aacb325b0477ec41e1707d2/result.json` |
| FULL-R002-RETAINED-Y | L001 | 4878d55 | 6aace5f8b0477ec41e33e210 | Wrong Answer | 14/15 | — | 00c1a86043e924a96a79d0976a51dd1a065cce71614484b09569814f7a2a782c | `实验/online/FULL-R002-RETAINED-Y/V001/6aace5f8b0477ec41e33e210/result.json` |
| FULL-R005-LARGE-TILE | L002 | 10af2e6 | 6aad349bb0477ec41e606254 | PASS | 15/15 | 25.71 | f8717dec00b916b967d3e7331bb35212edae31020cf08731974647146bcc9eac | `实验/online/FULL-R005-LARGE-TILE/V001/6aad349bb0477ec41e606254/result.json` |
| FULL-R015-MULTI-ROW-DMA | L001 | b99767f | 6aad46e5b0477ec41e6a178f | Runtime Error | 1/15 | — | 8e0fd09a5686315c481676defbfe83d811942aad7a92d58dd2bd80454b758521 | `实验/online/FULL-R015-MULTI-ROW-DMA/V001/6aad46e5b0477ec41e6a178f/result.json` |
| FULL-R019-NORMALIZATION | L003 | 2d0dd3b | 6aad5c03b0477ec41e73a5b7 | PASS | 15/15 | 18.68 | ce7be90cccd4e08cc24b44e3701bb5b598e043dd8c6bd0a1c0ca6fd2faec4954 | `实验/online/FULL-R019-NORMALIZATION/L003/6aad5c03b0477ec41e73a5b7/result.json` |
| FULL-R029-WIDE-CACHED-ROW | L005 | 2f91ca3 | 6aad7849b0477ec41e7fd3b1 | Runtime Error | 4/15 | — | 7e10cfe561cb718aa3c0a56584ecb5e02b1db69d44803581690248bd0360ebe9 | `实验/online/FULL-R029-WIDE-CACHED-ROW/V001/6aad7849b0477ec41e7fd3b1/result.json` |
| FULL-R030-WIDE-PARAM-REUSE | L004 | 245074f | 6aad8c4db0477ec41e864499 | Runtime Error | 4/15 | — | 84da7319b6bab7166635cfefba29a639a18ec871cf6d08b26d2493b76cc91f9b | `实验/online/FULL-R030-WIDE-PARAM-REUSE/V001/6aad8c4db0477ec41e864499/result.json` |
| FULL-R009-COPY-CENTRIC | L002 | c6c372d | 6aad748eb0477ec41e7e6b20 | PASS | 15/15 | 18.61 | 3c9dca95dbbae97deb3164aa109956c6366c4cb839839f051c01f496b75920d0 | `实验/online/FULL-R009-COPY-CENTRIC/L002/6aad748eb0477ec41e7e6b20/result.json` |
| FULL-R010-TAIL-CENTRIC | L001 | 33bd5d0 | 6aad6c97b0477ec41e7b50df | PASS | 15/15 | 17.66 | d68f5be582e709f407f6b26568fd9843ffdd47e4010923049c370a6cd4468a94 | `实验/online/FULL-R010-TAIL-CENTRIC/V001/6aad6c97b0477ec41e7b50df/result.json` |
| FULL-R012-ALIGNMENT-ROWGROUP | V001 | 9af42e9 | 6aad70a5b0477ec41e7cf41b | PASS | 15/15 | 22.96 | b00e8fef89d1bbc227e37ff137ed3ce44afb98caa454acdd722a439defbaf135 | `实验/online/FULL-R012-ALIGNMENT-ROWGROUP/V001/6aad70a5b0477ec41e7cf41b/result.json` |
| FULL-R014-PARAMETER-RESIDENCY-ARCH | upstream snapshot | f27c029 | not recorded | PASS | 15/15 | 20.09 | 404113c5fe365e5a7f307150d48f63125f2b2a2b3d7d61327974f15a2bdc794b | `实验/online/upstream-2026-09-18/results/FULL-R014-V001_RESULT.json` |
| FULL-R016-SCHEDULING-ARCH | CompileFix-A upstream snapshot | f27c029 | not recorded | PASS after CompileFix-A | 15/15 | 17.14 | 62de32dfc56a2b258704f658115fd01c8b224c6b814c106c049fd8cc9070c216 | `实验/online/upstream-2026-09-18/results/FULL-R016-V001_RESULT.json` |

## Architecture and failure notes

- R006 used a reduction-centric rewrite and failed correctness after case 1.
- R013 used native input ping-pong with MTE2/vector/MTE3 event ownership and passed all cases.
- R002 retained `y` between reduction and normalization; case 5 returned Wrong Answer.
- R005 used a wide 8192-element tile and passed all cases.
- R015 used multi-row DMA ownership and reached Runtime Error after case 1.
- R019 reorganized reciprocal normalization and passed all cases.
- R029 used a wide retained-row cache and reached Runtime Error after case 4.
- R030 reused gamma and bias across wide rows; case 5 returned `507035`, then cases 6-15 were skipped.
- R009, R010 and R012 centered copy movement, manual tail handling and 32-byte-safe row groups; each passed all cases.
- R014 used parameter residency and R016 used scheduling with same-V001 CompileFix-A; both passed all cases.

The complete raw testcase arrays remain in the listed JSON files. Official Score values above are copied from platform responses; no locally derived score is presented as Official Score. This phase stops here and does not start V002/V003, pairwise mix or router work.
