#!/usr/bin/env node
// Main-2 evidence census. Never modifies a route worktree or a Kernel.
import fs from 'node:fs';
import path from 'node:path';
import crypto from 'node:crypto';
import {execFileSync} from 'node:child_process';
import {fileURLToPath} from 'node:url';

export const ROOT = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
export const BASE_SHA = 'a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3';
export const DATA = path.join(ROOT, '本地实验/MAIN2-CANONICAL-V1');
export const REGISTRY = path.join(ROOT, '技术路线/MAIN2-CANONICAL-ROUTE-REGISTRY.tsv');
export const SCORER = 'MAIN2-CANONICAL-V1';
export const SUITE = 'CANONICAL_LOCAL_SUITE_V1';
export const exists = f => fs.existsSync(f);
export const sha = f => crypto.createHash('sha256').update(fs.readFileSync(f)).digest('hex');
export const text = f => exists(f) ? fs.readFileSync(f, 'utf8') : '';
export function json(f) { try { return JSON.parse(text(f)); } catch { return null; } }
export const key = r => (r.ROUTE || r.ROUTE_NAME) + '/' + r.REVISION;
export const safe = r => (r.ROUTE || r.ROUTE_NAME) + '__' + r.REVISION;
export const versionDir = r => path.join(DATA, 'versions', safe(r));
export function readTsv(f) {
  const lines = text(f).replace(/^\uFEFF/, '').split(/\r?\n/).filter(Boolean);
  if (!lines.length) return [];
  const h = lines.shift().split('\t');
  return lines.map(l => { const v = l.split('\t'); return Object.fromEntries(h.map((k,i)=>[k,v[i]??''])); });
}
export function write(f, value) {
  fs.mkdirSync(path.dirname(f), {recursive:true});
  fs.writeFileSync(f, value.endsWith('\n') ? value : value+'\n');
}
export function writeJson(f, value) { write(f, JSON.stringify(value,null,2)); }
export function writeTsv(f, rows, headers = Object.keys(rows[0] || {})) {
  const cell = v => v == null || v === '' || (typeof v==='number' && !Number.isFinite(v)) ? 'NA' : String(v).replace(/[\r\n\t]+/g,' ');
  write(f, [headers.join('\t'), ...rows.map(r=>headers.map(h=>cell(r[h])).join('\t'))].join('\n'));
}
export function git(args, cwd=ROOT) {
  try { return execFileSync('git',['-c','core.quotepath=false',...args],{cwd,encoding:'utf8',maxBuffer:64*1024*1024,stdio:['ignore','pipe','pipe']}).trimEnd(); }
  catch { return ''; }
}
const pick = (o, ...fields) => fields.map(k=>o?.[k]).find(v=>v!==undefined && v!==null && v!=='' && v!=='NA');
const str = v => typeof v === 'object' ? JSON.stringify(v) : v;
export function worktrees() {
  return git(['worktree','list','--porcelain']).split(/\n\n/).filter(Boolean).map(b=>({
    root:b.match(/^worktree (.+)$/m)?.[1], head:b.match(/^HEAD (.+)$/m)?.[1], branch:b.match(/^branch (.+)$/m)?.[1]
  })).filter(r=>r.root && exists(r.root));
}

function discoverScope() {
  const scopeFiles = ['研究/主代理/MAIN-2/初始化报告.md','研究/主代理/MAIN-2/CAMPAIGN-STATUS.md',
    '研究/主代理/MAIN-2-W2/campaign-status.md','归档/历史控制文件/main2-r2-route-registry.md','调度/当前任务.tsv'];
  const found = new Map();
  const add = (s,f) => {
    for(const r of s.match(/\b[A-Z][A-Z0-9]*(?:-[A-Z0-9]+)+\b/g)||[]) {
      if(!r.endsWith('-X') && r!=='EPI-X-FRESH') continue;
      if(!found.has(r)) found.set(r,new Set()); found.get(r).add(f);
    }
  };
  for(const f of scopeFiles) {
    for(const line of text(path.join(ROOT,f)).split('\n')) {
      if(f.endsWith('当前任务.tsv')) { if(line.includes('OWNER=MAIN-2')) add(line.split('\t')[0],f); }
      else if(f.endsWith('初始化报告.md')) { if(/^##\s+3\.\d+\s+/.test(line)) add(line,f); }
      else if(f.endsWith('main2-r2-route-registry.md')) {
        if(/^\s*##/.test(line)) add(line,f);
        else if(/^\s*\|/.test(line)) add(line.split('|')[1]||'',f);
      } else if(/^\s*\|/.test(line)) add(line,f);
    }
  }
  return found;
}

function relevantFiles(wts, scope) {
  const names=[...scope.keys()]; const files=new Set();
  for(const w of wts) {
    let list='';
    try { list=execFileSync('rg',['--files','--hidden','-g','!.git','-g','!node_modules','-g','!worktrees',
      '-g','!server_runs','-g','!build*','-g','!CMakeFiles','-g','*.asc','-g','source-meta.json',
      '-g','local-result.json','-g','REVISION*.md','-g','NO_UB_BUDGET.md'],{cwd:w.root,encoding:'utf8',maxBuffer:64*1024*1024}); } catch {}
    for(const f of list.split('\n')) if(f && names.some(n=>f.includes(n))) files.add(path.join(w.root,f));
  }
  return [...files].sort();
}
function infer(file, names) {
  for(const name of names) {
    const at=file.indexOf('/'+name+'/');
    if(at>=0) {
      const rev=file.slice(at+name.length+2).match(/(?:^|\/)(V\d{3})(?:\/|[._-]|$)/)?.[1];
      if(rev) return {ROUTE:name,REVISION:rev};
    }
    const escaped=name.replace(/[.*+?^${}()|[\]\\]/g,'\\$&');
    const m=file.match(new RegExp(escaped+'(?:__|-)(V\\d{3})(?:[/_.-]|$)'));
    if(m) return {ROUTE:name,REVISION:m[1]};
  }
  return null;
}
function priority(file) {
  if(file.includes('/worktrees/w2/m2/') && file.includes('/本地实验/')) return 100;
  if(file.includes('/cann-w2-m2-ub/') && file.includes('/UB-LIFETIME-SAFE-CHAMPION-X/V001/')) return 95;
  if(file.startsWith(ROOT+'/本地实验/')) return 85;
  if(file.startsWith(ROOT+'/线上结果/')) return 80;
  if(file.includes('/本地实验/')) return 70;
  if(file.includes('/source-cache/')) return 65;
  if(file.includes('/stages/')) return 60;
  return 40;
}

function buildInventory() {
  const wts=worktrees(), scope=discoverScope(), names=[...scope.keys()].sort((a,b)=>b.length-a.length);
  const files=relevantFiles(wts,scope), items=new Map(), sources=[], anomalies=[];
  const add=id=>{ if(!items.has(key(id))) items.set(key(id),{...id,meta:[],local:[],files:[],ledgers:[]}); return items.get(key(id)); };
  // Ledgers add evidence-backed identities, never a hand-maintained memory list.
  for(const wt of [wts.find(w=>w.root===ROOT),...wts.filter(w=>w.root!==ROOT)].filter(Boolean)) {
    for(const r of readTsv(path.join(wt.root,'技术路线/全版本记录.tsv'))) {
      if(scope.has(r.ROUTE) && /^V\d{3}$/.test(r.REVISION)) add(r).ledgers.push({...r,_file:path.join(wt.root,'技术路线/全版本记录.tsv')});
    }
  }
  for(const file of files) {
    const id=infer(file,names); if(!id) continue;
    const item=add(id); item.files.push(file);
    if(file.endsWith('source-meta.json')) { const data=json(file); if(data)item.meta.push({file,data}); }
    if(file.endsWith('local-result.json')) { const data=json(file); if(data)item.local.push({file,data}); }
    if(file.endsWith('.asc') && !/(?:runner|shim|main\.asc)/i.test(path.basename(file))) {
      const sourceText=text(file);
      if(sourceText.includes('run_kernel') && sourceText.includes('__aicore__')) sources.push({...id,file,sha:sha(file)});
    }
  }
  // Read frozen historical route branches through Git objects, never revive their
  // private contexts/worktrees. W1 snapshots are not all checked out on server3.
  const refs=git(['for-each-ref','--format=%(refname)','refs/remotes/origin/m2',
    'refs/remotes/origin/exp/main2-r2-epilogue-fuse','refs/remotes/origin/exp/main2-r2-sched-champion']).split('\n').filter(Boolean);
  const seenObjects=new Set();
  for(const ref of refs) {
    const entries=git(['ls-tree','-r',ref]).split('\n');
    for(const entry of entries) {
      const match=entry.match(/^\d+ blob ([a-f0-9]+)\t(.+)$/); if(!match)continue;
      const [,blob,relative]=match;const id=infer('/'+relative,names);if(!id)continue;
      const isMeta=relative.endsWith('source-meta.json'),isLocal=relative.endsWith('local-result.json');
      const isSource=relative.endsWith('.asc')&&!/(?:runner|shim|main\.asc)/i.test(path.basename(relative));
      if(!isMeta&&!isLocal&&!isSource)continue;
      const marker=key(id)+'/'+blob;if(seenObjects.has(marker))continue;seenObjects.add(marker);
      let content;try{content=execFileSync('git',['cat-file','blob',blob],{cwd:ROOT,maxBuffer:4*1024*1024});}catch{continue;}
      const file='git:'+ref+':'+relative;const item=add(id);item.files.push(file);
      if(isMeta||isLocal) {try{const data=JSON.parse(content.toString());(isMeta?item.meta:item.local).push({file,data});}catch{}}
      if(isSource && content.includes('__aicore__') && content.includes('run_kernel'))
        sources.push({...id,file,sha:crypto.createHash('sha256').update(content).digest('hex'),blob,ref,relative});
    }
  }
  const official=readTsv(path.join(ROOT,'研究/主代理/MAIN-2/ALL-OFFICIAL-RESULTS.tsv'));
  // The old census is only an additional pointer; every source is rehashed below.
  const old=readTsv(path.join(ROOT,'研究/主代理/MAIN-2/MAIN2-ALL-LOCAL-REVISIONS.tsv'));
  for(const r of old) if(scope.has(r.ROUTE) && /^V\d{3}$/.test(r.REVISION)) add(r);
  const rows=[];
  for(const item of [...items.values()].sort((a,b)=>key(a).localeCompare(key(b)))) {
    item.meta.sort((a,b)=>priority(b.file)-priority(a.file)); item.local.sort((a,b)=>priority(b.file)-priority(a.file));
    const prior=old.find(r=>key(r)===key(item))||{}, ledger=item.ledgers[0]||{}, meta=item.meta[0]?.data||{}, local=item.local[0]?.data||{};
    const off=official.find(r=>key(r)===key(item));
    let expected=off?.SOURCE_SHA || pick(meta,'candidate_source_sha256','local_sha256','source_sha256','source_sha','SOURCE_SHA','candidate_sha256') || prior.SOURCE_SHA || ledger.SOURCE_SHA;
    const available=sources.filter(r=>key(r)===key(item)).sort((a,b)=>priority(b.file)-priority(a.file));
    let source=available.find(r=>r.sha===expected);
    if(!source && typeof expected==='string') {
      const prefix=expected.match(/[a-f0-9]{8,64}/)?.[0];
      if(prefix) source=available.find(r=>r.sha.startsWith(prefix));
    }
    if(!source && available.length) {
      const unique=[...new Set(available.map(r=>r.sha))];
      if(unique.length===1) source=available[0];
      else {
        for(const m of item.meta) {
          const candidates=JSON.stringify(m.data).match(/[a-f0-9]{64}/g)||[];
          source=available.find(r=>candidates.includes(r.sha)); if(source)break;
        }
      }
    }
    let implemented=true;
    const noBudget=item.files.find(f=>f.endsWith('NO_UB_BUDGET.md'));
    if(noBudget && /No kernel change\. No compile\./.test(text(noBudget))) {
      implemented=false; source=null;
      anomalies.push({VERSION:key(item),ISSUE:'OLD_CENSUS_COUNTED_RESEARCH_AS_IMPLEMENTED',RESOLUTION:'IMPLEMENTED=NO; no source/compile exists',EVIDENCE:noBudget});
    }
    if(source && /^[a-f0-9]{64}$/.test(prior.SOURCE_SHA||'') && source.sha!==prior.SOURCE_SHA) {
      anomalies.push({VERSION:key(item),ISSUE:'OLD_SOURCE_SHA_MISMATCH',RESOLUTION:source.sha,EVIDENCE:source.file});
    }
    let parentSha=pick(meta,'parent_source_sha','PARENT_SOURCE_SHA','parent_source_sha256','parent_sha256')||prior.PARENT_SHA||ledger.PARENT_SOURCE_SHA;
    let parent=str(pick(meta,'direct_parent','DIRECT_PARENT','parent_revision')||prior.DIRECT_PARENT||ledger.DIRECT_PARENT||'UNKNOWN');
    if((!parentSha || !/^[a-f0-9]{64}$/.test(parentSha)) && /(?:FROZEN[_ -])?R31B[_ -]V011/.test(parent)) parentSha=BASE_SHA;
    const hypothesis=str(pick(meta,'single_hypothesis','SINGLE_HYPOTHESIS','hypothesis','HYPOTHESIS')||prior.HYPOTHESIS_ID||ledger.HYPOTHESIS||'UNKNOWN');
    let build=implemented ? str(pick(local,'build_status','BUILD_STATUS','build')||prior.BUILD_STATUS||ledger.BUILD_STATUS||'UNKNOWN') : 'NOT_RUN_RESEARCH_ONLY';
    let corr=implemented ? str(pick(local,'correctness_status','CORRECTNESS_STATUS','correctness')||prior.CORRECTNESS_STATUS||ledger.CORRECTNESS_STATUS||'UNKNOWN') : 'NOT_RUN_RESEARCH_ONLY';
    const out=versionDir(item), canonicalBuild=json(path.join(out,'build.json')), canonicalCorr=json(path.join(out,'correctness.json'));
    if(source && canonicalBuild?.SOURCE_SHA===source.sha) build=canonicalBuild.BUILD_STATUS;
    if(source && canonicalCorr?.SOURCE_SHA===source.sha) corr=canonicalCorr.CORRECTNESS_STATUS;
    const sourceSha=source?.sha || (/^[a-f0-9]{64}$/.test(expected||'')?expected:'UNKNOWN');
    const snapshot=source ? path.join(DATA,'sources',source.sha+'.asc') : '';
    const fixedFailure = item.ROUTE==='UB-LIVENESS-X' || item.ROUTE==='UB-LIFETIME-SAFE-CHAMPION-X' ||
      (item.ROUTE==='REDUCE-INVSCALE-X' && item.REVISION==='V001');
    const knownFailure = fixedFailure && /FAIL|PARTIAL|INCOMPLETE/i.test([corr,prior.CORRECTNESS_STATUS,ledger.CORRECTNESS_STATUS].join(';')) || (off && Number(off.PASS_COUNT)<15);
    const sourceWt=source && wts.filter(w=>source.file.startsWith(w.root+'/')).sort((a,b)=>b.root.length-a.root.length)[0];
    const sourceRel=sourceWt&&path.relative(sourceWt.root,source.file);
    const object=source?.blob || (source ? git(['hash-object',source.file]) : '');
    rows.push({
      ROUTE_ID:item.ROUTE,ROUTE_NAME:item.ROUTE,ROUTE:item.ROUTE,ROUTE_CLASS:/CHAMPION|HOTLOOP/.test(item.ROUTE)?'CHAMPION_ARCHITECTURE':'HISTORICAL_MAIN2_OR_SHARED_REFERENCE',
      REVISION:item.REVISION,HYPOTHESIS_ID:hypothesis,DIRECT_PARENT:parent,PARENT_SHA:parentSha||'UNKNOWN',SOURCE_SHA:sourceSha,
      IMPLEMENTED:implemented?'YES':'NO',SOURCE_RECOVERABLE:source?'YES':'NO',BUILDABLE:/^PASS/.test(build)?'YES':'NO',
      CORRECTNESS_KNOWN:/PASS|FAIL|PARTIAL/.test(corr)?'YES':'NO',CORRECTNESS_VALID:knownFailure?'NO':(/^PASS|^\d+\/\d+|^OUTHASH/.test(corr)?'YES':'UNKNOWN'),
      BUILD_STATUS:build,CORRECTNESS_STATUS:corr,KNOWN_UNRESOLVED_CORRECTNESS_FAILURE:knownFailure?'YES':'NO',
      OLD_LOCAL_SCORE:prior.LATENCY_DELTA_PERCENT&&prior.LATENCY_DELTA_PERCENT!=='NA'?prior.LATENCY_DELTA_PERCENT:(ledger.LOCAL_DELTA||'NA'),
      OLD_LOCAL_PROTOCOL:prior.LATENCY_DELTA_PERCENT&&prior.LATENCY_DELTA_PERCENT!=='NA'?'LEGACY_ROUTE_ENGINEERING_DELTA_PERCENT':'LEGACY_ROUTE_SPECIFIC;NOT_CANONICAL_SCORE',
      OLD_LOCAL_VERDICT:prior.LOCAL_VERDICT||ledger.LOCAL_VERDICT||'NA',OFFICIAL_SCORE:off?.OFFICIAL_SCORE||'NA',OFFICIAL_SUBMISSION_ID:off?.SUBMISSION_ID||'NA',
      OFFICIAL_PASS_COUNT:off?.PASS_COUNT||'NA',CURRENT_STATUS:implemented?'PERFORMANCE_FROZEN_PENDING_PLANNING':'RESEARCH_ONLY_NO_IMPLEMENTATION',
      SOURCE_PATH:source?.file||'NA',CANONICAL_SOURCE_PATH:snapshot||'NA',SOURCE_GIT_OBJECT:object||'NA',
      SOURCE_REF:sourceWt?(sourceWt.branch+'@'+sourceWt.head):(source?.ref||'NA'),SOURCE_RELATIVE_PATH:sourceRel||source?.relative||'NA',
      METADATA_PATH:item.meta[0]?.file||'NA',EVIDENCE_PATH:[...new Set([item.meta[0]?.file,item.local[0]?.file,source?.file,noBudget,ledger._file].filter(Boolean))].join(';'),
      SOURCE_CANDIDATES:available.map(r=>r.sha+'@'+r.file).join(';')||'NA',SCOPE_EVIDENCE:[...scope.get(item.ROUTE)].join(';')
    });
  }
  // A source SHA outranks stale text that incorrectly says FROZEN for a sibling.
  const bySha=new Map(rows.filter(r=>r.SOURCE_RECOVERABLE==='YES').map(r=>[r.SOURCE_SHA,key(r)])); bySha.set(BASE_SHA,'R31B/V011');
  for(const r of rows) {
    if(bySha.has(r.PARENT_SHA)) {
      const trueParent=bySha.get(r.PARENT_SHA);
      if(trueParent!==r.DIRECT_PARENT) anomalies.push({VERSION:key(r),ISSUE:'PARENT_NAME_NORMALIZED_BY_SHA',RESOLUTION:trueParent,EVIDENCE:r.METADATA_PATH});
      r.DIRECT_PARENT=trueParent;
    } else if(/^V\d{3}$/.test(r.DIRECT_PARENT)) {
      const p=rows.find(p=>p.ROUTE===r.ROUTE && p.REVISION===r.DIRECT_PARENT);
      if(p?.SOURCE_RECOVERABLE==='YES') {r.PARENT_SHA=p.SOURCE_SHA;r.DIRECT_PARENT=key(p);}
    }
  }
  for(const route of [...scope.keys()].sort()) if(!rows.some(r=>r.ROUTE===route)) {
    const row=Object.fromEntries(Object.keys(rows[0]).map(h=>[h,'NA']));
    Object.assign(row,{ROUTE_ID:route,ROUTE_NAME:route,ROUTE:route,ROUTE_CLASS:'RESEARCH_ONLY',REVISION:'NONE',
      HYPOTHESIS_ID:'SEE_COMMITTED_ROUTE_RESEARCH',IMPLEMENTED:'NO',SOURCE_RECOVERABLE:'NO',BUILDABLE:'NO',CORRECTNESS_KNOWN:'NO',
      CORRECTNESS_VALID:'NO',KNOWN_UNRESOLVED_CORRECTNESS_FAILURE:'NO',CURRENT_STATUS:'RESEARCH_ONLY_NO_IMPLEMENTATION',
      SCOPE_EVIDENCE:[...scope.get(route)].join(';'),EVIDENCE_PATH:[...scope.get(route)].join(';')}); rows.push(row);
  }
  return {rows,anomalies,wts,scope,files,sources};
}

function tree(rows) {
  const impl=rows.filter(r=>r.IMPLEMENTED==='YES'), roots=new Map();
  for(const r of impl) {const p=r.DIRECT_PARENT||'UNKNOWN';if(!roots.has(p))roots.set(p,[]);roots.get(p).push(r);}
  const lines=['# Main-2 Canonical Revision Tree','','Edges use exact parent-source identity and committed metadata, not revision chronology.',
    'Git commit ancestry is a storage/rollback history; it is not automatically Kernel inheritance.',
    'R31B/V011 is the scoring anchor for all rows; this does not change their Direct Parents.','', '```text'];
  const done=new Set();
  const draw=(r,prefix)=>{ if(done.has(key(r)))return;done.add(key(r));lines.push(prefix+key(r)+' ['+r.SOURCE_SHA.slice(0,12)+']');
    for(const c of roots.get(key(r))||[]) draw(c,prefix+'    └── '); };
  for(const [p,children] of roots) if(!impl.some(r=>key(r)===p)) {lines.push(p);for(const r of children)draw(r,'  └── ');}
  for(const r of impl) if(!done.has(key(r)))draw(r,'UNRESOLVED/CYCLE: ');
  lines.push('```','','## Research-only / declared but not implemented','');
  for(const r of rows.filter(r=>r.IMPLEMENTED!=='YES'))lines.push('- '+key(r)+': '+r.CURRENT_STATUS+'; '+r.EVIDENCE_PATH);
  lines.push('','## Source and parent audit','',
    'Full SHAs, original source locations, Git objects and evidence links: `MAIN2-CANONICAL-ROUTE-REGISTRY.tsv`.',
    'Corrections to the older inventory: `../本地实验/MAIN2-CANONICAL-V1/census/corrections.tsv`.',
    'UNKNOWN parents remain unknown; source copies do not invent ancestry. No new performance Revision was created.');
  return lines.join('\n');
}

function inventory(args) {
  const r=buildInventory();
  console.log(JSON.stringify({routes:r.scope.size,records:r.rows.length,implemented:r.rows.filter(r=>r.IMPLEMENTED==='YES').length,
    sourceRecoverable:r.rows.filter(r=>r.IMPLEMENTED==='YES'&&r.SOURCE_RECOVERABLE==='YES').length,
    missing:r.rows.filter(r=>r.IMPLEMENTED==='YES'&&r.SOURCE_RECOVERABLE==='NO').map(key),
    sourcesScanned:r.sources.length,filesScanned:r.files.length},null,2));
  if(args.includes('--dry-run'))return;
  for(const row of r.rows.filter(r=>r.SOURCE_RECOVERABLE==='YES')) {
    if(exists(row.CANONICAL_SOURCE_PATH)) {if(sha(row.CANONICAL_SOURCE_PATH)!==row.SOURCE_SHA)throw new Error('Immutable snapshot mismatch');}
    else {
      fs.mkdirSync(path.dirname(row.CANONICAL_SOURCE_PATH),{recursive:true});
      if(row.SOURCE_PATH.startsWith('git:'))fs.writeFileSync(row.CANONICAL_SOURCE_PATH,execFileSync('git',['cat-file','blob',row.SOURCE_GIT_OBJECT],{cwd:ROOT}));
      else fs.copyFileSync(row.SOURCE_PATH,row.CANONICAL_SOURCE_PATH);
    }
  }
  writeTsv(REGISTRY,r.rows);write(path.join(ROOT,'技术路线/MAIN2-CANONICAL-REVISION-TREE.md'),tree(r.rows));
  const dir=path.join(DATA,'census');
  writeTsv(path.join(dir,'corrections.tsv'),r.anomalies,['VERSION','ISSUE','RESOLUTION','EVIDENCE']);
  writeTsv(path.join(dir,'worktrees.tsv'),r.wts);write(path.join(dir,'branches.txt'),git(['branch','--all','--format=%(refname:short) %(objectname)']));
  write(path.join(dir,'control-status.txt'),git(['status','--short','--branch']));
  write(path.join(dir,'recent-history.txt'),git(['log','--all','-60','--format=%h %ad %s','--date=iso-strict']));
  writeTsv(path.join(dir,'source-artifacts.tsv'),r.sources.map(s=>({...s,git_object:s.blob||git(['hash-object',s.file])})),['ROUTE','REVISION','file','sha','git_object','ref','relative']);
}

if(process.argv[1] && path.resolve(process.argv[1])===fileURLToPath(import.meta.url)) {
  if(process.argv[2]==='inventory') inventory(process.argv.slice(3));
  else { console.error('Usage: node 工具/main2-canonical-assets.mjs inventory [--dry-run]'); process.exitCode=2; }
}
