options levelb
namespace test_global_subscopes expose x y arr

main: procedure
  arr = .string[10]
  x = 0
  y = 0
  
  call test_simple_no_subscope
  if y = 5 then say "test_simple_no_subscope OK"
  else say "test_simple_no_subscope FAIL: y="||y

  call test_simple
  if y = 10 then say "test_simple OK"
  else say "test_simple FAIL: y="||y

  call test_array
  if arr[1] = "changed" then say "test_array OK"
  else say "test_array FAIL: arr[1]="||arr[1]

  call test_nested
  if x = 99 then say "test_nested OK"
  else say "test_nested FAIL: x="||x

  call test_shadowing
  if x = 99 then say "test_shadowing OK" /* Should not be changed from 99 */
  else say "test_shadowing FAIL: x="||x
  if arr[5] = "changed again" then say "test_shadowing (array index) OK"
  else say "test_shadowing (array index) FAIL: arr[5]="||arr[5]
  
  return

test_simple_no_subscope: procedure
  /* y is exposed automatically via namespace */
  y = 5
  return

test_simple: procedure
  /* y is exposed automatically via namespace */
  do
    y = 10
  end
  return

test_array: procedure
  do
    arr[1] = "changed"
  end
  return

test_nested: procedure
  /* Modify the global x */
  do
    do
      x = 99
    end
  end
  return

test_shadowing: procedure
  /* Shadowing: the local typed x should not modify the global x */
  do
    x = .int
    x = 5
    arr[x] = "changed again"
  end
  return
