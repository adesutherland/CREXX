/* rexx linesize bif */
options levelb
/*
 * here mainly because it needs a native implementation on z/VM
 * and probably other OS
 */  

linesize: procedure = .int
  arg expose string1 = .string
  result = 0
  return result
