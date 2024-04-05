/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI/LookAndFeel.h"
#include "GUI/Placeholder.h"
#include "GUI/GlobalControls.h"
#include "GUI/CompressorBandControls.h"
#include "GUI/SpectrumAnalyzer.h"
#include "GUI/ControlBar.h"


class SimpleMBCompAudioProcessorEditor  : public juce::AudioProcessorEditor, juce::Timer
{
public:
    SimpleMBCompAudioProcessorEditor (SimpleMBCompAudioProcessor&);
    ~SimpleMBCompAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SimpleMBCompAudioProcessor& audioProcessor;
    
    //==============================================================================
    //==============================================================================
    
    LookAndFeel lnf;
    
    //Placeholder controlBar, analyzer, globalControls, bandControls;
    ControlBar controlBar;
    SpectrumAnalyzer analyzer { audioProcessor };
    GlobalControls globalControls {audioProcessor.apvts};
    CompressorBandControls bandControls {audioProcessor.apvts};
    
    void toggleGlobalBypassState();
    
    std::array<juce::AudioParameterBool*, 3> getBypassParams();
    
    void updateGlobalBypassButton();
    
    //==============================================================================
    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleMBCompAudioProcessorEditor)
};
