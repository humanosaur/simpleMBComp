/*
  ==============================================================================

    ControlBar.cpp
    Created: 5 Apr 2024 9:09:22am
    Author:  Joseph Skonie

  ==============================================================================
*/

#include "ControlBar.h"


ControlBar::ControlBar()
{
    analyzerButton.setToggleState(true, juce::dontSendNotification);
    addAndMakeVisible(analyzerButton);
    
    addAndMakeVisible(globalBypassButton);
}

void ControlBar::resized()
{
    auto bounds = getLocalBounds();
    
    analyzerButton.setBounds(bounds.removeFromLeft(50).withTrimmedTop(4).withTrimmedLeft(4));
    
    globalBypassButton.setBounds(bounds.removeFromRight(50).withTrimmedTop(4).withTrimmedBottom(4))
    ;}
