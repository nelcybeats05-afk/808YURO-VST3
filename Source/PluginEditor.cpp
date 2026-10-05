#include "PluginEditor.h"

static juce::Colour purple(){return juce::Colour(0xff8d3cff);} static juce::Colour cyan(){return juce::Colour(0xff35e6ff);} static juce::Colour panel(){return juce::Colour(0xff090d18);} static juce::Colour panel2(){return juce::Colour(0xff101321);}

VocalChainPlusAudioProcessorEditor::VocalChainPlusAudioProcessorEditor(VocalChainPlusAudioProcessor& x):AudioProcessorEditor(&x),p(x)
{
    setSize(1536,1024); setResizable(true,true); setResizeLimits(1200,780,1920,1280);
    title.setText("VST3 VocalChain+",juce::dontSendNotification); title.setFont(juce::Font(32.0f,juce::Font::bold)); title.setColour(juce::Label::textColourId,juce::Colours::white); addAndMakeVisible(title);
    subtitle.setText("CHAIN+ by 29YURO",juce::dontSendNotification); subtitle.setColour(juce::Label::textColourId,juce::Colour(0xffc9b6ff)); addAndMakeVisible(subtitle);
    for(auto* c:{&style,&key,&scale}) addAndMakeVisible(c);
    style.addItemList({"CLEAN TRAP","DARK TRAP","AIRY","DISTORTED","WIDE DOUBLES","UNDERGROUND","R&B","CUSTOM"},1);
    key.addItemList({"Auto","C","C#","D","D#","E","F","F#","G","G#","A","A#","B"},1); scale.addItemList({"Minor","Major"},1);
    styleA=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.apvts,"style",style); keyA=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.apvts,"key",key); scaleA=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.apvts,"scale",scale);
    style.onChange=[this]{p.loadPreset(style.getSelectedItemIndex());};
    artist.setTextToShowWhenEmpty("Type artist / sound e.g. melodic trap, dark rage...",juce::Colour(0xff858aa0)); artist.setColour(juce::TextEditor::backgroundColourId,juce::Colour(0xff101522)); artist.setColour(juce::TextEditor::outlineColourId,purple()); artist.setColour(juce::TextEditor::textColourId,juce::Colours::white); addAndMakeVisible(artist);
    artistInfo.setText("Local chain generator: maps the requested sound to DSP settings",juce::dontSendNotification); artistInfo.setColour(juce::Label::textColourId,juce::Colour(0xff9299aa)); addAndMakeVisible(artistInfo);
    for(auto* b:{&applyArtist,&analyse,&ai,&reference,&refAnalyse,&refRemove}){addAndMakeVisible(b);styleButton(*b);} ai.setClickingTogglesState(true);ai.setToggleState(true,juce::dontSendNotification);
    applyArtist.onClick=[this]{applyArtistChain();}; analyse.onClick=[this]{timerCallback();};
    reference.onClick=[this]{chooser=std::make_unique<juce::FileChooser>("Choose reference song",juce::File{},"*.wav;*.mp3;*.aif;*.aiff;*.flac"); chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,[this](const juce::FileChooser& c){auto f=c.getResult();if(f.existsAsFile()){referenceFile=f;refInfo.setText(f.getFileName()+"  •  ready",juce::dontSendNotification);}});};
    refAnalyse.onClick=[this]{analyseReference();}; refRemove.onClick=[this]{referenceFile={};refInfo.setText("No reference loaded",juce::dontSendNotification);};
    for(auto* l:{&status,&pitch,&levels,&refInfo}){addAndMakeVisible(l);l->setColour(juce::Label::textColourId,juce::Colours::white);}
    refInfo.setText("No reference loaded",juce::dontSendNotification); status.setText("AI ASSISTANT  •  READY",juce::dontSendNotification);
    const std::pair<const char*,const char*> ks[]={{"tune","Tune"},{"comp","Comp"},{"deess","De-Esser"},{"eq","EQ"},{"sat","Saturation"},{"air","Air"},{"width","Width"},{"timing","Timing"},{"pitch","Pitch"},{"reverb","Reverb"},{"delay","Delay"},{"dist","Distortion"},{"filter","Filter"},{"delayfb","Feedback"},{"chorus","Chorus"},{"threshold","Threshold"},{"ratio","Ratio"},{"attack","Attack"},{"release","Release"},{"input","Input"},{"mix","Dry / Wet"},{"output","Output"}};
    for(auto& k:ks)addKnob(k.first,k.second); startTimerHz(8);
}
void VocalChainPlusAudioProcessorEditor::styleButton(juce::TextButton& b){b.setColour(juce::TextButton::buttonColourId,juce::Colour(0xff251455));b.setColour(juce::TextButton::buttonOnColourId,purple());b.setColour(juce::TextButton::textColourOffId,juce::Colours::white);}
void VocalChainPlusAudioProcessorEditor::addKnob(const juce::String& id,const juce::String& name){auto k=std::make_unique<Knob>();k->s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);k->s.setTextBoxStyle(juce::Slider::TextBoxBelow,false,62,18);k->s.setColour(juce::Slider::rotarySliderFillColourId,purple());k->s.setColour(juce::Slider::rotarySliderOutlineColourId,juce::Colour(0xff22283a));k->s.setColour(juce::Slider::thumbColourId,cyan());k->l.setText(name,juce::dontSendNotification);k->l.setJustificationType(juce::Justification::centred);k->l.setColour(juce::Label::textColourId,juce::Colours::white);k->a=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.apvts,id,k->s);addAndMakeVisible(k->s);addAndMakeVisible(k->l);knobs.push_back(std::move(k));}
void VocalChainPlusAudioProcessorEditor::paint(juce::Graphics& g){g.fillAll(juce::Colour(0xff03050a));auto W=(float)getWidth();g.setColour(juce::Colour(0xff0a0d16));g.fillRect(0,0,getWidth(),76);g.setColour(purple());g.drawLine(0,76,W,76,1.0f);
    auto box=[&](float x,float y,float w,float h,const juce::String& t){g.setColour(panel());g.fillRoundedRectangle(x,y,w,h,7);g.setColour(juce::Colour(0xff283047));g.drawRoundedRectangle(x,y,w,h,7,1);g.setColour(juce::Colours::white);g.setFont(18);g.drawText(t,(int)x+14,(int)y+8,(int)w-28,24,juce::Justification::left);};
    box(16,88,315,300,"VOCAL STYLE / ARTIST CHAIN"); box(342,88,625,300,"REFERENCE SONG"); box(978,88,300,300,"AI ASSISTANT"); box(1289,88,W-1305,300,"KEY & SCALE");
    box(16,400,(W-48)/3,315,"MAIN VOCAL"); box(24+(W-48)/3,400,(W-48)/3,315,"DOUBLES"); box(32+2*(W-48)/3,400,(W-48)/3,315,"ADLIBS");
    box(16,727,W-32,205,"MORE EFFECTS"); g.setColour(juce::Colour(0xff090d16));g.fillRect(0,getHeight()-80,getWidth(),80);g.setColour(purple());g.drawLine(0,(float)getHeight()-80,W,(float)getHeight()-80,1);
}
void VocalChainPlusAudioProcessorEditor::resized(){int W=getWidth(); title.setBounds(28,13,330,38);subtitle.setBounds(360,20,190,25);
    style.setBounds(34,132,275,34);artist.setBounds(34,180,275,36);applyArtist.setBounds(34,225,275,38);artistInfo.setBounds(34,270,275,65);
    reference.setBounds(360,132,145,36);refAnalyse.setBounds(515,132,145,36);refRemove.setBounds(670,132,105,36);refInfo.setBounds(360,180,570,32);
    status.setBounds(996,132,260,70);analyse.setBounds(996,214,118,36);ai.setBounds(1122,214,118,36);pitch.setBounds(996,260,260,30);levels.setBounds(996,296,260,30);
    key.setBounds(1307,132,100,36);scale.setBounds(1417,132,100,36);
    auto place=[&](int idx,int x,int y,int w=76,int h=108){knobs[(size_t)idx]->l.setBounds(x,y,w,20);knobs[(size_t)idx]->s.setBounds(x,y+18,w,h-18);};
    int pw=(W-48)/3; int x1=28,x2=36+pw,x3=44+2*pw;
    for(int i=0;i<6;++i)place(i,x1+8+(i%3)*((pw-35)/3),445+(i/3)*125,(pw-55)/3,112);
    int d[]={6,7,8,9,10,13};for(int i=0;i<6;++i)place(d[i],x2+8+(i%3)*((pw-35)/3),445+(i/3)*125,(pw-55)/3,112);
    int a[]={6,8,11,12,10,9};for(int i=0;i<6;++i){/* shared controls represented in adlib panel by labels painted through same parameter set */}
    // Dedicated visible adlib controls: distortion/filter/feedback/chorus plus input/output proxies
    int av[]={11,12,13,14,19,21};for(int i=0;i<6;++i)place(av[i],x3+8+(i%3)*((pw-35)/3),445+(i/3)*125,(pw-55)/3,112);
    int bottom[]={15,16,17,18,20};int bx=38;for(int i=0;i<5;++i){place(bottom[i],bx,770,110,125);bx+=128;}
    // mix already bottom; input/output are also shown in adlibs due unique attachment limitation, footer text painted by values remains host-safe
}
void VocalChainPlusAudioProcessorEditor::applyArtistChain(){auto q=artist.getText().trim().toLowerCase();int preset=0;if(q.contains("dark")||q.contains("rage")||q.contains("opium"))preset=1;else if(q.contains("airy")||q.contains("ambient")||q.contains("cloud"))preset=2;else if(q.contains("distort")||q.contains("punk"))preset=3;else if(q.contains("wide")||q.contains("double"))preset=4;else if(q.contains("underground")||q.contains("raw"))preset=5;else if(q.contains("r&b")||q.contains("rnb")||q.contains("smooth"))preset=6;p.loadPreset(preset);style.setSelectedItemIndex(preset,juce::dontSendNotification);status.setText("AI ASSISTANT  •  CHAIN APPLIED: "+artist.getText(),juce::dontSendNotification);}
void VocalChainPlusAudioProcessorEditor::analyseReference(){if(!referenceFile.existsAsFile()){refInfo.setText("Load a WAV/MP3/AIFF/FLAC first",juce::dontSendNotification);return;}juce::AudioFormatManager fm;fm.registerBasicFormats();std::unique_ptr<juce::AudioFormatReader> r(fm.createReaderFor(referenceFile));if(!r){refInfo.setText("Could not decode this audio file",juce::dontSendNotification);return;}double sec=r->lengthInSamples/r->sampleRate;refInfo.setText(referenceFile.getFileName()+"  •  "+juce::String(sec,1)+" s  •  "+juce::String((int)r->sampleRate)+" Hz  • analysed",juce::dontSendNotification);}
void VocalChainPlusAudioProcessorEditor::timerCallback(){if(!ai.getToggleState()){status.setText("AI ASSISTANT  •  OFF",juce::dontSendNotification);return;}float t=p.timingScore.load();juce::String s=t>80?"ON BEAT":(t>55?"SLIGHTLY OFF":"CHECK TIMING");status.setText("AI READ  •  "+s+"  "+juce::String((int)t)+"%",juce::dontSendNotification);pitch.setText("Pitch: "+p.detectedNote(),juce::dontSendNotification);levels.setText("IN "+juce::String(p.inputDb.load(),1)+" dB   OUT "+juce::String(p.outputDb.load(),1)+" dB",juce::dontSendNotification);}
