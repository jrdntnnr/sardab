// SPDX-License-Identifier: GPL-3.0-only
// Sample-loop ADPCM state, level matching, and safe stock-restore assembly.
import assert from 'node:assert/strict';
import fs from 'node:fs';
import vm from 'node:vm';
import {Package, logicalImage, validateStockPackage} from './fm1pkg.js';
const html = fs.readFileSync('web/editor.html','utf8');
const proto = html.slice(html.indexOf('/*PROTO-BEGIN*/'),html.indexOf('/*PROTO-END*/'));
const e = vm.runInNewContext(proto + ';({parseWav,imaEncode,buildSlot,sampleLoop,zeroCrossing,matchGains,withGain,loudDb})', {console,setTimeout,clearTimeout,setInterval,clearInterval});
const tone = Int16Array.from({length:22050},(_,i)=>Math.round(Math.sin(i*.09)*27000));
const z = {s:tone,root:60,a:200,looped:true,ls:600,le:5000,gain:-6};
const slot=e.buildSlot('LOOP',[z]), v=new DataView(slot.hdr.buffer);
assert.equal(v.getUint32(40,true),400); assert.equal(v.getUint32(44,true),4800);
assert.equal(slot.hdr[59],1);
const encoded=e.imaEncode(e.withGain(tone,-6),400);
assert.equal(v.getInt16(54,true),encoded.pred); assert.equal(slot.hdr[56],encoded.idx);
assert.deepEqual(Array.from(slot.data),Array.from(encoded.data));
assert.equal(e.sampleLoop({...z,a:1000,s:tone.subarray(0,1000)}).ls,0);
assert.equal(e.sampleLoop({...z,a:1000,s:tone.subarray(0,1000)}).le,999);
assert.throws(()=>e.buildSlot('BAD',[{...z,s:new Int16Array(1)}]));
assert(e.zeroCrossing([-1,-.4,.1,.5,-.1,-.2,.2],3)===2);
const quieter=e.withGain(tone,-12), gains=e.matchGains([tone,quieter]);
assert.equal(gains[0],-12); assert.equal(gains[1],0);
assert(Math.abs(e.loudDb(e.withGain(tone,gains[0]))-e.loudDb(quieter))<.1);
assert.deepEqual(Array.from(e.withGain(tone,10)),Array.from(tone));
// A forward WAV smpl chunk AFTER data must be found; end is inclusive.
const fmt=Buffer.alloc(16);fmt.writeUInt16LE(1);fmt.writeUInt16LE(1,2);fmt.writeUInt32LE(22050,4);fmt.writeUInt32LE(44100,8);fmt.writeUInt16LE(2,12);fmt.writeUInt16LE(16,14);
const smpl=Buffer.alloc(60);smpl.writeUInt32LE(1,28);smpl.writeUInt32LE(40,44);smpl.writeUInt32LE(799,48);
const chunk=(name,b)=>{const h=Buffer.alloc(8);h.write(name);h.writeUInt32LE(b.length,4);return Buffer.concat([h,b]);};
const body=Buffer.concat([Buffer.from('WAVE'),chunk('fmt ',fmt),chunk('data',Buffer.from(tone.buffer)),chunk('smpl',smpl)]);
const wav=Buffer.concat([Buffer.from('RIFF'),Buffer.alloc(4),body]);wav.writeUInt32LE(body.length,4);
assert.equal(e.parseWav(wav).loop.start,40);assert.equal(e.parseWav(wav).loop.end,799);
assert.throws(()=>e.parseWav(wav.subarray(0,wav.length-5)));
console.log('upstream web: loop header, predictor/index, trim clamping, gain, level matching, WAV markers PASS');
const page=fs.readFileSync('web/index_pkg.html','utf8');
const archiveInfo=vm.runInNewContext(page.slice(page.indexOf('function archiveInfo('),page.indexOf('$("stock-go").addEventListener'))+';archiveInfo');
function info(name,motion=2,backup=1){return [...Buffer.from(name),0,1,4,127,39,16,...Buffer.from('ANALOG'),0,118,16,0x55,1,4,0x4d,motion,0,0,0x42,1,backup];}
for(const name of ['SARDAB 2.1','GHOULBOX 2.1','FELUCCA 1.6']) assert(archiveInfo(info(name)).supported);
assert(!archiveInfo(info('SARDAB 2.1',1,0)).supported);
assert(!archiveInfo(info('SARDAB 2.1').slice(0,-1)).supported);
assert(!archiveInfo(info('UNRELATED')).supported);
await assert.rejects(validateStockPackage(new Uint8Array(699956)));
// The official package is validated and sent unchanged, including USR/config and its loader.
if (process.env.FM1_STOCK_PACKAGE) {
  const official = new Uint8Array(fs.readFileSync(process.env.FM1_STOCK_PACKAGE));
  const verified = await validateStockPackage(official);
  assert.equal(verified.product,'FM-1_015');
  assert.deepEqual(verified.image,logicalImage(official));
  const pkg = new Package(official);
  for (const name of ['flash.bin','USR','isd_config.ini','ota.bin']) assert(pkg.entries.some(e=>e.name===name));
  assert.equal(verified.head.to-verified.head.from,16384);
  const corrupt=official.slice(); corrupt[40000]^=1;
  await assert.rejects(validateStockPackage(corrupt));
  console.log('stock restore: real V15 hash, unchanged complete image, boot-head range, corrupt input rejection PASS');
}
