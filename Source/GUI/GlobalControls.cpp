/*
  ==============================================================================

    GlobalControls.cpp
    Created: 27 Mar 2024 8:29:48pm
    Author:  Joseph Skonie

  ==============================================================================
*/

#include "GlobalControls.h"
#include "../DSP/Params.h"
#include "../Utilities.h"

GlobalControls::GlobalControls(juce::AudioProcessorValueTreeState& apvts)
{
    using namespace Params;
    using RSWL = RotarySliderWithLabels;
    const auto& params = GetParams();
    
    // Define our helper functions
    
    auto getParameterHelper = [&apvts, &params](const auto& pos) -> auto&
    {
        return getParam(pos, apvts, params);
    };
    
    auto& gainInParam = getParameterHelper(Names::Gain_In);
    auto& lowMidParam = getParameterHelper(Names::Low_Mid_Crossover_Freq);
    auto& midHighParam = getParameterHelper(Names::Mid_High_Crossover_Freq);
    auto& gainOutParam = getParameterHelper(Names::Gain_Out);
    
    auto makeAttachmentHelper = [&params, &apvts](auto& attachment, const auto&name, auto& slider)
    {
        makeAttachment(attachment, name, slider, params, apvts);
    };
    
    //Use our helper functions
    
    //Initialize RSWLs
    
    inGainSlider = std::make_unique<RSWL>( &gainInParam,
                                          "dB",
                                          "INPUT TRIM");
    
    lowMidXoverSlider = std::make_unique<RSWL>( &lowMidParam,
                                               "Hz",
                                               "LOW-MID X-OVER");
    
    midHighXoverSlider = std::make_unique<RSWL>( &midHighParam,
                                                "Hz",
                                                "MID-HIGH X-OVER");
    
    outGainSlider = std::make_unique<RSWL>( &gainOutParam,
                                            "dB",
                                           "OUTPUT TRIM");
    
    //Attach RSWLs to parameters
    
    makeAttachmentHelper(inGainSliderAttachment,
                         Names::Gain_In,
                         *inGainSlider);
    
    makeAttachmentHelper(lowMidXoverSliderAttachment,
                         Names::Low_Mid_Crossover_Freq,
                         *lowMidXoverSlider);
    
    makeAttachmentHelper(midHighXoverSliderAttachment,
                         Names::Mid_High_Crossover_Freq,
                         *midHighXoverSlider);
    
    makeAttachmentHelper(outGainSliderAttachment,
                         Names::Gain_Out,
                         *outGainSlider);
    
    //Add max/min label pairs
    
    addLabelPairs(inGainSlider->labels,
                  gainInParam,
                  "dB");
    
    addLabelPairs(lowMidXoverSlider->labels,
                  lowMidParam,
                  "Hz");
    
    addLabelPairs(midHighXoverSlider->labels,
                  midHighParam,
                  "Hz");
    
    addLabelPairs(outGainSlider->labels,
                  gainOutParam,
                  "dB");
    
    
    //Add and make visible
    
    addAndMakeVisible(*inGainSlider);
    addAndMakeVisible(*lowMidXoverSlider);
    addAndMakeVisible(*midHighXoverSlider);
    addAndMakeVisible(*outGainSlider);
}

void GlobalControls::paint(juce::Graphics& g)
{
    //using namespace juce;
    auto bounds = getLocalBounds();
    
    drawModuleBackground(g, bounds);
}

void GlobalControls::resized()
{
    auto bounds = getLocalBounds().reduced(5);
    
    using namespace juce;
    
    FlexBox flexBox;
    flexBox.flexDirection = FlexBox::Direction::row;
    flexBox.flexWrap = FlexBox::Wrap::noWrap;
    
    auto spacer = FlexItem().withWidth(4);
    auto endCap = FlexItem().withWidth(6);
    
    flexBox.items.add(endCap);
    flexBox.items.add(FlexItem(*inGainSlider).withFlex(1));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(*lowMidXoverSlider).withFlex(1));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(*midHighXoverSlider).withFlex(1));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(*outGainSlider).withFlex(1));
    flexBox.items.add(spacer);
    flexBox.items.add(endCap);
    
    flexBox.performLayout(bounds);
}
