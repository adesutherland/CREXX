options levelb comments_dash
import rexx
import rxfnsb

s = .rexx('aap noot mies wim zus jet')
say s
say s.substr(1,2)
say 'end of testrexx'

/*
 * do we have interference from the real bif?)
 */

say '--- testing real bif' 
r = aap
say substr(r,1,3)

say '--- testing abs'
t = .rexx('-4')
say t.abs()

say '--- testing c2d'
say s.c2d()

say '--- testing c2x'
say s.c2x()
