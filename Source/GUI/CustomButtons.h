/*
  ==============================================================================

    CustomButtons.h
    Created: 27 Mar 2024 8:29:53pm
    Author:  Joseph Skonie

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

struct PowerButton : juce::ToggleButton {};

struct AnalyzerButton : juce::ToggleButton
{
    void resized() override;
    juce::Path randomPath;
};
