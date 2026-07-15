/*
 * ooRexx object port of the Are We Fast Yet? Bounce benchmark.
 * Derived from the SOM benchmark suite under the MIT license; see
 * ../../LICENSE-SOM-MIT.txt and ../../README.md.
 */
parse arg repetitions
if repetitions='' then repetitions=1
if repetitions<1 then call fail 'repetitions must be positive'

result=0
do iteration=1 to repetitions
  benchmark=.BounceBenchmark~new
  result=benchmark~run
  if result<>1331 then call fail 'expected 1331 bounces, got' result
end

say 'benchmark=awfy_bounce repetitions='repetitions 'result='result
say 'PASS: AWFY Bounce ooRexx object port'
exit 0

fail: procedure
  parse arg message
  say 'FAIL:' message
  exit 1

::class AwfyRandom
::method init
  expose seedValue
  seedValue=74755

::method next
  expose seedValue
  seedValue=((seedValue*1309)+13849)//65536
  return seedValue

::class BounceBall
::method init
  expose x y xVelocity yVelocity
  use strict arg random
  x=random~next//500
  y=random~next//500
  xVelocity=(random~next//300)-150
  yVelocity=(random~next//300)-150

::method bounce
  expose x y xVelocity yVelocity
  bounced=.false
  x=x+xVelocity
  y=y+yVelocity
  if x>500 then do
    x=500
    xVelocity=-abs(xVelocity)
    bounced=.true
  end
  if x<0 then do
    x=0
    xVelocity=abs(xVelocity)
    bounced=.true
  end
  if y>500 then do
    y=500
    yVelocity=-abs(yVelocity)
    bounced=.true
  end
  if y<0 then do
    y=0
    yVelocity=abs(yVelocity)
    bounced=.true
  end
  return bounced

::class BounceBenchmark
::method run
  random=.AwfyRandom~new
  balls=.array~new(100)
  do i=1 to 100
    balls[i]=.BounceBall~new(random)
  end
  bounces=0
  do step=1 to 50
    do ball over balls
      if ball~bounce then bounces=bounces+1
    end
  end
  return bounces
