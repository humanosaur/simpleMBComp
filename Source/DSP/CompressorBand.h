/*
  ==============================================================================

    CompressorBand.h
    Created: 27 Mar 2024 9:18:31pm
    Author:  Joseph Skonie

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "../Utilities.h"

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
    
    float getRMSInputLevelDb() const { return rmsInputLevelDb; }
    float getRMSOutputLevelDb() const { return rmsOutputLevelDb; }
    
private:
    juce::dsp::Compressor<float> compressor;
    std::atomic<float> rmsInputLevelDb { NEGATIVE_INFINITY };
    std::atomic<float> rmsOutputLevelDb { NEGATIVE_INFINITY };
    
    template<typename T>
    float computeRMSLevel(const T& buffer)
    {
        int numChannels = static_cast<int>(buffer.getNumChannels());
        int numSamples = static_cast<int>(buffer.getNumSamples());
        auto rms = 0.f;
        for( int chan = 0; chan < numChannels; ++chan )
        {
            rms += buffer.getRMSLevel(chan, 0, numSamples);
        }
        rms /= static_cast<float>(numChannels);
        return rms;
    }
};
