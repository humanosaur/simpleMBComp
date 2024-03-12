/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
//==============================================================================

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
    auto size = juce::jmin(bounds.getWidth(),bounds.getHeight());
    
    size -= getTextHeight() * 2;
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);
    
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
        if( val > 999.f )
        {
            val /= 1000.f;
            addK = true;
        }
        
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

Placeholder::Placeholder()
{
    juce::Random r;
    customColor = juce::Colour(r.nextInt(255),r.nextInt(255),r.nextInt(255));
}

GlobalControls::GlobalControls(juce::AudioProcessorValueTreeState& apvts)
{
    addAndMakeVisible(inGainSlider);
    addAndMakeVisible(lowMidXoverSlider);
    addAndMakeVisible(midHighXoverSlider);
    addAndMakeVisible(outGainSlider);
    
    using namespace Params;
    const auto& params = GetParams();
    
    auto makeAttachmentHelper = [&params, &apvts](auto& attachment, const auto&name, auto& slider)
    {
        makeAttachment(attachment, name, slider, params, apvts);
    };
    
    makeAttachmentHelper(inGainSliderAttachment,
                         Names::Gain_In,
                         inGainSlider);
    
    makeAttachmentHelper(lowMidXoverSliderAttachment,
                         Names::Low_Mid_Crossover_Freq,
                         lowMidXoverSlider);
    
    makeAttachmentHelper(midHighXoverSliderAttachment,
                         Names::Mid_High_Crossover_Freq,
                         midHighXoverSlider);
    
    makeAttachmentHelper(outGainSliderAttachment,
                         Names::Gain_Out,
                         outGainSlider);
}

void GlobalControls::paint(juce::Graphics& g)
{
    using namespace juce;
    auto bounds = getLocalBounds();
    
    g.setColour(Colours::blueviolet);
    g.fillAll();
    
    auto localBounds = bounds;
    
    bounds.reduce(3, 3);
    g.setColour(Colours::black);
    g.fillRoundedRectangle(bounds.toFloat(), 3);
    
    g.drawRect(localBounds);
};

void GlobalControls::resized()
{
    auto bounds = getLocalBounds();
    
    using namespace juce;
    
    FlexBox flexBox;
    flexBox.flexDirection = FlexBox::Direction::row;
    flexBox.flexWrap = FlexBox::Wrap::noWrap;
    
    flexBox.items.add(FlexItem(inGainSlider).withFlex(1));
    flexBox.items.add(FlexItem(lowMidXoverSlider).withFlex(1));
    flexBox.items.add(FlexItem(midHighXoverSlider).withFlex(1));
    flexBox.items.add(FlexItem(outGainSlider).withFlex(1));
    
    flexBox.performLayout(bounds);
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
    //addAndMakeVisible(bandControls);
    
    //==============================================================================
    //==============================================================================
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 500);
}

SimpleMBCompAudioProcessorEditor::~SimpleMBCompAudioProcessorEditor()
{
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
