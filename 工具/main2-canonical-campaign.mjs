#!/usr/bin/env node
// Event-driven host build lookahead + one serial NPU lane. Local commits never
// wait for GitHub. No new source, route or Online action is implemented here.
import fs from 'node:fs';
import path from 'node:path';
import {spawn,execFileSync} from 'node:child_process';
import {ROOT,DATA,REGISTRY,BASE_SHA,readTsv,writeTsv,write,writeJson,json,exists,key,safe,versionDir,git} from './main2-canonical-assets.mjs';
import {scoreboard} from './main2-canonical-scorer.mjs';

const bench=path.join(ROOT,'工具/main2-canonical-benchmark.mjs');
const events=path.join(DATA,'campaign-events.tsv');
const dash=path.join(ROOT,'调度/MAIN2-CANONICAL-DASHBOARD.md');
const knownReuse=new Set(readTsv(path.join(ROOT,'研究/主代理/MAIN-2/calibration-v4/runs/20261005-v4-engineering/engineering-runs.tsv')).map(key));
const registry=readTsv(REGISTRY).filter(r=>r.IMPLEMENTED==='YES').sort((a,b)=>{
  const priority=r=>knownReuse.has(key(r))?0:r.OFFICIAL_SCORE!=='NA'?1:r.KNOWN_UNRESOLVED_CORRECTNESS_FAILURE==='YES'?3:2;
  return priority(a)-priority(b)||key(a).localeCompare(key(b));
});
let commitQueue=Promise.resolve();
function filesBelow(dir) {
  if(!exists(dir))return [];
  return fs.readdirSync(dir,{withFileTypes:true}).flatMap(e=>e.isDirectory()?filesBelow(path.join(dir,e.name)):[path.join(dir,e.name)]);
}
function commit(message,files) {
  commitQueue=commitQueue.then(()=>{
    const present=[...new Set(files)].filter(exists).map(f=>path.relative(ROOT,path.resolve(f)));
    if(!present.length)return;
    execFileSync('git',['add','--',...present],{cwd:ROOT,stdio:'pipe'});
    const changed=git(['diff','--cached','--name-only']);
    if(!changed)return;
    execFileSync('git',['-c','gc.auto=0','commit','-qm',message],{cwd:ROOT,stdio:'pipe'});
    console.log('COMMIT '+git(['rev-parse','--short','HEAD'])+' '+message);
  });return commitQueue;
}
function event(r,stage,result) {
  const all=readTsv(events);all.push({AT:new Date().toISOString(),VERSION:key(r),SOURCE_SHA:r.SOURCE_SHA,STAGE:stage,RESULT:result});
  writeTsv(events,all);
}
function run(action,r,stream=true) {
  return new Promise((resolve,reject)=>{
    const p=spawn(process.execPath,[bench,action,key(r),'4'],{cwd:ROOT,stdio:['ignore','pipe','pipe']});
    let out='',err='';
    p.stdout.on('data',b=>{out+=b;if(stream)process.stdout.write(b);});p.stderr.on('data',b=>{err+=b;process.stderr.write(b);});
    p.on('error',reject);p.on('close',code=>resolve({code,out,err}));
  });
}
async function buildOne(r) {
  const result=await run('build',r,false),b=json(path.join(versionDir(r),'build.json'));
  if(result.code!==0&&!b)throw new Error('Build infrastructure error '+key(r)+':'+result.err);
  event(r,'BUILD',b?.BUILD_STATUS||'INCOMPLETE');
  const files=filesBelow(versionDir(r)).filter(f=>/\/(?:build.json|configure.log|build.log|build-host-before.txt)$/.test(f));
  await commit('evidence(canonical-build): '+key(r)+' '+(b?.BUILD_STATUS||'INCOMPLETE'),[...files,events]);
  console.log('BUILD_READY '+key(r)+' '+b?.BUILD_STATUS);return b;
}
function refreshRegistry(r,b,corr) {
  const all=readTsv(REGISTRY),row=all.find(v=>key(v)===key(r));
  if(row) {
    row.BUILD_STATUS=b?.BUILD_STATUS||row.BUILD_STATUS;row.BUILDABLE=b?.BUILD_STATUS==='PASS'?'YES':'NO';
    if(corr) {
      row.CORRECTNESS_STATUS=corr.CORRECTNESS_STATUS;row.CORRECTNESS_KNOWN='YES';
      row.CORRECTNESS_VALID=corr.CORRECTNESS_STATUS==='PASS'&&row.KNOWN_UNRESOLVED_CORRECTNESS_FAILURE!=='YES'?'YES':'NO';
    }
    row.EVIDENCE_PATH+=';'+path.relative(ROOT,versionDir(r));
  }
  writeTsv(REGISTRY,all);
}
function dashboard(rows,current) {
  const scored=rows.filter(r=>r.CANONICAL_LOCAL_SCORE!=='UNSCORED').sort((a,b)=>Number(b.CANONICAL_LOCAL_SCORE)-Number(a.CANONICAL_LOCAL_SCORE));
  const leader=scored[0],head=git(['rev-parse','--short','HEAD']),remote=git(['rev-parse','--short','refs/remotes/origin/w2/main2/control']);
  write(dash,['# Main-2 Canonical Local Dashboard','','| Field | Current value |','|---|---|',
    '| Official Champion | R31B V011 |','| Official Score | 45.16 |',
    '| Canonical Local Champion | '+key(leader)+(leader.LOCAL_GAIN_FRAGILE==='YES'?' — provisional/fragile':'')+' |',
    '| Canonical Local Score | '+leader.CANONICAL_LOCAL_SCORE+' |','| R31B V011 Local Anchor | 100.000000 — fresh ENGINEERING_3RUN |',
    '| Current Active Route | NONE — performance development frozen |','| Current Active Revision | NONE — existing assets only |',
    '| Current Local Score | '+(current?.CANONICAL_LOCAL_SCORE||'N/A')+' ('+(current?key(current):'anchor')+') |',
    '| Delta vs Local Champion | '+(current?.CANONICAL_LOCAL_SCORE!=='UNSCORED'&&current?(Number(current.CANONICAL_LOCAL_SCORE)-Number(leader.CANONICAL_LOCAL_SCORE)).toFixed(6):'N/A')+' |',
    '| Next Planning Gate | Canonical asset census, scoreboard and route advice; MAIN_SELECTED=NONE |',
    '| PUSH_STATUS | '+(head===remote?'TRACKING_SHA_MATCH; final live verification required':'PUSH_PENDING_NETWORK; last verified remote '+remote)+' |','',
    'Truth: [scoreboard](../本地实验/MAIN2-CANONICAL-LOCAL-SCOREBOARD.tsv) · [registry](../技术路线/MAIN2-CANONICAL-ROUTE-REGISTRY.tsv).',
    'Local score is not an Official score or a prediction. Predictor history is not a gate.'].join('\n'));
}

// At most two host-only builds run concurrently. No NPU is reserved for build.
const readiness=new Map();let next=0;
const waiter=new Map(registry.map(r=>{let resolve;const promise=new Promise(r=>resolve=r);return [key(r),{resolve,promise}];}));
async function buildWorker() {
  while(next<registry.length) {
    const r=registry[next++];
    try {const b=await buildOne(r);readiness.set(key(r),b);waiter.get(key(r)).resolve({b});}
    catch(e){waiter.get(key(r)).resolve({error:e.message});}
  }
}

async function main() {
  if(!exists(path.join(ROOT,'本地实验/R31B-V011-CANONICAL-LOCAL-VECTOR.tsv')))throw new Error('FRESH_ANCHOR_REQUIRED');
  const workers=[buildWorker(),buildWorker()];
  for(const r of registry) {
    const ready=await waiter.get(key(r)).promise;if(ready.error)throw new Error(ready.error);
    const b=ready.b;
    let corr;
    if(b?.BUILD_STATUS==='PASS') {
      const c=await run('correctness',r,false);corr=json(path.join(versionDir(r),'correctness.json'));
      if(c.code!==0&&!corr)throw new Error('Correctness infrastructure error '+key(r));
      event(r,'CORRECTNESS',corr?.CORRECTNESS_STATUS||'INCOMPLETE');
      await commit('evidence(canonical-correctness): '+key(r)+' '+corr?.CORRECTNESS_STATUS,
        [path.join(versionDir(r),'correctness.json'),...filesBelow(path.join(versionDir(r),'correctness')),events]);
      console.log('CORRECTNESS_READY '+key(r)+' '+corr?.CORRECTNESS_STATUS);
      if(corr?.CORRECTNESS_STATUS==='PASS') {
        const reused=await run('reuse',r,false);
        if(reused.code!==0)throw new Error('Reuse audit failed '+key(r));
        if(!exists(path.join(versionDir(r),'measurements.tsv'))) {
          const timed=await run('measure',r);
          if(timed.code!==0) {
            writeJson(path.join(versionDir(r),'measurement-blocker.json'),{VERSION:key(r),ERROR:timed.err,RC:timed.code});
          }
        }
        event(r,'MEASUREMENT',json(path.join(versionDir(r),'raw-reuse.json'))?.SCORE_SOURCE||'FRESH_MEASUREMENT');
        await commit('evidence(canonical-timing): '+key(r),[
          path.join(versionDir(r),'measurements.tsv'),path.join(versionDir(r),'attempts.tsv'),path.join(versionDir(r),'raw-reuse.json'),
          path.join(versionDir(r),'measurement-blocker.json'),...filesBelow(path.join(versionDir(r),'measurement')),events]);
      }
    }
    refreshRegistry(r,b,corr);
    const rows=scoreboard({quiet:true}),current=rows.find(v=>key(v)===key(r));
    event(r,'SCORE',current?.CANONICAL_LOCAL_SCORE||'UNSCORED');dashboard(rows,current);
    const statusFile=path.join(versionDir(r),'RUN-STATUS.md');
    write(statusFile,['# '+key(r)+' — canonical local measurement','',
      'SCORER_VERSION=MAIN2-CANONICAL-V1','SUITE_VERSION=CANONICAL_LOCAL_SUITE_V1','SOURCE_SHA='+r.SOURCE_SHA,
      'CANONICAL_LOCAL_SCORE='+current?.CANONICAL_LOCAL_SCORE,'STATUS='+current?.STATUS,'REASON='+current?.UNSCORED_REASON,
      'Kernel unchanged; historical Local/Official verdicts unchanged; no next Revision; MAIN_SELECTED=NONE.'].join('\n'));
    await commit('score(canonical): '+key(r)+' '+current?.CANONICAL_LOCAL_SCORE,[
      path.join(ROOT,'本地实验/MAIN2-CANONICAL-LOCAL-SCOREBOARD.tsv'),path.join(DATA,'all-case-metrics.tsv'),
      path.join(versionDir(r),'canonical-vector.tsv'),path.join(versionDir(r),'canonical-score.json'),statusFile,REGISTRY,dash,events]);
    console.log('SCORE_READY '+key(r)+' '+current?.CANONICAL_LOCAL_SCORE+' '+current?.STATUS);
  }
  await Promise.all(workers);await commitQueue;
  console.log('CANONICAL_CAMPAIGN_COMPLETE; no route decision made');
}
main().catch(e=>{console.error(e);process.exitCode=1;});
