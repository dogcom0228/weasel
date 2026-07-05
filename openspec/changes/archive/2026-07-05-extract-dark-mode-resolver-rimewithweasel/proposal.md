## Why

`Initialize()` and `UpdateColorTheme()` contained a byte-for-byte identical block that sets the
dark-mode flag and, when dark, reads `style/color_scheme_dark` into a 255-char buffer and applies it
via `_UpdateUIStyleColor`. The only difference was the dark source (`IsUserDarkMode()` vs the
`darkMode` argument). Extract it once.

## What Changes

- Add a private method `_UpdateDarkModeColorScheme(RimeConfig* config, bool dark)` containing the
  shared block (`m_current_dark_mode = dark; if (m_current_dark_mode) { read color_scheme_dark ->
  _UpdateUIStyleColor }`).
- Replace the two duplicated blocks with `_UpdateDarkModeColorScheme(&config, IsUserDarkMode())` and
  `_UpdateDarkModeColorScheme(&config, darkMode)` respectively (the following
  `m_base_style = m_ui->style();` stays at each call site, unchanged).

## Capabilities

### New Capabilities
- (none)

### Modified Capabilities
- (none — behavior-identical dedup)

## Impact

- `include/RimeWithWeasel.h` (+1 decl) and `RimeWithWeasel/RimeWithWeasel.cpp` (two blocks → one
  helper). Both call sites are inside `if (m_ui)`, so the helper's `m_ui->style()` is reached exactly
  as before. `UpdateColorTheme`'s original `if (darkMode)` becomes `if (m_current_dark_mode)` after
  `m_current_dark_mode = darkMode` — identical, since the two are equal at that point. Not
  host-testable; verified by close reading + review. CRLF preserved.

## Non-goals

- The third, structurally-different dark-scheme path — the `update_color_scheme` lambda in
  `_LoadSchemaSpecificSettings` (per-schema key with a `weasel.yaml` fallback) — is NOT merged; it is
  a different operation and folding it would change behavior.
