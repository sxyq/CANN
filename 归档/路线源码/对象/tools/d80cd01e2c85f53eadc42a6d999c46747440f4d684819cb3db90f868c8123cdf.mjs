import test from 'node:test';
import assert from 'node:assert/strict';
import {median,geometricScore,scoreVectors,frozenSuite} from './main2-canonical-scorer.mjs';

test('frozen suite has exactly four fixed core cases and three diagnostics',()=>{
  const s=frozenSuite();assert.equal(s.filter(r=>r.CASE_ROLE==='CORE').length,4);
  assert.throws(()=>frozenSuite(s.slice(1)),/MISMATCH/);
  assert.throws(()=>frozenSuite(s.map((r,i)=>i? r:{...r,CASE_ROLE:'DIAGNOSTIC'})),/MISMATCH/);
});
test('identity anchor is exactly 100; higher speedup is monotone',()=>{
  assert.equal(geometricScore([1,2,3,4],[1,2,3,4]),100);
  assert.ok(Math.abs(geometricScore([1,2,3,4],[.5,1,1.5,2])-200)<1e-10);
  assert.ok(geometricScore([1,2,3,4],[.5,2,3,4])>100);
});
test('incomplete, zero, negative and nonfinite cases are rejected',()=>{
  for(const v of [[1,2,3],[1,2,3,0],[1,2,3,-1],[1,2,3,NaN],[1,2,3,Infinity]])
    assert.throws(()=>geometricScore([1,2,3,4],v));
});
test('one extremely fast or slow run cannot dominate the case',()=>{
  assert.equal(median([10,11,.01]),10);assert.equal(median([10,11,1e6]),11);
});
const v=(median,quality='GOOD')=>({median,quality,logRange:.001,madRatio:.001});
test('one-case positive and mixed signs are flagged fragile',()=>{
  const cells=[.5,1.01,1.01,1.01].map(x=>({anchor:v(1),candidate:v(x),parent:v(1),quality:'GOOD'}));
  const s=scoreVectors(cells);assert.ok(s.score>100);assert.equal(s.fragile,true);assert.equal(s.reliable,false);
});
test('all-case stable gain is not single-case dominated',()=>{
  const s=scoreVectors(Array.from({length:4},()=>({anchor:v(1),candidate:v(.9),parent:v(1),quality:'GOOD'})));
  assert.equal(s.fragile,false);assert.equal(s.reliable,true);assert.equal(s.status,'LOCAL_POSITIVE');
});
test('POOR data retain numerical score without supporting reliable improvement',()=>{
  const s=scoreVectors(Array.from({length:4},()=>({anchor:v(1),candidate:v(.8,'POOR'),parent:v(1),quality:'POOR'})));
  assert.ok(s.score>100);assert.equal(s.status,'LOCAL_NEUTRAL');assert.equal(s.reliable,false);
});
test('fresh-anchor versus paired-parent disagreement is visible',()=>{
  const s=scoreVectors(Array.from({length:4},()=>({anchor:v(1),candidate:v(.9),parent:v(.8),quality:'GOOD'})));
  assert.ok(s.score>100);assert.ok(s.pairedScore<100);assert.equal(s.fragile,true);
});
