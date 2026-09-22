# I001 V006 Architecture Metadata

candidate: I001
revision: V006
main_change: fix T02/T03 ~100% WA on tiny-D retained-y. uKeep was sized `cols` floats but copy length is `workN=Align32B(n)`, which overflows when D is not 32B-aligned. Overflow corrupts adjacent UB (redTmp/fB) and poisons invRms. V006 sizes uKeep to `cols+32` floats. Speed paths unchanged.

modes (host dispatch in run_kernel):
- mode 1 retained-y: (cols+32)*4 + tile workspace <= 140KiB. One GM read pair; u kept in UB.
- mode 0 two-pass stream: large tile 2048; two-pass sum then normalize.
- mode 2 D-split: rows<=4 && cols>=4096 && retain too big. Cores stripe D, 2x SyncAll, aclrtMalloc ws.

correctness carried from V004:
- DataCopyPad rightPadding align(n, 32/sizeof(T)); store exact n
- ReduceSum<float,true> with 8KiB separate tmp
- gamma/bias indexed at column `d` (shape (D,)), not row base

UB:
- row modes: 3*tile*sizeof(T) + 2*tile*4 + 64 + 8KiB [+ (cols+32)*4 if retained]
- split: same without full u row

expected: T02/T03 recover to 15/15; keep V005 speed wins (T09/T11/T15). T14 remains the slow outlier.
