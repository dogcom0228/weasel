## Why

`StandardLayout::GetPreeditSize` measures the three segments of the pre-edit string (before /
highlighted / after the highlighted range) to size the pre-edit area of the candidate window. The
highlighted segment is extracted with:

```cpp
std::wstring hilited_str = preedit.substr(_range.start, _range.end);
```

`std::wstring::substr(pos, count)` takes a **count** as its second argument, not an end index — so
this passes `_range.end` as the number of characters, measuring from `_range.start` for up to
`_range.end` characters (clamped at end-of-string) instead of just the `_range.start .. _range.end`
segment. Whenever `_range.start > 0` the highlighted measurement over-counts (it reads into the
"after" region), so the pre-edit highlight box comes out **wider/taller than the text actually
drawn**. The neighbouring lines already use the correct forms: `substr(0, _range.start)` for the
"before" part and `substr(_range.end)` for the "after" part.

## What Changes

- `StandardLayout.cpp`, `GetPreeditSize`: change the highlighted-segment extraction to
  `preedit.substr(_range.start, _range.end - _range.start)`.

The enclosing `if (_range.start < _range.end)` guarantees the count is positive.

## Capabilities

### New Capabilities
- (none)

### Modified Capabilities
- (none — a measurement bug fix; the highlighted pre-edit segment is now measured over exactly the
  highlighted character range, matching the paint code, so the highlight box no longer over-extends.)

## Impact

- `WeaselUI/StandardLayout.cpp` only (one line). Not host-testable (GDI/DirectWrite); verified by
  close reading of the `std::wstring::substr(pos, count)` contract against the sibling `substr`
  calls and adversarial review. CRLF line endings, no BOM — preserved.

## Non-goals

- The other WeaselUI layout findings (the DoLayout block duplication across the four layouts, the
  triple text-layout creation per string, the redundant `Xor` region op) are separate changes.
- No change to `_range` selection or to the paint-side rendering; only the highlighted-segment
  substring passed to `GetTextSizeDW`.
