# JUCE 8 CRITICAL SYSTEM PROTOCOLS
**REQUIRED READING:** strict constraints for Windows 11 & APC Monorepo.
## 1. ⚠️ GOLDEN BUILD RULES (HIGHEST PRIORITY)
### A. The "One-Script" Rule
- **NEVER** run cmake, msbuild, or cl.exe manually.
- **NEVER** try to copy VST3 files manually.
- **ALWAYS** use the master script for Building, Installing, and Repairing.
- **COMMAND:** .\scripts\build-and-install.ps1 -PluginName "TailSync"
### B. Monorepo & Path Logic
- **Root Context:** All build operations must happen from the Repository Root _nps/).
- **Subdirectories:** NEVER run commands inside plugins/[Name]/.
- **Environment:**
  - OS: **Windows 11**
  - Shell: **PowerShell** (Bashrmmkdir -p are FORBIDDEN).
  - Create Folders: New-Item -ItemType Directory -Force -Path "..."
---
## 2. 📂 FILE STRUCTURE & WEBVIEW
### A. WebView/GUI Architecture
- **Location:** HTML/JS/CSS files MUST reside in plugins/[Name]/WebUI/.
- **Forbidden:** Do NOT use Source/ui/public or Resources/web.
- **C++ Pathing:** In PluginEditor.cpp, load files dynamically or via hardcoded dev path during testing.
- **JS Interop:** Do NOT create a juce subfolder. Access native backend via window.__JUCE__.
### B. CMake Configuration
- **Root:** The Root CMakeLists.txt loads JUCE.
- **Plugins:** Plugin CMakeLists must **NOT** call juce_add_modules (causes duplicate target errors).
- **Correct Linking:**
  ```cmake
  juce_add_plugin(TailSync FORMATS VST3 PRODUCT_NAME "TailSync" NEEDS_WEB_BROWSER TRUE)
  target_link_libraries(TailSync PRIVATE juce::juce_dsp juce::juce_gui_extra)
  target_compile_definitions(TailSync PUBLIC JUCE_WEB_BROWSER=1)