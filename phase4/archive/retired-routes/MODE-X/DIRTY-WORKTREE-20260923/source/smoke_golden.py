#!/usr/bin/env python3
"""Local golden smoke for MODE-X mode_x_full_link. Not submitted."""
import os, struct, sys, math, subprocess, tempfile, shutil

def write_bin(path, arr, dtype):
    import array
    if dtype == 0:
        a = array.array('f', arr)
    elif dtype == 2:
        a = array.array('H', [bf16_bits(v) for v in arr])
    else:
        # store as fp16 bits via struct
        a = array.array('H', [fp16_bits(v) for v in arr])
    with open(path, 'wb') as f:
        f.write(a.tobytes())

def fp16_bits(v):
    import struct
    # IEEE754 binary16 via numpy if available else manual
    try:
        import numpy as np
        return int(np.float16(v).view(np.uint16))
    except Exception:
        # simple round-to-nearest via float32 truncation path
        return struct.unpack('<H', struct.pack('<e', float(v)))[0]

def bf16_bits(v):
    return struct.unpack('<I', struct.pack('<f', float(v)))[0] >> 16

def read_bin(path, n, dtype):
    import array
    if dtype == 0:
        a = array.array('f')
        a.frombytes(open(path,'rb').read())
        return list(a)
    else:
        a = array.array('H')
        a.frombytes(open(path,'rb').read())
        try:
            import numpy as np
            if dtype == 2:
                return [struct.unpack('<f', struct.pack('<I', int(x) << 16))[0] for x in a]
            return [float(np.uint16(x).view(np.float16)) for x in a]
        except Exception:
            if dtype == 2:
                return [struct.unpack('<f', struct.pack('<I', int(x) << 16))[0] for x in a]
            return [struct.unpack('<e', struct.pack('<H', x))[0] for x in a]

def golden(x, r, g, b, eps=1e-5):
    d = len(g)
    out = []
    rows = len(x)//d
    for i in range(rows):
        u = [x[i*d+j] + r[i*d+j] for j in range(d)]
        s = sum(v*v for v in u) / d + eps
        rms = math.sqrt(s)
        inv = 1.0/rms
        for j in range(d):
            out.append(u[j]*inv*g[j] + b[j])
    return out

def run_case(bin_dir, rows, d, dtype, tag):
    os.makedirs(os.path.join(bin_dir, 'input'), exist_ok=True)
    os.makedirs(os.path.join(bin_dir, 'output'), exist_ok=True)
    es = 4 if dtype == 0 else 2
    n = rows * d
    # deterministic pseudo data in [-1,1]
    x = [((i * 37 + 11) % 1000) / 500.0 - 1.0 for i in range(n)]
    r = [((i * 17 + 3) % 1000) / 500.0 - 1.0 for i in range(n)]
    g = [1.0 + ((j * 13) % 100) / 200.0 for j in range(d)]
    b = [((j * 7) % 100) / 200.0 - 0.25 for j in range(d)]
    write_bin(os.path.join(bin_dir, 'input/x.bin'), x, dtype)
    write_bin(os.path.join(bin_dir, 'input/residual.bin'), r, dtype)
    write_bin(os.path.join(bin_dir, 'input/gamma.bin'), g, dtype)
    write_bin(os.path.join(bin_dir, 'input/bias.bin'), b, dtype)
    exe = os.path.join(bin_dir, 'mode_x_full_link')
    p = subprocess.run([exe, str(rows), str(d), str(dtype)], cwd=bin_dir,
                       stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if p.returncode != 0:
        print(f'{tag} RUN_FAIL rc={p.returncode} {p.stderr.decode()[:300]}')
        return False
    out = read_bin(os.path.join(bin_dir, 'output/output.bin'), n, dtype)
    if dtype == 1:
        x = read_bin(os.path.join(bin_dir, 'input/x.bin'), n, dtype)
        r = read_bin(os.path.join(bin_dir, 'input/residual.bin'), n, dtype)
        g = read_bin(os.path.join(bin_dir, 'input/gamma.bin'), d, dtype)
        b = read_bin(os.path.join(bin_dir, 'input/bias.bin'), d, dtype)
    elif dtype == 2:
        x = read_bin(os.path.join(bin_dir, 'input/x.bin'), n, dtype)
        r = read_bin(os.path.join(bin_dir, 'input/residual.bin'), n, dtype)
        g = read_bin(os.path.join(bin_dir, 'input/gamma.bin'), d, dtype)
        b = read_bin(os.path.join(bin_dir, 'input/bias.bin'), d, dtype)
    ref = golden(x, r, g, b)
    max_abs = 0.0
    bad = 0
    for i in range(n):
        e = abs(out[i] - ref[i])
        # scale-aware tolerance: fp16 ~2e-3 relative; fp32 tighter
        tol = 2e-3 if dtype != 0 else 1e-4
        if e > tol * max(1.0, abs(ref[i])):
            bad += 1
        if e > max_abs:
            max_abs = e
    print(f'{tag} rows={rows} d={d} dtype={dtype} max_abs={max_abs:.6g} bad={bad}')
    return bad == 0

def main():
    root = os.path.dirname(os.path.abspath(__file__))
    exe_src = os.path.join(root, 'build', 'mode_x_full_link')
    work = os.path.join(root, 'smoke_work')
    os.makedirs(work, exist_ok=True)
    shutil.copy(exe_src, os.path.join(work, 'mode_x_full_link'))
    cases = [
        (1, 64, 0, 'SINGLE_N_fp32_tiny'),
        (1, 2048, 1, 'SINGLE_N_fp16_mid'),
        (4, 256, 2, 'MERGE_N_bf16'),
        (32, 128, 0, 'MERGE_N_fp32'),
        (64, 512, 1, 'MULTI_N_fp16'),
        (3, 32768, 0, 'SPLIT_D_fp32_wide'),
        (2, 32768, 1, 'SPLIT_D_fp16_wide'),
        (1, 100, 1, 'NORMAL_odd_d'),  # D not 32B-aligned in bytes for fp16? 200B, still ok
        (5, 77, 0, 'tail_D_fp32'),
    ]
    ok = True
    for rows, d, dtype, tag in cases:
        if not run_case(work, rows, d, dtype, tag):
            ok = False
    sys.exit(0 if ok else 1)

if __name__ == '__main__':
    main()
