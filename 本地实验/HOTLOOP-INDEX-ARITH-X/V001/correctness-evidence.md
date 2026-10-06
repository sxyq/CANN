# V001 correctness evidence

BUILD=PASS
CORRECTNESS=PASS
REASON=Existing unified run_kernel runner compiled in V001 support and executed parent/candidate against the established R31B wide-row case set; all returned RC=0 with bad=0.
SERVER=hwnput3
DEVICE=4
ENV_REGRESSION_EVENT=NONE
DEVICE_SNAPSHOT=Local NPU 0-7 visible; HBM usage 36729-61851/65536 MB; resident VLLM/python processes recorded; no process was stopped.
TARGET_CORRECTNESS=PASS
CASE_SET=rows=2; dtype=FP16/BF16; D=8192,12288,16384,32768; parent+cand=16 runs; warmup=0; samples=1; blocks=1; golden bad=0 for every run
RUNNER=V001/support/build-rerun2/hotloop_{parent,candidate}_probe
ONLINE=FORBIDDEN
DIRECT_PARENT=R31B V011
