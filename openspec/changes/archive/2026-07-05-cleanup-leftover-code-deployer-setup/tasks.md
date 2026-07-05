## 1. Fix + clean up (byte-preserving; files are CRLF)

- [x] 1.1 `SwitcherSettingsDialog.cpp`: both `delete selection;` → `delete[] selection;` (matches
      `new const char*[...]`).
- [x] 1.2 `Configurator.cpp`: remove the duplicate `#include "WeaselDeployer.h"` (line 18).
- [x] 1.3 `WeaselDeployer.h`: remove the duplicate `#include "resource.h"` (line 7).
- [x] 1.4 `WeaselSetup/imesetup.cpp`: remove the commented-out `Wow64…FsRedirection` `GetProcAddress`
      block.
- [x] 1.5 `git rm WeaselSetup/WeaselSetup.h` (comment-only stub; no build-file edit needed).

## 2. Verify, review, commit

- [x] 2.1 Confirm `delete selection;` (scalar) is gone and the kept includes remain; `git diff --stat`
      small; CRLF preserved; `WeaselSetup.h` unreferenced.
- [x] 2.2 Run `test/host/run.sh` (invariant sanity — unrelated portable layer stays green).
- [x] 2.3 Adversarial review agent on the diff (delete[]/new[] pairing incl. no double-free of the
      non-owned schema_id strings; deadness of removed includes/stub; line endings intact). Address
      blocking findings.
- [x] 2.4 Commit on `master` (`fix(WeaselDeployer):` — carries a real bug fix) + Co-Authored-By; then
      `openspec archive cleanup-leftover-code-deployer-setup --skip-specs`.
