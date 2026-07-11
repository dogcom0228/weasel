## 1. TDD via the host harness (golden_test.cpp is LF; Deserializer.h is CRLF — preserve both)

- [x] 1.1 Add `test_corrupt_cand_blob_no_crash` to test/host/golden_test.cpp: build a valid
      one-candidate blob, assert the known length field is present, corrupt it to a huge value,
      parse `action=ctx,status` + corrupt `ctx.cand=` + `status.ascii_mode=1`, assert
      `st.ascii_mode` was still parsed (parser survived).
- [x] 1.2 RED: run the harness via WSL — the new test CRASHED as predicted (uncaught
      `std::length_error`, golden_test FAIL exit 134), proving it exercises the escape.
- [x] 1.3 Fix `TryDeserialize` (WeaselIPC/Deserializer.h): added
      `catch (const std::exception& e)` after the existing `archive_exception` catch, reporting
      via the same `MessageBoxA` call with a `std::exception: ` prefix.
- [x] 1.4 GREEN: harness re-run — ALL PASS.

## 2. Verify, review, commit

- [x] 2.1 Confirmed the `archive_exception` branch is byte-identical (derived class caught first)
      and valid-payload behavior unchanged; CRLF (header) / LF (test) preserved.
- [x] 2.2 Adversarial review: PASS. Reviewer independently reproduced the red run (exit 134) and
      the green run; confirmed catch ordering, deterministic corruption (runtime-asserted,
      length parses into 64-bit size_t > wstring::max_size), non-vacuous post-condition, and that
      golden_test.cpp is compiled only by the host harness. Follow-up recorded: the
      `text_wiarchive` header-reading construction in ContextUpdater/Styler sits OUTSIDE
      TryDeserialize and can still throw (pre-existing escape; separate change).
- [x] 2.3 Commit on `master` (`fix(WeaselIPC):`) + Co-Authored-By; archive manually
      (git mv → openspec/changes/archive/2026-07-11-...).
