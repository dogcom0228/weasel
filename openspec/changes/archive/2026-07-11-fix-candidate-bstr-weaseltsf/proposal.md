## Why

`CCandidateList` implements `ITfCandidateListUIElement`, whose string accessors return `BSTR`
out-parameters that the TSF consumer (UI-less hosts — games, accessibility tools, some Office
paths) **owns and frees** per COM convention. Two of them hand out malformed BSTRs:

1. `GetString` (CandidateList.cpp) builds the BSTR with
   `SysAllocStringLen(str.c_str(), str.size() + 1)`. `SysAllocStringLen(psz, len)` copies `len`
   characters **and appends its own NUL**, so passing `size() + 1` copies the string's own
   terminating NUL into the payload and reports a length one too long — every candidate string
   handed to a TSF consumer carries a spurious trailing U+0000 and an off-by-one `SysStringLen`.
2. `GetDescription` returns a function-local `static` BSTR
   (`static auto str = SysAllocString(L"Candidate List");`). A conforming caller `SysFreeString`s
   the returned BSTR, so the shared static is freed after the first call and every subsequent call
   returns a dangling (freed) BSTR — a use-after-free / double-free across callers, and a one-time
   leak of the never-owned static.

## What Changes

- `GetString`: drop the `+ 1` — `SysAllocStringLen(str.c_str(), static_cast<UINT>(str.size()))`.
  The NUL is still appended by `SysAllocStringLen`, so the returned BSTR is well-formed and its
  reported length matches the candidate text.
- `GetDescription`: allocate a fresh BSTR on every call —
  `*pbstr = SysAllocString(L"Candidate List");` — so each caller owns exactly the BSTR it frees.
  The `if (pbstr)` null guard is kept.

## Capabilities

### New Capabilities
- (none)

### Modified Capabilities
- (none — memory-safety bug fixes to the ITfCandidateListUIElement string accessors; the candidate
  text and the description string returned are unchanged in content.)

## Impact

- `WeaselTSF/CandidateList.cpp` only (two methods, ~3 lines). Not host-testable (TSF/COM);
  verified by close reading against the documented `SysAllocStringLen`/`SysAllocString` +
  BSTR-ownership contracts and adversarial review. LF line endings, no BOM — preserved.

## Non-goals

- The other `CCandidateList` / TSF findings (uninitialized `uiid`, the WeaselTSF↔CCandidateList
  ownership cycle, the DWORD `_cRef` refcount, the shared IUnknown base) are separate later changes.
- No change to how candidate strings are produced or escaped; only how the BSTR wrapper is sized
  and owned.
