## Why

`WeaselTrayIcon::Refresh` runs on every UI update — i.e. every keystroke, via
`RimeWithWeaselHandler::_UpdateUI` → `_RefreshTrayIcon` → the tray callback. Its "should I re-set
the tray icon?" condition includes two "for initialize" clauses:

```cpp
if (mode != m_mode || m_schema_zhung_icon != m_style.current_zhung_icon ||
    (m_schema_zhung_icon.empty() && m_style.current_zhung_icon.empty()) ||   // (3)
    m_schema_ascii_icon != m_style.current_ascii_icon ||
    (m_schema_ascii_icon.empty() && m_style.current_ascii_icon.empty())) {   // (5)
```

For a schema with **no custom tray icons** — the default for `luna_pinyin` and most pinyin users —
`current_zhung_icon`/`current_ascii_icon` are empty, and after the first refresh the tracked
`m_schema_*_icon` are synced to those empty strings, so clauses (3) and (5) are `(empty && empty)`
== **permanently true**. The whole condition is therefore true on every keystroke, so `ShowIcon()` +
`LoadImage`/`SetIcon` + `Shell_NotifyIcon(NIM_MODIFY)` execute per keystroke instead of only when
the mode or icon actually changed — needless per-keystroke Win32 tray traffic on the server.

Crucially, the clauses are also **redundant for their stated purpose**: `m_mode` starts at `INITIAL`
(ctor) and `mode` is only ever `ZHUNG`/`ASCII`/`DISABLED`, so the genuine first-time initialization
is already caught by `mode != m_mode`; a later change back to empty icons is caught by the
`m_schema_*_icon != current_*_icon` comparison. Clauses (3)/(5) fire only in the unchanged steady
state — exactly when nothing should happen.

## What Changes

- Remove the two `(m_schema_*_icon.empty() && m_style.current_*_icon.empty())` clauses from the
  condition, leaving `mode != m_mode || m_schema_zhung_icon != m_style.current_zhung_icon ||
  m_schema_ascii_icon != m_style.current_ascii_icon`. Update the block comment to state that first
  init is covered by the `INITIAL` sentinel in `mode != m_mode`.

## Capabilities

### New Capabilities
- (none)

### Modified Capabilities
- (none intended beyond the fix — the tray icon is still set on first refresh (mode != INITIAL), on
  every 中/英 and enabled/disabled mode change, and whenever a schema's icon path changes; it is no
  longer re-set on keystrokes that change none of those. The `else if (!Visible()) ShowIcon();`
  visibility-recovery branch is unchanged.)

## Impact

- `WeaselServer/WeaselTrayIcon.cpp`, `Refresh` only (remove 2 clauses + comment). LF with UTF-8 BOM
  — preserved. Verified by x64 compile + close reading + adversarial review.

## Non-goals

- The `_RefreshTrayIcon` detached-thread data race (Wave C) and the WeaselTrayIcon/CSystemTray
  vendored-SDK trimming are separate changes.
