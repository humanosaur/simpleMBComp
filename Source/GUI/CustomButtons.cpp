/*
  ==============================================================================

    CustomButtons.cpp
    Created: 27 Mar 2024 8:29:53pm
    Author:  Joseph Skonie

  ==============================================================================
*/

#include "CustomButtons.h"

void AnalyzerButton::resized()
{
    auto bounds = getLocalBounds();
    auto insetRect = bounds.reduced(4);
    
    randomPath.clear();
    
    juce::Random r;
    
    randomPath.startNewSubPath(insetRect.getX(), insetRect.getY() + insetRect.getHeight() * r.nextFloat());
    
    for( auto x = insetRect.getX() + 1; x < insetRect.getRight() ; x += 2)
    {
        randomPath.lineTo(x, insetRect.getY() + insetRect.getHeight() * r.nextFloat());
        
    }
}
