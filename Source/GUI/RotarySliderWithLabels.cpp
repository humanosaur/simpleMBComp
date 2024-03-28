/*
  ==============================================================================

    RotarySliderWithLabels.cpp
    Created: 27 Mar 2024 8:29:21pm
    Author:  Joseph Skonie

  ==============================================================================
*/

#include "RotarySliderWithLabels.h"
#include "../Utilities.h"

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
