# CLAUDE.md — Weasel (小狼毫) refactor

Weasel is a **Windows TSF input method** (fork of `rime/weasel`). First-party C++ (~19k LOC)
bridges the upstream **librime** engine to a Windows front-end. This repo is in a large,
ongoing **refactor** whose rules are below.

## Mission (do not break these)

1. **Simplify / minimize** the first-party C++ and apply better design patterns — but
2. **preserve all existing behavior** (this is not a feature effort), and
3. **fix bugs** only where realistically fixable in this repo (not upstream librime).

Behavior preservation is paramount. Prefer small, mechanical, reviewable edits.

## ⚠️ Hard constraint: no MSVC on this box (yet)

Weasel builds only on **Windows + MSVC** (ATL, WTL, TSF, Boost; `weasel.sln` / `xmake.lua`).
The dev box is now **native Windows 11** (migrated off WSL 2026-07), but **MSVC and the
Windows SDK are not installed** — you still **cannot build or run**
WeaselTSF/WeaselUI/WeaselServer/etc. here. Every change is "blind" to the Windows compiler.
(If VS Build Tools get installed, rewrite this section — real compilation would then become
the first safety net.)

Because of this, the safety net is **review + the portable test harness**, not compilation.

## Verification

- **Portable layer (IPC text-protocol parser + `include/WeaselIPCData.h` data model + boost
  serialization) IS testable on Linux** via `test/host/` — a `windows.h` shim + real parser
  `.cpp`s + golden round-trip tests. Run it for ANY change touching that layer (from Windows,
  via the still-present WSL distro; inside WSL it builds with the mamba env `wtest`, gxx 15 + boost):
  ```
  wsl.exe -d Ubuntu-26.04 -- bash -lc "cd /mnt/d/workspace/weasel && ./test/host/run.sh"
  ```
- Everything else is verified by **close reading + an adversarial review agent** (mandatory
  before every commit) and by keeping the IPC text protocol / boost-archive payloads
  **byte-identical**.

## Workflow (per change)

1. Plan with **OpenSpec** (`openspec` CLI v1.5.0; skills `openspec-propose` / `-apply-change`
   / `-archive-change`). One roadmap item = one change = one commit.
2. Implement the change's tasks.
3. **Spawn a review agent** on the diff (and run `test/host/run.sh` if the portable layer is
   touched). Only commit if it passes.
4. **Commit on `master`** with a conventional-commit message; end with the required
   `Co-Authored-By` trailer. Then `openspec archive` the change.

Roadmap and full module/issue audit are tracked in OpenSpec changes and in agent memory.

## Conventions (living — Convention over Configuration)

Working conventions for this refactor, distilled from experience. They override unstated defaults.
**Mutable:** when one proves unsuitable, change it here and note why — this section is meant to
evolve as we learn.

### Editing

- **Line endings are MIXED — some files are CRLF, some LF.** `.gitattributes` pins `* -text`
  (no eol conversion) and the repo-local config sets `core.autocrlf=false` — Git-for-Windows'
  system-level `autocrlf=true` once CRLF-converted the whole worktree and broke `run.sh`
  (renormalized 2026-07-11); never re-enable conversion.
  Preserve each file's endings. The `Edit` tool preserves them — prefer it. For scripted/bulk edits,
  work in BINARY: detect the newline (`b'\r\n' if b'\r\n' in data else b'\n'`) and write bytes. After
  ANY edit, check `git diff --stat`: churn far larger than your change means a line-ending (or
  BOM/encoding) accident — revert and redo. Some files also carry a UTF-8 BOM (e.g.
  `WeaselServer.cpp`); don't strip it.
- Match surrounding code exactly: **Chromium** `clang-format`, `SortIncludes: false`.
- Never touch third-party / build-output trees: `include/wtl/`, `librime/`, `plum/`, `lib*/`,
  `output/`.
- `include/rime*.h` (the librime C API, incl. `RIME_STRUCT`) is git-ignored and **not present
  locally** — you cannot read it from the tree; don't rely on being able to.

### Proving code is "dead" (required before ANY removal — no compiler to catch mistakes)

- Grep the WHOLE repo across ALL file kinds: `*.cpp *.h *.hpp *.c *.cc *.rc *.def *.idl *.vcxproj
  *.vcxproj.filters *.lua *.props` (exclude `output librime plum include/wtl lib lib64 .git`).
- A symbol is dead only with ZERO references beyond its own decl/def. Watch the traps: **COM
  interface methods** (invoked via vtables — check the base interface and `WeaselTSF.def` exports),
  factory/registration code reached indirectly, function pointers, and build globs.
- Be conservative: <95% sure → leave it and note it. Removing a truly-unreferenced symbol can't
  change behavior, but a wrong "dead" call breaks an un-buildable module.
- Do NOT delete code that looks like an **unwired feature** (written and correct but never called —
  e.g. `DirectWriteResources::SetDpi`) during a mechanical pass; it may be a latent "should be
  called" bug. Flag it for separate investigation instead.

### Build files

- `xmake.lua` **globs `./*.cpp`** → adding/deleting a `.cpp` needs NO xmake edit; it never lists
  headers.
- `.vcxproj` / `.vcxproj.filters` list every source AND header **explicitly** → when you add/delete a
  file you MUST update both (the `<ClCompile>`/`<ClInclude>` entry and its filter block) or the MSVC
  build breaks. `weasel.sln` references projects, not files.

### OpenSpec

- One roadmap item = one change = one commit. `proposal.md` + `tasks.md` are the minimum; add
  `design.md` only for cross-cutting/ambiguous work; add a `specs/` delta only when a
  **requirement/behavior** actually changes.
- Archiving: a requirement/behavior change gets a real spec delta (`openspec archive <name>`); a
  pure mechanical refactor / dead-code change with no requirement change uses
  `openspec archive <name> --skip-specs --no-validate -y` (the spec-driven schema demands a delta —
  this is the sanctioned escape for spec-less changes, so `openspec/specs/` stays meaningful).
- Every `proposal.md` states what must stay byte-identical and has a **Non-goals** section; every
  `tasks.md` ends with an explicit review + (if the portable layer is touched) harness-run step.

### Scope & safety

- Keep changes small and single-purpose. When a mechanical change surfaces a **behavior fix** that
  needs more verification than close reading gives, SPLIT it into the right bug-fix change: record
  the bug (agent memory / roadmap) and fix it there, TDD via the harness where possible. (Bugs found
  so far route to `fix-ipc-parser-field-bugs`.)
- Sequence risky/structural work AFTER the safe foundations it depends on (see the wave roadmap).

### Commits & review

- **A separate adversarial review agent must pass on the diff before EVERY commit.** It re-proves
  deadness independently, checks line-ending/scope integrity, and (portable layer) confirms
  `test/host/run.sh` is green.
- Conventional-commit subjects matching history: `refactor(Module):`, `fix(Module):`,
  `perf(Module):`, `chore:`, `build(xmake):`, `ci:`, `test(host):`. Commit directly on `master`.
- The commit body should name the verification method used (harness / reference-proof / review),
  since there is no compiler here.
- **After committing, verify the commit's file list** (`git show --stat`). Stage files explicitly,
  not with a blanket `git add .`. Gotcha: a `git add` whose argument list names a path you already
  `git rm`'d fails fatally and stages *nothing*, so a pre-staged deletion can land in a commit by
  itself while your real edits are left behind — amend to fix.

## Module map

`WeaselTSF` (TSF DLL, IME core, biggest) · `WeaselUI` (candidate window, GDI/GDI+/D2D) ·
`WeaselServer` (server + tray, hosts Rime) · `RimeWithWeasel` (librime↔Weasel bridge) ·
`WeaselIPC` / `WeaselIPCServer` (shared-mem + pipe IPC) · `WeaselDeployer` (deploy/config) ·
`WeaselSetup` (TSF registration/installer, x86). Shared data model: `include/WeaselIPCData.h`.
