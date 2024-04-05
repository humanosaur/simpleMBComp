/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleMBCompAudioProcessor::SimpleMBCompAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    using namespace Params;
    const auto& params = GetParams();
    
    //The parameters are stored in our APVTS as RangedAudioParameters.
    
    //RangedAudioParameter is the base class that all other parameters are derived from.
    
    //So we must cast them to the proper types in order to
    //assign them to the member variables we created to store them.
    
    //Asserting here will help us catch any misspelled names, incorrect variable types,
    //or other things that would cause this to return a nullptr.
    
    //We'll have one helper function for each parameter type to handle the casting & assertion.
    
    auto floatHelper = [&apvts = this->apvts, &params](auto& param, const auto& paramName)
    {
        param = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);
    };
    
    
    auto choiceHelper = [&apvts = this->apvts, &params](auto& param, const auto& paramName)
    {
        param = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);
    };
    
    auto boolHelper = [&apvts = this->apvts, &params](auto& param, const auto& paramName)
    {
        param = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);
    };
    
    //Then we'll use the helper function for each parameter we need to cast.
    
    //Compressor 1
    
    floatHelper(lowBandComp.attack, Names::Attack_Low_Band);
    floatHelper(lowBandComp.release, Names::Release_Low_Band);
    floatHelper(lowBandComp.threshold, Names::Threshold_Low_Band);
    choiceHelper(lowBandComp.ratio, Names::Ratio_Low_Band);
    boolHelper(lowBandComp.bypassed, Names::Bypassed_Low_Band);
    boolHelper(lowBandComp.solo, Names::Solo_Low_Band);
    boolHelper(lowBandComp.mute, Names::Mute_Low_Band);
    
    //Compressor 2
    
    floatHelper(midBandComp.attack, Names::Attack_Mid_Band);
    floatHelper(midBandComp.release, Names::Release_Mid_Band);
    floatHelper(midBandComp.threshold, Names::Threshold_Mid_Band);
    choiceHelper(midBandComp.ratio, Names::Ratio_Mid_Band);
    boolHelper(midBandComp.bypassed, Names::Bypassed_Mid_Band);
    boolHelper(midBandComp.solo, Names::Solo_Mid_Band);
    boolHelper(midBandComp.mute, Names::Mute_Mid_Band);
    
    //Compressor 3
    
    floatHelper(highBandComp.attack, Names::Attack_High_Band);
    floatHelper(highBandComp.release, Names::Release_High_Band);
    floatHelper(highBandComp.threshold, Names::Threshold_High_Band);
    choiceHelper(highBandComp.ratio, Names::Ratio_High_Band);
    boolHelper(highBandComp.bypassed, Names::Bypassed_High_Band);
    boolHelper(highBandComp.solo, Names::Solo_High_Band);
    boolHelper(highBandComp.mute, Names::Mute_High_Band);
    
    //Crossover Frequencies
    
    floatHelper(lowMidCrossover, Names::Low_Mid_Crossover_Freq);
    floatHelper(midHighCrossover, Names::Mid_High_Crossover_Freq);
    
    //Input and output gain
    
    floatHelper(inputGainParam, Names::Gain_In);
    floatHelper(outputGainParam, Names::Gain_Out);
    
    //Filters
    
    LP1.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    HP1.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    
    AP2.setType(juce::dsp::LinkwitzRileyFilterType::allpass);
    
    LP2.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    HP2.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    
}

SimpleMBCompAudioProcessor::~SimpleMBCompAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleMBCompAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleMBCompAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleMBCompAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleMBCompAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleMBCompAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleMBCompAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleMBCompAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleMBCompAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleMBCompAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleMBCompAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleMBCompAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    //==============================================================================
    //==============================================================================
    
    //To prepare each compressor we must pass a process spec to it,
    //which we must declare and set up
    juce::dsp::ProcessSpec spec;
    
    //The spec needs to know how many samples it'll process at a time
    spec.maximumBlockSize = samplesPerBlock;
    
    //It also needs to know the number of channels.
    //This compressor can handle multiple channels,
    //so we'll use the number of channels the compressor is configured with.
    spec.numChannels = getTotalNumOutputChannels();
    
    //It also needs to know the sample rate
    spec.sampleRate = sampleRate;
    
    //Finally we pass this spec to the compressor to be prepared
    for( auto& compressor : compressors )
        compressor.prepare(spec);
    
    //Prep the filters

    LP1.prepare(spec);
    HP1.prepare(spec);
    
    AP2.prepare(spec);
    
    LP2.prepare(spec);
    HP2.prepare(spec);
    
    //Prep the gain params
    
    inputGain.prepare(spec);
    outputGain.prepare(spec);
    
    inputGain.setRampDurationSeconds(.05);
    outputGain.setRampDurationSeconds(.05);
    
    //We need to prepare the separate buffers that we are using the separate the audio into bands
    for( auto& buffer : filterBuffers )
    {
        buffer.setSize(spec.numChannels, samplesPerBlock);
    }
    
    
    //Prep the FIFOS
    
    leftChannelFifo.prepare(samplesPerBlock);
    rightChannelFifo.prepare(samplesPerBlock);
    
    osc.initialise([](float x){ return std::sin(x); });
    osc.prepare(spec);
    osc.setFrequency(getSampleRate() / ((2 << FFTOrder::order2048) -1) * 50);
    
    gain.prepare(spec);
    gain.setGainDecibels(-12.f);
    
    //==============================================================================
    //==============================================================================
}

void SimpleMBCompAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleMBCompAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

//==============================================================================
//==============================================================================
void SimpleMBCompAudioProcessor::updateState()
{
    for( auto& compressor : compressors )
        compressor.updateCompressorSettings();
    
    auto lowMidCutoffFreq = lowMidCrossover->get();
    LP1.setCutoffFrequency(lowMidCutoffFreq);
    HP1.setCutoffFrequency(lowMidCutoffFreq);
    
    auto midHighCutoffFreq = midHighCrossover->get();
    AP2.setCutoffFrequency(midHighCutoffFreq);
    LP2.setCutoffFrequency(midHighCutoffFreq);
    HP2.setCutoffFrequency(midHighCutoffFreq);
    
    inputGain.setGainDecibels(inputGainParam->get());
    outputGain.setGainDecibels(outputGainParam->get());
}

void SimpleMBCompAudioProcessor::splitBands(const juce::AudioBuffer<float> &inputBuffer)
{
    //Copy the buffer to each of the filterBuffers we created for our bands
    for( auto& fb : filterBuffers )
    {
        fb = inputBuffer;
    }
    
    //Create blocks and contexts for the filters
    
    auto fb0Block = juce::dsp::AudioBlock<float>(filterBuffers[0]);
    auto fb1Block = juce::dsp::AudioBlock<float>(filterBuffers[1]);
    auto fb2Block = juce::dsp::AudioBlock<float>(filterBuffers[2]);
    
    auto fb0Ctx = juce::dsp::ProcessContextReplacing<float>(fb0Block);
    auto fb1Ctx = juce::dsp::ProcessContextReplacing<float>(fb1Block);
    auto fb2Ctx = juce::dsp::ProcessContextReplacing<float>(fb2Block);
    
    
    //Processing the filters - pay special attention here
    
    //The first band is made up of LP1 and AP2, so we simply processes context0 through both
    LP1.process(fb0Ctx);
    AP2.process(fb0Ctx);
    
    //The second band is made up of HP1 and LP2, but we must be careful here
    HP1.process(fb1Ctx);
    //After processing context1 through HP1, we need to copy it like this
    filterBuffers[2] = filterBuffers[1];
    //Then continue processing context1 through LP2
    LP2.process(fb1Ctx);
    
    //And using our copied buffer we process context2 through the final HP
    HP2.process(fb2Ctx);
}
//==============================================================================
//==============================================================================

void SimpleMBCompAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    //==============================================================================
    //==============================================================================
    
    updateState();
    
    if ( /* DISABLES CODE */ (false) )
    {
        buffer.clear();
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        osc.process(context);
        
        gain.setGainDecibels(JUCE_LIVE_CONSTANT(-12));
        gain.process(context);
    }
    
    leftChannelFifo.update(buffer);
    rightChannelFifo.update(buffer);
    
    applyGain(buffer, inputGain);
    
    splitBands(buffer);
    
    //Process the split bands through their respective compressors
    
    for( size_t i = 0; i < filterBuffers.size(); ++i )
    {
        compressors[i].process(filterBuffers[i]);
    }
    

    buffer.clear();
    
    //Sum the separated buffers back into one
    
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();
    
    auto addFilterBand = [nc = numChannels, ns = numSamples](auto& inputBuffer, const auto& source)
    {
        for( auto i = 0; i < nc; ++i)
        {
            inputBuffer.addFrom(i, 0, source, i, 0, ns);
        }
    };
    
    //Check for soloed bands
    
    auto bandsAreSoloed = false;
    for( auto& compressor : compressors )
    {
        if( compressor.solo->get())
        {
            bandsAreSoloed = true;
            break;
        }
    }
    
    //If bands are soloed, add only solo band(s) to the buffer
    
    if( bandsAreSoloed )
    {
        for( size_t i = 0; i < compressors.size(); ++i )
        {
            auto& compressor = compressors[i];
            if( compressor.solo->get() )
            {
                addFilterBand(buffer, filterBuffers[i]);
            }
        }
    }
    else
    {
        for( size_t i = 0; i < compressors.size(); ++i )
        {
            auto& compressor = compressors[i];
            if( ! compressor.mute->get() )
            {
                addFilterBand(buffer, filterBuffers[i]);
            }
        }
    }
    
    applyGain(buffer, outputGain);
    
    //==============================================================================
    //==============================================================================
}

//==============================================================================
bool SimpleMBCompAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleMBCompAudioProcessor::createEditor()
{
    return new SimpleMBCompAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void SimpleMBCompAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
//==============================================================================
//==============================================================================
    //Saving the state to the memory block provided by the host
    //Create a memory stream
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
//==============================================================================
//==============================================================================
}

void SimpleMBCompAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
//==============================================================================
//==============================================================================
    //First we must check if the tree pulled from memory is valid
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    
    //If so, we can copy it to the plugic state
    if ( tree.isValid() )
    {
        apvts.replaceState(tree);
    }
//==============================================================================
//==============================================================================
}


juce::AudioProcessorValueTreeState::ParameterLayout SimpleMBCompAudioProcessor::createParameterLayout()
{
    //==============================================================================
    //==============================================================================

    APVTS::ParameterLayout layout;
    
    using namespace juce;
    using namespace Params;
    const auto& params = GetParams();
    
    
    //==============================================================================
    //==============================================================================
    
    //Compressor Parameters
    
    //==============================================================================
    //==============================================================================
    
    //Parameter 1 - Threshold
    
    auto thresholdRange = NormalisableRange<float>(MIN_THRESHOLD, //minimum
                                                   MAX_DECIBELS, // maximum
                                                   1, //step-size
                                                   1); //skew
    
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{params.at(Names::Threshold_Low_Band), 1},
                                                     params.at(Names::Threshold_Low_Band),
                                                     thresholdRange,
                                                     0));
    
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{params.at(Names::Threshold_Mid_Band), 1},
                                                     params.at(Names::Threshold_Mid_Band),
                                                     thresholdRange,
                                                     0));

    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{params.at(Names::Threshold_High_Band), 1},
                                                     params.at(Names::Threshold_High_Band),
                                                     thresholdRange,
                                                     0));
    
    //Attack and Release will have the same range so let's define it first here
    
    auto attackReleaseRange = NormalisableRange<float>(5, //min
                                                       500, //max
                                                       1, //step-size
                                                       1); //skew
    
    //Parameter 2 - Attack
    
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{params.at(Names::Attack_Low_Band),1},
                                                     params.at(Names::Attack_Low_Band),
                                                     attackReleaseRange,
                                                     50));

    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{params.at(Names::Attack_Mid_Band),1},
                                                     params.at(Names::Attack_Mid_Band),
                                                     attackReleaseRange,
                                                     50));
    
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{params.at(Names::Attack_High_Band),1},
                                                     params.at(Names::Attack_High_Band),
                                                     attackReleaseRange,
                                                     50));
    
    //Parameter 3 - Release
    
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{params.at(Names::Release_Low_Band),1},
                                                     params.at(Names::Release_Low_Band),
                                                     attackReleaseRange,
                                                     250));
    
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{params.at(Names::Release_Mid_Band),1},
                                                     params.at(Names::Release_Mid_Band),
                                                     attackReleaseRange,
                                                     250));
    
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{params.at(Names::Release_High_Band),1},
                                                     params.at(Names::Release_High_Band),
                                                     attackReleaseRange,
                                                     250));
    
    //Parameter 4 - Ratio
    
    //AudioParameterChoice requires a string array of choices
    //Define the choices
    auto choices = std::vector<double>{1, 1.5, 2, 3, 4, 5, 6, 7, 8, 10, 15, 20, 50, 100};
    
    //Declare the string array
    juce::StringArray sa;
    
    //Convert choices into string objects
    for ( auto choice : choices)
    {
        sa.add( juce::String(choice,1) );
    }
    
    layout.add(std::make_unique<AudioParameterChoice>(ParameterID{params.at(Names::Ratio_Low_Band), 1},
                                                      params.at(Names::Ratio_Low_Band),
                                                      sa,
                                                      3));
    
    layout.add(std::make_unique<AudioParameterChoice>(ParameterID{params.at(Names::Ratio_Mid_Band), 1},
                                                      params.at(Names::Ratio_Mid_Band),
                                                      sa,
                                                      3));
    
    layout.add(std::make_unique<AudioParameterChoice>(ParameterID{params.at(Names::Ratio_High_Band), 1},
                                                      params.at(Names::Ratio_High_Band),
                                                      sa,
                                                      3));
    
    //Bypass
    
    layout.add(std::make_unique<AudioParameterBool>(ParameterID{params.at(Names::Bypassed_Low_Band), 1},
                                                    params.at(Names::Bypassed_Low_Band),
                                                    false));
    
    layout.add(std::make_unique<AudioParameterBool>(ParameterID{params.at(Names::Bypassed_Mid_Band), 1},
                                                    params.at(Names::Bypassed_Mid_Band),
                                                    false));
    
    layout.add(std::make_unique<AudioParameterBool>(ParameterID{params.at(Names::Bypassed_High_Band), 1},
                                                    params.at(Names::Bypassed_High_Band),
                                                    false));
    
    //Solo
    
    layout.add(std::make_unique<AudioParameterBool>(ParameterID{params.at(Names::Solo_Low_Band), 1},
                                                    params.at(Names::Solo_Low_Band),
                                                    false));
    
    layout.add(std::make_unique<AudioParameterBool>(ParameterID{params.at(Names::Solo_Mid_Band), 1},
                                                    params.at(Names::Solo_Mid_Band),
                                                    false));
    
    layout.add(std::make_unique<AudioParameterBool>(ParameterID{params.at(Names::Solo_High_Band), 1},
                                                    params.at(Names::Solo_High_Band),
                                                    false));
    
    //Mute
    
    layout.add(std::make_unique<AudioParameterBool>(ParameterID{params.at(Names::Mute_Low_Band), 1},
                                                    params.at(Names::Mute_Low_Band),
                                                    false));
    
    layout.add(std::make_unique<AudioParameterBool>(ParameterID{params.at(Names::Mute_Mid_Band), 1},
                                                    params.at(Names::Mute_Mid_Band),
                                                    false));
    
    layout.add(std::make_unique<AudioParameterBool>(ParameterID{params.at(Names::Mute_High_Band), 1},
                                                    params.at(Names::Mute_High_Band),
                                                    false));
    
    
    //==============================================================================
    //==============================================================================
  
    //Filter Crossover Pameters
    
    //==============================================================================
    //==============================================================================
  
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{params.at(Names::Low_Mid_Crossover_Freq), 1},
                                                     params.at(Names::Low_Mid_Crossover_Freq),
                                                     NormalisableRange<float>(MIN_FREQUENCY, 999, 1, 1),
                                                     400));
    
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{params.at(Names::Mid_High_Crossover_Freq), 1},
                                                     params.at(Names::Mid_High_Crossover_Freq),
                                                     NormalisableRange<float>(1000, MAX_FREQUENCY, 1, 1),
                                                     2000));
    
    //Input and output gain parameters
    
    //We'll define a range from -24 to 24dB with a step size of 0.5dB
    auto gainRange = juce::NormalisableRange<float>(-24, 24, 0.5, 1);
    
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{params.at(Names::Gain_In),1},
                                                     params.at(Names::Gain_In),
                                                     gainRange,
                                                     0));
    
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{params.at(Names::Gain_Out),1},
                                                     params.at(Names::Gain_Out),
                                                     gainRange,
                                                     0));
    
    return layout;
    
    //==============================================================================
    //==============================================================================
};


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleMBCompAudioProcessor();
}
