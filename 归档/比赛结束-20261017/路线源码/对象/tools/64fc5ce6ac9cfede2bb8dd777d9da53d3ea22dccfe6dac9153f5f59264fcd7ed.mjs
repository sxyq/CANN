#!/usr/bin/env node

/**
 * Main-2 route census and Local Judge calibration utility.
 *
 * The generator is deliberately read-only with respect to source and evidence:
 * it scans formal ledgers/result packages and writes only the requested reports.
 * The score command accepts a complete 15-case timing vector and applies the
 * public CANNJudge formula without mixing engineering deltas into Official data.
 */

import fs from "node:fs";
import path from "node:path";
import { execFileSync } from "node:child_process";

const ANCHOR_SCORE = 45.16;
const ANCHOR_ROUTE = "R31B";
const ANCHOR_REVISION = "V011";
const ANCHOR_SHA = "a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3";
const FORMULA = "s_i = 100 / (1 + log_1.5(time_i / best_time_i)); total = mean(s_i)";
const PROBLEM_ENDPOINT = "https://cannjudge.cn/api/problems/name/addrmsnormbias";
const RANKING_ENDPOINT = "https://cannjudge.cn/api/problems/6a9a9a99bf41025d6013eb85/ranking?page=1&size=1";

// Main-2 scope is discovered from these committed control documents. The
// generator intentionally avoids maintaining a second hand-edited route list.
const MAIN2_SCOPE_FILES = [
  "研究/主代理/MAIN-2/初始化报告.md",
  "研究/主代理/MAIN-2/CAMPAIGN-STATUS.md",
  "研究/主代理/MAIN-2-W2/campaign-status.md",
  "归档/历史控制文件/main2-r2-route-registry.md",
  "调度/当前任务.tsv"
];

// Snapshot retrieved from the public problem API on 2026-10-04. The report
// records the API source; these values are not inferred from historical logs.
const CURRENT_BEST_TIMES = [
  [1, "6a9a9a99bf41025d6013eb8a", 1.26],
  [2, "6a9a9a99bf41025d6013eb8e", 1.85],
  [3, "6a9a9a99bf41025d6013eb92", 2.47],
  [4, "6a9a9a99bf41025d6013eb96", 4.05],
  [5, "6a9a9a99bf41025d6013eb9a", 5.2],
  [6, "6a9a9a99bf41025d6013eb9e", 9.48],
  [7, "6a9a9a99bf41025d6013eba2", 10.97],
  [8, "6a9a9a99bf41025d6013eba6", 30.16],
  [9, "6a9a9a99bf41025d6013ebaa", 67.79],
  [10, "6a9a9a99bf41025d6013ebae", 46.63],
  [11, "6a9a9a99bf41025d6013ebb2", 126.8],
  [12, "6a9a9a99bf41025d6013ebb6", 75.93],
  [13, "6a9a9a99bf41025d6013ebba", 392.72],
  [14, "6a9a9a99bf41025d6013ebbe", 3588.84],
  [15, "6a9a9a99bf41025d6013ebc2", 7948.23]
];

const DEFAULT_OUT = path.resolve(process.cwd(), "研究/主代理/MAIN-2");

function exists(file) {
  try {
    fs.accessSync(file);
    return true;
  } catch {
    return false;
  }
}

function discoverGitWorktreeRoots(root) {
  try {
    const output = execFileSync("git", ["worktree", "list", "--porcelain"], {
      cwd: root,
      encoding: "utf8",
      stdio: ["ignore", "pipe", "ignore"]
    });
    return output.split(/\r?\n/)
      .map((line) => line.match(/^worktree (.+)$/)?.[1]?.trim())
      .filter((value) => value && exists(value));
  } catch {
    return [];
  }
}

function readText(file) {
  try {
    return fs.readFileSync(file, "utf8");
  } catch {
    return "";
  }
}

function readJson(file) {
  try {
    return JSON.parse(fs.readFileSync(file, "utf8"));
  } catch {
    return null;
  }
}

function writeText(file, content) {
  fs.mkdirSync(path.dirname(file), { recursive: true });
  fs.writeFileSync(file, content.endsWith("\n") ? content : `${content}\n`, "utf8");
}

function scalar(value, fallback = "NA") {
  if (value === undefined || value === null || value === "") return fallback;
  if (typeof value === "number" && !Number.isFinite(value)) return fallback;
  if (typeof value === "object") return compact(value);
  return String(value);
}

function number(value) {
  if (value === undefined || value === null || value === "") return null;
  const n = Number(value);
  return Number.isFinite(n) ? n : null;
}

function fixed(value, digits = 6) {
  const n = number(value);
  return n === null ? "NA" : n.toFixed(digits);
}

function compact(value) {
  if (value === undefined || value === null || value === "") return "NA";
  if (typeof value === "string") return value.replace(/[\t\r\n]+/g, " ").trim() || "NA";
  try {
    return JSON.stringify(value).replace(/[\t\r\n]+/g, " ");
  } catch {
    return String(value).replace(/[\t\r\n]+/g, " ");
  }
}

function tsvCell(value) {
  return compact(value).replace(/\t/g, " ");
}

function tsv(headers, rows) {
  return [headers.join("\t"), ...rows.map((row) => headers.map((h) => tsvCell(row[h])).join("\t"))].join("\n");
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

function isRouteName(value) {
  return /^[A-Z][A-Z0-9]*(?:-[A-Z0-9]+)+$/.test(value)
    && (value.endsWith("-X") || value === "EPI-X-FRESH");
}

function extractRouteNames(text) {
  const tokens = text.match(/\b[A-Z][A-Z0-9]*(?:-[A-Z0-9]+)+\b/g) || [];
  return [...new Set(tokens.filter(isRouteName))];
}

function discoverMain2Scope(root) {
  const routeSources = new Map();
  const add = (route, file) => {
    if (!isRouteName(route)) return;
    if (!routeSources.has(route)) routeSources.set(route, new Set());
    routeSources.get(route).add(file);
  };

  for (const relative of MAIN2_SCOPE_FILES) {
    const file = path.join(root, relative);
    const text = readText(file);
    if (!text) continue;
    const lines = text.split(/\r?\n/);
    if (relative.endsWith("当前任务.tsv")) {
      for (const line of lines) {
        if (!line.includes("OWNER=MAIN-2")) continue;
        add(line.split("\t")[0]?.trim() || "", relative);
      }
      continue;
    }
    if (relative.endsWith("初始化报告.md")) {
      for (const line of lines) {
        if (!/^##\s+3\.\d+\s+/.test(line)) continue;
        for (const route of extractRouteNames(line)) add(route, relative);
      }
      continue;
    }
    // These are route registry/status tables. Restrict extraction to table
    // rows so prose mentioning another Main or historical donor is not enough
    // to claim ownership of that route. The R2 registry also records the
    // approved INTEGRATION route in section headings rather than a table row.
    for (const line of lines) {
      if (relative.endsWith("main2-r2-route-registry.md") && /^\s*##/.test(line)) {
        for (const route of extractRouteNames(line)) add(route, relative);
        continue;
      }
      if (!/^\s*\|/.test(line)) continue;
      if (relative.endsWith("main2-r2-route-registry.md")) {
        const firstCell = line.split("|")[1]?.trim() || "";
        for (const route of extractRouteNames(firstCell)) add(route, relative);
        continue;
      }
      for (const route of extractRouteNames(line)) add(route, relative);
    }
  }

  const routes = [...routeSources.keys()].sort();
  return {
    routes,
    routeSet: new Set(routes),
    sources: [...new Set([...routeSources.values()].flatMap((files) => [...files]))].sort(),
    routeSources: Object.fromEntries([...routeSources.entries()].map(([route, files]) => [route, [...files].sort()]))
  };
}

function walk(dir, callback, options = {}) {
  if (!exists(dir)) return;
  const skip = new Set(options.skip || [".git", "node_modules", "CMakeFiles", "build", "build-timing"]);
  let entries;
  try {
    entries = fs.readdirSync(dir, { withFileTypes: true });
  } catch {
    return;
  }
  for (const entry of entries) {
    if (skip.has(entry.name)) continue;
    const full = path.join(dir, entry.name);
    if (entry.isDirectory()) walk(full, callback, options);
    else callback(full);
  }
}

function findNamedDirs(root, names) {
  const found = [];
  if (!exists(root)) return found;
  const skip = [".git", "node_modules", "CMakeFiles", "build", "build-timing", "local-experiments", "server_runs"];
  function visit(dir) {
    let entries;
    try {
      entries = fs.readdirSync(dir, { withFileTypes: true });
    } catch {
      return;
    }
    for (const entry of entries) {
      if (skip.includes(entry.name)) continue;
      const full = path.join(dir, entry.name);
      if (!entry.isDirectory()) continue;
      if (names.has(entry.name)) found.push(full);
      else visit(full);
    }
  }
  visit(root);
  return found;
}

function collectEvidence(roots) {
  const localMeta = [];
  const localJson = [];
  const localResults = [];
  const onlineMeta = [];
  const onlineResults = [];
  const seen = new Set();
  for (const root of roots) {
    for (const dir of findNamedDirs(root, new Set(["本地实验", "线上结果"]))) {
      walk(dir, (file) => {
        const base = path.basename(file);
        if (!file.endsWith(".json")) return;
        const rel = path.relative(dir, file);
        const key = `${base}:${rel}:${readText(file).length}:${readText(file).slice(0, 80)}`;
        if (seen.has(key)) return;
        seen.add(key);
        if (dir.endsWith("本地实验")) {
          localJson.push(file);
          if (base === "source-meta.json") localMeta.push(file);
          if (base === "local-result.json") localResults.push(file);
        } else {
          if (base === "source-meta.json") onlineMeta.push(file);
          if (base === "result.json") onlineResults.push(file);
        }
      }, { skip: [".git", "node_modules", "CMakeFiles", "build", "build-timing"] });
    }
  }
  return { localMeta, localJson, localResults, onlineMeta, onlineResults };
}

function get(obj, ...keys) {
  for (const key of keys) {
    if (obj && obj[key] !== undefined && obj[key] !== null && obj[key] !== "") return obj[key];
  }
  return null;
}

function normalizeRevision(value) {
  const text = scalar(value, "").trim();
  if (!text) return "";
  return text.split("/")[0].trim();
}

function routeRevisionFromPath(file, kind) {
  const parts = path.normalize(file).split(path.sep);
  let marker = -1;
  if (kind === "online") {
    marker = parts.lastIndexOf("线上结果");
    if (marker < 0) marker = parts.lastIndexOf("online");
  } else {
    marker = parts.lastIndexOf("本地实验");
  }
  if (marker < 0 || !parts[marker + 1]) return { route: "", revision: "" };
  const route = parts[marker + 1];
  let revision = parts[marker + 2] || "";
  if (revision === "result.json" || revision === "source-meta.json" || revision === "local-result.json") revision = "ROOT_RESULT";
  if (kind === "online" && /^[0-9a-f]{20,}$/i.test(revision)) revision = parts[marker + 2] || "ROOT_RESULT";
  return { route, revision: normalizeRevision(revision) || "ROOT_RESULT" };
}

function parseMetaFile(file, kind) {
  const data = readJson(file) || {};
  const inferred = routeRevisionFromPath(file, kind);
  return {
    file,
    data,
    route: scalar(get(data, "ROUTE", "route"), inferred.route),
    revision: normalizeRevision(get(data, "REVISION", "revision")) || inferred.revision
  };
}

function caseRows(data) {
  if (Array.isArray(data?.result)) return data.result;
  if (Array.isArray(data?.testcases)) return data.testcases;
  return [];
}

function caseTime(row) {
  return number(get(row, "time", "timeUs", "time_us"));
}

function caseBest(row) {
  return number(get(row, "best_time", "bestTimeUs", "best_time_us"));
}

function scoreCase(time, best) {
  if (!(time > 0) || !(best > 0)) return null;
  return 100 / (1 + Math.log(time / best) / Math.log(1.5));
}

function formulaScore(times, bestTimes) {
  if (!Array.isArray(times) || times.length !== 15) return null;
  const scores = times.map((time, index) => scoreCase(Number(time), Number(bestTimes[index])));
  if (scores.some((score) => score === null)) return null;
  return scores.reduce((sum, value) => sum + value, 0) / scores.length;
}

function normalizeOfficial(file, metaMap) {
  const data = readJson(file) || {};
  const inferred = routeRevisionFromPath(file, "online");
  const sidecar = metaMap.get(path.dirname(file));
  const route = scalar(get(data, "route"), scalar(sidecar?.route, inferred.route));
  const revision = normalizeRevision(get(data, "revision")) || normalizeRevision(sidecar?.revision) || inferred.revision;
  const rows = caseRows(data);
  const times = rows.map(caseTime);
  const bests = rows.map(caseBest);
  const scores = rows.map((row, index) => {
    const recorded = number(get(row, "score"));
    return recorded !== null && recorded !== 0 ? recorded : scoreCase(times[index], bests[index]);
  });
  const officialScore = number(get(data, "officialScore", "official_score"));
  const calculated = formulaScore(times, bests);
  const source = data.source || {};
  const submissionId = scalar(get(data, "submissionId", "submission_id"), scalar(get(sidecar?.data || {}, "submission_id", "submissionId"), "NA"));
  const sourceSha = scalar(get(source, "sha256", "source_sha256"), scalar(get(sidecar?.data || {}, "submission_sha256", "source_sha256", "SOURCE_SHA"), "NA"));
  const passCount = get(data, "passCount", "pass_count") ?? rows.filter((row) => /pass|accept/i.test(String(get(row, "status", "testcase_status")))).length;
  const detailsMissing = rows.length !== 15 || rows.some((row) => {
    const time = caseTime(row);
    const best = caseBest(row);
    return !(time > 0) || !(best > 0);
  });
  return {
    route,
    revision,
    sourceSha,
    submissionId,
    passCount: scalar(passCount),
    officialScore: officialScore === null ? "NA" : fixed(officialScore, 3),
    formulaScore: calculated === null ? "NA" : fixed(calculated, 6),
    status: scalar(get(data, "status"), "UNKNOWN"),
    caseDetailsMissing: detailsMissing ? "YES" : "NO",
    times,
    bests,
    scores,
    path: file,
    raw: data
  };
}

function dedupeOfficial(records) {
  const map = new Map();
  for (const record of records) {
    const key = [record.route, record.revision, record.submissionId, record.sourceSha, record.officialScore, record.status].join("|");
    if (!map.has(key)) map.set(key, record);
  }
  return [...map.values()].sort((a, b) => `${a.route}/${a.revision}/${a.path}`.localeCompare(`${b.route}/${b.revision}/${b.path}`));
}

function chooseLocalRecords(files, kind) {
  const records = files.map((file) => parseMetaFile(file, kind));
  const map = new Map();
  for (const record of records) {
    const key = `${record.route}|${record.revision}`;
    if (!map.has(key)) map.set(key, record);
    else if (record.file.includes("cann-w2-main2-control")) map.set(key, record);
  }
  return map;
}

function findLocalResult(localResultFiles, route, revision) {
  const candidates = localResultFiles.filter((file) => {
    const inferred = routeRevisionFromPath(file, "local");
    return inferred.route === route && inferred.revision === revision;
  });
  if (!candidates.length) return null;
  const preferred = candidates.find((file) => file.includes("cann-w2-main2-control")) || candidates[0];
  return { file: preferred, data: readJson(preferred) || {} };
}

function parseLatestLocalScores(root) {
  const files = [
    path.join(root, "MAIN2-LATEST-LOCAL-SCORES.tsv"),
    path.join(root, "研究/主代理/MAIN-2/MAIN2-FRESH-NORMALIZED-SCORES.tsv"),
    path.join(root, "研究/主代理/MAIN-2/MAIN2-BASELINE-NORMALIZED-SCORES.tsv")
  ];
  const rows = [];
  for (const file of files) {
    if (!exists(file)) continue;
    for (const row of parseTsv(readText(file))) {
      const route = scalar(get(row, "ROUTE", "route"), "");
      const revision = normalizeRevision(get(row, "REVISION", "LATEST_REVISION", "revision"));
      if (!route || !revision || revision === "NONE" || revision === "CURRENT") continue;
      rows.push({ file, route, revision, row });
    }
  }
  const map = new Map();
  // File order is intentional: the current score matrix wins over the fresh
  // and historical fallback matrices, while the first available fallback is
  // retained instead of being overwritten by an older snapshot.
  for (const item of rows) {
    const key = `${item.route}|${item.revision}`;
    if (!map.has(key)) map.set(key, item);
  }
  return map;
}

function parseCalibration(root) {
  const file = path.join(root, "调度/本地线上校准.tsv");
  return exists(file) ? parseTsv(readText(file)) : [];
}

function findLedger(root) {
  const file = path.join(root, "技术路线/全版本记录.tsv");
  return exists(file) ? parseTsv(readText(file)) : [];
}

function findOnlineFor(officialRecords, route, revision) {
  const matches = officialRecords.filter((record) => record.route === route && record.revision === revision);
  if (!matches.length) return null;
  return matches.find((record) => record.officialScore !== "NA") || matches[0];
}

function firstValue(objects, keys) {
  for (const object of objects) {
    const value = get(object, ...keys);
    if (value !== null) return value;
  }
  return null;
}

function firstNumber(objects, keys) {
  for (const object of objects) {
    const value = number(get(object, ...keys));
    if (value !== null) return value;
  }
  return null;
}

function statusField(objects, keys, fallback = "NA") {
  let value = null;
  for (const key of keys) {
    for (const object of objects) {
      const candidate = get(object, key);
      if (candidate !== null) {
        value = candidate;
        break;
      }
    }
    if (value !== null) break;
  }
  if (value && typeof value === "object") {
    return scalar(firstValue([value], ["verdict", "status", "result", "value", "decision"]), compact(value));
  }
  return scalar(value, fallback);
}

function buildStatus(objects) {
  const value = (() => {
    const keys = [
    "BUILD_STATUS", "SERVER_BUILD_STATUS", "build_status", "COMPILE", "BUILD", "compile", "build", "status"
    ];
    for (const key of keys) {
      for (const object of objects) {
        const candidate = get(object, key);
        if (candidate !== null) return candidate;
      }
    }
    return null;
  })();
  if (value && typeof value === "object") {
    const nested = firstValue([value], ["verdict", "status", "result", "value", "decision"]);
    if (nested !== null) return scalar(nested);
    const rc = firstNumber([value], ["BUILD_RC", "build_rc", "compile_rc", "link_rc"]);
    const compile = scalar(get(value, "ASC_COMPILE", "asc_compile"), "");
    const link = scalar(get(value, "LINK", "link"), "");
    if (rc !== null && rc !== 0 || /fail|not_reached/i.test(`${compile} ${link}`)) return "BUILD_FAILED";
    if (/pass/i.test(`${compile} ${link}`)) return "BUILD_PASS";
    return compact(value);
  }
  if (value === null || /^(MISSING|NA)$/i.test(String(value))) {
    const execution = scalar(firstValue(objects, ["execution_result", "build_result"]), "");
    const returnCode = firstNumber(objects, ["runner_return_code", "build_rc", "compile_rc"]);
    if (/pass|success/i.test(execution) || returnCode === 0) return "PASS";
  }
  return scalar(value, "MISSING");
}

function supplementalLocalEvidence(localJsonFiles, route, revision) {
  const priority = (file) => {
    const base = path.basename(file);
    if (/^performance-final.*\.json$/i.test(base)) return 100;
    if (base === "performance-result.json") return 90;
    if (base === "local-result.json") return 80;
    if (/^correctness-result.*\.json$/i.test(base)) return 70;
    if (/^build-result.*\.json$/i.test(base)) return 60;
    if (base === "source-meta.json") return 10;
    return 0;
  };
  return localJsonFiles
    .filter((file) => {
      const inferred = routeRevisionFromPath(file, "local");
      return inferred.route === route && inferred.revision === revision && priority(file) > 0;
    })
    .map((file) => ({ file, data: readJson(file) || {}, priority: priority(file) }))
    .sort((a, b) => b.priority - a.priority || a.file.localeCompare(b.file));
}

function optionalScore(value) {
  const text = scalar(value, "NA");
  return /^(NONE|N\/A|NO_RECORD|NO_SCORE|UNKNOWN)$/i.test(text) ? "NA" : text;
}

function localMetricFields(latest, localData, metaData, ledger, officialLocal = []) {
  const row = latest?.row || {};
  const measurement = localData?.measurement && typeof localData.measurement === "object" ? localData.measurement : {};
  const metricsObject = localData?.metrics && typeof localData.metrics === "object" ? localData.metrics : {};
  const officialDelta = firstNumber(officialLocal, ["LOCAL_DELTA_PERCENT", "localDeltaPercent", "local_delta_percent", "local_delta_pct", "local_score_percent"]);
  const objects = officialDelta !== null
    ? [...officialLocal, localData, measurement, metricsObject, metaData, ledger]
    : [row, localData, measurement, metricsObject, metaData, ledger];
  const runRows = Array.isArray(localData?.runs) ? localData.runs : [];
  const parentRuns = [0, 1, 2].map((index) => runRows[index]
    ? firstNumber([runRows[index]], ["parent_avg_us", "parent_us", "parent_average_us", "PARENT_AVG_US"])
    : firstNumber([row], [
      `RUN${index + 1}_PARENT_US`, `PARENT_RUN${index + 1}_US`,
      `parent_run${index + 1}_us`
    ]));
  const candidateRuns = [0, 1, 2].map((index) => runRows[index]
    ? firstNumber([runRows[index]], ["candidate_avg_us", "candidate_us", "candidate_average_us", "CANDIDATE_AVG_US"])
    : firstNumber([row], [
      `RUN${index + 1}_CANDIDATE_US`, `CANDIDATE_RUN${index + 1}_US`,
      `candidate_run${index + 1}_us`
    ]));
  const parentAvg = firstNumber(objects, ["PARENT_AVG_US", "CHAMPION_AVG_US", "parent_avg_us", "parent_average_us", "parent_metric_us"]);
  const candidateAvg = firstNumber(objects, ["CANDIDATE_AVG_US", "candidate_avg_us", "candidate_average_us", "candidate_metric_us"]);
  const explicitDelta = firstNumber(objects, ["LATENCY_DELTA_PERCENT", "LOCAL_DELTA_PERCENT", "localDeltaPercent", "local_delta_percent", "local_delta_pct", "local_score_percent"]);
  const derivedDelta = explicitDelta === null && parentAvg !== null && candidateAvg !== null && parentAvg !== 0
    ? (candidateAvg / parentAvg - 1) * 100
    : null;
  const probes = [
    ...(Array.isArray(localData?.paired_local_probes?.cases) ? localData.paired_local_probes.cases : []),
    ...(Array.isArray(localData?.local_probes?.cases) ? localData.local_probes.cases : [])
  ];
  const probeDeltas = probes.map((probe) => number(get(probe, "delta_pct", "delta_percent", "local_delta_percent"))).filter((value) => value !== null);
  const delta = explicitDelta ?? derivedDelta ?? (probeDeltas.length ? probeDeltas.reduce((a, b) => a + b, 0) / probeDeltas.length : null);
  const parentThroughput = firstNumber(objects, [
    "CHAMPION_THROUGHPUT_INVOCATIONS_PER_SEC", "PARENT_THROUGHPUT", "parent_throughput",
    "parent_throughput_invocations_per_sec", "parent_invocations_per_sec"
  ]);
  const candidateThroughput = firstNumber(objects, [
    "CANDIDATE_THROUGHPUT_INVOCATIONS_PER_SEC", "CANDIDATE_THROUGHPUT", "candidate_throughput",
    "candidate_throughput_invocations_per_sec", "candidate_invocations_per_sec"
  ]);
  const throughputDelta = firstNumber(objects, ["THROUGHPUT_DELTA_PERCENT", "throughput_delta_percent"])
    ?? (parentThroughput !== null && candidateThroughput !== null && parentThroughput !== 0
      ? (candidateThroughput / parentThroughput - 1) * 100
      : null);
  const qualityValue = firstValue(objects, ["MEASUREMENT_QUALITY", "measurement_quality", "QUALITY", "LOCAL_QUALITY", "localQuality", "quality", "LOAD_QUALITY", "load_quality"]);
  const quality = qualityValue && typeof qualityValue === "object"
    ? scalar(firstValue([qualityValue], ["classification", "quality", "status", "load_quality"]), compact(qualityValue))
    : scalar(qualityValue, "NA");
  return {
    run1Parent: parentRuns[0] === null ? "NA" : fixed(parentRuns[0]),
    run1Candidate: candidateRuns[0] === null ? "NA" : fixed(candidateRuns[0]),
    run2Parent: parentRuns[1] === null ? "NA" : fixed(parentRuns[1]),
    run2Candidate: candidateRuns[1] === null ? "NA" : fixed(candidateRuns[1]),
    run3Parent: parentRuns[2] === null ? "NA" : fixed(parentRuns[2]),
    run3Candidate: candidateRuns[2] === null ? "NA" : fixed(candidateRuns[2]),
    parentAvg: parentAvg === null ? "NA" : fixed(parentAvg),
    candidateAvg: candidateAvg === null ? "NA" : fixed(candidateAvg),
    deltaPercent: delta === null ? "NA" : fixed(delta, 6),
    parentThroughput: parentThroughput === null ? "NA" : fixed(parentThroughput, 3),
    candidateThroughput: candidateThroughput === null ? "NA" : fixed(candidateThroughput, 3),
    throughputDelta: throughputDelta === null ? "NA" : fixed(throughputDelta, 6),
    quality
  };
}

function normalizeLocalRow({ ledger, meta, local, latest, online, calibration, supplemental = [] }) {
  const metaData = meta?.data || {};
  const localData = local?.data || {};
  const supplementalData = supplemental.map((item) => item.data);
  const statusObjects = [...supplementalData, latest?.row || {}, ledger, localData, metaData];
  const route = scalar(get(ledger, "ROUTE"), scalar(meta?.route, "UNKNOWN"));
  const revision = normalizeRevision(get(ledger, "REVISION")) || scalar(meta?.revision, "UNKNOWN");
  const sourceSha = scalar(get(ledger, "SOURCE_SHA"), scalar(firstValue([metaData, localData, ...supplementalData], ["SOURCE_SHA", "source_sha", "source_sha256", "SOURCE_SHA256", "candidate_source_sha256", "CANDIDATE_SOURCE_SHA256", "LOCAL_SHA256"]), "NA"));
  const parent = scalar(get(ledger, "DIRECT_PARENT"), scalar(firstValue([metaData, localData, ...supplementalData], ["DIRECT_PARENT", "direct_parent"]), "NA"));
  const parentSha = scalar(get(ledger, "PARENT_SOURCE_SHA"), scalar(firstValue([metaData, localData, ...supplementalData], ["PARENT_SOURCE_SHA", "PARENT_SOURCE_SHA256", "parent_source_sha", "parent_source_sha256"]), "NA"));
  const hypothesis = scalar(get(ledger, "HYPOTHESIS"), scalar(firstValue([metaData, localData, ...supplementalData], ["HYPOTHESIS_ID", "hypothesis_id", "SINGLE_HYPOTHESIS", "single_hypothesis", "ASSIGNED_HYPOTHESIS", "assigned_hypothesis", "SINGLE_HYPOTHESIS_AS_DELIVERED", "hypothesis"]), "NA"));
  const mechanism = scalar(firstValue([metaData, localData, ...supplementalData], ["ARCHITECTURE", "MECHANISM", "mechanism", "change_summary"]), hypothesis);
  const singleVariable = scalar(firstValue([metaData, localData, ...supplementalData], ["SINGLE_CHANGE_AUDIT", "single_change_audit"]), "NA");
  const exactFunction = scalar(firstValue([metaData, localData, ...supplementalData], ["EXACT_FUNCTION", "FUNCTION", "function"]), "NA");
  const targetShapes = scalar(firstValue([metaData, localData, ...supplementalData], ["TARGET_SHAPES", "TARGET_SHAPE", "target_shapes", "target_shape", "target"]), "NA");
  const targetDtypes = scalar(firstValue([metaData, localData, ...supplementalData], ["TARGET_DTYPES", "TARGET_DTYPE", "target_dtypes", "target_dtype", "dtype"]), "NA");
  const officialLocal = online?.raw?.localEngineeringEvidence && typeof online.raw.localEngineeringEvidence === "object"
    ? [online.raw.localEngineeringEvidence]
    : [];
  const metrics = localMetricFields(latest, localData, metaData, ledger, officialLocal);
  const calibrationRow = calibration.find((row) => row.route === route && normalizeRevision(row.revision) === revision);
  const localVerdict = statusField(statusObjects, ["FINAL_VERDICT", "final_verdict", "LOCAL_VERDICT", "local_verdict", "STATUS", "status", "decision", "DECISION", "VERDICT", "verdict"], "NOT_COMPLETE");
  const build = buildStatus(statusObjects);
  const correctness = statusField(statusObjects, ["CORRECTNESS_STATUS", "SERVER_CORRECTNESS_STATUS", "correctness_status", "CORRECTNESS", "correctness", "verdict", "result"], "MISSING");
  const classification = scalar(get(calibrationRow || {}, "decision", "CLASSIFICATION", "proxy_quality", "direction_match"), "NOT_ASSESSED");
  const officialScore = optionalScore(online?.officialScore || get(ledger, "OFFICIAL_SCORE"));
  const onlinePassCount = online?.passCount || scalar(get(ledger, "ONLINE_RESULT"), "NA");
  return {
    ROUTE: route,
    REVISION: revision,
    HYPOTHESIS_ID: hypothesis,
    DIRECT_PARENT: parent,
    PARENT_SHA: parentSha,
    SOURCE_SHA: sourceSha,
    EXACT_FUNCTION: exactFunction,
    MECHANISM: mechanism,
    SINGLE_VARIABLE_CHANGE: singleVariable,
    TARGET_SHAPES: targetShapes,
    TARGET_DTYPES: targetDtypes,
    BUILD_STATUS: build,
    CORRECTNESS_STATUS: correctness,
    RUN1_PARENT_US: metrics.run1Parent,
    RUN1_CANDIDATE_US: metrics.run1Candidate,
    RUN2_PARENT_US: metrics.run2Parent,
    RUN2_CANDIDATE_US: metrics.run2Candidate,
    RUN3_PARENT_US: metrics.run3Parent,
    RUN3_CANDIDATE_US: metrics.run3Candidate,
    PARENT_AVG_US: metrics.parentAvg,
    CANDIDATE_AVG_US: metrics.candidateAvg,
    LATENCY_DELTA_PERCENT: metrics.deltaPercent,
    PARENT_THROUGHPUT: metrics.parentThroughput,
    CANDIDATE_THROUGHPUT: metrics.candidateThroughput,
    THROUGHPUT_DELTA_PERCENT: metrics.throughputDelta,
    LOCAL_QUALITY: metrics.quality,
    LOCAL_VERDICT: localVerdict,
    ONLINE_SUBMITTED: online ? "YES" : "NO",
    OFFICIAL_SCORE: officialScore,
    ONLINE_PASS_COUNT: onlinePassCount,
    ONLINE_SHA: online?.sourceSha || "NA",
    LOCAL_ONLINE_CLASSIFICATION: classification,
    EVIDENCE_PATH: scalar(get(ledger, "EVIDENCE_PATH"), local?.file || meta?.file || "NA")
  };
}

function revisionInventory(root, evidence, officialRecords, routeSet) {
  const ledgerRows = findLedger(root).filter((row) => routeSet.has(row.ROUTE) && row.REVISION && row.REVISION !== "CURRENT");
  const metaMap = chooseLocalRecords(evidence.localMeta, "local");
  const localByKey = new Map();
  for (const file of evidence.localResults) {
    const inferred = routeRevisionFromPath(file, "local");
    localByKey.set(`${inferred.route}|${inferred.revision}`, { file, data: readJson(file) || {} });
  }
  const latest = parseLatestLocalScores(root);
  const calibration = parseCalibration(root);
  const rows = [];
  const keys = new Map();
  for (const ledger of ledgerRows) keys.set(`${ledger.ROUTE}|${normalizeRevision(ledger.REVISION)}`, ledger);
  for (const item of metaMap.values()) {
    if (!routeSet.has(item.route)) continue;
    if (!item.revision || item.revision === "CURRENT" || item.revision === "ROOT_RESULT") continue;
    const key = `${item.route}|${item.revision}`;
    if (!keys.has(key)) keys.set(key, { ROUTE: item.route, REVISION: item.revision, SOURCE_SHA: get(item.data, "SOURCE_SHA", "source_sha", "source_sha256", "candidate_source_sha256") || "" });
  }
  for (const file of evidence.localResults) {
    const inferred = routeRevisionFromPath(file, "local");
    if (!routeSet.has(inferred.route) || !inferred.revision || inferred.revision === "CURRENT" || inferred.revision === "ROOT_RESULT") continue;
    const data = readJson(file) || {};
    const route = scalar(get(data, "ROUTE", "route"), inferred.route);
    const revision = normalizeRevision(get(data, "REVISION", "revision")) || inferred.revision;
    const key = `${route}|${revision}`;
    if (!keys.has(key)) keys.set(key, {
      ROUTE: route,
      REVISION: revision,
      SOURCE_SHA: get(data, "SOURCE_SHA", "source_sha", "source_sha256", "candidate_source_sha256") || ""
    });
  }
  for (const [key, ledger] of keys) {
    const [route, revision] = key.split("|");
    const meta = metaMap.get(key);
    const local = localByKey.get(key) || findLocalResult(evidence.localResults, route, revision);
    const supplemental = supplementalLocalEvidence(evidence.localJson, route, revision);
    const online = findOnlineFor(officialRecords, route, revision);
    rows.push(normalizeLocalRow({ ledger, meta, local, latest: latest.get(key), online, calibration, supplemental }));
  }
  return rows.sort((a, b) => `${a.ROUTE}/${a.REVISION}`.localeCompare(`${b.ROUTE}/${b.REVISION}`));
}

function officialVectorRows() {
  return CURRENT_BEST_TIMES.map(([index, testcaseId, bestTimeUs]) => ({
    CASE: `case${index}`,
    TESTCASE_ID: testcaseId,
    BEST_TIME_US: bestTimeUs,
    SOURCE: PROBLEM_ENDPOINT,
    RETRIEVED_SNAPSHOT: "2026-10-04",
    STATUS: "PUBLIC_API_CONFIRMED"
  }));
}

function makeTree(rows, researchOnly, scope) {
  const children = new Map();
  const parentNames = new Set();
  const rowKeys = new Set(rows.map((row) => `${row.ROUTE}|${row.REVISION}`));
  const anchorKey = "__R31B_V011_ANCHOR__";
  const rowKeyForParent = (row) => {
    const parent = row.DIRECT_PARENT === "NA" ? "UNRESOLVED_PARENT" : row.DIRECT_PARENT;
    const parentSha = row.PARENT_SHA;
    if (/R31B[- ]V011|FROZEN_R31B_V011/i.test(parent)
      || parentSha === ANCHOR_SHA
      || (parentSha !== "NA" && ANCHOR_SHA.startsWith(String(parentSha).replace(/[^0-9a-f]/gi, "")))) {
      return anchorKey;
    }
    const sameRouteRevision = `${row.ROUTE}|${normalizeRevision(parent)}`;
    if (rowKeyForParent.revisionKeys.has(sameRouteRevision)) return sameRouteRevision;
    for (const candidate of rows) {
      const candidateKey = `${candidate.ROUTE}|${candidate.REVISION}`;
      const labels = [
        `${candidate.ROUTE} ${candidate.REVISION}`,
        `${candidate.ROUTE}-${candidate.REVISION}`,
        `${candidate.ROUTE}/${candidate.REVISION}`
      ];
      if (labels.some((label) => parent.includes(label))) return candidateKey;
      if (candidate.SOURCE_SHA !== "NA" && candidate.SOURCE_SHA.length >= 8
        && parent.includes(candidate.SOURCE_SHA)) return candidateKey;
      if (candidate.SOURCE_SHA !== "NA" && parentSha !== "NA"
        && String(candidate.SOURCE_SHA).startsWith(String(parentSha).replace(/[^0-9a-f]/gi, ""))) return candidateKey;
    }
    return `RAW:${parent}`;
  };
  rowKeyForParent.revisionKeys = rowKeys;
  for (const row of rows) {
    const parentKey = rowKeyForParent(row);
    if (parentKey.startsWith("RAW:")) parentNames.add(parentKey.slice(4));
    if (!children.has(parentKey)) children.set(parentKey, []);
    children.get(parentKey).push(row);
  }
  const lines = [
    "# Main-2 Revision Tree",
    "",
    `Anchor: ${ANCHOR_ROUTE} ${ANCHOR_REVISION} (Official ${ANCHOR_SCORE}; SHA ${ANCHOR_SHA})`,
    "",
    "The edges below use recorded DIRECT_PARENT/PARENT_SOURCE_SHA. Version order is not treated as code inheritance.",
    ""
  ];
  function render(parent, prefix, visited) {
    const list = (children.get(parent) || []).sort((a, b) => `${a.ROUTE}/${a.REVISION}`.localeCompare(`${b.ROUTE}/${b.REVISION}`));
    list.forEach((row, index) => {
      const last = index === list.length - 1;
      const branch = last ? "└── " : "├── ";
      lines.push(`${prefix}${branch}${row.ROUTE} ${row.REVISION} [parent=${row.DIRECT_PARENT}; local=${row.LOCAL_VERDICT}; official=${row.OFFICIAL_SCORE}]`);
      const key = `${row.ROUTE}|${row.REVISION}`;
      if (!visited.has(key)) {
        visited.add(key);
        render(key, `${prefix}${last ? "    " : "│   "}`, visited);
      }
    });
  }
  lines.push(`- ${ANCHOR_ROUTE} ${ANCHOR_REVISION}`);
  render(anchorKey, "  ", new Set());
  const external = [...parentNames];
  lines.push("", "## Non-anchor Parent Roots", "");
  for (const parent of external.sort()) {
    lines.push(`- ${parent}`);
    render(`RAW:${parent}`, "  ", new Set());
  }
  lines.push("", "## Research-only Routes (no implemented Revision in this inventory)", "");
  for (const route of researchOnly.sort()) lines.push(`- ${route}`);
  lines.push("", "## Scope", "", `Routes in Main-2 scope: ${scope.routes.length}`, `Implemented revisions: ${rows.length}`, "Official scores are never inferred from local percentages.", "", "Scope sources:");
  for (const source of scope.sources) lines.push(`- ${source}`);
  return lines.join("\n");
}

function calibrationDataset(root, officialRecords, revisionRows) {
  const rows = [];
  const calibration = parseCalibration(root);
  const freshAnchorFile = path.join(root, "研究/主代理/MAIN-2/R31B-V011-FRESH-LOCAL-BASELINE.tsv");
  for (const record of parseTsv(readText(freshAnchorFile))) {
    rows.push({
      SAMPLE_TYPE: "FRESH_ANCHOR_BASELINE",
      ROUTE: ANCHOR_ROUTE,
      REVISION: ANCHOR_REVISION,
      LOCAL_DELTA_PERCENT: "NA",
      LOCAL_QUALITY: scalar(record.QUALITY, "NA"),
      TARGET_SHAPES: scalar(record.TARGET_SHAPE, "NA"),
      TARGET_DTYPES: scalar(record.TARGET_DTYPE, "NA"),
      OFFICIAL_SCORE: fixed(ANCHOR_SCORE, 3),
      FORMULA_REPLAY_SCORE: "NA",
      OFFICIAL_DELTA_VS_ANCHOR: "0.000",
      CLASSIFICATION: "ANCHOR_ONLY",
      CALIBRATION_ROLE: "FRESH_BASELINE_NOT_FIT",
      EVIDENCE: scalar(record.EVIDENCE_PATH, freshAnchorFile),
      NOTE: `FRESH_NPU_RUN=${scalar(record.FRESH_NPU_RUN, "UNKNOWN")}; runs=${record.RUN1_US}/${record.RUN2_US}/${record.RUN3_US} us; runner=${scalar(record.RUNNER_BINARY_SHA256, "UNKNOWN")}; no Candidate and no composite Local score.`
    });
  }
  for (const record of officialRecords) {
    rows.push({
      SAMPLE_TYPE: "ONLINE_FORMULA_REPLAY",
      ROUTE: record.route,
      REVISION: record.revision,
      LOCAL_DELTA_PERCENT: "NA",
      LOCAL_QUALITY: "NO_LOCAL_FEATURES",
      TARGET_SHAPES: "UNKNOWN",
      TARGET_DTYPES: "UNKNOWN",
      OFFICIAL_SCORE: record.officialScore,
      FORMULA_REPLAY_SCORE: record.formulaScore,
      OFFICIAL_DELTA_VS_ANCHOR: number(record.officialScore) === null ? "NA" : fixed(number(record.officialScore) - ANCHOR_SCORE, 3),
      CLASSIFICATION: "FORMULA_ONLY_NOT_LOCAL_MODEL",
      CALIBRATION_ROLE: "NOT_FIT",
      EVIDENCE: record.path,
      NOTE: "Official 15-case payload; no local paired timing is assumed."
    });
  }
  for (const record of calibration) {
    const revision = normalizeRevision(record.revision);
    const score = revisionRows.find((row) => row.ROUTE === record.route && row.REVISION === revision);
    rows.push({
      SAMPLE_TYPE: "LOCAL_ONLINE_CALIBRATION",
      ROUTE: record.route,
      REVISION: revision,
      LOCAL_DELTA_PERCENT: scalar(record.local_delta),
      LOCAL_QUALITY: scalar(record.timing_quality || record.proxy_quality),
      TARGET_SHAPES: scalar(record.local_probe),
      TARGET_DTYPES: "NOT_RECOVERED",
      OFFICIAL_SCORE: scalar(record.online_after),
      FORMULA_REPLAY_SCORE: "NA",
      OFFICIAL_DELTA_VS_ANCHOR: number(record.online_after) === null ? "NA" : fixed(number(record.online_after) - ANCHOR_SCORE, 3),
      CLASSIFICATION: scalar(record.decision || record.proxy_quality),
      CALIBRATION_ROLE: "DESCRIPTIVE_ONLY_NO_EXACT_SUITE",
      EVIDENCE: scalar(record.context, score?.EVIDENCE_PATH || "NA"),
      NOTE: "Local engineering signal and Official total use different workload units; never fit as a per-case model."
    });
  }
  for (const record of revisionRows) {
    if (calibration.some((item) => item.route === record.ROUTE && normalizeRevision(item.revision) === record.REVISION)) continue;
    rows.push({
      SAMPLE_TYPE: "LOCAL_ONLY_REVISION",
      ROUTE: record.ROUTE,
      REVISION: record.REVISION,
      LOCAL_DELTA_PERCENT: record.LATENCY_DELTA_PERCENT,
      LOCAL_QUALITY: record.LOCAL_QUALITY,
      TARGET_SHAPES: record.TARGET_SHAPES,
      TARGET_DTYPES: record.TARGET_DTYPES,
      OFFICIAL_SCORE: record.OFFICIAL_SCORE,
      FORMULA_REPLAY_SCORE: "NA",
      OFFICIAL_DELTA_VS_ANCHOR: number(record.OFFICIAL_SCORE) === null ? "NA" : fixed(number(record.OFFICIAL_SCORE) - ANCHOR_SCORE, 3),
      CLASSIFICATION: record.LOCAL_ONLINE_CLASSIFICATION,
      CALIBRATION_ROLE: "DESCRIPTIVE_ONLY",
      EVIDENCE: record.EVIDENCE_PATH,
      NOTE: "No exact 15-case local suite; retained for route census and gate auditing."
    });
  }
  return rows;
}

function correlation(xs, ys) {
  if (xs.length < 2 || ys.length !== xs.length) return null;
  const rank = (values) => {
    const sorted = [...values].map((value, index) => ({ value, index })).sort((a, b) => a.value - b.value);
    const ranks = Array(values.length);
    sorted.forEach((item, index) => { ranks[item.index] = index + 1; });
    return ranks;
  };
  const a = rank(xs);
  const b = rank(ys);
  const meanA = a.reduce((s, v) => s + v, 0) / a.length;
  const meanB = b.reduce((s, v) => s + v, 0) / b.length;
  const numerator = a.reduce((s, v, i) => s + (v - meanA) * (b[i] - meanB), 0);
  const denA = Math.sqrt(a.reduce((s, v) => s + (v - meanA) ** 2, 0));
  const denB = Math.sqrt(b.reduce((s, v) => s + (v - meanB) ** 2, 0));
  return denA && denB ? numerator / (denA * denB) : null;
}

function validationReport(root, officialRecords, revisionRows) {
  const replay = officialRecords.filter((record) => {
    const status = String(record.status || "").toLowerCase();
    return /pass|accepted/.test(status)
      && record.caseDetailsMissing === "NO"
      && record.officialScore !== "NA"
      && record.formulaScore !== "NA";
  });
  const errors = replay.map((record) => Number(record.formulaScore) - Number(record.officialScore));
  const mae = errors.length ? errors.reduce((s, v) => s + Math.abs(v), 0) / errors.length : null;
  const rmse = errors.length ? Math.sqrt(errors.reduce((s, v) => s + v * v, 0) / errors.length) : null;
  const actual = replay.map((record) => Number(record.officialScore));
  const predictedReplay = replay.map((record) => Number(record.formulaScore));
  const calibration = parseCalibration(root);
  const falsePositives = calibration.filter((record) => String(record.false_positive).toUpperCase() === "YES").length;
  const falseNegatives = calibration.filter((record) => String(record.false_negative).toUpperCase() === "YES").length;
  const addr = calibration.find((record) => record.route === "HOTLOOP-ADDR-HOIST-CHAMPION-X" && normalizeRevision(record.revision) === "V001");
  const lines = [
    "# Local Judge V1 Validation",
    "",
    "## Result",
    "",
    "`LOCAL_JUDGE_MODE=CALIBRATED_SURROGATE`.",
    "The public API exposes testcase IDs and tbest values, but not the exact 15-case shape/dtype/input mapping. A complete Local→Online held-out prediction cannot be claimed from the current evidence.",
    "",
    `- Official result files scanned: ${officialRecords.length}`,
    `- Passing Official rows eligible for formula replay: ${replay.length}`,
    `- Local→Online rows with comparable full-suite features: 0`,
    `- Historical explicit false positives in the calibration ledger: ${falsePositives}`,
    `- Historical explicit false negatives in the calibration ledger: ${falseNegatives}`,
    `- Formula replay MAE: ${fixed(mae, 9)} points (diagnostic only)`,
    `- Formula replay RMSE: ${fixed(rmse, 9)} points (diagnostic only)`,
    `- Formula replay rank correlation: ${fixed(correlation(predictedReplay, actual), 6)} (same-payload replay; not predictive validation)`,
    "- Local→Online MAE/RMSE/rank correlation: NOT_COMPUTABLE",
    "- Champion decision accuracy / held-out false-positive rate: NOT_COMPUTABLE for a Local model; exact suite features are missing.",
    "",
    "## Why This Is Not a Successful Predictor Yet",
    "",
    "The existing local observations are single-shape or route-specific paired probes. They do not provide 15 local timings with a known case mapping. Fitting a model from the Official per-case payload would leak the label and would not measure Local→Online prediction. Therefore all local calibration rows are descriptive-only and no train/validation fit is reported.",
    "",
    "## ADDR H3",
    "",
    `- Old local engineering delta: ${scalar(addr?.local_delta, "-24.193320%")}`,
    "- Old local quality: POOR; historical Candidate faster 1/3; extreme sample dominates.",
    "- V1 predicted Official score: UNAVAILABLE (case coverage gate; no exact 15-case local vector).",
    "- V1 gate result: ONLINE_ELIGIBLE=NO; a missing prediction cannot become a false positive.",
    "- Actual Official score: 42.72 (15/15 PASS).",
    "- Per-case ADDR H3 timing/score details: MISSING in the retained result package; no case-level gain/loss is synthesized.",
    "- Conclusion: V1 does not yet numerically predict ADDR H3; it correctly blocks the old single-shape signal from being used as an Online gate.",
    "",
    "## Formula Replay Scope",
    "",
    `The replay uses ${FORMULA} with the best_time values stored in each historical result payload. It checks arithmetic consistency only. It does not validate shape coverage, local noise handling, throughput consistency, or candidate ranking.`,
    "",
    "## Required Next Calibration Evidence",
    "",
    "1. Recover exact 15-case shape/dtype/input definitions or obtain a sanctioned local runner that emits the same ordered cases.",
    "2. Run fresh R31B V011 and each candidate on that exact suite with the same binary, device class, warmup, sample policy, and throughput capture.",
    "3. Reserve at least two Official-labelled revisions as held-out validation before fitting any local-to-online mapping.",
    "4. Until then, keep all new performance revisions and Online submissions frozen at Planning review.",
    ""
  ];
  return lines.join("\n");
}

function specReport() {
  return [
    "# Local Judge V1 Specification",
    "",
    "## Status",
    "",
    "- Version: `LOCAL-JUDGE-V1`",
    "- Mode: `CALIBRATED_SURROGATE`",
    "- Exact 15-case reproduction: `NO`; testcase IDs and tbest are known, hidden shape/dtype/input mapping is not.",
    "- Official anchor: `R31B V011`, 15/15, 45.16.",
    "- Direct Online submission: disabled.",
    "",
    "## Formula",
    "",
    `\`${FORMULA}\``,
    "",
    "The scorer uses the ordered 15-case vector. It does not average local percentage deltas and it does not mix throughput into Official score.",
    "",
    "## Input Contract",
    "",
    "A candidate is numerically scorable only when the input contains exactly 15 ordered candidate timings and the matching 15 parent timings from the same suite, device class, binary/timing API, warmup, and sampling policy. Each row must retain raw samples, median, mean, throughput, load snapshot, and source identity.",
    "",
    "Required JSON shape for the score command:",
    "",
    "```json",
    "{",
    "  \"label\": \"candidate\",",
    "  \"parent_us\": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15],",
    "  \"candidate_us\": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15],",
    "  \"quality\": \"QUALIFIED\",",
    "  \"source_sha\": \"...\"",
    "}",
    "```",
    "",
    "The scorer returns per-case parent/candidate latency, local ratio, predicted point score, predicted total Official score, delta versus the parent, and quality. It rejects incomplete or POOR vectors rather than filling missing cases.",
    "",
    "## Gate",
    "",
    "`ONLINE_ELIGIBLE=YES` requires correctness clean, source identity clean, exact 15-case coverage, qualified same-binary measurements, no outlier-dominated result, consistent latency/throughput direction, and a held-out-calibrated prediction above the current champion by a margin derived from validation error. Since validation error is currently unavailable, the margin and Online eligibility are `UNDEFINED`/`NO`.",
    "",
    "## Case Reproducibility",
    "",
    "Current classification is `PARTIAL`: the public API confirms the 15 IDs, order, and current best-time snapshot; the public problem description confirms supported dtype and dimension ranges, but exact per-case shapes, dtypes, row counts, and hidden inputs are not exposed.",
    "",
    "## ADDR H3 Policy",
    "",
    "ADDR V001/H3 is retained as a validation/negative-gate case. Its old `-24.193320%` is an engineering delta with `POOR` quality, not a predicted Official score. V1 returns `UNAVAILABLE` for this candidate because the exact 15-case vector is missing, which blocks Online instead of declaring a numeric win.",
    ""
  ].join("\n");
}

function scoreCommand(inputFile, bestFile) {
  const input = readJson(inputFile);
  if (!input) throw new Error(`Cannot parse input JSON: ${inputFile}`);
  const best = bestFile ? readJson(bestFile) : CURRENT_BEST_TIMES.map((row) => row[2]);
  const parent = input.parent_us;
  const candidate = input.candidate_us;
  if (!Array.isArray(parent) || !Array.isArray(candidate) || parent.length !== 15 || candidate.length !== 15) {
    throw new Error("SCORE_INPUT_REQUIRES_EXACTLY_15_PARENT_AND_CANDIDATE_TIMINGS");
  }
  if (!Array.isArray(best) || best.length !== 15) throw new Error("BEST_TIME_VECTOR_REQUIRES_15_VALUES");
  const parentScores = parent.map((time, index) => scoreCase(time, best[index]));
  const candidateScores = candidate.map((time, index) => scoreCase(time, best[index]));
  if (parentScores.some((value) => value === null) || candidateScores.some((value) => value === null)) {
    throw new Error("SCORE_INPUT_TIMINGS_MUST_BE_POSITIVE");
  }
  const parentScore = parentScores.reduce((s, v) => s + v, 0) / 15;
  const candidateScore = candidateScores.reduce((s, v) => s + v, 0) / 15;
  const rows = candidate.map((time, index) => ({
    case: index + 1,
    parent_us: parent[index],
    candidate_us: time,
    local_ratio: Number(time) / Number(parent[index]),
    predicted_online_latency_us: time,
    predicted_point_score: candidateScores[index]
  }));
  const output = {
    LOCAL_JUDGE_VERSION: "LOCAL-JUDGE-V1",
    LOCAL_JUDGE_MODE: "CALIBRATED_SURROGATE",
    label: scalar(input.label, "candidate"),
    quality: scalar(input.quality, "UNSPECIFIED"),
    source_sha: scalar(input.source_sha, "NA"),
    formula: FORMULA,
    best_time_snapshot_us: best,
    cases: rows,
    parent_predicted_official_score: parentScore,
    candidate_predicted_official_score: candidateScore,
    delta_predicted_official: candidateScore - parentScore,
    online_eligible: input.quality === "QUALIFIED" && Boolean(input.correctness_clean) && candidateScore > ANCHOR_SCORE ? "REVIEW_REQUIRED" : "NO",
    note: "This command scores an exact supplied 15-case vector. It does not claim that a local single-shape probe predicts the vector."
  };
  process.stdout.write(`${JSON.stringify(output, null, 2)}\n`);
}

function generateCommand(args) {
  const root = path.resolve(args.root || process.cwd());
  const out = path.resolve(args.out || path.join(root, "研究/主代理/MAIN-2"));
  const extraRoots = (args.scanRoots || []).map((item) => path.resolve(item));
  const roots = [...new Set([root, ...discoverGitWorktreeRoots(root), ...extraRoots])];
  const scope = discoverMain2Scope(root);
  const evidence = collectEvidence(roots);
  const onlineMetaMap = new Map(evidence.onlineMeta.map((file) => [path.dirname(file), parseMetaFile(file, "online")]));
  const officialRecords = dedupeOfficial(evidence.onlineResults.map((file) => normalizeOfficial(file, onlineMetaMap)));
  const revisionRows = revisionInventory(root, evidence, officialRecords, scope.routeSet);
  const implementedRoutes = new Set(revisionRows.map((row) => row.ROUTE));
  const researchOnly = scope.routes.filter((route) => !implementedRoutes.has(route));
  const vectorRows = officialVectorRows();
  const datasetRows = calibrationDataset(root, officialRecords, revisionRows);
  const inventoryHeaders = [
    "ROUTE", "REVISION", "HYPOTHESIS_ID", "DIRECT_PARENT", "PARENT_SHA", "SOURCE_SHA",
    "EXACT_FUNCTION", "MECHANISM", "SINGLE_VARIABLE_CHANGE", "TARGET_SHAPES", "TARGET_DTYPES",
    "BUILD_STATUS", "CORRECTNESS_STATUS", "RUN1_PARENT_US", "RUN1_CANDIDATE_US", "RUN2_PARENT_US",
    "RUN2_CANDIDATE_US", "RUN3_PARENT_US", "RUN3_CANDIDATE_US", "PARENT_AVG_US", "CANDIDATE_AVG_US",
    "LATENCY_DELTA_PERCENT", "PARENT_THROUGHPUT", "CANDIDATE_THROUGHPUT", "THROUGHPUT_DELTA_PERCENT",
    "LOCAL_QUALITY", "LOCAL_VERDICT", "ONLINE_SUBMITTED", "OFFICIAL_SCORE", "ONLINE_PASS_COUNT",
    "ONLINE_SHA", "LOCAL_ONLINE_CLASSIFICATION", "EVIDENCE_PATH"
  ];
  const officialHeaders = ["ROUTE", "REVISION", "SOURCE_SHA", "SUBMISSION_ID", "PASS_COUNT", "OFFICIAL_SCORE"];
  for (let index = 1; index <= 15; index += 1) officialHeaders.push(`CASE${index}_TIME`);
  for (let index = 1; index <= 15; index += 1) officialHeaders.push(`CASE${index}_SCORE`);
  officialHeaders.push("FORMULA_SCORE", "CASE_DETAILS_MISSING", "STATUS", "RESULT_PATH");
  const officialRows = officialRecords.map((record) => {
    const row = {
      ROUTE: record.route,
      REVISION: record.revision,
      SOURCE_SHA: record.sourceSha,
      SUBMISSION_ID: record.submissionId,
      PASS_COUNT: record.passCount,
      OFFICIAL_SCORE: record.officialScore,
      FORMULA_SCORE: record.formulaScore,
      CASE_DETAILS_MISSING: record.caseDetailsMissing,
      STATUS: record.status,
      RESULT_PATH: record.path
    };
    for (let index = 1; index <= 15; index += 1) {
      row[`CASE${index}_TIME`] = record.times[index - 1] ?? "NA";
      row[`CASE${index}_SCORE`] = record.scores[index - 1] ?? "NA";
    }
    return row;
  });
  const datasetHeaders = [
    "SAMPLE_TYPE", "ROUTE", "REVISION", "LOCAL_DELTA_PERCENT", "LOCAL_QUALITY", "TARGET_SHAPES",
    "TARGET_DTYPES", "OFFICIAL_SCORE", "FORMULA_REPLAY_SCORE", "OFFICIAL_DELTA_VS_ANCHOR", "CLASSIFICATION",
    "CALIBRATION_ROLE", "EVIDENCE", "NOTE"
  ];
  writeText(path.join(out, "MAIN2-ALL-LOCAL-REVISIONS.tsv"), tsv(inventoryHeaders, revisionRows));
  writeText(path.join(out, "ALL-OFFICIAL-RESULTS.tsv"), tsv(officialHeaders, officialRows));
  writeText(path.join(out, "ONLINE-BEST-TIMES.tsv"), tsv(["CASE", "TESTCASE_ID", "BEST_TIME_US", "SOURCE", "RETRIEVED_SNAPSHOT", "STATUS"], vectorRows));
  writeText(path.join(out, "LOCAL-JUDGE-CALIBRATION-DATASET.tsv"), tsv(datasetHeaders, datasetRows));
  writeText(path.join(out, "MAIN2-REVISION-TREE.md"), makeTree(revisionRows, researchOnly, scope));
  writeText(path.join(out, "ONLINE-CASE-REPRODUCIBILITY.md"), [
    "# Online Case Reproducibility",
    "",
    "## Classification",
    "",
    "`CASE_REPRODUCIBILITY=PARTIAL` and `LOCAL_JUDGE_MODE=CALIBRATED_SURROGATE`.",
    "",
    "The public problem endpoint and ranking endpoint expose the 15 ordered testcase IDs and current `tbest` values. The public problem description exposes supported dtype and dimension ranges and examples, but does not expose the exact per-case shape, dtype, row count, hidden input values, or runner configuration needed for 1:1 local reproduction.",
    "",
    `Problem endpoint: ${PROBLEM_ENDPOINT}`,
    `Ranking endpoint: ${RANKING_ENDPOINT}`,
    "",
    "## Formula Confirmation",
    "",
    `The formula is present in \`工具/cannjudge.py\` and \`工具/cannjudge-submit.mjs\`, and is stated in the public problem description: ${FORMULA}`,
    "",
    "The current API vector is recorded in `ONLINE-BEST-TIMES.tsv`. Historical retained result payloads contain different best-time snapshots, so the vector is not treated as a timeless constant. Historical replay uses each submission's own recorded `best_time` when available.",
    "",
    "## Exact vs Surrogate Boundary",
    "",
    "Known exactly: testcase count, ordering, testcase IDs, formula, current public tbest snapshot, and historical result payload fields.",
    "Known only partially: shape/dtype/workload mapping and local runner equivalence.",
    "Unknown: hidden inputs and exact Official runner scheduling/configuration.",
    "",
    "No local tool in the repository can currently emit the exact 15 hidden cases. The Local Judge therefore refuses to turn a single-shape local delta into a numeric predicted Official score.",
    ""
  ].join("\n"));
  writeText(path.join(out, "LOCAL-JUDGE-V1-SPEC.md"), specReport());
  writeText(path.join(out, "LOCAL-JUDGE-VALIDATION.md"), validationReport(root, officialRecords, revisionRows));
  writeText(path.join(out, "LOCAL-JUDGE-R31B-V011-EXAMPLE.json"), JSON.stringify({
    LOCAL_JUDGE_VERSION: "LOCAL-JUDGE-V1",
    LOCAL_JUDGE_MODE: "CALIBRATED_SURROGATE",
    route: ANCHOR_ROUTE,
    revision: ANCHOR_REVISION,
    official_score: ANCHOR_SCORE,
    predicted_official_score: "UNAVAILABLE",
    quality: "ANCHOR_ONLY",
    fresh_npu_baseline: {
      run_count: 12,
      source: "研究/主代理/MAIN-2/R31B-V011-FRESH-LOCAL-BASELINE.tsv",
      composite_local_score: "NOT_FOUND",
      note: "Fresh Parent-only measurements are calibration anchors, not a cross-route Local Judge score."
    },
    online_eligible: "NO"
  }, null, 2));
  writeText(path.join(out, "LOCAL-JUDGE-ADDR-H3-EXAMPLE.json"), JSON.stringify({
    LOCAL_JUDGE_VERSION: "LOCAL-JUDGE-V1",
    LOCAL_JUDGE_MODE: "CALIBRATED_SURROGATE",
    route: "HOTLOOP-ADDR-HOIST-CHAMPION-X",
    revision: "V001",
    old_local_delta_percent: -24.19332,
    old_local_quality: "POOR",
    predicted_official_score: "UNAVAILABLE",
    actual_official_score: 42.72,
    official_pass_count: "15/15",
    case_details_missing: "YES",
    online_eligible: "NO",
    reason: "Exact 15-case local vector and retained per-case Official details are unavailable; single-shape Local evidence is rejected as an Online gate."
  }, null, 2));
  const summary = [
    "# Main-2 Local Route Census + Online Judge Alignment",
    "",
    `Generated: ${new Date().toISOString()}`,
    `Evidence roots: ${roots.join(", ")}`,
    "",
    "## Route Census",
    "",
    `- Main-2 route scope: ${scope.routes.length}`,
    `- Implemented revisions in scope: ${revisionRows.length}`,
    `- Research-only routes in scope: ${researchOnly.length}`,
    `- Official result files after evidence deduplication: ${officialRecords.length}`,
    "",
    "## Local Judge",
    "",
    "- Mode: CALIBRATED_SURROGATE",
    "- Exact 15 cases reproducible: NO; PARTIAL public metadata only",
    `- Current tbest vector: ${CURRENT_BEST_TIMES.length} cases from public API snapshot`,
    "- Formula replay is implemented; Local-to-Online prediction is blocked until exact case coverage exists.",
    "",
    "## ADDR H3",
    "",
    "- Old Local: -24.193320%, POOR, single-shape engineering signal",
    "- V1 predicted Online: UNAVAILABLE because exact 15-case local vector is missing",
    "- Actual Online: 42.72, 15/15 PASS",
    "- V1 gate: ONLINE_ELIGIBLE=NO",
    "- Per-case Official detail: MISSING; no case-level ADDR improvement/retreat is inferred.",
    "- Fresh V011 calibration: 12 Parent-only NPU runs retained; no composite Local score claimed.",
    "",
    "## Scope Safety",
    "",
    "No Candidate Kernel, shared ledger, route lifecycle, or Online submission was changed by this generator. New performance revisions remain frozen pending Planning review.",
    ""
  ];
  writeText(path.join(out, "MAIN2-LOCAL-JUDGE-RECONCILIATION.md"), summary.join("\n"));
  console.log(JSON.stringify({
    out,
    routes: scope.routes.length,
    implementedRevisions: revisionRows.length,
    researchOnlyRoutes: researchOnly.length,
    officialResults: officialRecords.length,
    localMetaFiles: evidence.localMeta.length,
    localResultFiles: evidence.localResults.length,
    scopeSources: scope.sources,
    mode: "CALIBRATED_SURROGATE",
    exactCases: false
  }, null, 2));
}

function parseArgs(argv) {
  const args = { command: argv[0] || "generate", scanRoots: [] };
  for (let index = 1; index < argv.length; index += 1) {
    const arg = argv[index];
    if (arg === "--root") args.root = argv[++index];
    else if (arg === "--out") args.out = argv[++index];
    else if (arg === "--scan-root") args.scanRoots.push(argv[++index]);
    else if (arg === "--input") args.input = argv[++index];
    else if (arg === "--best-times") args.bestTimes = argv[++index];
  }
  return args;
}

function main() {
  const args = parseArgs(process.argv.slice(2));
  if (args.command === "score") {
    if (!args.input) throw new Error("score requires --input <json>");
    scoreCommand(path.resolve(args.input), args.bestTimes ? path.resolve(args.bestTimes) : null);
    return;
  }
  if (args.command !== "generate") throw new Error(`Unknown command: ${args.command}`);
  generateCommand(args);
}

try {
  main();
} catch (error) {
  console.error(error?.stack || error);
  process.exitCode = 1;
}
