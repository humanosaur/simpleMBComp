/*
  ==============================================================================

    PathProducer.cpp
    Created: 29 Mar 2024 7:29:10pm
    Author:  Joseph Skonie

  ==============================================================================
*/

#include "PathProducer.h"


void PathProducer::process(juce::Rectangle<float> fftBounds, double sampleRate)
{
    /*
     This is where we bring together the following to draw the spectrum analyzer:
     
     Our SingleChannelSampleFifo (SCSF)
     The FFT Data Generator
     Our Path Producer
     The GUI
    */

    juce::AudioBuffer<float> tempIncomingBuffer;

    // While there are buffers to pull from SCSF, if we can pull a buffer, we send it to the FFT Data Generator
    // We need to be very careful to keep blocks in the same order throughout
    while( leftChannelFifo->getNumCompleteBuffersAvailable() > 0 )
    {
        if( leftChannelFifo->getAudioBuffer(tempIncomingBuffer) )
        {
            auto size = tempIncomingBuffer.getNumSamples();
            
            jassert( size <= monoBuffer.getNumSamples() );
            size = juce::jmin(size, monoBuffer.getNumSamples());
            
            
            // First, shift everything in the monoBuffer forward by however many samples are in the tempIncomingBuffer
            
            auto writePointer = monoBuffer.getWritePointer(0, 0);
            auto readPointer = monoBuffer.getReadPointer(0, size);
            
            std::copy(readPointer, //location of first sample in the source we want to copy
                      readPointer + (monoBuffer.getNumSamples() - size), //location of last sample + 1 that we want to copy
                      writePointer); //destination buffer
            
//            juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0,0),
//                                              monoBuffer.getReadPointer(0, size),
//                                              monoBuffer.getNumSamples() - size);
            
            // Then, copy the samples from the tempIncomingBuffer to the monoBuffer
            juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0,monoBuffer.getNumSamples() - size),
                                              tempIncomingBuffer.getReadPointer(0,0),
                                              size);
            
            // Send monoBuffers to the FFT Data Generator
            leftChannelFFTDataGenerator.produceFFTDataForRendering(monoBuffer, negativeInfinity);
        }
    }
    
    // While there are FFT data buffers to pull, if we can pull a buffer, generatae a path
    
    const auto fftSize = leftChannelFFTDataGenerator.getFFTSize();
    const auto binWidth = sampleRate / double(fftSize);
    
    while( leftChannelFFTDataGenerator.getNumAvailableFFTDataBlocks() > 0 )
    {
        std::vector<float> fftData;
        if( leftChannelFFTDataGenerator.getFFTData(fftData) )
        {
            pathProducer.generatePath(fftData, fftBounds, fftSize, binWidth, negativeInfinity);
        }
    }
    
    // While there are paths that can be pulled, pull as many as we can & display the most recent path
    
    while( pathProducer.getNumPathsAvailable() > 0 )
    {
        pathProducer.getPath(leftChannelFFTPath);
    }
}
