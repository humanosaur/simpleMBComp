/*
  ==============================================================================

    Utilities.cpp
    Created: 27 Mar 2024 8:53:05pm
    Author:  Joseph Skonie

  ==============================================================================
*/

#include "Utilities.h"
#include "GUI/LookAndFeel.h"


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

juce::Rectangle<int> drawModuleBackground(juce::Graphics& g,
                           juce::Rectangle<int> bounds)
{
    using namespace juce;
    
    g.setColour(ColorScheme::getModuleBorderColor());
    g.fillAll();
    
    auto localBounds = bounds;
    
    bounds.reduce(3, 3);
    g.setColour(Colours::black);
    g.fillRoundedRectangle(bounds.toFloat(), 3);
    
    g.drawRect(localBounds);
    
    return bounds;
}
