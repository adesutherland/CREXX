#!/bin/bash
set -u
root=/Users/adrian/CLionProjects/crexx-scaling-20260906.1lT795
export CREXX_HOME=/Users/adrian/CLionProjects/crexx-scaling-20260906.1lT795/install-baseline/
export SCALING_RUN=/Users/adrian/CLionProjects/crexx-scaling-20260906.1lT795/evidence/address-baseline
mkdir -p "$SCALING_RUN" /Users/adrian/CLionProjects/crexx-scaling-20260906.1lT795/address-measured-baseline
cd /Users/adrian/CLionProjects/crexx-scaling-20260906.1lT795/address-measured-baseline
/usr/bin/time -lp -o "$SCALING_RUN/wave.time" /Users/adrian/CLionProjects/crexx-scaling-20260906.1lT795/install-baseline/bin/crexx --library /Users/adrian/CLionProjects/crexx-scaling-20260906.1lT795/address-measured-baseline/rag_address_environment /Users/adrian/CLionProjects/crexx-scaling-20260906.1lT795/donor-qualification/crexx/application/surfaces/rag_address_environment.crexx --jobs 1 --noexec --nocolor --verbose1 -s '/Users/adrian/CLionProjects/crexx-scaling-20260906.1lT795/donor-qualification/crexx/providers;/Users/adrian/CLionProjects/crexx-scaling-20260906.1lT795/donor-qualification/crexx/application;/Users/adrian/CLionProjects/crexx-scaling-20260906.1lT795/donor-qualification/crexx/application/config;/Users/adrian/CLionProjects/crexx-scaling-20260906.1lT795/donor-qualification/crexx/application/config/profiles' -i '/Users/adrian/CLionProjects/crexx-scaling-20260906.1lT795/install-baseline/bin/providers;/Users/adrian/CLionProjects/crexx-scaling-20260906.1lT795/install-baseline/bin' > "$SCALING_RUN/wave.log" 2>&1 &
leader=$!
printf '%s\n' "$leader" > "$SCALING_RUN/leader.pid"
start=$SECONDS
while kill -0 "$leader" 2>/dev/null; do
 ps -axo pid=,ppid=,etime=,time=,rss=,command= | awk -v root="$root" 'index($0,root)>0' >> "$SCALING_RUN/process-samples.txt"
 if (( SECONDS - start >= 900 )); then
  echo 'BOUND EXPIRED at 900s' >> "$SCALING_RUN/wave.log"
  killtree() { local p; for p in $(pgrep -P "$1"); do killtree "$p"; done; kill -TERM "$1" 2>/dev/null || true; }
  killtree "$leader"
  break
 fi
 sleep 2
done
wait "$leader"
rc=$?
printf 'observed wave rc=%s\n' "$rc"
tail -20 "$SCALING_RUN/wave.log"
