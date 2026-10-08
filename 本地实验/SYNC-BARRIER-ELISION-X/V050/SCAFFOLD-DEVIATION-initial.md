# V050 Scaffold Deviation

The initial V050 scaffold copied `V049/submission.asc` instead of the exact R31B-V011 parent into `V050/submission.asc`. After the declared V050 deletion, that intermediate source had SHA256 `27d91a7f070288cc173b8bb0b89bd0aa944e2ed47f1ac96ea7984d0df2f83b80` and contained both the V049 post-`FromFloat` PIPE_V deletion and the V050 post-Store `SyncMTE3ToV()` deletion.

The intermediate source is preserved as `submission-scaffold-invalid-inherited-v049.asc`. Its Compile, runner-build and Correctness logs are preserved under `logs/` but are explicitly excluded from V050's valid evidence:

- `logs/compile-v050-20261008T000745Z.log`
- `logs/correctness-runner-build-v050-20261008T000827Z.log`
- `logs/correctness-v050-20261008T000949Z.log`

The V049 barrier was restored from the exact V050 Parent. The final Candidate SHA256 is `d48ef8b0cfe9633e06fca4abb36839add0ff00b4b7b5a489df6da53a0babb590`; its only diff from the Parent is the declared post-Store `SyncMTE3ToV()` deletion. The corrected Candidate was recompiled, the runner rebuilt, and all seven Correctness cases passed before Local timing.
