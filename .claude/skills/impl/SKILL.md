# Impl - DSP Implementation

**Trigger:** `/impl [PluginName]`
**Phase:** PHASE 4 - Implementation
**Primary Skill:** `.claude\skills\skill_implementation\SKILL.md`

---

## EXECUTION

When invoked, execute the complete workflow from:
**`.claude\skills\skill_implementation\SKILL.md`**

## WORKFLOW GATES

See `.claude\workflows\impl.md` for:
- Prerequisites (requires completed Design phase)
- State validation
- Build and test procedures
- Completion criteria

## PARAMETERS

- `PluginName` - Name of existing plugin from previous phases

## OUTPUT

Creates/Updates:
- `plugins/[Name]/Source/PluginProcessor.h`
- `plugins/[Name]/Source/PluginProcessor.cpp`
- `plugins/[Name]/Source/PluginEditor.h`
- `plugins/[Name]/Source/PluginEditor.cpp`
- Framework-specific files (VisageControls.h or WebView integration)
- Updates `plugins/[Name]/status.json`

## BUILD COMMAND

After implementation:
```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build-and-install.ps1 -PluginName [Name]
```

## NEXT PHASE

After completion, user should run: `/ship [PluginName]`
