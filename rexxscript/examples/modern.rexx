/* RexxScript 1: Level G expressions and Level C functions. */
subtotal = 0
do i = 1 to 5
  if i = 3 then iterate
  subtotal = subtotal + i ** 2
end
say 'total:' subtotal
say substr('naïve café',7)
if subtotal = 46 then say 'ok' else say 'wrong'
return subtotal
