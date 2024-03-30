/*
  ==============================================================================

    PathProducer.h
    Created: 29 Mar 2024 7:29:10pm
    Author:  Joseph Skonie

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "FFTDataGenerator.h"
#include "FFTOrder.h"
#include "AnalyzerPathGenerator.h"

struct PathProducer
{
    PathProducer(SingleChannelSampleFifo<SimpleMBCompAudioProcessor::BlockType>& scsf) :
    leftChannelFifo(&scsf)
    {
        leftChannelFFTDataGenerator.changeOrder(FFTOrder::order2048);
        monoBuffer.setSize(1, leftChannelFFTDataGenerator.getFFTSize());
    }
    void process(juce::Rectangle<float> fftBounds, double sampleRate);
    juce::Path getPath() { return leftChannelFFTPath; }
private:
    SingleChannelSampleFifo<SimpleMBCompAudioProcessor::BlockType>* leftChannelFifo;
    
    juce::AudioBuffer<float> monoBuffer;
    
    FFTDataGenerator<std::vector<float>> leftChannelFFTDataGenerator;
    
    AnalyzerPathGenerator<juce::Path> pathProducer;
    
    juce::Path leftChannelFFTPath;
};
