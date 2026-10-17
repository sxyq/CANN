REVISION=V021
ATTEMPT=03
SNAPSHOT_TIME=2026-10-12T14:05:25Z
PURPOSE=read-only pre-build disk, device, process, source, and build-directory snapshot
COMMAND=ssh cann-server3; date -Is; hostname; df -Pk /home/data4t2; npu-smi info; list VLLMWorker_TP and paired_runner_v016; du -sh existing V021 build directory; stat and sha256sum existing Runner source and executable
SNAPSHOT=results/fp32-adapter-pre-build-v021-attempt-03.txt
NOTE=historical snapshot; not used as the 2026-10-13 build or execution resource reading
