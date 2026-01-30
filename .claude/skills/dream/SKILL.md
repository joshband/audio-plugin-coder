# Dream - Plugin Ideation Phase

**Trigger:** `/dream [PluginName]`
**Phase:** PHASE 1 - Ideation
**Primary Skill:** `.claude\skills\skill_ideation\SKILL.md`

---

## EXECUTION

When invoked, execute the complete workflow from:
**`.claude\skills\skill_ideation\SKILL.md`**

## WORKFLOW GATES

See `.claude\workflows\dream.md` for:
- Prerequisites (none - entry point)
- State validation
- Completion criteria

## PARAMETERS

- `PluginName` - Name of the plugin to create (e.g., "GainKnob", "TailSync")

## OUTPUT

Creates:
- `plugins/[Name]/.ideas/creative-brief.md`
- `plugins/[Name]/.ideas/parameter-spec.md`
- `plugins/[Name]/status.json`

## NEXT PHASE

After completion, user should run: `/plan [PluginName]`
