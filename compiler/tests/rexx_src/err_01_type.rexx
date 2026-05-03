options levelb
/* Type Violation: Changing type after assignment is illegal [Ref: Language Manual 573] */
i = 10        /* Implicitly .int */
i = "String"  /* Error: Type mismatch */
