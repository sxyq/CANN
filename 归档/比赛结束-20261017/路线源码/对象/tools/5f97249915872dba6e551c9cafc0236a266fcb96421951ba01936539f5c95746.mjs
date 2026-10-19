#!/usr/bin/env node
// Label-free, fixed-suite local scorer. No network and no NPU side effects.
import path from 'node:path';
import {fileURLToPath} from 'node:url';
import {ROOT,DATA,BASE_SHA,REGISTRY,SCORER,SUITE,readTsv,writeTsv,writeJson,json,text,exists,sha,key,safe,versionDir} from './main2-canonical-assets.mjs';

export const mean = a => a.reduce((s,v)=>s+v,0)/a.length;
export function median(a) {const b=[...a].sort((x,y)=>x-y), i=Math.floor(b.length/2);return b.length%2?b[i]:(b[i-1]+b[i])/2;}
export const geo = a => Math.exp(mean(a.map(Math.log)));
export const fmt = v => Number.isFinite(v)?v.toFixed(6):'NA';
export const suite = () => readTsv(path.join(ROOT,'本地实验/CANONICAL-LOCAL-SUITE-V1.tsv'));
export function frozenSuite(rows=suite()) {
  const ids=['C12','C13','C14','C16','C01','C08','C11'];
  const dimensions=['8/8192/BF16','128/16384/FP16','128/16384/BF16','9/32768/BF16','1/128/FP16','8/2048/BF16','1/8192/FP32'];
  if(rows.length!==7 || rows.some((r,i)=>r.CASE_ID!==ids[i] || r.SUITE_VERSION!==SUITE ||
    r.CASE_ROLE!==(i<4?'CORE':'DIAGNOSTIC') || [r.ROWS,r.WIDTH,r.DTYPE].join('/')!==dimensions[i]))throw new Error('FROZEN_SUITE_IDENTITY_MISMATCH');
  return rows;
}
export function stats(file) {
  const result={};
  for(const line of text(file).split(/\r?\n/)) {
    const v=line.split('\t'); if(v.length===2)result[v[0]]=v[1];else if(v.length>=3)result[v[0]+'_'+v[1]]=v[2];
  }
  return result;
}
export function validateRun(r,c,expectedSha) {
  if(r.SCORER_VERSION!==SCORER||r.SUITE_VERSION!==SUITE)throw new Error('INCOMPATIBLE_SCORE_VERSION');
  if(r.SOURCE_SHA!==expectedSha)throw new Error('RUN_SOURCE_IDENTITY_MISMATCH');
  if(r.VALID_RUN!=='YES'||Number(r.RC)!==0||Number(r.BAD)!==0)throw new Error('INVALID_RUN');
  const s=stats(r.STATS_PATH), raw=readTsv(r.RAW_PATH);
  if(s.method!=='DEVICE_EVENT_PRIMARY+HOST_WALL_SECONDARY'||Number(s.warmup)!==45||Number(s.samples_per_block)!==31||
    Number(s.blocks)!==1||Number(s.batch_n)!==1||Number(s.gap_sec)!==0||Number(s.bad)!==0||
    Number(s.rows)!==Number(c.ROWS)||Number(s.width)!==Number(c.WIDTH)||Number(s.dtype)!==Number(c.DTYPE_CODE)||
    Number(s.device)!==Number(r.DEVICE))throw new Error('RAW_PROTOCOL_OR_SHAPE_MISMATCH');
  const samples=raw.map(v=>Number(v.device_us));
  if(samples.length!==31||samples.some(v=>!Number.isFinite(v)||v<=0)||raw.some((v,i)=>Number(v.block)!==1||Number(v.rep)!==i+1))throw new Error('INVALID_RAW_SAMPLES');
  const expectedRunner=sha(path.join(ROOT,'工具/main2-canonical-harness/runner_ref.inc'));
  if(r.HARNESS_SHA!==expectedRunner)throw new Error('HARNESS_IDENTITY_MISMATCH');
  const med=median(samples), avg=mean(samples), mad=median(samples.map(v=>Math.abs(v-med)));
  return {...r,us:med,rawMean:avg,mad,cv:Math.sqrt(mean(samples.map(v=>(v-avg)**2)))/avg,min:Math.min(...samples),max:Math.max(...samples)};
}
export function summarizeRuns(runs,c,expectedSha) {
  const valid=runs.filter(r=>r.VALID_RUN==='YES');
  if(valid.length!==3||new Set(valid.map(r=>r.RUN)).size!==3)throw new Error('THREE_UNIQUE_VALID_RUNS_REQUIRED');
  const values=valid.map(r=>validateRun(r,c,expectedSha)), us=values.map(v=>v.us), avg=mean(us), med=median(us);
  const spread=(Math.max(...us)-Math.min(...us))/avg;
  return {values,us,avg,median:med,spread,cv:Math.sqrt(mean(us.map(v=>(v-avg)**2)))/avg,
    madRatio:median(values.map(r=>r.mad/r.us)),logRange:Math.log(Math.max(...us)/Math.min(...us)),
    quality:spread<=0.10?'GOOD':spread<=0.25?'FAIR':'POOR',throughput:Number(c.ROWS)*Number(c.WIDTH)*1e6/med};
}
export function geometricScore(anchor, candidate) {
  if(anchor.length!==4||candidate.length!==4||[...anchor,...candidate].some(v=>!Number.isFinite(v)||v<=0))throw new Error('COMPLETE_FOUR_CORE_VECTOR_REQUIRED');
  return 100*geo(anchor.map((v,i)=>v/candidate[i]));
}
const qRank={GOOD:0,FAIR:1,POOR:2};
const worse=a=>a.sort((x,y)=>qRank[y]-qRank[x])[0];
export function scoreVectors(core) {
  if(core.length!==4)throw new Error('FOUR_CORE_CASES_REQUIRED');
  const score=geometricScore(core.map(r=>r.anchor.median),core.map(r=>r.candidate.median));
  const gain=core.map(r=>Math.log(r.anchor.median/r.candidate.median));
  const uncertainty=mean(core.map(r=>Math.max(r.anchor.logRange,r.candidate.logRange,r.parent?.logRange||0,
    2*r.anchor.madRatio,2*r.candidate.madRatio,2*(r.parent?.madRatio||0),
    r.parent?Math.abs(Math.log(r.parent.median/r.anchor.median)):0)));
  const low=score/Math.exp(uncertainty),high=score*Math.exp(uncertainty);
  const positiveGain=gain.filter(g=>g>0).reduce((s,g)=>s+g,0);
  const share=positiveGain?Math.max(...gain)/positiveGain:0;
  const leaveOne=gain.map((_,i)=>100*Math.exp(mean(gain.filter((_,j)=>j!==i))));
  const quality=worse(core.map(r=>r.quality));
  const pairedScore=core.every(r=>r.parent)?100*geo(core.map(r=>r.parent.median/r.candidate.median)):null;
  const driftContradiction=score>100 && pairedScore!==null && pairedScore<=100;
  const fragile=score>100 && (share>0.5||Math.min(...leaveOne)<=100||quality==='POOR'||driftContradiction);
  const reliable=low>100 && quality!=='POOR' && !fragile;
  const status=quality==='POOR'?'LOCAL_NEUTRAL':(low>100?'LOCAL_POSITIVE':high<100?'LOCAL_NEGATIVE':'LOCAL_NEUTRAL');
  return {score,low,high,uncertainty,quality,status,fragile,reliable,share,leaveOne,pairedScore,
    consistentGain:gain.every(g=>g>0),coreGainCount:gain.filter(g=>g>0).length};
}

export function scoreOne(reg, anchorRuns) {
  const rows=readTsv(path.join(versionDir(reg),'measurements.tsv'));
  const cells=[], errors=[];
  for(const c of frozenSuite()) {
    try {
      const anchor=summarizeRuns(anchorRuns.filter(r=>r.CASE_ID===c.CASE_ID&&r.SIDE==='anchor'),c,BASE_SHA);
      const candidate=summarizeRuns(rows.filter(r=>r.CASE_ID===c.CASE_ID&&r.SIDE==='candidate'),c,reg.SOURCE_SHA);
      const parent=summarizeRuns(rows.filter(r=>r.CASE_ID===c.CASE_ID&&r.SIDE==='parent'),c,BASE_SHA);
      const cell={c,anchor,candidate,parent,quality:worse([anchor.quality,candidate.quality,parent.quality])};
      cells.push(cell);
    } catch(e) {errors.push(c.CASE_ID+':'+e.message);}
  }
  if(errors.length)return {errors,cells};
  return {...scoreVectors(cells.filter(r=>r.c.CASE_ROLE==='CORE')),cells,errors};
}

export function anchorVector() {
  const anchor={ROUTE:'R31B',REVISION:'V011'}, runs=readTsv(path.join(versionDir(anchor),'measurements.tsv'));
  const result=[];
  for(const c of frozenSuite()) {
    const v=summarizeRuns(runs.filter(r=>r.CASE_ID===c.CASE_ID&&r.SIDE==='anchor'),c,BASE_SHA);
    if(v.values.some(r=>r.FRESH_NPU_RUN!=='YES'))throw new Error('ANCHOR_MUST_BE_FRESH');
    result.push({SCORER_VERSION:SCORER,SUITE_VERSION:SUITE,CASE_ID:c.CASE_ID,CASE_ROLE:c.CASE_ROLE,
      ROWS:c.ROWS,WIDTH:c.WIDTH,DTYPE:c.DTYPE,RUN1_US:fmt(v.us[0]),RUN2_US:fmt(v.us[1]),RUN3_US:fmt(v.us[2]),
      AVG_US:fmt(v.avg),MEDIAN_US:fmt(v.median),THROUGHPUT:fmt(v.throughput),QUALITY:v.quality,
      RUN_CV:fmt(v.cv),WITHIN_RUN_MAD_RATIO:fmt(v.madRatio),DEVICE:[...new Set(v.values.map(r=>r.DEVICE))].join(','),
      FRESH_NPU_RUN:'YES',SOURCE_SHA:BASE_SHA,LOAD_NOTE:v.values.map(r=>r.LOAD_NOTE).join(';'),RAW_PATHS:v.values.map(r=>r.RAW_PATH).join(';')});
  }
  writeTsv(path.join(ROOT,'本地实验/R31B-V011-CANONICAL-LOCAL-VECTOR.tsv'),result);
  return runs;
}

export function scoreboard() {
  const anchorRuns=anchorVector(), registry=readTsv(REGISTRY).filter(r=>r.IMPLEMENTED==='YES');
  const anchorQuality=readTsv(path.join(ROOT,'本地实验/R31B-V011-CANONICAL-LOCAL-VECTOR.tsv')).filter(r=>r.CASE_ROLE==='CORE');
  const rows=[{LOCAL_RANK:'',ROUTE:'R31B',REVISION:'V011',HYPOTHESIS:'FRESH_CANONICAL_ANCHOR',DIRECT_PARENT:'R31B/V010',SOURCE_SHA:BASE_SHA,
    SCORER_VERSION:SCORER,SUITE_VERSION:SUITE,CANONICAL_LOCAL_SCORE:'100.000000',DELTA_VS_R31B_V011:'0.000000',CORE_CASES_COMPLETE:'4/4',
    GOOD_CASES:anchorQuality.filter(c=>c.QUALITY==='GOOD').length,FAIR_CASES:anchorQuality.filter(c=>c.QUALITY==='FAIR').length,POOR_CASES:anchorQuality.filter(c=>c.QUALITY==='POOR').length,
    BUILD:'PASS',CORRECTNESS:'PASS_7_OF_7',OLD_LOCAL_SCORE:'NA',OLD_LOCAL_PROTOCOL:'NA',OFFICIAL_SCORE:'45.16',SCORE_SOURCE:'FRESH_MEASUREMENT',STATUS:'LOCAL_NEUTRAL',
    LOCAL_GAIN_FRAGILE:'NO',RELIABLE_LOCAL_GAIN:'BASELINE_ONLY',EMPIRICAL_LOW:'100.000000',EMPIRICAL_HIGH:'100.000000',CORE_GAIN_COUNT:0,
    PAIRED_DIAGNOSTIC_SCORE:'100.000000',EVIDENCE_PATH:'本地实验/R31B-V011-CANONICAL-LOCAL-VECTOR.tsv',UNSCORED_REASON:'NA'}];
  const allCells=[];
  for(const r of registry) {
    const dir=versionDir(r), build=json(path.join(dir,'build.json')), corr=json(path.join(dir,'correctness.json'));
    let reason='',status='';
    if(r.SOURCE_RECOVERABLE!=='YES')status='UNSCORED_SOURCE_MISSING',reason='No matching recoverable source';
    else if(!build||build.BUILD_STATUS!=='PASS'||build.SOURCE_SHA!==r.SOURCE_SHA)status='UNSCORED_BUILD_BLOCKED',reason=build?.REASON||'Unified build/identity missing';
    else if(r.KNOWN_UNRESOLVED_CORRECTNESS_FAILURE==='YES'||!corr||corr.CORRECTNESS_STATUS!=='PASS'||corr.SOURCE_SHA!==r.SOURCE_SHA)
      status='UNSCORED_CORRECTNESS_BLOCKED',reason=r.KNOWN_UNRESOLVED_CORRECTNESS_FAILURE==='YES'?'Known unresolved global correctness failure':(corr?.REASON||'Unified correctness incomplete');
    let scored=null;
    if(!status) {scored=scoreOne(r,anchorRuns);if(scored.errors.length)status='UNSCORED_MEASUREMENT_INVALID',reason=scored.errors.join(';');}
    const cells=scored?.cells||[],core=cells.filter(r=>r.c.CASE_ROLE==='CORE');
    const result={LOCAL_RANK:'',ROUTE:r.ROUTE,REVISION:r.REVISION,HYPOTHESIS:r.HYPOTHESIS_ID,DIRECT_PARENT:r.DIRECT_PARENT,SOURCE_SHA:r.SOURCE_SHA,
      SCORER_VERSION:SCORER,SUITE_VERSION:SUITE,CANONICAL_LOCAL_SCORE:status?'UNSCORED':fmt(scored.score),
      DELTA_VS_R31B_V011:status?'NA':fmt(scored.score-100),CORE_CASES_COMPLETE:core.length+'/4',
      GOOD_CASES:core.filter(c=>c.quality==='GOOD').length,FAIR_CASES:core.filter(c=>c.quality==='FAIR').length,POOR_CASES:core.filter(c=>c.quality==='POOR').length,
      BUILD:build?.BUILD_STATUS||r.BUILD_STATUS,CORRECTNESS:corr?.CORRECTNESS_STATUS||r.CORRECTNESS_STATUS,
      OLD_LOCAL_SCORE:r.OLD_LOCAL_SCORE,OLD_LOCAL_PROTOCOL:r.OLD_LOCAL_PROTOCOL,OFFICIAL_SCORE:r.OFFICIAL_SCORE,
      SCORE_SOURCE:json(path.join(dir,'raw-reuse.json'))?.SCORE_SOURCE||(status?'NA':'FRESH_MEASUREMENT'),STATUS:status||scored.status,
      LOCAL_GAIN_FRAGILE:status?'NA':scored.fragile?'YES':'NO',RELIABLE_LOCAL_GAIN:status?'NO':scored.reliable?'YES':'NO',
      EMPIRICAL_LOW:status?'NA':fmt(scored.low),EMPIRICAL_HIGH:status?'NA':fmt(scored.high),CORE_GAIN_COUNT:scored?.coreGainCount??'NA',
      PAIRED_DIAGNOSTIC_SCORE:status?'NA':fmt(scored.pairedScore),EVIDENCE_PATH:path.relative(ROOT,dir),UNSCORED_REASON:reason||'NA'};
    rows.push(result);
    if(!status) {
      const perCase=cells.map((v,i)=>({ROUTE:r.ROUTE,REVISION:r.REVISION,SCORER_VERSION:SCORER,SUITE_VERSION:SUITE,CASE_ID:v.c.CASE_ID,CASE_ROLE:v.c.CASE_ROLE,
        ROWS:v.c.ROWS,WIDTH:v.c.WIDTH,DTYPE:v.c.DTYPE,PATH:v.c.EXPECTED_PATH,
        ANCHOR_US:fmt(v.anchor.median),CANDIDATE_US:fmt(v.candidate.median),CANDIDATE_AVG_US:fmt(v.candidate.avg),
        RUN1_US:fmt(v.candidate.us[0]),RUN2_US:fmt(v.candidate.us[1]),RUN3_US:fmt(v.candidate.us[2]),
        LATENCY_RATIO:fmt(v.candidate.median/v.anchor.median),SPEEDUP:fmt(v.anchor.median/v.candidate.median),
        ANCHOR_THROUGHPUT:fmt(v.anchor.throughput),CANDIDATE_THROUGHPUT:fmt(v.candidate.throughput),
        PAIRED_PARENT_US:fmt(v.parent.median),PAIRED_LATENCY_RATIO:fmt(v.candidate.median/v.parent.median),
        PAIRED_PARENT_DRIFT:fmt(v.parent.median/v.anchor.median),PAIRED_FASTER_RUNS:v.candidate.us.filter((u,j)=>u<v.parent.us[j]).length,
        QUALITY:v.quality,WITHIN_RUN_MAD_RATIO:fmt(v.candidate.madRatio),RUN_CV:fmt(v.candidate.cv),
        LEAVE_ONE_CORE_OUT_SCORE:v.c.CASE_ROLE==='CORE'?fmt(scored.leaveOne[i]):'NOT_IN_SCORE',
        RAW_PATHS:v.candidate.values.map(r=>r.RAW_PATH).join(';')}));
      writeTsv(path.join(dir,'canonical-vector.tsv'),perCase);
      writeJson(path.join(dir,'canonical-score.json'),result);
      allCells.push(...perCase);
    }
  }
  const scored=rows.filter(r=>Number.isFinite(Number(r.CANONICAL_LOCAL_SCORE))).sort((a,b)=>Number(b.CANONICAL_LOCAL_SCORE)-Number(a.CANONICAL_LOCAL_SCORE)||key(a).localeCompare(key(b)));
  scored.forEach((r,i)=>r.LOCAL_RANK=i+1);
  if(scored[0])scored[0].STATUS='LOCAL_CHAMPION';
  writeTsv(path.join(ROOT,'本地实验/MAIN2-CANONICAL-LOCAL-SCOREBOARD.tsv'),[...scored,...rows.filter(r=>r.CANONICAL_LOCAL_SCORE==='UNSCORED')]);
  writeTsv(path.join(DATA,'all-case-metrics.tsv'),allCells);
  console.log(JSON.stringify({implemented:registry.length,scored:scored.length-1,unscored:rows.filter(r=>r.CANONICAL_LOCAL_SCORE==='UNSCORED').length,
    champion:scored[0],top10:scored.slice(0,10).map(r=>({version:key(r),score:r.CANONICAL_LOCAL_SCORE,fragile:r.LOCAL_GAIN_FRAGILE}))},null,2));
  return rows;
}

if(process.argv[1]&&path.resolve(process.argv[1])===fileURLToPath(import.meta.url)) {
  if(process.argv[2]==='anchor') {anchorVector();console.log('Fresh canonical anchor: 100.000000');}
  else if(process.argv[2]==='scoreboard')scoreboard();
  else {console.error('Usage: node 工具/main2-canonical-scorer.mjs anchor|scoreboard');process.exitCode=2;}
}
