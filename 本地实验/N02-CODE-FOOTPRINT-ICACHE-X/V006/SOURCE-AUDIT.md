# N02 V006 Source Audit

ROUTE=N02 CODE-FOOTPRINT-ICACHE-X
REVISION=V006
PARENT_REVISION=V005_REJECTED
PARENT_COMMIT=871a770df2f088eba1c25660581f2a8b2b550401
PRIOR_EXECUTABLE_PARENT=V004
DIRECT_PARENT=1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f
HEAD_AT_AUDIT=871a770df2f088eba1c25660581f2a8b2b550401

SOURCE=本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V004/submission.asc
SOURCE_SHA256=75aef181ceb2822c69fb32107b6cc20ac4dcc88d13a1fd727bb7755785314f2d
HOT_ENTRY=AddRmsNormBiasKernel<T>::Process at V004 submission.asc:159-169
HOT_FUNCTION=AddRmsNormBiasKernel<T>::ProcessWideLowPrecision at V004 submission.asc:3076-3365
CALL_CONDITION=widePath_ && !AscendC::IsSameType<T, float>::value

CURRENT_LAYOUT=The generic wide low-precision function flattens every retained row/tile into one unit stream, uses a two-slot MTE2 input pipeline, reduces one row at a time, then uses a two-slot gamma/bias MTE2 pipeline for the output pass. FP16 and BF16 are selected by existing compile-time branches inside the same function body.
PUBLIC_INTERVAL=dtype=FP16; 12288 <= D <= 32768; D % 4096 == 0; no hidden Case ID
WHY_INTERVAL=D > kCacheElems (8192) enters the existing wide path, and the V004 Init budget keeps wideFullYTileElems_ at kWideFullYTileElems (4096) throughout this interval.

V006_CHANGE=Add a compile-time FP16 static-layout microkernel with a fixed 4096-element tile and a fixed maximum retained-row contract. The public shape predicate selects it only for the interval above; the predicate is based on dtype and shape, not a mode field or R029 dispatch. The generic path remains for every other shape and dtype.
STRUCTURAL_INTENT=Replace the selected generic hotspot with a dedicated FP16 pass-1/pass-2 function body. The dedicated body keeps the original row ownership, y=x+residual arithmetic, FP32 square reduction, gamma/bias order, Load/Store offsets, event IDs, PipeBarrier calls, and V/MTE handoffs. It changes code layout by removing the BF16 branches and fixing the tile geometry at compile time.
FORBIDDEN_OVERLAP=R029 runtime mode dispatch; N01 register lifetime; N03 vector ILP; N04 GM/L2 access policy; N05 scalar address generation
CORRECTNESS_ASSUMPTION=For the selected public interval, rowWidth is an exact multiple of 4096 and the Init allocation has tileElems=4096. The dedicated path uses the same existing buffers and event schedule as the generic FP16 branch.
NEXT_ACTION=Create V006 candidate and structural diff, then upload it with the existing V016 single-version input and compile device/submission/full_link. Do not run the existing reset-capable runner.
