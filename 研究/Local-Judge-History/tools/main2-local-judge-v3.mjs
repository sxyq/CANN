#!/usr/bin/env node
/* Nested, route-grouped Local-to-Official calibration. Surrogate only. */
import fs from "node:fs";
import path from "node:path";

const CHAMPION = 45.16;
const CASE_COUNTS = [1, 2, 3, 4, 5];
const MODEL_TYPES = ["A_MEAN_RATIO", "B_GEOMEAN_RATIO", "C_STABILITY_WEIGHTED", "D_RIDGE", "E_ISOTONIC"];
const MODEL_CONFIGS = [
  ...CASE_COUNTS.flatMap(k => MODEL_TYPES.map(t => t + "__TOP" + k)),
  ...MODEL_TYPES.map(t => t + "__STABILITY_FILTERED")
];
const KNOWN_FALSE_POSITIVES = [
  ["HOTLOOP-ADDR-HOIST-CHAMPION-X", "V001", "-24.193320% BF16 9x32768 single-shape; POOR", 42.72, "ADDR H3"],
  ["R31A", "V017", "-4.33% FP32 rows=128 D=24576; unpaired proxy", 44.45, "Historical explicit false positive"],
  ["SCHED-CHAMPION-X", "V002", "Local accepted ownership result; common-suite vector is available in V2 dataset", 41.94, "Historical explicit false positive"],
  ["R31B", "V017", "-14.3% BF16-wide D32768; other wide FP16 gains", 44.68, "Historical explicit false positive"],
  ["R31A", "V028", "-8.3% cumulative chain; V028 -1.09%", 44.07, "Historical explicit false positive"],
  ["STORE-EPILOGUE-X", "V003", "-5.64% C14; other wide-shape gains", 44.38, "Historical explicit false positive"],
  ["EPILOGUE-ARITH-CHAMPION-X", "V002", "-3.13% to -6.64% D32768; -3.62% 2x16384", 44.96, "Historical explicit false positive"]
];
const V2_FALSE_POSITIVE = ["VECTOR-MATH-X", "V001", "V2 predicted 49.995841 from C13-only full-label selection", 44.22, "V2 model false positive"];
const CALIBRATION_CANDIDATES = [
  ["HOTLOOP-ADDR-HOIST-CHAMPION-X", "V002", "40b1548eb0862691e92b10f242e3f98ccf4d232596a2704ddbc25eab7b3aa3f3", "ADDR sibling; BF16 wide reduced-tail and FP16/BF16 mid-wide; correctness passed, local quality POOR."],
  ["HOTLOOP-BRANCH-HOIST-CHAMPION-X", "V001", "4dc1973ef198cd67693610b6ffedad6883b59f52207b9804052975a13f4e032b", "Branch-route negative/neutral contrast; correctness passed on tested domain; local rejected."],
  ["REDUCE-FINALIZE-HANDOFF-CHAMPION-X", "V001", "83d569d2968b3bcd932b87130a359ba763a0679cf9f94e96e152d16f78338df3", "Reduction-finalization mechanism, recorded LOCAL_NEUTRAL/POOR; adds an independent structural regime."],
  ["SCHED-CHAMPION-X", "V001", "ed232872fa1837678a0a05fc16de9f06a725d61ff9d85663193fdc49e8f33678", "Row-ownership sibling of an Official-tested route; correctness passed, local closure pending."],
  ["STORE-EPILOGUE-X", "V001", "06564134e4930eb3e2ca30297b9bda548135d1aa8a1f43238b75dafc8a8c09f9", "Store/epilogue control preceding the Official-tested V002/V003; correctness passed, local closure pending."]
];

const exists = p => { try { fs.accessSync(p); return true; } catch { return false; } };
const read = p => fs.readFileSync(p, "utf8");
const num = v => v == null || v === "" || v === "NA" ? null : (Number.isFinite(Number(v)) ? Number(v) : null);
const mean = xs => { const a = xs.filter(Number.isFinite); return a.length ? a.reduce((s, x) => s + x, 0) / a.length : null; };
const median = xs => { const a = xs.filter(Number.isFinite).sort((x, y) => x - y); if (!a.length) return null; const i = Math.floor(a.length / 2); return a.length % 2 ? a[i] : (a[i - 1] + a[i]) / 2; };
const cv = xs => { const a = xs.filter(x => Number.isFinite(x) && x > 0); if (a.length < 2) return null; const m = mean(a); return m ? Math.sqrt(a.reduce((s, x) => s + (x - m) ** 2, 0) / (a.length - 1)) / m : null; };
const geo = xs => { const a = xs.filter(x => Number.isFinite(x) && x > 0); return a.length ? Math.exp(mean(a.map(Math.log))) : null; };
const clamp = x => Number.isFinite(x) ? Math.max(0, Math.min(100, x)) : null;
const key = r => r.route + "/" + r.revision;
const fmt = x => Number.isFinite(x) ? Number(x.toFixed(6)) : "NA";
const bool = x => x ? "YES" : "NO";

function table(p) {
  if (!exists(p)) return [];
  const lines = read(p).replace(/^\uFEFF/, "").split(/\r?\n/).filter(Boolean);
  if (!lines.length) return [];
  const headers = lines[0].split("\t");
  return lines.slice(1).map(line => {
    const cells = line.split("\t");
    return Object.fromEntries(headers.map((h, i) => [h, cells[i] ?? ""]));
  });
}
function writeTsv(p, headers, rows) {
  fs.mkdirSync(path.dirname(p), { recursive: true });
  const cell = v => v == null || v === "" || (typeof v === "number" && !Number.isFinite(v)) ? "NA" : String(v).replace(/[\t\r\n]+/g, " ");
  fs.writeFileSync(p, [headers.join("\t"), ...rows.map(r => headers.map(h => cell(r[h])).join("\t"))].join("\n") + "\n");
}
function writeText(p, text) { fs.mkdirSync(path.dirname(p), { recursive: true }); fs.writeFileSync(p, text.endsWith("\n") ? text : text + "\n"); }
function parseArgs(argv) {
  const a = { command: argv[0] || "validate-all", root: process.cwd(), out: null, input: null };
  for (let i = 1; i < argv.length; i++) {
    if (argv[i] === "--root") a.root = path.resolve(argv[++i]);
    else if (argv[i] === "--out") a.out = path.resolve(argv[++i]);
    else if (argv[i] === "--input") a.input = path.resolve(argv[++i]);
  }
  a.out ||= path.join(a.root, "研究/主代理/MAIN-2");
  return a;
}
function load(root) {
  const dir = path.join(root, "研究/主代理/MAIN-2");
  const suite = table(path.join(dir, "MAIN2-UNIFIED-LOCAL-SUITE-V1.tsv"));
  const officialRows = table(path.join(dir, "ALL-OFFICIAL-RESULTS.tsv"));
  const official = new Map(officialRows.map(r => [r.ROUTE + "/" + r.REVISION, r]));
  const records = table(path.join(dir, "LOCAL-JUDGE-CALIBRATION-DATASET-V2.tsv"))
    .filter(r => !(r.ROUTE === "R31B" && r.REVISION === "V011"))
    .map(r => ({
      route: r.ROUTE, revision: r.REVISION, sha: r.SOURCE_SHA, score: num(r.OFFICIAL_SCORE),
      official: official.get(r.ROUTE + "/" + r.REVISION) || null,
      ratios: Object.fromEntries(suite.map(c => [c.CASE_ID, num(r["LOCAL_" + c.CASE_ID + "_RATIO"])])),
      row: r
    }))
    .filter(r => r.score !== null);
  const measurements = new Map();
  const files = [
    "calibration-v2/runs/20261004-v2-candidates-blocks/measurements.tsv",
    "calibration-v2/runs/20261004-v2-v011/measurements.tsv"
  ];
  for (const f of files) for (const r of table(path.join(dir, f))) {
    const k = [r.route, r.revision, r.case_id, r.side].join("/");
    if (!measurements.has(k)) measurements.set(k, []);
    measurements.get(k).push(r);
  }
  for (const xs of measurements.values()) xs.sort((a, b) => Number(a.run) - Number(b.run));
  const inventory = table(path.join(dir, "MAIN2-ALL-LOCAL-REVISIONS.tsv"));
  return { root, dir, suite, records, official, measurements, inventory };
}
function validMeasurements(xs) {
  return xs.filter(r => num(r.command_rc) === 0 && num(r.bad) === 0 && num(r.device_median_us) > 0);
}
function correlation(x, y, rank = false) {
  const pairs = x.map((v, i) => [v, y[i]]).filter(([a, b]) => Number.isFinite(a) && Number.isFinite(b));
  if (pairs.length < 3) return null;
  const xx = rank ? averageRanks(pairs.map(p => p[0])) : pairs.map(p => p[0]);
  const yy = rank ? averageRanks(pairs.map(p => p[1])) : pairs.map(p => p[1]);
  const mx = mean(xx), my = mean(yy);
  let xy = 0, vx = 0, vy = 0;
  for (let i = 0; i < xx.length; i++) { xy += (xx[i] - mx) * (yy[i] - my); vx += (xx[i] - mx) ** 2; vy += (yy[i] - my) ** 2; }
  return vx && vy ? xy / Math.sqrt(vx * vy) : null;
}
function averageRanks(xs) {
  const s = xs.map((v, i) => ({ v, i })).sort((a, b) => a.v - b.v), out = Array(xs.length).fill(0);
  for (let i = 0; i < s.length;) {
    let j = i + 1; while (j < s.length && s[j].v === s[i].v) j++;
    for (let k = i; k < j; k++) out[s[k].i] = (i + j + 1) / 2;
    i = j;
  }
  return out;
}
function spearman(x, y) { return correlation(x, y, true); }
function kendall(x, y) {
  const p = x.map((v, i) => [v, y[i]]).filter(([a, b]) => Number.isFinite(a) && Number.isFinite(b));
  if (p.length < 3) return null;
  let concordant = 0, discordant = 0, tx = 0, ty = 0;
  for (let i = 0; i < p.length; i++) for (let j = i + 1; j < p.length; j++) {
    const a = Math.sign(p[i][0] - p[j][0]), b = Math.sign(p[i][1] - p[j][1]);
    if (!a && !b) continue;
    if (!a) tx++; else if (!b) ty++; else if (a === b) concordant++; else discordant++;
  }
  const d = Math.sqrt((concordant + discordant + tx) * (concordant + discordant + ty));
  return d ? (concordant - discordant) / d : null;
}
function ols(xs, ys) {
  const p = xs.map((x, i) => [x, ys[i]]).filter(([x, y]) => Number.isFinite(x) && Number.isFinite(y));
  if (p.length < 3) return null;
  const mx = mean(p.map(q => q[0])), my = mean(p.map(q => q[1]));
  let covariance = 0, variance = 0;
  for (const [x, y] of p) { covariance += (x - mx) * (y - my); variance += (x - mx) ** 2; }
  if (!variance) return null;
  const slope = covariance / variance;
  return { slope, intercept: my - slope * mx };
}
function solve(matrix, vector) {
  const a = matrix.map((r, i) => [...r, vector[i]]), n = vector.length;
  for (let c = 0; c < n; c++) {
    let pivot = c;
    for (let r = c + 1; r < n; r++) if (Math.abs(a[r][c]) > Math.abs(a[pivot][c])) pivot = r;
    if (Math.abs(a[pivot][c]) < 1e-10) return null;
    [a[pivot], a[c]] = [a[c], a[pivot]];
    const d = a[c][c]; for (let j = c; j <= n; j++) a[c][j] /= d;
    for (let r = 0; r < n; r++) if (r !== c) { const f = a[r][c]; for (let j = c; j <= n; j++) a[r][j] -= f * a[c][j]; }
  }
  return a.map(r => r[n]);
}

function caseStability(records, suite, measurements) {
  const rows = suite.map(c => {
    const cvs = [], madRatios = [], signMajorities = [];
    let missing = 0, observed = 0;
    for (const rev of records) {
      const base = [rev.route, rev.revision, c.CASE_ID];
      const p = validMeasurements(measurements.get([...base, "parent"].join("/")) || []);
      const q = validMeasurements(measurements.get([...base, "candidate"].join("/")) || []);
      if (p.length < 3 || q.length < 3) { missing++; continue; }
      observed++;
      cvs.push(cv(p.map(x => num(x.device_median_us))), cv(q.map(x => num(x.device_median_us))));
      for (const x of [...p, ...q]) {
        const med = num(x.device_median_us), mad = num(x.device_mad_us);
        if (med > 0 && mad !== null) madRatios.push(mad / med);
      }
      const pRun = new Map(p.map(x => [String(x.run), num(x.device_median_us)]));
      const signs = q.map(x => pRun.has(String(x.run)) ? Math.sign(num(x.device_median_us) - pRun.get(String(x.run))) : 0).filter(Boolean);
      if (signs.length) signMajorities.push(Math.max(signs.filter(x => x < 0).length, signs.filter(x => x > 0).length) / signs.length);
    }
    return {
      CASE_ID: c.CASE_ID, DTYPE: c.DTYPE, SHAPE: c.ROWS + "x" + c.WIDTH, PATH: c.EXPECTED_PATH,
      RUN_VARIANCE: median(cvs), MAD_RATIO: median(madRatios), SIGN_STABILITY: median(signMajorities),
      LOAD_SENSITIVITY: "NA_PER_RUN_LOAD_NOT_CAPTURED",
      MISSING_RATE: records.length ? missing / records.length : null, MISSING_VERSIONS: missing, OBSERVED_VERSIONS: observed
    };
  });
  const cvMedian = median(rows.map(r => r.RUN_VARIANCE));
  const madMedian = median(rows.map(r => r.MAD_RATIO));
  for (const r of rows) {
    const sign = r.SIGN_STABILITY || 0;
    r.STABILITY_SCORE = sign / (1 + (r.RUN_VARIANCE ?? 1) + (r.MAD_RATIO ?? 1) + (r.MISSING_RATE ?? 1));
    r.KEEP_FOR_MODEL = r.RUN_VARIANCE !== null && r.MAD_RATIO !== null && r.MISSING_RATE <= 1 / 3 &&
      sign >= 2 / 3 && r.RUN_VARIANCE <= cvMedian && r.MAD_RATIO <= madMedian ? "YES" : "NO";
    r.STABILITY_RULE = "training-fold medians for CV/MAD; >=2/3 paired sign majority; missing <=1/3; no per-run load data";
  }
  rows.sort((a, b) => (b.STABILITY_SCORE || 0) - (a.STABILITY_SCORE || 0));
  rows.forEach((r, i) => { r.STABILITY_RANK = i + 1; });
  return rows;
}
function selectCases(train, suite, stability) {
  const diagnostics = suite.map(c => {
    const rows = train.filter(r => Number.isFinite(r.ratios[c.CASE_ID]) && r.ratios[c.CASE_ID] > 0);
    const x = rows.map(r => Math.log(r.ratios[c.CASE_ID])), y = rows.map(r => r.score);
    const sp = rows.length >= 5 ? spearman(x, y) : null;
    const kt = rows.length >= 5 ? kendall(x, y) : null;
    const st = stability.find(s => s.CASE_ID === c.CASE_ID);
    const signal = sp === null ? 0 : Math.abs(sp);
    const sw = st?.STABILITY_SCORE || 0;
    return {
      CASE_ID: c.CASE_ID, COVERAGE: rows.length, SPEARMAN_LOG_RATIO_OFFICIAL: sp,
      KENDALL_LOG_RATIO_OFFICIAL: kt, PEARSON_LOG_RATIO_OFFICIAL: rows.length >= 5 ? correlation(x, y) : null,
      ABS_RANK_SIGNAL: signal, ASSOCIATION_DIRECTION: sp === null ? "NOT_ESTIMABLE" : (sp < 0 ? "LOCAL_RATIO_DOWN_SCORE_UP" : "LOCAL_RATIO_UP_SCORE_UP"),
      STABILITY_SCORE: sw, SELECTION_SCORE: signal * sw, STABILITY_ELIGIBLE: st?.KEEP_FOR_MODEL === "YES" ? "YES" : "NO",
      CHAMPION_SEPARATION: "NOT_ESTIMABLE_NO_ABOVE_CHAMPION_LABELS"
    };
  });
  const ranked = diagnostics.filter(r => r.STABILITY_ELIGIBLE === "YES")
    .sort((a, b) => b.SELECTION_SCORE - a.SELECTION_SCORE || b.STABILITY_SCORE - a.STABILITY_SCORE || a.CASE_ID.localeCompare(b.CASE_ID))
    .map(r => r.CASE_ID);
  const sets = new Map(CASE_COUNTS.map(k => ["TOP" + k, ranked.slice(0, k)]));
  sets.set("STABILITY_FILTERED", ranked.slice(0, 8));
  return { diagnostics, ranked, sets };
}
function configParts(config) {
  const i = config.indexOf("__");
  return [config.slice(0, i), config.slice(i + 2)];
}
function selectedFeature(rev, cases, type, stability) {
  const vals = cases.map(c => rev.ratios[c]);
  if (!cases.length || vals.some(x => !Number.isFinite(x) || x <= 0)) return null;
  if (type === "A_MEAN_RATIO") return mean(vals);
  if (type === "B_GEOMEAN_RATIO") return geo(vals);
  const w = new Map(stability.map(r => [r.CASE_ID, r.STABILITY_SCORE || 0]));
  const weights = cases.map(c => w.get(c) || 0), total = weights.reduce((s, x) => s + x, 0);
  return total ? Math.exp(cases.reduce((s, c, i) => s + Math.log(rev.ratios[c]) * weights[i], 0) / total) : null;
}
function fitPredict(train, test, config, prepared) {
  const [type, selector] = configParts(config), cases = prepared.selection.sets.get(selector) || [];
  if (!cases.length || cases.some(c => !Number.isFinite(test.ratios[c]) || test.ratios[c] <= 0)) return { config, cases, pred: null };
  const complete = train.filter(r => cases.every(c => Number.isFinite(r.ratios[c]) && r.ratios[c] > 0));
  if (type === "A_MEAN_RATIO" || type === "B_GEOMEAN_RATIO" || type === "C_STABILITY_WEIGHTED") {
    const x = complete.map(r => selectedFeature(r, cases, type, prepared.stability));
    const fit = ols(x, complete.map(r => r.score));
    if (!fit) return { config, cases, pred: null };
    return { config, cases, pred: clamp(fit.intercept + fit.slope * selectedFeature(test, cases, type, prepared.stability)) };
  }
  if (type === "D_RIDGE") {
    if (complete.length < Math.max(5, cases.length + 2)) return { config, cases, pred: null };
    const columns = cases.map(c => complete.map(r => Math.log(r.ratios[c])));
    const centers = columns.map(mean), scales = columns.map(xs => Math.sqrt(mean(xs.map(x => (x - mean(xs)) ** 2)) || 0));
    const yMean = mean(complete.map(r => r.score)), matrix = Array.from({ length: cases.length }, () => Array(cases.length).fill(0)), rhs = Array(cases.length).fill(0);
    for (let row = 0; row < complete.length; row++) {
      const z = cases.map((_, j) => scales[j] > 1e-9 ? (columns[j][row] - centers[j]) / scales[j] : 0);
      const target = complete[row].score - yMean;
      for (let i = 0; i < z.length; i++) { rhs[i] += z[i] * target; for (let j = 0; j < z.length; j++) matrix[i][j] += z[i] * z[j]; }
    }
    for (let i = 0; i < cases.length; i++) matrix[i][i] += 1;
    const beta = solve(matrix, rhs);
    if (!beta) return { config, cases, pred: null };
    let pred = yMean;
    for (let i = 0; i < cases.length; i++) if (scales[i] > 1e-9) pred += beta[i] * (Math.log(test.ratios[cases[i]]) - centers[i]) / scales[i];
    return { config, cases, pred: clamp(pred) };
  }
  if (type === "E_ISOTONIC") {
    if (complete.length < 5) return { config, cases, pred: null };
    const points = complete.map(r => ({ x: geo(cases.map(c => r.ratios[c])), y: r.score })).filter(p => Number.isFinite(p.x));
    const direction = spearman(points.map(p => p.x), points.map(p => p.y));
    if (points.length < 5 || !direction) return { config, cases, pred: null };
    const sign = Math.sign(direction), blocks = [];
    points.sort((a, b) => sign * a.x - sign * b.x);
    for (const p of points) {
      const z = { lo: sign * p.x, hi: sign * p.x, sum: p.y, n: 1, avg: p.y };
      blocks.push(z);
      while (blocks.length > 1 && blocks.at(-2).avg > blocks.at(-1).avg) {
        const b = blocks.pop(), a = blocks.pop(), m = { lo: a.lo, hi: b.hi, sum: a.sum + b.sum, n: a.n + b.n };
        m.avg = m.sum / m.n; blocks.push(m);
      }
    }
    const q = sign * geo(cases.map(c => test.ratios[c]));
    const closest = blocks.reduce((a, b) => Math.abs((b.lo + b.hi) / 2 - q) < Math.abs((a.lo + a.hi) / 2 - q) ? b : a);
    return { config, cases, pred: clamp(closest.avg) };
  }
  return { config, cases, pred: null };
}
function prepare(train, data) {
  const stability = caseStability(train, data.suite, data.measurements);
  const selection = selectCases(train, data.suite, stability);
  return { stability, selection };
}
function crossFitCandidates(train, data) {
  const out = [];
  const routes = [...new Set(train.map(r => r.route))];
  for (const route of routes) {
    const fitRows = train.filter(r => r.route !== route), held = train.filter(r => r.route === route);
    if (new Set(fitRows.map(r => r.route)).size < 4) continue;
    const prepared = prepare(fitRows, data);
    for (const test of held) for (const config of MODEL_CONFIGS) {
      const p = fitPredict(fitRows, test, config, prepared);
      if (p.pred === null) continue;
      out.push({
        config, key: key(test), route: test.route, revision: test.revision, actual: test.score, pred: p.pred,
        cases: p.cases, trainN: fitRows.length, trainRoutes: new Set(fitRows.map(r => r.route)).size
      });
    }
  }
  return out;
}
function predictionMetrics(rows) {
  const a = rows.filter(r => Number.isFinite(r.pred) && Number.isFinite(r.actual));
  if (!a.length) return { n: 0, mae: null, rmse: null, spearman: null, kendall: null, fp: null, fn: null, accuracy: null, actualAbove: 0 };
  const p = a.map(r => r.pred), y = a.map(r => r.actual), errors = p.map((v, i) => v - y[i]);
  const decisionScore = r => Number.isFinite(r.conservative) ? r.conservative : r.pred;
  const rawFp = a.filter(r => r.pred > CHAMPION && r.actual < CHAMPION).length;
  const fp = a.filter(r => decisionScore(r) > CHAMPION && r.actual < CHAMPION).length;
  const above = a.filter(r => r.actual > CHAMPION);
  const rawFn = above.filter(r => r.pred <= CHAMPION).length;
  const fn = above.filter(r => decisionScore(r) <= CHAMPION).length;
  const correct = a.filter(r => (decisionScore(r) > CHAMPION) === (r.actual > CHAMPION)).length;
  return {
    n: a.length, mae: mean(errors.map(Math.abs)), rmse: Math.sqrt(mean(errors.map(x => x * x))),
    spearman: spearman(p, y), kendall: kendall(p, y), fp, fn, rawFp, rawFn,
    rawAccuracy: a.filter(r => (r.pred > CHAMPION) === (r.actual > CHAMPION)).length / a.length,
    accuracy: correct / a.length, actualAbove: above.length
  };
}
function chooseConfig(predictions) {
  const groups = new Map();
  for (const r of predictions) {
    if (!Number.isFinite(r.pred) || !Number.isFinite(r.actual)) continue;
    if (!groups.has(r.config)) groups.set(r.config, []);
    groups.get(r.config).push(r);
  }
  const candidates = [...groups].map(([config, rows]) => {
    const m = predictionMetrics(rows);
    return { config, rows, n: m.n, fp: m.fp, mae: m.mae, rank: m.spearman, cases: median(rows.map(r => r.cases.length)) };
  }).filter(x => x.n >= 5 && x.rows.every(r => r.cases.length >= 3));
  candidates.sort((a, b) => a.fp - b.fp || (a.mae ?? Infinity) - (b.mae ?? Infinity) ||
    (b.rank ?? -1) - (a.rank ?? -1) || (a.cases ?? Infinity) - (b.cases ?? Infinity) || a.config.localeCompare(b.config));
  return candidates[0] || null;
}
function nestedCrossFit(train, data) {
  const output = [];
  for (const route of [...new Set(train.map(r => r.route))]) {
    const fitRows = train.filter(r => r.route !== route), held = train.filter(r => r.route === route);
    if (new Set(fitRows.map(r => r.route)).size < 4) continue;
    const chosen = chooseConfig(crossFitCandidates(fitRows, data));
    if (!chosen) continue;
    const prepared = prepare(fitRows, data);
    for (const test of held) {
      const p = fitPredict(fitRows, test, chosen.config, prepared);
      if (p.pred === null) continue;
      output.push({
        key: key(test), route: test.route, revision: test.revision, actual: test.score, pred: p.pred,
        config: chosen.config, cases: p.cases, trainN: fitRows.length,
        trainRoutes: new Set(fitRows.map(r => r.route)).size, configSelectionN: chosen.n
      });
    }
  }
  return output;
}
function maxOverprediction(rows) {
  const valid = rows.filter(r => Number.isFinite(r.pred) && Number.isFinite(r.actual));
  return valid.length ? Math.max(0, ...valid.map(r => r.pred - r.actual)) : null;
}
function fitSelectionAndMargin(train, data) {
  const chosen = chooseConfig(crossFitCandidates(train, data));
  const residualPredictions = nestedCrossFit(train, data);
  return {
    config: chosen?.config || null,
    selectionN: chosen?.n || 0,
    selectionFP: chosen?.fp ?? null,
    selectionMAE: chosen?.mae ?? null,
    margin: maxOverprediction(residualPredictions),
    residualPredictions
  };
}
function outerValidation(records, data, mode) {
  const groups = mode === "LOO" ? records.map(r => [r]) :
    [...new Set(records.map(r => r.route))].map(route => records.filter(r => r.route === route));
  const output = [];
  for (const held of groups) {
    const heldKeys = new Set(held.map(key));
    const train = records.filter(r => !heldKeys.has(key(r)));
    const selected = fitSelectionAndMargin(train, data);
    if (!selected.config) {
      for (const test of held) output.push({ route: test.route, revision: test.revision, sha: test.sha, actual: test.score, fold: mode + ":" + (mode === "LOO" ? key(test) : test.route), config: "NOT_COMPUTABLE", cases: [], pred: null, margin: selected.margin, conservative: null, error: null, trainN: train.length, trainRoutes: new Set(train.map(r => r.route)).size, innerN: selected.selectionN, correctness: num(test.official?.PASS_COUNT) === 15 ? "PASS_15_OF_15" : "PASS_COUNT_" + (test.official?.PASS_COUNT || "NA"), localStable: "NO", decision: "NOT_COMPUTABLE", onlineEligible: "NO", leakage: "NO_OUTER_LABEL_USED" });
      continue;
    }
    const prepared = prepare(train, data);
    for (const test of held) {
      const p = fitPredict(train, test, selected.config, prepared);
      const conservative = p.pred !== null && selected.margin !== null ? clamp(p.pred - selected.margin) : null;
      const stableCases = p.cases.length >= 3 && p.cases.every(c => prepared.stability.some(s => s.CASE_ID === c && s.KEEP_FOR_MODEL === "YES"));
      const hasAbove = train.some(r => r.score > CHAMPION);
      const correctness = num(test.official?.PASS_COUNT) === 15 ? "PASS_15_OF_15" : "PASS_COUNT_" + (test.official?.PASS_COUNT || "NA");
      const eligible = conservative !== null && conservative > CHAMPION && correctness === "PASS_15_OF_15" && stableCases && hasAbove;
      output.push({
        route: test.route, revision: test.revision, sha: test.sha, actual: test.score, fold: mode + ":" + (mode === "LOO" ? key(test) : test.route),
        config: selected.config, cases: p.cases, caseCount: p.cases.length, pred: p.pred, margin: selected.margin,
        conservative, error: p.pred === null ? null : p.pred - test.score, trainN: train.length,
        trainRoutes: new Set(train.map(r => r.route)).size, innerN: selected.selectionN, innerFP: selected.selectionFP,
        innerMAE: selected.selectionMAE, correctness, localStable: bool(stableCases),
        hasAboveChampionTrainingLabel: bool(hasAbove),
        rawDecision: p.pred === null ? "NOT_COMPUTABLE" : p.pred > CHAMPION ? "ABOVE_CHAMPION" : "AT_OR_BELOW_CHAMPION",
        decision: conservative === null ? "NOT_COMPUTABLE" : conservative > CHAMPION ? "PREDICTED_ABOVE_CHAMPION" : "REJECT_BELOW_CHAMPION",
        onlineEligible: bool(eligible), leakage: "NO_OUTER_LABEL_USED"
      });
    }
  }
  return output;
}
function localRows(root, dirName) { return table(path.join(root, "研究/主代理/MAIN-2/calibration-v2/runs", dirName, "measurements.tsv")); }

function writeValidationFile(file, label, rows) {
  const metrics = predictionMetrics(rows, true);
  const headers = [
    "FOLD", "ROUTE", "REVISION", "SOURCE_SHA", "TRAIN_VERSIONS", "TRAIN_ROUTES",
    "SELECTED_MODEL", "SELECTED_CASES", "CASE_COUNT", "PREDICTED_SCORE", "ACTUAL_SCORE", "ERROR",
    "OVERPREDICTION_MARGIN", "CONSERVATIVE_SCORE", "RAW_DECISION", "CONSERVATIVE_DECISION",
    "CORRECTNESS_STATUS", "LOCAL_STABLE", "HAS_ABOVE_CHAMPION_TRAIN_LABEL", "ONLINE_ELIGIBLE",
    "INNER_CONFIG_SELECTION_N", "INNER_CONFIG_FALSE_POSITIVES", "INNER_CONFIG_MAE", "LEAKAGE_AUDIT"
  ];
  writeTsv(file, headers, rows.map(r => ({
    FOLD: r.fold, ROUTE: r.route, REVISION: r.revision, SOURCE_SHA: r.sha, TRAIN_VERSIONS: r.trainN,
    TRAIN_ROUTES: r.trainRoutes, SELECTED_MODEL: r.config, SELECTED_CASES: r.cases.join(","),
    CASE_COUNT: r.caseCount ?? r.cases.length, PREDICTED_SCORE: fmt(r.pred), ACTUAL_SCORE: fmt(r.actual),
    ERROR: fmt(r.error), OVERPREDICTION_MARGIN: fmt(r.margin), CONSERVATIVE_SCORE: fmt(r.conservative),
    RAW_DECISION: r.rawDecision, CONSERVATIVE_DECISION: r.decision, CORRECTNESS_STATUS: r.correctness,
    LOCAL_STABLE: r.localStable, HAS_ABOVE_CHAMPION_TRAIN_LABEL: r.hasAboveChampionTrainingLabel,
    ONLINE_ELIGIBLE: r.onlineEligible, INNER_CONFIG_SELECTION_N: r.innerN, INNER_CONFIG_FALSE_POSITIVES: r.innerFP,
    INNER_CONFIG_MAE: fmt(r.innerMAE), LEAKAGE_AUDIT: r.leakage
  })));
  return { label, metrics };
}
function generateStabilityAndPredictiveness(data, loo) {
  const full = caseStability(data.records, data.suite, data.measurements);
  const stabilityHeaders = ["CASE_ID", "DTYPE", "SHAPE", "PATH", "RUN_VARIANCE", "MAD_RATIO", "SIGN_STABILITY", "LOAD_SENSITIVITY", "MISSING_RATE", "MISSING_VERSIONS", "OBSERVED_VERSIONS", "STABILITY_SCORE", "STABILITY_RANK", "KEEP_FOR_MODEL", "STABILITY_RULE"];
  writeTsv(path.join(data.dir, "LOCAL-CASE-STABILITY.tsv"), stabilityHeaders, full.map(r => ({
    ...r, RUN_VARIANCE: fmt(r.RUN_VARIANCE), MAD_RATIO: fmt(r.MAD_RATIO), SIGN_STABILITY: fmt(r.SIGN_STABILITY),
    MISSING_RATE: fmt(r.MISSING_RATE), STABILITY_SCORE: fmt(r.STABILITY_SCORE)
  })));
  const predRows = [];
  for (const outer of loo) {
    const heldKey = outer.route + "/" + outer.revision;
    const train = data.records.filter(r => key(r) !== heldKey);
    const prep = prepare(train, data);
    for (const d of prep.selection.diagnostics) {
      predRows.push({
        FOLD: "LOO:" + heldKey, HELD_OUT_VERSION: heldKey, CASE_ID: d.CASE_ID,
        TRAIN_COVERAGE: d.COVERAGE, TRAIN_SPEARMAN: fmt(d.SPEARMAN_LOG_RATIO_OFFICIAL),
        TRAIN_KENDALL: fmt(d.KENDALL_LOG_RATIO_OFFICIAL), TRAIN_PEARSON: fmt(d.PEARSON_LOG_RATIO_OFFICIAL),
        ABS_RANK_SIGNAL: fmt(d.ABS_RANK_SIGNAL), STABILITY_SCORE: fmt(d.STABILITY_SCORE),
        SELECTED_IN_OUTER_MODEL: outer.cases.includes(d.CASE_ID) ? "YES" : "NO",
        OUTER_MODEL: outer.config, HELDOUT_LABEL_USED: "NO"
      });
    }
  }
  writeTsv(path.join(data.dir, "LOCAL-CASE-NESTED-PREDICTIVENESS.tsv"),
    ["FOLD", "HELD_OUT_VERSION", "CASE_ID", "TRAIN_COVERAGE", "TRAIN_SPEARMAN", "TRAIN_KENDALL", "TRAIN_PEARSON", "ABS_RANK_SIGNAL", "STABILITY_SCORE", "SELECTED_IN_OUTER_MODEL", "OUTER_MODEL", "HELDOUT_LABEL_USED"], predRows);
  return full;
}
function suiteV2(data, loo, stability) {
  const freq = new Map(data.suite.map(c => [c.CASE_ID, 0]));
  for (const r of loo) for (const c of r.cases) freq.set(c, (freq.get(c) || 0) + 1);
  const joined = data.suite.map(c => {
    const s = stability.find(x => x.CASE_ID === c.CASE_ID);
    const count = freq.get(c.CASE_ID) || 0;
    return {
      CASE_ID: c.CASE_ID, DTYPE: c.DTYPE, ROWS: c.ROWS, WIDTH: c.WIDTH, EXPECTED_PATH: c.EXPECTED_PATH,
      SOURCE_OF_CASE: c.SOURCE_OF_CASE, WHY_INCLUDED: c.WHY_INCLUDED,
      LOOCV_SELECTION_COUNT: count, LOOCV_SELECTION_RATE: loo.length ? count / loo.length : null,
      RUN_VARIANCE: s?.RUN_VARIANCE, MAD_RATIO: s?.MAD_RATIO, SIGN_STABILITY: s?.SIGN_STABILITY,
      MISSING_RATE: s?.MISSING_RATE, STABILITY_SCORE: s?.STABILITY_SCORE, STABILITY_ELIGIBLE: s?.KEEP_FOR_MODEL || "NO"
    };
  }).sort((a, b) => b.LOOCV_SELECTION_COUNT - a.LOOCV_SELECTION_COUNT ||
    (b.STABILITY_SCORE || 0) - (a.STABILITY_SCORE || 0) || a.CASE_ID.localeCompare(b.CASE_ID));
  const eligible = joined.filter(r => r.STABILITY_ELIGIBLE === "YES");
  const chosen = [];
  const remaining = [...eligible];
  while (remaining.length && chosen.length < 8) {
    let bestIndex = 0, bestScore = -Infinity;
    for (let i = 0; i < remaining.length; i++) {
      const r = remaining[i], diversity = chosen.length === 0 ? 2 : (
        (chosen.some(x => x.DTYPE !== r.DTYPE) ? 1 : 0) +
        (chosen.some(x => x.EXPECTED_PATH !== r.EXPECTED_PATH) ? 1 : 0)
      );
      const score = (r.LOOCV_SELECTION_COUNT || 0) * 100 + (r.STABILITY_SCORE || 0) * 10 + diversity;
      if (score > bestScore) { bestScore = score; bestIndex = i; }
    }
    const [next] = remaining.splice(bestIndex, 1);
    if (next.LOOCV_SELECTION_COUNT > 0 || chosen.length < 3) chosen.push(next);
  }
  const core = [...chosen];
  const scale = r => Number(r.WIDTH) <= 1024 ? "SMALL" : (Number(r.WIDTH) < 8192 ? "MID" : "WIDE");
  const addDiagnostic = predicate => {
    if (chosen.length >= 8) return;
    const option = joined.filter(r => !chosen.some(x => x.CASE_ID === r.CASE_ID) && predicate(r))
      .sort((a, b) => (b.STABILITY_SCORE || 0) - (a.STABILITY_SCORE || 0))[0];
    if (option) chosen.push(option);
  };
  if (!chosen.some(r => r.DTYPE === "FP32")) addDiagnostic(r => r.DTYPE === "FP32");
  if (!chosen.some(r => scale(r) === "SMALL")) addDiagnostic(r => scale(r) === "SMALL");
  if (!chosen.some(r => scale(r) === "MID" && r.EXPECTED_PATH !== "WIDE_MULTIROW_FULL")) addDiagnostic(r => scale(r) === "MID");
  const keep = new Set(chosen.map(r => r.CASE_ID));
  const coreIds = new Set(core.map(r => r.CASE_ID));
  for (const r of joined) {
    r.KEEP_FOR_V2 = coreIds.has(r.CASE_ID) ? "PROVISIONAL_CORE_FEATURE" :
      (keep.has(r.CASE_ID) ? "DIAGNOSTIC_COVERAGE_ONLY" : "NO");
    r.V2_STATUS = coreIds.has(r.CASE_ID) ? "NESTED_SELECTION_AND_STABILITY_SUPPORTED; NOT_PROVEN_PREDICTIVE" :
      (keep.has(r.CASE_ID) ? "DIVERSITY_SENTINEL; MEASUREMENT_STABILITY_SCREEN_FAILED; DO_NOT_MODEL" :
      (r.STABILITY_ELIGIBLE === "YES" ? "STABLE_BUT_NOT_SELECTED_OR_REDUNDANT" : "MEASUREMENT_STABILITY_SCREEN_FAILED"));
  }
  writeTsv(path.join(data.dir, "MAIN2-UNIFIED-LOCAL-SUITE-V2.tsv"),
    ["CASE_ID", "ROWS", "WIDTH", "DTYPE", "EXPECTED_PATH", "SOURCE_OF_CASE", "WHY_INCLUDED", "LOOCV_SELECTION_COUNT", "LOOCV_SELECTION_RATE", "RUN_VARIANCE", "MAD_RATIO", "SIGN_STABILITY", "MISSING_RATE", "STABILITY_SCORE", "STABILITY_ELIGIBLE", "KEEP_FOR_V2", "V2_STATUS"],
    joined.map(r => ({
      ...r, LOOCV_SELECTION_RATE: fmt(r.LOOCV_SELECTION_RATE), RUN_VARIANCE: fmt(r.RUN_VARIANCE),
      MAD_RATIO: fmt(r.MAD_RATIO), SIGN_STABILITY: fmt(r.SIGN_STABILITY), MISSING_RATE: fmt(r.MISSING_RATE), STABILITY_SCORE: fmt(r.STABILITY_SCORE)
    })));
  return joined.filter(r => keep.has(r.CASE_ID));
}
function c13Forensic(data) {
  const rows = data.records.filter(r => Number.isFinite(r.ratios.C13));
  const x = rows.map(r => Math.log(r.ratios.C13)), y = rows.map(r => r.score);
  const allSp = spearman(x, y), allKt = kendall(x, y);
  const leaveRoute = [...new Set(rows.map(r => r.route))].map(route => {
    const rest = rows.filter(r => r.route !== route);
    return { route, n: rest.length, spearman: spearman(rest.map(r => Math.log(r.ratios.C13)), rest.map(r => r.score)) };
  });
  const routeMeans = [...new Set(rows.map(r => r.route))].map(route => {
    const group = rows.filter(r => r.route === route);
    return { route, ratio: geo(group.map(r => r.ratios.C13)), score: mean(group.map(r => r.score)), n: group.length };
  });
  const routeSp = spearman(routeMeans.map(r => Math.log(r.ratio)), routeMeans.map(r => r.score));
  const descriptor = data.suite.find(c => c.CASE_ID === "C13");
  const stability = caseStability(data.records, data.suite, data.measurements).find(r => r.CASE_ID === "C13");
  const changedSign = leaveRoute.some(r => r.spearman !== null && allSp !== null && Math.sign(r.spearman) !== Math.sign(allSp));
  const text = [
    "# C13 Forensic",
    "",
    "- Case: " + descriptor.DTYPE + " rows=" + descriptor.ROWS + " D=" + descriptor.WIDTH + "; path=" + descriptor.EXPECTED_PATH + ".",
    "- V2 selected C13 after evaluating all 12 Official candidate labels; therefore that selection is leakage for V2 LOO metrics.",
    "- Full-label exploratory association: Spearman(log local ratio, Official)=" + fmt(allSp) + "; Kendall=" + fmt(allKt) + "; n=" + rows.length + ". This is descriptive only, not held-out validation.",
    "- Run stability from the available 3-block measurements: CV=" + fmt(stability.RUN_VARIANCE) + "; MAD/median=" + fmt(stability.MAD_RATIO) + "; paired direction majority=" + fmt(stability.SIGN_STABILITY) + ". Per-run load sensitivity is unavailable.",
    "- Route-averaged correlation (one point per Route)=" + fmt(routeSp) + " across " + routeMeans.length + " Routes. This limits repeated-Store weighting but is still exploratory.",
    "- Leave-one-Route-out C13 correlations: " + leaveRoute.map(r => r.route + "=" + fmt(r.spearman)).join("; ") + ".",
    "- Direction changes after removing any one Route: " + bool(changedSign) + ".",
    "- Interpretation: C13 is one FP16 128x16384 wide multirow path. Even if its historical association is numerically strongest, a single dtype/path cannot establish generalization; several cases have substantial timing dispersion, and the 12 labels contain no score above the 45.16 Champion, so champion sensitivity is untestable.",
    "- Decision: do not treat C13 as a standalone Local Judge or Online gate feature."
  ].join("\n");
  writeText(path.join(data.dir, "C13-FORENSIC.md"), text);
  return { allSp, allKt, routeSp, leaveRoute, changedSign, stability };
}
function falsePositiveAudit(data, loo) {
  const map = new Map(loo.map(r => [r.route + "/" + r.revision, r]));
  const rows = [];
  for (const [route, revision, signal, actual, note] of [...KNOWN_FALSE_POSITIVES, V2_FALSE_POSITIVE]) {
    const p = map.get(route + "/" + revision);
    rows.push({
      VERSION: route + "/" + revision, SCOPE: note, OLD_LOCAL_SIGNAL: signal, ACTUAL_OFFICIAL: actual,
      V3_PREDICTED: p?.pred ?? null, V3_OVERPREDICTION_MARGIN: p?.margin ?? null,
      V3_CONSERVATIVE_SCORE: p?.conservative ?? null, V3_DECISION: p?.decision || "NOT_COMPUTABLE_MISSING_UNIFIED_VECTOR",
      CORRECTLY_REJECTED: p ? bool(p.conservative !== null && p.conservative <= CHAMPION) : "NA",
      SOURCE: p ? "STRICT_OUTER_VERSION_HOLDOUT" : "NO_COMMON_16_CASE_VECTOR_IN_CALIBRATION_V2"
    });
  }
  writeTsv(path.join(data.dir, "LOCAL-JUDGE-FALSE-POSITIVE-AUDIT.tsv"),
    ["VERSION", "SCOPE", "OLD_LOCAL_SIGNAL", "ACTUAL_OFFICIAL", "V3_PREDICTED", "V3_OVERPREDICTION_MARGIN", "V3_CONSERVATIVE_SCORE", "V3_DECISION", "CORRECTLY_REJECTED", "SOURCE"],
    rows.map(r => ({
      ...r, V3_PREDICTED: fmt(r.V3_PREDICTED), V3_OVERPREDICTION_MARGIN: fmt(r.V3_OVERPREDICTION_MARGIN),
      V3_CONSERVATIVE_SCORE: fmt(r.V3_CONSERVATIVE_SCORE)
    })));
  return rows;
}
function candidateRecommendations(data) {
  const map = new Map(data.inventory.map(r => [r.ROUTE + "/" + r.REVISION, r]));
  const rows = CALIBRATION_CANDIDATES.map(([route, revision, sha, reason], i) => {
    const inv = map.get(route + "/" + revision) || {};
    return {
      VERSION: route + "/" + revision, ROUTE: route, REVISION: revision, SOURCE_SHA: sha,
      WHY_INFORMATIONAL: reason, LOCAL_FEATURE_DISTANCE: "NA_COMMON_SUITE_VECTOR_REQUIRED",
      MODEL_DISAGREEMENT: "NA_COMMON_SUITE_VECTOR_REQUIRED", PREDICTED_RANGE: "NA_COMMON_SUITE_VECTOR_REQUIRED",
      CORRECTNESS_STATUS: inv.CORRECTNESS_STATUS || "SEE_REVISION_INVENTORY",
      BUILD_STATUS: inv.BUILD_STATUS || "SEE_REVISION_INVENTORY",
      LOCAL_VERDICT: inv.LOCAL_VERDICT || "SEE_REVISION_INVENTORY",
      RECOMMENDATION_STATUS: "INFORMATIONAL_ONLY; RUN_UNIFIED_SUITE_FIRST; NOT_ONLINE_READY",
      PRIORITY: i + 1
    };
  });
  writeTsv(path.join(data.dir, "ONLINE-CALIBRATION-CANDIDATES.tsv"),
    ["PRIORITY", "VERSION", "ROUTE", "REVISION", "SOURCE_SHA", "WHY_INFORMATIONAL", "LOCAL_FEATURE_DISTANCE", "MODEL_DISAGREEMENT", "PREDICTED_RANGE", "BUILD_STATUS", "CORRECTNESS_STATUS", "LOCAL_VERDICT", "RECOMMENDATION_STATUS"], rows);
  return rows;
}
function modelComparison(data) {
  const cvRows = crossFitCandidates(data.records, data);
  const chosen = chooseConfig(cvRows);
  const groups = new Map();
  for (const r of cvRows) {
    if (!groups.has(r.config)) groups.set(r.config, []);
    groups.get(r.config).push(r);
  }
  const rows = [...groups].map(([config, values]) => {
    const m = predictionMetrics(values);
    return {
      MODEL_CONFIG: config, OOF_N: m.n, CASE_COUNT_MEDIAN: median(values.map(r => r.cases.length)),
      OOF_MAE: m.mae, OOF_RMSE: m.rmse, OOF_SPEARMAN: m.spearman, OOF_KENDALL: m.kendall,
      RAW_FALSE_POSITIVES: m.rawFp, CHAMPION_DECISION_ACCURACY_RAW: m.rawAccuracy,
      SELECTED_BY_INNER_RULE: chosen?.config === config ? "YES" : "NO",
      VALIDATION_SCOPE: "ROUTE-GROUPED OOF FOR INNER SELECTION ONLY; NOT FINAL OUTER VALIDATION"
    };
  }).sort((a, b) => (a.OOF_MAE ?? Infinity) - (b.OOF_MAE ?? Infinity));
  writeTsv(path.join(data.dir, "LOCAL-JUDGE-MODEL-COMPARISON.tsv"),
    ["MODEL_CONFIG", "OOF_N", "CASE_COUNT_MEDIAN", "OOF_MAE", "OOF_RMSE", "OOF_SPEARMAN", "OOF_KENDALL", "RAW_FALSE_POSITIVES", "CHAMPION_DECISION_ACCURACY_RAW", "SELECTED_BY_INNER_RULE", "VALIDATION_SCOPE"],
    rows.map(r => ({
      ...r, CASE_COUNT_MEDIAN: fmt(r.CASE_COUNT_MEDIAN), OOF_MAE: fmt(r.OOF_MAE), OOF_RMSE: fmt(r.OOF_RMSE),
      OOF_SPEARMAN: fmt(r.OOF_SPEARMAN), OOF_KENDALL: fmt(r.OOF_KENDALL), CHAMPION_DECISION_ACCURACY_RAW: fmt(r.CHAMPION_DECISION_ACCURACY_RAW)
    })));
  return rows;
}
function writeSpec(data, suiteV2Rows) {
  const lines = [
    "# Local Judge V3 Specification",
    "",
    "Mode: CALIBRATED_SURROGATE. Online hidden-case reproduction remains PARTIAL; this tool predicts an Official-score range proxy and must not be described as an exact Online reproduction.",
    "",
    "## Inputs and invariants",
    "",
    "- Training labels are the 12 Official-tested candidate revisions in LOCAL-JUDGE-CALIBRATION-DATASET-V2.tsv. R31B/V011 is the fixed local and Official Champion anchor (45.16), not a candidate training target.",
    "- Local features are the same-suite candidate-to-fresh-V011 latency ratios for the 16 V1 cases. Missing features remain missing; the fitter never fills them from another shape or old parent logs.",
    "- Per-run stability comes only from the completed V011 and candidates-blocks measurement roots. The interrupted candidates run is excluded. Per-run device-load sensitivity is unavailable.",
    "",
    "## Nested procedure",
    "",
    "For each outer held-out version or Route, exclude its complete group. Within the remaining training groups, grouped leave-one-Route-out predictions compare TOP-1..TOP-5 and the stability-filtered ensemble, then select one of five simple models. TOP-1 is measured as a baseline but is ineligible for the selected V3 pipeline: every selected model must use at least 3 cases. The selected pipeline is fitted only on the outer training set.",
    "",
    "The safety margin is the largest positive overprediction among nested, route-held-out predictions generated entirely inside the outer training set: max(0, predicted - actual). Conservative score = predicted score - this margin. This is empirical worst validated overprediction, not a statistical guarantee.",
    "",
    "Models: mean-ratio linear baseline, geometric-mean-ratio linear baseline, stability-weighted geometric baseline, fixed-ridge regression (lambda=1), and monotone isotonic mapping when data support it. Configuration selection orders false-positive count first, then MAE, rank correlation, and smaller feature count.",
    "",
    "## Gate",
    "",
    "ONLINE_ELIGIBLE requires correctness PASS, at least 3 stable selected cases, Conservative Score > 45.16, and at least one above-Champion Official training label. Since the current 12 historical candidate labels contain zero above-Champion outcomes, sensitivity is unvalidated and V3 currently returns NO for submission eligibility.",
    "",
    "## Commands",
    "",
    "- node 工具/main2-local-judge-v3.mjs validate-all",
    "- node 工具/main2-local-judge-v3.mjs validate-loocv",
    "- node 工具/main2-local-judge-v3.mjs validate-route-out",
    "- node 工具/main2-local-judge-v3.mjs audit-false-positive",
    "- node 工具/main2-local-judge-v3.mjs predict --input candidate-ratios.tsv",
    "",
    "LOCAL-JUDGE-MODEL-COMPARISON.tsv records TOP-1..TOP-5 and stability-filtered baseline comparisons for inner configuration selection only; it is not an outer validation result.",
    "",
    "Suite V2 currently has " + suiteV2Rows.length + " provisional cases. This is an evaluation/benchmark proposal only; it does not establish predictive readiness."
  ];
  writeText(path.join(data.dir, "LOCAL-JUDGE-V3-SPEC.md"), lines.join("\n"));
}
function writeValidationReport(data, loo, routeOut, fpRows, suiteV2Rows, c13, predictions) {
  const lm = predictionMetrics(loo, true), rm = predictionMetrics(routeOut, true);
  const looRaw = predictionMetrics(loo), routeRaw = predictionMetrics(routeOut);
  const computableKnown = fpRows.filter(r => r.SCOPE !== "V2 model false positive");
  const addr = fpRows.find(r => r.VERSION === "HOTLOOP-ADDR-HOIST-CHAMPION-X/V001");
  const vector = fpRows.find(r => r.VERSION === "VECTOR-MATH-X/V001");
  const routeKnownCount = new Set(data.records.map(r => r.route)).size;
  const ready = false;
  const needMore = lm.n < data.records.length || routeOut.length < data.records.length ||
    lm.spearman === null || rm.spearman === null || lm.fp > 0 || routeOut.some(r => r.decision === "NOT_COMPUTABLE") ||
    data.records.every(r => r.score <= CHAMPION);
  const lines = [
    "# Local Judge V3 Validation",
    "",
    "- V2 feature-selection leakage: YES. C13 was selected using all 12 Official candidate labels; its V2 validation metrics are OPTIMISTIC / NON-NESTED and are not accepted as formal validation.",
    "- Calibration: " + data.records.length + " Official candidate revisions, " + routeKnownCount + " Routes; Champion anchor R31B/V011 Official 45.16 is excluded from candidate fit.",
    "- Fresh V011 local vector is available for 15/16 suite cases. C15 (FP32 D32768) remains missing/invalid and is never imputed.",
    "- No NPU re-run, Kernel change, or Online submission was performed. Interrupted candidates-run data was excluded; C15 remains missing where raw evidence is absent/invalid.",
    "",
    "## Strict nested version-level LOOCV",
    "",
    "- Computable rows: " + lm.n + "/" + data.records.length + ".",
    "- Predicted-score MAE=" + fmt(lm.mae) + "; RMSE=" + fmt(lm.rmse) + "; Spearman=" + fmt(lm.spearman) + "; Kendall=" + fmt(lm.kendall) + ".",
    "- Raw-score champion false positives=" + (lm.rawFp ?? "NA") + "; conservative-gate false positives=" + (lm.fp ?? "NA") +
      "; conservative false negatives=" + (lm.fn ?? "NA") + "; conservative decision accuracy=" + fmt(lm.accuracy) + ".",
    "- Actual labels above Champion=" + lm.actualAbove + "; false-negative sensitivity is therefore not empirically testable.",
    "- Every raw held-out prediction is at or below Champion. The zero-FP count is a reject-all outcome, not evidence that V3 can identify a true winner.",
    "- Nested overprediction-margin range across outer folds: " + fmt(Math.min(...loo.map(r => r.margin).filter(Number.isFinite))) + " to " + fmt(Math.max(...loo.map(r => r.margin).filter(Number.isFinite))) + " Official points.",
    "",
    "## Strict leave-one-Route-out",
    "",
    "- Computable rows=" + rm.n + "/" + data.records.length + " across " + routeKnownCount + " Routes.",
    "- Predicted-score MAE=" + fmt(rm.mae) + "; RMSE=" + fmt(rm.rmse) + "; Spearman=" + fmt(rm.spearman) + "; Kendall=" + fmt(rm.kendall) + ".",
    "- Raw-score champion false positives=" + (rm.rawFp ?? "NA") + "; conservative-gate false positives=" + (rm.fp ?? "NA") +
      "; conservative false negatives=" + (rm.fn ?? "NA") + "; conservative decision accuracy=" + fmt(rm.accuracy) + ".",
    "- Every raw held-out prediction is at or below Champion; false-negative sensitivity is unavailable because no actual candidate exceeds Champion.",
    "",
    "## False-positive / hard-held-out cases",
    "",
    "- Seven historical explicit false positives were audited. Strict same-suite held-outs are reported where a vector exists; missing vectors are NOT_COMPUTABLE, never inferred from single-shape Local deltas.",
    "- ADDR H3: prediction=" + fmt(addr?.V3_PREDICTED) + "; actual=42.72; conservative=" + fmt(addr?.V3_CONSERVATIVE_SCORE) + "; correctly rejected=" + (addr?.CORRECTLY_REJECTED || "NA") + ".",
    "- VECTOR-MATH-X/V001: prediction=" + fmt(vector?.V3_PREDICTED) + "; actual=44.22; conservative=" + fmt(vector?.V3_CONSERVATIVE_SCORE) + "; correctly rejected=" + (vector?.CORRECTLY_REJECTED || "NA") + ".",
    "- Strict held-out means each case's Official label was excluded from case selection, weights, fitting, and margin calibration.",
    "",
    "## C13 and proposed suite",
    "",
    "- C13 is FP16, 128x16384, WIDE_MULTIROW_FULL; full-label association is descriptive only. Route-averaged Spearman=" + fmt(c13.routeSp) + "; removing one Route changes association direction=" + bool(c13.changedSign) + ". See C13-FORENSIC.md.",
    "- Proposed V2 suite (" + suiteV2Rows.length + "): " + (suiteV2Rows.map(r => r.CASE_ID + (r.KEEP_FOR_V2 === "DIAGNOSTIC_COVERAGE_ONLY" ? "[diagnostic-only]" : "[core-provisional]")).join(", ") || "none") + ". Diagnostic-only cases failed stability and must not enter model features.",
    "",
    "## Readiness",
    "",
    "- LOCAL_JUDGE_READY=NO; READY_FOR_PERFORMANCE_WAVE=NO.",
    "- MORE_OFFICIAL_LABELS_NEEDED=" + bool(needMore) + ". Current candidate labels include no result above 45.16; route-held-out ranking and sensitivity therefore cannot validate winner selection.",
    "- Information candidates are recorded in ONLINE-CALIBRATION-CANDIDATES.tsv with missing common-vector/model-disagreement fields explicitly NA. They are not Online-submission recommendations until the unified suite is measured and Planning approves quota.",
    "- Gate failure is intentional: formula alignment and a computable nested pipeline do not demonstrate useful Local-to-Online predictive power."
  ];
  writeText(path.join(data.dir, "LOCAL-JUDGE-V3-VALIDATION.md"), lines.join("\n"));
  return { lm, rm, looRaw, routeRaw, needMore, ready, computableKnown };
}
function predictionsForNewCandidate(data, inputFile, outDir) {
  const candidates = table(inputFile).map(r => ({
    route: r.ROUTE || "UNNAMED_ROUTE", revision: r.REVISION || "UNNAMED_VERSION", sha: r.SOURCE_SHA || "NA",
    ratios: Object.fromEntries(data.suite.map(c => [c.CASE_ID, num(r["LOCAL_" + c.CASE_ID + "_RATIO"])])),
    correctness: r.CORRECTNESS_STATUS || "UNKNOWN", quality: r.LOCAL_QUALITY || "UNKNOWN", row: r
  }));
  const fit = fitSelectionAndMargin(data.records, data);
  const prepared = prepare(data.records, data);
  const rows = candidates.map(c => {
    const pseudo = { route: c.route, revision: c.revision, ratios: c.ratios };
    const p = fit.config ? fitPredict(data.records, pseudo, fit.config, prepared) : { pred: null, cases: [] };
    const cons = p.pred !== null && fit.margin !== null ? clamp(p.pred - fit.margin) : null;
    const stable = p.cases.length >= 3 && p.cases.every(id => prepared.stability.some(s => s.CASE_ID === id && s.KEEP_FOR_MODEL === "YES"));
    const above = data.records.some(r => r.score > CHAMPION);
    const correctnessPass = /^PASS/i.test(c.correctness);
    const eligible = cons !== null && cons > CHAMPION && correctnessPass && stable && above;
    return {
      ROUTE: c.route, REVISION: c.revision, SOURCE_SHA: c.sha, SELECTED_MODEL: fit.config,
      SELECTED_CASES: p.cases.join(","), PER_CASE_LOCAL_RATIOS: p.cases.map(id => id + "=" + fmt(c.ratios[id])).join(";"),
      PREDICTED_SCORE: fmt(p.pred), OVERPREDICTION_MARGIN: fmt(fit.margin), CONSERVATIVE_SCORE: fmt(cons),
      CHAMPION_SCORE: CHAMPION, CORRECTNESS_STATUS: c.correctness, LOCAL_QUALITY: c.quality,
      ONLINE_ELIGIBLE: bool(eligible), GATE_REASON: !above ? "NO_ABOVE_CHAMPION_TRAINING_LABEL; SENSITIVITY_UNVALIDATED" :
        (p.cases.length < 3 ? "FEWER_THAN_3_SELECTED_CASES" : (cons === null ? "PREDICTION_OR_MARGIN_NOT_COMPUTABLE" : (eligible ? "ALL_CRITERIA_PASS" : "CONSERVATIVE_SCORE_OR_CORRECTNESS_GATE_FAILED")))
    };
  });
  writeTsv(path.join(outDir, "LOCAL-JUDGE-V3-PREDICTIONS.tsv"),
    ["ROUTE", "REVISION", "SOURCE_SHA", "SELECTED_MODEL", "SELECTED_CASES", "PER_CASE_LOCAL_RATIOS", "PREDICTED_SCORE", "OVERPREDICTION_MARGIN", "CONSERVATIVE_SCORE", "CHAMPION_SCORE", "CORRECTNESS_STATUS", "LOCAL_QUALITY", "ONLINE_ELIGIBLE", "GATE_REASON"], rows);
  return rows;
}
function generateAll(data) {
  const loo = outerValidation(data.records, data, "LOO");
  const routeOut = outerValidation(data.records, data, "ROUTE_OUT");
  writeValidationFile(path.join(data.dir, "LOCAL-JUDGE-NESTED-LOOCV.tsv"), "LOO", loo);
  writeValidationFile(path.join(data.dir, "LOCAL-JUDGE-LEAVE-ONE-ROUTE-OUT.tsv"), "ROUTE_OUT", routeOut);
  const stability = generateStabilityAndPredictiveness(data, loo);
  const suiteV2Rows = suiteV2(data, loo, stability);
  const fpRows = falsePositiveAudit(data, loo);
  const c13 = c13Forensic(data);
  const recommendations = candidateRecommendations(data);
  const modelRows = modelComparison(data);
  writeSpec(data, suiteV2Rows);
  const summary = writeValidationReport(data, loo, routeOut, fpRows, suiteV2Rows, c13, []);
  return { loo, routeOut, fpRows, suiteV2Rows, c13, recommendations, modelRows, summary };
}
function main() {
  const args = parseArgs(process.argv.slice(2));
  const data = load(args.root);
  if (args.command === "validate-all") {
    const r = generateAll(data);
    console.log(JSON.stringify({
      v2_feature_selection_leakage: true,
      loocv: predictionMetrics(r.loo, true),
      route_out: predictionMetrics(r.routeOut, true),
      addr_h3: r.fpRows.find(x => x.VERSION === "HOTLOOP-ADDR-HOIST-CHAMPION-X/V001"),
      vector_math_v001: r.fpRows.find(x => x.VERSION === "VECTOR-MATH-X/V001"),
      suite_v2: r.suiteV2Rows.map(x => x.CASE_ID),
      more_official_labels_needed: r.summary.needMore,
      local_judge_ready: false
    }, null, 2));
    return;
  }
  if (args.command === "validate-loocv") {
    const rows = outerValidation(data.records, data, "LOO");
    const result = writeValidationFile(path.join(args.out, "LOCAL-JUDGE-NESTED-LOOCV.tsv"), "LOO", rows);
    console.log(JSON.stringify(result, null, 2)); return;
  }
  if (args.command === "validate-route-out") {
    const rows = outerValidation(data.records, data, "ROUTE_OUT");
    const result = writeValidationFile(path.join(args.out, "LOCAL-JUDGE-LEAVE-ONE-ROUTE-OUT.tsv"), "ROUTE_OUT", rows);
    console.log(JSON.stringify(result, null, 2)); return;
  }
  if (args.command === "audit-false-positive") {
    const rows = outerValidation(data.records, data, "LOO");
    const fp = falsePositiveAudit(data, rows);
    console.log(JSON.stringify(fp, null, 2)); return;
  }
  if (args.command === "predict") {
    if (!args.input) throw new Error("predict requires --input candidate-ratios.tsv");
    console.log(JSON.stringify(predictionsForNewCandidate(data, args.input, args.out), null, 2)); return;
  }
  throw new Error("Unknown command: " + args.command);
}

main();
