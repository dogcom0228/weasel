# host-test-harness Specification

## Purpose
TBD - created by archiving change add-linux-test-harness. Update Purpose after archive.
## Requirements
### Requirement: Linux host build of the portable parser layer

The project SHALL provide a harness under `test/host/` that compiles the real IPC text-protocol
parser translation units (`WeaselIPC/ResponseParser.cpp`, `Deserializer.cpp`, `ActionLoader.cpp`,
`ContextUpdater.cpp`, `Committer.cpp`, `Configurator.cpp`, `Styler.cpp`) together with
`include/WeaselIPCData.h` on a Linux host that has no Windows SDK, using a minimal `windows.h` shim.
The harness MUST NOT require modifying any shipped source file.

#### Scenario: Harness builds on Linux without a Windows toolchain

- **WHEN** `test/host/run.sh` is executed in an environment with the documented toolchain (g++ ≥ 13,
  Boost with serialization) and no MSVC/Windows SDK
- **THEN** the real parser translation units and the golden test compile and link successfully

#### Scenario: Shim provides only what the parse path needs

- **WHEN** the parser sources are compiled against `test/host/shim/`
- **THEN** the shim supplies the Win32 typedefs, a no-op `MessageBoxA`, `_wtoi`, and the portable
  `unescape_string`/`EscapeChar` helpers, and no shipped header is edited to make the build succeed

### Requirement: Golden tests assert current IPC protocol behavior

The harness SHALL include golden tests that feed server-formatted responses through the parser and
assert the reconstructed data, covering at least: `action=noop`; `commit` with escape/unescape;
`ctx.preedit` with a three-field `cursor`; `status.*` flags; and a boost-archive round-trip of
`CandidateInfo` via the current `ctx.cand=<archive>` protocol. A failing assertion MUST make the
harness exit non-zero.

#### Scenario: All golden assertions pass on current code

- **WHEN** `test/host/run.sh` runs against the current parser sources
- **THEN** every golden assertion passes and the process exits with status 0

#### Scenario: A parser regression is caught

- **WHEN** the parser or data model is changed such that a covered protocol case no longer
  reconstructs the expected data
- **THEN** at least one golden assertion fails and the harness exits with a non-zero status equal to
  the number of failed assertions

### Requirement: Candidate boost-archive round-trip fidelity

The harness SHALL verify that a `CandidateInfo` serialized with boost `text_woarchive` (as the
server does in `RimeWithWeasel::_Respond`) is reconstructed field-for-field by the client parser,
guarding the boost-archive wire format against accidental change.

#### Scenario: CandidateInfo survives serialize → parse

- **WHEN** a `CandidateInfo` with candidates, labels, page, and highlight is boost-serialized into a
  `ctx.cand=<archive>` line and parsed
- **THEN** the parsed `CandidateInfo` equals the original in candidate strings, highlight index, and
  page counts

### Requirement: Input-position codec equivalence guard

The host harness SHALL verify that `include/InputPositionCodec.h`'s `EncodeInputPosition` and
`DecodeInputPosition` reproduce, bit-for-bit, the original hand-rolled WeaselIPC client encode and
WeaselIPCServer decode arithmetic, so the shared codec is a provably behavior-preserving replacement
for the duplicated logic. A mismatch MUST make the harness exit non-zero.

#### Scenario: Codec matches the original arithmetic across a sweep

- **WHEN** `test/host/run.sh` runs the input-position suite over its sampled left/top/height inputs
  and packed-word inputs
- **THEN** for every sample `EncodeInputPosition` equals the reference encode and
  `DecodeInputPosition` equals the reference decode, and the suite exits 0

#### Scenario: A codec regression is caught

- **WHEN** the codec is changed so a covered input no longer matches the original arithmetic
- **THEN** at least one equivalence check fails and the harness exits non-zero

### Requirement: IPC parser robustness regression coverage

The host harness SHALL guard the response parser against two known crash regressions: a
`ctx.preedit.cursor` value with only two comma-separated fields (must not read a third out of
bounds), and a `config.*` line delivered to a parser that has no Config sink (`p_config == nullptr`,
must not dereference it). A reintroduction of either MUST make the harness exit non-zero.

#### Scenario: Two-field preedit cursor does not read out of bounds

- **WHEN** the parser receives `ctx.preedit.cursor=0,3` (start,end with no cursor)
- **THEN** it records a highlight attribute with start 0 and end 3 (cursor left at its default) and
  does not index a third field

#### Scenario: config line with no Config sink is a safe no-op

- **WHEN** the parser has a Context sink but no Config sink and receives `config.inline_preedit=1`
- **THEN** it returns without dereferencing the null Config pointer (no crash)

#### Scenario: config line with a Config sink is applied

- **WHEN** the parser has a Config sink and receives `config.inline_preedit=1`
- **THEN** the Config's `inline_preedit` becomes true

