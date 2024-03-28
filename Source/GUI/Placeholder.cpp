/*
  ==============================================================================

    Placeholder.cpp
    Created: 27 Mar 2024 8:29:35pm
    Author:  Joseph Skonie

  ==============================================================================
*/

#include "Placeholder.h"

Placeholder::Placeholder()
{
    juce::Random r;
    customColor = juce::Colour(r.nextInt(255),r.nextInt(255),r.nextInt(255));
}
