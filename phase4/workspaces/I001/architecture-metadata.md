# I001 V004 Architecture Metadata

candidate: I001
revision: V004
main_change: aligned DataCopyPad (rightPadding), ReduceSum<float,true> with 8KiB tmp + separate dst, exact-n store, tile=256

fixes vs V003:
- T02 99% WA: unaligned D copies (rightPadding=0 on unaligned blockLen)
- T11 91% WA: same unaligned path on larger D
- T13 RE: ReduceSum sharedTmp was 128B overlapping dst; now 8KiB separate + partial dst

ABI: unchanged from V003 (online compile+run).
body: two-pass row-parallel, workN=align(n, 32/sizeof(T)), store exactly n.

UB: 3*TQue(256*sizeof(T)) + 2*256*4 + 64 + 8192 ≈ 20KiB float / less for half.
