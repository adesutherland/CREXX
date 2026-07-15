/* Tiny deterministic source for compile/load/first-result lifecycle timing. */
left=0
right=1
do index=1 to 20
  next=left+right
  left=right
  right=next
end
if left<>6765 then do
  say 'FAIL: expected 6765, got' left
  exit 1
end
say 'result='left
say 'PASS: Lifecycle Probe ooRexx'
exit 0
