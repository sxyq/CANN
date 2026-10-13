REVISION=V021
ATTEMPT=03
PURPOSE=rebuild the existing FP32 Runner adapter
INVOCATION=ssh cann-server3 with escaped multi-line remote command piped to tee
EXIT_CODE=2
RESULT=remote shell rejected the escaped command before CMake started; no remote build ran
LOG=results/fp32-adapter-runner-build-v021-attempt-03.log
