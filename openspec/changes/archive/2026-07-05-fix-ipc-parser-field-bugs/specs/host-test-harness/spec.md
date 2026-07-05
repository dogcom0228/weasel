## ADDED Requirements

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
