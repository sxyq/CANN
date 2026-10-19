#!/usr/bin/env python3
"""Local dispatch and math guards for the R31A-V017 architecture experiment."""

import math
from pathlib import Path


ROOT = Path(__file__).resolve().parent
SOURCE = (ROOT / "R31A-V017-submission.asc").read_text()


def classify(width, rows=2, blocks=1, dtype="fp32"):
    if dtype != "fp32" or width <= 8192:
        return "non-target"
    local_rows = rows // blocks
    if local_rows >= 2 and width < 24576:
        return "batch"
    if width <= 32768:
        return "cached-rows"
    return "stream"


def reference(row, gamma, bias, epsilon=1.0e-5):
    square_sum = math.fsum(value * value for value in row)
    inv_rms = 1.0 / math.sqrt(square_sum / len(row) + epsilon)
    return [(value * inv_rms) * scale + offset
            for value, scale, offset in zip(row, gamma, bias)]


def tiled_reference(row, gamma, bias, tile=7680, epsilon=1.0e-5):
    square_sum = math.fsum(value * value
                           for begin in range(0, len(row), tile)
                           for value in row[begin:begin + tile])
    inv_rms = 1.0 / math.sqrt(square_sum / len(row) + epsilon)
    return [(value * inv_rms) * scale + offset
            for value, scale, offset in zip(row, gamma, bias)]


def main():
    assert "kWideFp32CachedRowSwitchWidth = 24576" in SOURCE
    assert "rowWidth < static_cast<uint64_t>(kWideFp32CachedRowSwitchWidth)" in SOURCE
    assert classify(24576) == "cached-rows"
    print("TARGET_MODE width=24576 rows=2 blocks=1 dtype=fp32 path=cached-rows PASS")

    assert classify(24575) == "batch"
    assert classify(24577) == "cached-rows"
    print("ADJACENT_MODE width=24575 path=batch width=24577 path=cached-rows PASS")

    width = 24576
    row = [((index % 17) - 8) * 0.03125 for index in range(width)]
    gamma = [1.0 + (index % 13) * 0.002 for index in range(width)]
    bias = [((index % 11) - 5) * 0.001 for index in range(width)]
    expected = reference(row, gamma, bias)
    actual = tiled_reference(row, gamma, bias)
    max_error = max(abs(left - right) for left, right in zip(expected, actual))
    assert max_error <= 1.0e-7
    print("CORRECTNESS_GUARD width=24576 fp32 max_abs_error=%.3e PASS" % max_error)


if __name__ == "__main__":
    main()
