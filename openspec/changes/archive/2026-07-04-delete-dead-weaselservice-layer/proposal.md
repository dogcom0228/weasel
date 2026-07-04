## Why

`WeaselServer.exe` runs as an ordinary user process (`_tWinMain` → `WeaselServerApp`). It carries a
complete but entirely unreachable Windows **Service Control Manager** layer (`WeaselService`) that is
never instantiated and whose `Run/Start/Stop/ServiceMain/ServiceCtrlHandler` are never called. Its
only linkage is a stray `#include` in `WeaselServer.cpp`. Deleting this ~200-line dead subsystem is a
clean, high-leverage simplification.

## What Changes

- Delete `WeaselServer/WeaselService.cpp` and `WeaselServer/WeaselService.h` (the whole SCM layer,
  including the SCM-only `WEASEL_SERVICE_NAME` macro).
- Delete the orphaned stub `WeaselServer/WeaselServer.h` (its entire body is the comment
  `// WeaselServer.h`; not `#include`d anywhere and absent from all build files).
- In `WeaselServer.cpp`, **replace** `#include "WeaselService.h"` with `#include "WeaselServerApp.h"`.
  WeaselServer.cpp uses `WeaselServerApp` but currently reaches its declaration only *transitively*
  through `WeaselService.h → WeaselServerApp.h`; a plain deletion would break the build, so the
  include is redirected to the header it actually needs.
- Remove the `WeaselService.cpp`/`WeaselService.h` entries from `WeaselServer.vcxproj` and
  `WeaselServer.vcxproj.filters` (the MSVC build lists files explicitly). No `xmake.lua` edit is
  needed — it globs `./*.cpp`, and never lists headers.

## Capabilities

### New Capabilities
- (none)

### Modified Capabilities
- (none — WeaselServer.exe's behavior is unchanged; the deleted service mode was never reachable)

## Impact

- Files removed: `WeaselServer/WeaselService.cpp`, `WeaselServer/WeaselService.h`,
  `WeaselServer/WeaselServer.h`. Files edited: `WeaselServer/WeaselServer.cpp` (1-line include swap),
  `WeaselServer/WeaselServer.vcxproj`, `WeaselServer/WeaselServer.vcxproj.filters`.
- No runtime behavior change. Not part of the portable harness; verified by reference-search proof +
  adversarial review (no Windows compiler here). The include swap is the one build-affecting edit and
  is called out for careful review.

## Non-goals

- No changes to the live `WeaselServerApp` / `weasel::Server` code (the `Run/Start/Stop` that DO run
  belong to those, not to `WeaselService` — the name collision must not sweep them in).
- The vendored `SystemTraySDK` (CSystemTray) trimming is a separate later change.
