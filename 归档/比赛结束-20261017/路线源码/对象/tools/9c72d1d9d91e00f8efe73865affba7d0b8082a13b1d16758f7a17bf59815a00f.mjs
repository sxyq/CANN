#!/usr/bin/env node
/* Ranking-first Local Judge V4 and isolated calibration-suite runner. */
import fs from "node:fs";
import path from "node:path";
import crypto from "node:crypto";
import { spawnSync } from "node:child_process";

const CHAMPION = 45.16;
const PARENT_SHA = "a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3";
const RUN_REL = "calibration-v4/runs/20261005-v4-candidates";
const CASES = ["C13", "C14", "C16", "C12", "C01", "C11", "C08"];
const CORE_CASES = ["C13", "C14", "C16", "C12"];
const MAX_ATTEMPTS = 5;
const TARGET_PAIRS = 4;
const WARMUP = 45;
const SAMPLES = 31;
const QUALIFICATION_ATTEMPTS = 2;
const TIMING_DEADLINE_MS = 180000;
const BUILD_DEADLINE_MS = 300000;
const MODELS = ["MODEL_A_MEAN_RATIO", "MODEL_B_GEOMEAN_RATIO", "MODEL_C_BASELINE_STABILITY_WEIGHTED", "ENSEMBLE"];

const CANDIDATES = [
  {
    route: "HOTLOOP-ADDR-HOIST-CHAMPION-X", revision: "V002",
    sha: "40b1548eb0862691e92b10f242e3f98ccf4d232596a2704ddbc25eab7b3aa3f3",
    source: "/home/data4t2/lelinfeng/cann/worktrees/w2/m2/hotloop-addr/本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V002/submission.asc"
  },
  {
    route: "HOTLOOP-BRANCH-HOIST-CHAMPION-X", revision: "V001",
    sha: "4dc1973ef198cd67693610b6ffedad6883b59f52207b9804052975a13f4e032b",
    source: "/home/data4t2/lelinfeng/cann-w2-main2-control/研究/主代理/MAIN-2/calibration-v4/source-cache/HOTLOOP-BRANCH-HOIST-CHAMPION-X/V001/submission.asc"
  },
  {
    route: "REDUCE-FINALIZE-HANDOFF-CHAMPION-X", revision: "V001",
    sha: "83d569d2968b3bcd932b87130a359ba763a0679cf9f94e96e152d16f78338df3",
    source: "/home/data4t2/lelinfeng/cann/worktrees/w2/m2/reduce-finalize/本地实验/REDUCE-FINALIZE-HANDOFF-CHAMPION-X/V001/submission.asc"
  },
  {
    route: "SCHED-CHAMPION-X", revision: "V001",
    sha: "ed232872fa1837678a0a05fc16de9f06a725d61ff9d85663193fdc49e8f33678",
    source: "/home/data4t2/lelinfeng/cann-w2-m2-occupancy/本地实验/SCHED-CHAMPION-X/V001/submission.asc"
  },
  {
    route: "STORE-EPILOGUE-X", revision: "V001",
    sha: "06564134e4930eb3e2ca30297b9bda548135d1aa8a1f43238b75dafc8a8c09f9",
    source: "/home/data4t2/lelinfeng/cann-w2-m1-store/归档/历史工作区/STORE-EPILOGUE-X/V001/submission.asc"
  }
];

const exists = p => { try { fs.accessSync(p); return true; } catch { return false; } };
const num = v => v == null || v === "" || v === "NA" || v === "NaN" ? null : (Number.isFinite(Number(v)) ? Number(v) : null);
const fmt = v => Number.isFinite(v) ? Number(v.toFixed(6)) : "NA";
const key = r => r.route + "/" + r.revision;
const mean = a => { const x = a.filter(Number.isFinite); return x.length ? x.reduce((s, v) => s + v, 0) / x.length : null; };
const median = a => { const x = a.filter(Number.isFinite).sort((u, v) => u - v); if (!x.length) return null; const i = Math.floor(x.length / 2); return x.length % 2 ? x[i] : (x[i - 1] + x[i]) / 2; };
const quantile = (a, q) => { const x = a.filter(Number.isFinite).sort((u, v) => u - v); return x.length ? x[Math.max(0, Math.min(x.length - 1, Math.ceil(q * x.length) - 1))] : null; };
const geo = a => { const x = a.filter(v => Number.isFinite(v) && v > 0); return x.length ? Math.exp(mean(x.map(Math.log))) : null; };
const clamp = x => Number.isFinite(x) ? Math.max(0, Math.min(100, x)) : null;
const bool = x => x ? "YES" : "NO";
const safeName = c => c.route + "__" + c.revision;

function readTsv(file) {
  if (!exists(file)) return [];
  const lines = fs.readFileSync(file, "utf8").replace(/^\uFEFF/, "").split(/\r?\n/).filter(Boolean);
  if (!lines.length) return [];
  const h = lines[0].split("\t");
  return lines.slice(1).map(line => {
    const cells = line.split("\t");
    return Object.fromEntries(h.map((k, i) => [k, cells[i] ?? ""]));
  });
}
function writeTsv(file, headers, rows) {
  fs.mkdirSync(path.dirname(file), { recursive: true });
  const cell = x => x == null || x === "" || (typeof x === "number" && !Number.isFinite(x)) ? "NA" : String(x).replace(/[\t\r\n]+/g, " ");
  fs.writeFileSync(file, [headers.join("\t"), ...rows.map(r => headers.map(h => cell(r[h])).join("\t"))].join("\n") + "\n");
}
function writeText(file, value) {
  fs.mkdirSync(path.dirname(file), { recursive: true });
  fs.writeFileSync(file, value.endsWith("\n") ? value : value + "\n");
}
function sha256(file) { return crypto.createHash("sha256").update(fs.readFileSync(file)).digest("hex"); }
function appendTsv(file, headers, row) {
  fs.mkdirSync(path.dirname(file), { recursive: true });
  if (!exists(file)) fs.writeFileSync(file, headers.join("\t") + "\n");
  const cell = x => x == null || x === "" ? "NA" : String(x).replace(/[\t\r\n]+/g, " ");
  fs.appendFileSync(file, headers.map(h => cell(row[h])).join("\t") + "\n");
}
function parseArgs(argv) {
  const a = { command: argv[0] || "validate", root: process.cwd(), version: null, caseId: null, device: null, input: null, judgeLevel: "NOT_ASSESSED" };
  for (let i = 1; i < argv.length; i++) {
    if (argv[i] === "--root") a.root = path.resolve(argv[++i]);
    else if (argv[i] === "--version") a.version = argv[++i];
    else if (argv[i] === "--case") a.caseId = argv[++i];
    else if (argv[i] === "--device") a.device = Number(argv[++i]);
    else if (argv[i] === "--input") a.input = path.resolve(argv[++i]);
    else if (argv[i] === "--judge-level") a.judgeLevel = argv[++i];
  }
  a.mainDir = path.join(a.root, "研究/主代理/MAIN-2");
  a.runDir = path.join(a.mainDir, RUN_REL);
  return a;
}
function loadData(mainDir) {
  const suite = readTsv(path.join(mainDir, "MAIN2-UNIFIED-LOCAL-SUITE-V2.tsv"));
  const core = suite.filter(r => r.KEEP_FOR_V2 === "PROVISIONAL_CORE_FEATURE").map(r => r.CASE_ID);
  if (CORE_CASES.some(c => !core.includes(c)) || core.length !== CORE_CASES.length) throw new Error("Suite V2 core IDs changed; review V4 feature definition.");
  const official = new Map(readTsv(path.join(mainDir, "ALL-OFFICIAL-RESULTS.tsv")).map(r => [r.ROUTE + "/" + r.REVISION, r]));
  const dataset = readTsv(path.join(mainDir, "LOCAL-JUDGE-CALIBRATION-DATASET-V2.tsv"))
    .filter(r => !(r.ROUTE === "R31B" && r.REVISION === "V011"))
    .map(r => ({
      route: r.ROUTE, revision: r.REVISION, sha: r.SOURCE_SHA, score: num(r.OFFICIAL_SCORE),
      official: official.get(r.ROUTE + "/" + r.REVISION) || null,
      ratios: Object.fromEntries(CORE_CASES.map(c => [c, num(r["LOCAL_" + c + "_RATIO"])]))
    }))
    .filter(r => r.score !== null);
  const raw = readTsv(path.join(mainDir, "calibration-v2/runs/20261004-v2-v011/measurements.tsv"));
  const weights = Object.fromEntries(CORE_CASES.map(c => {
    const rows = raw.filter(r => r.route === "R31B" && r.revision === "V011" && r.case_id === c && num(r.command_rc) === 0 && num(r.bad) === 0);
    const times = rows.map(r => num(r.device_median_us)).filter(x => x > 0);
    const mu = mean(times);
    const cvVal = times.length > 1 && mu ? Math.sqrt(times.reduce((s, x) => s + (x - mu) ** 2, 0) / (times.length - 1)) / mu : null;
    const madRatios = rows.map(r => num(r.device_median_us) > 0 ? num(r.device_mad_us) / num(r.device_median_us) : null).filter(Number.isFinite);
    const madRatio = median(madRatios);
    return [c, cvVal === null || madRatio === null ? 1 : 1 / (1 + cvVal + madRatio)];
  }));
  return { suite, dataset, weights, official };
}
function feature(row, model, weights) {
  const vals = CORE_CASES.map(c => row.ratios[c]);
  if (vals.some(x => !Number.isFinite(x) || x <= 0)) return null;
  if (model === "MODEL_A_MEAN_RATIO") return mean(vals);
  if (model === "MODEL_B_GEOMEAN_RATIO") return geo(vals);
  const denominator = CORE_CASES.reduce((s, c) => s + weights[c], 0);
  return denominator ? Math.exp(CORE_CASES.reduce((s, c) => s + Math.log(row.ratios[c]) * weights[c], 0) / denominator) : null;
}
function fitLine(x, y) {
  const pairs = x.map((v, i) => [v, y[i]]).filter(([a, b]) => Number.isFinite(a) && Number.isFinite(b));
  if (pairs.length < 3) return null;
  const mx = mean(pairs.map(p => p[0])), my = mean(pairs.map(p => p[1]));
  const covariance = pairs.reduce((s, p) => s + (p[0] - mx) * (p[1] - my), 0);
  const variance = pairs.reduce((s, p) => s + (p[0] - mx) ** 2, 0);
  if (!variance) return null;
  const slope = covariance / variance;
  return { slope, intercept: my - slope * mx };
}
function predictModels(train, test, weights) {
  const result = {};
  for (const model of ["MODEL_A_MEAN_RATIO", "MODEL_B_GEOMEAN_RATIO", "MODEL_C_BASELINE_STABILITY_WEIGHTED"]) {
    const fit = fitLine(train.map(r => feature(r, model, weights)), train.map(r => r.score));
    const x = feature(test, model, weights);
    result[model] = fit && x !== null ? clamp(fit.intercept + fit.slope * x) : null;
  }
  const preds = Object.values(result).filter(Number.isFinite);
  result.ENSEMBLE = preds.length === 3 ? mean(preds) : null;
  result.MODEL_SPREAD = preds.length === 3 ? Math.max(...preds) - Math.min(...preds) : null;
  return result;
}
function crossValidate(records, weights, split) {
  const groups = split === "LOO" ? records.map(r => [r]) : [...new Set(records.map(r => r.route))].map(route => records.filter(r => r.route === route));
  const rows = [];
  for (const held of groups) {
    const heldKeys = new Set(held.map(key));
    const train = records.filter(r => split === "LOO" ? !heldKeys.has(key(r)) : !heldKeys.has(r.route));
    for (const test of held) {
      const p = predictModels(train, test, weights);
      rows.push({
        split, fold: split === "LOO" ? key(test) : test.route,
        route: test.route, revision: test.revision, sha: test.sha, actual: test.score,
        trainN: train.length, trainRoutes: new Set(train.map(r => r.route)).size,
        ...p, sourceExclusion: "HELDOUT_VERSION_AND_ROUTE_GROUP_EXCLUDED_FROM_FIT"
      });
    }
  }
  return rows;
}
function ranks(xs) {
  const sorted = xs.map((v, i) => ({ v, i })).sort((a, b) => a.v - b.v), out = Array(xs.length).fill(0);
  for (let i = 0; i < sorted.length;) {
    let j = i + 1; while (j < sorted.length && sorted[j].v === sorted[i].v) j++;
    for (let k = i; k < j; k++) out[sorted[k].i] = (i + j + 1) / 2;
    i = j;
  }
  return out;
}
function corr(x, y, rank = false) {
  const p = x.map((v, i) => [v, y[i]]).filter(([a, b]) => Number.isFinite(a) && Number.isFinite(b));
  if (p.length < 3) return null;
  const xx = rank ? ranks(p.map(q => q[0])) : p.map(q => q[0]);
  const yy = rank ? ranks(p.map(q => q[1])) : p.map(q => q[1]);
  const mx = mean(xx), my = mean(yy);
  const cov = xx.reduce((s, v, i) => s + (v - mx) * (yy[i] - my), 0);
  const vx = xx.reduce((s, v) => s + (v - mx) ** 2, 0), vy = yy.reduce((s, v) => s + (v - my) ** 2, 0);
  return vx && vy ? cov / Math.sqrt(vx * vy) : null;
}
function kendall(x, y) {
  const p = x.map((v, i) => [v, y[i]]).filter(([a, b]) => Number.isFinite(a) && Number.isFinite(b));
  let c = 0, d = 0, tx = 0, ty = 0;
  for (let i = 0; i < p.length; i++) for (let j = i + 1; j < p.length; j++) {
    const a = Math.sign(p[i][0] - p[j][0]), b = Math.sign(p[i][1] - p[j][1]);
    if (!a && !b) continue;
    if (!a) tx++; else if (!b) ty++; else if (a === b) c++; else d++;
  }
  const den = Math.sqrt((c + d + tx) * (c + d + ty));
  return den ? (c - d) / den : null;
}
function pairwise(rows, scoreKey) {
  const out = [];
  const values = rows.filter(r => Number.isFinite(r[scoreKey]) && Number.isFinite(r.actual));
  const actualOrder = [...values].sort((a, b) => b.actual - a.actual);
  const bandCount = Math.max(1, Math.ceil(actualOrder.length * 0.25));
  const near = new Set(actualOrder.slice(0, bandCount).map(key));
  for (let i = 0; i < values.length; i++) for (let j = i + 1; j < values.length; j++) {
    const a = values[i], b = values[j], ps = Math.sign(a[scoreKey] - b[scoreKey]), as = Math.sign(a.actual - b.actual);
    out.push({
      SPLIT: a.split, MODEL: scoreKey, VERSION_A: key(a), VERSION_B: key(b),
      PREDICTED_A: a[scoreKey], PREDICTED_B: b[scoreKey], ACTUAL_A: a.actual, ACTUAL_B: b.actual,
      PREDICTED_ORDER: !ps ? "TIE" : (ps > 0 ? "A>B" : "B>A"),
      ACTUAL_ORDER: as > 0 ? "A>B" : (as < 0 ? "B>A" : "TIE"),
      PAIRWISE_RESULT: !ps ? "TIE" : (ps === as ? "CORRECT" : "WRONG"),
      NEAR_CHAMPION_PAIR: near.has(key(a)) && near.has(key(b)) ? "YES" : "NO"
    });
  }
  return { rows: out, bandCount, bandFloor: actualOrder[bandCount - 1]?.actual ?? null };
}
function pairMetric(rows) {
  return {
    score: rows.length ? rows.reduce((s, r) => s + (r.PAIRWISE_RESULT === "CORRECT" ? 1 : (r.PAIRWISE_RESULT === "TIE" ? 0.5 : 0)), 0) / rows.length : null,
    n: rows.length, ties: rows.filter(r => r.PAIRWISE_RESULT === "TIE").length
  };
}
function metricSummary(rows, model) {
  const valid = rows.filter(r => Number.isFinite(r[model]) && Number.isFinite(r.actual));
  const err = valid.map(r => r[model] - r.actual), p = valid.map(r => r[model]), y = valid.map(r => r.actual);
  const pair = pairMetric(pairwise(valid, model).rows);
  const nearPair = pairMetric(pairwise(valid, model).rows.filter(r => r.NEAR_CHAMPION_PAIR === "YES"));
  const sortedActual = [...valid].sort((a, b) => b.actual - a.actual || key(a).localeCompare(key(b)));
  const topk = {};
  for (const k of [1, 3, 5]) {
    const predicted = [...valid].sort((a, b) => b[model] - a[model] || key(a).localeCompare(key(b))).slice(0, k);
    const truth = sortedActual.slice(0, k), overlap = predicted.filter(a => truth.some(b => key(a) === key(b))).length;
    topk[k] = { hit: k === 1 ? overlap : null, precision: predicted.length ? overlap / predicted.length : null, recall: truth.length ? overlap / truth.length : null };
  }
  const below = valid.filter(r => r.actual < CHAMPION), above = valid.filter(r => r.actual > CHAMPION);
  const fp = valid.filter(r => r[model] > CHAMPION && r.actual < CHAMPION).length;
  return {
    n: valid.length, mae: mean(err.map(Math.abs)), rmse: err.length ? Math.sqrt(mean(err.map(x => x * x))) : null,
    spearman: corr(p, y, true), kendall: kendall(p, y), pairwise: pair.score, pairwiseN: pair.n,
    nearPairwise: nearPair.score, nearPairN: nearPair.n, bandCount: Math.max(1, Math.ceil(valid.length * 0.25)),
    bandFloor: sortedActual[Math.max(0, Math.ceil(valid.length * 0.25) - 1)]?.actual ?? null,
    topk, falsePositives: fp, falsePositiveRate: below.length ? fp / below.length : null,
    falseNegatives: above.filter(r => r[model] <= CHAMPION).length, actualAboveChampion: above.length
  };
}
function writeValidation(args, data) {
  const loo = crossValidate(data.dataset, data.weights, "LOO");
  const routeOut = crossValidate(data.dataset, data.weights, "ROUTE_OUT");
  const modelKeys = MODELS;
  const pairRows = [];
  for (const rows of [loo, routeOut]) for (const model of modelKeys) pairRows.push(...pairwise(rows, model).rows);
  writeTsv(path.join(args.mainDir, "LOCAL-JUDGE-V4-PAIRWISE.tsv"),
    ["SPLIT", "MODEL", "VERSION_A", "VERSION_B", "PREDICTED_A", "PREDICTED_B", "ACTUAL_A", "ACTUAL_B", "PREDICTED_ORDER", "ACTUAL_ORDER", "PAIRWISE_RESULT", "NEAR_CHAMPION_PAIR"],
    pairRows.map(r => ({
      ...r, PREDICTED_A: fmt(r.PREDICTED_A), PREDICTED_B: fmt(r.PREDICTED_B),
      ACTUAL_A: fmt(r.ACTUAL_A), ACTUAL_B: fmt(r.ACTUAL_B)
    })));
  const topRows = [];
  for (const [split, rows] of [["LOO", loo], ["ROUTE_OUT", routeOut]]) for (const model of modelKeys) {
    const s = metricSummary(rows, model);
    for (const k of [1, 3, 5]) {
      const predSet = [...rows].filter(r => Number.isFinite(r[model])).sort((a, b) => b[model] - a[model] || key(a).localeCompare(key(b))).slice(0, k);
      const actualSet = [...rows].filter(r => Number.isFinite(r.actual)).sort((a, b) => b.actual - a.actual || key(a).localeCompare(key(b))).slice(0, k);
      const hit = predSet.filter(a => actualSet.some(b => key(a) === key(b))).length;
      topRows.push({
        SPLIT: split, MODEL: model, K: k, TOP1_HIT: k === 1 ? hit : "NA",
        TOPK_HIT_COUNT: hit, TOPK_RECALL: actualSet.length ? hit / actualSet.length : null,
        TOPK_PRECISION: predSet.length ? hit / predSet.length : null,
        PREDICTED_TOPK: predSet.map(key).join(","), ACTUAL_TOPK: actualSet.map(key).join(","),
        NEAR_CHAMPION_BAND_SIZE: s.bandCount, NEAR_CHAMPION_CUTOFF: fmt(s.bandFloor)
      });
    }
  }
  writeTsv(path.join(args.mainDir, "LOCAL-JUDGE-V4-TOPK.tsv"),
    ["SPLIT", "MODEL", "K", "TOP1_HIT", "TOPK_HIT_COUNT", "TOPK_RECALL", "TOPK_PRECISION", "PREDICTED_TOPK", "ACTUAL_TOPK", "NEAR_CHAMPION_BAND_SIZE", "NEAR_CHAMPION_CUTOFF"],
    topRows.map(r => ({ ...r, TOPK_RECALL: fmt(r.TOPK_RECALL), TOPK_PRECISION: fmt(r.TOPK_PRECISION) })));
  const metrics = {};
  for (const [split, rows] of [["LOO", loo], ["ROUTE_OUT", routeOut]]) {
    metrics[split] = {};
    for (const model of modelKeys) metrics[split][model] = metricSummary(rows, model);
  }
  const ensembleLoo = metrics.LOO.ENSEMBLE, ensembleRoute = metrics.ROUTE_OUT.ENSEMBLE;
  const level = args.judgeLevel;
  const lines = [
    "# Local Judge V4 — Ranking Validation",
    "",
    "Mode: CALIBRATED_SURROGATE. Models A/B/C use only core Suite V2 cases C13/C14/C16/C12: arithmetic mean, geometric mean, and fresh-V011-noise-weighted geometric mean of their local latency ratios. The three diagnostic-only cases never enter the models. Fixed model features avoid label-based feature selection in the V4 outer folds.",
    "",
    "## Suite and near-Champion definition",
    "",
    "- Official candidate labels: " + data.dataset.length + " revisions across " + new Set(data.dataset.map(r => r.route)).size + " Routes; Champion anchor R31B/V011 remains 45.16 and is not a candidate fit label.",
    "- Near-Champion band is the top quartile of the 12 Official candidate scores: " + ensembleLoo.bandCount + " versions; cutoff " + fmt(ensembleLoo.bandFloor) + ". The cutoff is derived from the observed score distribution.",
    "",
    "## Nested version-level LOOCV — Ensemble mean of A/B/C raw predictions",
    "",
    "- n=" + ensembleLoo.n + "; MAE=" + fmt(ensembleLoo.mae) + "; RMSE=" + fmt(ensembleLoo.rmse) + "; Spearman=" + fmt(ensembleLoo.spearman) + "; Kendall=" + fmt(ensembleLoo.kendall) + ".",
    "- Pairwise order accuracy=" + fmt(ensembleLoo.pairwise) + " (" + ensembleLoo.pairwiseN + " pairs). Near-Champion pairwise=" + fmt(ensembleLoo.nearPairwise) + " (" + ensembleLoo.nearPairN + " pairs).",
    "- TOP1 hit=" + fmt(ensembleLoo.topk[1].hit) + "; TOP3 precision/recall=" + fmt(ensembleLoo.topk[3].precision) + "/" + fmt(ensembleLoo.topk[3].recall) + "; TOP5 precision/recall=" + fmt(ensembleLoo.topk[5].precision) + "/" + fmt(ensembleLoo.topk[5].recall) + ".",
    "- Raw false positives=" + ensembleLoo.falsePositives + "; false-positive rate=" + fmt(ensembleLoo.falsePositiveRate) + "; actual scores above Champion=" + ensembleLoo.actualAboveChampion + ".",
    "",
    "## Leave-one-Route-out — Ensemble",
    "",
    "- n=" + ensembleRoute.n + "; MAE=" + fmt(ensembleRoute.mae) + "; RMSE=" + fmt(ensembleRoute.rmse) + "; Spearman=" + fmt(ensembleRoute.spearman) + "; Kendall=" + fmt(ensembleRoute.kendall) + ".",
    "- Pairwise order accuracy=" + fmt(ensembleRoute.pairwise) + " (" + ensembleRoute.pairwiseN + " pairs). Near-Champion pairwise=" + fmt(ensembleRoute.nearPairwise) + " (" + ensembleRoute.nearPairN + " pairs).",
    "- TOP1 hit=" + fmt(ensembleRoute.topk[1].hit) + "; TOP3 precision/recall=" + fmt(ensembleRoute.topk[3].precision) + "/" + fmt(ensembleRoute.topk[3].recall) + "; TOP5 precision/recall=" + fmt(ensembleRoute.topk[5].precision) + "/" + fmt(ensembleRoute.topk[5].recall) + ".",
    "",
    "## Ranking-level assessment",
    "",
    "- Judge level assessment=" + level + ". The evaluator does not automatically assign a level; Main must weigh overall, route-held-out, TOP-K, and near-Champion evidence.",
    "- Level 2 authorizes only limited local exploration, not automatic Online submission. Level 3 requires reliable near-Champion ordering; inspect its pair count and accuracy rather than overall rank alone.",
    "- No actual candidate label exceeds 45.16, so positive-class sensitivity remains unknown. Conservative V3 margins are not used as ranking scores.",
    "",
    ...modelKeys.flatMap(model => [
      "- " + model + " LOO: MAE=" + fmt(metrics.LOO[model].mae) + ", Spearman=" + fmt(metrics.LOO[model].spearman) +
        ", pairwise=" + fmt(metrics.LOO[model].pairwise) + ", near=" + fmt(metrics.LOO[model].nearPairwise) + ", TOP3 recall=" + fmt(metrics.LOO[model].topk[3].recall) + ".",
      "- " + model + " Route-out: MAE=" + fmt(metrics.ROUTE_OUT[model].mae) + ", Spearman=" + fmt(metrics.ROUTE_OUT[model].spearman) +
        ", pairwise=" + fmt(metrics.ROUTE_OUT[model].pairwise) + ", near=" + fmt(metrics.ROUTE_OUT[model].nearPairwise) + ", TOP3 recall=" + fmt(metrics.ROUTE_OUT[model].topk[3].recall) + "."
    ])
  ];
  writeText(path.join(args.mainDir, "LOCAL-JUDGE-V4-VALIDATION.md"), lines.join("\n"));
  writeTsv(path.join(args.runDir, "V4-OOF-PREDICTIONS.tsv"),
    ["SPLIT", "ROUTE", "REVISION", "SOURCE_SHA", "ACTUAL_SCORE", ...modelKeys, "TRAIN_N", "TRAIN_ROUTES", "HELDOUT_EXCLUDED"],
    [...loo, ...routeOut].map(r => ({
      SPLIT: r.split, ROUTE: r.route, REVISION: r.revision, SOURCE_SHA: r.sha, ACTUAL_SCORE: fmt(r.actual),
      ...Object.fromEntries(modelKeys.map(m => [m, fmt(r[m])])),
      TRAIN_N: r.trainN, TRAIN_ROUTES: r.trainRoutes, HELDOUT_EXCLUDED: r.sourceExclusion
    })));
  return { loo, routeOut, metrics, level };
}
function candidateManifest(mainDir) {
  const rows = readTsv(path.join(mainDir, "ONLINE-CALIBRATION-CANDIDATES.tsv"));
  if (rows.length !== CANDIDATES.length) throw new Error("Expected exactly five source-manifest candidates.");
  for (const c of CANDIDATES) {
    if (!exists(c.source)) throw new Error("SOURCE_RECOVERABLE=NO: " + c.source);
    const actual = sha256(c.source);
    if (actual !== c.sha) throw new Error("SOURCE_SHA_MISMATCH " + key(c) + " expected=" + c.sha + " actual=" + actual);
  }
  return CANDIDATES.map(c => {
    const inv = rows.find(r => r.ROUTE === c.route && r.REVISION === c.revision) || {};
    return { ...c, directParent: "R31B/V011", inventory: inv };
  });
}
function stagePath(args, candidate) { return path.join(args.runDir, "stages", safeName(candidate)); }
function prepareStage(args, candidate) {
  const template = path.join(args.mainDir, "calibration-v2/runs/20261004-v2-v011/stages/R31B__V011");
  const stage = stagePath(args, candidate);
  if (!exists(candidate.source) || sha256(candidate.source) !== candidate.sha) throw new Error("Candidate source missing or SHA mismatch: " + key(candidate));
  if (exists(stage)) {
    if (!exists(path.join(stage, "submission.asc")) || sha256(path.join(stage, "submission.asc")) !== candidate.sha ||
        !exists(path.join(stage, "parent.asc")) || sha256(path.join(stage, "parent.asc")) !== PARENT_SHA) {
      throw new Error("Existing stage identity mismatch; refusing to overwrite " + stage);
    }
    return stage;
  }
  fs.mkdirSync(stage, { recursive: true });
  const files = ["CMakeLists.txt", "runner_ref.inc", "local_types.h", "runner_ref_parent.asc", "runner_ref_candidate.asc",
    "runner_parent.asc", "runner_candidate.asc", "submission_shim.asc", "parent.asc", "main.asc"];
  for (const file of files) {
    const src = path.join(template, file);
    if (!exists(src)) throw new Error("Reference runner input missing: " + src);
    fs.copyFileSync(src, path.join(stage, file));
  }
  if (exists(path.join(template, "runner_main.inc"))) fs.copyFileSync(path.join(template, "runner_main.inc"), path.join(stage, "runner_main.inc"));
  fs.copyFileSync(candidate.source, path.join(stage, "submission.asc"));
  if (sha256(path.join(stage, "parent.asc")) !== PARENT_SHA || sha256(path.join(stage, "submission.asc")) !== candidate.sha) {
    throw new Error("Prepared stage source identity mismatch: " + key(candidate));
  }
  writeTsv(path.join(stage, "source-meta.tsv"),
    ["ROUTE", "REVISION", "DIRECT_PARENT", "PARENT_SHA", "SOURCE_SHA", "SOURCE_PATH", "RUNNER_TEMPLATE"],
    [{
      ROUTE: candidate.route, REVISION: candidate.revision, DIRECT_PARENT: "R31B/V011",
      PARENT_SHA, SOURCE_SHA: candidate.sha, SOURCE_PATH: candidate.source,
      RUNNER_TEMPLATE: "calibration-v2/runs/20261004-v2-v011/stages/R31B__V011"
    }]);
  return stage;
}
function shellQuote(s) { return "'" + String(s).replace(/'/g, "'\\''") + "'"; }
function runShell(command, cwd, timeout = BUILD_DEADLINE_MS) {
  return spawnSync("/bin/bash", ["-lc", command], { cwd, encoding: "utf8", timeout, maxBuffer: 16 * 1024 * 1024 });
}
function buildCandidate(args, candidate) {
  const stage = prepareStage(args, candidate);
  const build = path.join(stage, "build-v4");
  const envScript = "/usr/local/Ascend/ascend-toolkit/set_env.sh";
  const hcc = "$ASCEND_HOME_PATH/toolkit/toolchain/hcc";
  const hostIncludes = "${ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0:${ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu:${ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/backward:${ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include";
  const toolchainEnv = "export CPLUS_INCLUDE_PATH=" + hostIncludes + ":${CPLUS_INCLUDE_PATH:-}; export C_INCLUDE_PATH=" + hostIncludes + ":${C_INCLUDE_PATH:-}";
  const configure = "source " + shellQuote(envScript) + " >/dev/null 2>&1 && " + toolchainEnv + " && cmake -S " + shellQuote(stage) +
    " -B " + shellQuote(build) + " -DNPU_ARCH=dav-2201";
  const buildCmd = "source " + shellQuote(envScript) + " >/dev/null 2>&1 && " + toolchainEnv + " && cmake --build " + shellQuote(build) +
    " --target clx_device clx_ref_parent_probe clx_ref_candidate_probe --parallel 1";
  const before = runShell("uptime; free -h; df -h /home/data4t2/lelinfeng/cann", args.root, 10000);
  const conf = runShell(configure, stage);
  fs.mkdirSync(path.join(stage, "evidence", "build"), { recursive: true });
  fs.writeFileSync(path.join(stage, "evidence", "build", "cmake-configure.log"), (conf.stdout || "") + (conf.stderr || ""));
  const built = conf.status === 0 ? runShell(buildCmd, stage) : null;
  fs.writeFileSync(path.join(stage, "evidence", "build", "build.log"), built ? (built.stdout || "") + (built.stderr || "") : "BUILD_SKIPPED_CONFIGURE_FAILED\n");
  const after = runShell("uptime; free -h; df -h /home/data4t2/lelinfeng/cann", args.root, 10000);
  fs.writeFileSync(path.join(stage, "evidence", "build", "host-snapshot-before.txt"), (before.stdout || "") + (before.stderr || ""));
  fs.writeFileSync(path.join(stage, "evidence", "build", "host-snapshot-after.txt"), (after.stdout || "") + (after.stderr || ""));
  const parentExe = path.join(build, "clx_ref_parent_probe"), candidateExe = path.join(build, "clx_ref_candidate_probe");
  const ok = conf.status === 0 && built?.status === 0 && exists(parentExe) && exists(candidateExe);
  const row = {
    ROUTE: candidate.route, REVISION: candidate.revision, SOURCE_SHA: candidate.sha,
    PARENT_SHA, BUILD_STATUS: ok ? "PASS" : "BUILD_FAILED",
    CONFIGURE_RC: conf.status ?? "NO_STATUS", BUILD_RC: built?.status ?? "NOT_RUN",
    PARENT_RUNNER_SHA: exists(parentExe) ? sha256(parentExe) : "NA",
    CANDIDATE_RUNNER_SHA: exists(candidateExe) ? sha256(candidateExe) : "NA",
    BUILD_LOG: path.join(stage, "evidence", "build", "build.log")
  };
  const file = path.join(args.runDir, "build-status.tsv");
  const old = readTsv(file).filter(r => !(r.ROUTE === candidate.route && r.REVISION === candidate.revision));
  writeTsv(file, ["ROUTE", "REVISION", "SOURCE_SHA", "PARENT_SHA", "BUILD_STATUS", "CONFIGURE_RC", "BUILD_RC", "PARENT_RUNNER_SHA", "CANDIDATE_RUNNER_SHA", "BUILD_LOG"], [...old, row]);
  return { stage, row };
}
function parseDeviceSnapshot(text, device) {
  const lines = text.split(/\r?\n/);
  for (let i = 0; i < lines.length - 1; i++) {
    if (new RegExp("^\\|\\s*" + device + "\\s+910B3").test(lines[i])) {
      const m = lines[i + 1].match(/(\d+)\s*\/\s*(\d+)\s*\|/);
      if (m) return { used: Number(m[1]), total: Number(m[2]), free: Number(m[2]) - Number(m[1]) };
    }
  }
  return null;
}
function liveNpu(device) {
  const r = spawnSync("npu-smi", ["info"], { encoding: "utf8", timeout: 15000, maxBuffer: 8 * 1024 * 1024 });
  if (r.status !== 0) throw new Error("npu-smi info failed: " + (r.stderr || "unknown"));
  const parsed = parseDeviceSnapshot(r.stdout, device);
  if (!parsed) throw new Error("Could not parse HBM for NPU " + device);
  return { text: r.stdout, ...parsed };
}
function activeLeases(file) {
  const rows = readTsv(file), last = new Map();
  for (const row of rows) last.set(row.lease_id, row);
  return [...last.values()].filter(r => r.status === "LEASED");
}
function acquireLease(args, candidate, device) {
  const leasesFile = path.join(args.root, "调度/服务器设备使用.tsv");
  const live = liveNpu(device);
  if (live.free < 100) throw new Error("HBM_BLOCKED_FREE_LT_100MB device=" + device + " free=" + live.free);
  if (live.used >= live.total) throw new Error("TIMING_BLOCKED_HBM_FULL device=" + device);
  const conflict = activeLeases(leasesFile).find(r => Number(r.device) === device);
  if (conflict) throw new Error("LEASE_CONFLICT device=" + device + " lease_id=" + conflict.lease_id + " owner=" + conflict.owner);
  const id = "M2-V4-" + candidate.revision + "-D" + device + "-" + Date.now();
  const row = {
    device, owner: "MAIN-2", route: candidate.route, lease_id: id, status: "LEASED",
    start_time: new Date().toISOString(), end_time: "-",
    note: "Local Judge V4 calibration-only; exact historical source; device-event suite; no Kernel edit or Online submission."
  };
  appendTsv(leasesFile, ["device", "owner", "route", "lease_id", "status", "start_time", "end_time", "note"], row);
  return { id, file: leasesFile };
}
function releaseLease(lease, candidate, device, note) {
  const now = new Date().toISOString();
  appendTsv(lease.file, ["device", "owner", "route", "lease_id", "status", "start_time", "end_time", "note"], {
    device, owner: "MAIN-2", route: candidate.route, lease_id: lease.id, status: "RELEASED",
    start_time: now, end_time: now, note
  });
}
function runnerPath(stage, variant) { return path.join(stage, "build-v4", "clx_ref_" + variant + "_probe"); }
function invokeRunner(stage, variant, device, c, prefix, warmup, samples, blocks, gap) {
  const exe = runnerPath(stage, variant);
  if (!exists(exe)) return { rc: "RUNNER_MISSING", stats: null, exe };
  fs.mkdirSync(path.dirname(prefix), { recursive: true });
  const argv = [String(device), String(c.ROWS), String(c.WIDTH), String(c.DTYPE === "FP32" ? 0 : (c.DTYPE === "FP16" ? 1 : 2)),
    prefix, String(warmup), String(samples), String(blocks), String(gap)];
  const envScript = "/usr/local/Ascend/ascend-toolkit/set_env.sh";
  const run = spawnSync("/bin/bash", ["-lc", "source " + shellQuote(envScript) + " >/dev/null 2>&1 && exec \"$@\"", "v4-runner", exe, ...argv],
    { cwd: stage, encoding: "utf8", timeout: TIMING_DEADLINE_MS, maxBuffer: 4 * 1024 * 1024 });
  fs.writeFileSync(prefix + ".stdout.log", run.stdout || "");
  fs.writeFileSync(prefix + ".stderr.log", run.stderr || "");
  fs.writeFileSync(prefix + ".rc.txt", run.status == null ? "TIMEOUT_OR_SIGNAL:" + String(run.signal) : String(run.status));
  return {
    rc: run.status == null ? (run.error ? "SPAWN_ERROR:" + run.error.message : "TIMEOUT_OR_SIGNAL:" + String(run.signal)) : run.status,
    stats: statsFile(prefix), exe
  };
}
function statsFile(prefix) {
  const file = prefix + "-stats.txt";
  if (!exists(file)) return null;
  const map = {};
  for (const line of fs.readFileSync(file, "utf8").split(/\r?\n/).filter(Boolean)) {
    const c = line.split("\t");
    // Runner summary uses a two-column global `bad` row and three-column
    // metric rows such as `ALL_DEVICE\tmedian_us\t...`.
    if (c.length === 2) {
      map[c[0]] = num(c[1]);
      if (c[0] === "bad") map.ALL_DEVICE_bad = num(c[1]);
    }
    else if (c.length >= 3) map[c[0] + "_" + c[1]] = num(c[2]);
  }
  return map;
}
function qualify(s) {
  if (!s || s.ALL_DEVICE_bad !== 0) return { pass: false, reason: "PARENT_CORRECTNESS_OR_STATS_FAIL" };
  const b1m = s.B1_DEVICE_median_us, b2m = s.B2_DEVICE_median_us;
  if (!(b1m > 0 && b2m > 0 && s.ALL_DEVICE_median_us > 0)) return { pass: false, reason: "BLOCK_MEDIAN_MISSING" };
  const drift = Math.abs(b1m - b2m) / ((b1m + b2m) / 2);
  const fullMadRatio = s.ALL_DEVICE_MAD_us / s.ALL_DEVICE_median_us;
  const pass = s.B1_DEVICE_CV <= 0.15 && s.B2_DEVICE_CV <= 0.15 &&
    s.B1_DEVICE_max_min <= 1.30 && s.B2_DEVICE_max_min <= 1.30 &&
    fullMadRatio <= 0.10 && drift <= 0.10;
  return { pass, reason: pass ? "PASS" : "WINDOW_OR_SAME_BINARY_NOISE_FLOOR", drift, fullMadRatio };
}
function buildStatus(args, candidate) {
  return readTsv(path.join(args.runDir, "build-status.tsv")).find(r => r.ROUTE === candidate.route && r.REVISION === candidate.revision);
}
function statusRows(file, route, revision, caseId) {
  return readTsv(file).filter(r => r.ROUTE === route && r.REVISION === revision && (!caseId || r.CASE_ID === caseId));
}
function correctnessSuite(args, candidate, device) {
  const build = buildStatus(args, candidate);
  if (!build || build.BUILD_STATUS !== "PASS") throw new Error("BUILD_PASS_REQUIRED before correctness: " + key(candidate));
  const stage = stagePath(args, candidate), resultFile = path.join(args.runDir, "correctness-status.tsv");
  const cases = readTsv(path.join(args.mainDir, "MAIN2-UNIFIED-LOCAL-SUITE-V2.tsv")).filter(r => CASES.includes(r.CASE_ID));
  const results = [];
  for (const c of cases) {
    const old = readTsv(resultFile).find(r => r.ROUTE === candidate.route && r.REVISION === candidate.revision && r.CASE_ID === c.CASE_ID);
    if (old?.CORRECTNESS_STATUS === "PASS") { results.push(old); continue; }
    const dir = path.join(args.runDir, "stages", safeName(candidate), "evidence", c.CASE_ID);
    fs.mkdirSync(dir, { recursive: true });
    const before = liveNpu(device);
    writeText(path.join(dir, "correctness-npu-before.txt"), before.text);
    const parent = invokeRunner(stage, "parent", device, c, path.join(dir, "preflight-parent"), 0, 1, 1, 0);
    const candidateResult = invokeRunner(stage, "candidate", device, c, path.join(dir, "preflight-candidate"), 0, 1, 1, 0);
    const after = liveNpu(device);
    writeText(path.join(dir, "correctness-npu-after.txt"), after.text);
    const pb = parent.stats?.ALL_DEVICE_bad, cb = candidateResult.stats?.ALL_DEVICE_bad;
    const pass = parent.rc === 0 && candidateResult.rc === 0 && pb === 0 && cb === 0;
    const classification = pass ? "PASS" : (pb > 0 && cb > 0 ? "PARENT_AND_CANDIDATE_SHARED_LIMITATION" :
      (pb === 0 && cb > 0 ? "CORRECTNESS_FAILED_CANDIDATE" : "CORRECTNESS_INCOMPLETE_OR_PARENT_BLOCKER"));
    const row = {
      ROUTE: candidate.route, REVISION: candidate.revision, SOURCE_SHA: candidate.sha, CASE_ID: c.CASE_ID,
      DEVICE: device, PARENT_RC: parent.rc, PARENT_BAD: pb, CANDIDATE_RC: candidateResult.rc, CANDIDATE_BAD: cb,
      CORRECTNESS_STATUS: classification, PARENT_SHA, PARENT_EXE_SHA: exists(parent.exe) ? sha256(parent.exe) : "NA",
      CANDIDATE_EXE_SHA: exists(candidateResult.exe) ? sha256(candidateResult.exe) : "NA",
      PARENT_STATS: path.join(dir, "preflight-parent-stats.txt"), CANDIDATE_STATS: path.join(dir, "preflight-candidate-stats.txt")
    };
    const oldRows = readTsv(resultFile).filter(r => !(r.ROUTE === candidate.route && r.REVISION === candidate.revision && r.CASE_ID === c.CASE_ID));
    writeTsv(resultFile,
      ["ROUTE", "REVISION", "SOURCE_SHA", "CASE_ID", "DEVICE", "PARENT_RC", "PARENT_BAD", "CANDIDATE_RC", "CANDIDATE_BAD", "CORRECTNESS_STATUS", "PARENT_SHA", "PARENT_EXE_SHA", "CANDIDATE_EXE_SHA", "PARENT_STATS", "CANDIDATE_STATS"],
      [...oldRows, row]);
    results.push(row);
    console.log(key(candidate) + " " + c.CASE_ID + " correctness=" + classification + " rc(P,C)=" + parent.rc + "," + candidateResult.rc + " bad(P,C)=" + pb + "," + cb);
  }
  return results;
}
function runCell(args, candidate, stage, c, device) {
  const correctness = readTsv(path.join(args.runDir, "correctness-status.tsv")).find(r =>
    r.ROUTE === candidate.route && r.REVISION === candidate.revision && r.CASE_ID === c.CASE_ID);
  if (!correctness || correctness.CORRECTNESS_STATUS !== "PASS") {
    return { status: correctness?.CORRECTNESS_STATUS || "CORRECTNESS_NOT_RUN", validPairs: 0, attempts: 0 };
  }
  const cellDir = path.join(args.runDir, "stages", safeName(candidate), "evidence", c.CASE_ID);
  fs.mkdirSync(cellDir, { recursive: true });
  const qFile = path.join(args.runDir, "qualification-status.tsv");
  // A qualification failure is device-local evidence.  Permit the bounded
  // retry on one other safe device without reusing or erasing the first
  // device's failed window.
  let qRows = statusRows(qFile, candidate.route, candidate.revision, c.CASE_ID)
    .filter(r => Number(r.DEVICE) === device);
  let qPass = qRows.find(r => r.RESULT === "PASS");
  while (!qPass && qRows.length < QUALIFICATION_ATTEMPTS) {
    const attempt = qRows.length + 1;
    const snap = liveNpu(device);
    writeText(path.join(cellDir, "qualification-" + attempt + "-npu-before.txt"), snap.text);
    const prefix = path.join(cellDir, "qualification-" + attempt + "-parent");
    const parent = invokeRunner(stage, "parent", device, c, prefix, WARMUP, SAMPLES, 2, 1);
    const after = liveNpu(device);
    writeText(path.join(cellDir, "qualification-" + attempt + "-npu-after.txt"), after.text);
    const q = qualify(parent.stats);
    const row = {
      ROUTE: candidate.route, REVISION: candidate.revision, SOURCE_SHA: candidate.sha, CASE_ID: c.CASE_ID,
      DEVICE: device, ATTEMPT: attempt, RC: parent.rc, PARENT_BAD: parent.stats?.ALL_DEVICE_bad,
      BLOCK1_CV: parent.stats?.B1_DEVICE_CV, BLOCK1_MAX_MIN: parent.stats?.B1_DEVICE_max_min,
      BLOCK2_CV: parent.stats?.B2_DEVICE_CV, BLOCK2_MAX_MIN: parent.stats?.B2_DEVICE_max_min,
      ALL_MAD_RATIO: parent.stats?.ALL_DEVICE_MAD_us / parent.stats?.ALL_DEVICE_median_us,
      BLOCK_DRIFT: q.drift, RESULT: q.pass ? "PASS" : "NOT_QUALIFIED", REASON: q.reason, PREFIX: prefix
    };
    appendTsv(qFile,
      ["ROUTE", "REVISION", "SOURCE_SHA", "CASE_ID", "DEVICE", "ATTEMPT", "RC", "PARENT_BAD", "BLOCK1_CV", "BLOCK1_MAX_MIN", "BLOCK2_CV", "BLOCK2_MAX_MIN", "ALL_MAD_RATIO", "BLOCK_DRIFT", "RESULT", "REASON", "PREFIX"], row);
    qRows = statusRows(qFile, candidate.route, candidate.revision, c.CASE_ID)
      .filter(r => Number(r.DEVICE) === device);
    qPass = qRows.find(r => r.RESULT === "PASS");
    console.log(key(candidate) + " " + c.CASE_ID + " parent qualification=" + row.RESULT + " attempt=" + attempt);
  }
  if (!qPass) return { status: "MEASUREMENT_BLOCKED_PARENT_QUALIFICATION", validPairs: 0, attempts: 0 };

  const pairFile = path.join(args.runDir, "pair-status.tsv");
  let prior = statusRows(pairFile, candidate.route, candidate.revision, c.CASE_ID)
    .filter(r => Number(r.DEVICE) === device);
  let validPairs = prior.filter(r => r.VALID_PAIR === "YES").length;
  let attempts = prior.length ? Math.max(...prior.map(r => Number(r.ATTEMPT) || 0)) : 0;
  while (attempts < MAX_ATTEMPTS && validPairs < TARGET_PAIRS) {
    attempts++;
    const before = liveNpu(device);
    writeText(path.join(cellDir, "pair-" + attempts + "-npu-before.txt"), before.text);
    const first = attempts % 2 === 1 ? "parent" : "candidate";
    const second = first === "parent" ? "candidate" : "parent";
    const prefix1 = path.join(cellDir, "pair-" + String(attempts).padStart(2, "0") + "-" + first);
    const prefix2 = path.join(cellDir, "pair-" + String(attempts).padStart(2, "0") + "-" + second);
    const r1 = invokeRunner(stage, first, device, c, prefix1, WARMUP, SAMPLES, 1, 0);
    const r2 = invokeRunner(stage, second, device, c, prefix2, WARMUP, SAMPLES, 1, 0);
    const after = liveNpu(device);
    writeText(path.join(cellDir, "pair-" + attempts + "-npu-after.txt"), after.text);
    const p = first === "parent" ? r1 : r2, q = first === "candidate" ? r1 : r2;
    const pMed = p.stats?.ALL_DEVICE_median_us, qMed = q.stats?.ALL_DEVICE_median_us;
    const valid = p.rc === 0 && q.rc === 0 && p.stats?.ALL_DEVICE_bad === 0 && q.stats?.ALL_DEVICE_bad === 0 && pMed > 0 && qMed > 0;
    if (valid) validPairs++;
    appendTsv(pairFile,
      ["ROUTE", "REVISION", "SOURCE_SHA", "CASE_ID", "DEVICE", "ATTEMPT", "ORDER", "PARENT_RC", "CANDIDATE_RC", "PARENT_BAD", "CANDIDATE_BAD", "PARENT_MEDIAN_US", "CANDIDATE_MEDIAN_US", "LATENCY_RATIO", "VALID_PAIR", "PARENT_PREFIX", "CANDIDATE_PREFIX"],
      {
        ROUTE: candidate.route, REVISION: candidate.revision, SOURCE_SHA: candidate.sha, CASE_ID: c.CASE_ID,
        DEVICE: device, ATTEMPT: attempts, ORDER: first + "->" + second, PARENT_RC: p.rc, CANDIDATE_RC: q.rc,
        PARENT_BAD: p.stats?.ALL_DEVICE_bad, CANDIDATE_BAD: q.stats?.ALL_DEVICE_bad,
        PARENT_MEDIAN_US: pMed, CANDIDATE_MEDIAN_US: qMed, LATENCY_RATIO: pMed > 0 && qMed > 0 ? qMed / pMed : null,
        VALID_PAIR: valid ? "YES" : "NO", PARENT_PREFIX: first === "parent" ? prefix1 : prefix2,
        CANDIDATE_PREFIX: first === "candidate" ? prefix1 : prefix2
      });
    console.log(key(candidate) + " " + c.CASE_ID + " timing attempt " + attempts + "/" + MAX_ATTEMPTS +
      " valid_pairs=" + validPairs + "/" + TARGET_PAIRS + " RC(P,C)=" + p.rc + "," + q.rc);
    prior = statusRows(pairFile, candidate.route, candidate.revision, c.CASE_ID)
      .filter(r => Number(r.DEVICE) === device);
  }
  const result = validPairs >= TARGET_PAIRS ? "MEASURED_4_VALID_PAIRS" :
    (validPairs >= 3 ? "PARTIAL_3_VALID_PAIRS" : "MEASUREMENT_BLOCKED_INSUFFICIENT_VALID_PAIRS");
  return { status: result, validPairs, attempts };
}
function runCandidateSuite(args, candidate, device) {
  const build = buildStatus(args, candidate);
  if (!build || build.BUILD_STATUS !== "PASS") throw new Error("BUILD_PASS_REQUIRED: " + key(candidate));
  const dir = path.join(args.mainDir, "MAIN2-UNIFIED-LOCAL-SUITE-V2.tsv");
  const cases = readTsv(dir).filter(r => CASES.includes(r.CASE_ID));
  if (cases.length !== CASES.length) throw new Error("Suite V2 did not resolve all seven cases.");
  const lease = acquireLease(args, candidate, device);
  let leaseNote = "candidate suite completed";
  const results = [];
  try {
    // Correctness is a hard per-case eligibility gate; never let runCell
    // silently turn an untested case into a timing attempt.
    correctnessSuite(args, candidate, device);
    for (const c of cases) {
      const result = runCell(args, candidate, stagePath(args, candidate), c, device);
      results.push({ route: candidate.route, revision: candidate.revision, caseId: c.CASE_ID, ...result });
    }
  } catch (error) {
    leaseNote = "suite stopped: " + error.message;
    throw error;
  } finally {
    releaseLease(lease, candidate, device, leaseNote);
  }
  return results;
}
function sampleMadRatio(values) {
  const m = median(values);
  return m > 0 ? median(values.map(x => Math.abs(x - m))) / m : null;
}
function vectorize(args) {
  const pairRows = readTsv(path.join(args.runDir, "pair-status.tsv"));
  const qRows = readTsv(path.join(args.runDir, "qualification-status.tsv"));
  const correctnessRows = readTsv(path.join(args.runDir, "correctness-status.tsv"));
  const suite = readTsv(path.join(args.mainDir, "MAIN2-UNIFIED-LOCAL-SUITE-V2.tsv"));
  const data = [];
  for (const candidate of candidateManifest(args.mainDir)) {
    for (const c of suite.filter(r => CASES.includes(r.CASE_ID))) {
      const pairs = pairRows.filter(r => r.ROUTE === candidate.route && r.REVISION === candidate.revision && r.CASE_ID === c.CASE_ID);
      const valid = pairs.filter(r => r.VALID_PAIR === "YES");
      const q = qRows.filter(r => r.ROUTE === candidate.route && r.REVISION === candidate.revision && r.CASE_ID === c.CASE_ID);
      const correct = correctnessRows.find(r => r.ROUTE === candidate.route && r.REVISION === candidate.revision && r.CASE_ID === c.CASE_ID);
      const parentUs = valid.map(r => num(r.PARENT_MEDIAN_US)).filter(x => x > 0);
      const candidateUs = valid.map(r => num(r.CANDIDATE_MEDIAN_US)).filter(x => x > 0);
      const ratios = valid.map(r => num(r.LATENCY_RATIO)).filter(x => x > 0);
      const parentMedian = median(parentUs), candidateMedian = median(candidateUs), latencyRatio = median(ratios);
      const parentMadRatio = sampleMadRatio(parentUs), candidateMadRatio = sampleMadRatio(candidateUs);
      const parentThroughput = parentMedian > 0 ? Number(c.ROWS) * Number(c.WIDTH) * 1e6 / parentMedian : null;
      const candidateThroughput = candidateMedian > 0 ? Number(c.ROWS) * Number(c.WIDTH) * 1e6 / candidateMedian : null;
      const lastQualification = q.at(-1);
      const quality = correct?.CORRECTNESS_STATUS !== "PASS" ? "CORRECTNESS_NOT_PASS" :
        (lastQualification?.RESULT !== "PASS" ? "PARENT_NOT_QUALIFIED" :
          (valid.length < TARGET_PAIRS ? "INCOMPLETE_VALID_PAIR_COUNT" :
            (parentMadRatio <= 0.10 && candidateMadRatio <= 0.10 ? "MEASUREMENT_STABLE" : "JITTER_OR_PARTIAL")));
      const faster = ratios.filter(x => x < 1).length;
      data.push({
        ROUTE: candidate.route, REVISION: candidate.revision, VERSION: key(candidate), SOURCE_SHA: candidate.sha,
        DIRECT_PARENT: "R31B/V011", PARENT_SHA, SOURCE_PATH: candidate.source,
        CASE_ID: c.CASE_ID, DTYPE: c.DTYPE, ROWS: c.ROWS, WIDTH: c.WIDTH,
        CASE_ROLE: c.KEEP_FOR_V2, EXPECTED_PATH: c.EXPECTED_PATH,
        DEVICE: pairs.at(-1)?.DEVICE || q.at(-1)?.DEVICE || correct?.DEVICE || "NA",
        PARENT_QUALIFICATION: lastQualification?.RESULT || "NOT_RUN", PARENT_QUALIFICATION_ATTEMPTS: q.length,
        CORRECTNESS_STATUS: correct?.CORRECTNESS_STATUS || "NOT_RUN",
        ATTEMPTS: pairs.length, VALID_PAIRS: valid.length, REQUIRED_VALID_PAIRS: TARGET_PAIRS,
        PARENT_AVG_LATENCY_US: mean(parentUs), PARENT_MEDIAN_LATENCY_US: parentMedian,
        CANDIDATE_AVG_LATENCY_US: mean(candidateUs), CANDIDATE_MEDIAN_LATENCY_US: candidateMedian,
        LATENCY_RATIO: latencyRatio, LOCAL_DELTA_PERCENT: latencyRatio === null ? null : (latencyRatio - 1) * 100,
        PARENT_THROUGHPUT_ELEMENTS_S: parentThroughput, CANDIDATE_THROUGHPUT_ELEMENTS_S: candidateThroughput,
        THROUGHPUT_RATIO: parentThroughput && candidateThroughput ? candidateThroughput / parentThroughput : null,
        PARENT_MAD_MEDIAN_RATIO: parentMadRatio, CANDIDATE_MAD_MEDIAN_RATIO: candidateMadRatio,
        LOCAL_FASTER_PAIRS: faster, LOCAL_SIGN_STABILITY: ratios.length ? Math.max(faster, ratios.length - faster) / ratios.length : null,
        QUALITY: quality, SOURCE_IDENTITY: sha256(candidate.source) === candidate.sha ? "PASS" : "FAIL",
        RUNNER_PROTOCOL: "ACL_DEVICE_EVENT;45_WARMUP;31_SAMPLES;4_VALID_INTERLEAVED_PAIRS;MAX5_ATTEMPTS"
      });
    }
  }
  const file = path.join(args.mainDir, "CALIBRATION-CANDIDATES-VECTORS.tsv");
  const headers = Object.keys(data[0] || {});
  writeTsv(file, headers, data.map(r => Object.fromEntries(headers.map(h => [h, typeof r[h] === "number" ? fmt(r[h]) : r[h]]))));
  return data;
}
function candidateFeatureMap(rows) {
  const m = new Map();
  for (const r of rows) {
    if (!m.has(r.VERSION)) m.set(r.VERSION, { route: r.ROUTE, revision: r.REVISION, sha: r.SOURCE_SHA, ratios: {}, cells: [] });
    const v = m.get(r.VERSION);
    v.ratios[r.CASE_ID] = r.QUALITY === "MEASUREMENT_STABLE" ? num(r.LATENCY_RATIO) : null;
    v.cells.push(r);
  }
  return [...m.values()];
}
function distanceToTraining(candidate, records) {
  if (CORE_CASES.some(c => !Number.isFinite(candidate.ratios[c]))) return null;
  const means = Object.fromEntries(CORE_CASES.map(c => [c, mean(records.map(r => r.ratios[c]))]));
  const sds = Object.fromEntries(CORE_CASES.map(c => {
    const x = records.map(r => r.ratios[c]).filter(Number.isFinite), m = mean(x);
    return [c, x.length > 1 ? Math.sqrt(x.reduce((s, v) => s + (v - m) ** 2, 0) / (x.length - 1)) : 1];
  }));
  const distances = records.map(r => Math.sqrt(CORE_CASES.reduce((s, c) => s + ((candidate.ratios[c] - r.ratios[c]) / (sds[c] || 1)) ** 2, 0)));
  return distances.length ? Math.min(...distances) : null;
}
function rankCandidates(args, data, cv) {
  const vectorRows = readTsv(path.join(args.mainDir, "CALIBRATION-CANDIDATES-VECTORS.tsv"));
  const candidates = candidateFeatureMap(vectorRows);
  const oofResiduals = cv.loo.map(r => Math.abs(r.ENSEMBLE - r.actual)).filter(Number.isFinite);
  const uncertainty = quantile(oofResiduals, 0.90);
  const predictions = candidates.map(candidate => {
    const model = predictModels(data.dataset, candidate, data.weights);
    const allCells = candidate.cells;
    const coreCells = allCells.filter(r => r.CASE_ROLE === "PROVISIONAL_CORE_FEATURE");
    const diagCells = allCells.filter(r => r.CASE_ROLE === "DIAGNOSTIC_COVERAGE_ONLY");
    const coreComplete = coreCells.length === CORE_CASES.length && coreCells.every(r => r.QUALITY === "MEASUREMENT_STABLE");
    const correctnessComplete = allCells.length === CASES.length && allCells.every(r => r.CORRECTNESS_STATUS === "PASS");
    const sourceExact = CANDIDATES.some(c => c.route === candidate.route && c.revision === candidate.revision && c.sha === candidate.sha &&
      exists(c.source) && sha256(c.source) === c.sha);
    const build = buildStatus(args, { route: candidate.route, revision: candidate.revision });
    const localRunnable = sourceExact && build?.BUILD_STATUS === "PASS" && correctnessComplete;
    const pred = model.ENSEMBLE;
    const nearestOfficialScore = pred === null ? null : Math.min(...data.dataset.map(r => Math.abs(pred - r.score)));
    const knownRoutes = new Set(data.dataset.map(r => r.route));
    return {
      VERSION: key(candidate), ROUTE: candidate.route, REVISION: candidate.revision, SOURCE_SHA: candidate.sha,
      SOURCE_RECOVERABLE: bool(sourceExact), LOCAL_RUNNABLE: localRunnable ? "YES" : "NO",
      LOCAL_VECTOR_STATUS: coreComplete ? "CORE_VECTOR_COMPLETE" : "MEASUREMENT_BLOCKED_OR_INCOMPLETE",
      CORE_VALID_CASES: coreCells.filter(r => r.QUALITY === "MEASUREMENT_STABLE").map(r => r.CASE_ID).join(","),
      DIAGNOSTIC_VALID_CASES: diagCells.filter(r => r.QUALITY === "MEASUREMENT_STABLE").map(r => r.CASE_ID).join(","),
      MODEL_A_PRED: model.MODEL_A_MEAN_RATIO, MODEL_B_PRED: model.MODEL_B_GEOMEAN_RATIO,
      MODEL_C_PRED: model.MODEL_C_BASELINE_STABILITY_WEIGHTED, RAW_PREDICTED_SCORE: pred,
      PREDICTION_SPREAD: model.MODEL_SPREAD, UNCERTAINTY_OOF_Q90_ABS_ERROR: uncertainty,
      PREDICTION_LOW: pred === null || uncertainty === null ? null : Math.max(0, pred - uncertainty),
      PREDICTION_HIGH: pred === null || uncertainty === null ? null : Math.min(100, pred + uncertainty),
      LOCAL_FEATURE_DISTANCE: distanceToTraining(candidate, data.dataset),
      DISTANCE_FROM_EXISTING_LABELS: nearestOfficialScore,
      ROUTE_NOVELTY: knownRoutes.has(candidate.route) ? "NO" : "YES",
      WHY_INFORMATIONAL: readTsv(path.join(args.mainDir, "ONLINE-CALIBRATION-CANDIDATES.tsv")).find(r => r.ROUTE === candidate.route && r.REVISION === candidate.revision)?.WHY_INFORMATIONAL || "Calibration candidate",
      BUILD_STATUS: build?.BUILD_STATUS || "NOT_RECORDED",
      CORRECTNESS_STATUS: correctnessComplete ? "PASS_7_OF_7" : "INCOMPLETE",
      EXACT_SOURCE_IDENTITY: bool(sourceExact)
    };
  });
  const rankable = predictions.filter(r => r.LOCAL_RUNNABLE === "YES" && r.LOCAL_VECTOR_STATUS === "CORE_VECTOR_COMPLETE" &&
    Number.isFinite(r.RAW_PREDICTED_SCORE) && Number.isFinite(r.LOCAL_FEATURE_DISTANCE) && Number.isFinite(r.PREDICTION_SPREAD));
  const scoreMetrics = ["LOCAL_FEATURE_DISTANCE", "PREDICTION_SPREAD"];
  const rankMaps = scoreMetrics.map(field => {
    const sorted = [...rankable].sort((a, b) => b[field] - a[field]);
    return new Map(sorted.map((r, i) => [r.VERSION, i + 1]));
  });
  const routeNovel = new Map([...rankable].sort((a, b) => (a.ROUTE_NOVELTY === "YES" ? -1 : 1) - (b.ROUTE_NOVELTY === "YES" ? -1 : 1))
    .map((r, i) => [r.VERSION, i + 1]));
  for (const r of predictions) {
    const canRank = rankable.includes(r) && r.EXACT_SOURCE_IDENTITY === "YES";
    const ranks = [...rankMaps.map(m => m.get(r.VERSION)), routeNovel.get(r.VERSION)];
    r.INFORMATION_GAIN_SCORE = canRank ? mean(ranks) : null;
    r.INFORMATION_GAIN_STATUS = canRank ? "RANKABLE_INFORMATIONAL_ONLY" : "NOT_RANKABLE_MEASUREMENT_INCOMPLETE";
  }
  const sorted = predictions.filter(r => r.INFORMATION_GAIN_STATUS === "RANKABLE_INFORMATIONAL_ONLY")
    .sort((a, b) => a.INFORMATION_GAIN_SCORE - b.INFORMATION_GAIN_SCORE || a.VERSION.localeCompare(b.VERSION));
  sorted.forEach((r, i) => { r.INFORMATION_PRIORITY = i + 1; });
  const rankMap = new Map(sorted.map(r => [r.VERSION, r.INFORMATION_PRIORITY]));
  for (const r of predictions) r.TOP3_LABEL_PRIORITY = (rankMap.get(r.VERSION) || 999) <= 3 ? "YES" : "NO";
  const headers = [
    "INFORMATION_PRIORITY", "TOP3_LABEL_PRIORITY", "VERSION", "ROUTE", "REVISION", "SOURCE_SHA",
    "RAW_PREDICTED_SCORE", "MODEL_A_PRED", "MODEL_B_PRED", "MODEL_C_PRED", "PREDICTION_SPREAD",
    "UNCERTAINTY_OOF_Q90_ABS_ERROR", "PREDICTION_LOW", "PREDICTION_HIGH",
    "LOCAL_FEATURE_DISTANCE", "DISTANCE_FROM_EXISTING_LABELS", "ROUTE_NOVELTY",
    "CORE_VALID_CASES", "DIAGNOSTIC_VALID_CASES", "SOURCE_RECOVERABLE", "LOCAL_RUNNABLE",
    "LOCAL_VECTOR_STATUS", "BUILD_STATUS", "CORRECTNESS_STATUS", "EXACT_SOURCE_IDENTITY",
    "INFORMATION_GAIN_SCORE", "INFORMATION_GAIN_STATUS", "WHY_INFORMATIONAL"
  ];
  const output = predictions.map(r => Object.fromEntries(headers.map(h => [h, typeof r[h] === "number" ? fmt(r[h]) : r[h]])));
  writeTsv(path.join(args.mainDir, "ONLINE-CALIBRATION-CANDIDATES-V2.tsv"), headers, output);
  return predictions;
}
function selectCandidate(version) {
  const candidate = CANDIDATES.find(c => key(c) === version);
  if (!candidate) throw new Error("Unknown candidate version: " + version);
  return candidate;
}
function main() {
  const args = parseArgs(process.argv.slice(2));
  const data = loadData(args.mainDir);
  if (args.command === "validate") {
    const result = writeValidation(args, data);
    console.log(JSON.stringify({
      level: args.judgeLevel,
      loo: result.metrics.LOO.ENSEMBLE,
      route_out: result.metrics.ROUTE_OUT.ENSEMBLE,
      core_cases: CORE_CASES,
      diagnostic_cases: CASES.filter(c => !CORE_CASES.includes(c))
    }, null, 2));
    return;
  }
  if (args.command === "manifest") {
    console.log(JSON.stringify(candidateManifest(args.mainDir), null, 2));
    return;
  }
  if (args.command === "build") {
    const candidate = selectCandidate(args.version);
    console.log(JSON.stringify(buildCandidate(args, candidate).row, null, 2));
    return;
  }
  if (args.command === "measure") {
    const candidate = selectCandidate(args.version);
    if (!Number.isInteger(args.device)) throw new Error("measure requires --device N");
    const result = runCandidateSuite(args, candidate, args.device);
    console.log(JSON.stringify({ version: key(candidate), results: result }, null, 2));
    return;
  }
  if (args.command === "correctness") {
    const candidate = selectCandidate(args.version);
    if (!Number.isInteger(args.device)) throw new Error("correctness requires --device N");
    const lease = acquireLease(args, candidate, args.device);
    let note = "unified-suite correctness complete";
    try {
      console.log(JSON.stringify({ version: key(candidate), results: correctnessSuite(args, candidate, args.device) }, null, 2));
    } catch (error) {
      note = "correctness stopped: " + error.message;
      throw error;
    } finally {
      releaseLease(lease, candidate, args.device, note);
    }
    return;
  }
  if (args.command === "vectorize") {
    console.log(JSON.stringify({ rows: vectorize(args), file: path.join(args.mainDir, "CALIBRATION-CANDIDATES-VECTORS.tsv") }, null, 2));
    return;
  }
  if (args.command === "rank") {
    const cv = writeValidation(args, data);
    const vectors = vectorize(args);
    const ranked = rankCandidates(args, data, cv);
    console.log(JSON.stringify({ measured_cells: vectors.filter(r => r.QUALITY === "MEASUREMENT_STABLE").length, candidates: ranked }, null, 2));
    return;
  }
  if (args.command === "validate-and-rank") {
    const cv = writeValidation(args, data);
    const ranked = rankCandidates(args, data, cv);
    console.log(JSON.stringify({ level: args.judgeLevel, loo: cv.metrics.LOO.ENSEMBLE, route_out: cv.metrics.ROUTE_OUT.ENSEMBLE, candidates: ranked }, null, 2));
    return;
  }
  throw new Error("Unknown command: " + args.command);
}

main();
