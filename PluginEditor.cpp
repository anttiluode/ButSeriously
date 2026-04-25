#include "PluginEditor.h"

static const juce::Colour BG { 0xff0a0a12 };
static const juce::Colour TEAL { 0xff44ffaa };
static const juce::Colour PURPLE { 0xffcc66ff };

ButSeriouslyEditor::ButSeriouslyEditor(ButSeriouslyProcessor& p) : AudioProcessorEditor(&p), processor(p) {
    setSize(800, 600);
    setResizable(true, true);
    setResizeLimits(500, 400, 2500, 2000);

    setupSlider(sBeta, lBeta, "Cooling (Beta)");
    setupSlider(sDrive, lDrive, "EML Gate");
    setupSlider(sTime, lTime, "Base Delay (ms)");
    setupSlider(sFb, lFb, "Resonance");
    setupSlider(sMix, lMix, "Mix");

    aBeta = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.apvts, "beta", sBeta);
    aDrive = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.apvts, "eml_drive", sDrive);
    aTime = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.apvts, "time", sTime);
    aFb = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.apvts, "feedback", sFb);
    aMix = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.apvts, "mix", sMix);

    // PRESET SYSTEM INIT
    addAndMakeVisible(btnSave);
    addAndMakeVisible(btnLoad);
    btnSave.onClick = [this]() { savePreset(); };
    btnLoad.onClick = [this]() { loadPreset(); };
    btnSave.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    btnLoad.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);

    startTimerHz(45); 
}

ButSeriouslyEditor::~ButSeriouslyEditor() { stopTimer(); }

void ButSeriouslyEditor::setupSlider(juce::Slider& s, juce::Label& l, const juce::String& name) {
    addAndMakeVisible(s);
    addAndMakeVisible(l);
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 15);
    s.setColour(juce::Slider::thumbColourId, TEAL);
    s.setColour(juce::Slider::rotarySliderFillColourId, PURPLE);
    l.setText(name, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setColour(juce::Label::textColourId, juce::Colours::grey);
}

void ButSeriouslyEditor::savePreset() {
    fileChooser = std::make_unique<juce::FileChooser>("Save Preset", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.xml");
    fileChooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file != juce::File{}) {
                auto state = processor.apvts.copyState();
                std::unique_ptr<juce::XmlElement> xml(state.createXml());
                xml->writeTo(file);
            }
        });
}

void ButSeriouslyEditor::loadPreset() {
    fileChooser = std::make_unique<juce::FileChooser>("Load Preset", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.xml");
    fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file.existsAsFile()) {
                std::unique_ptr<juce::XmlElement> xml(juce::XmlDocument::parse(file));
                if (xml != nullptr && xml->hasTagName(processor.apvts.state.getType())) {
                    processor.apvts.replaceState(juce::ValueTree::fromXml(*xml));
                }
            }
        });
}

void ButSeriouslyEditor::timerCallback() {
    float x = processor.scopeX.load();
    float y = processor.scopeY.load();
    scopePath.push_back({x, y});
    if (scopePath.size() > MAX_SCOPE_PTS) scopePath.pop_front();
    repaint();
}

void ButSeriouslyEditor::paint(juce::Graphics& g) {
    g.fillAll(BG);

    // Draw Header text
    g.setColour(juce::Colours::white.withAlpha(0.7f));
    g.setFont(juce::Font("Courier New", 20.0f, juce::Font::bold));
    g.drawText("But Seriously", 20, 15, 300, 30, juce::Justification::left);

    juce::Rectangle<int> scopeRect(20, 50, getWidth() - 40, getHeight() - 180);
    g.setColour(juce::Colour(0xff111420));
    g.fillRect(scopeRect);
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawRect(scopeRect);

    int cx = scopeRect.getCentreX();
    int cy = scopeRect.getCentreY();
    g.drawLine(cx, scopeRect.getY(), cx, scopeRect.getBottom());
    g.drawLine(scopeRect.getX(), cy, scopeRect.getRight(), cy);

    if (scopePath.size() > 1) {
        float maxAbs = 0.001f;
        for (const auto& pt : scopePath) {
            maxAbs = std::max({maxAbs, std::abs(pt.x), std::abs(pt.y)});
        }
        float minDim = std::min(scopeRect.getWidth(), scopeRect.getHeight());
        float targetScale = (minDim * 0.45f) / maxAbs;

        if (targetScale < currentScale) currentScale = currentScale * 0.8f + targetScale * 0.2f; 
        else                            currentScale = currentScale * 0.98f + targetScale * 0.02f;

        juce::Path p;
        for (size_t i = 0; i < scopePath.size(); ++i) {
            float px = cx + scopePath[i].x * currentScale;
            float py = cy - scopePath[i].y * currentScale; 
            if (i == 0) p.startNewSubPath(px, py);
            else        p.lineTo(px, py);
        }
        
        float beta = sBeta.getValue();
        juce::Colour traceColor = PURPLE.interpolatedWith(TEAL, juce::jlimit(0.0f, 1.0f, beta));
        
        g.setColour(traceColor.withAlpha(0.2f));
        g.strokePath(p, juce::PathStrokeType(4.0f)); 
        g.setColour(traceColor.withAlpha(0.9f));
        g.strokePath(p, juce::PathStrokeType(1.5f));
    }
}

void ButSeriouslyEditor::resized() {
    // Position Preset Buttons
    btnSave.setBounds(getWidth() - 160, 15, 65, 25);
    btnLoad.setBounds(getWidth() - 85, 15, 65, 25);

    int w = 80;
    int y = getHeight() - 110; 
    int spacing = (getWidth() - (w * 5)) / 6;

    int curX = spacing;
    auto positionKnob = [&](juce::Slider& s, juce::Label& l) {
        s.setBounds(curX, y + 20, w, w);
        l.setBounds(curX, y, w, 20);
        curX += w + spacing;
    };

    positionKnob(sBeta, lBeta);
    positionKnob(sDrive, lDrive);
    positionKnob(sTime, lTime);
    positionKnob(sFb, lFb);
    positionKnob(sMix, lMix);
}