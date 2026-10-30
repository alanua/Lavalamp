import assert from 'node:assert/strict';
import fs from 'node:fs';
import path from 'node:path';
import os from 'node:os';
import { spawnSync } from 'node:child_process';
import test from 'node:test';
import { createHash } from 'node:crypto';

// Verbatim excerpts fetched from wled/WLED at the exact release commit.
// FX.cpp Git blob: 99ded1b928ecdbf4ae1419bc206feb7c68175bc6
// FX.h Git blob: a8f6e1cd76fb348347b00f6354f2e14c903c9496
// No effect algorithms are copied. Keep preprocessor guards: the inventory is
// the union of registrations across supported compile-time configurations.
const pinnedCommit = '29b389df1c1aaec6ff53aea742d17063b985906c';
const registrationSource = String.raw`  // --- 2D  effects ---
#ifndef WLED_DISABLE_2D
  addEffect(FX_MODE_2DPLASMAROTOZOOM, &mode_2Dplasmarotozoom, _data_FX_MODE_2DPLASMAROTOZOOM);
  addEffect(FX_MODE_2DSPACESHIPS, &mode_2Dspaceships, _data_FX_MODE_2DSPACESHIPS);
  addEffect(FX_MODE_2DCRAZYBEES, &mode_2Dcrazybees, _data_FX_MODE_2DCRAZYBEES);

  #ifdef WLED_PS_DONT_REPLACE_2D_FX
  addEffect(FX_MODE_2DGHOSTRIDER, &mode_2Dghostrider, _data_FX_MODE_2DGHOSTRIDER);
  addEffect(FX_MODE_2DBLOBS, &mode_2Dfloatingblobs, _data_FX_MODE_2DBLOBS);
  #endif

  addEffect(FX_MODE_2DSCROLLTEXT, &mode_2Dscrollingtext, _data_FX_MODE_2DSCROLLTEXT);
  addEffect(FX_MODE_2DDRIFTROSE, &mode_2Ddriftrose, _data_FX_MODE_2DDRIFTROSE);
  addEffect(FX_MODE_2DDISTORTIONWAVES, &mode_2Ddistortionwaves, _data_FX_MODE_2DDISTORTIONWAVES);
  addEffect(FX_MODE_2DGEQ, &mode_2DGEQ, _data_FX_MODE_2DGEQ); // audio
  addEffect(FX_MODE_2DNOISE, &mode_2Dnoise, _data_FX_MODE_2DNOISE);
  addEffect(FX_MODE_2DFIRENOISE, &mode_2Dfirenoise, _data_FX_MODE_2DFIRENOISE);
  addEffect(FX_MODE_2DSQUAREDSWIRL, &mode_2Dsquaredswirl, _data_FX_MODE_2DSQUAREDSWIRL);

  //non audio
  addEffect(FX_MODE_2DDNA, &mode_2Ddna, _data_FX_MODE_2DDNA);
  addEffect(FX_MODE_2DMATRIX, &mode_2Dmatrix, _data_FX_MODE_2DMATRIX);
  addEffect(FX_MODE_2DMETABALLS, &mode_2Dmetaballs, _data_FX_MODE_2DMETABALLS);
  addEffect(FX_MODE_2DFUNKYPLANK, &mode_2DFunkyPlank, _data_FX_MODE_2DFUNKYPLANK); // audio
  addEffect(FX_MODE_2DPULSER, &mode_2DPulser, _data_FX_MODE_2DPULSER);
  addEffect(FX_MODE_2DDRIFT, &mode_2DDrift, _data_FX_MODE_2DDRIFT);
  addEffect(FX_MODE_2DWAVERLY, &mode_2DWaverly, _data_FX_MODE_2DWAVERLY); // audio
  addEffect(FX_MODE_2DSUNRADIATION, &mode_2DSunradiation, _data_FX_MODE_2DSUNRADIATION);
  addEffect(FX_MODE_2DCOLOREDBURSTS, &mode_2DColoredBursts, _data_FX_MODE_2DCOLOREDBURSTS);
  addEffect(FX_MODE_2DJULIA, &mode_2DJulia, _data_FX_MODE_2DJULIA);
  addEffect(FX_MODE_2DGAMEOFLIFE, &mode_2Dgameoflife, _data_FX_MODE_2DGAMEOFLIFE);
  addEffect(FX_MODE_2DTARTAN, &mode_2Dtartan, _data_FX_MODE_2DTARTAN);
  addEffect(FX_MODE_2DPOLARLIGHTS, &mode_2DPolarLights, _data_FX_MODE_2DPOLARLIGHTS);
  addEffect(FX_MODE_2DSWIRL, &mode_2DSwirl, _data_FX_MODE_2DSWIRL); // audio
  addEffect(FX_MODE_2DLISSAJOUS, &mode_2DLissajous, _data_FX_MODE_2DLISSAJOUS);
  addEffect(FX_MODE_2DFRIZZLES, &mode_2DFrizzles, _data_FX_MODE_2DFRIZZLES);
  addEffect(FX_MODE_2DPLASMABALL, &mode_2DPlasmaball, _data_FX_MODE_2DPLASMABALL);
  addEffect(FX_MODE_2DHIPHOTIC, &mode_2DHiphotic, _data_FX_MODE_2DHIPHOTIC);
  addEffect(FX_MODE_2DSINDOTS, &mode_2DSindots, _data_FX_MODE_2DSINDOTS);
  addEffect(FX_MODE_2DDNASPIRAL, &mode_2DDNASpiral, _data_FX_MODE_2DDNASPIRAL);
  addEffect(FX_MODE_2DBLACKHOLE, &mode_2DBlackHole, _data_FX_MODE_2DBLACKHOLE);
  addEffect(FX_MODE_2DSOAP, &mode_2Dsoap, _data_FX_MODE_2DSOAP);
  addEffect(FX_MODE_2DOCTOPUS, &mode_2Doctopus, _data_FX_MODE_2DOCTOPUS);
  addEffect(FX_MODE_2DWAVINGCELL, &mode_2Dwavingcell, _data_FX_MODE_2DWAVINGCELL);
  addEffect(FX_MODE_2DAKEMI, &mode_2DAkemi, _data_FX_MODE_2DAKEMI); // audio

#ifndef WLED_DISABLE_PARTICLESYSTEM2D
  addEffect(FX_MODE_PARTICLEVOLCANO, &mode_particlevolcano, _data_FX_MODE_PARTICLEVOLCANO);
  addEffect(FX_MODE_PARTICLEFIRE, &mode_particlefire, _data_FX_MODE_PARTICLEFIRE);
  addEffect(FX_MODE_PARTICLEFIREWORKS, &mode_particlefireworks, _data_FX_MODE_PARTICLEFIREWORKS);
  addEffect(FX_MODE_PARTICLEVORTEX, &mode_particlevortex, _data_FX_MODE_PARTICLEVORTEX);
  addEffect(FX_MODE_PARTICLEPERLIN, &mode_particleperlin, _data_FX_MODE_PARTICLEPERLIN);
  addEffect(FX_MODE_PARTICLEPIT, &mode_particlepit, _data_FX_MODE_PARTICLEPIT);
  addEffect(FX_MODE_PARTICLEBOX, &mode_particlebox, _data_FX_MODE_PARTICLEBOX);
  addEffect(FX_MODE_PARTICLEATTRACTOR, &mode_particleattractor, _data_FX_MODE_PARTICLEATTRACTOR); // 872 bytes
  addEffect(FX_MODE_PARTICLEIMPACT, &mode_particleimpact, _data_FX_MODE_PARTICLEIMPACT);
  addEffect(FX_MODE_PARTICLEWATERFALL, &mode_particlewaterfall, _data_FX_MODE_PARTICLEWATERFALL);
  addEffect(FX_MODE_PARTICLESPRAY, &mode_particlespray, _data_FX_MODE_PARTICLESPRAY);
  addEffect(FX_MODE_PARTICLESGEQ, &mode_particleGEQ, _data_FX_MODE_PARTICLEGEQ);
  addEffect(FX_MODE_PARTICLECENTERGEQ, &mode_particlecenterGEQ, _data_FX_MODE_PARTICLECIRCULARGEQ);
  addEffect(FX_MODE_PARTICLEGHOSTRIDER, &mode_particleghostrider, _data_FX_MODE_PARTICLEGHOSTRIDER);
  addEffect(FX_MODE_PARTICLEBLOBS, &mode_particleblobs, _data_FX_MODE_PARTICLEBLOBS);
  addEffect(FX_MODE_PARTICLEGALAXY, &mode_particlegalaxy, _data_FX_MODE_PARTICLEGALAXY);
#endif // WLED_DISABLE_PARTICLESYSTEM2D
#endif // WLED_DISABLE_2D`;
const idSource = String.raw`#define FX_MODE_2DPLASMAROTOZOOM       114 // was Candy Cane prior to 0.14 (use Chase 2)
#define FX_MODE_2DSPACESHIPS           118 //gap fill
#define FX_MODE_2DCRAZYBEES            119 //gap fill
#define FX_MODE_2DGHOSTRIDER           120 //gap fill
#define FX_MODE_2DBLOBS                121 //gap fill
#define FX_MODE_2DSCROLLTEXT           122 //gap fill
#define FX_MODE_2DDRIFTROSE            123 //gap fill
#define FX_MODE_2DDISTORTIONWAVES      124 //gap fill
#define FX_MODE_2DSOAP                 125 //gap fill
#define FX_MODE_2DOCTOPUS              126 //gap fill
#define FX_MODE_2DWAVINGCELL           127 //gap fill
#define FX_MODE_2DGEQ                  139
#define FX_MODE_2DNOISE                146
#define FX_MODE_2DFIRENOISE            149
#define FX_MODE_2DSQUAREDSWIRL         150
#define FX_MODE_2DDNA                  152
#define FX_MODE_2DMATRIX               153
#define FX_MODE_2DMETABALLS            154
#define FX_MODE_2DFUNKYPLANK           160
#define FX_MODE_2DPULSER               162
#define FX_MODE_2DDRIFT                164
#define FX_MODE_2DWAVERLY              165
#define FX_MODE_2DSUNRADIATION         166
#define FX_MODE_2DCOLOREDBURSTS        167
#define FX_MODE_2DJULIA                168
#define FX_MODE_2DGAMEOFLIFE           172
#define FX_MODE_2DTARTAN               173
#define FX_MODE_2DPOLARLIGHTS          174
#define FX_MODE_2DSWIRL                175
#define FX_MODE_2DLISSAJOUS            176
#define FX_MODE_2DFRIZZLES             177
#define FX_MODE_2DPLASMABALL           178
#define FX_MODE_2DHIPHOTIC             180
#define FX_MODE_2DSINDOTS              181
#define FX_MODE_2DDNASPIRAL            182
#define FX_MODE_2DBLACKHOLE            183
#define FX_MODE_2DAKEMI                186
#define FX_MODE_PARTICLEVOLCANO        187
#define FX_MODE_PARTICLEFIRE           188
#define FX_MODE_PARTICLEFIREWORKS      189
#define FX_MODE_PARTICLEVORTEX         190
#define FX_MODE_PARTICLEPERLIN         191
#define FX_MODE_PARTICLEPIT            192
#define FX_MODE_PARTICLEBOX            193
#define FX_MODE_PARTICLEATTRACTOR      194
#define FX_MODE_PARTICLEIMPACT         195
#define FX_MODE_PARTICLEWATERFALL      196
#define FX_MODE_PARTICLESPRAY          197
#define FX_MODE_PARTICLESGEQ           198
#define FX_MODE_PARTICLECENTERGEQ      199
#define FX_MODE_PARTICLEGHOSTRIDER     200
#define FX_MODE_PARTICLEBLOBS          201
#define FX_MODE_PARTICLEGALAXY         217`;
const radialSymbols = new Set([
 'FX_MODE_2DDRIFTROSE', 'FX_MODE_2DDRIFT', 'FX_MODE_2DSUNRADIATION',
 'FX_MODE_2DPLASMABALL', 'FX_MODE_2DBLACKHOLE', 'FX_MODE_2DOCTOPUS',
 'FX_MODE_PARTICLEVORTEX', 'FX_MODE_PARTICLEATTRACTOR',
 'FX_MODE_PARTICLECENTERGEQ', 'FX_MODE_PARTICLEGALAXY',
]);

function registrations(source) {
 return [...source.matchAll(/addEffect\(\s*(FX_MODE_\w+)\s*,/g)].map(m => m[1]);
}

function availableRegistrations(defines) {
 const stack = [true];
 const active = [];
 for (const line of registrationSource.split('\n')) {
   const guard = line.match(/^\s*#(ifdef|ifndef)\s+(\w+)/);
   if (guard) {
     stack.push(stack.at(-1) && (defines.has(guard[2]) === (guard[1] === 'ifdef')));
   } else if (/^\s*#endif/.test(line)) {
     assert.ok(stack.length > 1, 'balanced upstream guards');
     stack.pop();
   } else if (stack.at(-1)) active.push(...registrations(line));
 }
 assert.equal(stack.length, 1, 'balanced upstream guards');
 return active;
}

test('compile-time guards retain legacy and particle entries in the union inventory', () => {
 const legacy = 'WLED_PS_DONT_REPLACE_2D_FX';
 const noParticles = 'WLED_DISABLE_PARTICLESYSTEM2D';
 const all = registrations(registrationSource);
 assert.deepEqual(availableRegistrations(new Set([legacy])), all);
 assert.equal(availableRegistrations(new Set()).length, 51);
 assert.equal(availableRegistrations(new Set([legacy, noParticles])).length, 37);
 assert.equal(availableRegistrations(new Set([noParticles])).length, 35);
 assert.deepEqual(availableRegistrations(new Set(['WLED_DISABLE_2D', legacy])), []);
 assert.deepEqual(all.filter(symbol => !availableRegistrations(new Set()).includes(symbol)),
   ['FX_MODE_2DGHOSTRIDER', 'FX_MODE_2DBLOBS']);
});
function verifyClassification(manifest, inventory, definitions) {
 const ids = new Map([...definitions.matchAll(/^#define\s+(FX_MODE_\w+)\s+(\d+)/gm)]
   .map(m => [m[1], Number(m[2])]));
 const body = manifest.match(/static constexpr Entry effects\[\] = \{([\s\S]*?)\n\};/);
 assert.ok(body, 'production manifest must exist');
 const rows = body[1].trim().split('\n');
 const entries = rows.map(row => {
   const m = row.match(/^\s*\{(\d+), "([^"]+)", (\w+)\},\s*\/\/\s*(FX_MODE_\w+)\s*$/);
   assert.ok(m, 'every manifest row must have an explicit classification and upstream symbol: '+row);
   return { id: Number(m[1]), name: m[2], mapping: m[3], symbol: m[4] };
 });
 assert.equal(inventory.length, 53, 'pinned native 2D inventory size');
 assert.equal(new Set(inventory).size, 53, 'no duplicate upstream registrations');
 assert.equal(entries.length, 53, 'no missing or extra manifest rows');
 for (const key of ['id', 'name', 'symbol'])
   assert.equal(new Set(entries.map(e => e[key])).size, 53, 'no duplicate '+key);
 assert.deepEqual(entries.map(e => e.symbol).sort(), [...inventory].sort(),
   'exact one-to-one upstream registration coverage');
 for (const e of entries) {
   assert.ok(ids.has(e.symbol), 'upstream ID definition exists for '+e.symbol);
   assert.equal(e.id, ids.get(e.symbol), 'pinned upstream numeric ID for '+e.symbol);
   assert.equal(e.mapping, radialSymbols.has(e.symbol) ? 'RADIAL_BOTTOM_POLAR' : 'FLAT_WRAP',
     'issue #13 classification for '+e.symbol);
 }
 assert.equal(entries.filter(e => e.mapping === 'RADIAL_BOTTOM_POLAR').length, 10);
 assert.equal(entries.filter(e => e.mapping === 'FLAT_WRAP').length, 43);
}

test('pinned source registration snapshot covers every conditional native 2D effect', () => {
 const script = fs.readFileSync(path.join(root, 'scripts/prepare-wled.ps1'), 'utf8');
 assert.ok(script.includes('$WledCommit = "'+pinnedCommit+'"'));
 assert.ok(header.includes(pinnedCommit));
 assert.match(registrationSource, /#ifdef WLED_PS_DONT_REPLACE_2D_FX/);
 assert.match(registrationSource, /#ifndef WLED_DISABLE_PARTICLESYSTEM2D/);
 assert.equal(createHash('sha256').update(registrationSource).digest('hex'), '641c6a6dda152ab0f7bd6f5501ceb77f7c7f521ab208aed8ec4484853a15b096');
 assert.equal(createHash('sha256').update(idSource).digest('hex'), '975f01fd9b5cfbc90297a9b820a4205a3a3653d3004b1e3572aa16053986b54a');
 verifyClassification(header, registrations(registrationSource), idSource);
});

test('coverage rejects missing, duplicate, added, unclassified and misclassified effects', () => {
 const inventory = registrations(registrationSource);
 const row = header.match(/^  \{114,.*$/m)[0];
 for (const manifest of [
   header.replace(row, ''),
   header.replace(row, row+'\n'+row),
   header.replace('"Rotozoomer", FLAT_WRAP', '"Rotozoomer", UNCLASSIFIED'),
   header.replace('"Pulser", FLAT_WRAP', '"Pulser", RADIAL_BOTTOM_POLAR'),
   header.replace('{114, "Rotozoomer"', '{115, "Rotozoomer"'),
 ]) assert.throws(() => verifyClassification(manifest, inventory, idSource));
 assert.throws(() => verifyClassification(header, [...inventory, 'FX_MODE_FUTURE_2D'], idSource));
 assert.throws(() => verifyClassification(header, inventory.slice(1), idSource));
 assert.throws(() => verifyClassification(header, [...inventory.slice(1), inventory[1]], idSource));
});
const root=path.resolve(import.meta.dirname,'..');
const header=fs.readFileSync(path.join(root,'overlays/wled/usermods/cylinder_lava/native_2d_cylinder_adapter.h'),'utf8');
// Independently extracted registrations from pinned upstream FX.cpp's 2D block.
const upstream=[
  [
    114,
    "Rotozoomer"
  ],
  [
    118,
    "Spaceships"
  ],
  [
    119,
    "Crazy Bees"
  ],
  [
    120,
    "Ghost Rider"
  ],
  [
    121,
    "Blobs"
  ],
  [
    122,
    "Scrolling Text"
  ],
  [
    123,
    "Drift Rose"
  ],
  [
    124,
    "Distortion Waves"
  ],
  [
    139,
    "GEQ"
  ],
  [
    146,
    "Noise2D"
  ],
  [
    149,
    "Firenoise"
  ],
  [
    150,
    "Squared Swirl"
  ],
  [
    152,
    "DNA"
  ],
  [
    153,
    "Matrix"
  ],
  [
    154,
    "Metaballs"
  ],
  [
    160,
    "Funky Plank"
  ],
  [
    162,
    "Pulser"
  ],
  [
    164,
    "Drift"
  ],
  [
    165,
    "Waverly"
  ],
  [
    166,
    "Sun Radiation"
  ],
  [
    167,
    "Colored Bursts"
  ],
  [
    168,
    "Julia"
  ],
  [
    172,
    "Game Of Life"
  ],
  [
    173,
    "Tartan"
  ],
  [
    174,
    "Polar Lights"
  ],
  [
    175,
    "Swirl"
  ],
  [
    176,
    "Lissajous"
  ],
  [
    177,
    "Frizzles"
  ],
  [
    178,
    "Plasma Ball"
  ],
  [
    180,
    "Hiphotic"
  ],
  [
    181,
    "Sindots"
  ],
  [
    182,
    "DNA Spiral"
  ],
  [
    183,
    "Black Hole"
  ],
  [
    125,
    "Soap"
  ],
  [
    126,
    "Octopus"
  ],
  [
    127,
    "Waving Cell"
  ],
  [
    186,
    "Akemi"
  ],
  [
    187,
    "PS Volcano"
  ],
  [
    188,
    "PS Fire"
  ],
  [
    189,
    "PS Fireworks"
  ],
  [
    190,
    "PS Vortex"
  ],
  [
    191,
    "PS Fuzzy Noise"
  ],
  [
    192,
    "PS Ballpit"
  ],
  [
    193,
    "PS Box"
  ],
  [
    194,
    "PS Attractor"
  ],
  [
    195,
    "PS Impact"
  ],
  [
    196,
    "PS Waterfall"
  ],
  [
    197,
    "PS Spray"
  ],
  [
    198,
    "PS GEQ 2D"
  ],
  [
    199,
    "PS GEQ Nova"
  ],
  [
    200,
    "PS Ghost Rider"
  ],
  [
    201,
    "PS Blobs"
  ],
  [
    217,
    "PS Galaxy"
  ]
];
test('all 53 pinned upstream 2D registrations occur exactly once',()=>{
 const entries=[...header.matchAll(/\{(\d+), "([^"]+)", (FLAT_WRAP|RADIAL_BOTTOM_POLAR)\}/g)];
 assert.deepEqual(entries.map(m=>[Number(m[1]),m[2]]),upstream);
 assert.equal(new Set(entries.map(m=>m[1])).size,53);
 assert.equal(entries.filter(m=>m[3]==='RADIAL_BOTTOM_POLAR').length,10);
 assert.equal(entries.filter(m=>m[3]==='FLAT_WRAP').length,43);
});
test('native adapter production geometry, periodic interpolation, stable center and identity resolver',
 ()=>{
 const tmp=fs.mkdtempSync(path.join(os.tmpdir(),'cy-native-'));
 try {
  const source=path.join(tmp,'host.cpp'), binary=path.join(tmp,'host');
  fs.writeFileSync(source,"#include <cassert>\n#include <cmath>\n#include <set>\n#include <string>\n#include \"native_2d_cylinder_adapter.h\"\nusing namespace CylinderNative;\nint main() {\n  static_assert(sizeof(effects)/sizeof(effects[0])==53,\"complete native manifest\");\n  unsigned radial=0;std::set<unsigned> ids;std::set<std::string> names;\n  uint32_t pixels[256];for(unsigned y=0;y<16;++y)for(unsigned x=0;x<16;++x)pixels[y*16+x]=x*8+(y*8<<8);\n  for(const auto& e:effects) {\n    assert(ids.insert(e.id).second && names.insert(e.name).second);\n    radial+=e.mapping==RADIAL_BOTTOM_POLAR;\n    for(int y=0;y<16;++y)for(int x=-32;x<32;++x)\n      assert(sample(pixels,e.mapping,x,y)==sample(pixels,e.mapping,x+16,y));\n    for(float x : {-0.00001f,0.0f,15.99999f,16.0f}) {\n      auto c=sourceCoord(e.mapping,x,15);\n      if(e.mapping==RADIAL_BOTTOM_POLAR)assert(fabs(c.x-7.5f)<0.00001f && fabs(c.y-7.5f)<0.00001f);\n    }\n    for(unsigned y=0;y<16;++y)for(unsigned x=0;x<16;++x) {\n      auto c=sourceCoord(e.mapping,x,y);\n      if(e.mapping==FLAT_WRAP)assert(c.x==x && c.y==y);\n      else {\n        assert(fabs(hypot(c.x-7.5f,c.y-7.5f)-7.5f*(15-y)/15)<0.00001f);\n        if(y<15)assert(fabs(remainder(atan2(c.y-7.5f,c.x-7.5f)-atan2(sin(x*6.283185307179586f/16),cos(x*6.283185307179586f/16)),6.283185307179586))<0.00001f);\n      }\n    }\n    assert(sample(pixels,e.mapping,0,7)==sample(pixels,e.mapping,16,7));\n  }\n  assert(radial==10 && ids.size()-radial==43 && !find(255));\n  assert(wrap(-1)==15 && wrap(16)==0);\n  assert(bilinear(pixels,-0.5f,0)==bilinear(pixels,15.5f,0));\n  assert(resolveCyIdentity(\"CY Anemone\")==-1);\n  for(unsigned i=0;i<17;++i)registerCyIdentity(i,238-i); // deliberately reordered IDs\n  for(unsigned i=0;i<17;++i)assert(resolveCyIdentity(cyNames[i])==int(238-i));\n  assert(resolveCyIdentity(\"CY Missing\")==-1 && resolveCyIdentity(nullptr)==-1);\n  for(unsigned i=0;i<17;++i)registerCyIdentity(i,220+i);\n  assert(resolveCyIdentity(\"CY Tidal Bloom\")==221);\n}\n");
  const result=spawnSync(process.env.CXX||'g++',['-std=c++17','-I'+path.join(root,'overlays/wled/usermods/cylinder_lava'),source,'-o',binary],{encoding:'utf8'});
  assert.equal(result.status,0,result.stderr);
  const run=spawnSync(binary,[],{encoding:'utf8'});assert.equal(run.status,0,run.stdout+run.stderr);
 } finally {fs.rmSync(tmp,{recursive:true,force:true});}
});
