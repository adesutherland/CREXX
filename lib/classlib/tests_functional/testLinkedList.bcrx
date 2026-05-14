options levelb comments_dash
import data_LinkedList

l = .LinkedList()

rc = l.append("aap")
rc = l.append("noot")
rc = l.prepend("mies")

-- say l.toString()         -- {mies, aap, noot}

rc = l.setFirst()
say l.current()          -- mies
say l.next()             -- aap
say l.next()             -- noot
say l.next()             -- ""

rc = l.insertBeforeCurrent("jet")
-- say l.toString()

rc = l.clear()

k = .LinkedList()
rc = k.append("wim")
rc = k.append("zus")
rc = k.prepend("jet")

rc = k.setFirst()
say k.current()          -- mies
say k.next()             -- aap
say k.next()             -- noot
say k.next()             -- ""

call k.debug()

x = .string[]
x = k.toArray()
say 'size of k:' x[0]

say x[1]
say x[2]
say x.3

say k.size()

say k.toString()
