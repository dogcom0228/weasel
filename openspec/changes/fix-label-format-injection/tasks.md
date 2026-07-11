## 1. Safe helper + TDD (portable layer — harness mandatory)

- [x] 1.1 Add `inline std::wstring format_label(const std::wstring& format, const std::wstring&
      label)` to `include/StringAlgorithm.hpp` (first `%s` → label; `%%` → `%`; everything else
      verbatim; no printf).
- [x] 1.2 Add `test/host/format_label_test.cpp` covering: default `%s.`; no-`%s`; `%%`; first-`%s`-
      only; stray `%d`/`%n` rendered literally; trailing `%`; empty format/label; unicode label.
- [x] 1.3 Add a third suite to `test/host/run.sh` (copy the `input_position_test` block: `-I include`,
      no Boost).
- [x] 1.4 Run the harness via WSL — all three suites green.

## 2. Swap the two call sites (x64 compile-check)

- [x] 2.1 `StandardLayout::GetLabelText` → `format_label(...)`; add `#include <StringAlgorithm.hpp>`.
- [x] 2.2 `RimeWithWeasel.cpp` `PREVIEW_ALL` swprintf block → `format_label(...)`.
- [x] 2.3 x64 compile-check green.

## 3. Verify, review, commit

- [x] 3.1 Confirm valid formats (default `%s.`, any `%s`+literal) yield byte-identical output to the
      old swprintf; the two sites now unreachable by any printf crash; each file's EOL/BOM preserved.
- [x] 3.2 Adversarial review: enumerate format edge cases vs the old swprintf semantics (wide `%s`,
      `%%`, `%d`/`%n`, `%S`, trailing `%`, >127 chars) and confirm the change is safe and byte-
      identical for all realistic (valid) inputs; confirm StandardLayout is the sole client chokepoint
      and the two are the only user-format-as-printf sites (grep label_text_format). Address blockers.
- [x] 3.3 Commit on `master` (`fix(WeaselUI):` or `fix:` — spans UI+bridge; use `fix(WeaselUI):`
      with body noting the server site) + Co-Authored-By; archive manually.
