#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

static float dbFromRms(float r) { return juce::Decibels::gainToDecibels(r, -60.0f); }

juce::AudioProcessorValueTreeState::ParameterLayout VocalChainPlusAudioProcessor::createLayout()
{
    using P = juce::AudioParameterFloat; using C = juce::AudioParameterChoice;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    auto add=[&](const char* id,const char* name,float lo,float hi,float def){p.push_back(std::make_unique<P>(id,name,juce::NormalisableRange<float>(lo,hi),def));};
    add("input","Input",-24,24,0); add("output","Output",-24,24,0); add("mix","Dry Wet",0,100,100);
    add("tune","Tune",0,100,0); add("comp","Comp",0,100,50); add("deess","DeEsser",0,100,35); add("eq","EQ",0,100,50); add("sat","Saturation",0,100,20); add("air","Air",0,100,30);
    add("width","Width",0,100,50); add("timing","Timing",0,100,50); add("pitch","Pitch",-12,12,0); add("dist","Distortion",0,100,0); add("filter","Filter",0,100,50);
    add("reverb","Reverb",0,100,25); add("delay","Delay",0,100,15); add("delayfb","Delay Feedback",0,95,25); add("chorus","Chorus",0,100,0);
    add("threshold","Threshold",-48,0,-18); add("ratio","Ratio",1,12,4); add("attack","Attack",1,100,10); add("release","Release",20,500,120);
    p.push_back(std::make_unique<C>("role","Vocal Role",juce::StringArray{"MAIN VOCAL","DOUBLES","ADLIBS"},0));
    p.push_back(std::make_unique<C>("style","Vocal Style",juce::StringArray{"CLEAN TRAP","DARK TRAP","AIRY","DISTORTED","WIDE DOUBLES","UNDERGROUND","R&B","CUSTOM"},0));
    p.push_back(std::make_unique<C>("key","Key",juce::StringArray{"Auto","C","C#","D","D#","E","F","F#","G","G#","A","A#","B"},0));
    p.push_back(std::make_unique<C>("scale","Scale",juce::StringArray{"Minor","Major"},0));
    return {p.begin(),p.end()};
}

VocalChainPlusAudioProcessor::VocalChainPlusAudioProcessor(): AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)), apvts(*this,nullptr,"PARAMETERS",createLayout()) {}

void VocalChainPlusAudioProcessor::prepareToPlay(double sampleRate,int block)
{
    sr=sampleRate;
    juce::dsp::ProcessSpec s{sampleRate,(juce::uint32)block,(juce::uint32)getTotalNumOutputChannels()};
    compressor.prepare(s); reverb.prepare(s); delay.prepare(s); chorus.prepare(s);
    lowEq.prepare(s); midEq.prepare(s); highEq.prepare(s);
    compressor.reset(); reverb.reset(); delay.reset(); chorus.reset(); lowEq.reset(); midEq.reset(); highEq.reset();
    analysisRing.assign((size_t)juce::jmax(4096,(int)(sampleRate*0.10)),0.0f); analysisWrite=0;
    const int pb=(int)sampleRate*2; pitchBuffer.assign((size_t)getTotalNumOutputChannels(),std::vector<float>((size_t)pb,0.0f)); pitchWrite=0; pitchPhase=0.0f;
}

bool VocalChainPlusAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const
{
    auto o=l.getMainOutputChannelSet();
    return (o==juce::AudioChannelSet::mono()||o==juce::AudioChannelSet::stereo()) && o==l.getMainInputChannelSet();
}

void VocalChainPlusAudioProcessor::updateAnalysis(const juce::AudioBuffer<float>& b)
{
    if(b.getNumSamples()==0 || analysisRing.empty()) return;
    float rms=0; for(int c=0;c<b.getNumChannels();++c) rms+=b.getRMSLevel(c,0,b.getNumSamples()); rms/=juce::jmax(1,b.getNumChannels()); inputDb.store(dbFromRms(rms));
    const float* x=b.getReadPointer(0);
    for(int i=0;i<b.getNumSamples();++i){analysisRing[analysisWrite++%analysisRing.size()]=x[i]; float a=std::abs(x[i]); env=0.995f*env+0.005f*a; ++samplesSinceTransient; if(a>juce::jmax(0.04f,env*2.5f)){ float beat=(float)(sr*60.0/120.0); float phase=std::fmod((float)samplesSinceTransient,beat); float d=juce::jmin(phase,beat-phase); timingScore.store(100.0f*juce::jlimit(0.0f,1.0f,1.0f-d/(beat*0.22f))); samplesSinceTransient=0; }}
    if(analysisRing.size()<2048 || rms<0.003f) return;
    float best=-1.0e9f; int bestLag=0; int minLag=(int)(sr/900.0), maxLag=juce::jmin((int)(sr/70.0),(int)analysisRing.size()/2);
    for(int lag=minLag;lag<=maxLag;++lag){double sum=0,aa=0,bb=0; for(int n=0;n<1024;++n){float a=analysisRing[(analysisWrite+analysisRing.size()-1-n)%analysisRing.size()]; float q=analysisRing[(analysisWrite+analysisRing.size()-1-n-lag)%analysisRing.size()]; sum+=a*q; aa+=a*a; bb+=q*q;} float corr=(float)(sum/std::sqrt(aa*bb+1.0e-12)); if(corr>best){best=corr;bestLag=lag;}}
    if(bestLag>0 && best>0.35f) pitchHz.store((float)(sr/bestLag));
}

void VocalChainPlusAudioProcessor::applyDeEsser(juce::AudioBuffer<float>& b,float amount)
{
    if(amount<=0.001f) return;
    const float alpha=(float)std::exp(-2.0*juce::MathConstants<double>::pi*5200.0/sr);
    static float lpState[2]={0,0};
    for(int c=0;c<b.getNumChannels();++c){float lp=lpState[juce::jmin(c,1)]; for(int i=0;i<b.getNumSamples();++i){float x=b.getSample(c,i); lp=(1.0f-alpha)*x+alpha*lp; float hi=x-lp; deEssEnv=0.995f*deEssEnv+0.005f*std::abs(hi); float reduction=1.0f-juce::jlimit(0.0f,0.65f,(deEssEnv-0.015f)*amount*18.0f); b.setSample(c,i,lp+hi*reduction);} lpState[juce::jmin(c,1)]=lp;}
}

void VocalChainPlusAudioProcessor::applyWidth(juce::AudioBuffer<float>& b,float amount)
{
    if(b.getNumChannels()<2) return; float side=juce::jlimit(0.0f,2.0f,amount/50.0f);
    auto* l=b.getWritePointer(0); auto* r=b.getWritePointer(1); for(int i=0;i<b.getNumSamples();++i){float m=(l[i]+r[i])*0.5f,s=(l[i]-r[i])*0.5f*side;l[i]=m+s;r[i]=m-s;}
}

void VocalChainPlusAudioProcessor::applyPitchShift(juce::AudioBuffer<float>& b,float semitones,float blend)
{
    if(std::abs(semitones)<0.01f || blend<=0.001f || pitchBuffer.empty()) return;
    float ratio=std::pow(2.0f,semitones/12.0f), phaseInc=(ratio-1.0f)/2048.0f; int size=(int)pitchBuffer[0].size();
    auto interp=[&](const std::vector<float>& q,float pos){while(pos<0)pos+=size;while(pos>=size)pos-=size;int a=(int)pos,b=(a+1)%size;float f=pos-a;return q[(size_t)a]+(q[(size_t)b]-q[(size_t)a])*f;};
    for(int i=0;i<b.getNumSamples();++i){float ph1=pitchPhase-std::floor(pitchPhase),ph2=std::fmod(ph1+0.5f,1.0f);float w1=0.5f-0.5f*std::cos(juce::MathConstants<float>::twoPi*ph1),w2=0.5f-0.5f*std::cos(juce::MathConstants<float>::twoPi*ph2);for(int c=0;c<b.getNumChannels();++c){auto& q=pitchBuffer[(size_t)c];float x=b.getSample(c,i);q[(size_t)pitchWrite]=x;float d1=256.0f+ph1*1792.0f,d2=256.0f+ph2*1792.0f;float y=(interp(q,pitchWrite-d1)*w1+interp(q,pitchWrite-d2)*w2)/(w1+w2+1.0e-6f);b.setSample(c,i,x*(1.0f-blend)+y*blend);}pitchWrite=(pitchWrite+1)%size;pitchPhase+=phaseInc;if(pitchPhase>1)pitchPhase-=1;if(pitchPhase<0)pitchPhase+=1;}
}

float VocalChainPlusAudioProcessor::targetCorrectionSemitones(float hz) const
{
    if(hz<60.0f) return 0.0f; float midi=69.0f+12.0f*std::log2(hz/440.0f); int key=(int)apvts.getRawParameterValue("key")->load(); if(key==0) return std::round(midi)-midi; int root=key-1; bool minor=((int)apvts.getRawParameterValue("scale")->load()==0); const int maj[7]={0,2,4,5,7,9,11},minr[7]={0,2,3,5,7,8,10}; float best=99; for(int oct=-2;oct<=2;++oct)for(int i=0;i<7;++i){int pc=root+(minor?minr[i]:maj[i])+12*oct;float candidate=std::floor(midi/12.0f)*12.0f+pc;float d=candidate-midi;if(std::abs(d)<std::abs(best))best=d;} return juce::jlimit(-2.0f,2.0f,best);
}

void VocalChainPlusAudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&)
{
    juce::ScopedNoDenormals no; for(int c=getTotalNumInputChannels();c<getTotalNumOutputChannels();++c)b.clear(c,0,b.getNumSamples()); updateAnalysis(b); juce::AudioBuffer<float> dry; dry.makeCopyOf(b);
    b.applyGain(juce::Decibels::decibelsToGain(apvts.getRawParameterValue("input")->load()));

    float tune=apvts.getRawParameterValue("tune")->load()/100.0f; float manual=apvts.getRawParameterValue("pitch")->load(); float corr=targetCorrectionSemitones(pitchHz.load())*tune; applyPitchShift(b,manual+corr,juce::jlimit(0.0f,1.0f,std::max(tune,std::abs(manual)/12.0f)));
    applyDeEsser(b,apvts.getRawParameterValue("deess")->load()/100.0f);

    float eq=apvts.getRawParameterValue("eq")->load()/100.0f, air=apvts.getRawParameterValue("air")->load()/100.0f;
    *lowEq.state=*juce::dsp::IIR::Coefficients<float>::makeLowShelf(sr,180.0f,0.707f,juce::Decibels::decibelsToGain((eq-0.5f)*4.0f));
    *midEq.state=*juce::dsp::IIR::Coefficients<float>::makePeakFilter(sr,2500.0f,0.9f,juce::Decibels::decibelsToGain((eq-0.5f)*5.0f));
    *highEq.state=*juce::dsp::IIR::Coefficients<float>::makeHighShelf(sr,9000.0f,0.707f,juce::Decibels::decibelsToGain(air*7.0f));
    juce::dsp::AudioBlock<float> block(b); juce::dsp::ProcessContextReplacing<float> ctx(block); lowEq.process(ctx);midEq.process(ctx);highEq.process(ctx);

    float compAmt=apvts.getRawParameterValue("comp")->load()/100.0f; compressor.setThreshold(juce::jmap(compAmt,0.0f,1.0f,0.0f,apvts.getRawParameterValue("threshold")->load())); compressor.setRatio(juce::jmap(compAmt,0.0f,1.0f,1.0f,apvts.getRawParameterValue("ratio")->load())); compressor.setAttack(apvts.getRawParameterValue("attack")->load()); compressor.setRelease(apvts.getRawParameterValue("release")->load()); compressor.process(ctx);

    float sat=apvts.getRawParameterValue("sat")->load()/100.0f + apvts.getRawParameterValue("dist")->load()/70.0f; if(sat>0.001f) for(int c=0;c<b.getNumChannels();++c) for(int i=0;i<b.getNumSamples();++i) b.setSample(c,i,std::tanh(b.getSample(c,i)*(1.0f+sat*5.0f))/(1.0f+sat*0.6f));
    applyWidth(b,apvts.getRawParameterValue("width")->load());

    float ch=apvts.getRawParameterValue("chorus")->load()/100.0f; chorus.setRate(0.35f+ch*1.8f); chorus.setDepth(0.15f+ch*0.7f); chorus.setCentreDelay(8.0f); chorus.setFeedback(ch*0.35f); chorus.setMix(ch*0.55f); chorus.process(ctx);

    juce::dsp::Reverb::Parameters rp; float rv=apvts.getRawParameterValue("reverb")->load()/100.0f; rp.roomSize=0.15f+0.75f*rv;rp.damping=0.45f;rp.wetLevel=0.38f*rv;rp.dryLevel=1.0f-rp.wetLevel*0.35f;reverb.setParameters(rp); reverb.process(ctx);
    float dm=apvts.getRawParameterValue("delay")->load()/100.0f,fb=apvts.getRawParameterValue("delayfb")->load()/100.0f;delay.setDelay((float)(sr*0.25));for(int c=0;c<b.getNumChannels();++c)for(int i=0;i<b.getNumSamples();++i){float d=delay.popSample(c);float v=b.getSample(c,i);delay.pushSample(c,v+d*fb);b.setSample(c,i,v+d*dm*0.5f);}

    float mix=apvts.getRawParameterValue("mix")->load()/100.0f;for(int c=0;c<b.getNumChannels();++c)for(int i=0;i<b.getNumSamples();++i)b.setSample(c,i,dry.getSample(c,i)*(1-mix)+b.getSample(c,i)*mix);
    b.applyGain(juce::Decibels::decibelsToGain(apvts.getRawParameterValue("output")->load()));float r=0;for(int c=0;c<b.getNumChannels();++c)r+=b.getRMSLevel(c,0,b.getNumSamples());outputDb.store(dbFromRms(r/juce::jmax(1,b.getNumChannels())));
}

juce::String VocalChainPlusAudioProcessor::detectedNote() const {float f=pitchHz.load(); if(f<60) return "--"; int midi=(int)std::round(69+12*std::log2(f/440.0f)); static const char* n[]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"}; return juce::String(n[(midi%12+12)%12])+juce::String(midi/12-1);}
void VocalChainPlusAudioProcessor::loadPreset(int i){struct V{const char* id;float v;}; std::vector<V> v; if(i==0)v={{"comp",52},{"sat",18},{"air",42},{"reverb",24},{"delay",12},{"width",35}}; else if(i==1)v={{"comp",65},{"sat",45},{"air",18},{"reverb",32},{"delay",24},{"filter",58}}; else if(i==2)v={{"comp",38},{"sat",8},{"air",75},{"reverb",52},{"delay",18}}; else if(i==3)v={{"comp",70},{"sat",70},{"dist",55},{"reverb",15},{"delay",30}}; else if(i==4)v={{"width",85},{"reverb",38},{"delay",22},{"comp",45}}; else if(i==5)v={{"sat",55},{"filter",65},{"reverb",18},{"delay",28}}; else if(i==6)v={{"comp",48},{"sat",14},{"air",55},{"reverb",42},{"delay",16}}; for(auto& x:v) if(auto* p=apvts.getParameter(x.id)) p->setValueNotifyingHost(p->convertTo0to1(x.v));}
void VocalChainPlusAudioProcessor::getStateInformation(juce::MemoryBlock& d){auto s=apvts.copyState(); std::unique_ptr<juce::XmlElement> x(s.createXml()); copyXmlToBinary(*x,d);} void VocalChainPlusAudioProcessor::setStateInformation(const void* d,int n){std::unique_ptr<juce::XmlElement>x(getXmlFromBinary(d,n));if(x&&x->hasTagName(apvts.state.getType()))apvts.replaceState(juce::ValueTree::fromXml(*x));}
juce::AudioProcessorEditor* VocalChainPlusAudioProcessor::createEditor(){return new VocalChainPlusAudioProcessorEditor(*this);} juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new VocalChainPlusAudioProcessor();}
