#!/usr/bin/env node

import { spawnSync } from "node:child_process";
import { createHash } from "node:crypto";
import { existsSync, readFileSync } from "node:fs";
import { mkdir, readFile, writeFile } from "node:fs/promises";
import { createInterface } from "node:readline/promises";
import { stdin as input, stdout as output } from "node:process";
import { resolve } from "node:path";
import { chromium } from "playwright";

const PROJECT_ROOT = resolve(import.meta.dirname, "..");
const SUBMIT_URL = "https://cannjudge.cn/public/op_challenge_xinan_prelim/addrmsnormbias/submit";
const PROBLEM_TOKEN = "addrmsnormbias";
const DEFAULT_PROFILE = resolve(
  PROJECT_ROOT,
  "管理/提交队列/runtime/cannjudge-browser-profile"
);
const DEFAULT_INTERVAL_MS = 2000;
const DEFAULT_MAX_WAIT_MS = 30 * 60 * 1000;
const REQUEST_TIMEOUT_MS = 15000;

function printHelp() {
  console.log(`Usage:
  npm run cannjudge:login
  npm run cannjudge:submit -- --yes --source <file>

Commands:
  login       Open the project browser profile for the first manual login.
  submit      Submit through the browser session and poll the result.

  Formal submit policy (enforced):
  - --source <file> is required for submit (stdin --source - and clipboard/terminal paste are disabled).
  - Print source path / line count / byte count / SHA256 / first and last non-empty lines.
  - If <source-dir>/submission.sha256 exists, local SHA must match or submit stops.
  - After submit, recompute kernel SHA from Judge files[].content and require LOCAL_SHA == REMOTE_SHA;
    otherwise mark INPUT_IDENTITY_MISMATCH and refuse to treat the result as a formal experiment result.

  Options:
  --yes                   Confirm one external submission. Required for submit.
  --source <file>         Required source file path for submit.
  --profile <dir>         Persistent browser profile directory.
  --cdp <url>             Attach to a running local browser CDP endpoint.
  --headed                Show the browser during submit.
  --interval <ms>         Poll interval. Default: 2000.
  --max-wait <ms>         Maximum wait. Default: 1800000.
  --out <file>            Write the final JSON result to a file.
  --dry-run               Print source identity without launching a browser.
  --json                  Print machine-readable output where possible.
  --help                  Show this help.

Authentication:
  The first run uses a project-local persistent browser profile. The browser
  keeps its own session state; this program never reads or exports cookies.
  A fingerprint browser can be used with --cdp if it exposes a CDP endpoint.
`);
}

function parseArgs(argv) {
  const args = [...argv];
  let command = "submit";
  if (args[0] && !args[0].startsWith("-")) command = args.shift();
  const options = {
    command,
    source: "",
    profile: DEFAULT_PROFILE,
    cdp: process.env.CANNJUDGE_CDP_URL || "",
    yes: false,
    headed: false,
    interval: DEFAULT_INTERVAL_MS,
    maxWait: DEFAULT_MAX_WAIT_MS,
    out: "",
    dryRun: false,
    json: false
  };

  for (let index = 0; index < args.length; index += 1) {
    const arg = args[index];
    if (arg === "--help" || arg === "-h") {
      options.help = true;
    } else if (arg === "--yes") {
      options.yes = true;
    } else if (arg === "--headed") {
      options.headed = true;
    } else if (arg === "--dry-run") {
      options.dryRun = true;
    } else if (arg === "--json") {
      options.json = true;
    } else if (["--source", "--profile", "--cdp", "--interval", "--max-wait", "--out"].includes(arg)) {
      const value = args[++index];
      if (!value || value.startsWith("--")) throw new Error(`${arg} requires a value`);
      if (arg === "--source") options.source = value === "-" ? "-" : resolve(PROJECT_ROOT, value);
      if (arg === "--profile") options.profile = resolve(PROJECT_ROOT, value);
      if (arg === "--cdp") options.cdp = value;
      if (arg === "--interval") options.interval = positiveNumber(value, arg);
      if (arg === "--max-wait") options.maxWait = positiveNumber(value, arg);
      if (arg === "--out") options.out = resolve(PROJECT_ROOT, value);
    } else {
      throw new Error(`Unknown option: ${arg}`);
    }
  }
  return options;
}

function positiveNumber(value, flag) {
  const number = Number(value);
  if (!Number.isFinite(number) || number <= 0) throw new Error(`${flag} must be a positive number`);
  return number;
}

function lineCount(text) {
  return text.length === 0 ? 0 : (text.match(/\n/g) || []).length + (text.endsWith("\n") ? 0 : 1);
}

function sourceFromBytes(sourcePath, bytes) {
  const decoder = new TextDecoder("utf-8", { fatal: true });
  let content;
  try {
    content = decoder.decode(bytes);
  } catch {
    throw new Error("Source is not valid UTF-8");
  }
  const encoded = new TextEncoder().encode(content);
  if (encoded.byteLength !== bytes.byteLength || encoded.some((value, index) => value !== bytes[index])) {
    throw new Error("Source changes during UTF-8 decoding; refusing to submit");
  }
  if (!content.includes("run_kernel")) throw new Error("Source does not contain run_kernel");
  return {
    path: sourcePath,
    content,
    bytes: Buffer.from(encoded),
    byteCount: encoded.byteLength,
    lineCount: lineCount(content),
    sha256: createHash("sha256").update(encoded).digest("hex")
  };
}

async function readSource(sourcePath) {
  let bytes;
  if (sourcePath === "-") {
    const chunks = [];
    for await (const chunk of input) chunks.push(Buffer.isBuffer(chunk) ? chunk : Buffer.from(chunk));
    bytes = Buffer.concat(chunks);
  } else {
    if (!existsSync(sourcePath)) throw new Error(`Source file does not exist: ${sourcePath}`);
    bytes = await readFile(sourcePath);
  }
  return sourceFromBytes(sourcePath === "-" ? "<stdin>" : sourcePath, bytes);
}

function readClipboardBytes() {
  if (process.platform !== "darwin") {
    throw new Error("Interactive clipboard input is currently supported on macOS; use --source <file> elsewhere");
  }
  const result = spawnSync("pbpaste", [], { encoding: null });
  if (result.error || result.status !== 0) {
    throw new Error(`Could not read the macOS clipboard${result.error ? `: ${result.error.message}` : ""}`);
  }
  return Buffer.from(result.stdout || []);
}

async function readInteractiveSource() {
  if (!input.isTTY || !output.isTTY) {
    throw new Error("Interactive paste requires a terminal; use --source <file> or --source -");
  }

  const pasteStart = Buffer.from("\x1b[200~");
  const pasteEnd = Buffer.from("\x1b[201~");
  const ctrlV = Buffer.from([0x16]);
  const hasEnter = (bytes) => bytes.includes(0x0a) || bytes.includes(0x0d);

  output.write("请粘贴完整代码（macOS Terminal 通常使用 Command+V；终端映射为 Ctrl+V 时也可以），粘贴完成后按 Enter 开始提交。\n");
  input.setRawMode(true);
  input.resume();
  output.write("\x1b[?2004h");

  return new Promise((resolveSource, rejectSource) => {
    let state = "waiting";
    let pending = Buffer.alloc(0);
    let pasted = Buffer.alloc(0);
    let finished = false;

    const cleanup = () => {
      input.off("data", onData);
      if (input.isTTY) input.setRawMode(false);
      input.pause();
      output.write("\x1b[?2004l");
    };
    const fail = (error) => {
      if (finished) return;
      finished = true;
      cleanup();
      output.write("\n");
      rejectSource(error);
    };
    const finish = (bytes) => {
      if (finished) return;
      finished = true;
      cleanup();
      output.write("\n");
      try {
        resolveSource(sourceFromBytes("<terminal>", bytes));
      } catch (error) {
        rejectSource(error);
      }
    };
    const onData = (chunk) => {
      pending = Buffer.concat([pending, Buffer.isBuffer(chunk) ? chunk : Buffer.from(chunk)]);
      if (pending.includes(0x03)) {
        fail(new Error("Interactive input cancelled"));
        return;
      }

      while (!finished) {
        if (state === "waiting") {
          const startIndex = pending.indexOf(pasteStart);
          const ctrlVIndex = pending.indexOf(ctrlV);
          if (startIndex >= 0 && (ctrlVIndex < 0 || startIndex < ctrlVIndex)) {
            pending = pending.subarray(startIndex + pasteStart.length);
            state = "pasting";
            output.write("已开始接收粘贴内容...\n");
            continue;
          }
          if (ctrlVIndex >= 0) {
            pending = pending.subarray(ctrlVIndex + ctrlV.length);
            try {
              pasted = readClipboardBytes();
            } catch (error) {
              fail(error);
              return;
            }
            if (pasted.length === 0) {
              fail(new Error("Clipboard is empty; paste the source code first"));
              return;
            }
            state = "ready";
            output.write("已读取剪贴板代码，请按 Enter 开始提交。\n");
            continue;
          }
          if (hasEnter(pending)) {
            fail(new Error("尚未收到代码，请先粘贴完整代码，再按 Enter"));
            return;
          }
          const keep = pasteStart.length - 1;
          if (pending.length > keep) pending = pending.subarray(pending.length - keep);
          break;
        }

        if (state === "pasting") {
          const endIndex = pending.indexOf(pasteEnd);
          if (endIndex >= 0) {
            pasted = Buffer.concat([pasted, pending.subarray(0, endIndex)]);
            pending = pending.subarray(endIndex + pasteEnd.length);
            state = "ready";
            output.write("已接收粘贴内容，请按 Enter 开始提交。\n");
            continue;
          }
          const keep = pasteEnd.length - 1;
          if (pending.length > keep) {
            pasted = Buffer.concat([pasted, pending.subarray(0, pending.length - keep)]);
            pending = pending.subarray(pending.length - keep);
          }
          break;
        }

        if (state === "ready") {
          if (hasEnter(pending)) finish(pasted);
          break;
        }
      }
    };

    input.on("data", onData);
  });
}

function sourceSummary(source) {
  const lines = source.content.split(/\r\n|\n|\r/);
  let firstNonEmpty = "";
  let lastNonEmpty = "";
  for (const line of lines) {
    if (line.trim()) {
      if (!firstNonEmpty) firstNonEmpty = line;
      lastNonEmpty = line;
    }
  }
  return {
    source: source.path,
    lines: source.lineCount,
    bytes: source.byteCount,
    sha256: source.sha256,
    firstNonEmptyLine: firstNonEmpty,
    lastNonEmptyLine: lastNonEmpty
  };
}

function readSidecarSha256(sourcePath) {
  if (!sourcePath || sourcePath === "<stdin>" || sourcePath === "<terminal>") return null;
  const sidecar = resolve(sourcePath, "..", "submission.sha256");
  if (!existsSync(sidecar)) return null;
  try {
    const raw = readFileSync(sidecar, "utf8");
    const match = raw.trim().split(/\s+/)[0];
    if (!/^[0-9a-f]{64}$/i.test(match || "")) {
      throw new Error(`Invalid submission.sha256 content: ${sidecar}`);
    }
    return { path: sidecar, sha256: match.toLowerCase() };
  } catch (error) {
    if (error?.code === "ENOENT") return null;
    throw error;
  }
}

function verifyLocalIdentity(source, preview) {
  const sidecar = readSidecarSha256(source.path);
  const localSha = String(preview.sha256 || "").toLowerCase();
  const checks = {
    sourcePath: preview.source,
    lineCount: preview.lines,
    byteCount: preview.bytes,
    localSha256: localSha,
    firstNonEmptyLine: preview.firstNonEmptyLine,
    lastNonEmptyLine: preview.lastNonEmptyLine,
    sidecarPath: sidecar?.path || null,
    sidecarSha256: sidecar?.sha256 || null,
    sidecarMatch: sidecar ? sidecar.sha256 === localSha : null
  };
  if (sidecar && sidecar.sha256 !== localSha) {
    const error = new Error(
      `SOURCE_SHA_MISMATCH: local ${localSha} != sidecar ${sidecar.sha256} (${sidecar.path})`
    );
    error.identity = checks;
    throw error;
  }
  return checks;
}

function extractRemoteKernelSha256(submission) {
  const files = Array.isArray(submission?.files) ? submission.files : [];
  const kernel = files.find((file) => file?.path === "kernel.asc") || files.find((file) => file?.path === "kernel.txt");
  if (!kernel || typeof kernel.content !== "string") {
    return { remoteSha256: null, remoteBytes: null, remoteLines: null, reason: "REMOTE_KERNEL_MISSING" };
  }
  const bytes = Buffer.from(kernel.content, "utf8");
  return {
    remoteSha256: createHash("sha256").update(bytes).digest("hex"),
    remoteBytes: bytes.byteLength,
    remoteLines: lineCount(kernel.content),
    reason: null
  };
}

function statusKey(status) {
  const value = String(status || "").toLowerCase();
  if (value.includes("pass") || value.includes("accepted") || value.includes("success") ||
    value.includes("complete") || value === "ac") return "pass";
  if (value.includes("skip")) return "skipped";
  if (value.includes("wrong") || value.includes("compile error") || value.includes("runtime") ||
    value.includes("error") || value.includes("fail") || value.includes("time limit")) return "fail";
  return "waiting";
}

function calculateCaseScore(time, best) {
  const current = Number(time);
  const reference = Number(best);
  if (!(current > 0) || !(reference > 0)) return null;
  return 100 / (1 + Math.log(current / reference) / Math.log(1.5));
}

function isTerminal(status) {
  const value = String(status || "").toLowerCase().trim();
  if (!value) return false;
  if (["wait", "running", "pending", "queue", "judging", "compiling", "preparing", "init"]
    .some((hint) => value.includes(hint))) return false;
  return statusKey(value) !== "waiting" ||
    ["finish", "done", "reject", "invalid", "cancel"].some((hint) => value.includes(hint));
}

function calculateScore(rows) {
  if (!Array.isArray(rows) || rows.length === 0) return null;
  const scores = [];
  for (const row of rows) {
    const score = calculateCaseScore(row?.time, row?.best_time);
    if (score == null) return null;
    scores.push(score);
  }
  return scores.reduce((sum, value) => sum + value, 0) / scores.length;
}

function testcaseDetails(rows) {
  return rows.map((row, index) => {
    const precisionRatio = row?.precision_ratio == null ? null : Number(row.precision_ratio);
    return {
      index: index + 1,
      testcaseId: row?.testcase_id || "",
      status: row?.testcase_status || "",
      statusKey: statusKey(row?.testcase_status),
      errorPercent: Number.isFinite(precisionRatio) ? (1 - precisionRatio) * 100 : null,
      timeUs: Number.isFinite(Number(row?.time)) ? Number(row.time) : null,
      bestTimeUs: Number.isFinite(Number(row?.best_time)) ? Number(row.best_time) : null,
      score: calculateCaseScore(row?.time, row?.best_time)
    };
  });
}

function renderResult(submission, submissionId, officialScore, source, identity = {}) {
  const rows = Array.isArray(submission?.result) ? submission.result : [];
  const details = testcaseDetails(rows);
  const passCount = details.filter((row) => row.statusKey === "pass").length;
  const errors = rows
    .map((row) => row?.precision_ratio == null ? null : (1 - Number(row.precision_ratio)) * 100)
    .filter((value) => Number.isFinite(value));
  const remote = extractRemoteKernelSha256(submission);
  const localSha = String(identity.localSha256 || sourceSummary(source).sha256 || "").toLowerCase();
  const identityOk = Boolean(localSha && remote.remoteSha256 && localSha === remote.remoteSha256);
  const formalResultEligible = identityOk && (identity.sidecarMatch !== false);
  return {
    submissionId,
    status: submission?.status || "",
    statusKey: statusKey(submission?.status),
    passCount,
    testcaseCount: rows.length,
    maxOutputErrorPercent: errors.length ? Math.max(...errors) : null,
    officialScore,
    calculatedScore: calculateScore(rows),
    scoreFormula: "s_i = 100 / (1 + log_1.5(time_i / best_time_i)); total = mean(s_i)",
    source: sourceSummary(source),
    identity: {
      ...identity,
      localSha256: localSha || null,
      remoteSha256: remote.remoteSha256,
      remoteBytes: remote.remoteBytes,
      remoteLines: remote.remoteLines,
      localEqualsRemote: identityOk ? "true" : "false",
      identityStatus: identityOk ? "LOCAL_SHA_EQ_REMOTE_SHA" : "INPUT_IDENTITY_MISMATCH",
      formalResultEligible
    },
    testcases: details,
    result: rows
  };
}

function formatNumber(value, digits = 2) {
  return Number.isFinite(Number(value)) ? Number(value).toFixed(digits) : "-";
}

function printHumanResult(result) {
  console.log(`Status: ${result.status}`);
  console.log(`Submission ID: ${result.submissionId}`);
  console.log(`Passed: ${result.passCount}/${result.testcaseCount}`);
  console.log(`Maximum output error: ${formatNumber(result.maxOutputErrorPercent, 3)}%`);
  console.log(`Official score: ${result.officialScore ?? "not listed"}`);
  console.log(`Local formula score: ${result.calculatedScore == null ? "-" : result.calculatedScore.toFixed(3)}`);
  console.log(`Formula: ${result.scoreFormula}`);
  console.log("Point details (ordered by testcase number; time unit: us):");
  console.log("No. | Status | Error (%) | Time (us) | Best (us) | Point score");
  console.log("----+--------+-----------+-----------+-----------+-----------");
  for (const testcase of result.testcases) {
    console.log([
      String(testcase.index).padStart(3),
      String(testcase.status || "-").padEnd(6),
      formatNumber(testcase.errorPercent, 3).padStart(9),
      formatNumber(testcase.timeUs, 2).padStart(9),
      formatNumber(testcase.bestTimeUs, 2).padStart(9),
      formatNumber(testcase.score, 3).padStart(10)
    ].join(" | "));
  }
}

function apiError(status, body) {
  const message = body?.message || body?.msg || body?.error || body?.detail || `HTTP ${status}`;
  const error = new Error(`${status}: ${message}`);
  error.status = status;
  error.body = body;
  return error;
}

async function pageApi(page, path, { method = "GET", data } = {}) {
  const response = await page.evaluate(async ({ path, method, data, timeoutMs }) => {
    const controller = new AbortController();
    const timer = setTimeout(() => controller.abort(), timeoutMs);
    try {
      const result = await fetch(path, {
        method,
        credentials: "same-origin",
        headers: { Accept: "application/json", ...(data === undefined ? {} : { "Content-Type": "application/json" }) },
        body: data === undefined ? undefined : JSON.stringify(data),
        signal: controller.signal
      });
      const text = await result.text();
      let body = {};
      try { body = text ? JSON.parse(text) : {}; } catch { body = { message: text }; }
      return { ok: result.ok, status: result.status, body };
    } finally {
      clearTimeout(timer);
    }
  }, { path, method, data, timeoutMs: REQUEST_TIMEOUT_MS });
  if (!response.ok) throw apiError(response.status, response.body);
  return response.body;
}

async function getStoredUser(page) {
  const raw = await page.evaluate(() => localStorage.getItem("cannjudge_user"));
  if (!raw) throw new Error("No CANNJudge user session in the browser profile; run npm run cannjudge:login first");
  let user;
  try { user = JSON.parse(raw); } catch { throw new Error("CANNJudge browser user state is invalid"); }
  if (!user || !user.ID) throw new Error("CANNJudge browser user state has no user ID");
  if (!user._id) {
    const latest = await pageApi(page, `/api/users/me/${encodeURIComponent(user.ID)}`);
    user = { ...user, ...latest };
  }
  if (!user._id) throw new Error("CANNJudge user object has no object id");
  return user;
}

function findBrowserExecutable() {
  const candidates = [
    process.env.CANNJUDGE_BROWSER_EXECUTABLE,
    "/Applications/Microsoft Edge.app/Contents/MacOS/Microsoft Edge",
    "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome",
    "/Applications/Chromium.app/Contents/MacOS/Chromium"
  ].filter(Boolean);
  return candidates.find((candidate) => existsSync(candidate)) || "";
}

async function openBrowser(options, { headed = false } = {}) {
  if (options.cdp) {
    const browser = await chromium.connectOverCDP(options.cdp);
    const context = browser.contexts()[0];
    if (!context) throw new Error("CDP browser has no browser context");
    const page = await context.newPage();
    return {
      page,
      close: async () => {
        await page.close().catch(() => {});
        if (typeof browser.disconnect === "function") browser.disconnect();
      }
    };
  }

  await mkdir(options.profile, { recursive: true });
  const executablePath = findBrowserExecutable();
  const launchOptions = {
    headless: !headed,
    viewport: { width: 1440, height: 1000 },
    acceptDownloads: false
  };
  if (executablePath) launchOptions.executablePath = executablePath;
  const context = await chromium.launchPersistentContext(options.profile, launchOptions);
  const page = context.pages()[0] || await context.newPage();
  return { page, close: () => context.close() };
}

async function openSubmitPage(page) {
  await page.goto(SUBMIT_URL, { waitUntil: "domcontentloaded", timeout: 30000 });
  await page.waitForTimeout(500);
}

async function login(options) {
  const session = await openBrowser(options, { headed: true });
  try {
    await openSubmitPage(session.page);
    console.log("Browser profile is open. Complete CANNJudge login in the browser, then press Enter here.");
    const readline = createInterface({ input, output });
    await readline.question("");
    readline.close();
    const user = await getStoredUser(session.page);
    console.log(`Login state detected for user ID ${user.ID}. Browser session remains in its project profile.`);
  } finally {
    await session.close();
  }
}

async function fetchOfficialScore(page, problemId, submissionId) {
  if (!problemId) return null;
  let pageNumber = 1;
  for (let count = 0; count < 10; count += 1) {
    const ranking = await pageApi(page, `/api/problems/${encodeURIComponent(problemId)}/ranking?page=${pageNumber}&size=100`);
    const rows = Array.isArray(ranking?.rows) ? ranking.rows : [];
    const match = rows.find((row) => String(row?.submission_id) === String(submissionId));
    if (match?.score != null) {
      const score = Number(match.score);
      return Number.isFinite(score) ? score : null;
    }
    const pages = Math.max(1, Number(ranking?.pages) || 1);
    if (pageNumber >= pages || rows.length === 0) return null;
    pageNumber += 1;
  }
  return null;
}

async function poll(page, problemId, submissionId, source, options) {
  const started = Date.now();
  let lastStatus = "";
  while (Date.now() - started <= options.maxWait) {
    const submission = await pageApi(page, `/api/submissions/${encodeURIComponent(submissionId)}`);
    const status = String(submission?.status || "");
    if (status !== lastStatus) {
      console.log(`[CANNJudge] status=${status || "waiting"}`);
      lastStatus = status;
    }
    if (isTerminal(status)) {
      let officialScore = null;
      if (statusKey(status) === "pass") {
        for (let attempt = 0; attempt < 5 && officialScore == null; attempt += 1) {
          officialScore = await fetchOfficialScore(page, problemId, submissionId);
          if (officialScore == null && attempt < 4) await delay(options.interval);
        }
      }
      return renderResult(submission, submissionId, officialScore, source, options.identity || {});
    }
    await delay(options.interval);
  }
  throw new Error(`Timed out while polling submission ${submissionId}`);
}

function delay(ms) {
  return new Promise((resolveDelay) => setTimeout(resolveDelay, ms));
}

async function submit(options) {
  if (!options.yes && !options.dryRun) throw new Error("External submission is disabled without --yes");
  if (!options.source) {
    throw new Error(
      "FORMAL_SUBMIT_REQUIRES_SOURCE: pass --source <file>. Clipboard/terminal paste is disabled for formal submissions."
    );
  }
  if (options.source === "-") {
    throw new Error(
      "FORMAL_SUBMIT_REQUIRES_FILE_SOURCE: --source - (stdin) is disabled; pass a local file path."
    );
  }
  const source = await readSource(options.source);
  const preview = sourceSummary(source);
  const identity = verifyLocalIdentity(source, preview);
  options.identity = identity;
  if (options.json) {
    console.log(JSON.stringify({ phase: "preflight", ...preview, identity }, null, 2));
  } else {
    console.log("=== preflight source identity ===");
    console.log(`source path        : ${preview.source}`);
    console.log(`line count         : ${preview.lines}`);
    console.log(`byte count         : ${preview.bytes}`);
    console.log(`SHA256             : ${preview.sha256}`);
    console.log(`first non-empty    : ${preview.firstNonEmptyLine}`);
    console.log(`last non-empty     : ${preview.lastNonEmptyLine}`);
    console.log(`submission.sha256  : ${identity.sidecarPath ? identity.sidecarSha256 : "(absent)"}`);
    console.log(`sidecar match      : ${identity.sidecarMatch == null ? "N/A" : identity.sidecarMatch ? "OK" : "MISMATCH"}`);
  }
  if (options.dryRun) return;

  const session = await openBrowser(options, { headed: options.headed });
  try {
    await openSubmitPage(session.page);
    const user = await getStoredUser(session.page);
    const problem = await pageApi(session.page, `/api/problems/name/${PROBLEM_TOKEN}`);
    const problemId = String(problem?._id || problem?.id || "").trim();
    if (!problemId) throw new Error("Could not resolve the CANNJudge problem id");

    const payload = {
      problemId,
      files: [{ path: "kernel.asc", content: source.content }],
      tiling_h: "",
      tiling_key_h: "",
      host_cpp: "",
      kernel_cpp: "",
      userId: user._id
    };
    console.log(`Submitting one request for problem ${problemId}...`);
    const response = await pageApi(session.page, "/api/submissions/submit", { method: "POST", data: payload });
    const submissionId = String(response?.data?.submissionId || response?.submissionId || "").trim();
    if (!submissionId) throw new Error("Submission response did not contain submissionId");
    console.log(`submissionId=${submissionId}`);
    const result = await poll(session.page, problemId, submissionId, source, options);
    if (options.out) {
      await mkdir(resolve(options.out, ".."), { recursive: true });
      await writeFile(options.out, `${JSON.stringify(result, null, 2)}\n`, "utf8");
    }
    if (options.json) console.log(JSON.stringify(result, null, 2));
    else {
      printHumanResult(result);
      console.log("--- input identity ---");
      console.log(`local SHA256   : ${result.identity?.localSha256}`);
      console.log(`remote SHA256  : ${result.identity?.remoteSha256}`);
      console.log(`identityStatus : ${result.identity?.identityStatus}`);
      console.log(`formalResultEligible: ${result.identity?.formalResultEligible}`);
      if (result.identity?.identityStatus !== "LOCAL_SHA_EQ_REMOTE_SHA") {
        console.error(
          "INPUT_IDENTITY_MISMATCH: remote kernel content does not match local source. " +
          "This submission must NOT be used as a formal experiment result."
        );
        process.exitCode = 3;
      }
    }
    if (result.identity?.formalResultEligible === false) process.exitCode = Math.max(process.exitCode || 0, 3);
  } finally {
    await session.close();
  }
}

async function main() {
  const options = parseArgs(process.argv.slice(2));
  if (options.help) {
    printHelp();
    return;
  }
  if (options.command === "login") {
    await login(options);
    return;
  }
  if (options.command !== "submit") throw new Error(`Unknown command: ${options.command}`);
  if (!options.source && !options.dryRun) {
    throw new Error(
      "FORMAL_SUBMIT_REQUIRES_SOURCE: pass --source <file>. Clipboard/terminal paste is disabled."
    );
  }
  await submit(options);
}

main().catch((error) => {
  console.error(`cannjudge-submit: ${error?.message || error}`);
  process.exitCode = 1;
});
