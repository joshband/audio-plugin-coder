# Plan - Architecture & Framework Selection

**Trigger:** `/plan [PluginName]`
**Phase:** PHASE 2 - Planning
**Primary Skill:** `.claude\skills\skill_planning\SKILL.md`

---

## EXECUTION

When invoked, execute the complete workflow from:
**`.claude\skills\skill_planning\SKILL.md`**

## WORKFLOW GATES

See `.claude\workflows\plan.md` for:
- Prerequisites (requires completed Dream phase)
- State validation
- Framework selection (Visage vs WebView)
- Completion criteria

## CRITICAL DECISION

This phase MUST determine:
- **UI Framework:** "visage" or "webview"
- **Complexity Score:** 1-5
- **DSP Architecture:** Component design

## PARAMETERS

- `PluginName` - Name of existing plugin from Dream phase

## OUTPUT

Creates:
- `plugins/[Name]/.ideas/architecture.md`
- `plugins/[Name]/.ideas/plan.md`
- Updates `plugins/[Name]/status.json` with framework selection

## NEXT PHASE

After completion, user should run: `/design [PluginName]`
