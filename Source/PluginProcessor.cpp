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
    //The parameters are stored in our APVTS as RangedAudioParameters.
    
    //RangedAudioParameter is the base class that all other parameters are derived from.
    
    //So we must cast them to the proper types
    //in order to assign them to the member variables we created to store them.
    
    //Asserting here will help us catch any misspelled names, incorrect variable types,
    //or other things that would cause this to return a nullptr.
    
    compressor.attack = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Attack"));
    jassert(compressor.attack != nullptr);
    
    compressor.release = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Release"));
    jassert(compressor.release != nullptr);
    
    compressor.threshold = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Threshold"));
    jassert(compressor.threshold != nullptr);
    
    compressor.ratio = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("Ratio"));
    jassert(compressor.ratio != nullptr);
    
    compressor.bypassed = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter("Bypassed"));
    jassert(compressor.bypassed != nullptr);
    
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
    
    //The spec needs to know how many samples it'll prcess at a time
    spec.maximumBlockSize = samplesPerBlock;
    
    //It also needs to know the number of channels.
    //This compressor can handle multiple channels,
    //so we'll use the number of channels the compressor is configured with.
    spec.numChannels = getTotalNumOutputChannels();
    
    //It also needs to know the sample rate
    spec.sampleRate = sampleRate;
    
    //Finally we pass this spec to the compressor to be prepared
    compressor.prepare(spec);
    
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
    
    compressor.updateCompressorSettings();
    compressor.process(buffer);

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
    
    //Parameter 1 - Threshold
    //Range: -60dB to +12dB
    //Step size: 1dB (i.e. we can adjust the threshold in 1dB increments)
    //Skew: 1 (distributes the range evenly across the slider)
    
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{"Threshold", 1},
                                                     "Threshold",
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
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{"Attack",1},
                                                     "Attack",
                                                     attackReleaseRange,
                                                     50));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{"Release",1},
                                                     "Release",
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
    
    layout.add(std::make_unique<AudioParameterChoice>(ParameterID{"Ratio", 1},
                                                      "Ratio",
                                                      sa,
                                                      3));
    
    layout.add(std::make_unique<AudioParameterBool>(ParameterID{"Bypassed", 1},
                                                    "Bypassed",
                                                    false));
    
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
