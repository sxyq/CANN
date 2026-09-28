# Main Review — UB-LIVENESS-X V002
SINGLE_CHANGE_AUDIT=PASS (correctness-only)
CORRECTNESS=FAIL (partial improvement FP16/BF16 large-D; FP32 still fail)
compile/link=PASS
probes raw-only INVALID_WITHOUT_CORRECTNESS
decision=LOCAL_REJECTED
NEXT=V003 correctness-only: force pass1 acc equal pass2 FuseU u^2 sum (isolation target FP32 D=64 acc=60.197); keep alias architecture; no perf variable.
Freeze V002 SHA 888bd60c1efbde1c5f90b1fa13daaaee59a057673315caa8437d16c2b23e3929 as parent for V003.
No online.
