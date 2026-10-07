# Second exact-parent C15 confirmation

- ROUTE / REVISION: `R-W4-4 / MODE-DISPATCH-CUTOFF-X / V028`; exact direct parent `V027`.
- PARENT SHA256: `f0ab43545e13d943e4c5bb5ff22400426e18f560ab313c1c6856b39fa904f033`.
- HOST / DEVICE: local `hwnput3`, device 4 (910B3); no SSH and no repaired Parent.
- TIME: `2026-10-07T06:03:40.751336573Z` to `2026-10-07T06:03:52.335175510Z`.
- RESULT: `rc=3`, `bad=28174`, `max_abs=1.2031`; raw output and statistics are adjacent in this directory.
- ADMISSION: conservative free-HBM lower bound `5898 MB`; AIVector/AICore `0%`; existing PID remained untouched.
- DISPOSITION: V028 remains blocked by exact-Parent correctness failure. No Candidate correctness or Local run on this retry; route score `NONE`. No V029 or Online action.
- PRIOR CONFIRMATION: first exact-parent failure is commit `4515b5c8bb5016c54730e0b6cd413df4da0787b9`.
