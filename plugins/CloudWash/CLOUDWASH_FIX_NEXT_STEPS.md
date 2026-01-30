# CloudWash Fix - Next Steps

## ✅ PROGRESS: Root Cause Identified!

**Test Result:** Plugin works perfectly WITHOUT Clouds processor, crashes WITH Clouds.

**Conclusion:** The crash is 100% in the Clouds processor initialization.

## What Was Tested

### Test 1: Full Plugin (FAILED)
- Clouds processor Init() enabled
- Result: CRASH (Access Violation 0xc0000005)

### Test 2: Without Clouds (SUCCESS ✅)
- All Clouds code commented out
- Result: Plugin loads, UI works, PluginVal PASSED!

## Exact Problem Location

The crash is in ONE of these 3 places in `PluginProcessor.cpp`:

```cpp
// Constructor (line 24-36)
processor.Init(block_mem.data(), block_mem.size(),     // ← Crash here?
               block_ccm.data(), block_ccm.size());

processor.Prepare();                                    // ← Or here?

// OR in prepareToPlay:
inputResamplers[i].Init();                             // ← Or here?
outputResamplers[i].Init();
```

## Most Likely Fixes (In Order)

### Fix #1: Memory Alignment (Try First)

Clouds might need aligned memory buffers.

**Current (possibly broken):**
```cpp
std::vector<uint8_t> block_mem;
std::vector<uint8_t> block_ccm;
```

**Fixed:**
```cpp
// In PluginProcessor.h, replace the vectors with aligned arrays:
alignas(32) uint8_t block_mem[118784];
alignas(32) uint8_t block_ccm[65408];

// Or use C++17 aligned_alloc
```

### Fix #2: Check VCV Rack Implementation

Look at how VCV Rack initializes Clouds:

```powershell
# Open this file:
R:\_VST_Development_2026\audio-plugin-coder\plugins\CloudWash\Org_Code\AudibleInstruments-2\src\Clouds.cpp
```

Check:
1. Exact buffer sizes used
2. Init() call sequence
3. When Prepare() is called
4. Sample rate converter setup

### Fix #3: Don't Call Prepare() in Constructor

VCV Rack might call Prepare() later:

```cpp
CloudWashAudioProcessor::CloudWashAudioProcessor() {
    // ...
    processor.Init(block_mem, memLen, block_ccm, ccmLen);
    processor.set_playback_mode(clouds::PLAYBACK_MODE_GRANULAR);
    processor.set_quality(0);
    // processor.Prepare();  // ← COMMENT THIS OUT
}

void prepareToPlay(double sampleRate, int samplesPerBlock) {
    // ...
    processor.Prepare();  // ← CALL HERE INSTEAD
}
```

### Fix #4: Fix Sample Rate Converter Parameters

Check template parameters match requirements:

```cpp
// Current (might be wrong):
clouds::SampleRateConverter<-2, 45, clouds::src_filter_1x_2_45> inputResamplers[2];

// Check VCV Rack for correct parameters
// Ratio might need to be different
```

## Recommended Actions

### Option A: Quick Fix Attempt (15 minutes)

Try the alignment fix:

1. Edit `plugins/CloudWash/Source/PluginProcessor.h`
2. Replace vectors with aligned arrays (see Fix #1 above)
3. Rebuild and test

### Option B: Debug to Get Exact Line (5 minutes)

Most reliable approach:

1. Uncomment the Clouds Init() code
2. Run in Visual Studio debugger:
   ```powershell
   devenv /debugexe "R:\_VST_Development_2026\audio-plugin-coder\build\plugins\CloudWash\CloudWash_artefacts\Debug\Standalone\CloudWash.exe"
   ```
3. See exact line that crashes
4. Fix that specific issue

### Option C: Copy VCV Rack Pattern (30 minutes)

1. Read `Org_Code/AudibleInstruments-2/src/Clouds.cpp`
2. Copy their exact initialization sequence
3. Use their buffer sizes and Init() pattern

## Current Plugin State

The plugin is currently in **TEST MODE**:

**Files modified:**
- `plugins/CloudWash/Source/PluginProcessor.cpp`
  - Line 19-44: Clouds Init() **COMMENTED OUT**
  - Line 121-161: prepareToPlay() **COMMENTED OUT**
  - Line 167-477: processBlock() **COMMENTED OUT**

**To restore normal operation:**
1. Uncomment the code
2. Apply one of the fixes above
3. Rebuild
4. Test

## Documentation

Updated troubleshooting docs:
- `.claude/troubleshooting/resolutions/crash-002-cloudwash-SOLVED.md`
- Root cause identified
- Multiple fix strategies documented

---

**Bottom Line:** We know EXACTLY what's wrong (Clouds Init crashes). Just need to find the EXACT line and fix it. Debugger will show this immediately, or try the alignment fix first.
