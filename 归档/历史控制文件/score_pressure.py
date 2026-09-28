"""Rank saved timings against current public TBest without inferring shapes."""

import csv
from datetime import datetime, timezone
import io
import json
import math
from pathlib import Path
import urllib.request


ROOT = Path(__file__).resolve().parents[2]
URL = "https://cannjudge.cn/api/problems/6a9a9a99bf41025d6013eb85/ranking?page=1&size=1"
RESULTS = {
    "R31A-V002": ROOT / "phase4/online/R31A/V002/result.json",
    "R31B-V002": ROOT / "phase4/online/R31B/V002/result.json",
}


def point_score(time_us, best_us):
    if time_us <= 0 or best_us <= 0:
        raise ValueError("Timing must be positive")
    return 100 / (1 + math.log(time_us / best_us, 1.5))


def main():
    opener = urllib.request.build_opener(urllib.request.ProxyHandler({}))
    request = urllib.request.Request(URL, headers={"Accept": "application/json"})
    with opener.open(request, timeout=20) as response:
        public = json.load(response)
    retrieved = datetime.now(timezone.utc).isoformat(timespec="seconds")
    best = {case["_id"]: float(case["tbest"]) for case in public["testcases"]}
    fields = [
        "revision", "testcase", "testcase_id", "candidate_time_us", "best_time_us",
        "best_time_at_submission_us", "ratio", "current_point_score",
        "estimated_score_pressure", "recorded_official_score", "dtype", "D",
        "rowCount", "localRows", "avgRows", "mode", "retrieved_at", "best_time_source",
    ]
    output = io.StringIO()
    writer = csv.DictWriter(output, fields, delimiter="\t", lineterminator="\n")
    writer.writeheader()
    for revision, path in RESULTS.items():
        saved = json.loads(path.read_text())
        if saved.get("passCount") != 15 or len(saved["testcases"]) != 15:
            raise ValueError(f"Expected 15 passing measurements for {revision}")
        rows = []
        for case in saved["testcases"]:
            testcase_id = case["testcaseId"]
            current_best = best[testcase_id]
            elapsed = float(case["timeUs"])
            score = point_score(elapsed, current_best)
            rows.append({
                "revision": revision,
                "testcase": f"T{case['index']:02d}",
                "testcase_id": testcase_id,
                "candidate_time_us": elapsed,
                "best_time_us": current_best,
                "best_time_at_submission_us": case["bestTimeUs"],
                "ratio": round(elapsed / current_best, 6),
                "current_point_score": round(score, 6),
                "estimated_score_pressure": round((100 - score) / 15, 6),
                "recorded_official_score": saved["officialScore"],
                **{field: "UNOBSERVED" for field in ("dtype", "D", "rowCount", "localRows", "avgRows", "mode")},
                "retrieved_at": retrieved,
                "best_time_source": URL,
            })
        rows.sort(key=lambda row: row["ratio"], reverse=True)
        writer.writerows(rows)
        recalculated = sum(row["current_point_score"] for row in rows) / 15
        best_two = recalculated + sum(row["estimated_score_pressure"] for row in rows[:2])
        best_four = recalculated + sum(row["estimated_score_pressure"] for row in rows[:4])
        print(f"{revision}: recorded Official Score={saved['officialScore']}; current-formula estimate={recalculated:.4f}")
        print(f"  largest ratios: {', '.join(str(row['testcase']) + '=' + str(row['ratio']) for row in rows[:4])}")
        print(f"  if those cases reach TBest: top 2 -> {best_two:.4f}; top 4 -> {best_four:.4f}")
    (ROOT / "phase4/control/r31-score-pressure.tsv").write_text(output.getvalue())
    print("wrote phase4/control/r31-score-pressure.tsv; shape-to-mode mapping remains unobserved")


if __name__ == "__main__":
    main()
