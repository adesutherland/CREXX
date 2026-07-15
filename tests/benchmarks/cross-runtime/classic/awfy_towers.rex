/*
 * Classic Rexx procedural port of the Are We Fast Yet? Towers benchmark.
 * Derived from the SOM benchmark suite under the MIT license; see
 * ../../LICENSE-SOM-MIT.txt and ../../README.md.
 *
 * Disk objects are represented by allocated numeric node ids plus stems. This
 * preserves the recursive algorithm and link mutations but not object-dispatch
 * or allocator costs, so NR-02 classifies it as a disclosed adaptation.
 */
parse arg repetitions
if repetitions='' then repetitions=1
if repetitions<1 then call fail 'repetitions must be positive'

result=0
do iteration=1 to repetitions
  next_id=0
  disk_size.=0
  next_disk.=0
  pile.=0
  moves_done=0
  call build_tower_at 0,13
  moves_done=0
  call move_disks 13,0,1
  result=moves_done
  if result<>8191 then call fail 'expected 8191 moves, got' result
end

say 'benchmark=awfy_towers repetitions='repetitions 'result='result
say 'PASS: AWFY Towers Classic procedural adaptation'
exit 0

new_disk: procedure expose next_id disk_size. next_disk.
  parse arg size
  next_id=next_id+1
  disk_size.next_id=size
  next_disk.next_id=0
  return next_id

push_disk: procedure expose disk_size. next_disk. pile.
  parse arg disk,pile_number
  top=pile.pile_number
  if top<>0 then do
    if disk_size.disk>=disk_size.top then return
    next_disk.disk=top
  end
  pile.pile_number=disk
  return

pop_disk_from: procedure expose next_disk. pile.
  parse arg pile_number
  top=pile.pile_number
  pile.pile_number=next_disk.top
  return top

move_top_disk: procedure expose disk_size. next_disk. pile. moves_done
  parse arg from_pile,to_pile
  disk=pop_disk_from(from_pile)
  call push_disk disk,to_pile
  moves_done=moves_done+1
  return

build_tower_at: procedure expose next_id disk_size. next_disk. pile.
  parse arg pile_number,disks
  do i=disks to 0 by -1
    disk=new_disk(i)
    call push_disk disk,pile_number
  end
  return

move_disks: procedure expose disk_size. next_disk. pile. moves_done
  parse arg disks,from_pile,to_pile
  if disks=1 then call move_top_disk from_pile,to_pile
  else do
    other_pile=(3-from_pile)-to_pile
    call move_disks disks-1,from_pile,other_pile
    call move_top_disk from_pile,to_pile
    call move_disks disks-1,other_pile,to_pile
  end
  return

fail: procedure
  parse arg message
  say 'FAIL:' message
  exit 1
