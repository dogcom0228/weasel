## 1. Drop the redundant "for initialize" clauses (byte-preserving; WeaselTrayIcon.cpp is LF + UTF-8 BOM)

- [x] 1.1 `Refresh`: remove clauses (3) and (5); keep `mode != m_mode || zhung-icon-changed ||
      ascii-icon-changed`. Rewrite the block comment (first init = INITIAL sentinel).

## 2. Verify, review, commit

- [x] 2.1 x64 compile-check green. Confirm: first Refresh still sets the icon (m_mode==INITIAL !=
      mode); 中/英 toggle still re-sets (mode change); schema icon path change still re-sets;
      no-change keystroke no longer re-sets; visibility-recovery branch intact. LF+BOM preserved.
- [x] 2.2 Adversarial review: prove the removed clauses were redundant (every state they caught is
      caught by mode/icon-change or the !Visible() branch) AND were the sole cause of the
      per-keystroke Shell_NotifyIcon; confirm no first-init or icon-change regression on all
      transitions (INITIAL→ZHUNG/ASCII/DISABLED, ZHUNG↔ASCII, enable/disable, custom↔empty icons).
      Address blockers.
- [x] 2.3 Commit on `master` (`fix(WeaselServer):`) + Co-Authored-By; archive manually.
