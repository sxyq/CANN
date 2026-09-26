# Main Review — ALIGN-TAIL-X V001
SINGLE_CHANGE_AUDIT=PASS; CORRECTNESS=PASS pair_max_abs=0
Window2 ~81min no clean load; PROBES PARKED; SHA frozen f573d16d…
decision=NEEDS_ONE_MORE_LOCAL + PARK until Main clean window; no V002; no online.

## Round-dev4 Main Review
SHA frozen f573d16d…; CORRECTNESS PASS on pairs; LOAD=RESIDUAL_VLLM_ACCEPTED_DEVICE4
PROBE_DELTAS [+107.7, +345.6, -32.8, -47.1]% DIR=2/4 NOISE=393pp CONF=LOW
decision=NEEDS_ONE_MORE_LOCAL
Finding: free-HBM+AICore0 insufficient; concurrent next6 on same device worsens noise.
Action: do not loop more sets on ALIGN alone; wait for serialized single-route dev4 slot.

## L002 controlled Main Review
CORRECTNESS PASS; LOAD=MODERATE (parent CV 0.205); DIR 3/4; MEDIAN +19.22% V001 slower; gain not >> parent jitter floor 25µs.
decision=NEEDS_ONE_MORE_LOCAL. Not ONLINE. Not REJECT. Not technical-fail. d4 released.
