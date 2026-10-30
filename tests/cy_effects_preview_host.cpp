#include <array>
#include <iostream>
#include <vector>
#include <set>
#include "cy_effects_scene.h"
#include "native_2d_cylinder_adapter.h"

SegmentStub SEGMENT;
StripStub strip;
using namespace CylinderLamp;
using Renderer = void (*)(RenderState&, const Surface&, uint16_t);
static const Renderer renderers[] = {
  renderCyAnemone, renderCyTidalBloom, renderCyLavaLamp, renderCyFlame,
  renderCyPlasmaCore, renderCyDeepNoise, renderCyAuroraTube, renderCyInnerSwirl,
  renderCyBubblesVolume, renderCyRingRipples, renderCyRingRipplesRainbow,
  renderCyBottomRays, renderCyRisingBands, renderCyHelicalPlasma,
  renderCyNoiseWavesTube, renderCyCellMembraneFlow, renderCyCrossBandsTube
};
static unsigned energy(const CRGB& c) { return unsigned(c.r)+c.g+c.b; }
static unsigned delta(const CRGB& a, const CRGB& b) {
  return abs(int(a.r)-b.r)+abs(int(a.g)-b.g)+abs(int(a.b)-b.b);
}
static void require(bool ok, const char* message) {
  if (!ok) { std::cerr << message << "\n"; std::exit(1); }
}
int main() {
  Surface surface;
  RenderState prepared{};
  require(prepare(prepared,surface), "prepare");
  require(surface.width==16 && surface.height==16 && surface.count==256, "full logical surface");
  require(cyCoord(0,15,0,surface).h==0 && cyCoord(0,0,0,surface).h==1, "canonical bottom");
  for (unsigned kind=2; kind<17; ++kind)
    for (float t : {0.0f,1000.0f,10000.0f})
      for (unsigned y=0;y<16;++y) {
        CyCoord a=cyCoord(0,y,1,surface), b=a; b.theta+=CY_TWO_PI;
        require(fabsf(cyField(CyEffectKind(kind),a,t)-cyField(CyEffectKind(kind),b,t))<0.0001f,"analytic field periodicity");
      }
  for (int y=0;y<16;++y) {
    auto a=fxCoordCylinderShell(0,y,16,16,0,0.14f,1,1);
    auto b=fxCoordCylinderShell(16,y,16,16,0,0.14f,1,1);
    require(fabsf(a.x-b.x)<0.0001f && fabsf(a.y-b.y)<0.0001f,"kernel periodicity");
  }
  std::vector<std::vector<float>> features;
  for (unsigned kind=0;kind<17;++kind) {
    uint64_t signatures[2]={};
    unsigned peakMean=0, minEnergy=~0u, maxEnergy=0, movement=0;
    std::vector<float> feature;
    for(unsigned repeat=0;repeat<2;++repeat) {
      RenderState state{};
      std::array<CRGB,256> previous{};
      uint64_t hash=1469598103934665603ULL;
      for(unsigned frame=0;frame<601;++frame) {
        strip.now=frame*33;
        std::fill(std::begin(SEGMENT.writes),std::end(SEGMENT.writes),0);
        SEGMENT.clearPixels();
        renderers[kind](state,surface,33);
        unsigned total=0, change=0;
        for(unsigned i=0;i<256;++i) {
          require(SEGMENT.writes[i]==1,"every logical pixel must be written exactly once");
          const auto c=SEGMENT.pixels[i];
          total+=energy(c);
          change+=delta(c,previous[i]);
          for (auto byte : {c.r,c.g,c.b}) {hash^=byte;hash*=1099511628211ULL;}
          if(repeat==0 && frame%60==0) feature.push_back(float(energy(c))/765.0f);
          previous[i]=c;
        }
        require(total>256,"dead or effectively black frame");
        if(frame && repeat==0) {
          peakMean=std::max(peakMean,change/256);
          movement+=change;
        }
        minEnergy=std::min(minEnergy,total);maxEnergy=std::max(maxEnergy,total);
      }
      if(kind==1) require(state.tidalBloom.initialized && state.tidalBloom.nextTargetMs>1200,"dedicated Tidal Bloom state exercised");
      signatures[repeat]=hash;
    }
    require(signatures[0]==signatures[1],"deterministic repeat");
    require(movement>100,"frozen renderer");
    require(peakMean<90,"frame transition strobe");
    features.push_back(feature);
    std::cout << CylinderNative::cyNames[kind] << " signature=" << signatures[0]
      << " min_energy=" << minEnergy << " max_energy=" << maxEnergy << " peak_mean_delta=" << peakMean << "\n";
  }
  for(unsigned a=0;a<17;++a) for(unsigned b=a+1;b<17;++b) {
    float error=0,aa=0,bb=0,ab=0;
    for(unsigned i=0;i<features[a].size();++i) {
      float x=features[a][i],y=features[b][i];
      error+=(x-y)*(x-y); aa+=x*x;bb+=y*y;ab+=x*y;
    }
    const float rms=sqrtf(error/features[a].size());
    const float cosine=ab/sqrtf(aa*bb);
    if(rms<0.015f || cosine>0.998f) {
      std::cerr << "near-alias " << a << " " << b << " rms=" << rms << " cosine=" << cosine << "\n";return 1;
    }
  }
}
