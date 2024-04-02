/*
  ==============================================================================

    CompressorBand.cpp
    Created: 27 Mar 2024 9:18:31pm
    Author:  Joseph Skonie

  ==============================================================================
*/

#include "CompressorBand.h"

void CompressorBand::prepare(const juce::dsp::ProcessSpec& spec)
{
    compressor.prepare(spec);
}

void CompressorBand::updateCompressorSettings()
{
    //Before we process anything, we need to configure the parameters
    compressor.setAttack(attack->get());
    compressor.setRelease(release->get());
    compressor.setThreshold(threshold->get());
    compressor.setRatio(ratio->getCurrentChoiceName().getFloatValue()); //we need to extract the float value of the current choice from the array
}

void CompressorBand::process(juce::AudioBuffer<float>& buffer)
{
    auto preRMS = computeRMSLevel(buffer);
    
    auto block = juce::dsp::AudioBlock<float>(buffer); //create an audio block out of the buffer
    auto context = juce::dsp::ProcessContextReplacing<float>(block); //create our context from the block
    
    //Next, we want to toggle whether or not the compressor processes the audio
    //depending on the state of the bypass parameter.
    //The easiest way to do this is by setting isBypassed,
    //since there is already an if block in the process() function
    //that will bypass processing if set to isBypassed is true.
    
    context.isBypassed = bypassed->get();
    
    compressor.process(context); //process the context with the compressor
    
    auto postRMS = computeRMSLevel(buffer);
    
    auto convertToDb = [](auto input){ return juce::Decibels::gainToDecibels(input); };
    
    rmsInputLevelDb.store(convertToDb(preRMS));
    rmsOutputLevelDb.store(convertToDb(postRMS));
}
