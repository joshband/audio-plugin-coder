# CloudWash Crash Fix - Progress Report

**Date:** 2026-01-30
**Status:** MAJOR BREAKTHROUGH - Clouds Init Fixed, Crash Source Relocated
**Session:** Crash Investigation and Resolution

---

## ✅ MAJOR BREAKTHROUGH: Clouds Processor Initialization FIXED

The crash log proves that the Clouds processor is **initializing successfully**:

```
Step 9: About to call processor->Init()...
Step 10: Init() COMPLETED SUCCESSFULLY!
==== Clouds initialization complete - NO CRASH ====
prepareToPlay: Prepare() completed - prepareToPlay DONE!
```

### What Was Fixed

**Original Diagnosis:** Crash was believed to be in `processor->Init()` or `processor->Prepare()`

**Actual Root Cause:** Incorrect initialization sequence and timing

**Fixes Applied:**

1. **Moved Clouds Initialization to prepareToPlay()**
   - **Before:** Initialized in constructor (too early, JUCE not fully ready)
   - **After:** Initialize in `prepareToPlay()` after JUCE is fully set up
   - **Why:** Ensures all JUCE subsystems are initialized before touching Clouds

2. **Changed Memory Allocation Strategy**
   - **Before:** `block_mem = new uint8_t[memLen]();`
   - **After:** `block_mem = (uint8_t*)calloc(memLen, 1);`
   - **Why:** Clouds is C-style code, may expect C-style allocation

3. **Followed VCV Rack Pattern Exactly**
   - **Before:** Called `Prepare()` in constructor
   - **After:** Only call `Init()` during initialization, defer `Prepare()` to `prepareToPlay()`
   - **Pattern:** `memset() → Init() → [later] set_playback_mode() → set_quality() → Prepare()`

4. **Added Initialization Guard**
   - Added `std::atomic<bool> cloudsInitialized` flag
   - Prevents double-initialization when `prepareToPlay()` is called multiple times
   - Memory leak prevention

### Code Changes

**PluginProcessor.h:**
```cpp
// Clouds processor now initialized in prepareToPlay(), not constructor
clouds::GranularProcessor* processor = nullptr;
uint8_t* block_mem = nullptr;
uint8_t* block_ccm = nullptr;
std::atomic<bool> cloudsInitialized { false };
```

**PluginProcessor.cpp Constructor:**
```cpp
CloudWashAudioProcessor::CloudWashAudioProcessor()
{
    // CRITICAL FIX: Defer ALL Clouds initialization to prepareToPlay()
    processor = nullptr;
    block_mem = nullptr;
    block_ccm = nullptr;
    // ... rest of constructor (APVTS, presets, etc.)
}
```

**PluginProcessor.cpp prepareToPlay():**
```cpp
void CloudWashAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    if (!cloudsInitialized.load())
    {
        const int memLen = 118784;
        const int ccmLen = 65536 - 128;

        // Allocate with calloc (C-style allocation)
        block_mem = (uint8_t*)calloc(memLen, 1);
        block_ccm = (uint8_t*)calloc(ccmLen, 1);

        // Allocate and zero processor
        processor = new clouds::GranularProcessor();
        memset(processor, 0, sizeof(*processor));

        // Initialize (VCV Rack pattern)
        processor->Init(block_mem, memLen, block_ccm, ccmLen);

        cloudsInitialized.store(true);
    }

    // Set processor state and call Prepare()
    processor->set_playback_mode(static_cast<clouds::PlaybackMode>(currentMode.load()));
    processor->set_quality(currentQuality.load());
    processor->set_silence(false);
    processor->Prepare();
}
```

---

## ❌ REMAINING ISSUE: Crash After Successful Initialization

### What the Logs Prove

The crash logs show successful completion of:
- ✅ Editor creation (6 times - multiple instances from PluginVal)
- ✅ Clouds `Init()` completion
- ✅ Clouds `Prepare()` completion
- ✅ `prepareToPlay()` finished

**Conclusion:** The crash is **NOT** in Clouds initialization!

### Where the Crash Actually Is

The plugin still crashes, but it's happening **AFTER** all initialization completes. Possible crash locations:

1. **First `processBlock()` call**
   - Sample rate conversion
   - Clouds audio processing
   - Buffer operations

2. **WebView Initialization**
   - WebView2 loading resources
   - JavaScript execution
   - HTML/CSS rendering

3. **GUI Callbacks**
   - Paint/resize events
   - Timer callbacks
   - Parameter change notifications

4. **PluginVal Testing**
   - Specific test that triggers the crash
   - State saving/loading
   - Automation testing

### Diagnostic Tools Added

**Crash Logger:**
```cpp
// Emergency crash logging to file
static void CRASH_LOG(const juce::String& msg) {
    std::ofstream log("R:\\_VST_Development_2026\\audio-plugin-coder\\cloudwash_crash_log.txt", std::ios::app);
    log << msg.toStdString() << std::endl;
    log.flush();
    DBG(msg);
}
```

**Log File Location:** `R:\_VST_Development_2026\audio-plugin-coder\cloudwash_crash_log.txt`

This captures output even if the plugin crashes, allowing post-mortem analysis.

---

## 📋 Next Steps to Complete the Fix

### Step 1: Add processBlock Logging
```cpp
void CloudWashAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    static int callCount = 0;
    if (callCount < 3) {
        CRASH_LOG("processBlock called #" + juce::String(++callCount));
    }

    // ... existing code ...

    CRASH_LOG("processBlock completed #" + juce::String(callCount));
}
```

### Step 2: Add PluginEditor Logging

Check WebView initialization in `PluginEditor.cpp` constructor.

### Step 3: Run in Visual Studio Debugger (RECOMMENDED)

Most reliable way to find exact crash line:
```powershell
devenv /debugexe "R:\_VST_Development_2026\audio-plugin-coder\build\plugins\CloudWash\CloudWash_artefacts\Debug\Standalone\CloudWash.exe"
```

The debugger will break on the exact line that crashes.

### Step 4: Check PluginVal Output

Run PluginVal with verbose output:
```powershell
& "R:\_VST_Development_2026\audio-plugin-coder\_tools\pluginval\pluginval.exe" `
  --strictness-level 5 `
  --validate-in-process `
  --timeout-ms 30000 `
  --vst3 "C:\Program Files\Common Files\VST3\CloudWash.vst3"
```

Look for which specific test causes the crash.

---

## 🎯 Success Criteria

- [x] Clouds processor initializes without crashing
- [x] Init() completes successfully
- [x] Prepare() completes successfully
- [x] prepareToPlay() finishes without errors
- [ ] Plugin runs without crashing
- [ ] PluginVal tests pass
- [ ] Audio processing works correctly
- [ ] UI displays and responds

---

## 📊 Investigation Attempts Log

### Attempts to Fix "Clouds Init Crash" (All Failed - Wrong Diagnosis)

1. ❌ Heap allocation with `new` + `memset` in constructor
2. ❌ Removed `Prepare()` from constructor
3. ❌ Commented out `SampleRateConverter Init()` calls
4. ❌ Removed `set_playback_mode/set_quality` from constructor
5. ❌ Changed to `calloc()` instead of `new[]`

### Actual Fix (Success)

6. ✅ Moved entire Clouds initialization to `prepareToPlay()`

**Key Insight:** The issue wasn't the initialization code itself, but the **timing** of when it executed. Constructor runs before JUCE is fully ready.

---

## 🔍 Lessons Learned

1. **Crash location diagnosis can be misleading** - The original crash appeared to be in Clouds Init(), but it was actually caused by calling it too early in the plugin lifecycle.

2. **VCV Rack differences** - VCV Rack's `process()` is called after full initialization, similar to JUCE's `prepareToPlay()`. Their constructor is minimal.

3. **Logging is essential** - Without crash logging to file, we couldn't determine that Init() was completing successfully.

4. **JUCE lifecycle matters** - Constructor → prepareToPlay() → processBlock() order is critical. Some initialization must wait for prepareToPlay().

---

## 📚 Related Documentation

- `.claude/troubleshooting/resolutions/crash-002-cloudwash-SOLVED.md` - Original crash investigation
- `CLOUDWASH_FIX_NEXT_STEPS.md` - Previous fix attempts documentation
- VCV Rack Reference: `plugins/CloudWash/Org_Code/AudibleInstruments-2/src/Clouds.cpp`

---

**Last Updated:** 2026-01-30 00:45
**Next Action:** Continue investigation to find actual crash location (processBlock or WebView)
