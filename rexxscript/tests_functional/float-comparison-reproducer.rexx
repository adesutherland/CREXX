options levelg numeric_common
/* RS-F01 regression: both optimized and -n compilation must print 0.
 * The historical optimized result was 1; develop repaired constant transport. */
say 0.1 + 0.2 = 0.3
