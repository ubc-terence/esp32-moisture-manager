@AGENTS.md

## Claude Code

- Shared project config is committed in `.claude/settings.json` (safety rails only: it denies reading
  `include/config.local.h` and running flash-erase commands). Put your own permissions and preferences in
  `.claude/settings.local.json` or `CLAUDE.local.md`; both are git-ignored.
- Project skills (in `.claude/skills/`): `/verify-firmware` (tests, build, optional flash and serial check) and
  `/diagnose-sensor` (is the sensor pin driven or floating?).
- Keep machine-specific facts (serial port names, home paths, your PlatformIO layout) out of the checked-in
  files. They belong in your own `CLAUDE.local.md` or memory.
- Keep `AGENTS.md` and this file short. Only add what a fresh session can't derive by reading the code:
  gotchas, measured hardware facts and safety rules. It is loaded into every session.
