#!/usr/bin/env node
// Derived asset consolidation only: no Kernel, NPU, Online, or Git mutation.
import fs from 'node:fs';
import path from 'node:path';
import {fileURLToPath} from 'node:url';
import {spawnSync} from 'node:child_process';
import {ROOT,DATA,REGISTRY,BASE_SHA,SCORER,SUITE,readTsv,writeTsv,writeJson,write,json,text,exists,sha,key,versionDir,git} from './main2-canonical-assets.mjs';
import {median,fmt,scoreOne} from './main2-canonical-scorer.mjs';

const SCOREBOARD=path.join(ROOT,'本地实验/MAIN2-CANONICAL-LOCAL-SCOREBOARD.tsv');
const SUMMARY=path.join(ROOT,'技术路线/MAIN2-ROUTE-SCORE-SUMMARY.tsv');
const HISTORY='研究/Local-Judge-History';
const MAIN='研究/主代理/MAIN-2';
const MANIFEST=path.join(ROOT,'归档/MAIN2-LOCAL-ASSET-CLEANUP-MANIFEST.tsv');
const finite=v=>v!==''&&v!=='NA'&&v!=null&&Number.isFinite(Number(v));
const rel=f=>path.relative(ROOT,path.resolve(f));
const planningStatus=()=>{
  const scheduler=text(path.join(ROOT,'调度/当前任务.tsv'));
  const m=scheduler.match(/^ROW-OCCUPANCY-CHAMPION-X\t[^\n]*?MAIN_SELECTED=([^;\t]+)/m);
  return m ? `PLANNING_SELECTED=${m[1]}` : 'MAIN_SELECTED=NONE';
};

export function direction(row) {
  if(!row || row.CANONICAL_LOCAL_SCORE==='UNSCORED')return 'UNSCORED';
  if(Number(row.POOR_CASES)>0)return 'LOCAL_NEUTRAL';
  if(Number(row.EMPIRICAL_LOW)>100)return 'LOCAL_POSITIVE';
  if(Number(row.EMPIRICAL_HIGH)<100)return 'LOCAL_NEGATIVE';
  return 'LOCAL_NEUTRAL';
}
export function trend(rows) {
  const usable=rows.filter(r=>finite(r.CANONICAL_LOCAL_SCORE)&&Number(r.POOR_CASES)===0);
  if(usable.length<2)return 'INSUFFICIENT_DATA';
  const directions=usable.map(direction);
  if(directions.every(d=>d==='LOCAL_POSITIVE'))return 'IMPROVING';
  if(directions.every(d=>d==='LOCAL_NEGATIVE'))return 'REGRESSING';
  if(directions.every(d=>d==='LOCAL_NEUTRAL'))return 'PLATEAU';
  return 'MIXED';
}
export function loadClosed() {
  const registry=readTsv(REGISTRY),implemented=registry.filter(r=>r.IMPLEMENTED==='YES');
  const open=implemented.filter(r=>r.SCORE_EVENT_CLOSED!=='YES');
  if(open.length)throw new Error('CAMPAIGN_EVENTS_STILL_OPEN: '+open.map(key).join(','));
  const board=readTsv(SCOREBOARD);
  if(new Set(board.map(key)).size!==board.length)throw new Error('DUPLICATE_SCOREBOARD_KEY');
  if(board.length!==implemented.length+1)throw new Error('SCOREBOARD_CENSUS_MISMATCH');
  return {registry,implemented,board};
}
export function summary() {
  const {registry,implemented,board}=loadClosed();
  const routes=[...new Set(registry.map(r=>r.ROUTE))].sort();
  const out=routes.map(route=>{
    const revisions=implemented.filter(r=>r.ROUTE===route);
    const rows=board.filter(r=>r.ROUTE===route).sort((a,b)=>a.REVISION.localeCompare(b.REVISION));
    const scored=rows.filter(r=>finite(r.CANONICAL_LOCAL_SCORE));
    const best=[...scored].sort((a,b)=>Number(b.CANONICAL_LOCAL_SCORE)-Number(a.CANONICAL_LOCAL_SCORE))[0];
    const official=revisions.map(r=>r.OFFICIAL_SCORE).filter(finite).map(Number);
    const states=[...new Set(registry.filter(r=>r.ROUTE===route).map(r=>r.CURRENT_STATUS))];
    const sameParent=new Map();
    for(const r of revisions) {const a=sameParent.get(r.PARENT_SHA)||[];a.push(r.REVISION);sameParent.set(r.PARENT_SHA,a);}
    return {ROUTE:route,IMPLEMENTED_REVISIONS:revisions.length,SCORED_REVISIONS:scored.length,
      UNSCORED_REVISIONS:rows.length-scored.length,BEST_LOCAL_SCORE:best?.CANONICAL_LOCAL_SCORE||'NA',
      BEST_REVISION:best?.REVISION||'NONE',MEDIAN_LOCAL_SCORE:scored.length?fmt(median(scored.map(r=>Number(r.CANONICAL_LOCAL_SCORE)))):'NA',
      POSITIVE_COUNT:rows.filter(r=>direction(r)==='LOCAL_POSITIVE').length,
      NEUTRAL_COUNT:rows.filter(r=>direction(r)==='LOCAL_NEUTRAL').length,
      NEGATIVE_COUNT:rows.filter(r=>direction(r)==='LOCAL_NEGATIVE').length,
      POINT_ABOVE_ANCHOR_COUNT:scored.filter(r=>Number(r.CANONICAL_LOCAL_SCORE)>100).length,
      FRAGILE_GAIN_COUNT:scored.filter(r=>r.LOCAL_GAIN_FRAGILE==='YES').length,
      RELIABLE_GAIN_COUNT:scored.filter(r=>r.RELIABLE_LOCAL_GAIN==='YES').length,
      LATEST_REVISION:rows.at(-1)?.REVISION||'NONE',LATEST_DIRECTION:direction(rows.at(-1)),
      OFFICIAL_BEST:official.length?fmt(Math.max(...official)):'NA',CURRENT_ROUTE_STATUS:states.join(';'),
      ROUTE_TREND:trend(rows),TREND_BASIS:'Quality-qualified results only; POOR is not proof of plateau/regression',
      NUMERIC_SEQUENCE:rows.map(r=>r.REVISION+'='+r.CANONICAL_LOCAL_SCORE).join(';')||'RESEARCH_ONLY',
      REAL_PARENT_GROUPS:[...sameParent].map(([parent,revs])=>parent+':'+revs.join(',')).join(';')||'NONE',
      SCORER_VERSION:SCORER,SUITE_VERSION:SUITE,
      ROUTE_DECISION:route==='ROW-OCCUPANCY-CHAMPION-X'
        ? `${planningStatus()};V001_LOCAL_REJECTED;H2_DIRECT_V011_SIBLING_UNDER_REVIEW`
        : 'UNCHANGED;MAIN_SELECTED=NONE',
      EVIDENCE_PATH:'本地实验/MAIN2-CANONICAL-LOCAL-SCOREBOARD.tsv;技术路线/MAIN2-CANONICAL-ROUTE-REGISTRY.tsv'};
  });
  writeTsv(SUMMARY,out);
  return out;
}
export function audit({save=false}={}) {
  const {registry,implemented,board}=loadClosed(),issues=[];
  const anchor=readTsv(path.join(versionDir({ROUTE:'R31B',REVISION:'V011'}),'measurements.tsv'));
  const byKey=new Map(board.map(r=>[key(r),r]));
  const coverage=[];
  for(const r of implemented) {
    const d=versionDir(r),row=byKey.get(key(r));
    if(!row) {issues.push(key(r)+':MISSING_SCOREBOARD_ROW');continue;}
    if(!exists(r.CANONICAL_SOURCE_PATH)||sha(r.CANONICAL_SOURCE_PATH)!==r.SOURCE_SHA)issues.push(key(r)+':SOURCE_SHA');
    const attempts=readTsv(path.join(d,'attempts.tsv'));
    const failed=attempts.filter(a=>a.VALID_PAIR==='NO');
    const correctnessFailure=failed.some(a=>/BAD=[1-9][0-9]*|RC=(?!0(?:,|;|$))[^,;]+/.test(a.DETAIL));
    if(correctnessFailure&&row.STATUS!=='UNSCORED_CORRECTNESS_BLOCKED')issues.push(key(r)+':TIMING_CORRECTNESS_FAILURE_NEEDS_EXPLICIT_CLASSIFICATION');
    if(finite(row.CANONICAL_LOCAL_SCORE)) {
      const result=scoreOne(r,anchor);
      if(result.errors.length||fmt(result.score)!==row.CANONICAL_LOCAL_SCORE)issues.push(key(r)+':SCORE_RECOMPUTATION');
      if(correctnessFailure)issues.push(key(r)+':CORRECTNESS_FAILURE_MUST_NOT_BE_SCORED');
      if(r.KNOWN_UNRESOLVED_CORRECTNESS_FAILURE==='YES')issues.push(key(r)+':KNOWN_FAILURE_SCORED');
    }
    const b=json(path.join(d,'build.json')),corr=json(path.join(d,'correctness.json'));
    coverage.push({VERSION:key(r),SOURCE_SHA:r.SOURCE_SHA,BUILD:b?.BUILD_STATUS||'NA',
      CORRECTNESS:corr?.CORRECTNESS_STATUS||'NA',ATTEMPTS:attempts.length,INVALID_ATTEMPTS:failed.length,
      TIMING_CORRECTNESS_OR_RUNTIME_FAILURE:correctnessFailure?'YES':'NO',SCORE:row.CANONICAL_LOCAL_SCORE,
      SCORE_SOURCE:row.SCORE_SOURCE,STATUS:row.STATUS,REASON:row.UNSCORED_REASON,
      RAW_RUN_ROWS:readTsv(path.join(d,'measurements.tsv')).length,EVIDENCE:rel(d)});
  }
  const result={SCORER_VERSION:SCORER,SUITE_VERSION:SUITE,ROUTES:new Set(registry.map(r=>r.ROUTE)).size,
    IMPLEMENTED:implemented.length,SCORED:coverage.filter(r=>finite(r.SCORE)).length,
    UNSCORED:coverage.filter(r=>!finite(r.SCORE)).length,ANCHOR_SOURCE_SHA:BASE_SHA,
    ANCHOR_FRESH:anchor.length===21&&anchor.every(r=>r.FRESH_NPU_RUN==='YES'),ISSUES:issues,
    KERNEL_PERFORMANCE_REVISIONS_CREATED:0,DIRECT_ONLINE_SUBMISSIONS:0,
    MAIN_SELECTED:planningStatus().replace(/^PLANNING_SELECTED=/,'')};
  if(save) {writeTsv(path.join(DATA,'coverage-audit.tsv'),coverage);writeJson(path.join(DATA,'coverage-audit.json'),result);}
  return result;
}

export function sourceReview() {
  const {implemented,board}=loadClosed();
  const notes=json(path.join(ROOT,'研究/MAIN2-CANONICAL-SOURCE-REVIEW-NOTES.json'));
  if(!notes)throw new Error('MAIN_SOURCE_REVIEW_NOTES_REQUIRED');
  const ranked=board.filter(r=>r.ROUTE!=='R31B'&&finite(r.CANONICAL_LOCAL_SCORE));
  const topRoutes=[...new Set(ranked.map(r=>r.ROUTE))].slice(0,3);
  const selected=[...new Map([...ranked.slice(0,5),...topRoutes.map(route=>ranked.find(r=>r.ROUTE===route))].map(r=>[key(r),r])).values()];
  const metrics=readTsv(path.join(DATA,'all-case-metrics.tsv'));
  const records=[];
  const lines=['# Main-2 Canonical source-level review','',
    'Scope: five highest-scored Candidate revisions (anchor excluded) and the best representative of each of the three highest-scored routes.',
    'The numeric order is a view of the canonical scoreboard, not causal evidence or a new Parent selection.',
    'Method: exact source/parent diff, reached path, producer/consumer lifetime, and the fixed Core vector. No Kernel was edited.',
    'Directive priority: the task requires this written planning evidence; the quick-review skill is used for its targeted review method only.',
    '', '## Interpretation boundary','',
    'All listed ratios are Candidate/frozen-fresh-V011 latency; <1 is a numerical gain. Paired-parent drift and POOR quality prevent reliable claims.',
    'An unchanged reached path cannot be credited with the advertised mechanism solely from a score difference.',
    'This is not a comprehensive API/correctness certification. Seven-case preflight and historical global failures remain separately recorded.',
    '', '## Reviews',''];
  for(const r of selected) {
    const n=notes.find(n=>n.VERSION===key(r));
    if(!n||n.SOURCE_SHA!==r.SOURCE_SHA)throw new Error('UNREVIEWED_TOP_SOURCE: '+key(r));
    const reg=implemented.find(v=>key(v)===key(r));
    const parent=path.join(DATA,'sources',reg.PARENT_SHA+'.asc');
    if(!exists(parent)||sha(parent)!==reg.PARENT_SHA||sha(reg.CANONICAL_SOURCE_PATH)!==r.SOURCE_SHA)throw new Error('REVIEW_SOURCE_IDENTITY');
    const diff=spawnSync('git',['-c','core.quotepath=false','diff','--no-index','--unified=8','--',parent,reg.CANONICAL_SOURCE_PATH],{cwd:ROOT,encoding:'utf8',maxBuffer:8*1024*1024});
    if(![0,1].includes(diff.status))throw new Error('SOURCE_DIFF_FAILED');
    const patchFile=path.join(ROOT,'研究/MAIN2-CANONICAL-SOURCE-REVIEW/diffs',r.ROUTE+'__'+r.REVISION+'.patch');
    write(patchFile,diff.stdout||'No byte differences.');
    const cases=metrics.filter(c=>key(c)===key(r)&&c.CASE_ROLE==='CORE');
    const record={...n,DIRECT_PARENT:reg.DIRECT_PARENT,PARENT_SHA:reg.PARENT_SHA,
      WHICH_CORE_CASES_GAIN:cases.filter(c=>Number(c.LATENCY_RATIO)<1).map(c=>c.CASE_ID).join(',')||'NONE',
      WHICH_CORE_CASES_REGRESS:cases.filter(c=>Number(c.LATENCY_RATIO)>1).map(c=>c.CASE_ID).join(',')||'NONE',
      CORE_RATIOS:cases.map(c=>c.CASE_ID+'='+c.LATENCY_RATIO).join(';'),
      LOCAL_SCORE:r.CANONICAL_LOCAL_SCORE,RELIABLE_LOCAL_GAIN:r.RELIABLE_LOCAL_GAIN,DIFF_PATH:rel(patchFile)};
    records.push(record);
    lines.push('### '+key(r),'');
    for(const [k,v] of Object.entries(record))lines.push('- '+k+': '+v);
    lines.push('', '| Core case | Anchor us | Candidate us | C/A ratio | Paired-parent drift | Quality | Without this case score |',
      '|---|---:|---:|---:|---:|---|---:|');
    for(const c of cases)lines.push('| '+[c.CASE_ID,c.ANCHOR_US,c.CANDIDATE_US,c.LATENCY_RATIO,c.PAIRED_PARENT_DRIFT,c.QUALITY,c.LEAVE_ONE_CORE_OUT_SCORE].join(' | ')+' |');
    lines.push('');
  }
  writeTsv(path.join(ROOT,'研究/MAIN2-CANONICAL-SOURCE-REVIEW.tsv'),records);
  write(path.join(ROOT,'研究/MAIN2-CANONICAL-SOURCE-REVIEW.md'),lines.join('\n'));
  const topKeys=new Set(board.filter(r=>finite(r.CANONICAL_LOCAL_SCORE)).slice(0,10).map(key));
  writeTsv(path.join(DATA,'top-core-ratios.tsv'),metrics.filter(c=>topKeys.has(key(c))&&c.CASE_ROLE==='CORE'));
  return {REVIEWED_CANDIDATE_VERSIONS:selected.map(key),TOP_ROUTES:topRoutes,MAIN_SELECTED:'NONE'};
}

const archiveName=n=>/^(?:LOCAL-JUDGE|LOCAL-CASE|C13-FORENSIC|CALIBRATION-CANDIDATES|MAIN2-UNIFIED-LOCAL|MAIN2-(?:ALL-LOCAL-REVISIONS|REVISION-TREE|BASELINE-CALIBRATION|BASELINE-NORMALIZED|FRESH-BASELINE-CALIBRATION|FRESH-NORMALIZED|LOCAL-JUDGE-RECONCILIATION|NEXT-TRACK-B-PLANNING-PACK|OFFICIAL-CALIBRATION-SET)|ONLINE-CALIBRATION-CANDIDATES|PERFORMANCE-NEXT-WAVE-GATE|R31B-V011-(?:FRESH-LOCAL-BASELINE|LOCAL-BASELINE|LOCAL-VECTOR))/.test(n)||n==='本地评分器校准.md';
export function manifest() {
  loadClosed();
  if(exists(MANIFEST))throw new Error('MANIFEST_ALREADY_EXISTS; preserve pre-cleanup evidence');
  const rows=[];
  const add=(p,type,owner,rev,action,reason,repro,replacement='NA')=>{
    const full=path.join(ROOT,p),stat=fs.statSync(full);
    rows.push({PATH:p,TYPE:type,OWNER_ROUTE:owner,REVISION:rev,ACTION:action,REASON:reason,
      CAN_REPRODUCE:repro,CANONICAL_REPLACEMENT:replacement,SHA256:stat.isFile()?sha(full):'DIRECTORY_RECURSIVE_KEEP',
      BYTES:stat.isFile()?stat.size:'DIRECTORY',UNRESOLVED:'NO',MANIFEST_STAGE:'BEFORE_CLEANUP'});
  };
  const canonical=['技术路线/MAIN2-CANONICAL-ROUTE-REGISTRY.tsv','技术路线/MAIN2-CANONICAL-REVISION-TREE.md',
    '技术路线/MAIN2-ROUTE-SCORE-SUMMARY.tsv','本地实验/CANONICAL-LOCAL-SUITE-V1.tsv','本地实验/CANONICAL-LOCAL-SCORER-V1.md',
    '本地实验/R31B-V011-CANONICAL-LOCAL-VECTOR.tsv','本地实验/MAIN2-CANONICAL-LOCAL-SCOREBOARD.tsv',
    '本地实验/MAIN2-CANONICAL-V1','工具/main2-canonical-harness','工具/main2-canonical-assets.mjs',
    '工具/main2-canonical-benchmark.mjs','工具/main2-canonical-campaign.mjs','工具/main2-canonical-scorer.mjs',
    '工具/main2-canonical-scorer.test.mjs','工具/main2-canonical-consolidate.mjs','工具/main2-canonical-consolidate.test.mjs',
    '研究/MAIN2-CANONICAL-SOURCE-REVIEW.md','研究/MAIN2-CANONICAL-SOURCE-REVIEW.tsv',
    '研究/MAIN2-CANONICAL-SOURCE-REVIEW-NOTES.json','研究/MAIN2-CANONICAL-SOURCE-REVIEW',
    '调度/MAIN2-CANONICAL-DASHBOARD.md'];
  for(const p of canonical)add(p,'CANONICAL_ASSET','MAIN-2','ALL','KEEP','Authoritative or required reproducibility asset','YES');
  // Archive superseded derived tables/tools; raw dependencies stay at proven paths.
  for(const entry of fs.readdirSync(path.join(ROOT,MAIN),{withFileTypes:true})) {
    const p=MAIN+'/'+entry.name;
    if(entry.isFile()&&archiveName(entry.name))add(p,'EXPERIMENTAL_PREDICTOR_DERIVED','MAIN-2','V1-V4','ARCHIVE',
      'EXPERIMENTAL / NOT_CANONICAL_SCORE; superseded analysis retained byte-for-byte','YES',HISTORY+'/derived/'+entry.name);
    else add(p,entry.isDirectory()?'HISTORICAL_EVIDENCE_TREE':'HISTORICAL_FACT','MAIN-2','HISTORICAL','KEEP',
      'Raw, source identity, Official census, provenance or referenced historical fact; indexed in archive','PRESERVED_NOT_REGENERATED');
  }
  for(const n of ['main2-local-judge.mjs','main2-local-judge-v2.mjs','main2-local-judge-v3.mjs','main2-local-judge-v4.mjs'])
    add('工具/'+n,'EXPERIMENTAL_PREDICTOR_TOOL','MAIN-2','V1-V4','ARCHIVE',
      'Remove experimental predictor from active tool directory; preserve exact bytes','YES',HISTORY+'/tools/'+n);
  // Preserve full dashboards before replacing their front pages with canonical links.
  for(const p of [MAIN+'/CAMPAIGN-STATUS.md','研究/主代理/MAIN-2-W2/DASHBOARD.md','研究/主代理/MAIN-2-W2/campaign-status.md']) {
    if(!exists(path.join(ROOT,p)))continue;
    const existing=rows.findIndex(r=>r.PATH===p);if(existing>=0)rows.splice(existing,1);
    add(p,'SUPERSEDED_DASHBOARD','MAIN-2','HISTORICAL','CANONICALIZE',
      'Archive exact historical front page; replace active page with canonical-dashboard link','YES',
      HISTORY+'/dashboards/'+(p.includes('MAIN-2-W2/')?'MAIN-2-W2-':'MAIN-2-')+path.basename(p));
  }
  // Permanent Official/source/revision facts outside Main-2 control remain in place.
  for(const p of ['线上结果','本地实验','归档/历史控制文件','归档/历史工作区'])if(exists(path.join(ROOT,p)))
    add(p,'OUTSIDE_CLEANUP_SCOPE_TREE','SHARED_READ_ONLY','ALL','KEEP','No deletion or rewrite of other routes, Official facts or historical evidence','PRESERVED_NOT_REGENERATED');
  writeTsv(MANIFEST,rows);
  return {MANIFEST:rel(MANIFEST),GROUPS:rows.length,ACTIONS:countBy(rows,'ACTION'),DELETE_SAFE:0,
    NOTE:'Counts are manifest entries/recursive asset groups, not individual raw files. No deletion is approved.'};
}
function countBy(rows,field) {const c={};for(const r of rows)c[r[field]]=(c[r[field]]||0)+1;return c;}
export function verifyCleanup() {
  const rows=readTsv(MANIFEST),result=[];
  if(!rows.length)throw new Error('MANIFEST_REQUIRED_BEFORE_CLEANUP');
  for(const r of rows) {
    const moved=['ARCHIVE','CANONICALIZE'].includes(r.ACTION);
    const f=path.join(ROOT,moved?r.CANONICAL_REPLACEMENT:r.PATH);
    const ok=exists(f)&&(r.SHA256==='DIRECTORY_RECURSIVE_KEEP'||sha(f)===r.SHA256);
    result.push({PATH:r.PATH,ACTION:r.ACTION,DESTINATION:rel(f),SHA_VERIFIED:ok?'YES':'NO',
      ACTIVE_REDIRECT_EXISTS:r.ACTION==='CANONICALIZE'?(exists(path.join(ROOT,r.PATH))?'YES':'NO'):'NOT_APPLICABLE'});
  }
  writeTsv(path.join(ROOT,'归档/MAIN2-LOCAL-ASSET-CLEANUP-RESULT.tsv'),result);
  return {GROUPS:rows.length,ACTIONS:countBy(rows,'ACTION'),VERIFIED:result.filter(r=>r.SHA_VERIFIED==='YES').length,
    UNRESOLVED:result.filter(r=>r.SHA_VERIFIED!=='YES'||r.ACTIVE_REDIRECT_EXISTS==='NO').length,DELETED_SAFE:0};
}

if(process.argv[1]&&path.resolve(process.argv[1])===fileURLToPath(import.meta.url)) {
  const cmd=process.argv[2];
  if(cmd==='summary')console.log(JSON.stringify({routes:summary().length},null,2));
  else if(cmd==='audit')console.log(JSON.stringify(audit({save:process.argv.includes('--save')}),null,2));
  else if(cmd==='manifest')console.log(JSON.stringify(manifest(),null,2));
  else if(cmd==='source-review')console.log(JSON.stringify(sourceReview(),null,2));
  else if(cmd==='verify-cleanup')console.log(JSON.stringify(verifyCleanup(),null,2));
  else {console.error('Usage: summary | audit [--save] | source-review | manifest | verify-cleanup');process.exitCode=2;}
}
