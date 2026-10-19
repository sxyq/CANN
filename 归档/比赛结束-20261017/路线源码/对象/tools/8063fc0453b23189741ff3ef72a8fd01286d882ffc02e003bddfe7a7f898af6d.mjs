import test from 'node:test';
import assert from 'node:assert/strict';
import {direction,trend} from './main2-canonical-consolidate.mjs';

const row=(score,low,high,poor=0)=>({CANONICAL_LOCAL_SCORE:String(score),EMPIRICAL_LOW:String(low),EMPIRICAL_HIGH:String(high),POOR_CASES:String(poor)});
test('unscored is never a performance regression',()=>{
  assert.equal(direction({CANONICAL_LOCAL_SCORE:'UNSCORED'}),'UNSCORED');
  assert.equal(trend([{CANONICAL_LOCAL_SCORE:'UNSCORED'}]),'INSUFFICIENT_DATA');
});
test('noisy numerical gains cannot establish improvement or plateau',()=>{
  assert.equal(direction(row(120,110,130,1)),'LOCAL_NEUTRAL');
  assert.equal(trend([row(120,110,130,1),row(99,90,108,1)]),'INSUFFICIENT_DATA');
});
test('quality-qualified aggregate direction remains distinct from rank',()=>{
  assert.equal(direction({...row(110,102,118),STATUS:'LOCAL_CHAMPION'}),'LOCAL_POSITIVE');
  assert.equal(direction(row(85,80,90)),'LOCAL_NEGATIVE');
  assert.equal(direction(row(101,97,106)),'LOCAL_NEUTRAL');
});
test('trend needs multiple quality-qualified observations',()=>{
  assert.equal(trend([row(110,102,118)]),'INSUFFICIENT_DATA');
  assert.equal(trend([row(110,102,118),row(108,101,115)]),'IMPROVING');
  assert.equal(trend([row(89,80,95),row(85,80,90)]),'REGRESSING');
  assert.equal(trend([row(101,97,105),row(99,95,103)]),'PLATEAU');
  assert.equal(trend([row(110,102,118),row(85,80,90)]),'MIXED');
});
