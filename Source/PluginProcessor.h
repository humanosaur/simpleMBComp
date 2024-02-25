/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
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
    
    struct CompressorBand
    {
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

private:
    //==============================================================================
    //==============================================================================
    CompressorBand compressor;
    
    //We will want some easily accessible versions of our parameters.
    //There's an APVTS member function to do this,
    //but it would be very costly to do this in the process block,
    //so instead let's store them in member variables
    juce::AudioParameterFloat* attack { nullptr };
    juce::AudioParameterFloat* release { nullptr };
    juce::AudioParameterFloat* threshold { nullptr };
    juce::AudioParameterChoice* ratio { nullptr };
    juce::AudioParameterBool* bypassed { nullptr };
    //==============================================================================
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleMBCompAudioProcessor)
};
