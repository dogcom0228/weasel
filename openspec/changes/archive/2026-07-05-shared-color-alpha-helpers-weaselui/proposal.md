## Why

WeaselUI's alpha/color helper macros (`COLORTRANSPARENT`, `COLORNOTTRANSPARENT`,
`GDPCOLOR_FROM_COLORREF`, `HALF_ALPHA_COLOR`) were defined privately at the top of
`WeaselPanel.cpp`, so the layout files could not use them and instead repeated the raw
`(color & 0xff000000)` alpha test 24 times. Sharing the helpers removes that duplication and gives
the alpha test a name.

## What Changes

- Add `WeaselUI/UIColor.h` and move the four helper macros into it **verbatim** (byte-identical
  expansions).
- `WeaselPanel.cpp`: drop the four now-moved macros and `#include "UIColor.h"`; keep the unrelated
  `TRANS_COLOR` define in place.
- `HorizontalLayout.cpp`, `VerticalLayout.cpp`, `VHorizontalLayout.cpp`: `#include "UIColor.h"` and
  replace the 24 raw `(color & 0xff000000)` **boolean** alpha tests with `COLORNOTTRANSPARENT(color)`
  (every occurrence is a boolean context — `if` / `?:` / `&&` operand / `bool` initializer — where
  `(x & 0xff000000)` is exactly `((x & 0xff000000) != 0)`).
- Add `UIColor.h` to `WeaselUI.vcxproj`/`.filters` (IDE metadata; matches how `Layout.h` is listed).

## Capabilities

### New Capabilities
- (none)

### Modified Capabilities
- (none — pure de-duplication; every macro expansion and every replaced test is behavior-identical)

## Impact

- New `WeaselUI/UIColor.h`; edits to `WeaselPanel.cpp` and the three layout `.cpp` files; project-file
  metadata. Not host-testable (GDI/GDI+); verified by close reading (each replaced site is a boolean
  test) + adversarial review. Mixed CRLF/LF preserved.

## Non-goals

- The hand-rolled channel splits in `WeaselPanel.cpp` (`GetRValue/…` + `>> 24` alpha at ~564-568 and
  the D2D float split at ~1298-1301) are NOT collapsed: their locals are reused downstream / target a
  different color type, so folding them into a helper would risk behavior. Deferred.
- Macros are shared as-is, not converted to inline functions (a larger, separate cleanup).
