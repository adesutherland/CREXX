#!/usr/bin/env bash
set -Eeuo pipefail

DEFAULT_FOCUSED_REGEX='^(linked_opt_runtime_artifacts_build|crexx_spaced_source_smoke|inline_cross_file_.*|source_import_.*|interface_.*import.*)$'

build_dir="cmake-build-debugasan"
build_jobs=4
test_jobs=8
test_jobs_set=0
build_targets=()
phase="full"
build_leaks="on"
test_leaks="off"
test_leaks_set=0
regex=""
exclude_regex=""
exclude_regex_file=""
ctest_index=""
fixture_exclude_setup=""
fixture_exclude_any=""
log_root=""
tail_lines=80
poll_seconds=30
live_tail=1
stop_on_failure="auto"
active_pid=""
active_name=""
log_dir=""

usage() {
    cat <<'USAGE'
Usage: tools/asan-run.sh [options]

Phases:
  --phase build          Full cmake build only.
  --phase full           Full build, then full ctest. Default test leaks: on.
  --phase ctest          Run ctest only. Use after a full build.
  --phase focused-lsan   Build focused dependencies, then focused LSan tests.
  --phase rerun-failed   Run ctest --rerun-failed.

Options:
  --build-dir DIR        Build directory. Default: cmake-build-debugasan.
  --build-jobs N         Build parallelism. Default: 4.
  --build-target TARGET  Build only this target in --phase build. Repeatable.
  --test-jobs N          CTest parallelism. Default: 8, except focused-lsan defaults to 1.
  --leaks on|off         Set test-phase leak detection. focused-lsan defaults to on.
  --build-leaks on|off   Set build-phase leak detection. Default: on.
  --regex REGEX          CTest -R regex. focused-lsan has a built-in default.
  --exclude-regex REGEX  CTest -E regex for excluding already-covered tests.
  --exclude-regex-file FILE
                         Read CTest -E regex from FILE.
  --ctest-index SPEC     Pass a CTest -I range, e.g. '125,,' to continue from test 125.
  --fixture-exclude-setup REGEX
                         Pass CTest --fixture-exclude-setup. Use after prebuilding fixture targets.
  --fixture-exclude-any REGEX
                         Pass CTest --fixture-exclude-any.
  --log-root DIR         Parent directory for run logs. Default: BUILD_DIR/asan-logs.
  --tail-lines N         Lines printed on progress/failure. Default: 80.
  --poll-seconds N       Seconds between live tail updates. Default: 30.
  --no-live-tail         Do not print periodic tails while a command runs.
  --stop-on-failure      Stop CTest at the first failing test.
  --keep-going           Do not stop CTest at the first failing test.
  --status RUN_DIR       Show current command and tail current log for a run directory.
  --kill RUN_DIR         Send SIGINT, then SIGTERM if needed, to the current command group.
  -h, --help             Show this help.

Examples:
  tools/asan-run.sh --phase focused-lsan
  tools/asan-run.sh --phase build --build-target test_highlight_editor_diagnostics
  tools/asan-run.sh --phase full --test-jobs 8
  tools/asan-run.sh --phase ctest --regex '^crexx_spaced_source_smoke$' --leaks on
  tools/asan-run.sh --phase full --leaks on
USAGE
}

die() {
    echo "asan-run: $*" >&2
    exit 2
}

read_pid() {
    local run_dir=$1
    [[ -f "$run_dir/current.pid" ]] || die "no current.pid in $run_dir"
    tr -d '[:space:]' < "$run_dir/current.pid"
}

status_run() {
    local run_dir=$1
    [[ -d "$run_dir" ]] || die "run directory not found: $run_dir"
    echo "run: $run_dir"
    [[ -f "$run_dir/current.name" ]] && echo "current: $(cat "$run_dir/current.name")"
    [[ -f "$run_dir/current.pid" ]] && echo "pid: $(cat "$run_dir/current.pid")"
    if [[ -L "$run_dir/current.log" || -f "$run_dir/current.log" ]]; then
        echo "log: $(readlink -f "$run_dir/current.log" 2>/dev/null || echo "$run_dir/current.log")"
        tail -n "$tail_lines" "$run_dir/current.log" || true
    else
        echo "no current.log"
    fi
}

kill_run() {
    local run_dir=$1
    local pid
    [[ -d "$run_dir" ]] || die "run directory not found: $run_dir"
    pid=$(read_pid "$run_dir")
    if ! kill -0 "$pid" 2>/dev/null; then
        echo "process $pid is not running"
        return 0
    fi
    echo "sending SIGINT to process group $pid"
    kill -INT "-$pid" 2>/dev/null || kill -INT "$pid" 2>/dev/null || true
    sleep 3
    if kill -0 "$pid" 2>/dev/null; then
        echo "sending SIGTERM to process group $pid"
        kill -TERM "-$pid" 2>/dev/null || kill -TERM "$pid" 2>/dev/null || true
    fi
}

cleanup_active() {
    local rc=$?
    if [[ -n "${active_pid:-}" ]] && kill -0 "$active_pid" 2>/dev/null; then
        echo "asan-run: stopping $active_name process group $active_pid" >&2
        kill -INT "-$active_pid" 2>/dev/null || kill -INT "$active_pid" 2>/dev/null || true
        sleep 3
        if kill -0 "$active_pid" 2>/dev/null; then
            kill -TERM "-$active_pid" 2>/dev/null || kill -TERM "$active_pid" 2>/dev/null || true
        fi
    fi
    exit "$rc"
}

asan_options_for_leaks() {
    local value=$1
    local detect_leaks
    case "$value" in
        on) detect_leaks=1 ;;
        off) detect_leaks=0 ;;
        *) die "leak setting must be on or off" ;;
    esac

    if [[ -n "${BASE_ASAN_OPTIONS:-}" ]]; then
        printf 'detect_leaks=%s:%s' "$detect_leaks" "$BASE_ASAN_OPTIONS"
    else
        printf 'detect_leaks=%s' "$detect_leaks"
    fi
}

strip_detect_leaks_option() {
    local input=$1
    local output=""
    local part
    local old_ifs=$IFS

    IFS=':'
    for part in $input; do
        if [[ "$part" == detect_leaks=* ]]; then
            continue
        fi
        if [[ -z "$output" ]]; then
            output=$part
        else
            output="$output:$part"
        fi
    done
    IFS=$old_ifs
    printf '%s' "$output"
}

run_logged() {
    local name=$1
    local leaks_value=$2
    local command_asan_options
    command_asan_options=$(asan_options_for_leaks "$leaks_value")
    shift
    shift
    local logfile="$log_dir/$name.log"
    local logfile_abs
    local rc=0
    local monitor_pid=""

    echo "==> $name"
    echo "    log: $logfile"
    {
        printf 'Command:'
        printf ' %q' "$@"
        printf '\n'
        printf 'ASAN_OPTIONS=%s\n\n' "$command_asan_options"
    } > "$logfile"

    if command -v setsid >/dev/null 2>&1; then
        ASAN_OPTIONS="$command_asan_options" setsid "$@" >> "$logfile" 2>&1 &
    else
        ASAN_OPTIONS="$command_asan_options" "$@" >> "$logfile" 2>&1 &
    fi

    active_pid=$!
    active_name=$name
    echo "$active_pid" > "$log_dir/current.pid"
    echo "$name" > "$log_dir/current.name"
    case "$logfile" in
        /*) logfile_abs=$logfile ;;
        *) logfile_abs=$PWD/$logfile ;;
    esac
    ln -sfn "$logfile_abs" "$log_dir/current.log"

    if [[ "$live_tail" -eq 1 ]]; then
        (
            while true; do
                sleep "$poll_seconds"
                echo "==> $name still running; tail -n $tail_lines $logfile"
                tail -n "$tail_lines" "$logfile" || true
            done
        ) &
        monitor_pid=$!
    fi

    if wait "$active_pid"; then
        rc=0
    else
        rc=$?
    fi

    if [[ -n "$monitor_pid" ]]; then
        kill "$monitor_pid" 2>/dev/null || true
        wait "$monitor_pid" 2>/dev/null || true
    fi

    active_pid=""
    active_name=""

    if [[ "$rc" -ne 0 ]]; then
        echo "==> $name failed with rc $rc"
        tail -n "$tail_lines" "$logfile" || true
        return "$rc"
    fi

    echo "==> $name passed"
    tail -n "$tail_lines" "$logfile" || true
}

run_cmake_build() {
    local name=$1
    shift
    run_logged "$name" "$build_leaks" cmake --build "$build_dir" "$@" --parallel "$build_jobs"
}

run_ctest() {
    local name=$1
    shift
    local args=(--test-dir "$build_dir" --output-on-failure)
    if [[ "$stop_on_failure" -eq 1 ]]; then
        args+=(--stop-on-failure)
    fi
    if [[ -n "$regex" ]]; then
        args+=(-R "$regex")
    fi
    if [[ -n "$exclude_regex" ]]; then
        args+=(-E "$exclude_regex")
    fi
    if [[ -n "$ctest_index" ]]; then
        args+=(-I "$ctest_index")
    fi
    if [[ -n "$fixture_exclude_setup" ]]; then
        args+=(--fixture-exclude-setup "$fixture_exclude_setup")
    fi
    if [[ -n "$fixture_exclude_any" ]]; then
        args+=(--fixture-exclude-any "$fixture_exclude_any")
    fi
    args+=(--parallel "$test_jobs")
    run_logged "$name" "$test_leaks" ctest "${args[@]}" "$@"
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --build-dir) build_dir=${2:?}; shift 2 ;;
        --build-jobs) build_jobs=${2:?}; shift 2 ;;
        --build-target) build_targets+=("${2:?}"); shift 2 ;;
        --test-jobs) test_jobs=${2:?}; test_jobs_set=1; shift 2 ;;
        --phase) phase=${2:?}; shift 2 ;;
        --leaks) test_leaks=${2:?}; test_leaks_set=1; shift 2 ;;
        --build-leaks) build_leaks=${2:?}; shift 2 ;;
        --regex) regex=${2:?}; shift 2 ;;
        --exclude-regex) exclude_regex=${2:?}; shift 2 ;;
        --exclude-regex-file) exclude_regex_file=${2:?}; shift 2 ;;
        --ctest-index) ctest_index=${2:?}; shift 2 ;;
        --fixture-exclude-setup) fixture_exclude_setup=${2:?}; shift 2 ;;
        --fixture-exclude-any) fixture_exclude_any=${2:?}; shift 2 ;;
        --log-root) log_root=${2:?}; shift 2 ;;
        --tail-lines) tail_lines=${2:?}; shift 2 ;;
        --poll-seconds) poll_seconds=${2:?}; shift 2 ;;
        --no-live-tail) live_tail=0; shift ;;
        --stop-on-failure) stop_on_failure=1; shift ;;
        --keep-going) stop_on_failure=0; shift ;;
        --status) status_run "${2:?}"; exit 0 ;;
        --kill) kill_run "${2:?}"; exit 0 ;;
        -h|--help) usage; exit 0 ;;
        *) die "unknown option: $1" ;;
    esac
done

case "$phase" in
    build|full|ctest|focused-lsan|rerun-failed) ;;
    *) die "unknown phase: $phase" ;;
esac
if [[ "${#build_targets[@]}" -gt 0 && "$phase" != "build" ]]; then
    die "--build-target is only valid with --phase build"
fi

if [[ "$phase" == "focused-lsan" ]]; then
    if [[ "$test_leaks_set" -eq 0 ]]; then test_leaks="on"; fi
    if [[ "$test_jobs_set" -eq 0 ]]; then test_jobs=1; fi
    if [[ -z "$regex" ]]; then regex="$DEFAULT_FOCUSED_REGEX"; fi
fi
if [[ "$phase" == "full" && "$test_leaks_set" -eq 0 ]]; then
    test_leaks="on"
fi

case "$build_leaks" in on|off) ;; *) die "--build-leaks must be on or off" ;; esac
case "$test_leaks" in on|off) ;; *) die "--leaks must be on or off" ;; esac
if [[ "$stop_on_failure" == "auto" ]]; then
    if [[ "$test_leaks" == "on" ]]; then
        stop_on_failure=1
    else
        stop_on_failure=0
    fi
fi
BASE_ASAN_OPTIONS=$(strip_detect_leaks_option "${ASAN_OPTIONS:-}")
if [[ -n "$exclude_regex_file" ]]; then
    [[ -f "$exclude_regex_file" ]] || die "exclude regex file not found: $exclude_regex_file"
    exclude_regex=$(tr -d '\n' < "$exclude_regex_file")
fi

if [[ -z "$log_root" ]]; then
    log_root="$build_dir/asan-logs"
fi
timestamp=$(date +%Y%m%d-%H%M%S)
log_dir="$log_root/$timestamp-$phase"
mkdir -p "$log_dir"
ln -sfn "$log_dir" "$log_root/latest"

cat > "$log_dir/run.env" <<EOF
phase=$phase
build_dir=$build_dir
build_jobs=$build_jobs
test_jobs=$test_jobs
build_leaks=$build_leaks
test_leaks=$test_leaks
stop_on_failure=$stop_on_failure
regex=$regex
exclude_regex=$exclude_regex
exclude_regex_file=$exclude_regex_file
ctest_index=$ctest_index
fixture_exclude_setup=$fixture_exclude_setup
fixture_exclude_any=$fixture_exclude_any
base_asan_options=$BASE_ASAN_OPTIONS
EOF
if [[ "${#build_targets[@]}" -gt 0 ]]; then
    printf 'build_targets=' >> "$log_dir/run.env"
    printf ' %q' "${build_targets[@]}" >> "$log_dir/run.env"
    printf '\n' >> "$log_dir/run.env"
fi

echo "ASan run directory: $log_dir"
trap cleanup_active INT TERM

case "$phase" in
    build)
        if [[ "${#build_targets[@]}" -gt 0 ]]; then
            run_cmake_build build --target "${build_targets[@]}"
        else
            run_cmake_build build
        fi
        ;;
    full)
        run_cmake_build build
        run_ctest ctest
        ;;
    ctest)
        run_ctest ctest
        ;;
    focused-lsan)
        run_cmake_build focused-build --target rxc crexx linked_opt_runtime_artifacts
        run_ctest focused-lsan
        ;;
    rerun-failed)
        run_ctest rerun-failed --rerun-failed
        ;;
esac

echo "ASan run complete: $log_dir"
