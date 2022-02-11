/* rexx */
options levelb

/* say trunc(3.1415926,0) */
/* say trunc(3.1415926,1) */
/* say trunc(3.1415926,2) */
/* say trunc(3.1415926,3) */
/* say trunc(3141.5926,12) */

/* return */

/* These from TRL */
errors=0


if trunc(12.3) \= 12 then do
  errors=errors+1
  say 'TRUNC failed in test 1 '
end
if trunc(127.09782,3) \= 127.097 then do
  errors=errors+1
  say 'TRUNC failed in test 2 '
end
if trunc(127.1,3) \= 127.100 then do
  errors=errors+1
  say 'TRUNC failed in test 3 '
end
  /* 
end
if trunc(127.2) \= 127.00 then do
errors=errors+1
say 'TRUNC failed in test 4 '  struck off after conferring with Walter */
/* These from Mark Hessling. */


if trunc(1234.5678, 2) \= '1234.56' then do
  errors=errors+1
  say 'TRUNC failed in test 5 '
end
if trunc(-1234.5678) \= '-1234' then do
  errors=errors+1
  say 'TRUNC failed in test 6 '
end
if trunc(.5678) \= '0' then do
  errors=errors+1
  say 'TRUNC failed in test 7 '
end
if trunc(.00123) \= '0' then do
  errors=errors+1
  say 'TRUNC failed in test 8 '
end
if trunc(.00123,4) \= '0.0012' then do
  errors=errors+1
  say 'TRUNC failed in test 9 '
end
if trunc(.00127,4) \= '0.0012' then do
  errors=errors+1
  say 'TRUNC failed in test 10 '
end
if trunc(.1678) \= '0' then do
  errors=errors+1
  say 'TRUNC failed in test 11 '
end
if trunc(1234.5678) \= '1234' then do
  errors=errors+1
  say 'TRUNC failed in test 12 '
end
if trunc(4.5678, 7) \= '4.5678000' then do
  errors=errors+1
  say 'TRUNC failed in test 13 '
end
if trunc(10000005.0,2) \= 10000005.00 then do
  errors=errors+1
  say 'TRUNC failed in test 14 '
end
if trunc(10000000.5,2) \= 10000000.50 then do
  errors=errors+1
  say 'TRUNC failed in test 15 '
end
if trunc(10000000.05,2) \= 10000000.10 then do
  errors=errors+1
  say 'TRUNC failed in test 16 '
end
if trunc(10000000.005,2) \= 10000000.00 then do
  errors=errors+1
  say 'TRUNC failed in test 17 '
end
if trunc(10000005.5,2) \= 10000005.50 then do
  errors=errors+1
  say 'TRUNC failed in test 18 '
end
if trunc(10000000.55,2) \= 10000000.60 then do
  errors=errors+1
  say 'TRUNC failed in test 19 '
end
if trunc(10000000.055,2) \= 10000000.10 then do
  errors=errors+1
  say 'TRUNC failed in test 20 '
end
if trunc(10000000.0055,2) \= 10000000.00 then do
  errors=errors+1
  say 'TRUNC failed in test 21 '
end
if trunc(10000000.04,2) \= 10000000.00 then do
  errors=errors+1
  say 'TRUNC failed in test 22 '
end
if trunc(10000000.045,2) \= 10000000.00 then do
  errors=errors+1
  say 'TRUNC failed in test 23 '
end
if trunc(10000000.45,2) \= 10000000.50 then do
  errors=errors+1
  say 'TRUNC failed in test 24 '
end
/* Duplicates of test 16
   
   end
   if trunc(10000000.05,2) \= 10000000.10 then do
   errors=errors+1
   say 'TRUNC failed in test 25 '
   
   end
   if trunc(10000000.05,2) \= 10000000.10 then do
   errors=errors+1
   say 'TRUNC failed in test 26 '
   
   end
   if trunc(10000000.05,2) \= 10000000.10 then do
   errors=errors+1
   say 'TRUNC failed in test 27 '
 */


if trunc(99999999.,2) \= 99999999.00 then do
  errors=errors+1
  say 'TRUNC failed in test 28 '
end
if trunc(99999999.9,2) \= 99999999.90 then do
  errors=errors+1
  say 'TRUNC failed in test 29 '
end
if trunc(99999999.99,2) \= 100000000.00 then do
  errors=errors+1
  say 'TRUNC failed in test 30 '
end
if trunc(1E2,0) \= 100 then do
  errors=errors+1
  say 'TRUNC failed in test 31 '
end
if trunc(12E1,0) \= 120 then do
  errors=errors+1
  say 'TRUNC failed in test 32 '
end
if trunc(123.,0) \= 123 then do
  errors=errors+1
  say 'TRUNC failed in test 33 '
end
if trunc(123.1,0) \= 123 then do
  errors=errors+1
  say 'TRUNC failed in test 34 '
end
if trunc(123.12,0) \= 123 then do
  errors=errors+1
  say 'TRUNC failed in test 35 '
end
if trunc(123.123,0) \= 123 then do
  errors=errors+1
  say 'TRUNC failed in test 36 '
end
if trunc(123.1234,0) \= 123 then do
  errors=errors+1
  say 'TRUNC failed in test 37 '
end
if trunc(123.12345,0) \= 123 then do
  errors=errors+1
  say 'TRUNC failed in test 38 '
end
if trunc(1E2,1) \= 100.0 then do
  errors=errors+1
  say 'TRUNC failed in test 39 '
end
if trunc(12E1,1) \= 120.0 then do
  errors=errors+1
  say 'TRUNC failed in test 40 '
end
if trunc(123.,1) \= 123.0 then do
  errors=errors+1
  say 'TRUNC failed in test 41 '
end
if trunc(123.1,1) \= 123.1 then do
  errors=errors+1
  say 'TRUNC failed in test 42 '
end
if trunc(123.12,1) \= 123.1 then do
  errors=errors+1
  say 'TRUNC failed in test 43 '
end
if trunc(123.123,1) \= 123.1 then do
  errors=errors+1
  say 'TRUNC failed in test 44 '
end
if trunc(123.1234,1) \= 123.1 then do
  errors=errors+1
  say 'TRUNC failed in test 45 '
end
if trunc(123.12345,1) \= 123.1 then do
  errors=errors+1
  say 'TRUNC failed in test 46 '
end
if trunc(1E2,2) \= 100.00 then do
  errors=errors+1
  say 'TRUNC failed in test 47 '
end
if trunc(12E1,2) \= 120.00 then do
  errors=errors+1
  say 'TRUNC failed in test 48 '
end
if trunc(123.,2) \= 123.00 then do
  errors=errors+1
  say 'TRUNC failed in test 49 '
end
if trunc(123.1,2) \= 123.10 then do
  errors=errors+1
  say 'TRUNC failed in test 50 '
end
if trunc(123.12,2) \= 123.12 then do
  errors=errors+1
  say 'TRUNC failed in test 51 '
end
if trunc(123.123,2) \= 123.12 then do
  errors=errors+1
  say 'TRUNC failed in test 52 '
end
if trunc(123.1234,2) \= 123.12 then do
  errors=errors+1
  say 'TRUNC failed in test 53 '
end
if trunc(123.12345,2) \= 123.12 then do
  errors=errors+1
  say 'TRUNC failed in test 54 '
end
if trunc(1E2,3) \= 100.000 then do
  errors=errors+1
  say 'TRUNC failed in test 55 '
end
if trunc(12E1,3) \= 120.000 then do
  errors=errors+1
  say 'TRUNC failed in test 56 '
end
if trunc(123.,3) \= 123.000 then do
  errors=errors+1
  say 'TRUNC failed in test 57 '
end
if trunc(123.1,3) \= 123.100 then do
  errors=errors+1
  say 'TRUNC failed in test 58 '
end
if trunc(123.12,3) \= 123.120 then do
  errors=errors+1
  say 'TRUNC failed in test 59 '
end
if trunc(123.123,3) \= 123.123 then do
  errors=errors+1
  say 'TRUNC failed in test 60 '
end
if trunc(123.1234,3) \= 123.123 then do
  errors=errors+1
  say 'TRUNC failed in test 61 '
end
if trunc(123.12345,3) \= 123.123 then do
  errors=errors+1
  say 'TRUNC failed in test 62 '
end
if trunc(1E2,4) \= 100.0000 then do
  errors=errors+1
  say 'TRUNC failed in test 63 '
end
if trunc(12E1,4) \= 120.0000 then do
  errors=errors+1
  say 'TRUNC failed in test 64 '
end
if trunc(123.,4) \= 123.0000 then do
  errors=errors+1
  say 'TRUNC failed in test 65 '
end
if trunc(123.1,4) \= 123.1000 then do
  errors=errors+1
  say 'TRUNC failed in test 66 '
end
if trunc(123.12,4) \= 123.1200 then do
  errors=errors+1
  say 'TRUNC failed in test 67 '
end
if trunc(123.123,4) \= 123.1230 then do
  errors=errors+1
  say 'TRUNC failed in test 68 '
end
if trunc(123.1234,4) \= 123.1234 then do
  errors=errors+1
  say 'TRUNC failed in test 69 '
end
if trunc(123.12345,4) \= 123.1234 then do
  errors=errors+1
  say 'TRUNC failed in test 70 '
end
if trunc(1E2,5) \= 100.00000 then do
  errors=errors+1
  say 'TRUNC failed in test 71 '
end
if trunc(12E1,5) \= 120.00000 then do
  errors=errors+1
  say 'TRUNC failed in test 72 '
end
if trunc(123.,5) \= 123.00000 then do
  errors=errors+1
  say 'TRUNC failed in test 73 '
end
if trunc(123.1,5) \= 123.10000 then do
  errors=errors+1
  say 'TRUNC failed in test 74 '
end
if trunc(123.12,5) \= 123.12000 then do
  errors=errors+1
  say 'TRUNC failed in test 75 '
end
if trunc(123.123,5) \= 123.12300 then do
  errors=errors+1
  say 'TRUNC failed in test 76 '
end
if trunc(123.1234,5) \= 123.12340 then do
  errors=errors+1
  say 'TRUNC failed in test 77 '
end
if trunc(123.12345,5) \= 123.12345 then do
  errors=errors+1
  say 'TRUNC failed in test 78 '
end
  /* trunc()  */
trunc: procedure = .string
arg number = .string, fraction = 0
