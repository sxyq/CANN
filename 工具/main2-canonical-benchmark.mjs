#!/usr/bin/env node
// Exact-source support-only orchestration. No predictor, Kernel edits or Online.
import fs from 'node:fs';
import path from 'node:path';
import {spawnSync} from 'node:child_process';
import {fileURLToPath} from 'node:url';
import {ROOT,DATA,BASE_SHA,REGISTRY,SCORER,SUITE,exists,sha,text,json,readTsv,write,writeJson,writeTsv,key,safe,versionDir,git,worktrees} from './main2-canonical-assets.mjs';
import {frozenSuite,stats,validateRun,median} from './main2-canonical-scorer.mjs';

const HIST=path.join(ROOT,'研究/主代理/MAIN-2');
const V4=path.join(HIST,'calibration-v4/runs/20261005-v4-candidates');
const ENG=path.join(HIST,'calibration-v4/runs/20261005-v4-engineering');
const TEMPLATE=path.join(HIST,'calibration-v2/runs/20261004-v2-v011/stages/R31B__V011');
const HARNESS=path.join(ROOT,'工具/main2-canonical-harness');
const ENV='/usr/local/Ascend/ascend-toolkit/set_env.sh';
const LEASE_FILE=path.join(ROOT,'调度/服务器设备使用.tsv');
const LEASE_HEADERS=['device','owner','route','lease_id','status','start_time','end_time','note'];
const shellQuote = s=>"'"+String(s).replace(/'/g,"'\\''")+"'";
const stamp = ()=>new Date().toISOString();
const command = (cmd,args,cwd=ROOT,timeout=300000)=>spawnSync(cmd,args,{cwd,encoding:'utf8',timeout,maxBuffer:16*1024*1024});
const shell = (cmd,cwd=ROOT,timeout=300000)=>command('/bin/bash',['-lc',cmd],cwd,timeout);
const shellSetup = 'source '+shellQuote(ENV)+' >/dev/null 2>&1';
const hccIncludes='${ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0:${ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu:${ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/backward:${ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include';
const buildSetup=shellSetup+' && export CPLUS_INCLUDE_PATH='+hccIncludes+':${CPLUS_INCLUDE_PATH:-} && export C_INCLUDE_PATH='+hccIncludes+':${C_INCLUDE_PATH:-}';
const append=(file,row,headers=Object.keys(row))=>{if(!exists(file))writeTsv(file,[row],headers);else fs.appendFileSync(file,headers.map(h=>String(row[h]??'NA').replace(/[\r\n\t]+/g,' ')).join('\t')+'\n');};

export function bootstrap() {
  const files=['CMakeLists.txt','runner_ref.inc','local_types.h','runner_ref_parent.asc','runner_ref_candidate.asc',
    'runner_parent.asc','runner_candidate.asc','submission_shim.asc','main.asc'];
  const rows=[];
  for(const name of files) {
    const src=path.join(TEMPLATE,name),dest=path.join(HARNESS,name);
    if(!exists(src))throw new Error('TEMPLATE_MISSING '+src);
    if(exists(dest)&&sha(dest)!==sha(src))throw new Error('FROZEN_HARNESS_MISMATCH '+name);
    if(!exists(dest)){fs.mkdirSync(HARNESS,{recursive:true});fs.copyFileSync(src,dest);}
    rows.push({FILE:name,SHA256:sha(dest),SOURCE:path.relative(ROOT,src),SOURCE_GIT_BLOB:git(['hash-object',src]),ROLE:'UNCHANGED_EXISTING_SUPPORT_ONLY'});
  }
  writeTsv(path.join(HARNESS,'MANIFEST.tsv'),rows);
  const src=path.join(ROOT,'线上结果/R31B/V011/submission.asc'),dest=path.join(DATA,'sources',BASE_SHA+'.asc');
  if(sha(src)!==BASE_SHA)throw new Error('ANCHOR_SOURCE_MISMATCH');
  if(!exists(dest)){fs.mkdirSync(path.dirname(dest),{recursive:true});fs.copyFileSync(src,dest);}
  writeJson(path.join(DATA,'protocol.json'),{SCORER_VERSION:SCORER,SUITE_VERSION:SUITE,TIMING_MODE:'ENGINEERING_3RUN',
    WARMUP:45,SAMPLES:31,BLOCKS:1,BATCH_N:1,GAP:0,VALID_RUNS:3,MAX_ATTEMPTS:5,ORDER:'P-C/C-P/P-C',
    ANCHOR_DEVICE:4,EXECUTION_DEVICES:[0,1,2,3,4,5,7],MAX_TIMING_JOBS_PER_DEVICE:1,FORMAL_QUALIFICATION_GATE:false,HARNESS_SHA:sha(path.join(HARNESS,'runner_ref.inc')),
    CORE_CASES:['C12','C13','C14','C16'],DIAGNOSTIC_CASES:['C01','C08','C11'],BASE_SHA,
    FROZEN_SUITE_SHA:sha(path.join(ROOT,'本地实验/CANONICAL-LOCAL-SUITE-V1.tsv'))});
  return rows;
}
export function select(version) {
  if(version==='R31B/V011')return {ROUTE:'R31B',REVISION:'V011',SOURCE_SHA:BASE_SHA,
    CANONICAL_SOURCE_PATH:path.join(DATA,'sources',BASE_SHA+'.asc'),SOURCE_RECOVERABLE:'YES',KNOWN_UNRESOLVED_CORRECTNESS_FAILURE:'NO'};
  const row=readTsv(REGISTRY).find(r=>key(r)===version);
  if(!row||row.IMPLEMENTED!=='YES')throw new Error('UNKNOWN_OR_UNIMPLEMENTED_VERSION '+version);
  return row;
}
function checkHarness(stage) {
  return readTsv(path.join(HARNESS,'MANIFEST.tsv')).every(r=>exists(path.join(stage,r.FILE))&&sha(path.join(stage,r.FILE))===r.SHA256);
}
function reusable(r) {
  const isAnchor=r.SOURCE_SHA===BASE_SHA;
  for(const old of readTsv(path.join(V4,'build-status.tsv'))) {
    if(!isAnchor&&(old.ROUTE!==r.ROUTE||old.REVISION!==r.REVISION||old.SOURCE_SHA!==r.SOURCE_SHA))continue;
    const stage=path.join(V4,'stages',old.ROUTE+'__'+old.REVISION),variant=isAnchor?'parent':'candidate';
    const exe=path.join(stage,'build-v4','clx_ref_'+variant+'_probe');
    const expected=isAnchor?old.PARENT_RUNNER_SHA:old.CANDIDATE_RUNNER_SHA;
    if(old.BUILD_STATUS==='PASS'&&exists(exe)&&sha(exe)===expected&&checkHarness(stage)&&
      sha(path.join(stage,isAnchor?'parent.asc':'submission.asc'))===r.SOURCE_SHA)
      return {STAGE:stage,EXE:exe,EXE_SHA:expected,BUILD_SOURCE:'REUSED_IDENTITY_VERIFIED_BINARY',
        BUILD_EVIDENCE:path.join(V4,'build-status.tsv'),BUILD_LOG:old.BUILD_LOG};
  }
  return null;
}
export function build(r) {
  frozenSuite();
  const out=versionDir(r),file=path.join(out,'build.json');
  const prior=json(file);
  if(prior) {
    if(prior.SOURCE_SHA!==r.SOURCE_SHA)throw new Error('IMMUTABLE_BUILD_SOURCE_MISMATCH');
    if(prior.BUILD_STATUS==='PASS'&&exists(prior.EXE)&&sha(prior.EXE)===prior.EXE_SHA)return prior;
    if(prior.BUILD_STATUS!=='PASS')return prior; // one bounded build; no kernel repair
    throw new Error('EXISTING_BUILD_IDENTITY_INVALID');
  }
  const base={ROUTE:r.ROUTE,REVISION:r.REVISION,SOURCE_SHA:r.SOURCE_SHA,SCORER_VERSION:SCORER,SUITE_VERSION:SUITE,
    CREATED_AT:stamp(),HARNESS_SHA:sha(path.join(HARNESS,'runner_ref.inc')),NPU_ARCH:'dav-2201',SOC:'Ascend910B3'};
  if(r.SOURCE_RECOVERABLE!=='YES'||!exists(r.CANONICAL_SOURCE_PATH)) {
    const result={...base,BUILD_STATUS:'SOURCE_MISSING',REASON:'SOURCE_NOT_RECOVERABLE'};writeJson(file,result);return result;
  }
  if(sha(r.CANONICAL_SOURCE_PATH)!==r.SOURCE_SHA)throw new Error('SOURCE_SHA_MISMATCH');
  const old=reusable(r);
  if(old){const result={...base,...old,BUILD_STATUS:'PASS'};writeJson(file,result);return result;}
  const stage=path.join(DATA,'builds',safe(r)),buildDir=path.join(stage,'build');
  fs.mkdirSync(stage,{recursive:true});
  for(const row of readTsv(path.join(HARNESS,'MANIFEST.tsv')))fs.copyFileSync(path.join(HARNESS,row.FILE),path.join(stage,row.FILE));
  fs.copyFileSync(r.CANONICAL_SOURCE_PATH,path.join(stage,'submission.asc'));
  fs.copyFileSync(path.join(DATA,'sources',BASE_SHA+'.asc'),path.join(stage,'parent.asc'));
  const configure=buildSetup+' && cmake -S '+shellQuote(stage)+' -B '+shellQuote(buildDir)+' -DNPU_ARCH=dav-2201';
  const compile=buildSetup+' && cmake --build '+shellQuote(buildDir)+' --target clx_ref_candidate_probe --parallel 1';
  const hostBefore=shell('uptime; free -h; df -h '+shellQuote(ROOT),ROOT,10000);
  write(path.join(out,'build-host-before.txt'),hostBefore.stdout+hostBefore.stderr);
  const conf=shell(configure,stage),built=conf.status===0?shell(compile,stage):null;
  write(path.join(out,'configure.log'),(conf.stdout||'')+(conf.stderr||''));
  write(path.join(out,'build.log'),built?(built.stdout||'')+(built.stderr||''):'NOT_RUN_CONFIGURE_FAILED');
  const exe=path.join(buildDir,'clx_ref_candidate_probe'),ok=conf.status===0&&built?.status===0&&exists(exe);
  const result={...base,STAGE:stage,EXE:exe,EXE_SHA:ok?sha(exe):'NA',BUILD_SOURCE:'FRESH_EXACT_SOURCE_BUILD',BUILD_STATUS:ok?'PASS':'BUILD_FAILED',
    CONFIGURE_COMMAND:configure,BUILD_COMMAND:compile,CONFIGURE_RC:conf.status??'NO_STATUS',BUILD_RC:built?.status??'NOT_RUN',
    REASON:ok?'NONE':'Support build failed; no performance Kernel modified',BUILD_LOG:path.join(out,'build.log')};
  writeJson(file,result);return result;
}

export function resource(device,prefix) {
  const usage=command('npu-smi',['info','-t','usages','-i',String(device)],ROOT,15000);
  const proc=command('npu-smi',['info','-t','proc-mem','-i',String(device)],ROOT,15000);
  const snapshot=command('npu-smi',['info'],ROOT,15000);
  const host=shell('uptime; free -h; df -h '+shellQuote(ROOT),ROOT,10000);
  if(usage.status!==0)throw new Error('RESOURCE_USAGE_QUERY_FAILED');
  const value=name=>Number(usage.stdout.match(new RegExp(name+'\\s*:\\s*([0-9]+)'))?.[1]??NaN);
  const capacity=value('HBM Capacity\\(MB\\)'),rate=value('HBM Usage Rate\\(%\\)');
  // Integer reported percent: conservative bound, not an invented exact value.
  const freeLower=Math.max(0,capacity*(1-(rate+1)/100));
  if(!Number.isFinite(freeLower)||freeLower<100)throw new Error('HBM_FREE_LOWER_BOUND_LT_100_OR_UNKNOWN');
  const bytes=command('df',['-Pk',ROOT],ROOT,10000).stdout.split('\n')[1]?.trim().split(/\s+/);
  if(!bytes||Number(bytes[3])<15*1024*1024)throw new Error('AVAILABLE_DISK_LT_15G_OR_UNKNOWN');
  const row={AT:stamp(),DEVICE:device,HBM_CAPACITY_MB:capacity,HBM_USAGE_INTEGER_PERCENT:rate,
    FREE_HBM_LOWER_BOUND_MB:freeLower,AICORE:value('Aicore Usage Rate\\(%\\)'),AIVECTOR:value('Aivector Usage Rate\\(%\\)'),
    HOST_LOAD:host.stdout.split('\n')[0],EXISTING_PROCESSES:proc.stdout.trim().replace(/\s*\n\s*/g,';'),
    HBM_PRECISION:'CONSERVATIVE_LOWER_BOUND; main-table raw snapshot also retained',NOTE:'OBSERVED_ONLY;NO_KILL_PAUSE_MIGRATE'};
  if(prefix) {
    write(prefix+'-usage.txt',usage.stdout+usage.stderr);write(prefix+'-processes.txt',proc.stdout+proc.stderr);
    write(prefix+'-npu.txt',snapshot.stdout+snapshot.stderr);write(prefix+'-host.txt',host.stdout+host.stderr);writeJson(prefix+'-resource.json',row);
  }
  return row;
}
function activeLeases() {
  const grouped=new Map();
  for(const w of worktrees()) for(const row of readTsv(path.join(w.root,'调度/服务器设备使用.tsv'))) {
    const prev=grouped.get(row.lease_id);
    const time=row.status==='RELEASED'?row.end_time:row.start_time;
    if(!prev||String(time)>String(prev.time)||time===prev.time&&row.status==='RELEASED')grouped.set(row.lease_id,{row,time});
  }
  return [...grouped.values()].map(v=>v.row).filter(r=>r.status==='LEASED');
}
function acquire(r,device) {
  const conflict=activeLeases().find(l=>Number(l.device)===device);
  if(conflict)throw new Error('LEASE_CONFLICT '+conflict.lease_id);
  let common=git(['rev-parse','--git-common-dir']);common=path.resolve(ROOT,common);
  const lock=path.join(common,'main2-canonical-d'+device+'.lock');
  const fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,JSON.stringify({owner:'MAIN-2',version:key(r),pid:process.pid,at:stamp()}));
  fs.closeSync(fd);
  const lease={device,owner:'MAIN-2',route:r.ROUTE,lease_id:'M2-CANONICAL-'+safe(r)+'-D'+device+'-'+Date.now(),
    status:'LEASED',start_time:stamp(),end_time:'-',note:'ENGINEERING_3RUN; fixed canonical suite; exact historical source; no Online'};
  append(LEASE_FILE,lease,LEASE_HEADERS);return {...lease,lock};
}
function release(l,note) {
  append(LEASE_FILE,{...l,status:'RELEASED',end_time:stamp(),note},LEASE_HEADERS);
  const current=json(l.lock);if(current?.pid===process.pid)fs.unlinkSync(l.lock);
}
function choose(r,device,fallback) {
  const errors=[];
  for(const d of [...new Set([device,fallback].filter(Number.isInteger))]) {
    try{resource(d);const lease=acquire(r,d);return {device:d,lease};}catch(e){errors.push('d'+d+':'+e.message);}
  }
  throw new Error('NO_SAFE_DEVICE '+errors.join(';'));
}
function invoke(build,c,device,prefix,warmup,samples) {
  if(sha(build.EXE)!==build.EXE_SHA)throw new Error('EXE_IDENTITY_CHANGED');
  if(!checkHarness(build.STAGE))throw new Error('HARNESS_CHANGED');
  fs.mkdirSync(path.dirname(prefix),{recursive:true});
  const argv=[String(device),c.ROWS,c.WIDTH,c.DTYPE_CODE,prefix,String(warmup),String(samples),'1','0','1'];
  const cmd=shellSetup+' && exec "$@"';
  const run=command('/bin/bash',['-lc',cmd,'canonical-runner',build.EXE,...argv],build.STAGE,180000);
  write(prefix+'.stdout.log',run.stdout||'');write(prefix+'.stderr.log',run.stderr||'');
  write(prefix+'.rc.txt',String(run.status??('SIGNAL:'+run.signal)));
  writeJson(prefix+'.command.json',{EXE:build.EXE,EXE_SHA:build.EXE_SHA,ARGV:argv,RC:run.status,SIGNAL:run.signal,ERROR:run.error?.message||null});
  return {rc:run.status??'SIGNAL_OR_TIMEOUT',s:stats(prefix+'-stats.txt'),prefix};
}
export function correctness(r,device=4) {
  const file=path.join(versionDir(r),'correctness.json');if(exists(file))return json(file);
  const b=json(path.join(versionDir(r),'build.json'));if(!b||b.BUILD_STATUS!=='PASS')throw new Error('BUILD_PASS_REQUIRED');
  if(r.KNOWN_UNRESOLVED_CORRECTNESS_FAILURE==='YES') {
    const result={ROUTE:r.ROUTE,REVISION:r.REVISION,SOURCE_SHA:r.SOURCE_SHA,CORRECTNESS_STATUS:'KNOWN_GLOBAL_FAILURE',
      REASON:r.CORRECTNESS_STATUS+'; retained, not bypassed by subset',EVIDENCE:r.EVIDENCE_PATH};writeJson(file,result);return result;
  }
  const cells=[];
  for(const c of frozenSuite()) {
    const dir=path.join(versionDir(r),'correctness',c.CASE_ID);resource(device,path.join(dir,'before'));
    const run=invoke(b,c,device,path.join(dir,'preflight'),0,1);
    const pass=run.rc===0&&Number(run.s.bad)===0;
    cells.push({CASE_ID:c.CASE_ID,DEVICE:device,RC:run.rc,BAD:run.s.bad??'NA',STATUS:pass?'PASS':'FAIL',STATS:run.prefix+'-stats.txt'});
    console.log(key(r)+' '+c.CASE_ID+' correctness='+cells.at(-1).STATUS+' bad='+cells.at(-1).BAD);
  }
  const result={ROUTE:r.ROUTE,REVISION:r.REVISION,SOURCE_SHA:r.SOURCE_SHA,EXE_SHA:b.EXE_SHA,
    HARNESS_SHA:b.HARNESS_SHA,CORRECTNESS_STATUS:cells.every(c=>c.STATUS==='PASS')?'PASS':'CORRECTNESS_BLOCKED',
    PASS_COUNT:cells.filter(c=>c.STATUS==='PASS').length,CASE_COUNT:cells.length,CELLS:cells,
    REASON:cells.filter(c=>c.STATUS!=='PASS').map(c=>c.CASE_ID+':RC='+c.RC+',BAD='+c.BAD).join(';')||'NONE'};
  writeJson(file,result);return result;
}
function normalized(r,c,side,run,attempt,b,res,prefix,rc,s,fresh='YES') {
  return {SCORER_VERSION:SCORER,SUITE_VERSION:SUITE,ROUTE:r.ROUTE,REVISION:r.REVISION,
    SOURCE_SHA:side==='parent'||side==='anchor'?BASE_SHA:r.SOURCE_SHA,CASE_ID:c.CASE_ID,SIDE:side,RUN:run,ATTEMPT:attempt,
    DEVICE:res.DEVICE,RC:rc,BAD:s.bad??'NA',VALID_RUN:rc===0&&Number(s.bad)===0?'YES':'NO',
    MEDIAN_US:s.ALL_DEVICE_median_us??'NA',MEAN_US:s.ALL_DEVICE_mean_us??'NA',MAD_US:s.ALL_DEVICE_MAD_us??'NA',CV:s.ALL_DEVICE_CV??'NA',
    EXE_SHA:b.EXE_SHA,HARNESS_SHA:b.HARNESS_SHA,RAW_PATH:prefix+'-raw.tsv',STATS_PATH:prefix+'-stats.txt',FRESH_NPU_RUN:fresh,
    LOAD_NOTE:'FREE_HBM_LOWER_BOUND_MB='+res.FREE_HBM_LOWER_BOUND_MB+';AICORE='+res.AICORE+';HOST='+res.HOST_LOAD,
    RESOURCE_EVIDENCE:res.RESOURCE_EVIDENCE||'SEE_BATCH_RESOURCE',LEASE:res.LEASE||'HISTORICAL_REUSE'};
}
export function measure(r,device=4,fallback=null) {
  const corr=json(path.join(versionDir(r),'correctness.json'));
  if(corr?.CORRECTNESS_STATUS!=='PASS')throw new Error('CORRECTNESS_PASS_REQUIRED');
  const b=json(path.join(versionDir(r),'build.json')),a=json(path.join(versionDir({ROUTE:'R31B',REVISION:'V011'}),'build.json'));
  const isAnchor=r.SOURCE_SHA===BASE_SHA;
  if(!isAnchor&&!exists(path.join(ROOT,'本地实验/R31B-V011-CANONICAL-LOCAL-VECTOR.tsv')))throw new Error('FRESH_ANCHOR_FIRST');
  const runFile=path.join(versionDir(r),'measurements.tsv'),attemptFile=path.join(versionDir(r),'attempts.tsv');
  const selected=choose(r,device,fallback),d=selected.device;
  const batchDir=path.join(versionDir(r),'measurement');
  let note='Completed canonical engineering suite';
  try {
    const before=resource(d,path.join(batchDir,'before'));
    before.LEASE=selected.lease.lease_id;before.RESOURCE_EVIDENCE=path.join(batchDir,'before-resource.json');
    for(const c of frozenSuite()) {
      const existing=readTsv(runFile).filter(v=>v.CASE_ID===c.CASE_ID);
      const done=new Set(existing.filter(v=>v.SIDE===(isAnchor?'anchor':'candidate')).map(v=>Number(v.RUN)));
      let attempt=readTsv(attemptFile).filter(v=>v.CASE_ID===c.CASE_ID).length;
      for(let run=1;run<=3;run++) {
        if(done.has(run))continue;
        let acquired=false;
        while(attempt<5&&!acquired) {
          attempt++;
          const res=resource(d);Object.assign(res,{LEASE:before.LEASE,RESOURCE_EVIDENCE:before.RESOURCE_EVIDENCE});
          const order=isAnchor?['anchor']:(run%2?['parent','candidate']:['candidate','parent']);
          const collected=[];
          for(const side of order) {
            const bd=side==='parent'?a:b;
            const prefix=path.join(batchDir,c.CASE_ID,'attempt-'+attempt+'-'+side);
            const raw=invoke(bd,c,d,prefix,45,31);
            const row=normalized(r,c,side,run,attempt,bd,res,prefix,raw.rc,raw.s);
            try{validateRun(row,c,side==='candidate'?r.SOURCE_SHA:BASE_SHA);}catch(e){row.VALID_RUN='NO';row.INVALID_REASON=e.message;}
            collected.push(row);
          }
          acquired=collected.every(v=>v.VALID_RUN==='YES');
          append(attemptFile,{CASE_ID:c.CASE_ID,RUN:run,ATTEMPT:attempt,VALID_PAIR:acquired?'YES':'NO',
            DETAIL:collected.map(v=>v.SIDE+':RC='+v.RC+',BAD='+v.BAD+',VALID='+v.VALID_RUN).join(';')});
          writeJson(path.join(batchDir,c.CASE_ID,'attempt-'+attempt+'-identity.json'),collected);
          if(acquired)for(const v of collected)append(runFile,v);
          console.log(key(r)+' '+c.CASE_ID+' run='+run+' attempt='+attempt+' valid='+acquired+' '+collected.map(v=>v.SIDE+'='+v.MEDIAN_US).join(' '));
          if(collected.some(v=>Number(v.BAD)>0||Number(v.RC)!==0)) {
            // Correctness/runtime errors are not jitter. Preserve, stop this cell.
            note='Measurement includes correctness/runtime blocker';break;
          }
        }
        if(!acquired)break;
      }
    }
    resource(d,path.join(batchDir,'after'));
  }catch(e){note='Stopped: '+e.message;throw e;}finally{release(selected.lease,note);}
  return readTsv(runFile);
}

export function reuseEngineering(r) {
  const b=json(path.join(versionDir(r),'build.json')),corr=json(path.join(versionDir(r),'correctness.json'));
  if(b?.BUILD_STATUS!=='PASS'||corr?.CORRECTNESS_STATUS!=='PASS')throw new Error('BUILD_AND_CORRECTNESS_REQUIRED');
  const file=path.join(versionDir(r),'measurements.tsv');if(exists(file))return {status:'EXISTS_NO_OVERWRITE'};
  const old=readTsv(path.join(ENG,'engineering-runs.tsv')).filter(v=>v.ROUTE===r.ROUTE&&v.REVISION===r.REVISION&&v.SOURCE_SHA===r.SOURCE_SHA);
  if(old.length!==21)return {status:'NO_COMPATIBLE_ENGINEERING_VECTOR'};
  const buildRow=readTsv(path.join(V4,'build-status.tsv')).find(v=>v.ROUTE===r.ROUTE&&v.REVISION===r.REVISION);
  if(!buildRow||buildRow.CANDIDATE_RUNNER_SHA!==b.EXE_SHA)return {status:'BINARY_IDENTITY_NOT_MATCHED'};
  const config=readTsv(path.join(ENG,'run-config.tsv'))[0];
  if(config?.WARMUP!=='45'||config?.SAMPLES_PER_RUN!=='31'||config?.RUNS_PER_SIDE!=='3')throw new Error('OLD_PROTOCOL_INCOMPATIBLE');
  const normalizedRows=[];
  for(const c of frozenSuite())for(const row of old.filter(v=>v.CASE_ID===c.CASE_ID))for(const side of ['parent','candidate']) {
    const prefix=row[side==='parent'?'PARENT_PREFIX':'CANDIDATE_PREFIX'];
    const s=stats(prefix+'-stats.txt'),rc=Number(text(prefix+'.rc.txt').trim());
    const bd={EXE_SHA:side==='parent'?buildRow.PARENT_RUNNER_SHA:buildRow.CANDIDATE_RUNNER_SHA,HARNESS_SHA:b.HARNESS_SHA};
    const rsrc={DEVICE:Number(row.DEVICE),RESOURCE_EVIDENCE:path.join(ENG,'batch-resource.tsv'),LEASE:'HISTORICAL_ENG_D4',HOST_LOAD:'SEE_RETAINED_RESOURCE',FREE_HBM_LOWER_BOUND_MB:'HISTORICAL',AICORE:'HISTORICAL'};
    const n=normalized(r,c,side,Number(row.RUN),Number(row.RUN),bd,rsrc,prefix,rc,s,'NO_REUSED_VALID_RAW');
    validateRun(n,c,side==='parent'?BASE_SHA:r.SOURCE_SHA);normalizedRows.push(n);
  }
  writeTsv(file,normalizedRows);
  writeJson(path.join(versionDir(r),'raw-reuse.json'),{SCORE_SOURCE:'REUSED_VALID_RAW',SOURCE_SHA:r.SOURCE_SHA,
    WARMUP:45,SAMPLES:31,BLOCKS:1,BATCH_N:1,VALID_PAIRS:21,CORE_CASES:4,DIAGNOSTIC_CASES:3,
    SOURCE_FILE:path.join(ENG,'engineering-runs.tsv'),SOURCE_FILE_SHA:sha(path.join(ENG,'engineering-runs.tsv')),
    HARNESS_SHA:b.HARNESS_SHA,NOTE:'All raw/stats/protocol/source/executable identities checked; reaggregate with new fixed anchor; no old score copied.'});
  return {status:'REUSED_VALID_RAW',runs:normalizedRows.length};
}

if(process.argv[1]&&path.resolve(process.argv[1])===fileURLToPath(import.meta.url)) {
  const cmd=process.argv[2],version=process.argv[3],device=Number(process.argv[4]||4);
  if(!Number.isInteger(device)||![0,1,2,3,4,5,7].includes(device))throw new Error('Device outside static execution plan; d6 lease protected');
  if(cmd==='bootstrap')console.log(JSON.stringify(bootstrap(),null,2));
  else if(cmd==='resource')console.log(JSON.stringify(resource(device),null,2));
  else {
    const r=select(version);
    if(cmd==='build')console.log(JSON.stringify(build(r),null,2));
    else if(cmd==='correctness')console.log(JSON.stringify(correctness(r,device),null,2));
    else if(cmd==='measure')measure(r,device,null);
    else if(cmd==='reuse')console.log(JSON.stringify(reuseEngineering(r),null,2));
    else {console.error('Usage: bootstrap|build|correctness|measure|reuse VERSION [DEVICE]');process.exitCode=2;}
  }
}
