options levelc

a = 8
b = 3
say a + b
say a - b
say a * b
say 8 / 2
say a % b
say a // b
say 2 ** 3
say -"5"
say +"6"
say "a" || "b" "c"
say 9 = "09"
say "a" = "a "
say "a" == "a "
say a > b
say 2 < 1
say \0
say 1 && 0
flag = 0
say 0 & mark()
say flag
say 1 | mark()
say flag
say 1 & mark()
say flag
say 0 | (substr("abc", 1, 1) = "a")
say 1 & (substr("abc", 1, 1) = "a")
say length("a" || "bc")
exit

mark:
procedure expose flag
flag = flag + 1
return 1
