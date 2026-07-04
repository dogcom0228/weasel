# CLAUDE.md — Weasel (小狼毫) refactor

Weasel is a **Windows TSF input method** (fork of `rime/weasel`). First-party C++ (~19k LOC)
bridges the upstream **librime** engine to a Windows front-end. This repo is in a large,
ongoing **refactor** whose rules are below.

## Mission (do not break these)

1. **Simplify / minimize** the first-party C++ and apply better design patterns — but
2. **preserve all existing behavior** (this is not a feature effort), and
3. **fix bugs** only where realistically fixable in this repo (not upstream librime).

Behavior preservation is paramount. Prefer small, mechanical, reviewable edits.

## ⚠️ Hard constraint: this box cannot compile the Windows code

Weasel builds only on **Windows + MSVC** (ATL, WTL, TSF, Boost; `weasel.sln` / `xmake.lua`).
The dev box is **Linux/WSL with no MSVC and no Windows SDK** — you **cannot build or run**
WeaselTSF/WeaselUI/WeaselServer/etc. here. Every change is "blind" to the Windows compiler.

Because of this, the safety net is **review + the portable test harness**, not compilation.

## Verification

- **Portable layer (IPC text-protocol parser + `include/WeaselIPCData.h` data model + boost
  serialization) IS testable on Linux** via `test/host/` — a `windows.h` shim + real parser
  `.cpp`s + golden round-trip tests. Run it for ANY change touching that layer:
  ```
  test/host/run.sh        # builds with the mamba env `wtest` (gxx 15 + boost) and runs tests
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

## Conventions

- Style: **Chromium** `clang-format`, `SortIncludes: false`. Match surrounding code exactly.
- Commit prefixes match history: `refactor(Module):`, `fix(Module):`, `perf(Module):`,
  `chore:`, `build(xmake):`, `ci:`.
- Do **not** touch `include/wtl/`, `librime/`, `plum/`, `lib*/`, `output/` (third-party / build output).

## Module map

`WeaselTSF` (TSF DLL, IME core, biggest) · `WeaselUI` (candidate window, GDI/GDI+/D2D) ·
`WeaselServer` (server + tray, hosts Rime) · `RimeWithWeasel` (librime↔Weasel bridge) ·
`WeaselIPC` / `WeaselIPCServer` (shared-mem + pipe IPC) · `WeaselDeployer` (deploy/config) ·
`WeaselSetup` (TSF registration/installer, x86). Shared data model: `include/WeaselIPCData.h`.
