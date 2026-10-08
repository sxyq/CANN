"""R06 address-only model; no device operations or latency estimates."""

import json


def align32(value):
    return (value + 31) // 32 * 32


def contained(interval, view):
    return view[0] <= interval[0] <= interval[1] <= view[1]


def ub_cases():
    print("UB_FIELDS=tile,valid,slot,L,P,old_capacity,shared_capacity,span,dst_stride")
    count = 0
    for tile in (2048, 2560, 3072, 3584, 4096):
        slot_bytes = tile * 2
        old_root_bytes = 2 * slot_bytes
        shared_bytes = 4 * slot_bytes
        assert 2 * align32(old_root_bytes) == align32(shared_bytes)
        assert old_root_bytes % 32 == 0
        for valid in (1, 15, 16, 17, tile - 1, tile):
            length = valid * 2
            padded = align32(length)
            for slot in (0, 1):
                start = slot * slot_bytes
                gap = old_root_bytes - padded
                assert gap >= 0 and gap % 32 == 0
                dst_stride = gap // 32
                span = 2 * padded + dst_stride * 32
                old_capacity = old_root_bytes - start
                shared_capacity = shared_bytes - start
                assert span > old_capacity
                assert span <= shared_capacity
                x_write = (start, start + padded)
                r_write = (start + old_root_bytes, start + old_root_bytes + padded)
                x_slot = (start, start + slot_bytes)
                r_slot = (start + old_root_bytes, start + old_root_bytes + slot_bytes)
                assert contained(x_write, x_slot)
                assert contained(r_write, r_slot)
                other = 1 - slot
                live_x = (other * slot_bytes, (other + 1) * slot_bytes)
                live_r = (old_root_bytes + live_x[0], old_root_bytes + live_x[1])
                for written in (x_write, r_write):
                    for live in (live_x, live_r):
                        assert written[1] <= live[0] or live[1] <= written[0]
                print("UB=" + ",".join(map(str, (
                    tile, valid, slot, length, padded, old_capacity,
                    shared_capacity, span, dst_stride,
                ))))
                count += 1
    assert count == 60
    return count


def gm_case(name, total_bytes, offset, length, delta, expected_field, expected_view):
    x_addr = 1 << 40
    r_addr = x_addr + delta
    x_view = (x_addr, x_addr + total_bytes)
    r_view = (r_addr, r_addr + total_bytes)
    x_read = (x_addr + offset, x_addr + offset + length)
    r_read = (r_addr + offset, r_addr + offset + length)
    assert contained(x_read, x_view) and contained(r_read, r_view)
    uint32_max = (1 << 32) - 1
    assert max(x_view[1], r_view[1]) < (1 << 64)
    stride = delta - length
    field_ok = 0 <= stride <= uint32_max
    one_view_ok = field_ok and contained((x_read[0], r_read[1]), x_view)
    assert field_ok == expected_field
    assert one_view_ok == expected_view
    source_span = 2 * length + stride if field_ok else None
    sdk_u16_span = 2 * length + (stride & 0xffff) if field_ok else None
    sdk_u16_within_x = (
        offset + sdk_u16_span <= total_bytes if field_ok else None
    )
    result = {
        "name": name,
        "synthetic_addresses": True,
        "N": total_bytes,
        "offset": offset,
        "L": length,
        "delta": delta,
        "src_stride": stride,
        "u32_field_fits": field_ok,
        "both_original_loads_in_their_own_views": True,
        "source_span": source_span,
        "single_original_x_view_contains_span": one_view_ok,
        "sdk_u16_span_model": sdk_u16_span,
        "sdk_u16_span_within_x": sdk_u16_within_x,
    }
    print("GM=" + json.dumps(result, sort_keys=True))
    return result


def gm_cases():
    uint32_max = (1 << 32) - 1
    cases = (
        ("disjoint_small_gap", 8193 * 2, 0, 8192, 32768, True, False),
        ("adjacent_complete_inputs", 16 * 16384 * 2, 0, 8192, 524288, True, False),
        ("reverse_order", 8193 * 2, 0, 8192, -32768, False, False),
        ("same_pointer", 8193 * 2, 0, 8192, 0, False, False),
        ("overlap_less_than_block", 8193 * 2, 0, 8192, 8190, False, False),
        ("u32_stride_limit", 8193 * 2, 0, 8192, 8192 + uint32_max, True, False),
        ("u32_stride_overflow", 8193 * 2, 0, 8192, 8192 + uint32_max + 1, False, False),
        ("overlapping_view_first_tile", 8193 * 2, 0, 8192, 8192, True, True),
        ("overlapping_view_second_tile", 8193 * 2, 8192, 8192, 8192, True, False),
        ("overlapping_view_tail", 8193 * 2, 16384, 2, 8192, True, False),
    )
    results = [gm_case(*case) for case in cases]
    small = results[0]
    assert small["source_span"] == 40960 > small["N"]
    assert small["sdk_u16_span_within_x"] is False
    adjacent = results[1]
    assert adjacent["source_span"] == 532480 > adjacent["N"]
    assert adjacent["sdk_u16_span_model"] == 73728
    assert adjacent["sdk_u16_span_within_x"] is True
    return len(results)


if __name__ == "__main__":
    ub_count = ub_cases()
    gm_count = gm_cases()
    print(f"SUMMARY UB_CASES={ub_count} GM_CASES={gm_count} RESULT=PASS")
    print("MODE=ADDRESS_ONLY COMPILE=NOT_RUN CORRECTNESS=NOT_RUN LOCAL=NOT_RUN")
