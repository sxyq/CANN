# Main Review — ALIGN-TAIL-X V001
SINGLE_CHANGE_AUDIT=PASS; CORRECTNESS=PASS pair_max_abs=0
Window2 ~81min historical strict poll; PROBES PARKED; SHA frozen f573d16d…
decision=NEEDS_ONE_MORE_LOCAL + PARK until Main grants live-HBM/lease admission and the exact shape is requalified; no V002; no online.

## Round-dev4 Main Review
SHA frozen f573d16d…; CORRECTNESS PASS on pairs; LOAD=RESIDUAL_VLLM_ACCEPTED_DEVICE4
PROBE_DELTAS [+107.7, +345.6, -32.8, -47.1]% DIR=2/4 NOISE=393pp CONF=LOW
decision=NEEDS_ONE_MORE_LOCAL
Finding: free-HBM plus an unconflicted lease permits timing, but it does not guarantee a valid shape noise floor; concurrent next6 activity and resident load can worsen the measured spread. AICore 0% is not required.
Action: do not loop more sets on ALIGN alone; continue on the next device admitted by live HBM and lease state. No exclusive d4 wait is required.

## L002 controlled Main Review
CORRECTNESS PASS; LOAD=MODERATE (parent CV 0.205); DIR 3/4; MEDIAN +19.22% V001 slower; gain not >> parent jitter floor 25µs.
decision=NEEDS_ONE_MORE_LOCAL. Not ONLINE. Not REJECT. Not technical-fail. d4 released.
