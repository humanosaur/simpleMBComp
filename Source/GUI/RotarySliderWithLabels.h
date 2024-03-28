/*
  ==============================================================================

    RotarySliderWithLabels.h
    Created: 27 Mar 2024 8:29:21pm
    Author:  Joseph Skonie

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

struct RotarySliderWithLabels : juce::Slider
{
    RotarySliderWithLabels(juce::RangedAudioParameter* rap, const juce::String& unitSuffix, const juce::String& title) : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                    juce::Slider::TextEntryBoxPosition::NoTextBox),
    param(rap),
    suffix(unitSuffix)
    {
        setName(title);
    }
    
    //this structure holds string that displays min and max values and the position to display those values
    struct LabelPos
    {
        float pos;
        juce::String label;
    };
    
    //we can add all our labels in this array and then draw them with the paint function
    juce::Array<LabelPos> labels;
    
    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getSliderBounds() const;
    int getTextHeight() const { return 14; }
    
    virtual juce::String getDisplayString() const;
    
    void changeParam(juce::RangedAudioParameter* p);
    
protected:
    
    juce::RangedAudioParameter* param;
    juce::String suffix;
    
private:
    
};

struct RatioSlider : RotarySliderWithLabels
{
    RatioSlider(juce::RangedAudioParameter* rap, const juce::String& unitSuffix) : RotarySliderWithLabels(rap, unitSuffix, "RATIO") {}
    
    juce::String getDisplayString() const override;
};
