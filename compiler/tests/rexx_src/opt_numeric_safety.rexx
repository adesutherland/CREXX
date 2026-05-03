/* Options Level B */
options levelb

call ScenarioA
call ScenarioB
call ScenarioC
call ScenarioD
return

ScenarioA: procedure
    /* Safe to Fold: Explicit digits and standard */
    numeric digits 5
    numeric form scientific
    numeric standard common
    numeric case upper
    say 1.0d/3
    return

ScenarioB: procedure
    /* Unsafe: Inherited Digits */
    numeric digits inherited
    say 1.0d/3
    return

ScenarioC: procedure
    /* Unsafe: Inherited Standard */
    /* digits set to 9 to ensure digits is not the blocker */
    numeric digits 9
    numeric standard inherited
    say 5 % 2
    return

ScenarioD: procedure
    /* Unsafe: Inherited Fuzz */
    /* digits set to 9 to ensure digits is not the blocker */
    numeric digits 9
    numeric fuzz inherited
    if 1.000000001 = 1.0 then say "Equal"
    else say "Unequal"
    return
