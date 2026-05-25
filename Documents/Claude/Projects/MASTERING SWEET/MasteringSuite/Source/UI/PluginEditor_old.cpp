#include "PluginEditor.h"
#include "../PluginProcessor.h"

MasteringSuiteEditor::MasteringSuiteEditor(MasteringSuiteProcessor& proc)
    : AudioProcessorEditor(proc), processor(proc)
{
    setSize(1000, 700);
    setResizable(false, false);

    // Apply custom look and feel
    setLookAndFeel(&lookAndFeel);

    // Create 5-zone components
    sweetHeader = std::make_unique<SweetHeader>();
    moduleTabStrip = std::make_unique<ModuleTabStrip>();
    moduleHeader = std::make_unique<ModuleHeader>();
    bottomStrip = std::make_unique<BottomStrip>(proc.getAPVTS());
    footerBar = std::make_unique<FooterBar>();

    // Module content
    dynamicsModule = std::make_unique<DynamicsModule>(proc.getCompressor(), proc.getAPVTS());

    addAndMakeVisible(*sweetHeader);
    addAndMakeVisible(*moduleTabStrip);
    addAndMakeVisible(*moduleHeader);
    addAndMakeVisible(*bottomStrip);
    addAndMakeVisible(*footerBar);
    addAndMakeVisible(*dynamicsModule);

    // Set initial state
    moduleTabStrip->setActiveModule(activeModule);
    moduleHeader->setTitle("DYNAMICS", "Compression & Expansion");

    // Tab switching callback
    moduleTabStrip->onModuleSelected = [this](int idx) {
        showModule(idx);
    };

    // Start meter refresh timer (33ms = 30Hz)
    startTimer(Theme::Animation::METER_REFRESH_MS);
}

MasteringSuiteEditor::~MasteringSuiteEditor()
{
    stopTimer();
}

void MasteringSuiteEditor::paint(juce::Graphics& g)
{
    g.fillAll(Theme::Color::toColour(Theme::Color::BG_0));
}

void MasteringSuiteEditor::resized()
{
    auto bounds = getLocalBounds();

    // SweetHeader (y=0, h=50)
    sweetHeader->setBounds(bounds.removeFromTop(50));

    // ModuleTabStrip (y=50, h=44)
    moduleTabStrip->setBounds(bounds.removeFromTop(44));

    // ModuleHeader (y=94, h=44)
    moduleHeader->setBounds(bounds.removeFromTop(44));

    // ModuleContentArea (y=138, h~342)
    auto contentArea = bounds.removeFromTop(342);
    dynamicsModule->setBounds(contentArea);

    // BottomStrip (y=480, h=120)
    bottomStrip->setBounds(bounds.removeFromTop(120));

    // FooterBar (y=600, h=100)
    footerBar->setBounds(bounds);
}

void MasteringSuiteEditor::timerCallback()
{
    // Update meter values from EbuR128Meter
    auto& meter = processor.getMeter();
    float lufsI = meter.getIntegratedLufs();
    float lufsS = meter.getShortTermLufs();
    float lufsM = meter.getMomentaryLufs();
    float truePeak = meter.getTruePeakDbfs();

    bottomStrip->setMeterValues(lufsI, lufsS, lufsM, truePeak);

    // Update current parameter display based on active module
    if (activeModule == 1) {  // DYNAMICS
        auto threshold = processor.getAPVTS().getRawParameterValue("dynThreshold")->load();
        bottomStrip->setCurrentParameter("Threshold", juce::String::formatted("%.1f dB", threshold));
    }

    // Update system info
    moduleTabStrip->updateSystemInfo(processor.getSampleRate(), processor.getBlockSize());

    // Update footer status
    auto latencyMs = processor.getTailLengthSeconds() * 1000.0;
    footerBar->setStatusText(juce::String::fromUTF8(juce::String::formatted("SR %.1fK \xC2\xB7 32-BIT FLOAT \xC2\xB7 LATENCY %.1f MS",
                                                       processor.getSampleRate() / 1000.0,
                                                       latencyMs)));
    footerBar->setCpuUsage(0.0f);  // Placeholder for actual CPU measurement}

void MasteringSuiteEditor::showModule(int moduleIndex)
{
    if (moduleIndex < 0 || moduleIndex > 5)
        return;
    if (moduleIndex == activeModule)
        return;

    activeModule = moduleIndex;

    // Update module header title
    const juce::StringArray moduleNames{"EQUALIZER", "DYNAMICS", "HARMONICS", "IMAGER", "CLIPPER", "LIMITER"};
    const juce::StringArray moduleSubtitles{"High/Low-Shelf Filtering", "Compression & Expansion",
                                            "Saturation & Harmonics", "Stereo Width Control",
                                            "Peak Limiting", "Soft Knee Dynamics"};

    moduleHeader->setTitle(moduleNames[moduleIndex], moduleSubtitles[moduleIndex]);

    // Show/hide module content (only DYNAMICS for now)
    dynamicsModule->setVisible(moduleIndex == 1);

    // Update tab indicator
    moduleTabStrip->setActiveModule(moduleIndex);
}
