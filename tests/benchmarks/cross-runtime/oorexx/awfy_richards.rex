/*
 * ooRexx state-machine port of the Are We Fast Yet? Richards benchmark.
 * Derived from the SOM/Java Richards benchmark; see ../../THIRD_PARTY_NOTICES.md.
 * The task-kind dispatch matches the cREXX and NetRexx ports.
 */
parse arg repetitions
if repetitions='' then repetitions=1
if repetitions<1 then call fail 'repetitions must be positive'

queueCount=0
holdCount=0
do iteration=1 to repetitions
  scheduler=.RichardsScheduler~new
  result=scheduler~run
  queueCount=scheduler~queueCount
  holdCount=scheduler~holdCount
  if \result then call fail 'expected queue/hold counts 23246/9297, got' queueCount'/'holdCount
end

say 'benchmark=awfy_richards repetitions='repetitions 'queue_packets='queueCount 'holds='holdCount
say 'PASS: AWFY Richards ooRexx port'
exit 0

fail: procedure
  parse arg message
  say 'FAIL:' message
  exit 1

::class RichardsScheduler
::method init
  expose task_link. task_priority. task_input. task_pending. task_waiting. task_holding. -
    handler_work. handler_device. device_pending. packet_link. packet_identity. packet_kind. -
    packet_datum. packet_data. task_list current_task current_identity packet_count -
    queue_packet_count hold_count idle_control idle_count worker_destination worker_count
  task_link.=0
  task_priority.=0
  task_input.=0
  task_pending.=0
  task_waiting.=0
  task_holding.=0
  handler_work.=0
  handler_device.=0
  device_pending.=0
  packet_link.=0
  packet_identity.=0
  packet_kind.=0
  packet_datum.=0
  packet_data.=0
  task_list=-1
  current_task=-1
  current_identity=0
  packet_count=0
  queue_packet_count=0
  hold_count=0
  idle_control=1
  idle_count=10000
  worker_destination=2
  worker_count=0

::method run
  expose queue_packet_count hold_count
  self~createTask(0,0,0,0,0,0)
  workQueue=self~createPacket(0,1,1)
  workQueue=self~createPacket(workQueue,1,1)
  self~createTask(1,1000,workQueue,1,1,0)
  workQueue=self~createPacket(0,4,0)
  workQueue=self~createPacket(workQueue,4,0)
  workQueue=self~createPacket(workQueue,4,0)
  self~createTask(2,2000,workQueue,1,1,0)
  workQueue=self~createPacket(0,5,0)
  workQueue=self~createPacket(workQueue,5,0)
  workQueue=self~createPacket(workQueue,5,0)
  self~createTask(3,3000,workQueue,1,1,0)
  self~createTask(4,4000,0,0,1,0)
  self~createTask(5,5000,0,0,1,0)
  self~schedule
  return queue_packet_count=23246 & hold_count=9297

::method queueCount
  expose queue_packet_count
  return queue_packet_count

::method holdCount
  expose hold_count
  return hold_count

::method createTask private
  expose task_link. task_priority. task_input. task_pending. task_waiting. task_holding. task_list
  use strict arg identity,priority,input,pending,waiting,holding
  task_link.identity=task_list
  task_priority.identity=priority
  task_input.identity=input
  task_pending.identity=pending
  task_waiting.identity=waiting
  task_holding.identity=holding
  task_list=identity

::method createPacket private
  expose packet_count packet_link. packet_identity. packet_kind. packet_datum. packet_data.
  use strict arg link,identity,kind
  packet_count=packet_count+1
  packet_link.packet_count=link
  packet_identity.packet_count=identity
  packet_kind.packet_count=kind
  packet_datum.packet_count=0
  do i=0 to 3
    packet_data.packet_count.i=0
  end
  return packet_count

::method appendPacket private
  expose packet_link.
  use strict arg packet,queueHead
  packet_link.packet=0
  if queueHead=0 then return packet
  mouse=queueHead
  link=packet_link.mouse
  do while link<>0
    mouse=link
    link=packet_link.mouse
  end
  packet_link.mouse=packet
  return queueHead

::method setRunning private
  expose task_pending. task_waiting. task_holding.
  use strict arg identity
  task_pending.identity=0
  task_waiting.identity=0
  task_holding.identity=0

::method setPacketPending private
  expose task_pending. task_waiting. task_holding.
  use strict arg identity
  task_pending.identity=1
  task_waiting.identity=0
  task_holding.identity=0

::method isHoldingOrWaiting private
  expose task_pending. task_waiting. task_holding.
  use strict arg identity
  return task_holding.identity | (task_pending.identity=0 & task_waiting.identity)

::method isWaitingWithPacket private
  expose task_pending. task_waiting. task_holding.
  use strict arg identity
  return task_pending.identity & task_waiting.identity & task_holding.identity=0

::method addInput private
  expose task_input. task_pending. task_priority.
  use strict arg identity,packet,oldTask
  if task_input.identity=0 then do
    task_input.identity=packet
    task_pending.identity=1
    if task_priority.identity>task_priority.oldTask then return identity
  end
  else task_input.identity=self~appendPacket(packet,task_input.identity)
  return oldTask

::method holdSelf private
  expose hold_count task_holding. task_link. current_task
  hold_count=hold_count+1
  task_holding.current_task=1
  return task_link.current_task

::method markWaiting private
  expose task_waiting. current_task
  task_waiting.current_task=1
  return current_task

::method release private
  expose task_holding. task_priority. current_task
  use strict arg identity
  task_holding.identity=0
  if task_priority.identity>task_priority.current_task then return identity
  return current_task

::method queuePacket private
  expose packet_identity. packet_link. queue_packet_count current_identity current_task
  use strict arg packet
  task=packet_identity.packet
  queue_packet_count=queue_packet_count+1
  packet_link.packet=0
  packet_identity.packet=current_identity
  return self~addInput(task,packet,current_task)

::method runTask private
  expose task_input. packet_link.
  use strict arg identity
  message=0
  if self~isWaitingWithPacket(identity) then do
    message=task_input.identity
    task_input.identity=packet_link.message
    if task_input.identity=0 then self~setRunning(identity)
    else self~setPacketPending(identity)
  end
  select
    when identity=0 then return self~runIdler
    when identity=1 then return self~runWorker(message)
    when identity=2 | identity=3 then return self~runHandler(identity,message)
    when identity=4 | identity=5 then return self~runDevice(identity,message)
    otherwise return -1
  end

::method runIdler private
  expose idle_count idle_control
  idle_count=idle_count-1
  if idle_count=0 then return self~holdSelf
  if idle_control//2=0 then do
    idle_control=idle_control%2
    return self~release(4)
  end
  idle_control=self~xorInt(idle_control%2,53256)
  return self~release(5)

::method xorInt private
  use strict arg left,right
  return x2d(c2x(bitxor(d2c(left,2),d2c(right,2))))

::method runWorker private
  expose worker_destination worker_count packet_identity. packet_datum. packet_data.
  use strict arg work
  if work=0 then return self~markWaiting
  if worker_destination=2 then worker_destination=3
  else worker_destination=2
  packet_identity.work=worker_destination
  packet_datum.work=0
  do i=0 to 3
    worker_count=worker_count+1
    if worker_count>26 then worker_count=1
    packet_data.work.i=65+worker_count-1
  end
  return self~queuePacket(work)

::method runHandler private
  expose packet_kind. packet_link. packet_datum. packet_data. handler_work. handler_device.
  use strict arg identity,work
  if work<>0 then do
    if packet_kind.work=1 then handler_work.identity=self~appendPacket(work,handler_work.identity)
    else handler_device.identity=self~appendPacket(work,handler_device.identity)
  end
  workPacket=handler_work.identity
  if workPacket=0 then return self~markWaiting
  count=packet_datum.workPacket
  if count>=4 then do
    handler_work.identity=packet_link.workPacket
    return self~queuePacket(workPacket)
  end
  devicePacket=handler_device.identity
  if devicePacket=0 then return self~markWaiting
  handler_device.identity=packet_link.devicePacket
  packet_datum.devicePacket=packet_data.workPacket.count
  packet_datum.workPacket=count+1
  return self~queuePacket(devicePacket)

::method runDevice private
  expose device_pending.
  use strict arg identity,work
  if work=0 then do
    pending=device_pending.identity
    if pending=0 then return self~markWaiting
    device_pending.identity=0
    return self~queuePacket(pending)
  end
  device_pending.identity=work
  return self~holdSelf

::method schedule private
  expose current_task current_identity task_list task_link.
  current_task=task_list
  do while current_task>=0
    if self~isHoldingOrWaiting(current_task) then current_task=task_link.current_task
    else do
      current_identity=current_task
      current_task=self~runTask(current_task)
    end
  end
