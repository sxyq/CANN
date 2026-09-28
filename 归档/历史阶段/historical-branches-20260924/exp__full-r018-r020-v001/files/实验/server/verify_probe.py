import math
import os
import struct


def read_values(path, dtype):
    data = open(path, "rb").read()
    if dtype == "fp32":
        return list(struct.unpack("<%df" % (len(data) // 4), data))
    words = struct.unpack("<%dH" % (len(data) // 2), data)
    if dtype == "bf16":
        return [struct.unpack("<f", struct.pack("<I", word << 16))[0]
                for word in words]
    return list(struct.unpack("<%de" % (len(data) // 2), data))


def main():
    dtype = os.environ.get("PROBE_DTYPE", "fp16")
    width = int(os.environ["PROBE_D"])
    rows = int(os.environ["PROBE_ROWS"])
    x = read_values("input/x.bin", dtype)
    residual = read_values("input/residual.bin", dtype)
    gamma = read_values("input/gamma.bin", dtype)
    bias = read_values("input/bias.bin", dtype)
    output = read_values("output/output.bin", dtype)
    reference = []
    for row in range(rows):
        begin = row * width
        y = [x[begin + i] + residual[begin + i] for i in range(width)]
        rms = math.sqrt(sum(v * v for v in y) / width + 1.0e-5)
        reference.extend(y[i] / rms * gamma[i] + bias[i] for i in range(width))
    errors = [abs(a - b) for a, b in zip(output, reference)]
    max_abs = max(errors) if errors else 0.0
    max_rel = max((e / max(abs(b), 1.0e-12) for e, b in zip(errors, reference)),
                  default=0.0)
    atol = 1.0e-2 if dtype == "bf16" else 2.0e-3
    rtol = 1.0e-2 if dtype == "bf16" else 2.0e-3
    passed = all(e <= atol + rtol * abs(b) for e, b in zip(errors, reference))
    print("VERIFY dtype=%s width=%d rows=%d max_abs=%.9g max_rel=%.9g status=%s" %
          (dtype, width, rows, max_abs, max_rel, "PASS" if passed else "FAIL"))
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
