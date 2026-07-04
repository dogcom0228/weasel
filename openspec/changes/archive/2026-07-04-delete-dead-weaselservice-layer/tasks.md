## 1. Redirect the transitive include (do FIRST, before deleting the header)

- [x] 1.1 In `WeaselServer/WeaselServer.cpp`, replace `#include "WeaselService.h"` with
      `#include "WeaselServerApp.h"`.

## 2. Delete dead files

- [x] 2.1 `git rm WeaselServer/WeaselService.cpp WeaselServer/WeaselService.h`.
- [x] 2.2 `git rm WeaselServer/WeaselServer.h` (orphaned stub).

## 3. Update the MSVC build lists

- [x] 3.1 In `WeaselServer/WeaselServer.vcxproj`, remove `<ClCompile Include="WeaselService.cpp" />`
      and `<ClInclude Include="WeaselService.h" />`.
- [x] 3.2 In `WeaselServer/WeaselServer.vcxproj.filters`, remove the `WeaselService.cpp` ClCompile
      block and the `WeaselService.h` ClInclude block (3 lines each).

## 4. Verify, review, commit

- [x] 4.1 Re-grep the repo to confirm `WeaselService`, `WEASEL_SERVICE_NAME`, and
      `#include "WeaselServer.h"` have zero remaining references, and that `.Run/.Start/.Stop` call
      sites all bind to `WeaselServerApp`/`weasel::Server`, not the deleted class.
- [x] 4.2 Run `test/host/run.sh` (invariant sanity — unrelated portable layer must stay green).
- [x] 4.3 Adversarial review agent on the diff: deadness proof, the include-swap correctness (does
      WeaselServerApp.h transitively provide everything WeaselServer.cpp used via WeaselService.h?),
      and that no live `WeaselServerApp`/`Server` code was touched. Address blocking findings.
- [x] 4.4 Commit on `master` (`refactor(WeaselServer):`) + Co-Authored-By; then
      `openspec archive delete-dead-weaselservice-layer --skip-specs`.
