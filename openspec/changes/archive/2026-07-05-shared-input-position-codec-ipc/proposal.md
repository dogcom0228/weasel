## Why

The 32-bit "input position" IPC word (WEASEL_IPC_UPDATE_INPUT_POS) was packed in the WeaselIPC
client and unpacked in the WeaselIPCServer with the exact bit layout hand-duplicated in both places
(shifts 31/24/12, masks 0x7f/0xfff/0x7ff/0x800, sign-extension trick). Duplicated bit-twiddling with
no test is exactly the kind of code that silently drifts. Extracting one shared, portable codec makes
the layout a single source of truth AND — crucially, given we can't compile the Windows build here —
makes it **host-testable**: an equivalence test locks the new codec to the original arithmetic.

## What Changes

- Add `include/InputPositionCodec.h` — portable (`<algorithm>`/`<cstdint>`, no `<windows.h>`)
  `EncodeInputPosition`/`DecodeInputPosition` over a small `InputPosition{left,top,height}` struct,
  documenting the packed layout once.
- `WeaselIPC/WeaselClientImpl.cpp`: replace the inline encode in `UpdateInputPosition` with
  `EncodeInputPosition({rc.left, rc.top, rc.bottom - rc.top})`.
- `WeaselIPCServer/WeaselServerImpl.cpp`: replace the inline decode in `OnUpdateInputPosition` with
  `DecodeInputPosition(wParam)` + the same rect reconstruction (`right = left + 6`,
  `bottom = top + height`). The DPI transform is unchanged.
- Add `test/host/input_position_test.cpp` — copies the original encode/decode arithmetic verbatim as
  a reference and asserts the codec matches it, bit-for-bit, across a sweep of ~2700 inputs plus a
  word sweep and edge anchors. Wire it into `test/host/run.sh` (now runs two suites).

The codec also removes two latent undefined behaviors present in the original (signed `1 << 31` and
left-shift of a negative value) by doing the packing in the unsigned domain — producing an identical
bit pattern on every real target.

## Capabilities

### New Capabilities
- (none)

### Modified Capabilities
- `host-test-harness`: now also guards the input-position codec (an equivalence test proving the
  extracted codec reproduces the original client/server bit arithmetic exactly).

## Impact

- New `include/InputPositionCodec.h` (found via the existing `$(SolutionDir)\include` include path —
  no `.vcxproj` edit needed, consistent with `WeaselIPCData.h`). Edits to the client and server call
  sites (net simplification). New host test + `run.sh` update. `.gitignore` for the new test binary.
- **The IPC wire word is byte-identical** (proven by the equivalence test); no protocol change.
- Mixed CRLF/LF preserved.

## Non-goals

- No change to the IPC message set, the DPI transform, or the fixed reconstruction width (6px).
- Not converting other IPC bit-packing (there is none comparable); this is scoped to the input
  position.
