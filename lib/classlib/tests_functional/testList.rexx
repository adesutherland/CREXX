options levelb comments_dash
import data_List

l = .List()

rc = l.add("A")
rc = l.add("B")

say l.size()     -- 2
say l[2]         -- B

l[1] = "X"
say l.get(1)     -- X

say l.remove(1)  -- X
say l.isEmpty()  -- 0
