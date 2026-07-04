## Why

`WeaselPanel.cpp` and the `DirectWriteResources` helper carry a few unreferenced fragments and two
small defects (a garbled debug string and dead local variables). Cleaning them up reduces noise
before later WeaselUI layout/rendering refactors.

## What Changes

- `WeaselUI/WeaselPanel.cpp`: remove the `#if 0` debug-draw block in `_TextOut` (never compiled).
- `include/WeaselUI.h`: remove the unused `DirectWriteResources::ResetLayout()` inline (zero callers).
- `WeaselUI/WeaselPanel.cpp`: fix the garbled debug string `"OpenClipord ailed"` →
  `"OpenClipboard failed"` in `_CaptureRect`.
- `WeaselUI/WeaselPanel.cpp`: remove three dead local variables (`before_str`, `hilited_str`,
  `after_str`) in `_DrawPreedit` — computed but never read (each branch recomputes its own
  `str_before`/`str_highlight`/`str_after`).

## Capabilities

### New Capabilities
- (none)

### Modified Capabilities
- (none — dead-code/defect cleanup with no user-visible behavior change; the debug-string fix only
  affects a log message)

## Impact

- Files: `WeaselUI/WeaselPanel.cpp`, `include/WeaselUI.h`. 9 lines removed, 1 changed. Mixed CRLF/LF
  preserved. Not part of the portable harness; verified by reference-search proof + adversarial
  review (no Windows compiler here).

## Non-goals

- `DirectWriteResources::SetDpi` (also zero callers) is deliberately NOT removed: it is DPI-scaling
  logic that looks like an unwired feature, not incidental dead code. Erasing it in a mechanical pass
  could hide a "should be called" bug relevant to DPI/position issues; flagged for separate review.
- Local-identifier typos (`max_comment_heihgt`, `max_height_curren_candidate`) and comment typos
  (`truely`) are left for opportunistic fixes — multi-occurrence renames with no functional benefit.
- The `DrawRect` method referenced only by the removed `#if 0` block is left in place (a separate
  judgment call, not incidental to this cleanup).
