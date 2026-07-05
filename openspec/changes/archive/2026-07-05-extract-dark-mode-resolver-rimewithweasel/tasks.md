## 1. Extract the shared dark-mode block (byte-preserving; files are CRLF)

- [x] 1.1 Declare `_UpdateDarkModeColorScheme(RimeConfig* config, bool dark)` in RimeWithWeasel.h and
      define it in RimeWithWeasel.cpp with the shared block verbatim.
- [x] 1.2 Replace the duplicated blocks in `Initialize` and `UpdateColorTheme` with calls; keep the
      trailing `m_base_style = m_ui->style();` at each site.

## 2. Verify, review, commit

- [x] 2.1 Confirm only one `config_get_string("style/color_scheme_dark")` remains (the helper); CRLF
      preserved; harness green.
- [x] 2.2 Adversarial review: confirm the helper body is identical to both original blocks, that
      `if (m_current_dark_mode)` == the original `if (darkMode)`/`if (m_current_dark_mode)` after the
      assignment, and that the helper is only reached under `if (m_ui)`. Confirm the
      `_LoadSchemaSpecificSettings` variant was not touched. Address blocking findings.
- [x] 2.3 Commit on `master` (`refactor(RimeWithWeasel):`) + Co-Authored-By; then
      `openspec archive extract-dark-mode-resolver-rimewithweasel --skip-specs`.
