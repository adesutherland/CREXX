/* rexx */
options levelb

/* say trunc(3.1415926,0) */
/* say trunc(3.1415926,1) */
/* say trunc(3.1415926,2) */
/* say trunc(3.1415926,3) */
/* say trunc(3141.5926,12) */

/* return */

/* These from TRL */
say "Look for TRUNC OK"
if trunc(12.3) \= 12 then say 'failed in test 1 '
if trunc(127.09782,3) \= 127.097 then say 'failed in test 2 '
if trunc(127.1,3) \= 127.100 then say 'failed in test 3 '
/* if trunc(127.2) \= 127.00 then say 'failed in test 4 '  struck off after conferring with Walter */
/* These from Mark Hessling. */
if trunc(1234.5678, 2) \= '1234.56' then say 'failed in test 5 '
if trunc(-1234.5678) \= '-1234' then say 'failed in test 6 '
if trunc(.5678) \= '0' then say 'failed in test 7 '
if trunc(.00123) \= '0' then say 'failed in test 8 '
if trunc(.00123,4) \= '0.0012' then say 'failed in test 9 '
if trunc(.00127,4) \= '0.0012' then say 'failed in test 10 '
if trunc(.1678) \= '0' then say 'failed in test 11 '
if trunc(1234.5678) \= '1234' then say 'failed in test 12 '
if trunc(4.5678, 7) \= '4.5678000' then say 'failed in test 13 '
if trunc(10000005.0,2) \= 10000005.00 then say 'failed in test 14 '
if trunc(10000000.5,2) \= 10000000.50 then say 'failed in test 15 '
if trunc(10000000.05,2) \= 10000000.10 then say 'failed in test 16 '
if trunc(10000000.005,2) \= 10000000.00 then say 'failed in test 17 '
if trunc(10000005.5,2) \= 10000005.50 then say 'failed in test 18 '
if trunc(10000000.55,2) \= 10000000.60 then say 'failed in test 19 '
if trunc(10000000.055,2) \= 10000000.10 then say 'failed in test 20 '
if trunc(10000000.0055,2) \= 10000000.00 then say 'failed in test 21 '
if trunc(10000000.04,2) \= 10000000.00 then say 'failed in test 22 '
if trunc(10000000.045,2) \= 10000000.00 then say 'failed in test 23 '
if trunc(10000000.45,2) \= 10000000.50 then say 'failed in test 24 '
/* Duplicates of test 16
if trunc(10000000.05,2) \= 10000000.10 then say 'failed in test 25 '
if trunc(10000000.05,2) \= 10000000.10 then say 'failed in test 26 '
if trunc(10000000.05,2) \= 10000000.10 then say 'failed in test 27 '
*/
if trunc(99999999.,2) \= 99999999.00 then say 'failed in test 28 '
if trunc(99999999.9,2) \= 99999999.90 then say 'failed in test 29 '
if trunc(99999999.99,2) \= 100000000.00 then say 'failed in test 30 '
if trunc(1E2,0) \= 100 then say 'failed in test 31 '
if trunc(12E1,0) \= 120 then say 'failed in test 32 '
if trunc(123.,0) \= 123 then say 'failed in test 33 '
if trunc(123.1,0) \= 123 then say 'failed in test 34 '
if trunc(123.12,0) \= 123 then say 'failed in test 35 '
if trunc(123.123,0) \= 123 then say 'failed in test 36 '
if trunc(123.1234,0) \= 123 then say 'failed in test 37 '
if trunc(123.12345,0) \= 123 then say 'failed in test 38 '
if trunc(1E2,1) \= 100.0 then say 'failed in test 39 '
if trunc(12E1,1) \= 120.0 then say 'failed in test 40 '
if trunc(123.,1) \= 123.0 then say 'failed in test 41 '
if trunc(123.1,1) \= 123.1 then say 'failed in test 42 '
if trunc(123.12,1) \= 123.1 then say 'failed in test 43 '
if trunc(123.123,1) \= 123.1 then say 'failed in test 44 '
if trunc(123.1234,1) \= 123.1 then say 'failed in test 45 '
if trunc(123.12345,1) \= 123.1 then say 'failed in test 46 '
if trunc(1E2,2) \= 100.00 then say 'failed in test 47 '
if trunc(12E1,2) \= 120.00 then say 'failed in test 48 '
if trunc(123.,2) \= 123.00 then say 'failed in test 49 '
if trunc(123.1,2) \= 123.10 then say 'failed in test 50 '
if trunc(123.12,2) \= 123.12 then say 'failed in test 51 '
if trunc(123.123,2) \= 123.12 then say 'failed in test 52 '
if trunc(123.1234,2) \= 123.12 then say 'failed in test 53 '
if trunc(123.12345,2) \= 123.12 then say 'failed in test 54 '
if trunc(1E2,3) \= 100.000 then say 'failed in test 55 '
if trunc(12E1,3) \= 120.000 then say 'failed in test 56 '
if trunc(123.,3) \= 123.000 then say 'failed in test 57 '
if trunc(123.1,3) \= 123.100 then say 'failed in test 58 '
if trunc(123.12,3) \= 123.120 then say 'failed in test 59 '
if trunc(123.123,3) \= 123.123 then say 'failed in test 60 '
if trunc(123.1234,3) \= 123.123 then say 'failed in test 61 '
if trunc(123.12345,3) \= 123.123 then say 'failed in test 62 '
if trunc(1E2,4) \= 100.0000 then say 'failed in test 63 '
if trunc(12E1,4) \= 120.0000 then say 'failed in test 64 '
if trunc(123.,4) \= 123.0000 then say 'failed in test 65 '
if trunc(123.1,4) \= 123.1000 then say 'failed in test 66 '
if trunc(123.12,4) \= 123.1200 then say 'failed in test 67 '
if trunc(123.123,4) \= 123.1230 then say 'failed in test 68 '
if trunc(123.1234,4) \= 123.1234 then say 'failed in test 69 '
if trunc(123.12345,4) \= 123.1234 then say 'failed in test 70 '
if trunc(1E2,5) \= 100.00000 then say 'failed in test 71 '
if trunc(12E1,5) \= 120.00000 then say 'failed in test 72 '
if trunc(123.,5) \= 123.00000 then say 'failed in test 73 '
if trunc(123.1,5) \= 123.10000 then say 'failed in test 74 '
if trunc(123.12,5) \= 123.12000 then say 'failed in test 75 '
if trunc(123.123,5) \= 123.12300 then say 'failed in test 76 '
if trunc(123.1234,5) \= 123.12340 then say 'failed in test 77 '
if trunc(123.12345,5) \= 123.12345 then say 'failed in test 78 '
say "TRUNC OK"

/* trunc()  */
trunc: procedure = .string
  arg number = .float, fraction = 0
