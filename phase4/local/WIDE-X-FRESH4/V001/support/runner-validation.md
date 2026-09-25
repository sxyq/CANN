# V001 Runner Preparation Record

- Route/revision: WIDE-X-FRESH4 / V001.
- Candidate source SHA256: `f7628795e6699288669dbff8963181e376e10741ba4d46c768af51cf09bf6895`.
- Direct Parent source SHA256: `5d0ee01165e46a281cbb7d1605feba3605ad59883845400a063e97704f27f2be`.
- Both source files were verified against these identities in the Route worktree and in the server3 staging attempt.
- Cleanup path: stream drain is attempted first. A drain error is printed and skips event destruction, device-buffer release, stream destruction, device reset, and runtime finalization. The already selected process exit code is retained; a drain error creates a nonzero result only when the run had no earlier failure.
- Server3 build: not completed. The first attempt stopped before CMake because the installed toolkit did not provide `set_env.sh`; no compiler or linker was invoked. The build script now has a fallback based on the installed toolkit package directory.
- Follow-up server3 build: unavailable after the SSH agent connection closed. The fallback has only passed shell syntax validation; CANN compile/link remain unverified.
- Local build tools: `clang++` and `g++` are present; CMake and local ACL headers are absent, so this host cannot compile or link the production runner.
- Runner execution, NPU correctness through this runner, and performance timing were not run. No current Main device lease was present.
- V001 Candidate and Parent source files were not changed.
- Fresh Blind review note: while locating the problem definition, an archived problem-analysis document was opened and included material beyond the core problem statement. No other Route kernel source or historical champion source was read. Main should decide whether this affects the Route's Fresh Blind classification.
