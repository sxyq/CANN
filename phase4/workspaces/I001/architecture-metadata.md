# I001 V005 Architecture Metadata

candidate: I001
revision: V005
main_change: wide-D modes — retained-y, large-tile two-pass, few-row D-split. ABI = V003/V004 (15/15).

modes (host dispatch in run_kernel):
- mode 1 retained-y: cols*4 + tile workspace <= 140KiB. One GM read pair; u kept in UB.
- mode 0 two-pass stream: large tile 2048; two-pass sum then normalize.
- mode 2 D-split: rows<=4 && cols>=4096 && retain too big. Cores stripe D, 2x SyncAll, aclrtMalloc ws.

correctness carried from V004:
- DataCopyPad rightPadding align(n, 32/sizeof(T)); store exact n
- ReduceSum<float,true> with 8KiB separate tmp

UB:
- row modes: 3*2048*sizeof(T) + 2*2048*4 + 64 + 8KiB [+ D*4 if retained]
- split: same without full u row

expected wins: T13 (retained/stream), T14/T15 (D-split if few-row wide, else large-tile + retained).
