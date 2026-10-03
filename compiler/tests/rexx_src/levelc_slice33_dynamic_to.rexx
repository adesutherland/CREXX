options levelc
limit=3
do i=1 to limit
  say 'fixed' i limit
  limit=1
end i
say 'after-fixed' i limit
old=10
calls=0
do old=2 to endpoint() for 0
  say 'bad-zero'
end old
say 'after-zero' old calls
lower=1
do down=3 to lower by -1
  say 'down' down lower
  lower=9
end down
say 'after-down' down lower
upper=3
do checked=1 to upper while checked<3
  say 'while' checked
end checked
say 'after-while' checked
upper=3
do after=1 to upper until after=2
  say 'until' after
end after
say 'after-until' after
upper=3
do outer=1 to upper
  if outer>6 then exit
  do inner=1 to 2
    if inner=1 then iterate OuTeR
  end inner
end outer
say 'after-outer' outer
exit
endpoint: procedure expose old calls
calls=calls+1
say 'endpoint' old
return 4
