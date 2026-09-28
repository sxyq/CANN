import math

def A32(x): return (x + 31) & ~31
BUDGET = 184*1024   # 188416 internal
HW = 192*1024       # 196608

# ---- OLD (V003) model ----
def est_old(tile, dim, e, dimF, pf, d2):
    slot = A32(tile*e); tileF = A32(tile*4)
    ns = 4 if d2 else 2
    param = 2*dimF if pf else 2*tileF
    reduce_ = 8192
    pool = ns*slot + 2*tileF
    if True:  # alias=1
        return A32(param+reduce_) + A32(pool)

def select_old(dim, e):
    dimF = A32(dim*4)
    for c in [2048,1024,512,256,128,64]:
        tt = min(c, dim)
        if tt < 64 and dim >= 64: continue
        for (pf,d2) in [(True,True),(True,False),(False,True),(False,False)]:
            if est_old(tt,dim,e,dimF,pf,d2) <= BUDGET:
                return tt, pf, d2, est_old(tt,dim,e,dimF,pf,d2)
    return None

def actual_old(tile, dim, e, pf, d2):
    bufTile = max(tile,512)
    slot = A32(bufTile*e); tileF = A32(bufTile*4)
    nX = 2 if d2 else 1
    pBytes = 2*A32(dim*4) if pf else 2*tileF
    return (A32(pBytes) + 8192 + 64 + nX*slot*3 + A32(2*max(tileF,2048)),
            slot, tileF)

# ---- NEW (post-H1) corrected live-set model == actual ----
def corrected(tile, e, d2, alias=True):
    bufTile = max(tile,512)
    slot = A32(bufTile*e); tileF = A32(bufTile*4)
    n = 2 if d2 else 1
    b = 2*tileF + 2*n*slot + 2*tileF + (0 if alias else n*slot)
    return A32(b)

def select_new(dim, e):
    for c in [8192,4096,2048,1024,512,256,128,64]:
        tt = min(c, dim)
        if tt < 64 and dim >= 64: continue
        for d2 in [True, False]:
            v = corrected(tt, e, d2)
            if v <= BUDGET:
                return tt, d2, v
    return None

def peak(tile, e, d2):
    # 2*slot_inst + 2*formCap + 2*tileF (worst-case simultaneous liveness, alias=1)
    bufTile = max(tile,512)
    slot = A32(bufTile*e); tileF = A32(bufTile*4)
    formCap = max(tileF, 2048)
    return 2*slot + 2*formCap + 2*tileF

def tiles_pass(dim, t): return math.ceil(dim/t)

DT = {'FP32':4, 'FP16':2, 'BF16':2}
DIMS = [64,256,1024,4096,8192,16384,32768]

for name, e in DT.items():
    print(f"\n### {name} (elemBytes={e})")
    print("| D | OLD EstBytes值 | OLD命中(t,full,d2) | V003实际预留 | OLD模型缺口 | NEW修正模型=预留 | NEW命中(t,d2) | 真实活跃峰值 | 余量vs184K | tile旧→新 | 往返/pass(pass1+pass2) |")
    print("|---:|---:|---|---:|---:|---:|---|---:|---:|---|---|")
    for dim in DIMS:
        o = select_old(dim, e)
        n = select_new(dim, e)
        tt, pf, d2, ov = o
        act_o, slot, tileF = actual_old(tt, dim, e, pf, d2)
        gap = act_o - ov
        nt, nd2, nv = n
        pk = peak(nt, e, nd2)
        slack = BUDGET - nv
        ro = tiles_pass(dim, tt); rn = tiles_pass(dim, nt)
        print(f"| {dim} | {ov} | ({tt},{pf},{d2}) | {act_o} | +{gap} | {nv} | ({nt},{nd2}) | {pk} | {slack} | {tt}→{nt} | {ro}→{rn} |")

# safety check: any NEW actual > HW?
print("\n### Safety: NEW actual vs HW 196608")
for name,e in DT.items():
    for dim in DIMS:
        nt,nd2,nv = select_new(dim,e)
        print(f"  {name} d={dim}: t={nt} d2={nd2} actual={nv} <=188416:{nv<=BUDGET} <=196608:{nv<=HW}")
