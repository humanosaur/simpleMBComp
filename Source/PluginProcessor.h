/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
//==============================================================================

namespace Params
{
    //This enum contains all of the paramaters we have in the project,
    //and we will add more as we add DSP and GUI functionality.
    enum Names
    {
        Low_Mid_Crossover_Freq,
        Mid_High_Crossover_Freq,
        
        Threshold_Low_Band,
        Threshold_Mid_Band,
        Threshold_High_Band,
        
        Attack_Low_Band,
        Attack_Mid_Band,
        Attack_High_Band,
        
        Release_Low_Band,
        Release_Mid_Band,
        Release_High_Band,
        
        Ratio_Low_Band,
        Ratio_Mid_Band,
        Ratio_High_Band,
        
        Bypassed_Low_Band,
        Bypassed_Mid_Band,
        Bypassed_High_Band,
    }; //end enum Names
    
    //Providing a map will allow us to look things up
    //and not worry about misspelling and other such errors.
    inline const std::map<Names, juce::String>& GetParams()
    {
        static std::map<Names, juce::String> params =
        {
            {Low_Mid_Crossover_Freq, "Low-Mid Crossover Freq"},
            {Mid_High_Crossover_Freq,"Mid-High Crossover Freq"},
                        
            {Threshold_Low_Band, "Threshold Low Band"},
            {Threshold_Mid_Band, "Threshold Mid Band"},
            {Threshold_High_Band, "Threshold High Band"},
            
            {Attack_Low_Band, "Attack Low Band"},
            {Attack_Mid_Band, "Attack Mid Band"},
            {Attack_High_Band, "Attack High Band"},
            
            {Release_Low_Band, "Release Low Band"},
            {Release_Mid_Band, "Release Mid Band"},
            {Release_High_Band, "Release High Band"},
            
            {Ratio_Low_Band, "Ratio Low Band"},
            {Ratio_Mid_Band, "Ratio Mid Band"},
            {Ratio_High_Band, "Ratio High Band"},
            
            {Bypassed_Low_Band, "Bypassed Low Band"},
            {Bypassed_Mid_Band, "Bypassed Mid Band"},
            {Bypassed_High_Band, "Bypassed High Band"}
        };
        
        return params;
    }
}

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
    
    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        compressor.prepare(spec);
    }
    
    void updateCompressorSettings()
    {
        //Before we process anything, we need to configure the parameters
        compressor.setAttack(attack->get());
        compressor.setRelease(release->get());
        compressor.setThreshold(threshold->get());
        compressor.setRatio(ratio->getCurrentChoiceName().getFloatValue()); //we need to extract the float value of the current choice from the array
    }
    
    void process(juce::AudioBuffer<float>& buffer)
    {
        auto block = juce::dsp::AudioBlock<float>(buffer); //create an audio block out of the buffer
        auto context = juce::dsp::ProcessContextReplacing<float>(block); //create our context from the block
        
        //Next, we want to toggle whether or not the compressor processes the audio
        //depending on the state of the bypass parameter.
        //The easiest way to do this is by setting isBypassed,
        //since there is already an if block in the process() function
        //that will bypass processing if set to isBypassed is true.
        
        context.isBypassed = bypassed->get();
        
        compressor.process(context); //process the context with the compressor
    }
    
private:
    juce::dsp::Compressor<float> compressor;
};
//==============================================================================
//==============================================================================



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
    
//    Filter invAP1, invAP2;
//    juce::AudioBuffer<float> invAPBuffer;
        
    //Cached audio parameter for the crossover frequency
    juce::AudioParameterFloat* lowMidCrossover { nullptr };
    juce::AudioParameterFloat* midHighCrossover { nullptr };
    
    //Buffers to copy the signal to so that we can process bands separately
    std::array<juce::AudioBuffer<float>, 3> filterBuffers;
    
    //==============================================================================
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleMBCompAudioProcessor)
};
