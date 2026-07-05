## 1. Extract + de-duplicate (byte-preserving; files are CRLF, new header LF)

- [x] 1.1 Add `WeaselUI/UIColor.h` with the 4 helper macros copied verbatim.
- [x] 1.2 `WeaselPanel.cpp`: remove the 4 moved macros, `#include "UIColor.h"`, keep `TRANS_COLOR`.
- [x] 1.3 `HorizontalLayout.cpp` / `VerticalLayout.cpp` / `VHorizontalLayout.cpp`: add the include and
      replace the 24 raw boolean alpha tests with `COLORNOTTRANSPARENT`.
- [x] 1.4 Add `UIColor.h` to `WeaselUI.vcxproj` + `.vcxproj.filters`.

## 2. Verify, review, commit

- [x] 2.1 Confirm 0 residual raw `& 0xff000000` in the layouts; `TRANS_COLOR` preserved; WeaselPanel's
      existing 23 macro uses still resolve (from UIColor.h); CRLF preserved; harness still green.
- [x] 2.2 Adversarial review agent: confirm every replaced site is a boolean context (identical
      behavior), macro bodies are byte-identical, TRANS_COLOR untouched, no value-use site was
      changed, line endings intact. Address blocking findings.
- [x] 2.3 Commit on `master` (`refactor(WeaselUI):`) + Co-Authored-By; then
      `openspec archive shared-color-alpha-helpers-weaselui --skip-specs`.
