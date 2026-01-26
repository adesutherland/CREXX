options levelb
/* Test Integer Path */
i = -10
i_res = POCABS(i)      /* Should infer INTEGER */

/* Test Float Path */
f = -10.5
f_res = POCABS(f)      /* Should infer FLOAT */

/* Test Recursion/Nesting */
nest = POCSQUARE(POCABS(i)) /* Should infer INTEGER */

say i_res f_res nest
