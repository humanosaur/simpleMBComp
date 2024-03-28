/*
  ==============================================================================

    Placeholder.h
    Created: 27 Mar 2024 8:29:35pm
    Author:  Joseph Skonie

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

struct Placeholder : juce::Component
{
    
    Placeholder();
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(customColor);
    }
    
    juce::Colour customColor;
};
