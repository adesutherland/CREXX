#!/bin/zsh
set -eu

panel_dir=${0:A:h}
repo_dir=${panel_dir:h:h:h}
release_bin=$repo_dir/cmake-build-release/bin
raw_dir=$panel_dir/raw
summary=$panel_dir/pilot-timings.csv
mkdir -p $raw_dir
printf '%s\n' 'vm,candidate,mode,family,size,iterations,elapsed_us,checksum,order' > $summary

modes=(get-hit get-miss set-existing set-new reset-default multi-tail)
rxvm_orders=('d1 d2 d2h' 'd2 d2h d1' 'd2h d1 d2')
rxbvm_orders=('d2h d2 d1' 'd1 d2h d2' 'd2 d1 d2h')

for vm in rxvm rxbvm
do
  mode_index=1
  for mode in $modes
  do
    iterations=200000
    if [[ $mode == set-new ]]
    then
      iterations=20000
    fi
    order_index=$(( (($mode_index - 1) % 3) + 1 ))
    if [[ $vm == rxvm ]]
    then
      candidates=(${=rxvm_orders[$order_index]})
    else
      candidates=(${=rxbvm_orders[$order_index]})
    fi
    position=1
    for candidate in $candidates
    do
      stem=$vm-$candidate-$mode-ascii-64-pilot
      log=$raw_dir/$stem.log
      $release_bin/$vm $panel_dir/nr15_architecture_control.rxbin \
        $release_bin/library.rxbin -a $candidate $mode 64 $iterations ascii \
        > $log 2>&1
      elapsed=$(awk -F= '$1 == "elapsed_us" {print $2}' $log)
      checksum=$(awk -F= '$1 == "checksum" {print $2}' $log)
      printf '%s,%s,%s,ascii,64,%s,%s,%s,%s\n' \
        $vm $candidate $mode $iterations $elapsed $checksum $position >> $summary
      position=$((position + 1))
    done
    mode_index=$((mode_index + 1))
  done
done
