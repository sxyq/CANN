# Set-3 free-window result (Main policy unpark)

Device: ASCEND_DEVICE_ID=4
LOAD_QUALITY: DEV4_FREE_WINDOW_VLLM_RESIDENT
Session: 2026-09-24T04:04:22+00:00 -> 2026-09-24T04:05:15+00:00
SHA unchanged: 0fae0a42e3942356fe3477cd518b17180b6104d57350cee705cba37a5895e65c

Deltas % (V001 vs parent): [24.256, -11.066, -2.655, 58.492]
Median: 10.8
Directional: 0.5 favor V001
Aligned control: 58.492%
Unaligned median: -2.655%
Noise margin: 34.779

Pairs:
- {'pair': '01', 'shape': 'rows=7 width=65 dtype=0', 'shape_kind': 'unaligned', 'parent_us': 118.113, 'v001_us': 146.762, 'delta_percent': 24.256, 'device': '4', 'order': 'parent_then_v001'}
- {'pair': '02', 'shape': 'rows=33 width=100 dtype=0', 'shape_kind': 'unaligned', 'parent_us': 202.6, 'v001_us': 180.181, 'delta_percent': -11.066, 'device': '4', 'order': 'v001_then_parent'}
- {'pair': '03', 'shape': 'rows=17 width=257 dtype=0', 'shape_kind': 'unaligned', 'parent_us': 131.464, 'v001_us': 127.974, 'delta_percent': -2.655, 'device': '4', 'order': 'parent_then_v001'}
- {'pair': '04', 'shape': 'rows=17 width=256 dtype=0', 'shape_kind': 'aligned_d256', 'parent_us': 96.915, 'v001_us': 153.603, 'delta_percent': 58.492, 'device': '4', 'order': 'v001_then_parent'}

Decision: NEEDS_ONE_MORE_LOCAL
Reason: Set-3 on ASCEND_DEVICE_ID=4 under DEV4_FREE_WINDOW_VLLM_RESIDENT: deltas [24.256, -11.066, -2.655, 58.492]% median 10.800%, directional 0.50 favor V001, aligned control 58.492%, unaligned median -2.655%. Aligned control |58.492|% >= 5% still non-neutral; magnitudes not fully attributable. Correctness PASS; source unchanged; await Main.

set1/set2 preserved; set3 at support/free-window-set-3/.
No source change. No V002. No CANNJudge.
