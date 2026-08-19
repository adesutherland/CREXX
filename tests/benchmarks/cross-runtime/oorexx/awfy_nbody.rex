/*
 * ooRexx decimal/native-math port of the Are We Fast Yet? NBody benchmark.
 * Derived from upstream commit 74306fec151070fd07157cefeacf19e7e0bcdc89;
 * see ../../THIRD_PARTY_NOTICES.md. RxCalcSqrt is the disclosed ooRexx
 * native-math substrate.
 */
numeric digits 18
parse arg steps
if steps='' then steps=1
if steps<1 then call fail 'steps must be positive'
if steps<>1 & steps<>250000 then call fail 'no reference energy for steps' steps

system=.NBodySystem~new
do iteration=1 to steps
  system~advance(0.01)
end
energy=system~energy

expected=-0.16907495402506745
if steps=250000 then expected=-0.1690859889909308
if abs(energy-expected)>0.0000000000001 then -
  call fail 'expected' expected', got' energy

say 'benchmark=awfy_nbody steps='steps 'energy='energy
say 'PASS: AWFY NBody ooRexx decimal/native-math port'
exit 0

fail: procedure
  parse arg message
  say 'FAIL:' message
  exit 1

::class NBodyBody
::attribute x
::attribute y
::attribute z
::attribute vx
::attribute vy
::attribute vz
::attribute mass

::method init
  expose x y z vx vy vz mass
  numeric digits 18
  use strict arg initialX,initialY,initialZ,initialVX,initialVY,initialVZ,initialMass
  daysPerYear=365.24
  solarMass=39.478417604357434
  x=initialX
  y=initialY
  z=initialZ
  vx=initialVX*daysPerYear
  vy=initialVY*daysPerYear
  vz=initialVZ*daysPerYear
  mass=initialMass*solarMass

::method offsetMomentum
  expose vx vy vz
  numeric digits 18
  use strict arg px,py,pz
  solarMass=39.478417604357434
  vx=0-(px/solarMass)
  vy=0-(py/solarMass)
  vz=0-(pz/solarMass)

::class NBodySystem
::method init
  expose bodies
  numeric digits 18
  bodies=.array~of( -
    .NBodyBody~new(0,0,0,0,0,0,1), -
    .NBodyBody~new(4.84143144246472090,-1.16032004402742839, -
      -0.103622044471123139,0.00166007664274403694, -
      0.00769901118419740425,-0.0000690460016972063023, -
      0.000954791938424326609), -
    .NBodyBody~new(8.34336671824457987,4.12479856412430479, -
      -0.403523417114321381,-0.00276742510726862411, -
      0.00499852801234917238,0.0000230417297573763929, -
      0.000285885980666130812), -
    .NBodyBody~new(12.8943695621391310,-15.1111514016986312, -
      -0.223307578892655734,0.00296460137564761618, -
      0.00237847173959480950,-0.0000296589568540237556, -
      0.0000436624404335156298), -
    .NBodyBody~new(15.3796971148509165,-25.9193146099879641, -
      0.179258772950371181,0.00268067772490389322, -
      0.00162824170038242295,-0.0000951592254519715870, -
      0.0000515138902046611451))

  px=0
  py=0
  pz=0
  do body over bodies
    px=px+body~vx*body~mass
    py=py+body~vy*body~mass
    pz=pz+body~vz*body~mass
  end
  bodies[1]~offsetMomentum(px,py,pz)

::method advance
  expose bodies
  numeric digits 18
  use strict arg dt
  bodyCount=bodies~items
  do i=1 to bodyCount
    iBody=bodies[i]
    do j=i+1 to bodyCount
      jBody=bodies[j]
      dx=iBody~x-jBody~x
      dy=iBody~y-jBody~y
      dz=iBody~z-jBody~z
      squared=dx*dx+dy*dy+dz*dz
      distance=sqrt(squared)
      magnitude=dt/(squared*distance)

      iBody~vx=iBody~vx-dx*jBody~mass*magnitude
      iBody~vy=iBody~vy-dy*jBody~mass*magnitude
      iBody~vz=iBody~vz-dz*jBody~mass*magnitude
      jBody~vx=jBody~vx+dx*iBody~mass*magnitude
      jBody~vy=jBody~vy+dy*iBody~mass*magnitude
      jBody~vz=jBody~vz+dz*iBody~mass*magnitude
    end
  end

  do body over bodies
    body~x=body~x+dt*body~vx
    body~y=body~y+dt*body~vy
    body~z=body~z+dt*body~vz
  end

::method energy
  expose bodies
  numeric digits 18
  total=0
  bodyCount=bodies~items
  do i=1 to bodyCount
    iBody=bodies[i]
    total=total+0.5*iBody~mass*(iBody~vx*iBody~vx+ -
      iBody~vy*iBody~vy+iBody~vz*iBody~vz)
    do j=i+1 to bodyCount
      jBody=bodies[j]
      dx=iBody~x-jBody~x
      dy=iBody~y-jBody~y
      dz=iBody~z-jBody~z
      distance=sqrt(dx*dx+dy*dy+dz*dz)
      total=total-(iBody~mass*jBody~mass)/distance
    end
  end
  return total

::routine sqrt public external "library rxmath RxCalcSqrt"
