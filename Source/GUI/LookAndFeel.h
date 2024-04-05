/*
  ==============================================================================

    LookAndFeel.h
    Created: 27 Mar 2024 8:25:58pm
    Author:  Joseph Skonie

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

#define USE_LIVE_CONSTANT false

#if USE_LIVE_CONSTANT
#define colorHelper(c) JUCE_LIVE_CONSTANT(c);
#else
#define colorHelper(c) c;
#endif

namespace ColorScheme
{
inline juce::Colour getSliderBorderColor()
{
    return colorHelper( juce::Colour(0xff9c947d) );
}
inline juce::Colour getModuleBorderColor()
{
    return colorHelper( juce::Colour(0xff307d6e) );
}
//inline juce::Colour getGlobalBypassButtonFillColor()
//{
//    return colorHelper( juce::Colour(0xff00cbe7) );
//}
inline juce::Colour getGlobalBypassButtonBorderColor()
{
    return colorHelper( juce::Colour(0xfff450c1))
}

//Text

inline juce::Colour getLabelTextColor()
{
    return colorHelper( juce::Colour(0xff1690c1));
}

}

struct LookAndFeel : juce::LookAndFeel_V4
{
    void drawRotarySlider (juce::Graphics&,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider&) override;
    
    void drawToggleButton (juce::Graphics &g,
                           juce::ToggleButton & toggleButton,
                           bool shouldDrawButtonAsHighlighted,
                           bool shouldDrawButtonAsDown) override;
};
