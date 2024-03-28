/*
  ==============================================================================

    GlobalControls.h
    Created: 27 Mar 2024 8:29:48pm
    Author:  Joseph Skonie

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "RotarySliderWithLabels.h"

struct GlobalControls : juce::Component
{
    GlobalControls(juce::AudioProcessorValueTreeState& apvts);
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
private:
    //RotarySlider inGainSlider, lowMidXoverSlider, midHighXoverSlider, outGainSlider;
    std::unique_ptr<RotarySliderWithLabels> inGainSlider, lowMidXoverSlider, midHighXoverSlider, outGainSlider;
    
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowMidXoverSliderAttachment, midHighXoverSliderAttachment, inGainSliderAttachment, outGainSliderAttachment;
};
