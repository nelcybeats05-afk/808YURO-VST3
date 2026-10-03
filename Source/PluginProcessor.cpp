#include "PluginProcessor.h"
#include "PluginEditor.h"
YuroExactProcessor::YuroExactProcessor()
: AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),
  apvts(*this,nullptr,"PARAMETERS",createParams()) {}
juce::AudioProcessorValueTreeState::ParameterLayout YuroExactProcessor::createParams(){
 std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
 p.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("amount",1),"Amount",juce::NormalisableRange<float>(0.f,1.f,.001f),.65f));
 p.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("effect",1),"Effect",juce::StringArray{"GHOST VERB","ECHO STAIRS","PITCH LIFT","FILTER OPEN","STUTTER GUN","CRUSH CLIMB","GHOST SWELL","DROP CUT","ECHO RUN","STUTTER CUT","TAPE BRAKE","AIRLOCK","WIDE VOID","GLASS LIFT","NIGHT SHIFT","DEEP DIVE"},0));
 p.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("style",1),"Style",juce::StringArray{"CLASSIC","PHONE","CHORUS","TREMOLO"},0));
 p.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("grid",1),"Grid",juce::StringArray{"1/8","1/16"},0));
 p.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("bpm",1),"BPM",juce::NormalisableRange<float>(40.f,240.f,1.f),120.f));
 p.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("mode",1),"Mode",juce::StringArray{"FX","RAW","LISTEN"},0));
 return {p.begin(),p.end()};
}
void YuroExactProcessor::prepareToPlay(double s,int block){sr=s;juce::dsp::ProcessSpec spec{s,(juce::uint32)block,2};delay.prepare(spec);delay.reset();delay.setMaximumDelayInSamples((int)(s*1.5));lpL.prepare(spec);lpR.prepare(spec);hpL.prepare(spec);hpR.prepare(spec);lpL.reset();lpR.reset();hpL.reset();hpR.reset();}
void YuroExactProcessor::releaseResources(){delay.reset();lpL.reset();lpR.reset();hpL.reset();hpR.reset();}
bool YuroExactProcessor::isBusesLayoutSupported(const BusesLayout& x) const {auto in=x.getMainInputChannelSet(),out=x.getMainOutputChannelSet();return in==out&&(out==juce::AudioChannelSet::mono()||out==juce::AudioChannelSet::stereo());}
void YuroExactProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&){
 const float a=apvts.getRawParameterValue("amount")->load(); const int e=(int)apvts.getRawParameterValue("effect")->load(); const int st=(int)apvts.getRawParameterValue("style")->load(); const int gr=(int)apvts.getRawParameterValue("grid")->load(); const float bpm=apvts.getRawParameterValue("bpm")->load(); const int mode=(int)apvts.getRawParameterValue("mode")->load(); const int n=b.getNumSamples(); const int ch=b.getNumChannels();
 const float beat = 60.0f / juce::jmax(40.0f, bpm);
const float grid = beat * (gr == 0 ? 0.5f : 0.25f);
 lpL.coefficients=juce::dsp::IIR::Coefficients<float>::makeLowPass(sr,700.f+11000.f*a); lpR.coefficients=lpL.coefficients;
 hpL.coefficients=juce::dsp::IIR::Coefficients<float>::makeHighPass(sr,350.f); hpR.coefficients=hpL.coefficients;
 for(int i=0;i<n;++i){float lfo=.5f+.5f*std::sin(juce::MathConstants<float>::twoPi*i/(sr*juce::jmax(.001f,grid)));
  for(int c=0;c<ch;++c){auto*d=b.getWritePointer(c);float dry=d[i],w=dry;int d1=(int)(sr*.09),d2=(int)(sr*.18),d3=(int)(sr*.36);
   if(e==0||e==6||e==11){float x=delay.popSample(c,(float)d1),y=delay.popSample(c,(float)d2),z=delay.popSample(c,(float)d3);delay.pushSample(c,dry);w=dry*(1-.35f*a)+(x*.55f+y*.32f+z*.18f)*a;}
   else if(e==1||e==8||e==12){float x=delay.popSample(c,(float)d1),y=delay.popSample(c,(float)d2),z=delay.popSample(c,(float)d3);delay.pushSample(c,dry);w=dry+(x*.46f+y*.27f+z*.14f)*a;}
   else if(e==2){w=std::tanh(dry*(1.f+5.f*a))*.72f;}
   else if(e==3||e==13){w=c==0?lpL.processSample(dry):lpR.processSample(dry);}
   else if(e==4||e==9){int per=juce::jmax(32,(int)(sr*grid));int on=juce::jmax(1,(int)(per*(.18f+.70f*(1-a))));w=(i%per)<on?dry:0.f;}
   else if(e==5||e==14){float bits=4.f+12.f*(1.f-a),steps=std::pow(2.f,bits);w=std::round(dry*steps)/steps;}
   else if(e==7){int per=juce::jmax(32,(int)(sr*grid*2));int pos=i%per;int fade=(int)(per*.62f);float g=pos<fade?1.f:1.f-(float)(pos-fade)/juce::jmax(1,per-fade);w=dry*g;}
   else if(e==10||e==15){float u=(float)i/juce::jmax(1,n-1);w=dry*juce::jmax(.04f,1.f-a*u);}
   if(st==1){w=c==0?hpL.processSample(w):hpR.processSample(w);w=c==0?lpL.processSample(w):lpR.processSample(w);w*=1.2f;}
   else if(st==2){float mod=sr*(.012f+.006f*std::sin(i*.002f));float q=delay.popSample(c,mod);delay.pushSample(c,dry);w=w*.8f+q*.32f;}
   else if(st==3)w*=.55f+.45f*lfo;
   float out=(mode==1)?dry:(mode==2?w:(dry*(1-a)+w*a)); d[i]=juce::jlimit(-1.f,1.f,out);
  }
 }
}
juce::AudioProcessorEditor* YuroExactProcessor::createEditor(){return new YuroExactProcessorEditor(*this);} 
void YuroExactProcessor::getStateInformation(juce::MemoryBlock& d){if(auto x=apvts.copyState().createXml())copyXmlToBinary(*x,d);} 
void YuroExactProcessor::setStateInformation(const void*d,int s){if(auto x=getXmlFromBinary(d,s))if(x->hasTagName(apvts.state.getType()))apvts.replaceState(juce::ValueTree::fromXml(*x));}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new YuroExactProcessor();}
