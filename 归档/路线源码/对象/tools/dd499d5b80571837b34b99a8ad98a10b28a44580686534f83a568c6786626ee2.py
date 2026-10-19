"""Reuse R12 task/event matching and retain every R05 sample."""
import argparse
import csv
import json
import math
import statistics as st
from decimal import Decimal
from pathlib import Path


def read(path, delimiter="\t"):
    with path.open() as f:
        return list(csv.DictReader(f, delimiter=delimiter))


def quantile(xs, fraction):
    a=sorted(xs); p=(len(a)-1)*fraction; lo=int(p); hi=min(lo+1,len(a)-1)
    return a[lo]*(hi-p)+a[hi]*(p-lo) if hi!=lo else a[lo]


def stats(xs):
    median=st.median(xs); mean=st.mean(xs); sd=st.stdev(xs) if len(xs)>1 else 0
    mad=st.median(abs(x-median) for x in xs)
    return dict(n=len(xs),median=median,mean=mean,stdev=sd,cv=sd/mean if mean else None,
                min=min(xs),max=max(xs),mad=mad,mad_over_median=mad/median if median else None,
                p10=quantile(xs,.1),p90=quantile(xs,.9))


def summarize(rows, metric):
    labels=sorted({r["side"] for r in rows})
    left,right=("P1","P2") if "P1" in labels else ("P","C")
    sides={s:stats([float(r[metric]) for r in rows if r["side"]==s]) for s in labels}
    blocks={str(b):{s:stats([float(r[metric]) for r in rows if r["side"]==s and int(r["block"])==b])
                    for s in labels} for b in (0,1)}
    drifts={s:abs(blocks["1"][s]["median"]-blocks["0"][s]["median"])/sides[s]["median"] for s in labels}
    pairs={}
    for r in rows:
        pair=pairs.setdefault((r["block"],r["pair"]),{})
        pair[r["side"]]=float(r[metric])
        if r["position"]=="0": pair["first"]=r["side"]
    diffs=[v[right]-v[left] for v in pairs.values()]
    ordered={s:st.median(v[right]-v[left] for v in pairs.values() if v["first"]==s) for s in labels}
    pooled=stats([float(r[metric]) for r in rows])
    pooled_blocks=[st.median(float(r[metric]) for r in rows if int(r["block"])==b) for b in (0,1)]
    pooled_drift=abs(pooled_blocks[1]-pooled_blocks[0])/pooled["median"]
    valid=all(sides[s]["mad_over_median"]<=.10 and drifts[s]<=.10 for s in labels)
    return dict(sides=sides,blocks=blocks,side_drift=drifts,pooled=pooled,pooled_block_medians=pooled_blocks,
                pooled_drift=pooled_drift,delta_us=sides[right]["median"]-sides[left]["median"],
                delta_percent=(sides[right]["median"]/sides[left]["median"]-1)*100,
                paired_difference_median_us=st.median(diffs),order_difference_median_us=ordered,
                paired_abs_difference_p90_us=quantile([abs(x) for x in diffs],.90),
                measurement_floor_us=max(quantile([abs(x) for x in diffs],.90),abs(pooled_blocks[1]-pooled_blocks[0])),
                r12_pooled_rule_pass=pooled["mad_over_median"]<=.10 and pooled_drift<=.10,
                side_stability="PASS" if valid else "MEASUREMENT_BLOCKED")


def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("prefix",type=Path)
    ap.add_argument("export",type=Path)
    ap.add_argument("--calls",type=Path)
    args=ap.parse_args()
    rows=read(Path(str(args.prefix)+".raw.tsv"))
    calls_path=args.calls or Path(str(args.prefix)+".calls.tsv")
    calls=read(calls_path)
    arm=rows[0]["arm"]
    assert all(r["arm"]==arm for r in rows)
    opfiles=list(args.export.glob("op_summary_*.csv")); taskfiles=list(args.export.glob("task_time_*.csv"))
    assert len(opfiles)==len(taskfiles)==1
    ops=read(opfiles[0],","); tasks=read(taskfiles[0],",")
    timestamp=lambda v:Decimal(v.replace("\\t","").strip())
    ops.sort(key=lambda x:timestamp(x["Task Start Time(us)"]))
    assert len(ops)==len(calls), (len(ops),len(calls))
    index={(t["Device_id"],t["stream_id"],t["task_id"]):t for t in tasks}
    assert len(index)==len(tasks)
    for op,call in zip(ops,calls):
        assert op["Device_id"]=="2" and op["Task Type"]=="AI_VECTOR_CORE"
        assert int(op["Block Dim"])==int(call["block_dim"])
        assert op["Op Name"]=="_Z24add_rms_norm_bias_customIfEvPhS0_S0_S0_S0_mmjff"
        task=index[(op["Device_id"],op["Stream ID"],op["Task ID"])]
        assert timestamp(op["Task Start Time(us)"])==timestamp(task["task_start(us)"])
        assert Decimal(op["Task Duration(us)"])==Decimal(task["task_time(us)"])
    mapped=[]
    for r in rows:
        ordinal=int(r["launch_ordinal"]); op=ops[ordinal-1]; call=calls[ordinal-1]
        assert call["phase"]=="timed" and call["arm"]==arm and call["case"]==r["case"] and call["side"]==r["side"]
        key=(op["Device_id"],op["Stream ID"],op["Task ID"])
        task=index[key]; before=index[(key[0],key[1],str(int(key[2])-1))]; after=index[(key[0],key[1],str(int(key[2])+1))]
        assert before["kernel_type"]==after["kernel_type"]=="EVENT_RECORD"
        begin=timestamp(before["task_start(us)"]); end=timestamp(after["task_start(us)"])
        start=timestamp(task["task_start(us)"]); stop=timestamp(task["task_stop(us)"])
        assert begin<=start<=stop<=end
        assert math.isfinite(float(r["device_us"])) and float(r["device_us"])>0
        mapped.append(dict(r,device_id=key[0],stream_id=key[1],task_id=key[2],
            kernel_us=float(task["task_time(us)"]),kernel_start_us=str(start),
            start_record_task_id=before["task_id"],stop_record_task_id=after["task_id"],
            event_minus_task_us=float(r["device_us"])-float(task["task_time(us)"]),
            interval_error_us=float(end-begin)-float(r["device_us"]),
            record_to_kernel_us=float(start-begin),kernel_to_record_us=float(end-stop)))
    assert len(mapped)==372 and sum(c["phase"]=="timed" and c["arm"]==arm for c in calls)==372
    result=dict(comparison_arm=arm,all_samples_retained=True,complete_kernel_calls=len(calls),timed_calls=len(mapped),
                op_task_matches=len(ops),event_brackets=len(mapped),device=2,
                max_interval_error_us=max(abs(r["interval_error_us"]) for r in mapped),cases={})
    for case in dict.fromkeys(r["case"] for r in mapped):
        part=[r for r in mapped if r["case"]==case]
        assert len(part)==124
        result["cases"][case]={m:summarize(part,m) for m in ("kernel_us","device_us","wall_us")}
        result["cases"][case]["event_minus_task_us"]=stats([r["event_minus_task_us"] for r in part])
    with Path(str(args.prefix)+".task-map.tsv").open("x") as f:
        writer=csv.DictWriter(f,fieldnames=list(mapped[0]),delimiter="\t",lineterminator="\n")
        writer.writeheader(); writer.writerows(mapped)
    with Path(str(args.prefix)+".summary.json").open("x") as f:
        json.dump(result,f,indent=2,allow_nan=False); f.write("\n")
    print(json.dumps({k:v["kernel_us"] for k,v in result["cases"].items()},indent=2,allow_nan=False))


if __name__=="__main__":
    main()
