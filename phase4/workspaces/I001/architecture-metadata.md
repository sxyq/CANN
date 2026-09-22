# I001 V007 Architecture Metadata

candidate: I001
revision: V007
main_change: force V004 two-pass for D<256 (T02/T03 ~100% WA). Retained-y only for D>=256. V006 uKeep slack did not clear the WA — retained-y tiny-D has a second defect (save/restore of u or tiny-tile keep path). Speed paths for wide D unchanged.

modes (host dispatch in run_kernel):
- mode 1 retained-y: cols>=256 AND (cols+32)*4 + tile workspace <= 140KiB. One GM read pair; u kept in UB.
- mode 0 two-pass stream: D<256 (always) or retain too big. Two-pass sum then normalize (V004 algorithm).
- mode 2 D-split: rows<=4 && cols>=4096 && retain too big. Cores stripe D, 2x SyncAll, aclrtMalloc ws.

line-by-line V004 vs V005/V006 retained-y on tiny D (T02/T03):
| step | V004 two-pass (PASS) | V005/V006 retained-y (FAIL) |
| tile | Align(D) | Align(D) |
| pass1 | LoadPad x,res; u=x+res; u^2; ReduceSum | same + Adds(uKeep[0], u, workN) |
| pass2 | LoadPad x,res; u=x+res; *invRms | Muls(fA, uKeep[0], invRms) no reload |
| gamma/bias | LoadPad at column d | LoadPad at column d (same) |
| pad | rightPadding=workN-n | same |
Only functional delta is pass2 u source and the Adds save. uKeep sizing (V006) did not help.

correctness carried from V004:
- DataCopyPad rightPadding align(n, 32/sizeof(T)); store exact n
- ReduceSum<float,true> with 8KiB separate tmp
- gamma/bias indexed at column `d` (shape (D,))

UB:
- row modes: 3*tile*sizeof(T) + 2*tile*4 + 64 + 8KiB [+ (cols+32)*4 if retained]
- split: same without full u row

expected: T02/T03 Pass via two-pass; 15/15 at calc ~25.3. T14 remains the slow outlier.
