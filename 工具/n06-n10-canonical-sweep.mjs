#!/usr/bin/env node
// Batch the existing Main-2 Canonical V1 build, correctness, Local and scorer
// over the exact N06-N10 source identities already recorded by each route.
// No route source is checked out or rewritten; evidence is published to that
// route's own results tree. The frozen harness/scorer are copied into an
// isolated temporary ROOT so their fixed Main-2 registry/dashboard are untouched.
import fs from 'node:fs';
import path from 'node:path';
import os from 'node:os';
import crypto from 'node:crypto';
import {spawn, execFileSync} from 'node:child_process';
import {fileURLToPath, pathToFileURL} from 'node:url';

const ROOT = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const KERNEL = 'phase4/workspaces/R31B/R31B-V011-LP-ROW-PIPELINE_kernel.asc';
const BASE_SHA = 'a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3';
const CANONICAL_ROOT = process.env.CANN_CANONICAL_TOOL_ROOT ||
  '/home/data4t2/lelinfeng/cann-w2-main2-control';
const ROUTES = [
  {id:'N06', dir:'n06-ub-bank-placement', result:'results'},
  {id:'N07', dir:'n07-parameter-tile-reuse', result:'phase4/workspaces/R31B/results'},
  {id:'N08', dir:'n08-tail-gm-transaction', result:'results'},
  {id:'N09', dir:'n09-rms-reduction-topology', result:'results'},
  {id:'N10', dir:'n10-row-core-balance', result:'results'},
];
const MAIN2_FILES = [
  '工具/main2-canonical-assets.mjs',
  '工具/main2-canonical-benchmark.mjs',
  '工具/main2-canonical-scorer.mjs',
];

function readJson(file) {
  try { return JSON.parse(fs.readFileSync(file, 'utf8')); } catch { return null; }
}
function sha256(bytes) { return crypto.createHash('sha256').update(bytes).digest('hex'); }
function keyValues(text) {
  return Object.fromEntries(text.split(/\r?\n/).map(line => {
    const i = line.indexOf('=');
    return i < 0 ? null : [line.slice(0, i).trim(), line.slice(i + 1).trim()];
  }).filter(Boolean));
}
function parseArgs(argv) {
  const out = {dryRun:false, resume:false, buildJobs:2, device:4, limit:Infinity, route:null, batchId:null};
  for (let i=0; i<argv.length; i++) {
    const a=argv[i];
    if (a==='--dry-run') out.dryRun=true;
    else if (a==='--resume') out.resume=true;
    else if (a==='--batch-id') out.batchId=argv[++i];
    else if (a==='--build-jobs') out.buildJobs=Number(argv[++i]);
    else if (a==='--device') out.device=Number(argv[++i]);
    else if (a==='--limit') out.limit=Number(argv[++i]);
    else if (a==='--route') out.route=argv[++i];
    else if (a==='--help' || a==='-h') out.help=true;
    else throw new Error('Unknown argument: '+a);
  }
  if (!Number.isInteger(out.buildJobs)||out.buildJobs<1||!Number.isInteger(out.device)||out.device<0||
      !(out.limit>0)) throw new Error('Invalid --build-jobs, --device or --limit');
  return out;
}
function usage() {
  return 'node 工具/n06-n10-canonical-sweep.mjs [--dry-run] [--route N06] [--limit N] [--build-jobs N] [--device N] [--batch-id ID] [--resume]';
}
function runGit(wt, args, encoding='utf8') {
  return execFileSync('git', args, {cwd:wt, encoding, maxBuffer:16*1024*1024, stdio:['ignore','pipe','pipe']});
}

function routeFiles(spec) {
  const dir=path.join(ROOT,'worktrees',spec.dir,spec.result);
  if(!fs.existsSync(dir)) return [];
  return fs.readdirSync(dir).sort().map(name=>path.join(dir,name));
}

function buildRouteIndex(spec) {
  const wt=path.join(ROOT,'worktrees',spec.dir), bySha=new Map();
  const commits=runGit(wt,['log','--format=%H','HEAD','--',KERNEL]).trim().split(/\r?\n/).filter(Boolean);
  for(const commit of commits) {
    try {
      const bytes=runGit(wt,['show',`${commit}:${KERNEL}`],'buffer');
      const sha=sha256(bytes);
      if(!bySha.has(sha)) bySha.set(sha,{bytes,commit,origin:'git:'+commit+':'+KERNEL});
    } catch {}
  }
  let snapshot;
  try {
    const bytes=fs.readFileSync(path.join(wt,KERNEL));
    snapshot={bytes,commit:'WORKTREE_SNAPSHOT',origin:'worktree:'+spec.dir+'/'+KERNEL};
    bySha.set(sha256(bytes),snapshot);
  } catch {}
  return {wt,bySha,snapshot};
}

function candidateFor(index, spec, revision, sourceSha, recordPath, recordedCommit='') {
  const normalizedSha=String(sourceSha||'').toLowerCase();
  const row={route:spec.id,revision:String(revision||path.basename(recordPath)),sourceSha:normalizedSha,
    recordPaths:[path.relative(ROOT,recordPath)],recordedCommit:String(recordedCommit||''),source:null,
    benchRevision:'',evidenceDir:'',sourceStatus:'UNKNOWN'};
  if(!/^[a-f0-9]{64}$/.test(normalizedSha)) { row.sourceStatus='SOURCE_SHA_MISSING_OR_INVALID'; return row; }
  const indexCommit=()=>index.bySha.get(normalizedSha);
  const direct=String(recordedCommit||'');
  if(direct) {
    try {
      const resolved=runGit(index.wt,['rev-parse',`${direct}^{commit}`]).trim();
      const bytes=runGit(index.wt,['show',`${resolved}:${KERNEL}`],'buffer');
      if(sha256(bytes)===normalizedSha) row.source={bytes,commit:resolved,origin:'recorded-commit:'+resolved};
    } catch {}
  }
  if(!row.source) row.source=indexCommit()||null;
  if(!row.source && index.snapshot && sha256(index.snapshot.bytes)===normalizedSha) row.source=index.snapshot;
  row.sourceStatus=row.source?'RESOLVED':'SOURCE_NOT_RECOVERABLE_IN_ROUTE_TREE';
  return row;
}

function collectCandidates() {
  const all=[];
  for(const spec of ROUTES) {
    const files=routeFiles(spec), index=buildRouteIndex(spec), found=[];
    const add=(revision,sha,file,commit='')=>found.push(candidateFor(index,spec,revision,sha,file,commit));
    if(spec.id==='N06') for(const f of files.filter(p=>/^run_[0-9]+\.json$/.test(path.basename(p)))) {
      const j=readJson(f); if(!j) continue;
      const sha=j.source_sha256?.[KERNEL]||j.source_sha256;
      const runNo=path.basename(f).match(/^run_([0-9]+)\.json$/)?.[1];
      const rev=/^R[0-9]+$/.test(String(j.revision))?j.revision:(runNo?`R${runNo.padStart(3,'0')}`:path.basename(f,'.json'));
      add(rev,sha,f,j.source_commit||(/^[a-f0-9]{40}$/i.test(j.revision||'')?j.revision:''));
    }
    if(spec.id==='N07') for(const f of files.filter(p=>/^n07_r[0-9]+_build\.json$/i.test(path.basename(p)))) {
      const j=readJson(f); if(!j) continue;
      add(j.revision||path.basename(f).match(/r[0-9]+/i)?.[0],j.source_sha256,f,j.source_commit||'');
    }
    if(spec.id==='N08') for(const f of files.filter(p=>/^N08-R[0-9]+\.txt$/.test(path.basename(p)))) {
      const v=keyValues(fs.readFileSync(f,'utf8'));
      add(v.revision||path.basename(f,'.txt').slice(4),v.source_sha256,f,v.source_commit||'');
    }
    if(spec.id==='N09') for(const f of files.filter(p=>/^build_attempt_[0-9]+_state\.txt$/.test(path.basename(p)))) {
      const v=keyValues(fs.readFileSync(f,'utf8'));
      const attempt=path.basename(f).match(/([0-9]+)/)?.[1]||'unknown';
      add(v.revision||`ATTEMPT-${attempt.padStart(3,'0')}`,v.kernel_sha256||v.source_sha256,f,
        v.source_commit||v.parent_or_source_commit||'');
    }
    if(spec.id==='N10') for(const f of files.filter(p=>/^N10-R[0-9]+\.json$/.test(path.basename(p)))) {
      const j=readJson(f); if(!j) continue;
      const commits=Array.isArray(j.source_commits)?j.source_commits:[];
      add(j.revision||path.basename(f).slice(4,-5),j.source_sha256,f,commits.at(-1)||j.source_commit||'');
    }
    const merged=new Map();
    for(const row of found) {
      const k=[row.revision,row.sourceSha].join('@');
      if(merged.has(k)) merged.get(k).recordPaths.push(...row.recordPaths);
      else merged.set(k,row);
    }
    for(const row of merged.values()) {
      row.benchRevision=`${row.revision}-${row.sourceSha.slice(0,10)}`;
      row.evidenceDir=path.join(ROOT,'worktrees',spec.dir,spec.result,'canonical-sweep');
      all.push(row);
    }
  }
  return all.sort((a,b)=>a.route.localeCompare(b.route)||a.revision.localeCompare(b.revision,undefined,{numeric:true})||a.sourceSha.localeCompare(b.sourceSha));
}

function writeTsv(file, rows, columns) {
  const cell=v=>String(v??'NA').replace(/[\r\n\t]+/g,' ');
  fs.mkdirSync(path.dirname(file),{recursive:true});
  fs.writeFileSync(file,[columns.join('\t'),...rows.map(r=>columns.map(c=>cell(r[c])).join('\t'))].join('\n')+'\n');
}
function routeOutput(spec,batchId) {
  return path.join(ROOT,'worktrees',spec.dir,spec.result,'canonical-sweep',batchId);
}
function outFor(candidate,batchId) {
  const spec=ROUTES.find(r=>r.id===candidate.route);
  return path.join(routeOutput(spec,batchId),'versions',`${candidate.revision}-${candidate.sourceSha.slice(0,10)}`);
}
function emitPlan(rows, opts) {
  const byRoute=new Map(ROUTES.map(r=>[r.id,rows.filter(x=>x.route===r.id)]));
  for(const [route,items] of byRoute) {
    const resolved=items.filter(x=>x.sourceStatus==='RESOLVED').length;
    console.log(`${route}: versions=${items.length} recoverable=${resolved} unresolved=${items.length-resolved}`);
  }
  console.log(`TOTAL: versions=${rows.length} recoverable=${rows.filter(x=>x.sourceStatus==='RESOLVED').length} unresolved=${rows.filter(x=>x.sourceStatus!=='RESOLVED').length}`);
  if(opts.limit!==Infinity) console.log(`LIMIT: first ${opts.limit} candidates in manifest order`);
  if(opts.dryRun) for(const r of rows.slice(0,opts.limit))
    console.log([r.route,r.revision,r.sourceSha,r.source?.commit||'UNRESOLVED',r.sourceStatus,r.recordPaths.join(';')].join('\t'));
}

function copyCanonicalSandbox(scratch) {
  const toolsDir=path.join(scratch,'工具'), dataDir=path.join(scratch,'本地实验/MAIN2-CANONICAL-V1');
  fs.mkdirSync(toolsDir,{recursive:true});
  fs.mkdirSync(path.join(dataDir,'sources'),{recursive:true});
  fs.mkdirSync(path.join(dataDir,'versions','R31B__V011'),{recursive:true});
  for(const rel of MAIN2_FILES) fs.copyFileSync(path.join(CANONICAL_ROOT,rel),path.join(scratch,rel));
  fs.cpSync(path.join(CANONICAL_ROOT,'工具/main2-canonical-harness'),path.join(toolsDir,'main2-canonical-harness'),{recursive:true});
  fs.mkdirSync(path.join(scratch,'本地实验'),{recursive:true});
  fs.copyFileSync(path.join(CANONICAL_ROOT,'本地实验/CANONICAL-LOCAL-SUITE-V1.tsv'),
    path.join(scratch,'本地实验/CANONICAL-LOCAL-SUITE-V1.tsv'));
  fs.copyFileSync(path.join(CANONICAL_ROOT,'本地实验/R31B-V011-CANONICAL-LOCAL-VECTOR.tsv'),
    path.join(scratch,'本地实验/R31B-V011-CANONICAL-LOCAL-VECTOR.tsv'));
  fs.copyFileSync(path.join(CANONICAL_ROOT,'线上结果/R31B/V011/submission.asc'),
    path.join(dataDir,'sources',BASE_SHA+'.asc'));
  const anchorSource=path.join(CANONICAL_ROOT,'本地实验/MAIN2-CANONICAL-V1/versions/R31B__V011');
  for(const name of ['build.json','correctness.json','measurements.tsv'])
    fs.copyFileSync(path.join(anchorSource,name),path.join(dataDir,'versions/R31B__V011',name));
  const anchor=readJson(path.join(dataDir,'versions/R31B__V011/build.json'));
  if(!anchor||anchor.BUILD_STATUS!=='PASS'||!fs.existsSync(anchor.EXE)||sha256(fs.readFileSync(anchor.EXE))!==anchor.EXE_SHA)
    throw new Error('Canonical V1 anchor executable identity is missing or invalid');

  // The historical campaign has fixed resource admission thresholds. This
  // isolated copy removes only those two preflight gates; telemetry remains
  // recorded and real CANN/runtime errors still determine command status.
  const benchmarkPath=path.join(toolsDir,'main2-canonical-benchmark.mjs');
  let source=fs.readFileSync(benchmarkPath,'utf8');
  const hbmGate="if(!Number.isFinite(freeLower)||freeLower<100)throw new Error('HBM_FREE_LOWER_BOUND_LT_100_OR_UNKNOWN');";
  const diskGate="if(!bytes||Number(bytes[3])<15*1024*1024)throw new Error('AVAILABLE_DISK_LT_15G_OR_UNKNOWN');";
  if(!source.includes(hbmGate)||!source.includes(diskGate)) throw new Error('Canonical benchmark resource gate text changed; review before adapting');
  source=source.replace(hbmGate,"if(!Number.isFinite(freeLower))throw new Error('HBM_USAGE_UNKNOWN');")
    .replace(diskGate,"if(!bytes)throw new Error('DISK_USAGE_QUERY_FAILED');");
  fs.writeFileSync(benchmarkPath,source);
  fs.writeFileSync(path.join(scratch,'sandbox-adaptation.json'),JSON.stringify({
    source_tool_root:CANONICAL_ROOT,
    changes:[
      'Copied exact Canonical V1 assets/harness/scorer into an isolated temporary ROOT.',
      'Removed only fixed HBM-free and 15GB-disk preflight thresholds in the temporary benchmark copy.',
      'Device/load/HBM and host/disk observations remain captured; build/runtime errors remain authoritative.',
      'No change to suite, runner_ref.inc, timing protocol, scorer, score formula, Main-2 registry or scoreboard.'
    ],
    suite_sha256:sha256(fs.readFileSync(path.join(scratch,'本地实验/CANONICAL-LOCAL-SUITE-V1.tsv'))),
    harness_sha256:sha256(fs.readFileSync(path.join(toolsDir,'main2-canonical-harness/runner_ref.inc')))
  },null,2)+'\n');
}

function atomicJson(file,value) {
  fs.mkdirSync(path.dirname(file),{recursive:true});
  const temp=file+'.tmp-'+process.pid;
  fs.writeFileSync(temp,JSON.stringify(value,null,2)+'\n');
  fs.renameSync(temp,file);
}

function copyEvidence(scratch, candidate, batchId) {
  const scratchDir=path.join(scratch,'本地实验/MAIN2-CANONICAL-V1/versions',`${candidate.route}__${candidate.benchRevision}`);
  const dest=path.join(outFor(candidate,batchId),'evidence');
  if(fs.existsSync(scratchDir)) fs.cpSync(scratchDir,dest,{recursive:true,force:true});
}

async function workerMain(args) {
  const [scratch,batchId,route,revision,benchRevision,sourceSha,sourceFile,outDir,phase,deviceText,sourceCommit]=args;
  if(!scratch||!batchId||!route||!revision||!benchRevision||!sourceSha||!sourceFile||!outDir||!phase)
    throw new Error('Malformed --worker invocation');
  const [{build,correctness,measure},{scoreOne},{readTsv,json,writeJson,versionDir}]=await Promise.all([
    import(pathToFileURL(path.join(scratch,'工具/main2-canonical-benchmark.mjs'))),
    import(pathToFileURL(path.join(scratch,'工具/main2-canonical-scorer.mjs'))),
    import(pathToFileURL(path.join(scratch,'工具/main2-canonical-assets.mjs'))),
  ]);
  const device=Number(deviceText), r={ROUTE:route,REVISION:benchRevision,SOURCE_SHA:sourceSha,
    SOURCE_RECOVERABLE:'YES',CANONICAL_SOURCE_PATH:sourceFile,KNOWN_UNRESOLVED_CORRECTNESS_FAILURE:'NO'};
  fs.mkdirSync(outDir,{recursive:true});
  const statusFile=path.join(outDir,'status.json'), prev=readJson(statusFile)||{
    route,revision,source_commit:sourceCommit,source_sha256:sourceSha,batch_id:batchId,
    build:'NOT_RUN',correctness:'NOT_RUN',local_score:null,local_status:'NOT_RUN'
  };
  try {
    if(phase==='build') {
      const b=build(r);
      prev.build=b.BUILD_STATUS; prev.artifact_sha256=b.EXE_SHA||null;
      prev.build_command=b.BUILD_COMMAND||b.BUILD_COMMAND||null;
      prev.build_return_code=b.BUILD_RC??null; prev.build_reason=b.REASON||null;
    } else if(phase==='verify') {
      const b=json(path.join(versionDir(r),'build.json'));
      if(!b||b.BUILD_STATUS!=='PASS') throw new Error('BUILD_NOT_PASS');
      const c=correctness(r,device);
      prev.correctness=c.CORRECTNESS_STATUS;
      prev.correctness_pass_count=c.PASS_COUNT??0;
      prev.correctness_case_count=c.CASE_COUNT??7;
      if(c.CORRECTNESS_STATUS==='PASS') {
        measure(r,device);
        const anchorRuns=readTsv(path.join(versionDir({ROUTE:'R31B',REVISION:'V011'}),'measurements.tsv'));
        const scored=scoreOne(r,anchorRuns);
        if(scored.errors.length) {
          prev.local_status='UNSCORED_MEASUREMENT_INVALID'; prev.score_errors=scored.errors;
        } else {
          const perCase=scored.cells.map(cell=>({
            case_id:cell.c.CASE_ID,role:cell.c.CASE_ROLE,
            anchor_median_us:cell.anchor.median,candidate_median_us:cell.candidate.median,
            parent_median_us:cell.parent.median,quality:cell.quality,
            anchor_run_medians_us:cell.anchor.us,candidate_run_medians_us:cell.candidate.us
          }));
          const local={scorer_version:'MAIN2-CANONICAL-V1',suite_version:'CANONICAL_LOCAL_SUITE_V1',
            route,revision,source_commit:sourceCommit,source_sha256:sourceSha,
            canonical_local_score:Number(scored.score.toFixed(6)),status:scored.status,
            quality:scored.quality,local_gain_fragile:scored.fragile?'YES':'NO',
            reliable_local_gain:scored.reliable?'YES':'NO',core_gain_count:scored.coreGainCount,
            uncertainty:scored.uncertainty,empirical_low:scored.low,empirical_high:scored.high,
            paired_diagnostic_score:scored.pairedScore,cases:perCase};
          writeJson(path.join(versionDir(r),'canonical-score.json'),local);
          prev.local_score=local.canonical_local_score; prev.local_status=local.status;
          prev.local_quality=local.quality; prev.local_gain_fragile=local.local_gain_fragile;
          prev.reliable_local_gain=local.reliable_local_gain;
        }
      } else {
        prev.local_status='UNSCORED_CORRECTNESS_BLOCKED';
      }
    } else throw new Error('Unknown worker phase: '+phase);
    prev.updated_at=new Date().toISOString();
    atomicJson(statusFile,prev);
    copyEvidence(scratch,{route,revision,sourceSha,benchRevision},batchId);
    console.log(`${route} ${revision} ${phase}: ${phase==='build'?prev.build:prev.local_status}`);
  } catch(e) {
    prev[phase==='build'?'build':'local_status']=phase==='build'?'BUILD_ERROR':'ERROR';
    prev.last_error=String(e?.stack||e); prev.updated_at=new Date().toISOString();
    atomicJson(statusFile,prev); copyEvidence(scratch,{route,revision,sourceSha,benchRevision},batchId);
    console.error(`${route} ${revision} ${phase} ERROR: ${e?.message||e}`);
    process.exitCode=1;
  }
}

function runWorker(scriptArgs) {
  return new Promise(resolve=>{
    const child=spawn(process.execPath,[fileURLToPath(import.meta.url),'--worker',...scriptArgs],
      {cwd:ROOT,stdio:'inherit'});
    child.on('error',e=>resolve({code:1,error:String(e)}));
    child.on('close',code=>resolve({code:code??1}));
  });
}

async function pool(items,limit,fn) {
  let cursor=0;
  const workers=Array.from({length:Math.min(limit,items.length)},async()=>{
    while(cursor<items.length) { const item=items[cursor++]; await fn(item); }
  });
  await Promise.all(workers);
}

function renderSummary(candidates,batchId) {
  for(const spec of ROUTES.filter(r=>candidates.some(c=>c.route===r.id))) {
    const routeRows=candidates.filter(c=>c.route===spec.id).map(c=>{
      const state=readJson(path.join(outFor(c,batchId),'status.json'))||{};
      return {route:c.route,revision:c.revision,source_commit:c.source?.commit||'UNKNOWN',source_sha256:c.sourceSha,
        source_status:c.sourceStatus,build:state.build||'NOT_RUN',correctness:state.correctness||'NOT_RUN',
        canonical_local_score:state.local_score??'NONE',local_status:state.local_status||'NOT_RUN',
        quality:state.local_quality||'NA',local_gain_fragile:state.local_gain_fragile||'NA',
        record_paths:c.recordPaths.join(';'),evidence_path:path.relative(ROOT,outFor(c,batchId))};
    });
    const dir=routeOutput(spec,batchId);
    writeTsv(path.join(dir,'summary.tsv'),routeRows,
      ['route','revision','source_commit','source_sha256','source_status','build','correctness','canonical_local_score','local_status','quality','local_gain_fragile','record_paths','evidence_path']);
    const scored=routeRows.filter(r=>Number.isFinite(Number(r.canonical_local_score))).sort((a,b)=>Number(b.canonical_local_score)-Number(a.canonical_local_score));
    atomicJson(path.join(dir,'best.json'),{route:spec.id,versions:routeRows.length,scored:scored.length,
      best:scored[0]||null,unscored:routeRows.length-scored.length});
    if(scored[0]) console.log(`BEST ${spec.id} ${scored[0].revision} ${scored[0].canonical_local_score}`);
  }
}

async function main(opts) {
  if(opts.help) { console.log(usage()); return; }
  if(!fs.existsSync(path.join(CANONICAL_ROOT,'工具/main2-canonical-benchmark.mjs')))
    throw new Error('Existing Canonical V1 benchmark not found at '+CANONICAL_ROOT);
  let candidates=collectCandidates();
  if(opts.route) candidates=candidates.filter(c=>c.route===opts.route);
  if(!candidates.length) throw new Error('No recorded versions found for selected scope');
  emitPlan(candidates,opts);
  if(opts.dryRun) return;
  if(candidates.some(c=>c.sourceStatus!=='RESOLVED'))
    console.warn('Some historical sources are unresolved; those versions will be recorded as NOT_RUN, not silently omitted.');
  candidates=candidates.slice(0,opts.limit);

  const batchId=opts.batchId||`v1-${new Date().toISOString().replace(/[-:.]/g,'')}`;
  if(!/^[A-Za-z0-9._-]+$/.test(batchId)) throw new Error('Unsafe batch id');
  const activeRoutes=ROUTES.filter(r=>candidates.some(c=>c.route===r.id));
  const outputDirs=activeRoutes.map(r=>routeOutput(r,batchId));
  let scratch='';
  if(opts.resume) {
    const marker=outputDirs.map(d=>readJson(path.join(d,'batch.json'))).find(Boolean);
    if(!marker?.scratch_root||!fs.existsSync(marker.scratch_root))
      throw new Error('Cannot resume: the original isolated scratch root is missing; start a new --batch-id to preserve evidence.');
    scratch=marker.scratch_root;
  } else {
    if(outputDirs.some(fs.existsSync)) throw new Error(`Batch output already exists; use --resume --batch-id ${batchId} or choose a new id`);
    scratch=fs.mkdtempSync(path.join(os.tmpdir(),'cann-n06-n10-canonical-'));
    copyCanonicalSandbox(scratch);
  }
  const batchMeta={batch_id:batchId,created_at:new Date().toISOString(),scratch_root:scratch,
    canonical_root:CANONICAL_ROOT,device:opts.device,build_jobs:opts.buildJobs,
    canonical_suite:'CANONICAL_LOCAL_SUITE_V1',scorer:'MAIN2-CANONICAL-V1',
    source_inventory_count:candidates.length,execution_command:`node ${path.relative(ROOT,fileURLToPath(import.meta.url))} --batch-id ${batchId}`};
  for(const spec of activeRoutes) {
    const dir=routeOutput(spec,batchId); fs.mkdirSync(dir,{recursive:true});
    atomicJson(path.join(dir,'batch.json'),{...batchMeta,route:spec.id});
    const rows=candidates.filter(c=>c.route===spec.id);
    writeTsv(path.join(dir,'manifest.tsv'),rows.map(c=>({route:c.route,revision:c.revision,bench_revision:c.benchRevision,
      source_commit:c.source?.commit||'UNKNOWN',source_sha256:c.sourceSha,source_status:c.sourceStatus,
      source_origin:c.source?.origin||'UNKNOWN',record_paths:c.recordPaths.join(';')})),
      ['route','revision','bench_revision','source_commit','source_sha256','source_status','source_origin','record_paths']);
  }
  atomicJson(path.join(scratch,'RUN.json'),batchMeta);
  const sourceDir=path.join(scratch,'本地实验/MAIN2-CANONICAL-V1/sources');
  for(const c of candidates) if(c.source) {
    const file=path.join(sourceDir,c.sourceSha+'.asc');
    if(!fs.existsSync(file)) fs.writeFileSync(file,c.source.bytes);
  }

  const runnable=candidates.filter(c=>c.sourceStatus==='RESOLVED');
  console.log(`BUILD_PHASE start versions=${runnable.length} jobs=${opts.buildJobs}`);
  await pool(runnable,opts.buildJobs,async c=>{
    const sourceFile=path.join(sourceDir,c.sourceSha+'.asc');
    await runWorker([scratch,batchId,c.route,c.revision,c.benchRevision,c.sourceSha,sourceFile,outFor(c,batchId),'build',String(opts.device),c.source.commit]);
  });
  console.log(`CANONICAL_DEVICE_PHASE start device=${opts.device}; serial order to retain paired sampling`);
  for(const c of runnable) {
    const state=readJson(path.join(outFor(c,batchId),'status.json'))||{};
    if(state.build!=='PASS') continue;
    const sourceFile=path.join(sourceDir,c.sourceSha+'.asc');
    await runWorker([scratch,batchId,c.route,c.revision,c.benchRevision,c.sourceSha,sourceFile,outFor(c,batchId),'verify',String(opts.device),c.source.commit]);
  }
  for(const c of candidates.filter(v=>v.sourceStatus!=='RESOLVED')) {
    atomicJson(path.join(outFor(c,batchId),'status.json'),{route:c.route,revision:c.revision,
      source_sha256:c.sourceSha,build:'NOT_RUN_SOURCE_MISSING',correctness:'NOT_RUN',
      local_score:null,local_status:'UNSCORED_SOURCE_MISSING',record_paths:c.recordPaths});
  }
  renderSummary(candidates,batchId);
  console.log(`BATCH_COMPLETE ${batchId}; scratch=${scratch}`);
}

const argv=process.argv.slice(2);
if(argv[0]==='--worker') {
  workerMain(argv.slice(1)).catch(e=>{console.error(e);process.exitCode=1;});
} else {
  try { await main(parseArgs(argv)); }
  catch(e) { console.error(e?.stack||e); process.exitCode=1; }
}
