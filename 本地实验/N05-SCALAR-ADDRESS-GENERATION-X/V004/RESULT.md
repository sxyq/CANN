# N05 V004 Result

ROUTE=N05-SCALAR-ADDRESS-GENERATION-X
REVISION=V004
PARENT=V003
CHANGE=Replace ProcessWideLowPrecision pass-1/pass-2 address arithmetic with the executable WideLowPrecisionCursor structure; preserve tile order, GM offsets, arithmetic, DMA, events, and row/core ownership.
SOURCE_REVIEW=PASS
COMPILE=PASS
CONFIGURE_RC=0
BUILD_RC=0
FULL_LINK=PASS
CORRECTNESS=NONE
LOCAL_SCORE=NONE
DEVICE_ID=NONE
RUNNER=NOT_CREATED
PUSH=NO

The existing V016 CMake entry built `device`, `submission`, and `full_link` successfully. Complete command output and pre/post resource snapshots are in `compile-attempt-01.log`; artifact sizes and SHA256 values are in `build-artifacts.txt`.

The V004 source is a substantive change from V003. `WideLowPrecisionCursor` drives both address-generation paths through `Advance()`. Vector instruction scheduling, cache hints, arithmetic, DMA calls, event operations, tile traversal, and row/core ownership remain unchanged. Correctness and Local remain `NONE` because no runtime runner was created or executed.
