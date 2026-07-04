## Why

The IPC client/server modules carry small leftover fragments — commented-out alternative
implementations, a misspelled unused forward declaration, and two unreferenced members — that add
noise. Removing them is provably safe (reference-search proof) and part of the module's cleanup
before later structural work. Two of the touched files are covered by the Linux host-test harness, so
those removals are additionally compile-verified.

## What Changes

- `WeaselIPC/Styler.h`: remove the bogus `namespace weasel { class Deserializr; }` forward
  declaration — `Deserializr` (misspelled; the real base is `weasel::Deserializer`) is referenced
  nowhere.
- `WeaselIPC/Deserializer.cpp`: remove two commented-out `insert(make_pair(...))` alternatives that
  sit beside the live `map::operator[]` assignments.
- `WeaselIPC/WeaselClientImpl.h`: remove the unused `ClientImpl::_Connected()` accessor (zero call
  sites; `_Active()` is the one actually used).
- `WeaselIPCServer/WeaselServerImpl.cpp`: remove `PipeServer::GetServerRunner` (declaration +
  definition) and the now-unused `ServerRunner` alias — no call sites.
- `WeaselIPCServer/WeaselServerImpl.h`: remove a leftover commented-out `// class ServerImpl`
  alternative class declaration.

## Capabilities

### New Capabilities
- (none)

### Modified Capabilities
- (none — pure removal of unreferenced code/comments; behavior unchanged)

## Impact

- Files: `WeaselIPC/Styler.h`, `WeaselIPC/Deserializer.cpp`, `WeaselIPC/WeaselClientImpl.h`,
  `WeaselIPCServer/WeaselServerImpl.cpp`, `WeaselIPCServer/WeaselServerImpl.h`. 16 lines removed, no
  additions. `xmake.lua` globs `*.cpp`, so no build-file edits needed (no whole-file changes).
- `Styler.h` and `Deserializer.cpp` are exercised by `test/host/` — the harness stays green,
  compile-verifying those edits. The others are verified by reference-search proof + review.

## Non-goals

- Two real bugs found nearby are deferred to the `fix-ipc-parser-field-bugs` change (both
  harness-testable via TDD): `ContextUpdater::_StoreText`'s `vec[2]` guard and
  `Configurator::Store`'s wrong-pointer null check. This change removes dead code only.
- The misspelled-but-live `ServerImpl::_Finailize` is left alone (a rename is cross-site churn, not
  dead code).
