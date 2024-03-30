/*
  ==============================================================================

    LookAndFeel.cpp
    Created: 27 Mar 2024 8:25:58pm
    Author:  Joseph Skonie

  ==============================================================================
*/

#include "LookAndFeel.h"
#include "RotarySliderWithLabels.h"
#include "CustomButtons.h"

void LookAndFeel::drawRotarySlider(juce::Graphics& g,
                                   int x,
                                   int y,
                                   int width,
                                   int height,
                                   float sliderPosProportional,
                                   float rotaryStartAngle,
                                   float rotaryEndAngle,
                                   juce::Slider & slider)
{
    using namespace juce;
    
    auto bounds = Rectangle<float>(x, y, width, height);
    
    auto enabled = slider.isEnabled();
    
    //create and fill a circle
    g.setColour(enabled ? Colour(327.f, 62.f, 75.f, 0.3f) : Colours::dimgrey);
    g.fillEllipse(bounds);
    
    //draw a border around the circle
    g.setColour(Colours::black);
    g.drawEllipse(bounds, 0.5f);
    
    if ( auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider))
    {
        //create a narrow rectangle to represent the indicator of the rotary dial
        auto center = bounds.getCentre();
        
        Path p;
        
        Rectangle<float> r;
        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setTop(bounds.getY());
        r.setBottom(center.getY() - rswl->getTextHeight() * 2);
        
        g.setColour(Colours::black);
        p.addRoundedRectangle(r,2.f);
        
        jassert(rotaryStartAngle < rotaryEndAngle);
        
        //convert slider's normalized value to an angle in radians
        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);
        
        //rotate the rectangle to the angle we just calculated
        p.applyTransform(AffineTransform().rotated(sliderAngRad,center.getX(), center.getY()));
        
        g.fillPath(p);
        
        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);
        
        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(bounds.getCentre());
        
        //g.setColour(Colours::blanchedalmond);
        //g.fillRect(r);
        
        g.setColour(Colours::black);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
    }
}

void LookAndFeel::drawToggleButton(juce::Graphics &g,
                                   juce::ToggleButton &toggleButton,
                                   bool shouldDrawButtonAsHighlighted,
                                   bool shouldDrawButtonAsDown)
{
    using namespace juce;
    
    if( auto* pb = dynamic_cast<PowerButton*>(&toggleButton))
    {
        Path powerButton;
        
        auto bounds = toggleButton.getLocalBounds();
        
        g.setColour(Colour(105u,60u,28u));
        g.fillRect(bounds);
        g.setColour(Colours::black);
        g.drawRect(bounds);
        
        auto size = jmin(bounds.getWidth(),bounds.getHeight()) - 6; //JUCE_LIVE_CONSTANT(6);
        auto r = bounds.withSizeKeepingCentre(size, size).toFloat();
        
        float ang = 24.f; //JUCE_LIVE_CONSTANT(30);
        
        size -= 7; //JUCE_LIVE_CONSTANT(6);
        
        powerButton.addCentredArc(r.getCentreX(),
                                  r.getCentreY(),
                                  size * 0.5,
                                  size * 0.5,
                                  0.f,
                                  degreesToRadians(ang),
                                  degreesToRadians(360.f - ang),
                                  true);
        
        powerButton.startNewSubPath(r.getCentreX(), r.getY());
        powerButton.lineTo(r.getCentre());
        
        PathStrokeType pst(1.f,PathStrokeType::JointStyle::curved);
        
        auto color = toggleButton.getToggleState() ? Colours::dimgrey : Colour(242u, 65u, 163u);
        
        g.setColour(color);
        g.strokePath(powerButton, pst);
        g.drawEllipse(r,1.5);
    }
    else if( auto* analyzerButton = dynamic_cast<AnalyzerButton*>(&toggleButton))
    {
        auto color = ! toggleButton.getToggleState() ? Colours::dimgrey : Colour(242u, 65u, 163u);
        
        g.setColour(color);
        
        auto bounds = toggleButton.getLocalBounds();
        g.drawRect(bounds);
        
        g.strokePath(analyzerButton->randomPath, PathStrokeType(1.f));
    }
    else
    {
        auto bounds = toggleButton.getLocalBounds().reduced(2);
        
        auto buttonIsOn = toggleButton.getToggleState();
        
        const int cornerSize = 4;
        
        g.setColour(buttonIsOn ?
                    toggleButton.findColour(TextButton::ColourIds::buttonOnColourId) :
                    toggleButton.findColour(TextButton::ColourIds::buttonColourId));
        
        g.fillRoundedRectangle(bounds.toFloat(), cornerSize); //background
        
        g.setColour(buttonIsOn ? juce::Colours::black : juce::Colours::white);
        g.drawRoundedRectangle(bounds.toFloat(), cornerSize, 1); //border
        g.drawFittedText(toggleButton.getName(), bounds, juce::Justification::centred, 1); //text
        
    }
}
