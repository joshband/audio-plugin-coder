# Ship - Packaging & Distribution

**Trigger:** `/ship [PluginName]`
**Phase:** PHASE 5 - Shipping
**Primary Skill:** `.claude\skills\skill_packaging\SKILL.md`

---

## EXECUTION

When invoked, execute the complete workflow from:
**`.claude\skills\skill_packaging\SKILL.md`**

## WORKFLOW GATES

See `.claude\workflows\ship.md` for:
- Prerequisites (requires completed Implementation phase)
- State validation
- Testing requirements
- Distribution preparation

## PARAMETERS

- `PluginName` - Name of existing plugin from previous phases

## OUTPUT

Creates:
- `dist/[Name]_v[version]/` - Distribution package
- Final VST3 build
- Installation instructions
- Updates `plugins/[Name]/status.json` to "ship_complete"

## NEXT STEPS

After shipping, plugin is ready for distribution or further iteration.
