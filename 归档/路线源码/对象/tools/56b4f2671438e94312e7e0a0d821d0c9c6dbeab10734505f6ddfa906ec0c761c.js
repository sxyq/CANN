/*
 * CANNJudge browser-side submit helper.
 *
 * Run this file in the CANNJudge page console while already signed in.
 * It uses same-origin fetch, so the browser keeps the current session.
 * It never reads cookies, passwords, authorization headers, or browser files.
 *
 * Flow:
 *   1. fetch the public problem/template metadata;
 *   2. choose one local kernel.txt/kernel.asc file;
 *   3. print file identity and project payload;
 *   4. require the exact confirmation string "CONFIRM <filename>";
 *   5. send one POST and poll the returned submissionId.
 *
 * The helper does not retry POST. If the request result is uncertain, keep the
 * returned submissionId (if any) and use cannjudge.py poll instead.
 */
(() => {
  "use strict";

  const PROBLEM_TOKEN = "addrmsnormbias";
  const CONFIRM_PREFIX = "CONFIRM ";
  const POLL_INTERVAL_MS = 15000;
  const POLL_MAX_MS = 30 * 60 * 1000;

  const sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms));

  async function getJson(path, init = {}) {
    const response = await fetch(path, {
      ...init,
      credentials: "same-origin",
      headers: {
        Accept: "application/json",
        ...(init.headers || {})
      }
    });
    const text = await response.text();
    let body = {};
    try {
      body = text ? JSON.parse(text) : {};
    } catch {
      body = { message: text };
    }
    if (!response.ok) {
      const message = body?.message || body?.msg || body?.error || `HTTP ${response.status}`;
      throw new Error(`${response.status}: ${message}`);
    }
    return body;
  }

  async function sha256Hex(bytes) {
    const digest = await crypto.subtle.digest("SHA-256", bytes);
    return [...new Uint8Array(digest)]
      .map((value) => value.toString(16).padStart(2, "0"))
      .join("");
  }

  function readBrowserUserId() {
    let raw = "";
    try {
      raw = localStorage.getItem("cannjudge_user") || "";
    } catch {
      return "";
    }
    if (!raw) return "";
    try {
      const user = JSON.parse(raw);
      return String(user?._id || user?.id || "").trim();
    } catch {
      return "";
    }
  }

  function chooseFile() {
    return new Promise((resolve, reject) => {
      const input = document.createElement("input");
      input.type = "file";
      input.accept = ".txt,.asc,text/plain";
      input.style.display = "none";
      document.body.appendChild(input);
      input.addEventListener("change", () => {
        const file = input.files?.[0] || null;
        input.remove();
        if (!file) reject(new Error("未选择源码文件"));
        else resolve(file);
      }, { once: true });
      input.click();
    });
  }

  function lineCount(text) {
    return text.length === 0 ? 0 : (text.match(/\n/g) || []).length + (text.endsWith("\n") ? 0 : 1);
  }

  function statusKey(status) {
    const value = String(status || "").toLowerCase();
    if (value.includes("pass") || value.includes("accepted") || value === "ac") return "pass";
    if (value.includes("skip")) return "skipped";
    if (value.includes("wrong") || value.includes("compile") || value.includes("runtime") || value.includes("error") || value.includes("fail")) return "fail";
    return "waiting";
  }

  function isTerminal(status) {
    return statusKey(status) !== "waiting";
  }

  function printResult(submission, submissionId) {
    const rows = Array.isArray(submission?.result) ? submission.result : [];
    console.group(`CANNJudge submission ${submissionId}`);
    console.log("总状态:", submission?.status);
    console.log("提交 ID:", submissionId);
    console.table(rows.map((row, index) => ({
      测试点: index + 1,
      状态: row?.testcase_status,
      失配率百分比: row?.precision_ratio == null ? null : (1 - Number(row.precision_ratio)) * 100,
      用时微秒: row?.time,
      最优用时微秒: row?.best_time,
      信息: String(row?.msg || "").slice(0, 240)
    })));
    console.groupEnd();
  }

  async function pollSubmission(submissionId) {
    const started = Date.now();
    let lastStatus = "";
    while (Date.now() - started <= POLL_MAX_MS) {
      const submission = await getJson(`/api/submissions/${encodeURIComponent(submissionId)}`);
      const status = String(submission?.status || "");
      if (status !== lastStatus) {
        console.log(`[CANNJudge] ${new Date().toLocaleTimeString()} status=${status}`);
        lastStatus = status;
      }
      if (isTerminal(status)) {
        printResult(submission, submissionId);
        window.CANNJUDGE_LAST_SUBMISSION = { submissionId, status, result: submission?.result || [] };
        return submission;
      }
      await sleep(POLL_INTERVAL_MS);
    }
    console.warn(`[CANNJudge] 超过 ${POLL_MAX_MS / 60000} 分钟仍未完成，请使用 cannjudge.py poll ${submissionId}`);
    return null;
  }

  async function main() {
    if (location.origin !== "https://cannjudge.cn") {
      throw new Error("请在 https://cannjudge.cn 的已登录页面控制台运行此脚本");
    }

    const userId = readBrowserUserId();
    if (!userId) {
      throw new Error("当前页面没有可用的登录用户标识，请先登录并刷新 CANNJudge 页面");
    }

    const problem = await getJson(`/api/problems/name/${PROBLEM_TOKEN}`);
    const problemId = String(problem?._id || "").trim();
    if (!problemId) throw new Error("无法解析 problemId");

    const templateResponse = await getJson(`/api/problems/${problemId}/template`);
    const templateFiles = Array.isArray(templateResponse?.data?.files)
      ? templateResponse.data.files
      : [];
    const editableFiles = templateFiles.filter((file) => file?.editable === true);
    if (!editableFiles.some((file) => file?.path === "kernel.asc")) {
      throw new Error("模板没有返回可编辑的 kernel.asc，停止发送");
    }

    const file = await chooseFile();
    const bytes = await file.arrayBuffer();
    const content = new TextDecoder("utf-8", { fatal: false }).decode(bytes);
    const sha256 = await sha256Hex(bytes);
    const uploadedFiles = editableFiles.map((item) => ({
      path: item.path,
      content: item.path === "kernel.asc" ? content : String(item.content || "")
    }));
    const kernel = uploadedFiles.find((item) => item.path === "kernel.asc");
    const preview = {
      problemId,
      userId,
      localFile: file.name,
      uploadPath: "kernel.asc",
      lineCount: lineCount(content),
      byteCount: bytes.byteLength,
      sha256,
      firstLine: content.split("\n", 1)[0] || "",
      lastNonEmptyLine: (content.match(/[^\r\n]+(?=\r?\n?$)/)?.[0] || "").trim(),
      hasRunKernel: content.includes("run_kernel"),
      editablePaths: editableFiles.map((item) => item.path)
    };
    console.log("CANNJudge 提交预览（尚未发送 POST）:", preview);
    console.table([preview]);

    const confirmation = window.prompt(
      `将把 ${file.name} 作为 kernel.asc 提交。\n` +
      `行数=${preview.lineCount}，字节=${preview.byteCount}\n` +
      `SHA-256=${sha256}\n\n` +
      `如确认发送一次线上提交，请输入：${CONFIRM_PREFIX}${file.name}`
    );
    if (confirmation !== `${CONFIRM_PREFIX}${file.name}`) {
      console.log("已取消：没有发送 POST");
      return;
    }

    const payload = {
      problemId,
      userId,
      files: uploadedFiles,
      tiling_h: "",
      tiling_key_h: "",
      host_cpp: "",
      kernel_cpp: ""
    };
    const response = await getJson("/api/submissions/submit", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(payload)
    });
    const submissionId = String(response?.data?.submissionId || response?.submissionId || "").trim();
    if (!submissionId) {
      console.log("提交接口返回：", response);
      throw new Error("提交响应中没有 submissionId，请保留页面响应并停止重试");
    }
    console.log(`已提交一次，submissionId=${submissionId}`);
    await pollSubmission(submissionId);
  }

  main().catch((error) => {
    console.error("CANNJudge 提交助手停止：", error);
    window.CANNJUDGE_LAST_ERROR = String(error?.message || error);
  });
})();
