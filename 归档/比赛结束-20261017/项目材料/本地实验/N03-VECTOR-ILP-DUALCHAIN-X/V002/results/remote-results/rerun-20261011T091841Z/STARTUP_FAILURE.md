# V002 Rerun Startup Failure

- Run id: `rerun-v002-20261011T091841Z`
- Device: d0 on `hwnput3`
- Toolchain: CANN `8.5.0.alpha002`
- Command intended: source the CANN environment, then invoke the existing V002-bound `paired_runner_v016 --qualify-v011`.
- Result: remote shell returned `1` while sourcing `set_env.sh`: `LD_LIBRARY_PATH: unbound variable`.
- Cause: the wrapper enabled `set -u`; the environment script reads `LD_LIBRARY_PATH` before assigning it.
- The Runner did not start and no device Kernel was launched. The captured identity, command, and pre-control snapshots are retained beside this note.
- The next attempt removed only `set -u`; it ran the same V002-bound binary and completed both control and paired modes with return code `0`.
