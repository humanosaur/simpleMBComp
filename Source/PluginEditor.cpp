/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
//==============================================================================

template<typename T>
bool truncateKiloValue(T& value)
{
    if( value > static_cast<T>(999) )
    {
        value /= static_cast<T>(1000);
        return true;
    }
    return false;
}

juce::String getValString(const juce::RangedAudioParameter& param, bool getLow, juce::String suffix)
{
    juce::String str;
    
    auto val = (getLow ? param.getNormalisableRange().start : param.getNormalisableRange().end);
    
    bool useK = truncateKiloValue(val);
    
    str << val;
    
    if( useK )
        str << "k";
    
    str << suffix;
    
    return str;
}

void drawModuleBackground(juce::Graphics& g,
                           juce::Rectangle<int> bounds)
{
    using namespace juce;
    
    g.setColour(Colours::blueviolet);
    g.fillAll();
    
    auto localBounds = bounds;
    
    bounds.reduce(3, 3);
    g.setColour(Colours::black);
    g.fillRoundedRectangle(bounds.toFloat(), 3);
    
    g.drawRect(localBounds);
}

void LookAndFeel::drawRotarySlider(juce::Graphics& g,
                                   int x,
                                   int y,
                                   int width,
                                   int height,
                                   float sliderPosProportional,
                                   float rotaryStartAngle,
                                   float rotaryEndAngle,
                                   juce::Slider & slider)
{
    using namespace juce;
    
    auto bounds = Rectangle<float>(x, y, width, height);
    
    auto enabled = slider.isEnabled();
    
    //create and fill a circle
    g.setColour(enabled ? Colour(327.f, 62.f, 75.f, 0.3f) : Colours::dimgrey);
    g.fillEllipse(bounds);
    
    //draw a border around the circle
    g.setColour(Colours::black);
    g.drawEllipse(bounds, 0.5f);
    
    if ( auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider))
    {
        //create a narrow rectangle to represent the indicator of the rotary dial
        auto center = bounds.getCentre();
        
        Path p;
        
        Rectangle<float> r;
        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setTop(bounds.getY());
        r.setBottom(center.getY() - rswl->getTextHeight() * 2);
        
        g.setColour(Colours::black);
        p.addRoundedRectangle(r,2.f);
        
        jassert(rotaryStartAngle < rotaryEndAngle);
        
        //convert slider's normalized value to an angle in radians
        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);
        
        //rotate the rectangle to the angle we just calculated
        p.applyTransform(AffineTransform().rotated(sliderAngRad,center.getX(), center.getY()));
        
        g.fillPath(p);
        
        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);
        
        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(bounds.getCentre());
        
        //g.setColour(Colours::blanchedalmond);
        //g.fillRect(r);
        
        g.setColour(Colours::black);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
    }
}

void LookAndFeel::drawToggleButton(juce::Graphics &g,
                                   juce::ToggleButton &toggleButton,
                                   bool shouldDrawButtonAsHighlighted,
                                   bool shouldDrawButtonAsDown)
{
    using namespace juce;
    
    if( auto* pb = dynamic_cast<PowerButton*>(&toggleButton))
    {
        Path powerButton;
        
        auto bounds = toggleButton.getLocalBounds();
        
        g.setColour(Colour(105u,60u,28u));
        g.fillRect(bounds);
        g.setColour(Colours::black);
        g.drawRect(bounds);
        
        auto size = jmin(bounds.getWidth(),bounds.getHeight()) - 6; //JUCE_LIVE_CONSTANT(6);
        auto r = bounds.withSizeKeepingCentre(size, size).toFloat();
        
        float ang = 24.f; //JUCE_LIVE_CONSTANT(30);
        
        size -= 7; //JUCE_LIVE_CONSTANT(6);
        
        powerButton.addCentredArc(r.getCentreX(),
                                  r.getCentreY(),
                                  size * 0.5,
                                  size * 0.5,
                                  0.f,
                                  degreesToRadians(ang),
                                  degreesToRadians(360.f - ang),
                                  true);
        
        powerButton.startNewSubPath(r.getCentreX(), r.getY());
        powerButton.lineTo(r.getCentre());
        
        PathStrokeType pst(1.f,PathStrokeType::JointStyle::curved);
        
        auto color = toggleButton.getToggleState() ? Colours::dimgrey : Colour(242u, 65u, 163u);
        
        g.setColour(color);
        g.strokePath(powerButton, pst);
        g.drawEllipse(r,1.5);
    }
    else if( auto* analyzerButton = dynamic_cast<AnalyzerButton*>(&toggleButton))
    {
        auto color = ! toggleButton.getToggleState() ? Colours::dimgrey : Colour(242u, 65u, 163u);
        
        g.setColour(color);
        
        auto bounds = toggleButton.getLocalBounds();
        g.drawRect(bounds);
        
        g.strokePath(analyzerButton->randomPath, PathStrokeType(1.f));
    }
    else
    {
        auto bounds = toggleButton.getLocalBounds().reduced(2);
        
        auto buttonIsOn = toggleButton.getToggleState();
        
        const int cornerSize = 4;
        
        g.setColour(buttonIsOn ? juce::Colours::white : juce::Colours::black);
        g.fillRoundedRectangle(bounds.toFloat(), cornerSize); //background
        
        g.setColour(buttonIsOn ? juce::Colours::black : juce::Colours::white);
        g.drawRoundedRectangle(bounds.toFloat(), cornerSize, 1); //border
        g.drawFittedText(toggleButton.getName(), bounds, juce::Justification::centred, 1); //text
        
    }
}


void RotarySliderWithLabels::paint(juce::Graphics &g)
{
    using namespace juce;
    
    //set the starting point (0) at roughly 7 o'clock
    auto startAng = degreesToRadians(180.f + 45.f);
    
    //set the end point (1) at roughly 5 o'clock
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;
    
    auto range = getRange();
    
    auto sliderBounds = getSliderBounds();
    
    auto bounds = getLocalBounds();
    
    g.setColour(Colours::skyblue);
    g.drawFittedText(getName(), bounds.removeFromTop(getTextHeight() + 2), Justification::centredBottom, 1);
    
// these are the bounding boxes around the sliders, leaving in for debugging
//    g.setColour(Colours::red);
//    g.drawRect(getLocalBounds());
//    g.setColour(Colours::yellow);
//    g.drawRect(sliderBounds);
    
    getLookAndFeel().drawRotarySlider(g,
                                      sliderBounds.getX(),
                                      sliderBounds.getY(),
                                      sliderBounds.getWidth(),
                                      sliderBounds.getHeight(),
                                      jmap(getValue(),range.getStart(),range.getEnd(),0.0,1.0),
                                      startAng,
                                      endAng,
                                      *this);
    
    //create a bounding box for our min/max label text, centered on a normalized point we decide
    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth() * 0.5f;
    
    g.setColour(Colour(105u,60u,28u)); //kinda brown
    g.setFont(getTextHeight());
    
    auto numChoices = labels.size();
    for( int i = 0; i < numChoices; ++i)
    {
        auto pos = labels[i].pos;
        jassert(0.f <= pos);
        jassert(pos <= 1.f);
        
        auto ang = jmap(pos, 0.f, 1.f, startAng, endAng);
        
        //define the point to center the bounding box on
        
        //this point is at the edge of the rotary slider
        auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5f, ang);
        
        //this gets us a little further out and down from the edge of the rotary slider
        Rectangle<float> r;
        auto str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY()+ getTextHeight());
        
        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
    }
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    auto bounds = getLocalBounds();
    
    bounds.removeFromTop(getTextHeight()* 1.5);
    
    auto size = juce::jmin(bounds.getWidth(),bounds.getHeight());
    
    size -= getTextHeight() * 1.5;
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(bounds.getY());
    
    return r;
}

juce::String RotarySliderWithLabels::getDisplayString() const
{
    if( auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param) )
        return choiceParam->getCurrentChoiceName();
    
    juce::String str;
    bool addK = false;
    
    //even though we haven't added any parameter types other than float, we'll check just in case
    if( auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param) )
    {
        float val = getValue();
        
        //check if over 1000 and if so, add k to kHz
        //don't need to check which value it is since dBs will never go over 1000

        addK = truncateKiloValue(val);
        
        str = juce::String(val, (addK ? 2 : 0));
    }
    else{
        jassertfalse; //this shouldn't happen!
    }
    
    if( suffix.isNotEmpty() )
    {
        str << " ";
        if( addK )
            str << "k";
        str << suffix;
    }
    return str;
}

void RotarySliderWithLabels::changeParam(juce::RangedAudioParameter *p)
{
    param = p;
    repaint();
}

Placeholder::Placeholder()
{
    juce::Random r;
    customColor = juce::Colour(r.nextInt(255),r.nextInt(255),r.nextInt(255));
}

GlobalControls::GlobalControls(juce::AudioProcessorValueTreeState& apvts)
{
    using namespace Params;
    using RSWL = RotarySliderWithLabels;
    const auto& params = GetParams();
    
    // Define our helper functions
    
    auto getParameterHelper = [&apvts, &params](const auto& pos) -> auto&
    {
        return getParam(pos, apvts, params);
    };
    
    auto& gainInParam = getParameterHelper(Names::Gain_In);
    auto& lowMidParam = getParameterHelper(Names::Low_Mid_Crossover_Freq);
    auto& midHighParam = getParameterHelper(Names::Mid_High_Crossover_Freq);
    auto& gainOutParam = getParameterHelper(Names::Gain_Out);
    
    auto makeAttachmentHelper = [&params, &apvts](auto& attachment, const auto&name, auto& slider)
    {
        makeAttachment(attachment, name, slider, params, apvts);
    };
    
    //Use our helper functions
    
    //Initialize RSWLs
    
    inGainSlider = std::make_unique<RSWL>( &gainInParam,
                                          "dB",
                                          "INPUT TRIM");
    
    lowMidXoverSlider = std::make_unique<RSWL>( &lowMidParam,
                                               "Hz",
                                               "LOW-MID X-OVER");
    
    midHighXoverSlider = std::make_unique<RSWL>( &midHighParam,
                                                "Hz",
                                                "MID-HIGH X-OVER");
    
    outGainSlider = std::make_unique<RSWL>( &gainOutParam,
                                            "dB",
                                           "OUTPUT TRIM");
    
    //Attach RSWLs to parameters
    
    makeAttachmentHelper(inGainSliderAttachment,
                         Names::Gain_In,
                         *inGainSlider);
    
    makeAttachmentHelper(lowMidXoverSliderAttachment,
                         Names::Low_Mid_Crossover_Freq,
                         *lowMidXoverSlider);
    
    makeAttachmentHelper(midHighXoverSliderAttachment,
                         Names::Mid_High_Crossover_Freq,
                         *midHighXoverSlider);
    
    makeAttachmentHelper(outGainSliderAttachment,
                         Names::Gain_Out,
                         *outGainSlider);
    
    //Add max/min label pairs
    
    addLabelPairs(inGainSlider->labels,
                  gainInParam,
                  "dB");
    
    addLabelPairs(lowMidXoverSlider->labels,
                  lowMidParam,
                  "Hz");
    
    addLabelPairs(midHighXoverSlider->labels,
                  midHighParam,
                  "Hz");
    
    addLabelPairs(outGainSlider->labels,
                  gainOutParam,
                  "dB");
    
    
    //Add and make visible
    
    addAndMakeVisible(*inGainSlider);
    addAndMakeVisible(*lowMidXoverSlider);
    addAndMakeVisible(*midHighXoverSlider);
    addAndMakeVisible(*outGainSlider);
}

void GlobalControls::paint(juce::Graphics& g)
{
    //using namespace juce;
    auto bounds = getLocalBounds();
    
    drawModuleBackground(g, bounds);
};

void GlobalControls::resized()
{
    auto bounds = getLocalBounds().reduced(5);
    
    using namespace juce;
    
    FlexBox flexBox;
    flexBox.flexDirection = FlexBox::Direction::row;
    flexBox.flexWrap = FlexBox::Wrap::noWrap;
    
    auto spacer = FlexItem().withWidth(4);
    auto endCap = FlexItem().withWidth(6);
    
    flexBox.items.add(endCap);
    flexBox.items.add(FlexItem(*inGainSlider).withFlex(1));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(*lowMidXoverSlider).withFlex(1));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(*midHighXoverSlider).withFlex(1));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(*outGainSlider).withFlex(1));
    flexBox.items.add(spacer);
    flexBox.items.add(endCap);
    
    flexBox.performLayout(bounds);
}

CompressorBandControls::CompressorBandControls(juce::AudioProcessorValueTreeState& apvts) :
apvts(apvts),
attackSlider(nullptr, "ms", "ATTACK"),
releaseSlider(nullptr, "ms", "RELEASE"),
thresholdSlider(nullptr, "dB", "THRESHOLD"),
ratioSlider(nullptr, "")
{
//    using namespace Params;
//    //using RSWL = RotarySliderWithLabels;
//    const auto& params = GetParams();
//
//    //Parameters
//
//    auto getParameterHelper = [&apvts, &params](const auto& pos) -> auto&
//    {
//        return getParam(pos, apvts, params);
//    };
//
//    attackSlider.changeParam(&getParameterHelper(Names::Attack_Mid_Band));
//    releaseSlider.changeParam(&getParameterHelper(Names::Release_Mid_Band));
//    thresholdSlider.changeParam(&getParameterHelper(Names::Threshold_Mid_Band));
//    ratioSlider.changeParam(&getParameterHelper(Names::Ratio_Mid_Band));
    
    
    //Attachments
    
//    auto makeAttachmentHelper = [&params, &apvts](auto& attachment, const auto&name, auto& slider)
//    {
//        makeAttachment(attachment, name, slider, params, apvts);
//    };
    
//    makeAttachmentHelper(attackSliderAttachment,
//                         Names::Attack_Mid_Band,
//                         attackSlider);
//
//    makeAttachmentHelper(releaseSliderAttachment,
//                         Names::Release_Mid_Band,
//                         releaseSlider);
//
//    makeAttachmentHelper(thresholdSliderAttachment,
//                         Names::Threshold_Mid_Band,
//                         thresholdSlider);
//
//    makeAttachmentHelper(ratioSliderAttachment,
//                         Names::Ratio_Mid_Band,
//                         ratioSlider);
    
//    makeAttachmentHelper(bypassButtonAttachment,
//                         Names::Bypassed_Mid_Band,
//                         bypassButton);
//
//    makeAttachmentHelper(soloButtonAttachment,
//                         Names::Solo_Mid_Band,
//                         soloButton);
//
//    makeAttachmentHelper(muteButtonAttachment,
//                         Names::Mute_Mid_Band,
//                         muteButton);
    
    //Add max/min label pairs
    
//    addLabelPairs(attackSlider.labels, getParameterHelper(Names::Attack_Mid_Band), "ms");
//    addLabelPairs(releaseSlider.labels, getParameterHelper(Names::Release_Mid_Band), "ms");
//    addLabelPairs(thresholdSlider.labels, getParameterHelper(Names::Threshold_Mid_Band), "dB");

    //The ratio slider needs a little extra help
//    ratioSlider.labels.add({0, "1:1"});
//    auto ratioParam = dynamic_cast<juce::AudioParameterChoice*>(&getParameterHelper(Names::Ratio_Mid_Band));
//    ratioSlider.labels.add({1, juce::String(ratioParam->choices.getReference(ratioParam->choices.size() - 1).getIntValue()) + ":1"});
    
    
    //Add and make visible
    
    addAndMakeVisible(attackSlider);
    addAndMakeVisible(releaseSlider);
    addAndMakeVisible(thresholdSlider);
    addAndMakeVisible(ratioSlider);
    
    bypassButton.setName("X");
    soloButton.setName("S");
    muteButton.setName("M");
    
    addAndMakeVisible(bypassButton);
    addAndMakeVisible(soloButton);
    addAndMakeVisible(muteButton);
    
    lowBandButton.setName("Low");
    midBandButton.setName("Mid");
    highBandButton.setName("High");
    
    lowBandButton.setRadioGroupId(1);
    midBandButton.setRadioGroupId(1);
    highBandButton.setRadioGroupId(1);
    
    auto buttonSwitcher = [safePtr = this->safePtr]()
    {
      if (auto* c = safePtr.getComponent())
      {
          c->updateAttachments();
      }
    };
    
    lowBandButton.onClick = buttonSwitcher;
    midBandButton.onClick = buttonSwitcher;
    highBandButton.onClick = buttonSwitcher;
    
    lowBandButton.setToggleState(true, juce::NotificationType::dontSendNotification);
    
    updateAttachments();
    
    addAndMakeVisible(lowBandButton);
    addAndMakeVisible(midBandButton);
    addAndMakeVisible(highBandButton);
}

void CompressorBandControls::paint(juce::Graphics& g)
{
    //using namespace juce;
    auto bounds = getLocalBounds();
    
    drawModuleBackground(g, bounds);
};

void CompressorBandControls::updateAttachments()
{
    enum BandType
    {
        Low,
        Mid,
        High
    };
    
    BandType bandType = [this]()
    {
        if( lowBandButton.getToggleState() )
            return  BandType::Low;
        
        if( midBandButton.getToggleState() )
            return  BandType::Mid;
        
        return  BandType::High;
    }();
    
    using namespace Params;
    
    std::vector<Params::Names> names;
    
    switch (bandType)
    {
        case Low:
        {
            names = std::vector<Params::Names>
            {
                Names::Attack_Low_Band,
                Names::Release_Low_Band,
                Names::Threshold_Low_Band,
                Names::Ratio_Low_Band,
                Names::Bypassed_Low_Band,
                Names::Solo_Low_Band,
                Names::Mute_Low_Band,
            };
            
            break;
        }
        case Mid:
        {
            names = std::vector<Params::Names>
            {
                Names::Attack_Mid_Band,
                Names::Release_Mid_Band,
                Names::Threshold_Mid_Band,
                Names::Ratio_Mid_Band,
                Names::Bypassed_Mid_Band,
                Names::Solo_Mid_Band,
                Names::Mute_Mid_Band,
            };
            
            break;
        }
        case High:
        {
            names = std::vector<Params::Names>
            {
                Names::Attack_High_Band,
                Names::Release_High_Band,
                Names::Threshold_High_Band,
                Names::Ratio_High_Band,
                Names::Bypassed_High_Band,
                Names::Solo_High_Band,
                Names::Mute_High_Band,
            };
            
            break;
        }
    }
    
    enum Pos
    {
        Attack,
        Release,
        Threshold,
        Ratio,
        Bypass,
        Solo,
        Mute,
    };
    
    const auto& params = GetParams();
    
    auto getParameterHelper = [&apvts = this->apvts, &params, &names](const auto& pos) -> auto&
    {
        return getParam(names.at(pos), apvts, params);
    };
    
    attackSliderAttachment.reset();
    releaseSliderAttachment.reset();
    thresholdSliderAttachment.reset();
    ratioSliderAttachment.reset();
    bypassButtonAttachment.reset();
    soloButtonAttachment.reset();
    muteButtonAttachment.reset();
    
    auto& attackParam = getParameterHelper(Pos::Attack);
    addLabelPairs(attackSlider.labels, attackParam, "ms");
    attackSlider.changeParam(&attackParam);
    
    auto& releaseParam = getParameterHelper(Pos::Release);
    addLabelPairs(releaseSlider.labels, releaseParam, "ms");
    releaseSlider.changeParam(&releaseParam);
    
    auto& thresholdParam = getParameterHelper(Pos::Threshold);
    addLabelPairs(thresholdSlider.labels, thresholdParam, "dB");
    thresholdSlider.changeParam(&thresholdParam);
    
    auto& ratioParamRap = getParameterHelper(Pos::Ratio);
    ratioSlider.labels.add({0, "1:1"});
    
    auto ratioParam = dynamic_cast<juce::AudioParameterChoice*>(&ratioParamRap);
    ratioSlider.labels.add({1, juce::String(ratioParam->choices.getReference(ratioParam->choices.size() - 1).getIntValue()) + ":1"});
    ratioSlider.changeParam(&ratioParamRap);
    
    makeAttachment(attackSliderAttachment, names[Pos::Attack], attackSlider, params, apvts);
    makeAttachment(releaseSliderAttachment, names[Pos::Release], releaseSlider, params, apvts);
    makeAttachment(thresholdSliderAttachment, names[Pos::Threshold], thresholdSlider, params, apvts);
    makeAttachment(ratioSliderAttachment, names[Pos::Ratio], ratioSlider, params, apvts);
    makeAttachment(bypassButtonAttachment, names[Pos::Bypass], bypassButton, params, apvts);
    makeAttachment(soloButtonAttachment, names[Pos::Solo], soloButton, params, apvts);
    makeAttachment(muteButtonAttachment, names[Pos::Mute], muteButton, params, apvts);
}

void CompressorBandControls::resized()
{
    auto bounds = getLocalBounds().reduced(5);
    
    using namespace juce;
    
    auto createBandControlBox = [](std::vector<Component*> comps)
    {
        FlexBox flexBox;
        flexBox.flexDirection = FlexBox::Direction::column;
        flexBox.flexWrap = FlexBox::Wrap::noWrap;
        
        auto spacer = FlexItem().withHeight(2);
        
        for( auto* comp : comps)
        {
            flexBox.items.add(spacer);
            flexBox.items.add(FlexItem(*comp).withFlex(1));
        }
        
        flexBox.items.add(spacer);
        
        return flexBox;
    };
    
    auto bandButtonControlBox = createBandControlBox({&bypassButton, &soloButton, &muteButton});
    
    auto bandSelectControlBox = createBandControlBox({&lowBandButton, &midBandButton, &highBandButton});
    
    FlexBox flexBox;
    flexBox.flexDirection = FlexBox::Direction::row;
    flexBox.flexWrap = FlexBox::Wrap::noWrap;
    
    auto spacer = FlexItem().withWidth(4);
//    auto endCap = FlexItem().withWidth(6);
    
    flexBox.items.add(FlexItem(bandSelectControlBox).withWidth(50));
    
//    flexBox.items.add(endCap);
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(attackSlider).withFlex(1));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(releaseSlider).withFlex(1));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(thresholdSlider).withFlex(1));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(ratioSlider).withFlex(1));
    flexBox.items.add(spacer);
//    flexBox.items.add(endCap);
    
    flexBox.items.add(FlexItem(bandButtonControlBox).withWidth(30));
    
    flexBox.performLayout(bounds);
}

juce::String RatioSlider::getDisplayString() const
{
    auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param);
    jassert(choiceParam != nullptr);
    
    auto currentChoice = choiceParam->getCurrentChoiceName();
    
    if (currentChoice.contains(".0"))
        currentChoice = currentChoice.substring(0, currentChoice.indexOf("."));
    
    currentChoice << ":1";
    
    return currentChoice;
}

//==============================================================================
//==============================================================================

//==============================================================================
SimpleMBCompAudioProcessorEditor::SimpleMBCompAudioProcessorEditor (SimpleMBCompAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    
    //==============================================================================
    //==============================================================================
    
    //addAndMakeVisible(controlBar);
    //addAndMakeVisible(analyzer);
    addAndMakeVisible(globalControls);
    addAndMakeVisible(bandControls);
    
    setLookAndFeel(&lnf);
    
    //==============================================================================
    //==============================================================================
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 500);
}

SimpleMBCompAudioProcessorEditor::~SimpleMBCompAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void SimpleMBCompAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

//    g.setColour (juce::Colours::white);
//    g.setFont (15.0f);
//    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void SimpleMBCompAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    //==============================================================================
    //==============================================================================
    
    auto bounds = getLocalBounds();
    
    controlBar.setBounds(bounds.removeFromTop(35));
    bandControls.setBounds(bounds.removeFromBottom(125));
    analyzer.setBounds(bounds.removeFromTop(215));
    globalControls.setBounds(bounds);
    
    
    //==============================================================================
    //==============================================================================
}
