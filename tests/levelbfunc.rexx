/* Level B Function Test */
options levelb

result = 0
data = 5
assembler
do
end
say xxx
assembler
    time result
assembler
do
    iadd result,result,data
    time result
    1
    load result,5
    say "hello"
end

/*
*/
say result
/*
say time(1)
say ("AAA") (1+2)
*/