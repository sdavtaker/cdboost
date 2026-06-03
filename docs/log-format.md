# cdboost Log Format

cdboost emits structured **NDJSON** (Newline-Delimited JSON) — one JSON object per line.
Output goes to stdout via spdlog. Call `cdboost::log::init()` once at program startup to
activate logging; without it, all `emit()` calls are no-ops.

## Required Fields

Every log record contains exactly these fields:

| Field   | Type   | Description |
|---------|--------|-------------|
| `ts`    | string | Wall-clock timestamp, ISO 8601 UTC with millisecond precision: `2026-04-27T14:05:32.417Z` |
| `level` | string | Severity: `debug`, `info`, `warn`, or `error` |
| `event` | string | Structured event name (see Event Names below) |
| `msg`   | string | Human-readable description of the event |

## Optional Fields

| Field      | Type   | Description |
|------------|--------|-------------|
| `sim_time` | string | Simulation time in the type's native string representation (e.g. `"0.5"` for float/double, `"1/10"` for rational, `"100"` for fixed-point). Trailing zeros are suppressed for floating-point (`"1"` not `"1.0"`). **Never replaces `ts`** — `ts` is always wall-clock time. `sim_time` carries the logical simulation time. |

## ts vs sim_time

`ts` is the wall-clock instant the log line was written. `sim_time` is the logical time inside
the simulation. They measure different things and are always independent:

- A simulation running 7200 logical seconds may complete in 0.4 wall-clock seconds.
- Use `ts` to correlate log lines with external events (CI timestamps, profiling).
- Use `sim_time` to understand where in the simulation an event occurred.

## Event Names

| Event               | Level  | sim_time | Emitted by |
|---------------------|--------|----------|------------|
| `simulation_start`  | info   | yes (initial time) | `runner::runUntil`, `runner::runUntilPassivate` |
| `tick`              | info   | yes (event time)   | `runner` — once per output message when formatter is set |
| `simulation_end`    | info   | yes (end time)     | `runner::runUntil`, `runner::runUntilPassivate` |
| `performance`       | info   | no                 | `runner` — wall time of the simulation loop |
| `model_setup`       | info   | no                 | Example programs — model construction steps |

## Examples

```ndjson
{"ts":"2026-04-27T14:05:32.000Z","level":"info","event":"model_setup","msg":"Creating atomic models for 3 needles"}
{"ts":"2026-04-27T14:05:32.001Z","level":"info","event":"simulation_start","msg":"Starting simulation","sim_time":"0"}
{"ts":"2026-04-27T14:05:32.002Z","level":"info","event":"tick","msg":"second","sim_time":"1"}
{"ts":"2026-04-27T14:05:32.003Z","level":"info","event":"tick","msg":"second","sim_time":"2"}
{"ts":"2026-04-27T14:05:32.004Z","level":"info","event":"tick","msg":"minute","sim_time":"60"}
{"ts":"2026-04-27T14:05:32.391Z","level":"info","event":"simulation_end","msg":"Simulation ended","sim_time":"7200"}
{"ts":"2026-04-27T14:05:32.391Z","level":"info","event":"performance","msg":"Wall time: 0.389 sec"}
```

## Exception Handling

`cdboost::log::log_exception(e, "event_name")` emits an `error`-level record and flushes
the logger before rethrowing. Call it inside a catch block:

```cpp
try {
    r.runUntil(end_time);
} catch (const std::exception& e) {
    cdboost::log::log_exception(e, "simulation_error");
    throw;
}
```

The flush ensures the error record reaches stdout even if the process exits immediately after.

## Parsing with jq

Extract all tick events:
```sh
jq 'select(.event == "tick")' sim.ndjson
```

Extract sim_time of all ticks as a stream:
```sh
jq -r 'select(.event == "tick") | .sim_time' sim.ndjson
```

Note: `sim_time` is a JSON string. To sort or compare numerically, parse the value
in your processing layer (e.g. `tonumber` in jq for float/integer outputs).

Show simulation wall time:
```sh
jq -r 'select(.event == "performance") | .msg' sim.ndjson
```

Count ticks:
```sh
jq -s '[.[] | select(.event == "tick")] | length' sim.ndjson
```

## C++ API Reference

```cpp
#include <cdboost/log.hpp>

// Initialize once at startup. No-op if already initialized.
cdboost::log::init();

// Emit a log record. sim_time is optional.
cdboost::log::emit(cdboost::log::level::info, "event_name", "message");
cdboost::log::emit(cdboost::log::level::info, "tick", "second",
                   cdboost::log::to_sim_string(my_time_value));

// Flush pending records to stdout.
cdboost::log::flush();

// Log an exception at error level and flush (call inside catch block before rethrow).
cdboost::log::log_exception(e, "simulation_error");

// Convert any TIME value to its native string representation for sim_time.
// Float/double: full round-trip precision via std::format.
// Any other stream-insertable type: uses operator<< (e.g. "1/10" for rational,
// "100" for fixed-point). Non-streamable types produce "?".
std::string s = cdboost::log::to_sim_string(my_time_value);
```
