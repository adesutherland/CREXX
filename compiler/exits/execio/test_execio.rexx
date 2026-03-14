options levelb
import rxfnsb

main: procedure
/* Write to a temp file first to ensure it exists */
out_stem = .string[]
out_stem[1] = "Line 1: Hello from Execio"
out_stem[2] = "Line 2: Testing the exit"
out_stem[3] = "Line 3: Goodbye"
EXECIO 3 DISKW 'test_execio_temp.txt' (STEM out_stem FINIS

/* Notice: We DO NOT declare mystem here!
 * It will be hoisted automatically by the Execio pre_process hook.
 */
EXECIO * DISKR 'test_execio_temp.txt' (STEM mystem FINIS

/* mystem should now be hoisted and typed properly */
say "Read back from file:"
say "1: " mystem[1]
say "2: " mystem[2]
say "3: " mystem[3]
return


