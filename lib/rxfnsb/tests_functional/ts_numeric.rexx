/* Test Numeric Access Functions: digits(), fuzz(), form(), numcase(), and standard() */
options levelb
import rxfnsb

rc = test1()
if rc \= 0 then return rc
rc = test2()
return rc

/* Test 1 with one set of numeric options */
test1: procedure = .int
    numeric case upper
    numeric digits 6
    numeric fuzz 2
    numeric form engineering
    numeric standard classic

    if standard() \= "classic" then do
        say "Error: standard() expected classic but got" standard()
        return 1
    end
    if form() \= "engineering" then do
        say "Error: form() expected engineering but got" form()
        return 1
    end
    if numcase() \= "upper" then do
        say "Error: numcase() expected upper but got" numcase()
        return 1
    end
    if digits() \= 6 then do
        say "Error: digits() expected 6 but got" digits()
        return 1
    end
    if fuzz() \= 2 then do
        say "Error: fuzz() expected 2 but got" fuzz()
        return 1
    end
    return 0

/* Test 2 with another set of numeric options */
test2: procedure = .int
    numeric case lower
    numeric digits 10
    numeric fuzz 0
    numeric form scientific
    numeric standard common
    if standard() \= "common" then do
        say "Error: standard() expected common but got" standard()
        return 1
    end
    if form() \= "scientific" then do
        say "Error: form() expected scientific but got" form()
        return 1
    end
    if numcase() \= "lower" then do
        say "Error: numcase() expected lower but got" numcase()
        return 1
    end
    if digits() \= 10 then do
        say "Error: digits() expected 10 but got" digits()
        return 1
    end
    if fuzz() \= 0 then do
        say "Error: fuzz() expected 0 but got" fuzz()
        return 1
    end
    return 0
