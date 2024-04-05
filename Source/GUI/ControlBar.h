/*
  ==============================================================================

    ControlBar.h
    Created: 5 Apr 2024 9:09:22am
    Author:  Joseph Skonie

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "CustomButtons.h"

struct ControlBar : juce::Component
{
    ControlBar();
    void resized() override;
    
    AnalyzerButton analyzerButton;
    
    PowerButton globalBypassButton;
};
