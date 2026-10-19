import os
import random
import struct


def write_values(path, values, dtype_name):
    if dtype_name in {"fp32", "0"}:
        payload = b"".join(struct.pack("<f", value) for value in values)
    elif dtype_name in {"bf16", "2"}:
        payload = b"".join(
            struct.pack("<H", (struct.unpack("<I", struct.pack("<f", value))[0] >> 16) & 0xFFFF)
            for value in values
        )
    else:
        payload = b"".join(struct.pack("<e", value) for value in values)
    with open(path, "wb") as output:
        output.write(payload)


def main():
    width = int(os.environ.get("PROBE_D", "64"))
    rows = int(os.environ.get("PROBE_ROWS", "1"))
    dtype_name = os.environ.get("PROBE_DTYPE", "fp16")
    seed = int(os.environ.get("PROBE_SEED", "42"))
    rng = random.Random(seed)
    x = [rng.uniform(-2.0, 2.0) for _ in range(rows * width)]
    residual = [rng.uniform(-2.0, 2.0) for _ in range(rows * width)]
    gamma = [rng.uniform(0.8, 1.2) for _ in range(width)]
    bias = [rng.uniform(-0.3, 0.3) for _ in range(width)]
    os.makedirs("input", exist_ok=True)
    os.makedirs("output", exist_ok=True)
    write_values("input/x.bin", x, dtype_name)
    write_values("input/residual.bin", residual, dtype_name)
    write_values("input/gamma.bin", gamma, dtype_name)
    write_values("input/bias.bin", bias, dtype_name)


if __name__ == "__main__":
    main()
