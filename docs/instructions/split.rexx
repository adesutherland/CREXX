loop i=1 while lines('mnem_cat.txt') > 0
parse linein('mnem_cat.txt') mnemonic '|' cat
say left(mnemonic,20) cat
end
