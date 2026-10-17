"""Byte-range model for R15 research; no Ascend runtime or device execution."""

import json


def align32(size):
    return (size + 31) // 32 * 32


def ext_ranges(width, first_row, rows, col, valid, elem_bytes, ub_pitch):
    payload = valid * elem_bytes
    padded = align32(payload)
    src_stride = (width - valid) * elem_bytes
    dst_stride = (ub_pitch - padded) // 32
    assert 0 < valid <= width - col
    assert 0 <= src_stride <= 0xFFFFFFFF
    assert ub_pitch >= padded and (ub_pitch - padded) % 32 == 0
    gm_start = (first_row * width + col) * elem_bytes
    gm = [(gm_start + i * (payload + src_stride),
           gm_start + i * (payload + src_stride) + payload)
          for i in range(rows)]
    ub = [(i * ub_pitch, i * ub_pitch + padded) for i in range(rows)]
    span = rows * padded + (rows - 1) * dst_stride * 32
    assert span == ub[-1][1]
    for i in range(rows - 1):
        assert gm[i][1] <= gm[i + 1][0]
        assert ub[i][1] <= ub[i + 1][0]
    return gm, ub, span


def main():
    counts = {}
    full_rows = 0
    unaligned_rows = 0
    for elem_bytes in (2, 4):
        for width in range(129, 2049):
            pitch = align32(width * elem_bytes)
            gm, ub, span = ext_ranges(width, 3, 2, 0, width, elem_bytes, pitch)
            assert gm[0][0] == 3 * width * elem_bytes
            assert gm[-1][1] == 5 * width * elem_bytes
            assert span <= 4096 * elem_bytes
            assert ub[1][0] // elem_bytes >= width
            assert ub[1][0] + width * elem_bytes <= span
            full_rows += 1
            unaligned_rows += width * elem_bytes % 32 != 0
    counts['full_row_cases'] = full_rows
    counts['unaligned_subset'] = unaligned_rows

    for elem_bytes in (2, 4):
        pitch = align32(2049 * elem_bytes)
        _, _, span = ext_ranges(2049, 3, 2, 0, 2049, elem_bytes, pitch)
        assert span > 4096 * elem_bytes
    counts['nonwide_capacity_exclusions'] = 2

    wide_cases = 0
    for tile in (2048, 2560, 3072, 3584, 4096):
        for valid in (1, 15, 16, 17, tile - 1, tile):
            width = 4 * tile + valid
            gm, ub, span = ext_ranges(width, 1, 2, 4 * tile, valid, 2, tile * 2)
            assert gm[-1][1] == 3 * width * 2
            assert ub[1][0] == tile * 2
            assert span == tile * 2 + align32(valid * 2)
            assert span <= 2 * tile * 2
            assert span > tile * 2  # The B-offset view has only one tile left.
            wide_cases += 1
    counts['wide_view_cases'] = wide_cases

    witnesses = []
    for tile_count in (3, 4):
        stream = [(row, tile) for row in range(2) for tile in range(tile_count)]
        slots = [stream[0], (1, 0)]
        prefetched_row = slots[1]
        assert prefetched_row != stream[1]
        slots[1] = stream[1]  # Parent issues u+1 to B before consuming u=0.
        assert prefetched_row not in slots
        witnesses.append({'tile_count': tile_count, 'lost_unit': prefetched_row,
                          'parent_next_unit': stream[1], 'overwritten_slot': 'B'})
    counts['lifetime_counterexamples'] = len(witnesses)

    seam_cases = 0
    for tail in (1, 15, 16, 17, 4095, 4096):
        tile = 4096
        width = 2 * tile + tail
        tail_start = (width + 2 * tile) * 2
        head_start = 2 * width * 2
        assert tail_start + tail * 2 == head_start
        assert head_start + tile * 2 <= 3 * width * 2
        flat_second_offset = tail * 2
        assert (flat_second_offset == tile * 2) == (tail == tile)
        equal_block_src_stride = head_start - (tail_start + tile * 2)
        assert (equal_block_src_stride >= 0) == (tail == tile)
        seam_cases += 1
    counts['tail_head_cases'] = seam_cases

    print(json.dumps({'status': 'PASS', 'scope': 'HOST_ADDRESS_AND_LIFETIME_ONLY',
                      'counts': counts,
                      'total_cases': sum(v for k, v in counts.items()
                                         if k != 'unaligned_subset'),
                      'lifetime_witnesses': witnesses,
                      'reference_correctness': 'NOT_RUN',
                      'device_operation': 'NONE'}, ensure_ascii=False, indent=2))


if __name__ == '__main__':
    main()
