# N04 V019 Handoff

Candidate commit `1c80adf17`, SHA-256 `13435a1b4b14e5094f99931cfe99647a4cf43603bea8592cf580743870cbcaf5`. Relative to V018 source SHA `8f3f547622f39667fe0de8b527db617332577688722c4beab3b621e9833d77bc`, only the wide low-precision output hint changes from `DISABLE` to `NORMAL`.

The existing CANN 8.5 `paired_runner_v016` entry compiled successfully. R31B V011 Champion and V019 input identities are recorded in `remote-inputs.txt`; executable SHA-256 is `d3de11ce4967e47175dbc14a47e3596d2bded44d998cb1c22e91e9fe8e4776ce`. Correctness passed 4/4 on d7 with return code 0.

Local was not run. Recent d7 reads showed HBM bandwidth 33%, then AICore 11% and AIVector 44%; shared PID 439848 remained present and untouched. `LOCAL_SCORE=NONE`. V018 Local is also unmeasured and is not treated as V019 data.
