/*
  ==============================================================================

    CompressorBandControls.h
    Created: 27 Mar 2024 8:30:07pm
    Author:  Joseph Skonie

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "RotarySliderWithLabels.h"

struct CompressorBandControls : juce::Component, juce::Button::Listener
{
    CompressorBandControls(juce::AudioProcessorValueTreeState& apvts);
    
    void resized() override;
    
    void paint(juce::Graphics& g) override;
    
    void buttonClicked(juce::Button* button) override;
    
    ~CompressorBandControls() override;
    
    juce::AudioProcessorValueTreeState& apvts;
    
private:
    RotarySliderWithLabels attackSlider, releaseSlider, thresholdSlider /*, ratioSlider*/;
    RatioSlider ratioSlider;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackSliderAttachment, releaseSliderAttachment, thresholdSliderAttachment, ratioSliderAttachment;
    
    juce::ToggleButton bypassButton, soloButton, muteButton, lowBandButton, midBandButton, highBandButton;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassButtonAttachment, soloButtonAttachment, muteButtonAttachment;
    
    juce::Component::SafePointer<CompressorBandControls> safePtr { this };
    
    juce::ToggleButton* activeBand = &lowBandButton;
    
    void updateActiveBandFillColors(juce::Button& clickedButton);
    
    void resetActiveBandFillColors();
    
    void refreshBandButtonColors(juce::Button& band, juce::Button& colorSource);
    
    void updateBandSelectButtonStates();
    
    void updateSliderEnablements();
    
    void updateSoloMuteBypassToggleStates(juce::Button& clickedButton);
    
    void updateAttachments();
    
};
