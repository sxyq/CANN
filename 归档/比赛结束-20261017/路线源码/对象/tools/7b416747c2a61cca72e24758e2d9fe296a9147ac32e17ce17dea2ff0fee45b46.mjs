#!/usr/bin/env node

import fs from "fs";
import path from "path";

const readFile = fs.promises.readFile;
const join = path.join;

const resultDir = process.argv[2];
if (!resultDir) {
  console.error("usage: node summarize_window.mjs <result-directory>");
  process.exit(2);
}

function median(values) {
  const sorted = [...values].sort((a, b) => a - b);
  const middle = Math.floor(sorted.length / 2);
  return sorted.length % 2
    ? sorted[middle]
    : (sorted[middle - 1] + sorted[middle]) / 2;
}

function summarize(values) {
  const mean = values.reduce((sum, value) => sum + value, 0) / values.length;
  const variance = values.length > 1
    ? values.reduce((sum, value) => sum + (value - mean) ** 2, 0) / (values.length - 1)
    : 0;
  const min = Math.min(...values);
  const max = Math.max(...values);
  return {
    rep_medians_us: values,
    median_us: median(values),
    mean_us: mean,
    stdev_us: Math.sqrt(variance),
    cv: mean === 0 ? null : Math.sqrt(variance) / mean,
    min_us: min,
    max_us: max,
    max_min: min <= 0 ? null : max / min,
  };
}

async function readRep(block, rep) {
  const name = `PRECHECK-${block}-${String(rep).padStart(2, "0")}-raw.tsv`;
  const content = await readFile(join(resultDir, name), "utf8");
  const lines = content.trim().split(/\r?\n/);
  const headers = lines.shift().split("\t");
  const deviceIndex = headers.indexOf("device_us");
  const phaseIndex = headers.indexOf("phase");
  const blockIndex = headers.indexOf("precheck");
  const repIndex = headers.indexOf("rep");
  const variantIndex = headers.indexOf("variant");
  if ([deviceIndex, phaseIndex, blockIndex, repIndex, variantIndex].some((index) => index < 0)) {
    throw new Error(`${name}: unexpected raw TSV header`);
  }
  const samples = lines.filter(Boolean).map((line) => {
    const fields = line.split("\t");
    if (fields[phaseIndex] !== "WINDOW_PRECHECK" ||
        fields[blockIndex] !== block || Number(fields[repIndex]) !== rep ||
        fields[variantIndex] !== "PARENT") {
      throw new Error(`${name}: row does not match requested precheck`);
    }
    return Number(fields[deviceIndex]);
  });
  if (samples.length !== 21 || samples.some((value) => !Number.isFinite(value) || value <= 0)) {
    throw new Error(`${name}: expected 21 positive device-event samples, got ${samples.length}`);
  }
  return median(samples);
}

async function main() {
  try {
    const blocks = {};
    for (const block of ["A", "B"]) {
      const repMedians = [];
      for (let rep = 1; rep <= 6; rep += 1) {
        repMedians.push(await readRep(block, rep));
      }
      const stats = summarize(repMedians);
      stats.result = stats.cv <= 0.15 && stats.max_min <= 1.30 ? "PASS" : "FAIL";
      blocks[block] = stats;
    }
    const qualified = blocks.A.result === "PASS" && blocks.B.result === "PASS";
    const output = {
      phase: "PARENT_WINDOW_QUALIFICATION",
      route: "SYNC-TOPOLOGY-CHAMPION-X",
      revision: "V001",
      direct_parent: "R31B-V011",
      device: 4,
      shape: [2, 12288],
      dtype: "FP16",
      process_reps_per_block: 6,
      samples_per_process: 21,
      thresholds: { cv_max: 0.15, max_min_max: 1.30 },
      blocks,
      result: qualified ? "QUALIFIED" : "WINDOW_UNQUALIFIED",
      candidate_timing_allowed: qualified,
    };
    console.log(JSON.stringify(output, null, 2));
    process.exitCode = qualified ? 0 : 1;
  } catch (error) {
    console.error(`WINDOW_SUMMARY_ERROR ${error.message}`);
    process.exitCode = 2;
  }
}

main();
