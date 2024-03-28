/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DSP/Params.h"
#include "DSP/CompressorBand.h"

class SimpleMBCompAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SimpleMBCompAudioProcessor();
    ~SimpleMBCompAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //==============================================================================
    //==============================================================================
    
    //Here we declare the APVTS
    //The APVTS synchronizes our parameters with the host and with the GUI
    //We must provide all the parameters when the APVTS is constructed,
    //so we use a ParameterLayout to define that
    using APVTS = juce::AudioProcessorValueTreeState;
    static APVTS::ParameterLayout createParameterLayout();
    APVTS apvts {*this, nullptr, "Parameters", createParameterLayout()};
    
    
    //==============================================================================
    //==============================================================================

private:
    //==============================================================================
    //==============================================================================
   
    std::array<CompressorBand, 3> compressors;
    CompressorBand& lowBandComp = compressors[0];
    CompressorBand& midBandComp = compressors[1];
    CompressorBand& highBandComp = compressors[2];
    
    using Filter = juce::dsp::LinkwitzRileyFilter<float>;

    //      Fc0     Fc1
    Filter  LP1,    AP2,
            HP1,    LP2,
                    HP2;
        
    //Cached audio parameters for the crossover frequency
    juce::AudioParameterFloat* lowMidCrossover { nullptr };
    juce::AudioParameterFloat* midHighCrossover { nullptr };
    
    //Gain processors and cached gain parameters
    juce::dsp::Gain<float> inputGain, outputGain;
    juce::AudioParameterFloat* inputGainParam { nullptr };
    juce::AudioParameterFloat* outputGainParam { nullptr };
    
    template<typename T, typename U>
    void applyGain(T& buffer, U& gain)
    {
        auto block = juce::dsp::AudioBlock<float>(buffer);
        auto ctx = juce::dsp::ProcessContextReplacing<float>(block);
        gain.process(ctx);
    }
    
    //Buffers to copy the signal to so that we can process bands separately
    std::array<juce::AudioBuffer<float>, 3> filterBuffers;
    
    void updateState();
    
    void splitBands(const juce::AudioBuffer<float>& inputBuffer);
    
    //==============================================================================
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleMBCompAudioProcessor)
};
