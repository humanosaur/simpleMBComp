/*
  ==============================================================================

    CompressorBand.h
    Created: 27 Mar 2024 9:18:31pm
    Author:  Joseph Skonie

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

struct CompressorBand
{
    //We will want some easily accessible versions of our parameters.
    //There's an APVTS member function to do this,
    //but it would be very costly to do this in the process block,
    //so instead let's store them in member variables
    juce::AudioParameterFloat* attack { nullptr };
    juce::AudioParameterFloat* release { nullptr };
    juce::AudioParameterFloat* threshold { nullptr };
    juce::AudioParameterChoice* ratio { nullptr };
    juce::AudioParameterBool* bypassed { nullptr };
    juce::AudioParameterBool* solo { nullptr };
    juce::AudioParameterBool* mute { nullptr };
    
    void prepare(const juce::dsp::ProcessSpec& spec);
    
    void updateCompressorSettings();
    
    void process(juce::AudioBuffer<float>& buffer);
    
private:
    juce::dsp::Compressor<float> compressor;
};
