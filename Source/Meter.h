#pragma once

#include <JuceHeader.h>

class Meter : public Component
{
    
public:
    void paint(Graphics& g) override
    {
        auto bounds = getLocalBounds();
        
        g.setColour(Colours::black);
        g.fillRect(bounds);
        
        g.setGradientFill(gradient);
        const auto scaledY = jmap(level, -60.f, 6.f, 0.f, static_cast<float>(getHeight()));
        g.fillRect(bounds.removeFromBottom(scaledY));
    
    }
    
    void resized() override
    {
        const auto bounds = getLocalBounds();
        gradient = ColourGradient(Colours::green, bounds.getBottomLeft().toFloat(), Colours::red, bounds.getTopLeft().toFloat(), false);
        gradient.addColour(0.5, Colours::yellow);
    }
    
    void setLevel(const float value)
    {
        level = value;
    }
    
private:
    
    float level = -60.f;
        
    ColourGradient gradient;
};
