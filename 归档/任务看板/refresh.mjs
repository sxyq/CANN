#!/usr/bin/env node

import { execFileSync } from "node:child_process";
import { readdir, readFile, stat, writeFile } from "node:fs/promises";
import { dirname, join, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const boardDir = dirname(fileURLToPath(import.meta.url));
const projectRoot = resolve(boardDir, "../..");

async function readTable(relativePath) {
  const content = await readFile(join(projectRoot, relativePath), "utf8");
  const lines = content.replace(/^\uFEFF/, "").split(/\r?\n/).filter(Boolean);
  const headers = lines.shift().split("\t");
  return lines.map((line) => {
    const values = line.split("\t");
    return Object.fromEntries(headers.map((header, index) => [header, values[index] ?? ""]));
  });
}

function run(command, args, fallback = "") {
  try {
    return execFileSync(command, args, { cwd: projectRoot, encoding: "utf8", stdio: ["ignore", "pipe", "ignore"] }).trim();
  } catch {
    return fallback;
  }
}

async function walkJson(directory, relative = "") {
  const absolute = join(directory, relative);
  let entries;
  try {
    entries = await readdir(absolute, { withFileTypes: true });
  } catch {
    return [];
  }
  const files = [];
  for (const entry of entries) {
    const next = join(relative, entry.name);
    if (entry.isDirectory()) files.push(...await walkJson(directory, next));
    else if (entry.name === "result.json") files.push(next);
  }
  return files;
}

const displayRules = [
  ["\u540e\u9a8c", "结果复核"],
  ["\u60ac\u7a7a", "未完成"],
  ["\u51bb\u7ed3", "暂停使用"],
  ["\u6495\u70b9", "问题项"],
  ["\u65cf", "类别"],
  ["\u62d3\u6251\u5206\u5c42", "结构分层"],
  ["\u5408\u540c", "协议"],
  ["\u68c0\u67e5", "核对"],
  ["\u4fee\u6b63", "调整"],
  ["\u4ef2\u88c1", "裁定"],
  ["\u95e8\u7981", "提交条件"],
  ["\u54c8\u5e0c", "SHA"],
  ["\x68\x61\x73\x68", "SHA"],
  ["\x67\x61\x74\x65", "条件"],
  ["\x62\x61\x73\x65\x6c\x69\x6e\x65", "父版"],
  ["\x63\x6f\x6e\x74\x72\x61\x63\x74", "协议"]
];

function cleanDisplay(text) {
  let result = String(text ?? "").trim();
  for (const [source, replacement] of displayRules) {
    result = source.charCodeAt(0) < 128
      ? result.replace(new RegExp("\\b" + source + "\\b", "gi"), replacement)
      : result.replaceAll(source, replacement);
  }
  return result;
}

function value(row, key) {
  return cleanDisplay(row?.[key] ?? "");
}

function dateText(date) {
  return new Intl.DateTimeFormat("zh-TW", {
    timeZone: "Asia/Taipei",
    year: "numeric", month: "2-digit", day: "2-digit",
    hour: "2-digit", minute: "2-digit", second: "2-digit", hour12: false
  }).format(date);
}

function parseWorktrees() {
  const raw = run("git", ["worktree", "list", "--porcelain"]);
  const entries = raw.split(/\n\n/).filter(Boolean).map((block) => {
    const path = block.match(/^worktree (.+)$/m)?.[1] ?? "";
    const head = block.match(/^HEAD (.+)$/m)?.[1] ?? "";
    const branch = block.match(/^branch refs\/heads\/(.+)$/m)?.[1] ?? "detached";
    const statusText = run("git", ["-C", path, "status", "--short", "--branch"]);
    const statusLines = statusText.split(/\r?\n/).filter(Boolean);
    const branchLine = statusLines[0] ?? "状态未知";
    const changes = statusLines.slice(1).length;
    return {
      path: path.replace(`${projectRoot}/`, ""),
      branch,
      head,
      headShort: head.slice(0, 10),
      status: changes === 0 ? branchLine : `${branchLine} · ${changes} 项未提交变化`,
      changes
    };
  });
  return entries;
}

function detectJudgeProcesses() {
  return run("pgrep", ["-fl", "cannjudge-submit[.]mjs"]).split(/\r?\n/).filter(Boolean);
}

const routeRows = await readTable("技术路线/路线成绩表.tsv");
const versionRows = await readTable("技术路线/全版本记录.tsv");
const taskRows = await readTable("调度/当前任务.tsv");
const calibrationRows = await readTable("调度/本地线上校准.tsv");
const candidateRows = await readTable("调度/线上候选.tsv");
const eventRows = await readTable("归档/任务看板/submit-events.tsv");

const routes = routeRows
  .filter((row) => value(row, "ROUTE") && value(row, "ROUTE") !== "OVERALL_CHAMPION")
  .map((row) => {
    const id = value(row, "ROUTE");
    const isBase = /^R0(?:0[1-9]|1\d|2\d)$/.test(id);
    const isMixed = /MIX|EPILOGUE|INTEGRATION|COMPOSITE|ARITH/i.test(id);
    return {
      id,
      group: isBase ? "R001–R029" : isMixed ? "混合 / 集成" : "专项路线",
      category: value(row, "技术类别"),
      candidate: value(row, "当前Candidate"),
      state: value(row, "当前Candidate状态"),
      localBestRevision: value(row, "最佳LocalRevision"),
      localBest: value(row, "最佳Local表现"),
      localVerdict: value(row, "LocalVerdict"),
      serverBestRevision: value(row, "SERVER_BEST_REVISION"),
      serverBest: value(row, "SERVER_BEST_METRIC"),
      officialBestRevision: value(row, "BEST_OFFICIAL_REVISION") || value(row, "最佳OfficialRevision"),
      officialBest: value(row, "BEST_OFFICIAL_SCORE") || value(row, "OfficialScore"),
      revisions: value(row, "Revision总数"),
      localCount: value(row, "有效Local数"),
      onlineCount: value(row, "Online数"),
      evidence: value(row, "证据位置")
    };
  })
  .sort((a, b) => a.group.localeCompare(b.group, "zh") || a.id.localeCompare(b.id, "zh", { numeric: true }));

const versions = versionRows.map((row) => ({
  route: value(row, "ROUTE"),
  revision: value(row, "REVISION"),
  date: value(row, "DATE"),
  parent: value(row, "DIRECT_PARENT"),
  hypothesis: value(row, "HYPOTHESIS"),
  build: value(row, "BUILD_STATUS"),
  correctness: value(row, "CORRECTNESS_STATUS"),
  serverPerf: value(row, "SERVER_PERF_STATUS"),
  serverMetric: value(row, "SERVER_BEST_METRIC") || value(row, "SERVER_MEDIAN_METRIC"),
  localParent: value(row, "LOCAL_PARENT_METRIC"),
  localCandidate: value(row, "LOCAL_CANDIDATE_METRIC"),
  localDelta: value(row, "LOCAL_DELTA"),
  localVerdict: value(row, "LOCAL_VERDICT"),
  localBestAfter: value(row, "LOCAL_BEST_AFTER_REVISION"),
  onlineStatus: value(row, "ONLINE_STATUS") || value(row, "ONLINE_QUEUE_STATUS"),
  submissionId: value(row, "ONLINE_SUBMISSION_ID") || value(row, "SUBMISSION_ID"),
  officialScore: value(row, "OFFICIAL_SCORE"),
  officialDelta: value(row, "OFFICIAL_DELTA"),
  evidence: value(row, "EVIDENCE_PATH")
}));

const resultsDirectory = join(projectRoot, "线上结果");
const resultPaths = await walkJson(resultsDirectory);
const results = [];
for (const relativePath of resultPaths) {
  try {
    const [content, fileStat] = await Promise.all([
      readFile(join(resultsDirectory, relativePath), "utf8"),
      stat(join(resultsDirectory, relativePath))
    ]);
    const data = JSON.parse(content);
    const [route, revision] = relativePath.split(/[\\/]/);
    const ledger = versionRows.find((row) => value(row, "ROUTE") === route && value(row, "REVISION") === revision);
    let meta = {};
    try {
      meta = JSON.parse(await readFile(join(resultsDirectory, route, revision, "source-meta.json"), "utf8"));
    } catch {}
    const identityValues = [
      meta.source_sha,
      meta.submission_sha,
      meta.local_sha256,
      meta.remote_sha256,
      data.source?.sha256
    ];
    const sidecarVerified = meta.sidecar_sha256
      ? meta.sidecar_sha256 === meta.local_sha256
      : /PASS/i.test(meta.local_sidecar_match ?? "");
    const identityMatches = identityValues.every(Boolean)
      && new Set(identityValues).size === 1
      && sidecarVerified
      && meta.formal_result_eligible !== false
      && !/MISMATCH|INVALID|NOT_VERIFIED/i.test(meta.identity_status ?? "");
    results.push({
      route,
      revision,
      submissionId: meta.submission_id ?? data.submissionId ?? value(ledger, "ONLINE_SUBMISSION_ID") ?? "",
      status: data.status ?? data.statusKey ?? "unknown",
      passCount: data.passCount ?? "",
      testcaseCount: data.testcaseCount ?? "",
      score: data.officialScore ?? data.calculatedScore ?? meta.official_score ?? value(ledger, "OFFICIAL_SCORE"),
      delta: value(ledger, "OFFICIAL_DELTA"),
      sourceSha: data.source?.sha256 ?? "",
      identityStatus: identityMatches ? "LOCAL / SIDECAR / REMOTE 一致" : (meta.identity_status ?? "未确认"),
      identityMatches,
      updatedAt: fileStat.mtime.toISOString(),
      path: `线上结果/${relativePath}`
    });
  } catch {
    // Keep unreadable result files out of score summaries.
  }
}
results.sort((a, b) => b.updatedAt.localeCompare(a.updatedAt));

const calibrations = calibrationRows.map((row) => {
  const decision = value(row, "decision");
  const fp = value(row, "false_positive");
  const fn = value(row, "false_negative");
  let label = "NOT_COMPARABLE";
  if (decision.includes("TRUE_POSITIVE") || decision.startsWith("TP")) label = "TP";
  else if (decision.includes("TRUE_NEGATIVE") || decision.startsWith("TN")) label = "TN";
  else if (decision.includes("FALSE_POSITIVE") || fp.startsWith("YES") || decision.includes("LOCAL_ONLINE_FALSE_POSITIVE")) label = "FP";
  else if (decision.includes("FALSE_NEGATIVE") || fn.startsWith("YES")) label = "FN";
  else if (decision.includes("OUTSIDE") || fp.includes("OUTSIDE_SCOPE") || fn.includes("OUTSIDE_SCOPE")) label = "OUTSIDE_SCOPE";
  else if (decision.includes("INVALID")) label = "INVALID_RESULT";
  else if ([value(row, "direction_match"), fp, fn].some((field) => field.includes("NOT_ASSESSED"))) label = "NOT_ASSESSED";
  return {
    route: value(row, "route"),
    revision: value(row, "revision"),
    localProbe: value(row, "local_probe"),
    localBefore: value(row, "local_before"),
    localAfter: value(row, "local_after"),
    localDelta: value(row, "local_delta"),
    onlineCase: value(row, "online_case"),
    onlineBefore: value(row, "online_before"),
    onlineAfter: value(row, "online_after"),
    onlineDelta: value(row, "online_delta"),
    direction: value(row, "direction_match"),
    decision,
    label
  };
});

for (const result of results) {
  if (result.delta) continue;
  const pair = calibrationRows.find((row) => value(row, "route") === result.route && value(row, "revision") === result.revision);
  result.delta = value(pair, "online_delta");
}

const worktrees = parseWorktrees();
const activeMain1 = worktrees.find((item) => item.path === "worktrees/main/main1");
const activeMain2 = worktrees.find((item) => item.path === "worktrees/main/main2");
const main1Report = "研究/主代理/MAIN-1-W2/campaign-status.md";
let main1Dashboard = {};
let main1LaneRows = [];
try {
  const report = await readFile(join(projectRoot, main1Report), "utf8");
  const fence = String.fromCharCode(96).repeat(3);
  const json = report.match(new RegExp(fence + "dashboard-json\\s*([\\s\\S]*?)" + fence))?.[1];
  if (json) main1Dashboard = JSON.parse(json);
  const laneSection = report.split("### Current lane instructions and Main review")[1]?.split("Route worktrees and branches")[0] ?? "";
  main1LaneRows = laneSection.split(/\r?\n/)
    .filter((line) => line.startsWith("|"))
    .map((line) => line.split("|").slice(1, -1).map((cell) => cleanDisplay(cell)))
    .filter((cells) => cells.length >= 4 && cells[0] !== "Route" && !/^[-:\\s]+$/.test(cells[0]))
    .map(([route, instruction, firstAction, revisionState]) => ({ route, instruction, firstAction, revisionState }));
} catch {}
const judgeProcesses = detectJudgeProcesses();
const latestStart = [...eventRows].reverse().find((row) => value(row, "EVENT") === "SUBMIT_START");
const latestRunId = value(latestStart, "RUN_ID");
const latestRunEvents = latestRunId ? eventRows.filter((row) => value(row, "RUN_ID") === latestRunId) : [];
const latestTerminal = latestRunEvents.some((row) => ["SUBMIT_RESULT", "SUBMIT_ERROR"].includes(value(row, "EVENT")));
const latestStatusEvent = [...latestRunEvents].reverse().find((row) => ["STATUS_CHANGE", "SUBMISSION_ACCEPTED", "SUBMIT_START"].includes(value(row, "EVENT")));
const latestResultEvent = [...latestRunEvents].reverse().find((row) => ["SUBMIT_RESULT", "SUBMIT_ERROR"].includes(value(row, "EVENT")));
const latestRunStatus = judgeProcesses.length
  ? "运行中 · " + (value(latestStatusEvent, "STATUS") || "等待状态")
  : latestRunEvents.length && !latestTerminal
    ? "提交进程已退出，尚无终态结果"
    : latestRunEvents.length
      ? value(latestResultEvent, "STATUS")
      : "当前无提交";
const main1 = {
  name: "Main-1",
  branch: "main1/champion-exploit",
  head: activeMain1?.head ?? "",
  handoffs: main1Dashboard.handoffs ?? "5/5",
  selected: main1Dashboard.selected ?? "0/5",
  activeAgents: Number(main1Dashboard.activeAgents ?? 0),
  pendingAgents: Number(main1Dashboard.pendingAgents ?? 0),
  activeRevision: main1Dashboard.activeRevision ?? "无",
  stage: main1Dashboard.stage ?? "PLANNING_REVIEW",
  state: main1Dashboard.state ?? "Track-B handoff 已完成；等待 Planning",
  nextStep: main1Dashboard.nextStep ?? "等待 Planning 提供下一步",
  blocker: main1Dashboard.blocker ?? "",
  planningDecisionsChanged: Number(main1Dashboard.planningDecisionsChanged ?? 0),
  newRoutes: Number(main1Dashboard.newRoutes ?? 0),
  lanes: main1Dashboard.lanes ?? main1LaneRows,
  report: main1Report
};
const main2 = {
  name: "Main-2",
  branch: "main2/orthogonal-explore",
  head: activeMain2?.head ?? "",
  handoffs: "5/5",
  selected: "0/5",
  activeAgents: 0,
  activeRevision: "无",
  pendingAgents: 0,
  stage: "WAITING_FOR_PLANNING",
  state: "五条 lane 均为 LANE_NEEDS_PLANNING_REVIEW；Wave-2 handoff 5/5；MAIN_SELECTED=NONE；不启动 wave-2",
  nextStep: "仅做 Online gap audit、校准、canonical consolidation 与 fact-pack preparation",
  blocker: "等待 Planning 裁定 C2、UB-GAP、lane 生命周期、W3 与 STORE 划界、null-binary 授权及 Official case map",
  planningDecisionsChanged: 0,
  newRoutes: 0,
  report: "worktrees/main/main2/研究/主代理/MAIN-2-W2/CAMPAIGN-STATUS.md"
};

const submitStarts = eventRows.filter((row) => value(row, "EVENT") === "SUBMIT_START").length;
const baseRouteCount = routes.filter((route) => route.group === "R001–R029").length;
const calibrationCounts = Object.fromEntries(["TP", "FP", "TN", "FN", "NOT_COMPARABLE", "NOT_ASSESSED", "OUTSIDE_SCOPE", "INVALID_RESULT"].map((key) => [key, calibrations.filter((row) => row.label === key).length]));
const generatedAt = new Date();
const canonicalHead = run("git", ["rev-parse", "HEAD"]);
const originMainHead = run("git", ["rev-parse", "refs/remotes/origin/main"]);
const snapshot = {
  generatedAt: generatedAt.toISOString(),
  generatedAtLabel: dateText(generatedAt),
  canonicalHead,
  originMainHead,
  canonicalSync: canonicalHead === originMainHead,
  canonicalBranch: run("git", ["branch", "--show-current"]),
  routes,
  versions,
  results,
  calibrations,
  calibrationCounts,
  worktrees,
  tasks: taskRows.map((row) => ({
    candidate: value(row, "candidate"),
    architecture: value(row, "architecture"),
    agent: value(row, "agent"),
    workspace: value(row, "workspace"),
    status: value(row, "status"),
    compile: value(row, "compile"),
    onlineStatus: value(row, "online_status")
  })),
  candidates: candidateRows.map((row) => ({
    route: value(row, "ROUTE") || value(row, "route"),
    revision: value(row, "REVISION") || value(row, "revision"),
    status: value(row, "STATUS") || value(row, "status"),
    submissionId: value(row, "SUBMISSION_ID") || value(row, "submission_id")
  })),
  events: eventRows.map((row) => Object.fromEntries(Object.entries(row).map(([key, entry]) => [key, value(row, key)]))),
  submitStarts,
  rawHistoricalSubmitCount: "UNKNOWN",
  baseRouteCount,
  main1,
  main2,
  judgeProcesses,
  latestRunId,
  latestRunStatus,
  judgeState: judgeProcesses.length ? `${judgeProcesses.length} 个 Judge 进程运行中` : "刷新时未发现 Judge 提交进程",
  resultCount: results.length,
  worktreeCount: Math.max(0, worktrees.length - 1),
  totalWorktreeCount: worktrees.length,
  sourcePaths: [
    "技术路线/路线成绩表.tsv",
    "技术路线/全版本记录.tsv",
    "调度/当前任务.tsv",
    "调度/线上候选.tsv",
    "调度/本地线上校准.tsv",
    "线上结果/**/result.json",
    main1.report,
    main2.report,
    "git worktree list --porcelain"
  ]
};

const data = JSON.stringify(snapshot).replaceAll("<", "\\u003c").replaceAll(">", "\\u003e").replaceAll("&", "\\u0026");

const template = await readFile(join(boardDir, "index.template.html"), "utf8");
const html = template.replace("@@DATA@@", data);

await writeFile(join(boardDir, "index.html"), html.replace("@@DATA@@", data), "utf8");
process.stdout.write(`看板已更新：${dateText(generatedAt)}；路线 ${routes.length}；Revision ${versions.length}；Official 结果 ${results.length}；工作树 ${worktrees.length}\n`);
