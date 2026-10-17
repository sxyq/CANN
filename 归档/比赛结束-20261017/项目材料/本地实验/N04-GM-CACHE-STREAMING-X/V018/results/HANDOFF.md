# N04 V018 Handoff

Candidate commit `f4be331e4`, SHA-256 `8f3f547622f39667fe0de8b527db617332577688722c4beab3b621e9833d77bc`. It changes only the wide low-precision residual L2 hint from `PERSISTENT` to `NORMAL` relative to V017 source SHA `786b0100c88adb232219fd221c4ffeea68ee393dc692405a41cbaf3cb97e646f`.

The existing CANN 8.5 `paired_runner_v016` entry compiled successfully. R31B V011 Champion and V018 source identities match `remote-inputs.txt`; the executable SHA is `06ed29ab35b9deee8b29dfcb64582347008066c2af692ee1b4fb36fed31b3980`. Correctness passed 4/4 on d7.

Local remains unrun. d7 AIVector rose from 0% at 00:15:44 to 48% at 00:16:42 and 90% at 00:17:47; the last read showed HBM bandwidth 30%. Shared PID 439848 remained present and untouched. `LOCAL_SCORE=NONE`; take a fresh read before any later Local run. V017 raw Local remains recorded in its own results and is not used as V018 data.
