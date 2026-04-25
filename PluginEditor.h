#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <deque>

class ButSeriouslyEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit ButSeriouslyEditor(ButSeriouslyProcessor&);
    ~ButSeriouslyEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    ButSeriouslyProcessor& processor;

    juce::Slider sBeta, sDrive, sTime, sFb, sMix;
    juce::Label  lBeta, lDrive, lTime, lFb, lMix;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> aBeta, aDrive, aTime, aFb, aMix;

    // PRESET BUTTONS
    juce::TextButton btnSave {"Save Preset"};
    juce::TextButton btnLoad {"Load Preset"};
    std::unique_ptr<juce::FileChooser> fileChooser;

    std::deque<juce::Point<float>> scopePath;
    static constexpr int MAX_SCOPE_PTS = 200;
    float currentScale = 1.0f;

    void setupSlider(juce::Slider& s, juce::Label& l, const juce::String& name);
    void savePreset();
    void loadPreset();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ButSeriouslyEditor)
};