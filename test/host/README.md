# Host test harness (`test/host/`)

A Linux-buildable regression oracle for the **portable** slice of Weasel — the part that does not
need Windows and can actually run in CI or a dev container.

## Why it exists

Weasel is a Windows TSF input method built with MSVC/ATL/WTL, so most of it can only be compiled on
Windows. But the client-side **IPC text-protocol parser** (`WeaselIPC/ResponseParser.cpp` and its
deserializers), the shared **data model** (`include/WeaselIPCData.h`), and the **boost
serialization** of candidates/style are ordinary portable C++. This harness compiles those *real*
sources against a tiny `windows.h` shim and runs golden tests, giving the refactor a real safety net
for every change that touches the IPC/data layer and guarding the wire format against drift.

## Scope (and limits)

- ✅ Covers: the text protocol (`action`, `commit`, `ctx.preedit`/`cursor`, `status.*`), and the
  `ctx.cand=<boost archive>` candidate round-trip.
- ❌ Does NOT cover: TSF, UI rendering, IPC transport (shared memory / pipes / window messages),
  the server, or anything needing the real Win32 API. Those are verified by review + protocol
  byte-equivalence, not here.

The `shim/` directory is test-only scaffolding: a minimal `windows.h` (typedefs, no-op
`MessageBoxA`, `_wtoi`), empty MSVC platform headers, and a reduced `WeaselUtility.h` exposing only
the portable string helpers the parser uses. It is never part of any shipped or Windows build.

## Running

```sh
# one-time toolchain bootstrap (g++ + Boost, no root needed):
mamba create -n wtest gxx_linux-64 libboost-devel

# then:
test/host/run.sh
```

`run.sh` activates the conda env (override with `WEASEL_TEST_ENV=<name>`), compiles the golden test
plus the seven parser translation units, runs the binary, and exits non-zero with the count of
failing assertions. All pass ⇒ exit 0 / `[host-test] PASS`.
