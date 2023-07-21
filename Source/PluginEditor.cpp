/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BasicCompressorAudioProcessorEditor::BasicCompressorAudioProcessorEditor (BasicCompressorAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),

attackAttach(audioProcessor.apvts, "attack", attack),
releaseAttach(audioProcessor.apvts, "release", release),
threshAttach(audioProcessor.apvts, "threshold", threshold),
ratioAttach(audioProcessor.apvts, "ratio", ratio),
inputGainAttach(audioProcessor.apvts, "inputGain", inputGain),
outputGainAttach(audioProcessor.apvts, "outputGain", outputGain),
bypassAttach(audioProcessor.apvts, "bypass", bypass)

{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    for(auto* slider : getSliders()){
        slider->setSliderStyle(Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, false, 50, 20);
        addAndMakeVisible(slider);
    }
    
    addAndMakeVisible(audioProcessor.waveViewer);
    audioProcessor.waveViewer.setColours(Colours::darkgrey, Colours::black);
    
    addAndMakeVisible(waveZoom);
    waveZoom.setSliderStyle(Slider::SliderStyle::LinearHorizontal);
    waveZoom.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    waveZoom.setRange(128, 1024);
    waveZoom.setValue(576);
    waveZoom.onValueChange = [this]()
    {
        audioProcessor.waveViewer.setBufferSize(waveZoom.getValue());
    };
    
    addAndMakeVisible(bypass);
    prepTextButton(&bypass, "A/B");
    
    addAndMakeVisible(inputMeterL);
    addAndMakeVisible(inputMeterR);
    addAndMakeVisible(outputMeterL);
    addAndMakeVisible(outputMeterR);

    startTimerHz(60);
    
    setSize (600, 500);

}

BasicCompressorAudioProcessorEditor::~BasicCompressorAudioProcessorEditor()
{
}

//==============================================================================
void BasicCompressorAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(Colours::black);
    auto bounds = getLocalBounds();
    auto titleBar = bounds.removeFromTop(bounds.getHeight() * 0.1).reduced(5.f);
    auto topArea = bounds.removeFromTop(bounds.getHeight() * 0.7).reduced(5.f);
    auto dials = bounds.reduced(5.f);
    
    g.setColour(Colours::skyblue);
    g.fillRect(topArea.toFloat());
    g.fillRect(dials.toFloat());
    g.fillRect(titleBar.toFloat());
    
}

void BasicCompressorAudioProcessorEditor::paintOverChildren(Graphics& g)
{
    if(*audioProcessor.bypassPtr == false)
    {
        auto bounds = getLocalBounds();
        auto topArea = bounds.removeFromTop(bounds.getHeight() * 0.7).reduced(5.f);
        auto graphArea = topArea.removeFromLeft(topArea.getWidth() * 0.7).reduced(5.f);
        
        auto left = graphArea.getX();
        auto right = graphArea.getRight();
        auto top = graphArea.getY();
        auto midline = graphArea.getHeight()/2;
        auto bottom = graphArea.getBottom();
        
        //    g.drawHorizontalLine(, left, right);
        
        auto yTop = jmap(float(threshold.getValue()), -60.f, 20.f, float(midline), float(top));
        auto yBottom = jmap(float(threshold.getValue()), -60.f, 20.f, float(midline), float(bottom));
        
        auto topHorizontalLine = Rectangle<float>(left, yTop, right-left, 3.f);
        auto bottomHorizontalLine = Rectangle<float>(left, yBottom, right-left, 3.f);
        
        g.setColour(Colours::lightgrey.withAlpha(0.25f));
        g.fillRect(Rectangle<float>(topHorizontalLine.getBottomLeft().getX(), yTop, right-left, bottomHorizontalLine.getY()-topHorizontalLine.getY()));
        
        g.setColour(Colours::black);
        g.fillRect(topHorizontalLine);
        g.fillRect(bottomHorizontalLine);
    }
}

void BasicCompressorAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds();
    auto titleBar = bounds.removeFromTop(bounds.getHeight() * 0.1);
    auto topArea = bounds.removeFromTop(bounds.getHeight() * 0.7).reduced(5.f);
    auto waveViewerBounds = topArea.removeFromLeft(topArea.getWidth() * 0.7).reduced(5.f);
    waveZoom.setBounds(waveViewerBounds.removeFromBottom(waveViewerBounds.getHeight() * 0.2));
    audioProcessor.waveViewer.setBounds(waveViewerBounds);
    
    auto gainBounds = topArea;
    auto gainControl = gainBounds.removeFromTop(gainBounds.getHeight() * 0.3);
    inputGain.setBounds(gainControl.removeFromLeft(gainControl.getWidth() * 0.5));
    outputGain.setBounds(gainControl);
    auto inMeterBounds = gainBounds.removeFromLeft(gainBounds.getWidth() * 0.5).reduced(10.f, 5.f);
    auto outMeterBounds = gainBounds.reduced(10.f, 5.f);

    inputMeterL.setBounds(inMeterBounds.removeFromLeft(inMeterBounds.getWidth() * 0.5).reduced(2.5f, 0));
    inputMeterR.setBounds(inMeterBounds.reduced(2.5f, 0));
    outputMeterL.setBounds(outMeterBounds.removeFromLeft(outMeterBounds.getWidth() * 0.5).reduced(2.5f, 0));
    outputMeterR.setBounds(outMeterBounds.reduced(2.5f, 0));

    
    bounds.reduced(10.f);
    attack.setBounds(bounds.removeFromLeft(bounds.getWidth() * 0.25).reduced(10.f));
    release.setBounds(bounds.removeFromLeft(bounds.getWidth() * 0.33).reduced(10.f));
    threshold.setBounds(bounds.removeFromLeft(bounds.getWidth() * 0.5).reduced(10.f));
    ratio.setBounds(bounds.reduced(10.f));
    bypass.setBounds(titleBar.removeFromRight(titleBar.getWidth() * 0.1).reduced(5.f));
}

void BasicCompressorAudioProcessorEditor::timerCallback()
{
    inputMeterL.setLevel(audioProcessor.getRmsLevel(true, 0));
    inputMeterR.setLevel(audioProcessor.getRmsLevel(true, 1));
    outputMeterL.setLevel(audioProcessor.getRmsLevel(false, 0));
    outputMeterR.setLevel(audioProcessor.getRmsLevel(false, 1));
    
    inputMeterR.repaint();
    inputMeterL.repaint();
    outputMeterR.repaint();
    outputMeterL.repaint();
}

void BasicCompressorAudioProcessorEditor::prepTextButton(TextButton* button, String text)
{
    button->setColour(TextButton::ColourIds::buttonOnColourId, Colours::green);
    button->setColour(TextButton::ColourIds::buttonColourId, Colours::red);
    button->setButtonText(text);
    button->setColour(TextButton::ColourIds::textColourOnId, Colours::black);
    button->setColour(TextButton::ColourIds::textColourOffId, Colours::black);
    button->setClickingTogglesState(true);
}


std::vector<juce::Slider*> BasicCompressorAudioProcessorEditor::getSliders()
{
    return
    {
        &attack,
        &release,
        &threshold,
        &ratio,
        &inputGain,
        &outputGain
    };
}
