/* Extracted core ZIP proof: native Classic transport and codepage support. */
options levelc
file = 'classic-stream.bin'
if stream(file,'C','OPEN WRITE REPLACE ENCODING Windows-1252') \== 'READY:' then exit 1
if charout(file,'€é') \== 0 then exit 2
if stream(file,'C','CLOSE') \== 'READY:' then exit 3
if stream(file,'C','OPEN READ ENCODING Windows-1252') \== 'READY:' then exit 4
text = charin(file,1,2)
if stream(file,'C','CLOSE') \== 'READY:' then exit 5
if text \== '€é' then exit 6
say 'PASS: packaged Classic encoded stream'
exit 0
