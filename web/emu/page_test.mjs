// SPDX-License-Identifier: GPL-3.0-only
// DOM-model smoke test of the merged page startup and event wiring (not visual/browser QA).
import fs from 'node:fs';
import vm from 'node:vm';
import assert from 'node:assert/strict';
const html=fs.readFileSync(new URL('./index.html',import.meta.url),'utf8');
const ids=new Map();
class Element {
  constructor(tag='div') { this.tagName=tag.toUpperCase();this.children=[];this.style={setProperty(){}};this.attrs={};this.handlers={};this.value='';this.dataset={}; const classes=new Set();this.classList={add:x=>classes.add(x),remove:x=>classes.delete(x),contains:x=>classes.has(x),toggle(x,on){const yes=on??!classes.has(x);yes?classes.add(x):classes.delete(x);return yes;}}; }
  setAttribute(k,v){this.attrs[k]=String(v);if(k==='id')ids.set(String(v),this);if(k==='class')String(v).split(' ').forEach(c=>this.classList.add(c));}
  appendChild(e){this.children.push(e);return e;} append(...e){this.children.push(...e);} insertBefore(e){this.children.push(e);} remove(){} focus(){}
  addEventListener(k,f){(this.handlers[k]??=[]).push(f);} removeEventListener(){}
  querySelectorAll(selector){return this.children.filter(x=>x instanceof Element && selector[0]==='.' && x.classList.contains(selector.slice(1))).concat(this.children.flatMap(x=>x instanceof Element?x.querySelectorAll(selector):[]));}
  querySelector(s){return this.querySelectorAll(s)[0]||null;}
  getBBox(){return {x:0,y:0,width:48,height:48};} getBoundingClientRect(){return {x:0,y:0,left:0,top:0,width:1000,height:624};}
  getContext(){return {createImageData:(w,h)=>({data:new Uint8ClampedArray(w*h*4)}),putImageData(){}};}
}
for(const m of html.matchAll(/<([a-z]+)\b[^>]*\bid="([^"]+)"[^>]*>/g)){const e=new Element(m[1]);e.id=m[2];ids.set(m[2],e);}
for(const m of html.matchAll(/<select\b[^>]*id="([^"]+)"[^>]*>[\s\S]*?<option value="([^"]+)"/g))ids.get(m[1]).value=m[2];
const label=new Element('span');label.setAttribute('class','record-label');ids.get('audio-record').append(label);
const document={getElementById:id=>ids.get(id)||null,createElement:tag=>new Element(tag),createElementNS:(_,tag)=>new Element(tag),documentElement:new Element(),body:new Element(),addEventListener(){},hidden:false};
let source=html.match(/<script type="module">([\s\S]*?)<\/script>/)[1];
const globals={document,console,Uint8Array,Uint8ClampedArray,Map,Set,Math,Number,String,Array,Object,Promise,JSON,Date,performance,Blob,URL,setTimeout,clearTimeout,atob,btoa,
 navigator:{},localStorage:{getItem(){return null;},setItem(){},removeItem(){}},matchMedia:()=>({matches:false,addEventListener(){}}),addEventListener(){},requestAnimationFrame(){return 1;},cancelAnimationFrame(){},confirm:()=>false};
for(const m of [...source.matchAll(/import\s*\{([^}]+)\}\s*from\s*"([^"]+)";/g)]){const mod=await import(new URL(m[2],import.meta.url));for(const name of m[1].split(',').map(x=>x.trim()).filter(Boolean)){assert(name in mod,name);globals[name]=mod[name];}source=source.replace(m[0],'');}
const state=vm.runInNewContext(source+';({build,midiMessage,knobs,buttons,keys,toolsReady})',globals);
assert.equal(state.knobs.length,8);assert.equal(state.buttons.length,14);assert.equal(state.keys.length,27);
state.build('tall');assert.equal(state.keys.length,27);state.build('wide');
assert(ids.get('go').handlers.click.length);assert(ids.get('load-tar').handlers.click.length);
assert(ids.get('audio-record').handlers.click.length);assert(ids.get('state-import').handlers.click.length);
assert.equal(ids.get('midi-map').children.length,22);
assert.equal(ids.get('audio-format').value,'flac');state.toolsReady();
for(const msg of [[0x90,60,100],[0x80,60,0],[0xb0,113,70]])state.midiMessage(msg);
console.log('emu page: wide/portrait panel startup, Tar/recording/backup handlers and MIDI dispatch PASS (DOM model)');
