#pragma once

#include <JuceHeader.h>

class Meter : public Component, public Timer
{
    Meter(std::function<float()>&& valueFunction) : valueSupplier(std::move(valueFunction))
    {
        startTimer(24);
    }
    
public:
    void paint(Graphics& g) override
    {
        const auto level = valueSupplier();
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
    
    void timerCallback() override
    {
        repaint();
    }
    
    std::function<float()> valueSupplier;
    
    ColourGradient gradient;
};
