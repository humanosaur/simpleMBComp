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
    
    floatHelper(compressor.attack, Names::Attack_Low_Band);
    floatHelper(compressor.release, Names::Release_Low_Band);
    floatHelper(compressor.threshold, Names::Threshold_Low_Band);
    choiceHelper(compressor.ratio, Names::Ratio_Low_Band);
    boolHelper(compressor.bypassed, Names::Bypassed_Low_Band);
    
    floatHelper(lowMidCrossover, Names::Low_Mid_Crossover_Freq);
    floatHelper(midHighCrossover, Names::Mid_High_Crossover_Freq);
    
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
    
    //To prepare the compressor we must pass a process spec to it,
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
    compressor.prepare(spec);
    
    LP1.prepare(spec);
    HP1.prepare(spec);
    
    AP2.prepare(spec);
    
    LP2.prepare(spec);
    HP2.prepare(spec);
    
    //We need to prepare the separate buffers that we are using the separate the audio into bands
    for( auto& buffer : filterBuffers )
    {
        buffer.setSize(spec.numChannels, samplesPerBlock);
    }
    
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
    
//    compressor.updateCompressorSettings();
//    compressor.process(buffer);
    
    for( auto& fb : filterBuffers )
    {
        fb = buffer;
    }
    
    auto lowMidCutoffFreq = lowMidCrossover->get();
    LP1.setCutoffFrequency(lowMidCutoffFreq);
    HP1.setCutoffFrequency(lowMidCutoffFreq);
    
    auto midHighCutoffFreq = midHighCrossover->get();
    AP2.setCutoffFrequency(midHighCutoffFreq);
    LP2.setCutoffFrequency(midHighCutoffFreq);
    HP2.setCutoffFrequency(midHighCutoffFreq);
    
    //Blocks and contexts for the filters
    
    auto fb0Block = juce::dsp::AudioBlock<float>(filterBuffers[0]);
    auto fb1Block = juce::dsp::AudioBlock<float>(filterBuffers[1]);
    auto fb2Block = juce::dsp::AudioBlock<float>(filterBuffers[2]);
    
    auto fb0Ctx = juce::dsp::ProcessContextReplacing<float>(fb0Block);
    auto fb1Ctx = juce::dsp::ProcessContextReplacing<float>(fb1Block);
    auto fb2Ctx = juce::dsp::ProcessContextReplacing<float>(fb2Block);
    
    //Processing - pay special attention here
    //This is how the videos showed it,
    //though curiously in the write-up the copying step is skipped
    
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
    
    
    //Sum the separated buffers back into one
    
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();
    
    if( compressor.bypassed->get() )
        return;
    
    buffer.clear();
    
    auto addFilterBand = [nc = numChannels, ns = numSamples](auto& inputBuffer, const auto& source)
    {
        for( auto i = 0; i < nc; ++i)
        {
            inputBuffer.addFrom(i, 0, source, i, 0, ns);
        }
    };
    
    //Rather than looping, we'll define these explicitly
    //so that we can mute, bypass, and solo them indivually later
    addFilterBand(buffer, filterBuffers[0]);
    addFilterBand(buffer, filterBuffers[1]);
    addFilterBand(buffer, filterBuffers[2]);
    
//    if( compressor.bypassed->get() )
//    {
//        for( auto ch = 0; ch < numChannels; ++ch)
//        {
//            juce::FloatVectorOperations::multiply(apBuffer.getWritePointer(ch),
//                                                  -1.f,
//                                                  numSamples);
//        }
//        addFilterBand(buffer, apBuffer);
//    }
    
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
    //return new SimpleMBCompAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
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

//==============================================================================
//==============================================================================

juce::AudioProcessorValueTreeState::ParameterLayout SimpleMBCompAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout layout;
    
    using namespace juce;
    using namespace Params;
    const auto& params = GetParams();
    
    //Parameter 1 - Threshold
    //Range: -60dB to +12dB
    //Step size: 1dB (i.e. we can adjust the threshold in 1dB increments)
    //Skew: 1 (distributes the range evenly across the slider)
    
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{params.at(Names::Threshold_Low_Band), 1},
                                                     params.at(Names::Threshold_Low_Band),
                                                     NormalisableRange<float>(-60, //minimum
                                                                              12, // maximum
                                                                              1, //step-size
                                                                              1), //skew
                                                     0));
    
    //Parameters 2 and 3 - Attack and Release
    //Minimum attack time: 5ms
    //Maximum attack time: 500ms
    //Step-size and skew: 1
    
    auto attackReleaseRange = NormalisableRange<float>(5, 500, 1, 1);
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{params.at(Names::Attack_Low_Band),1},
                                                     params.at(Names::Attack_Low_Band),
                                                     attackReleaseRange,
                                                     50));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{params.at(Names::Release_Low_Band),1},
                                                     params.at(Names::Release_Low_Band),
                                                     attackReleaseRange,
                                                     250));
    
    //Parameter 4 - Ratio
    //AudioParameterChoice requires a string array of choices
    //Define the choices
    auto choices = std::vector<double>{1, 1.5, 2, 3, 4, 5, 6, 7, 8, 10, 15, 20, 50, 100};
    
    //Declare the string array
    juce::StringArray sa;
    
    //Concvert choices into string objects
    for ( auto choice : choices)
    {
        sa.add( juce::String(choice,1) );
    }
    
    layout.add(std::make_unique<AudioParameterChoice>(ParameterID{params.at(Names::Ratio_Low_Band), 1},
                                                      params.at(Names::Ratio_Low_Band),
                                                      sa,
                                                      3));
    
    layout.add(std::make_unique<AudioParameterBool>(ParameterID{params.at(Names::Bypassed_Low_Band), 1},
                                                    params.at(Names::Bypassed_Low_Band),
                                                    false));
    
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{params.at(Names::Low_Mid_Crossover_Freq), 1},
                                                     params.at(Names::Low_Mid_Crossover_Freq),
                                                     NormalisableRange<float>(20, 999, 1, 1),
                                                     400));
    
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{params.at(Names::Mid_High_Crossover_Freq), 1},
                                                     params.at(Names::Mid_High_Crossover_Freq),
                                                     NormalisableRange<float>(1000, 20000, 1, 1),
                                                     2000));
    
    return layout;
};

//==============================================================================
//==============================================================================



//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleMBCompAudioProcessor();
}
