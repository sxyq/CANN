# C001 Architecture Metadata

## Target and source

- Target: Ascend910B3 / DAV_2201 (`dav-c220-vec`), CANN 8.5.0.alpha002.
- Source: `phase4/workspaces/C001/c001_kernel.cpp`.
- Build: `phase4/workspaces/C001/CMakeLists.txt`, invoked by `build.sh` on `cann-server3`.
- Kernel type: AIV-only. The launch block dimension is an explicit multiple of four.

## Cooperative topology

- Row-group: four Vector Cores (`kGroupSize = 4`) cooperate on one row at a time.
- D slice: lane `l` owns the half-open range `[floor(D*l/4), floor(D*(l+1)/4))`.
- Partial: each lane computes `sum((x + residual)^2)` for its slice and writes one FP32 value to its 32-byte lane slot.
- Merge: lane 0 reads all four partial slots after the first group barrier and writes the FP32 inverse RMS value.
- Output: all four lanes apply the shared inverse RMS, gamma and bias over their own D slice.
- Row scheduling: group `g` handles rows `g, g + group_count, ...`; groups with no row return before any barrier.

## Synchronization API and count

The CANN 8.5.0.alpha002 DAV_2201 headers declare and compile:

```cpp
AscendC::GroupBarrier<AscendC::PipeMode::MTE3_MODE> barrier(
    GM_ADDR groupWorkspace, uint32_t arriveSize, uint32_t waitSize);
barrier.Arrive(lane);
barrier.Wait(lane);
```

The implementation uses two group barriers per row:

1. all four partial writes complete;
2. lane 0 publishes inverse RMS before all lanes read it.

The synchronization count is four API calls per active row (`Arrive`, `Wait`, `Arrive`, `Wait`), or two barrier rounds. Each active lane follows that exact sequence.

All four active lanes execute the same two calls in the same order. A group with no assigned rows returns before the first call, so an unscheduled row-group never waits. `GroupBarrier::GetWorkspaceLen()` is 2,048 bytes for a four-lane group on DAV_2201; the local layout reserves that full region.

## usedCores and blockDim

- `usedCores = 4 * group_count` Vector Cores.
- `blockDim = 4 * group_count`.
- `group_count` is a tiling field in `[1, 10]`, with ten groups as the upper bound in this source.
- The kernel uses `GetBlockIdx()` directly; no block outside the configured groups enters a barrier.

## Workspace

The candidate workspace starts at `GetUserWorkspace(workspace)` after the framework-reserved area. Per group:

| Region | Offset | Size | Purpose |
| --- | ---: | ---: | --- |
| GroupBarrier arrive/wait state | 0 | 2,048 B | two 64-byte barrier arrays with 512-byte cache-line stride |
| Partial slots | 2,048 B | 128 B | four 32-byte FP32 slots |
| Inverse RMS | 2,176 B | 32 B | one aligned FP32 slot |
| Per-group total | 0 | 2,208 B | rounded allocation stride |

For ten groups the GM reduction and barrier allocation is 22,080 bytes. No TQue or double-buffer allocation is used by this prototype. The framework-reserved GM area is excluded from the candidate allocation and is obtained through `GetUserWorkspace`.

The GM reduction and barrier workspace above is separate from UB accounting. On DAV_2201, `GroupBarrier` reserves 50 x 32 B = 1,600 B at the top of UB for its local counters. With the 8 KiB (`TMP_UB_SIZE`) framework reservation and zero TQue, double-buffer, or user temporary tensors, the explicit UB reservation is 9,792 B, leaving 178,624 B below the 184 KiB candidate limit. D-slice alignment is scalar and has no extra UB padding.

## Complete-domain behavior

- Hot path: FP16, BF16, FP32, `64 <= D <= 32768`, and `D % 32 == 0`.
- Private generic fallback: block zero performs the complete scalar row traversal for 2D/3D/4D flattened rows, all valid D values including unaligned tails, arbitrary positive epsilon, NaN and Inf.
- The fallback has no group barrier and therefore does not strand other scheduled blocks.

## Platform confirmation

The server3 toolkit header `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/ascendc/include/basic_api/interface/core_mng/roc/kernel_operator_group_barrier_intf.h` declares `GroupBarrier` for `__NPU_ARCH__ == 2201`. Its DAV_C220 implementation requires AIV execution, `arriveSizeIn` and `waitSizeIn` no larger than `GetBlockNum()`, uses 512-byte cache-line spacing, and reports `max(arriveSize, waitSize) * 512` bytes. The actual compile command below selected `--cce-aicore-arch=dav-c220-vec` and completed successfully.

## Compile evidence

- Host: `cann-server3` (`hwnput3`).
- Compiler: `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/ccec_compiler/bin/bisheng`.
- Log: `phase4/workspaces/C001/compile.log`.
- Status: PASS for the FP16, BF16 and FP32 template instantiations.
