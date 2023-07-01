ostem=''
address system 'cat insert.sql' with output stem ostem
do i=1 to ostem[0]
  parse ostem[i] start ",'" name "'" rest
  say start ",'"name.strip()"'" rest
end
