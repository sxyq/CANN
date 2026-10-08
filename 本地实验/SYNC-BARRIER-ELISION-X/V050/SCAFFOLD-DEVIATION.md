# V050 Scaffold Deviation

The initial V050 scaffold copied `V049/submission.asc` instead of the exact R31B-V011 parent into `V050/submission.asc`. After the declared V050 deletion, that intermediate source had SHA256 `27d91a7f070288cc173b8bb0b89bd0aa944e2ed47f1ac96ea7984d0df2f83b80` and contained both the V049 post-`FromFloat` PIPE_V deletion and the V050 post-Store `SyncMTE3ToV()` deletion.

The intermediate source is preserved as `submission-scaffold-invalid-inherited-v049.asc`. Its Compile, runner-build and Correctness logs are preserved under `logs/` but are explicitly excluded from V050's valid evidence:

- `logs/compile-v050-20261008T000745Z.log`
- `logs/correctness-runner-build-v050-20261008T000827Z.log`
- `logs/correctness-v050-20261008T000949Z.log`

The V049 barrier was restored from the exact V050 Parent. A subsequent Candidate with SHA256 `d48ef8b0cfe9633e06fca4abb36839add0ff00b4b7b5a489df6da53a0babb590` deleted the corresponding post-Store `SyncMTE3ToV()` in the FP32 `else` branch, not the declared FP16 branch. Its Compile/Correctness/Local artifacts are preserved separately as wrong-branch-invalid evidence and do not qualify V050.

Wrong-branch evidence is retained as `submission-v050-wrong-branch-invalid.asc`, `diff-wrong-branch-invalid.patch`, `submission-wrong-branch-invalid.sha256`, `source-meta-wrong-branch-invalid.json`, `compile-result-wrong-branch-invalid.json`, `correctness-result-wrong-branch-invalid.json`, `local-result-wrong-branch-invalid.json`, and `REVISION-DECLARATION-wrong-branch-invalid.md`. The associated logs are `logs/compile-v050-sibling-fix-20261008T001335Z.log`, `logs/correctness-runner-build-v050-sibling-fix-20261008T001406Z.log`, `logs/correctness-v050-valid-20261008T001445Z.log`, `logs/qualification-v050-valid-20261008T001649Z.log`, and `logs/local-interleaved-v050-20261008T001757Z.log`; their `d48ef8b0` identity makes them invalid for the declared FP16 factor. The separate inherited-V049 scaffold and its logs remain listed above.

The corrected Candidate SHA256 is `a8c12738f9937f1f6c2d9a08ad7f84fd182ecf1ed08065d196c787b3d270573c`; its only diff from the exact R31B-V011 Parent is the declared post-Store deletion in the FP16 branch. It was recompiled, passed all seven Correctness cases, and completed the valid qualification/Local sequence. The valid evidence paths and numeric noisy result are in `REVISION-DECLARATION.md` and `local-result.json`. A runner invocation made without the toolkit environment exited before sampling and is preserved as a separate tool-failure log.
