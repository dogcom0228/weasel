## Why

`style/label_format` is **user configuration** (default `L"%s."`, `include/WeaselIPCData.h:316`).
It is fed directly as a `printf`-family format string to `swprintf_s` at two sites, with the
candidate label as the lone argument:

- server: `RimeWithWeasel/RimeWithWeasel.cpp:905` (the `PREVIEW_ALL` inline-preview body);
- client: `WeaselUI/StandardLayout.cpp:10` (`GetLabelText`, the single chokepoint every layout —
  Horizontal/Vertical/VHorizontal/WeaselPanel — routes candidate labels through).

A stray conversion in the user's config — `%d`, `%x`, or the classic `%n` — makes the CRT read a
non-existent argument and invoke the **invalid-parameter handler, terminating the process**. Because
the client site runs in-process under TSF, a bad `label_format` crashes **the host application the
user is typing into**; the server site crashes WeaselServer. `swprintf_s<128>` also invokes the same
handler if the formatted result exceeds 127 wide chars. An input method must never let its own
config file crash the apps it serves.

## What Changes

- Add a safe, portable `format_label(format, label)` to `include/StringAlgorithm.hpp`: it replaces
  the **first** `%s` with `label`, treats `%%` as a literal `%`, and emits every other character —
  including a stray `%d`/`%n` or a trailing `%` — **verbatim**, never passing user text to a
  `printf` family function. Output is a `std::wstring` (no 127-char truncation).
- `StandardLayout::GetLabelText`: `return format_label(format, labels.at(id).str);` (add
  `#include <StringAlgorithm.hpp>`).
- `RimeWithWeasel.cpp` `PREVIEW_ALL`: `label_w = format_label(session_status.style.label_text_format,
  cinfo.labels.at(i).str);` (already includes StringAlgorithm.hpp).
- TDD: new `test/host/format_label_test.cpp` (pure-portable, no Boost) + a third suite in
  `test/host/run.sh`, mirroring the existing `input_position_test` suite.

## Capabilities

### New Capabilities
- (none — a hardening/bug fix.)

### Modified Capabilities
- (none for realistic formats — for any `%s`-plus-literal format (incl. the default `%s.`),
  `format_label` produces the byte-identical label text `swprintf_s` did, so the candidate window
  and the `PREVIEW_ALL` IPC body are unchanged. Behavior differs on two classes of atypical config:
  (a) formats that previously **crashed** (stray `%d`/`%n`/`%*s`/`%S`) now render literally; and
  (b) valid printf **width/precision/flag** specifiers (`%5s`, `%-s`, `%.3s`) that previously
  formatted the label now also render literally — these buy nothing for single-glyph candidate
  labels and are not part of Rime's documented `label_format`, so this is an accepted trade for
  never crashing the host. Over-long output is also no longer truncated at 127 wchars.)

## Impact

- `include/StringAlgorithm.hpp` (+ helper, LF no BOM), `WeaselUI/StandardLayout.cpp` (CRLF no BOM,
  + include + one-line body), `RimeWithWeasel/RimeWithWeasel.cpp` (CRLF + UTF-8 BOM, replace the
  swprintf block), `test/host/format_label_test.cpp` (new, LF), `test/host/run.sh` (+ suite, LF).
  StringAlgorithm.hpp is portable → `test/host/run.sh` (via WSL) is mandatory; the two .cpp sites
  are covered by the x64 compile-check. No vcxproj change (header already built; test/host is
  Linux-only).

## Non-goals

- `mark_text` and other style strings are plain text (not format strings) — untouched.
- The other layouts are not modified; they already delegate to `StandardLayout::GetLabelText`.
- Not changing the meaning of a valid `%s` format or the default value.
