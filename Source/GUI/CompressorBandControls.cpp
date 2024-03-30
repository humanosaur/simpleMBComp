/*
  ==============================================================================

    CompressorBandControls.cpp
    Created: 27 Mar 2024 8:30:07pm
    Author:  Joseph Skonie

  ==============================================================================
*/

#include "CompressorBandControls.h"
#include "../DSP/Params.h"
#include "../Utilities.h"

CompressorBandControls::CompressorBandControls(juce::AudioProcessorValueTreeState& apvts) :
apvts(apvts),
attackSlider(nullptr, "ms", "ATTACK"),
releaseSlider(nullptr, "ms", "RELEASE"),
thresholdSlider(nullptr, "dB", "THRESHOLD"),
ratioSlider(nullptr, "")
{
    //Set names and colours
    bypassButton.setName("X");
    soloButton.setName("S");
    muteButton.setName("M");
    lowBandButton.setName("Low");
    midBandButton.setName("Mid");
    highBandButton.setName("High");
    
    bypassButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::maroon);
    bypassButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    soloButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::white);
    soloButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    muteButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::grey);
    muteButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    
    lowBandButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::lightblue);
    lowBandButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    midBandButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::lightblue);
    midBandButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    highBandButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::lightblue);
    highBandButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    
    //Assign a radio group number to the band select buttons
    lowBandButton.setRadioGroupId(1);
    midBandButton.setRadioGroupId(1);
    highBandButton.setRadioGroupId(1);
    
    //Add and make visible
    addAndMakeVisible(attackSlider);
    addAndMakeVisible(releaseSlider);
    addAndMakeVisible(thresholdSlider);
    addAndMakeVisible(ratioSlider);
    addAndMakeVisible(bypassButton);
    addAndMakeVisible(soloButton);
    addAndMakeVisible(muteButton);
    addAndMakeVisible(lowBandButton);
    addAndMakeVisible(midBandButton);
    addAndMakeVisible(highBandButton);
    
    //Listener for our bypass/solo/mute buttons to do all the things we need them to do
    bypassButton.addListener(this);
    soloButton.addListener(this);
    muteButton.addListener(this);
    
    
    //Now the part where we set which band the controls are assigned to
    auto buttonSwitcher = [safePtr = this->safePtr]()
    {
      if (auto* c = safePtr.getComponent())
      {
          c->updateAttachments();
      }
    };
    
    
    //Every time a button is clicked, invoke the above
    lowBandButton.onClick = buttonSwitcher;
    midBandButton.onClick = buttonSwitcher;
    highBandButton.onClick = buttonSwitcher;
    
    
    //We will default to selecting the low band
    lowBandButton.setToggleState(true, juce::NotificationType::dontSendNotification);
    
    updateAttachments();
    
    updateSliderEnablements();
    updateBandSelectButtonStates();
    
}

CompressorBandControls::~CompressorBandControls()
{
    bypassButton.removeListener(this);
    soloButton.removeListener(this);
    muteButton.removeListener(this);
}

void CompressorBandControls::paint(juce::Graphics& g)
{
    //using namespace juce;
    auto bounds = getLocalBounds();
    
    drawModuleBackground(g, bounds);
}

void CompressorBandControls::buttonClicked(juce::Button *button)
{
    updateSliderEnablements();
    
    updateSoloMuteBypassToggleStates(*button);
    
    updateActiveBandFillColors(*button);
}

void CompressorBandControls::updateActiveBandFillColors(juce::Button& clickedButton)
{
    jassert(activeBand != nullptr);
    DBG( "Active band: " << activeBand->getName());
    
    if( clickedButton.getToggleState() == false )
    {
        resetActiveBandFillColors();
    }
    else
    {
        refreshBandButtonColors(*activeBand, clickedButton);
    }
}

void CompressorBandControls::resetActiveBandFillColors()
{
    activeBand->setColour((juce::TextButton::ColourIds::buttonOnColourId), juce::Colours::lightblue);
    activeBand->setColour((juce::TextButton::ColourIds::buttonColourId), juce::Colours::black);
    activeBand->repaint();
}

void CompressorBandControls::refreshBandButtonColors(juce::Button& band, juce::Button& colorSource)
{
    band.setColour(juce::TextButton::ColourIds::buttonOnColourId,
                   colorSource.findColour(juce::TextButton::ColourIds::buttonOnColourId));
    
    band.setColour(juce::TextButton::ColourIds::buttonColourId,
                   colorSource.findColour(juce::TextButton::ColourIds::buttonOnColourId));
    
    band.repaint();
}

void CompressorBandControls::updateBandSelectButtonStates()
{
    using namespace Params;
    
    std::vector<std::array<Names, 3>> paramsToCheck
    {
        {Names::Solo_Low_Band, Names::Mute_Low_Band, Names::Bypassed_Low_Band},
        {Names::Solo_Mid_Band, Names::Mute_Mid_Band, Names::Bypassed_Mid_Band},
        {Names::Solo_High_Band, Names::Mute_High_Band, Names::Bypassed_High_Band}
    };
    
    const auto& params = GetParams();
    auto paramHelper = [&params, this](auto name)
    {
        return dynamic_cast<juce::AudioParameterBool*>(&getParam(name, apvts, params));
    };
    
    for(size_t i = 0; i < paramsToCheck.size(); ++i)
    {
        auto& list = paramsToCheck[i];
        
        auto* bandButton = i == 0 ? &lowBandButton :
                            i == 1 ? &midBandButton :
                                    &highBandButton;
        if( auto* solo = paramHelper(list[0]);
           solo->get() )
        {
            refreshBandButtonColors(*bandButton, soloButton);
        }
        else if ( auto* mute = paramHelper(list[1]);
                 mute->get() )
        {
            refreshBandButtonColors(*bandButton, muteButton);
        }
        else if ( auto* bypass = paramHelper(list[2]);
                 bypass->get() )
        {
            refreshBandButtonColors(*bandButton, bypassButton);
        }
    }
}

void CompressorBandControls::updateSliderEnablements()
{
  //If the band is muted or bypassed, disable the sliders
    auto disabled = muteButton.getToggleState() || bypassButton.getToggleState();
    
    attackSlider.setEnabled(!disabled);
    releaseSlider.setEnabled(!disabled);
    ratioSlider.setEnabled(!disabled);
    thresholdSlider.setEnabled(!disabled);
}

void CompressorBandControls::updateSoloMuteBypassToggleStates(juce::Button &clickedButton)
{
    //Here we basically implement a radio button functionality
    //with the caveat that it is okay for all buttons to be off.
    //With regular radio buttons, this would not be possible,
    //but we need it to be. So the logic is simple:
    //whichever button is toggled on, toggle the others off,
    //and if a button is toggled off, do nothing.
    
    //We must send the notification since that's how the ParameterAttachment
    //knows to update the audio parameter
    
    if( &clickedButton == &soloButton && soloButton.getToggleState() )
    {
        bypassButton.setToggleState(false, juce::NotificationType::sendNotification);
        muteButton.setToggleState(false, juce::NotificationType::sendNotification);
    }
    else if( &clickedButton == &muteButton && muteButton.getToggleState() )
    {
        bypassButton.setToggleState(false, juce::NotificationType::sendNotification);
        soloButton.setToggleState(false, juce::NotificationType::sendNotification);
    }
    else if( &clickedButton == &bypassButton && bypassButton.getToggleState() )
    {
        muteButton.setToggleState(false, juce::NotificationType::sendNotification);
        soloButton.setToggleState(false, juce::NotificationType::sendNotification);
    }
}

void CompressorBandControls::updateAttachments()
{
    enum BandType
    {
        Low,
        Mid,
        High
    };
    
    //Since our band select buttons are radio buttons, only one can be true at a time.
    //We'll use this fact to set which band we want to attach our sliders to.
    BandType bandType = [this]()
    {
        if( lowBandButton.getToggleState() )
            return  BandType::Low;
        
        if( midBandButton.getToggleState() )
            return  BandType::Mid;
        
        return  BandType::High;
    }();
    
    using namespace Params;
    
    //We'll declare a vector and update it with the parameter names based on which band is toggled on.
    std::vector<Params::Names> names;
    
    switch (bandType)
    {
        case Low:
        {
            names = std::vector<Params::Names>
            {
                Names::Attack_Low_Band,
                Names::Release_Low_Band,
                Names::Threshold_Low_Band,
                Names::Ratio_Low_Band,
                Names::Bypassed_Low_Band,
                Names::Solo_Low_Band,
                Names::Mute_Low_Band,
            };
            activeBand = &lowBandButton;
            break;
        }
        case Mid:
        {
            names = std::vector<Params::Names>
            {
                Names::Attack_Mid_Band,
                Names::Release_Mid_Band,
                Names::Threshold_Mid_Band,
                Names::Ratio_Mid_Band,
                Names::Bypassed_Mid_Band,
                Names::Solo_Mid_Band,
                Names::Mute_Mid_Band,
            };
            activeBand = &midBandButton;
            break;
        }
        case High:
        {
            names = std::vector<Params::Names>
            {
                Names::Attack_High_Band,
                Names::Release_High_Band,
                Names::Threshold_High_Band,
                Names::Ratio_High_Band,
                Names::Bypassed_High_Band,
                Names::Solo_High_Band,
                Names::Mute_High_Band,
            };
            activeBand = &highBandButton;
            break;
        }
    }
    
    //These positions are in the same order as the names above, allowing us to
    //find which name to send into our parameter helper below.
    enum Pos
    {
        Attack,
        Release,
        Threshold,
        Ratio,
        Bypass,
        Solo,
        Mute,
    };
    
    //Grab our parameters
    const auto& params = GetParams();
    
    //Helper function for getting specific parameters
    auto getParameterHelper = [&apvts = this->apvts, &params, &names](const auto& pos) -> auto&
    {
        return getParam(names.at(pos), apvts, params);
    };
    
    //For some reason we have to reset the attachments before creating
    //new attachments. It seems unecessary since makeAttachment ends up
    //calling the same destructor of the sliderAttachment class, but if this is
    //omitted then the sliders don't display the correct value when they are refreshed.
    attackSliderAttachment.reset();
    releaseSliderAttachment.reset();
    thresholdSliderAttachment.reset();
    ratioSliderAttachment.reset();
    bypassButtonAttachment.reset();
    soloButtonAttachment.reset();
    muteButtonAttachment.reset();
    
    auto& attackParam = getParameterHelper(Pos::Attack);
    addLabelPairs(attackSlider.labels, attackParam, "ms");
    attackSlider.changeParam(&attackParam);
    
    auto& releaseParam = getParameterHelper(Pos::Release);
    addLabelPairs(releaseSlider.labels, releaseParam, "ms");
    releaseSlider.changeParam(&releaseParam);
    
    auto& thresholdParam = getParameterHelper(Pos::Threshold);
    addLabelPairs(thresholdSlider.labels, thresholdParam, "dB");
    thresholdSlider.changeParam(&thresholdParam);
    
    auto& ratioParamRap = getParameterHelper(Pos::Ratio);
    ratioSlider.labels.add({0, "1:1"});
    
    auto ratioParam = dynamic_cast<juce::AudioParameterChoice*>(&ratioParamRap);
    ratioSlider.labels.add({1, juce::String(ratioParam->choices.getReference(ratioParam->choices.size() - 1).getIntValue()) + ":1"});
    ratioSlider.changeParam(&ratioParamRap);
    
    //And finally, make the attachments
    makeAttachment(attackSliderAttachment, names[Pos::Attack], attackSlider, params, apvts);
    makeAttachment(releaseSliderAttachment, names[Pos::Release], releaseSlider, params, apvts);
    makeAttachment(thresholdSliderAttachment, names[Pos::Threshold], thresholdSlider, params, apvts);
    makeAttachment(ratioSliderAttachment, names[Pos::Ratio], ratioSlider, params, apvts);
    makeAttachment(bypassButtonAttachment, names[Pos::Bypass], bypassButton, params, apvts);
    makeAttachment(soloButtonAttachment, names[Pos::Solo], soloButton, params, apvts);
    makeAttachment(muteButtonAttachment, names[Pos::Mute], muteButton, params, apvts);
}

void CompressorBandControls::resized()
{
    auto bounds = getLocalBounds().reduced(5);
    
    using namespace juce;
    
    auto createBandControlBox = [](std::vector<Component*> comps)
    {
        FlexBox flexBox;
        flexBox.flexDirection = FlexBox::Direction::column;
        flexBox.flexWrap = FlexBox::Wrap::noWrap;
        
        auto spacer = FlexItem().withHeight(2);
        
        for( auto* comp : comps)
        {
            flexBox.items.add(spacer);
            flexBox.items.add(FlexItem(*comp).withFlex(1));
        }
        
        flexBox.items.add(spacer);
        
        return flexBox;
    };
    
    auto bandButtonControlBox = createBandControlBox({&bypassButton, &soloButton, &muteButton});
    
    auto bandSelectControlBox = createBandControlBox({&lowBandButton, &midBandButton, &highBandButton});
    
    FlexBox flexBox;
    flexBox.flexDirection = FlexBox::Direction::row;
    flexBox.flexWrap = FlexBox::Wrap::noWrap;
    
    auto spacer = FlexItem().withWidth(4);
//    auto endCap = FlexItem().withWidth(6);
    
    flexBox.items.add(FlexItem(bandSelectControlBox).withWidth(50));
    
//    flexBox.items.add(endCap);
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(attackSlider).withFlex(1));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(releaseSlider).withFlex(1));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(thresholdSlider).withFlex(1));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(ratioSlider).withFlex(1));
    flexBox.items.add(spacer);
//    flexBox.items.add(endCap);
    
    flexBox.items.add(FlexItem(bandButtonControlBox).withWidth(30));
    
    flexBox.performLayout(bounds);
}
