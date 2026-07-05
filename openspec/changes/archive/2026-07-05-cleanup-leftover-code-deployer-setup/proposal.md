## Why

WeaselDeployer and WeaselSetup carry small leftovers (duplicate includes, a commented-out block, a
comment-only stub header) and one real memory bug (a mismatched array delete). Cleaning these up and
fixing the bug is low-risk and reduces noise.

## What Changes

- `WeaselDeployer/SwitcherSettingsDialog.cpp` (**bug fix**): `selection` is allocated with
  `new const char*[...]` but freed with scalar `delete` on both paths of `OnOK` — undefined
  behavior. Change both to `delete[] selection;`. (The array holds non-owned `const char*` from
  librime, so `delete[]` frees only the array, not the strings — no double free.)
- `WeaselDeployer/Configurator.cpp`: remove a duplicate `#include "WeaselDeployer.h"` (kept once).
- `WeaselDeployer/WeaselDeployer.h`: remove a duplicate `#include "resource.h"` (kept once).
- `WeaselSetup/imesetup.cpp`: remove a commented-out `Wow64…FsRedirection` `GetProcAddress` block
  (dead comment beside the live `Wow64DisableWow64FsRedirection` call).
- Delete `WeaselSetup/WeaselSetup.h` — a comment-only stub (`// WeaselSetup.h`) that is `#include`d
  nowhere and is not a `<ClInclude>` in the project (only an inert MIDL `<HeaderFileName>` default).

## Capabilities

### New Capabilities
- (none)

### Modified Capabilities
- (none — dead-code/duplicate cleanup; the `delete[]` fix corrects UB with no functional change on
  conforming platforms)

## Impact

- Files: `WeaselDeployer/SwitcherSettingsDialog.cpp`, `WeaselDeployer/Configurator.cpp`,
  `WeaselDeployer/WeaselDeployer.h`, `WeaselSetup/imesetup.cpp`; deleted `WeaselSetup/WeaselSetup.h`.
  `xmake.lua` globs `*.cpp` and lists no headers, so deleting the stub header needs no build edit.
- Not part of the portable harness; verified by reference proof + close reading (the `new[]`/
  `delete[]` pairing) + adversarial review. Mixed CRLF/LF preserved.

## Non-goals

- The unused `resource.h` macros (e.g. `IDI_WEASELSETUP`, `IDC_CHECK1`, `IDD_WEASELDEPLOYER_DIALOG`)
  are NOT removed: resource IDs are `.rc`-coupled and low-value; touching them risks the (uncompilable
  here) resource build for little benefit. Left for a dedicated resource-cleanup pass if ever wanted.
- Commented-out `MessageBox` blocks and the misspelled-but-live identifiers are left alone.
