#!/usr/bin/env node

/*
 * Main-2 Local Judge V2.
 *
 * This is a calibrated surrogate, not an implementation of the hidden Online
 * runner. It consumes only unified-suite measurements and retained Official
 * results. Missing, failed, or outlier-heavy cases remain explicit NA values.
 */

import fs from "node:fs";
import path from "node:path";

const ANCHOR_ROUTE = "R31B";
const ANCHOR_REVISION = "V011";
const ANCHOR_SCORE = 45.16;
const OLD_ADDR_SIGNAL = -24.19332;
const FORMULA = "s_i = 100 / (1 + log_1.5(time_i / best_time_i)); total = mean(s_i)";

function exists(file) {
  try {
    fs.accessSync(file);
    return true;
  } catch {
    return false;
  }
}

function readText(file) {
  return fs.readFileSync(file, "utf8");
}

function writeText(file, text) {
  fs.mkdirSync(path.dirname(file), { recursive: true });
  fs.writeFileSync(file, text.endsWith("\n") ? text : `${text}\n`, "utf8");
}

function num(value) {
  if (value === undefined || value === null || value === "" || value === "NA") return null;
  const parsed = Number(value);
  return Number.isFinite(parsed) ? parsed : null;
}

function fixed(value, digits = 6) {
  const parsed = num(value);
  return parsed === null ? "NA" : parsed.toFixed(digits);
}

function median(values) {
  const sorted = values.filter((value) => num(value) !== null).map(Number).sort((a, b) => a - b);
  if (!sorted.length) return null;
  const middle = Math.floor(sorted.length / 2);
  return sorted.length % 2 ? sorted[middle] : (sorted[middle - 1] + sorted[middle]) / 2;
}

function mean(values) {
  const valid = values.filter((value) => num(value) !== null).map(Number);
  return valid.length ? valid.reduce((sum, value) => sum + value, 0) / valid.length : null;
}

function geometricMean(values) {
  const valid = values.filter((value) => num(value) !== null && Number(value) > 0).map(Number);
  return valid.length ? Math.exp(valid.reduce((sum, value) => sum + Math.log(value), 0) / valid.length) : null;
}

function parseTsv(text) {
  const lines = text.replace(/^\uFEFF/, "").split(/\r?\n/).filter((line) => line.length > 0);
  if (!lines.length) return [];
  const headers = lines[0].split("\t");
  return lines.slice(1).map((line) => {
    const cells = line.split("\t");
    return Object.fromEntries(headers.map((header, index) => [header, cells[index] ?? ""]));
  });
}

function readTsv(file) {
  return exists(file) ? parseTsv(readText(file)) : [];
}

function cell(value) {
  if (value === undefined || value === null || value === "") return "NA";
  return String(value).replace(/[\t\r\n]+/g, " ");
}

function writeTsv(file, headers, rows) {
  const lines = [headers.join("\t")];
  for (const row of rows) lines.push(headers.map((header) => cell(row[header])).join("\t"));
  writeText(file, lines.join("\n"));
}

function parseArgs(argv) {
  const args = { root: process.cwd(), runRoots: [], baselineRoots: [], out: null };
  for (let index = 0; index < argv.length; index += 1) {
    const arg = argv[index];
    if (arg === "--root") args.root = path.resolve(argv[++index]);
    else if (arg === "--run-root") args.runRoots.push(path.resolve(argv[++index]));
    else if (arg === "--baseline-run-root") args.baselineRoots.push(path.resolve(argv[++index]));
    else if (arg === "--out") args.out = path.resolve(argv[++index]);
  }
  args.out ||= path.join(args.root, "研究/主代理/MAIN-2");
  return args;
}

function measurementRows(runRoots) {
  const rows = [];
  for (const runRoot of runRoots) {
    const file = path.join(runRoot, "measurements.tsv");
    for (const row of readTsv(file)) rows.push({ ...row, _runRoot: runRoot });
  }
  return rows;
}

function versionStatus(runRoots) {
  const map = new Map();
  for (const runRoot of runRoots) {
    for (const row of readTsv(path.join(runRoot, "version-status.tsv"))) {
      map.set(`${row.route}/${row.revision}`, { ...row, _runRoot: runRoot });
    }
  }
  return map;
}

function aggregateSide(rows, caseId, side) {
  const relevant = rows.filter((row) => row.case_id === caseId && row.side === side);
  const byRun = new Map();
  for (const row of relevant) {
    const run = row.run || "?";
    if (!byRun.has(run)) byRun.set(run, row);
  }
  const runs = [...byRun.values()];
  const validRuns = runs.filter((row) => num(row.device_median_us) !== null
    && num(row.device_median_us) > 0 && num(row.command_rc) === 0 && num(row.bad) === 0);
  const medians = validRuns.map((row) => Number(row.device_median_us));
  const cvs = validRuns.map((row) => num(row.device_cv)).filter((value) => value !== null);
  const mads = validRuns.map((row) => {
    const medianValue = num(row.device_median_us);
    const madValue = num(row.device_mad_us);
    return medianValue && madValue !== null ? madValue / medianValue : null;
  }).filter((value) => value !== null);
  const aggregateMedian = median(medians);
  const quality = validRuns.length >= 3 && aggregateMedian !== null && (Math.max(...cvs, 0) <= 0.5)
    ? "QUALIFIED" : validRuns.length ? "JITTER_OR_PARTIAL" : "INVALID";
  return {
    runs: runs.length,
    validRuns: validRuns.length,
    median_us: aggregateMedian,
    mean_run_median_us: mean(medians),
    median_cv: median(cvs),
    median_mad_ratio: median(mads),
    throughput: aggregateMedian ? null : null,
    quality,
    bad: runs.some((row) => num(row.bad) !== 0),
    commandFailure: runs.some((row) => num(row.command_rc) !== 0)
  };
}

function formulaScore(times, bestTimes) {
  if (!times || times.length !== 15 || !bestTimes || bestTimes.length !== 15) return null;
  const scores = times.map((time, index) => {
    const t = num(time);
    const best = num(bestTimes[index]);
    if (!(t > 0) || !(best > 0)) return null;
    return 100 / (1 + Math.log(t / best) / Math.log(1.5));
  });
  if (scores.some((score) => score === null)) return null;
  return mean(scores);
}

function correlation(xs, ys) {
  const pairs = xs.map((x, index) => [num(x), num(ys[index])]).filter(([x, y]) => x !== null && y !== null);
  if (pairs.length < 3) return null;
  const mx = mean(pairs.map(([x]) => x));
  const my = mean(pairs.map(([, y]) => y));
  let numerator = 0;
  let dx = 0;
  let dy = 0;
  for (const [x, y] of pairs) {
    numerator += (x - mx) * (y - my);
    dx += (x - mx) ** 2;
    dy += (y - my) ** 2;
  }
  return dx > 0 && dy > 0 ? numerator / Math.sqrt(dx * dy) : null;
}

function ranks(values) {
  const indexed = values.map((value, index) => ({ value, index })).sort((a, b) => a.value - b.value);
  const output = Array(values.length).fill(0);
  let index = 0;
  while (index < indexed.length) {
    let end = index + 1;
    while (end < indexed.length && indexed[end].value === indexed[index].value) end += 1;
    const rank = (index + end - 1) / 2 + 1;
    for (let cursor = index; cursor < end; cursor += 1) output[indexed[cursor].index] = rank;
    index = end;
  }
  return output;
}

function spearman(xs, ys) {
  const pairs = xs.map((x, index) => [num(x), num(ys[index])]).filter(([x, y]) => x !== null && y !== null);
  if (pairs.length < 3) return null;
  return correlation(ranks(pairs.map(([x]) => x)), ranks(pairs.map(([, y]) => y)));
}

function kendall(xs, ys) {
  const pairs = xs.map((x, index) => [num(x), num(ys[index])]).filter(([x, y]) => x !== null && y !== null);
  if (pairs.length < 3) return null;
  let concordant = 0;
  let discordant = 0;
  let ties = 0;
  for (let i = 0; i < pairs.length; i += 1) {
    for (let j = i + 1; j < pairs.length; j += 1) {
      const dx = pairs[i][0] - pairs[j][0];
      const dy = pairs[i][1] - pairs[j][1];
      if (dx === 0 || dy === 0) ties += 1;
      else if (dx * dy > 0) concordant += 1;
      else discordant += 1;
    }
  }
  const denominator = concordant + discordant + ties;
  return denominator ? (concordant - discordant) / denominator : null;
}

function fitLinear(xs, ys, monotonic = false) {
  const pairs = xs.map((x, index) => [num(x), num(ys[index])]).filter(([x, y]) => x !== null && y !== null);
  if (pairs.length < 3) return null;
  const mx = mean(pairs.map(([x]) => x));
  const my = mean(pairs.map(([, y]) => y));
  let numerator = 0;
  let denominator = 0;
  for (const [x, y] of pairs) {
    numerator += (x - mx) * (y - my);
    denominator += (x - mx) ** 2;
  }
  let slope = denominator ? numerator / denominator : 0;
  if (monotonic) slope = Math.min(0, slope);
  return { slope, intercept: my - slope * mx, n: pairs.length };
}

function fitNearest(x, xs, ys) {
  const pairs = xs.map((value, index) => [num(value), num(ys[index])]).filter(([value, y]) => value !== null && y !== null);
  if (!pairs.length || x === null) return null;
  pairs.sort((a, b) => Math.abs(a[0] - x) - Math.abs(b[0] - x));
  return pairs[0][1];
}

function fitInterpolation(x, xs, ys) {
  const pairs = xs.map((value, index) => [num(value), num(ys[index])]).filter(([value, y]) => value !== null && y !== null)
    .sort((a, b) => a[0] - b[0]);
  if (!pairs.length || x === null) return null;
  if (pairs.length === 1 || x <= pairs[0][0]) return pairs[0][1];
  if (x >= pairs[pairs.length - 1][0]) return pairs[pairs.length - 1][1];
  for (let index = 1; index < pairs.length; index += 1) {
    if (x <= pairs[index][0]) {
      const [x0, y0] = pairs[index - 1];
      const [x1, y1] = pairs[index];
      const weight = (x - x0) / (x1 - x0 || 1);
      return y0 + (y1 - y0) * weight;
    }
  }
  return pairs[pairs.length - 1][1];
}

function clampScore(value) {
  const parsed = num(value);
  return parsed === null ? null : Math.max(0, Math.min(100, parsed));
}

function parseBestTimes(root) {
  const rows = readTsv(path.join(root, "研究/主代理/MAIN-2/ONLINE-BEST-TIMES.tsv"));
  return rows.sort((a, b) => Number(a.CASE) - Number(b.CASE)).map((row) => num(row.BEST_TIME_US));
}

function officialMap(root) {
  const map = new Map();
  for (const row of readTsv(path.join(root, "研究/主代理/MAIN-2/ALL-OFFICIAL-RESULTS.tsv"))) {
    map.set(`${row.ROUTE}/${row.REVISION}`, row);
  }
  return map;
}

function statusMap(runRoots) {
  const map = new Map();
  for (const runRoot of runRoots) {
    for (const row of readTsv(path.join(runRoot, "version-status.tsv"))) {
      map.set(`${row.route}/${row.revision}`, row);
    }
  }
  return map;
}

function caseFeatures(manifestRow, suite, allMeasurements, anchorStats, anchorRoute, anchorRevision) {
  const route = manifestRow.ROUTE;
  const revision = manifestRow.REVISION;
  const versionRows = allMeasurements.filter((row) => row.route === route && row.revision === revision);
  const cases = {};
  const ratios = [];
  let validCases = 0;
  let outlierCases = 0;
  for (const testCase of suite) {
    const candidate = aggregateSide(versionRows, testCase.CASE_ID, "candidate");
    const parent = aggregateSide(versionRows, testCase.CASE_ID, "parent");
    const anchor = anchorStats[testCase.CASE_ID];
    const ratio = route === anchorRoute && revision === anchorRevision
      ? 1 : (candidate.median_us !== null && anchor?.median_us ? candidate.median_us / anchor.median_us : null);
    const caseValid = candidate.validRuns >= 3 && parent.validRuns >= 3 && anchor?.validRuns >= 3 && ratio !== null;
    if (caseValid) validCases += 1;
    if (candidate.quality !== "QUALIFIED" || parent.quality !== "QUALIFIED") outlierCases += 1;
    if (ratio !== null) ratios.push(ratio);
    cases[testCase.CASE_ID] = {
      parent,
      candidate,
      ratio: caseValid ? ratio : null,
      valid: caseValid,
      throughput: candidate.median_us ? (Number(testCase.ROWS) * Number(testCase.WIDTH)) / (candidate.median_us / 1e6) : null,
      path: testCase.EXPECTED_PATH,
      shape: `${testCase.ROWS}x${testCase.WIDTH}`,
      dtype: testCase.DTYPE
    };
  }
  const geoRatio = geometricMean(ratios);
  const meanRatio = mean(ratios);
  const medianRatio = median(ratios);
  const logRatio = geoRatio ? Math.log(geoRatio) : null;
  return {
    route,
    revision,
    sourceSha: manifestRow.SOURCE_SHA,
    officialScore: num(manifestRow.OFFICIAL_SCORE),
    cases,
    validCases,
    outlierCases,
    meanRatio,
    geoRatio,
    medianRatio,
    logRatio,
    quality: validCases >= Math.max(8, Math.ceil(suite.length * 0.75)) && outlierCases <= 4 ? "QUALIFIED"
      : validCases >= 4 ? "PARTIAL" : "POOR"
  };
}

function anchorCaseStats(manifest, suite, allMeasurements, baselineRoots) {
  const anchorRows = allMeasurements.filter((row) => row.route === ANCHOR_ROUTE && row.revision === ANCHOR_REVISION
    && (!baselineRoots.length || baselineRoots.includes(row._runRoot)));
  const output = {};
  for (const testCase of suite) {
    const parent = aggregateSide(anchorRows, testCase.CASE_ID, "parent");
    output[testCase.CASE_ID] = parent;
  }
  return output;
}

function surrogateScore(ratio, anchorTimes, bestTimes) {
  if (ratio === null || !anchorTimes || anchorTimes.length !== 15 || !bestTimes || bestTimes.length !== 15) return null;
  const base = formulaScore(anchorTimes, bestTimes);
  const scaled = formulaScore(anchorTimes.map((time) => Number(time) * ratio), bestTimes);
  return base === null || scaled === null ? null : ANCHOR_SCORE + scaled - base;
}

function modelPredictions(rows, anchorTimes, bestTimes) {
  const candidates = rows.filter((row) => !(row.route === ANCHOR_ROUTE && row.revision === ANCHOR_REVISION)
    && row.officialScore !== null && row.logRatio !== null && row.validCases >= 4);
  const output = [];
  for (const row of rows) {
    const isAnchor = row.route === ANCHOR_ROUTE && row.revision === ANCHOR_REVISION;
    if (isAnchor) {
      output.push({ ...row, predictedFormulaGeo: ANCHOR_SCORE, predictedFormulaMean: ANCHOR_SCORE, predictedLinear: ANCHOR_SCORE, predictedNearest: ANCHOR_SCORE, predictedMonotonic: ANCHOR_SCORE, trainingN: 0, primary: ANCHOR_SCORE });
      continue;
    }
    const train = candidates.filter((candidate) => !(candidate.route === row.route && candidate.revision === row.revision));
    const xs = train.map((candidate) => candidate.logRatio);
    const ys = train.map((candidate) => candidate.officialScore);
    const linear = fitLinear(xs, ys, false);
    const monotonic = fitLinear(xs, ys, true);
    const formulaGeo = surrogateScore(row.geoRatio, anchorTimes, bestTimes);
    const formulaMean = surrogateScore(row.meanRatio, anchorTimes, bestTimes);
    const nearest = fitNearest(row.logRatio, xs, ys);
    const monoPrediction = monotonic ? monotonic.intercept + monotonic.slope * row.logRatio : null;
    const linearPrediction = linear ? linear.intercept + linear.slope * row.logRatio : null;
    const primary = linearPrediction !== null ? linearPrediction : formulaGeo;
    output.push({
      ...row,
      predictedFormulaGeo: clampScore(formulaGeo),
      predictedFormulaMean: clampScore(formulaMean),
      predictedLinear: clampScore(linearPrediction),
      predictedNearest: clampScore(nearest),
      predictedMonotonic: clampScore(monoPrediction),
      trainingN: train.length,
      primary: clampScore(primary)
    });
  }
  return output;
}

function officialTimes(row) {
  if (!row) return [];
  return Array.from({ length: 15 }, (_, index) => num(row[`CASE${index + 1}_TIME`]));
}

function metric(values) {
  const valid = values.filter((value) => num(value) !== null).map(Number);
  if (!valid.length) return null;
  return mean(valid);
}

function validationRows(predictedRows) {
  return predictedRows.filter((row) => !(row.route === ANCHOR_ROUTE && row.revision === ANCHOR_REVISION)
    && row.officialScore !== null && row.primary !== null);
}

function buildValidationDoc({ root, rows, cvRows, caseRows, anchor, h3, ready }) {
  const actual = cvRows.map((row) => row.actual);
  const predicted = cvRows.map((row) => row.primary);
  const absErrors = cvRows.map((row) => Math.abs(row.error)).filter((value) => Number.isFinite(value));
  const squared = cvRows.map((row) => row.error ** 2).filter((value) => Number.isFinite(value));
  const fp = cvRows.filter((row) => row.primary > ANCHOR_SCORE && row.actual < ANCHOR_SCORE);
  const fn = cvRows.filter((row) => row.primary <= ANCHOR_SCORE && row.actual > ANCHOR_SCORE);
  const actualRank = rows.filter((row) => row.officialScore !== null && row.logRatio !== null).map((row) => row.officialScore);
  const localRank = rows.filter((row) => row.officialScore !== null && row.logRatio !== null).map((row) => row.logRatio);
  const lines = [
    "# Main-2 Local Judge V2 Validation",
    "",
    `Generated: ${new Date().toISOString()}`,
    "",
    "## Boundary",
    "",
    "- `LOCAL_JUDGE_MODE=CALIBRATED_SURROGATE`.",
    "- Online hidden case shape/dtype/input mapping remains `PARTIAL`; this is not an exact 15-case reproduction.",
    `- Formula: ${FORMULA}`,
    "- Throughput is retained as an anomaly/load feature and is not mixed into the Official score formula.",
    "",
    "## Calibration Set",
    "",
    `- Manifest versions: ${rows.length}`,
    `- Historical candidates: ${rows.filter((row) => !(row.route === ANCHOR_ROUTE && row.revision === ANCHOR_REVISION)).length}`,
    `- Fresh anchor valid cases: ${anchor.validCases}`,
    `- Successfully benchmarked candidates with at least four valid cases: ${rows.filter((row) => row.validCases >= 4 && !(row.route === ANCHOR_ROUTE && row.revision === ANCHOR_REVISION)).length}`,
    "",
    "## LOO Metrics",
    "",
    `- Samples with primary prediction: ${cvRows.length}`,
    `- MAE: ${fixed(metric(absErrors), 6)}`,
    `- RMSE: ${squared.length ? fixed(Math.sqrt(metric(squared)), 6) : "NA"}`,
    `- Spearman (local log-ratio vs Official score): ${fixed(spearman(localRank, actualRank), 6)}`,
    `- Kendall (local log-ratio vs Official score): ${fixed(kendall(localRank, actualRank), 6)}`,
    `- False positives (predicted > 45.16, actual < 45.16): ${fp.length}`,
    `- False negatives (predicted <= 45.16, actual > 45.16): ${fn.length}`,
    "",
    "## ADDR H3 Held-Out Check",
    "",
    `- Old Local signal: ${OLD_ADDR_SIGNAL.toFixed(6)}% (POOR, single-shape; not used as a vector feature).`,
    `- Predicted Official score: ${fixed(h3?.primary, 6)}`,
    "- Actual Official score: 42.72 (15/15 PASS).",
    `- Correctly rejected below anchor: ${h3?.primary !== null && h3.primary <= ANCHOR_SCORE ? "YES" : "NO"}`,
    "",
    "## Case Predictiveness",
    "",
    "Per-case correlation, coverage, and jitter are in `LOCAL-CASE-PREDICTIVENESS.tsv`. A case is not retained solely because it produced a local win.",
    "",
    "## Gate",
    "",
    `- LOCAL_JUDGE_READY=${ready ? "YES" : "NO"}`,
    "- ONLINE_ELIGIBLE=NO until Planning/Review accepts the calibrated error and the external Judge owner approves a submission.",
    "- A single-shape percentage cannot enter the Online gate.",
    "",
    "## Leakage Controls",
    "",
    "- R31B V011 is the fresh baseline anchor and is excluded from candidate training targets.",
    "- Every candidate prediction leaves that candidate out of its linear/nearest/monotonic training set.",
    "- Missing or correctness-invalid local cases are retained as NA, never filled from old Parent logs.",
    ""
  ];
  return lines.join("\n");
}

function run(args) {
  const root = args.root;
  const out = args.out;
  const manifest = readTsv(path.join(root, "研究/主代理/MAIN-2/MAIN2-OFFICIAL-CALIBRATION-SET.tsv"));
  const suite = readTsv(path.join(root, "研究/主代理/MAIN-2/MAIN2-UNIFIED-LOCAL-SUITE-V1.tsv"));
  const allRoots = [...new Set([...args.runRoots, ...args.baselineRoots])];
  if (!allRoots.length) throw new Error("calibrate-v2 requires --run-root and/or --baseline-run-root");
  const measurements = measurementRows(allRoots);
  const statuses = statusMap(allRoots);
  const official = officialMap(root);
  const bestTimes = parseBestTimes(root);
  const anchorOfficial = official.get(`${ANCHOR_ROUTE}/${ANCHOR_REVISION}`);
  const anchorTimes = officialTimes(anchorOfficial);
  const anchorStats = anchorCaseStats(manifest, suite, measurements, args.baselineRoots);
  const featureRows = manifest.map((row) => caseFeatures(row, suite, measurements, anchorStats, ANCHOR_ROUTE, ANCHOR_REVISION));
  const predictedRows = modelPredictions(featureRows, anchorTimes, bestTimes);
  const byKey = new Map(predictedRows.map((row) => [`${row.route}/${row.revision}`, row]));

  const datasetHeaders = [
    "ROUTE", "REVISION", "SOURCE_SHA", "OFFICIAL_SCORE", "LOCAL_RUNNABLE", "CORRECTNESS_VALID",
    "VALID_CASE_COUNT", "LOCAL_QUALITY", "OUTLIER_CASE_COUNT", "LOCAL_MEAN_RATIO", "LOCAL_GEOMEAN_RATIO",
    "LOCAL_MEDIAN_RATIO", "LOCAL_LOG_RATIO_MEAN", "PREDICTED_OFFICIAL_SCORE", "PREDICTION_QUALITY",
    "CALIBRATION_ROLE"
  ];
  for (const testCase of suite) {
    datasetHeaders.push(`LOCAL_${testCase.CASE_ID}_PARENT_US`, `LOCAL_${testCase.CASE_ID}_CANDIDATE_US`,
      `LOCAL_${testCase.CASE_ID}_RATIO`, `LOCAL_${testCase.CASE_ID}_THROUGHPUT`, `LOCAL_${testCase.CASE_ID}_QUALITY`);
  }
  for (let index = 1; index <= 15; index += 1) datasetHeaders.push(`ONLINE_CASE${index}_TIME`, `ONLINE_CASE${index}_RATIO_TO_V011`);

  const datasetRows = predictedRows.map((row) => {
    const key = `${row.route}/${row.revision}`;
    const status = statuses.get(key);
    const officialRow = official.get(key);
    const output = {
      ROUTE: row.route,
      REVISION: row.revision,
      SOURCE_SHA: row.sourceSha,
      OFFICIAL_SCORE: row.officialScore,
      LOCAL_RUNNABLE: status?.build_status === "PASS" ? "YES" : (status?.build_status || "NO"),
      CORRECTNESS_VALID: row.validCases === suite.length ? "YES" : (row.validCases ? "PARTIAL" : "NO"),
      VALID_CASE_COUNT: row.validCases,
      LOCAL_QUALITY: row.quality,
      OUTLIER_CASE_COUNT: row.outlierCases,
      LOCAL_MEAN_RATIO: row.meanRatio,
      LOCAL_GEOMEAN_RATIO: row.geoRatio,
      LOCAL_MEDIAN_RATIO: row.medianRatio,
      LOCAL_LOG_RATIO_MEAN: row.logRatio,
      PREDICTED_OFFICIAL_SCORE: row.primary,
      PREDICTION_QUALITY: row.trainingN >= 3 && row.validCases >= 4 ? "LOO_CALIBRATED" : "INSUFFICIENT_TRAINING",
      CALIBRATION_ROLE: key === `${ANCHOR_ROUTE}/${ANCHOR_REVISION}` ? "FRESH_BASELINE_ANCHOR" : "HELD_OUT_CANDIDATE"
    };
    for (const testCase of suite) {
      const feature = row.cases[testCase.CASE_ID];
      output[`LOCAL_${testCase.CASE_ID}_PARENT_US`] = feature.parent.median_us;
      output[`LOCAL_${testCase.CASE_ID}_CANDIDATE_US`] = feature.candidate.median_us;
      output[`LOCAL_${testCase.CASE_ID}_RATIO`] = feature.ratio;
      output[`LOCAL_${testCase.CASE_ID}_THROUGHPUT`] = feature.throughput;
      output[`LOCAL_${testCase.CASE_ID}_QUALITY`] = feature.valid ? feature.candidate.quality : "INVALID";
    }
    const anchorOnline = official.get(`${ANCHOR_ROUTE}/${ANCHOR_REVISION}`);
    for (let index = 1; index <= 15; index += 1) {
      const time = num(officialRow?.[`CASE${index}_TIME`]);
      const anchorTime = num(anchorOnline?.[`CASE${index}_TIME`]);
      output[`ONLINE_CASE${index}_TIME`] = time;
      output[`ONLINE_CASE${index}_RATIO_TO_V011`] = time && anchorTime ? time / anchorTime : null;
    }
    return output;
  });
  writeTsv(path.join(out, "LOCAL-JUDGE-CALIBRATION-DATASET-V2.tsv"), datasetHeaders, datasetRows);

  const candidates = predictedRows.filter((row) => !(row.route === ANCHOR_ROUTE && row.revision === ANCHOR_REVISION)
    && row.officialScore !== null && row.primary !== null);
  const cvRows = candidates.map((row) => ({
    ROUTE: row.route,
    REVISION: row.revision,
    SOURCE_SHA: row.sourceSha,
    ACTUAL_OFFICIAL_SCORE: row.officialScore,
    PREDICTED_FORMULA_GEOMEAN: row.predictedFormulaGeo,
    PREDICTED_FORMULA_MEAN: row.predictedFormulaMean,
    PREDICTED_LINEAR_LOO: row.predictedLinear,
    PREDICTED_NEAREST_LOO: row.predictedNearest,
    PREDICTED_MONOTONIC_LOO: row.predictedMonotonic,
    PREDICTED_SCORE: row.primary,
    SCORE_ERROR: row.primary === null ? null : row.primary - row.officialScore,
    ABS_ERROR: row.primary === null ? null : Math.abs(row.primary - row.officialScore),
    LOCAL_LOG_RATIO: row.logRatio,
    VALID_CASE_COUNT: row.validCases,
    QUALITY: row.quality,
    MODEL_TRAINING_N: row.trainingN
  }));
  writeTsv(path.join(out, "LOCAL-JUDGE-CROSS-VALIDATION.tsv"), Object.keys(cvRows[0] || {
    ROUTE: "NA", REVISION: "NA"
  }), cvRows);

  const casePredictability = suite.map((testCase) => {
    const values = predictedRows.filter((row) => !(row.route === ANCHOR_ROUTE && row.revision === ANCHOR_REVISION)
      && row.officialScore !== null).map((row) => row.cases[testCase.CASE_ID]);
    const ratios = values.map((value) => value.ratio);
    const scores = predictedRows.filter((row) => !(row.route === ANCHOR_ROUTE && row.revision === ANCHOR_REVISION)
      && row.officialScore !== null).map((row) => row.officialScore);
    const valid = values.filter((value) => value.ratio !== null);
    const corr = correlation(valid.map((value) => Math.log(value.ratio)), scores.filter((_, index) => values[index].ratio !== null));
    const stability = median(valid.map((value) => value.candidate.median_cv));
    const keep = valid.length >= 5 && corr !== null && Math.abs(corr) >= 0.25 && (stability === null || stability <= 0.5);
    return {
      CASE_ID: testCase.CASE_ID,
      SHAPE: `${testCase.ROWS}x${testCase.WIDTH}`,
      DTYPE: testCase.DTYPE,
      PATH: testCase.EXPECTED_PATH,
      VALID_VERSION_COUNT: valid.length,
      CORRELATION_WITH_OFFICIAL: corr,
      STABILITY_MEDIAN_CV: stability,
      KEEP_DROP: keep ? "KEEP" : "DROP_OR_REVIEW",
      REASON: valid.length < 5 ? "INSUFFICIENT_VERSION_COVERAGE" : (corr === null ? "NO_CORRELATION" : "REVIEW_CORRELATION_OR_JITTER")
    };
  });
  writeTsv(path.join(out, "LOCAL-CASE-PREDICTIVENESS.tsv"), Object.keys(casePredictability[0]), casePredictability);

  const anchor = byKey.get(`${ANCHOR_ROUTE}/${ANCHOR_REVISION}`);
  const h3 = byKey.get("HOTLOOP-ADDR-HOIST-CHAMPION-X/V001");
  const cvForMetrics = validationRows(predictedRows).map((row) => ({
    actual: row.officialScore,
    primary: row.primary,
    error: row.primary - row.officialScore,
    route: row.route,
    revision: row.revision
  }));
  const falsePositives = cvForMetrics.filter((row) => row.primary > ANCHOR_SCORE && row.actual < ANCHOR_SCORE);
  const falseNegatives = cvForMetrics.filter((row) => row.primary <= ANCHOR_SCORE && row.actual > ANCHOR_SCORE);
  const ready = cvForMetrics.length >= 5 && anchor?.validCases >= Math.ceil(suite.length * 0.75)
    && h3?.primary !== null && h3?.primary <= ANCHOR_SCORE && falsePositives.length === 0;
  writeText(path.join(out, "LOCAL-JUDGE-VALIDATION-V2.md"), buildValidationDoc({
    root,
    rows: predictedRows,
    cvRows: cvForMetrics,
    caseRows: casePredictability,
    anchor,
    h3,
    ready
  }));

  const vectorRows = suite.map((testCase) => {
    const stats = anchor?.cases[testCase.CASE_ID]?.parent;
    return {
      CASE_ID: testCase.CASE_ID,
      SHAPE: `${testCase.ROWS}x${testCase.WIDTH}`,
      DTYPE: testCase.DTYPE,
      ROWS: testCase.ROWS,
      WIDTH: testCase.WIDTH,
      DEVICE: "7",
      RUNS: stats?.runs,
      VALID_RUNS: stats?.validRuns,
      AVG_LATENCY_US: stats?.mean_run_median_us,
      MEDIAN_LATENCY_US: stats?.median_us,
      MEDIAN_CV: stats?.median_cv,
      THROUGHPUT: stats?.median_us ? (Number(testCase.ROWS) * Number(testCase.WIDTH)) / (stats.median_us / 1e6) : null,
      QUALITY: stats?.quality,
      EVIDENCE: args.baselineRoots.join(",")
    };
  });
  writeTsv(path.join(out, "R31B-V011-LOCAL-VECTOR.tsv"), Object.keys(vectorRows[0]), vectorRows);

  const suiteV2Rows = suite.map((testCase) => {
    const prediction = casePredictability.find((row) => row.CASE_ID === testCase.CASE_ID);
    return { ...testCase, KEEP_STATUS: prediction?.KEEP_DROP || "REVIEW", V2_NOTE: prediction?.REASON || "NO_DATA" };
  });
  writeTsv(path.join(out, "MAIN2-UNIFIED-LOCAL-SUITE-V2.tsv"), [...Object.keys(suite[0]), "KEEP_STATUS", "V2_NOTE"], suiteV2Rows);

  const metricErrors = cvForMetrics.map((row) => Math.abs(row.error));
  const metricSquared = cvForMetrics.map((row) => row.error ** 2);
  const metricX = cvForMetrics.map((row) => row.primary);
  const metricY = cvForMetrics.map((row) => row.actual);
  const summary = {
    calibration_versions_total: manifest.length,
    locally_runnable: Math.max(0, predictedRows.filter((row) => row.validCases >= 4).length - 1),
    successfully_benchmarked: Math.max(0, predictedRows.filter((row) => row.validCases >= 4).length - 1),
    suite_cases_v1: suite.length,
    suite_cases_retained: suiteV2Rows.filter((row) => row.KEEP_STATUS === "KEEP").length,
    mae: metric(metricErrors),
    rmse: metricSquared.length ? Math.sqrt(metric(metricSquared)) : null,
    spearman: spearman(metricX, metricY),
    kendall: kendall(metricX, metricY),
    false_positives: falsePositives.length,
    false_negatives: falseNegatives.length,
    addr_h3_predicted: h3?.primary ?? null,
    addr_h3_actual: 42.72,
    addr_h3_correctly_rejected: h3?.primary !== null && h3.primary <= ANCHOR_SCORE,
    champion_official: ANCHOR_SCORE,
    champion_local_cases: anchor?.validCases ?? 0,
    local_judge_ready: ready,
    mode: "CALIBRATED_SURROGATE",
    online_case_reproducibility: "PARTIAL"
  };
  writeText(path.join(out, "LOCAL-JUDGE-V2-SUMMARY.json"), JSON.stringify(summary, null, 2));
  writeText(path.join(out, "LOCAL-JUDGE-V2-SPEC.md"), [
    "# Local Judge V2 Specification",
    "",
    "## Status",
    "",
    "`LOCAL_JUDGE_MODE=CALIBRATED_SURROGATE`; Online hidden cases are only partially reproducible.",
    `- Unified Suite V1 cases: ${suite.length}`,
    `- Retained Suite V2 cases: ${summary.suite_cases_retained}`,
    `- Fresh R31B V011 valid cases: ${summary.champion_local_cases}`,
    `- LOO candidate rows: ${cvForMetrics.length}`,
    `- MAE: ${fixed(summary.mae, 6)}; RMSE: ${fixed(summary.rmse, 6)}`,
    `- Spearman: ${fixed(summary.spearman, 6)}; Kendall: ${fixed(summary.kendall, 6)}`,
    "",
    "## Input Contract",
    "",
    "The scorer consumes the same suite, device-event protocol, warmup, sample policy, source identity, correctness status, and load evidence for every version. It aggregates the median of three independent run medians and never uses a best-of-run sample.",
    "",
    "## Surrogate Model",
    "",
    "Per-version features are local candidate/anchor ratios. The primary LOO prediction is a simple linear mapping from mean log local ratio to Official score, trained without the held-out version. Formula-scaled, nearest-neighbor, and monotonic alternatives are retained in the cross-validation table.",
    "",
    `The exact Official formula remains: ${FORMULA}`,
    "",
    "## Gate",
    "",
    `LOCAL_JUDGE_READY=${ready ? "YES" : "NO"}`,
    "Online eligibility remains NO until the validation error, false-positive behavior, correctness, source identity, and Planning/Judge-owner decision all pass.",
    "",
    "## Failure Handling",
    "",
    "A failed build, correctness-invalid case, missing run, source mismatch, or high-jitter case is recorded as NA/invalid. No old Parent timing is copied into a fresh feature vector.",
    ""
  ].join("\n"));

  console.log(JSON.stringify({ out, ...summary }, null, 2));
}

try {
  run(parseArgs(process.argv.slice(2)));
} catch (error) {
  console.error(error?.stack || error);
  process.exitCode = 1;
}
