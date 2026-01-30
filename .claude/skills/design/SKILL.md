# Design - UI Design Phase

**Trigger:** `/design [PluginName]`
**Phase:** PHASE 3 - Design
**Primary Skill:** `.claude\skills\skill_design\SKILL.md`

---

## EXECUTION

When invoked, execute the complete workflow from:
**`.claude\skills\skill_design\SKILL.md`**

## WORKFLOW GATES

See `.claude\workflows\design.md` for:
- Prerequisites (plan_complete phase required)
- Framework selection validation
- Completion criteria

## PARAMETERS

- `PluginName` - Name of the plugin to design (e.g., "GainKnob", "TailSync")

## OUTPUT

Creates:
- `plugins/[Name]/Design/v1-ui-spec.md`
- `plugins/[Name]/Design/v1-style-guide.md`
- `plugins/[Name]/Design/v1-test.html` (WebView preview)
- `plugins/[Name]/Source/VisageControls.h` (Visage only, if approved for implementation)

## NEXT PHASE

After completion, user should run: `/impl [PluginName]`
