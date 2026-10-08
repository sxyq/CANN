# V062 Local Environment Fix Evidence

- Initial attempt: `/usr/local/Ascend/ascend-toolkit/latest/set_env.sh` was absent; every runner exited 127 before `main()` with `libmsprofiler.so: cannot open shared object file`.
- Candidate source and binaries were not changed.
- Direct fix: source `/usr/local/Ascend/ascend-toolkit/set_env.sh` with `ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/latest` and the existing `ASC_DIR`.
- Verification: `ldd` resolved `libmsprofiler.so`, `libgert.so`, `libascendcl_impl.so`, `libge_executor.so`, `libgraph.so`, `libascend_watchdog.so`, `libascend_protobuf.so.3.13.0.0`, `libopp_registry.so`, `libmetadef.so`, and `libruntime_common.so`.
- Smoke test: the existing V062 Parent Local executable selected `ProcessSmallLowPrecisionContiguousBatched`, returned correctness `PASS`, emitted device-event timings, and exited 0.
- Initial failed logs: `environment-source-run.log`, `parent-stability.log`, `parent-block1.log`, `candidate-block1.log`, `parent-block2.log`, `candidate-block2.log`, `parent-block3.log`, `candidate-block3.log`, `parent-block4.log`, `candidate-block4.log`.
- Fixed run logs use the `*-retry.log` names and are the only files used for the numeric Local result.
