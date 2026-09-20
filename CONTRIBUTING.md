# Contributing

Thanks for helping. This is a small hobby project, so the process is light.

## Before you open a pull request

```sh
pio test -e native      # unit tests: must pass
pio run                 # firmware must build
pio run -t buildfs      # only if you touched data/index.html
```

- Say in the PR whether you **tested on hardware**, and which board and sensor. A change that only
  compiles is fine, but the PR should say so plainly.
- If you changed the dashboard, include a screenshot.
- Keep changes focused. Unrelated cleanups belong in their own PR.
- Commit messages: an imperative subject line, then a short body explaining *why*.

New pure logic should come with a unit test in `test/` (see the existing ones). Hardware-dependent code
can't be unit-tested on the host; describe how you verified it.

## Setting up

See [docs/development.md](docs/development.md) for commands, storage layout, diagnostics and
troubleshooting. Set your own Wi-Fi name and password in `include/config.local.h` (git-ignored, copy it from
`include/config.local.example.h`); never commit it.

## Other boards and sensors

The firmware is tested on one board and one sensor. Ports and reports are very welcome: open an issue with
your board, sensor, wiring and the serial output. Pins are constants at the top of `src/main.cpp`. Please keep
the `docs/hardware.md` numbers labeled with the hardware they were measured on.

## Working with Claude Code and other coding agents

The repo is set up so each contributor's own agent setup works well without stepping on anyone else's:

- **`AGENTS.md`** is the tool-neutral project guide; **`CLAUDE.md`** imports it for Claude Code. Both are loaded
  into every session, so keep them short and add only what a fresh session *can't derive from the code*:
  gotchas, measured hardware facts, safety rules. Don't paste layouts, dependency lists or things a linter
  already enforces.
- **`.claude/settings.json`** is shared and deliberately minimal: it only denies reading `include/config.local.h`
  and running flash-erase commands. Please don't add broad `allow` rules there. Put your own permissions and
  preferences in `.claude/settings.local.json` or `CLAUDE.local.md`, which are git-ignored.
- **`.claude/skills/`** holds shared project workflows (`/verify-firmware`, `/diagnose-sensor`). Add a new skill
  when a multi-step procedure keeps coming up; keep each `SKILL.md` short and specific.
- **Personal notes and memory stay local.** Don't commit machine-specific facts such as serial port names, home
  directory paths or your PlatformIO layout.
- **Hardware is a person's real device.** Agents should ask before flashing, running `uploadfs` (which wipes the
  saved history) or writing calibration, and should never erase flash. The shared settings enforce the last one.
- Agent-authored changes go through the same review as any other: run the checks above and read the diff.
