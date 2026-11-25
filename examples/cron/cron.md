# CRON for cREXX

CRON is a small, cron-style job scheduler written in cREXX Level B. It reads a crontab file containing schedules and commands, then starts jobs when they are due.

This example is a cREXX port of Mike Beer's `CRON.rexx` for Regina REXX. The two versions share the scheduling engine, crontab format and commands. The original project is available at [mikebeer/CRON](https://github.com/mikebeer/CRON).

## Build and test with RXBUILD

From the cREXX project root, run:

```text
crexx examples/cron/cron.graph.build
```

This assumes the graph definition is named `cron.graph.build`; use its actual filename if different. RXBUILD compiles `examples/cron/cron.crexx` to `.crexx-build/cron/cron.rxbin` and runs the built-in `selftest` in `.crexx-build/cron`. The self-test is the default target. This build tests CRON; it does not start the scheduler.

## Run CRON

After the RXBUILD step, the program is in `.crexx-build/cron`. For the commands below, either change to that directory (and put your crontab there) or specify the appropriate paths to the program and crontab.

Alternatively, compile the program directly, then use `rxvme` to run it:

```text
crexx cron.crexx -noexec
rxvme cron -a selftest
rxvme cron -a list jobs.crontab
rxvme cron -a run jobs.crontab VERBOSE
```

The first command creates `cron.rxbin`. Arguments for the CRON program follow `-a` when using `rxvme`. Run the commands from a location where the program and the specified crontab can be found, or adjust the paths for your setup.

The scheduler runs until stopped. From another terminal, request a stop with:

```text
rxvme cron -a stop jobs.crontab
```

The stop request can take up to five seconds to take effect. To check whether the scheduler reports itself as running:

```text
rxvme cron -a status jobs.crontab
```

## A crontab example

A crontab is a text file with one job per line. Each job has a schedule followed by a command:

```text
# minute hour day-of-month month day-of-week  command
*/5 * * * *         echo every five minutes
30 2 * * 1-5        echo weekdays at 02:30
0 9,17 * * MON-FRI  echo 09:00 and 17:00 on weekdays
@hourly             echo top of the hour
@daily              echo midnight
```

Blank lines and lines beginning with `#` are skipped. Lines of the form `NAME=value` set an environment variable for the jobs that follow. For example:

```text
REPORT_MODE=summary
0 9 * * 1-5  echo weekday report
```

CRON rereads the crontab every minute, so edits can take effect without restarting the scheduler. Times use the machine's local clock. A scheduler started partway through a minute waits for the next full minute; jobs missed while it was not running are not caught up.

## Cron expressions

A standard expression has five fields:

```text
minute  hour  day-of-month  month  day-of-week
```

| Field | Values |
| --- | --- |
| Minute | `0`–`59` |
| Hour | `0`–`23` |
| Day of month | `1`–`31` |
| Month | `1`–`12` or `JAN`–`DEC` |
| Day of week | `0`–`7` or `SUN`–`SAT`; both `0` and `7` mean Sunday |

Fields support `*`, individual values, ranges, lists and steps. Examples include `*/5`, `5-30/10` and `9,17`. CRON also recognises `@hourly`, `@daily`, `@midnight`, `@weekly`, `@monthly`, `@yearly` and `@annually`.

When both day-of-month and day-of-week are restricted, a date matching **either** field is eligible, following Vixie cron behaviour.

## Commands

The crontab filename is the first argument for commands that use one. In the table below, `stamp` defaults to the current local time if omitted. It accepts `YYYY-MM-DD HH:MM` or `YYYY-MM-DDTHH:MM[:SS]`; quote a timestamp containing a space when passing it through a shell.

| Command | Purpose |
| --- | --- |
| `run crontab [options]` | Run the scheduler loop until stopped. |
| `tick crontab [stamp]` | Run jobs due at the specified time; useful for testing or invocation by another scheduler. |
| `next "m h dom mon dow" [stamp]` | Show the next fire time after the specified time. |
| `match "m h dom mon dow" [stamp]` | Return `1` if the expression matches the time, otherwise `0`. |
| `valid "m h dom mon dow"` | Return `1` for a valid expression, otherwise `0`. |
| `add crontab "m h dom mon dow" command...` | Validate the expression and append a job. |
| `list crontab` | List numbered jobs and their next fire times. |
| `remove crontab n` | Remove the job with the number shown by `list`. |
| `stop crontab` | Ask the running scheduler for that crontab to stop. |
| `status crontab` | Report whether the scheduler is running. |
| `selftest` | Run the built-in tests, which use temporary files. |

For example:

```text
rxvme cron -a valid "*/5 * * * *"
rxvme cron -a next "0 9 * * MON-FRI"
rxvme cron -a add jobs.crontab "0 10 * * *" echo hello
rxvme cron -a list jobs.crontab
```

## Scheduler options

Options follow the crontab filename on `run`:

| Option | Purpose |
| --- | --- |
| `VERBOSE` | Provide more information while the scheduler runs. |
| `SYNC` | Wait for each job instead of starting jobs in the background. |
| `DRYRUN` | Test scheduling without actually running the jobs. |
| `ONCE` | Run the scheduler in its one-shot mode. |

For example:

```text
rxvme cron -a run jobs.crontab VERBOSE DRYRUN
```

Jobs start in the background by default. Use `SYNC` when you want the scheduler to wait for each job.

## Platform notes

The example is intended for Linux, Windows and macOS. The information supplied with the original project reports that the cREXX self-test passed on Linux; its Windows version was not yet tested there. Verify the current project build and tests on each platform before relying on them.

In the cREXX beta 1 environment described by the source, `ADDRESS` does not use a shell. CRON therefore writes each job to a small script alongside the crontab (`.job.sh` or `.job.cmd`) and runs that script. **File names must not contain spaces** in that version. Set `CRON_OS` to `WIN` or `UNIX` to override operating-system detection if needed.

A previous version of the source also documented a `lineout` workaround for bare variables. Consult the source for the current implementation and its compatibility notes.

## Acknowledgement

CRON was created by Mike Beer. This cREXX example is based on his Regina REXX implementation.
