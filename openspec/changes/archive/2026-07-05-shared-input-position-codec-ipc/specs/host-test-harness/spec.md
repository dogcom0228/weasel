## ADDED Requirements

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
