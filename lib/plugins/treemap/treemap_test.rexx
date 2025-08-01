/* Treemap Plugin Test */
options levelb
import treemap
import rxfnsb

map=tmcreate("Fred")         ## create a treemap
mary=tmcreate("Mary")        ## create a treemap
mike=tmcreate("Mike")        ## create a treemap

say "Lookup Fred "tmlookup('FReD') map
say "Lookup Mary "tmlookup('marY') mary
say "Lookup Mike "tmlookup('mikE') mike
say "Lookup Walter "tmlookup('WALter') "<- not present"

say 'MAPID 'map
say "Adding Key/Value pairs to the tree"
say "----------------------------------"
say 'TMPUT 'tmput(map,"London","United Kingdom")   ## Put key record into treemap
say 'TMPUT 'tmput(map,"London","United Kingdom")   ## Put key record into treemap
say 'TMPUT 'tmput(map,"Den Haag","Netherlands")    ## Put key record into treemap
say 'TMPUT 'tmput(map,"Berlin","Germany")          ## Put key record into treemap
say " "
say "Number of entries of the tree "tmsize(map)
say "--------------------------------"

say "Retrieve Value via Key of the tree"
say "----------------------------------"
say 'TMGET 'tmget(map,"London")        ## Retrieve record via key from treemap
say 'TMGET 'tmget(map,"Den Haag")      ## Retrieve record via key from treemap
say 'TMGET 'tmget(map,"Berlin")        ## Retrieve record via key from treemap
say "First and Last Key of the tree"
say "------------------------------"
say 'FIRST 'tmfirstkey(map)       ## first key of treemap
say 'LAST  'tmlastkey(map)        ## last key of treemap
say "is Key part of the tree"
say "-----------------------"
say 'HASKEY "'tmhaskey(map,"Paris")'"'    ## is key available in treemap
say 'HASKEY "'tmhaskey(map,"London")'"'   ## is key available in treemap
say "is Value already define in the tree"
say "-----------------------------------"
say 'HASVALUE Japan? "'tmhasvalue(map,"Japan")'"'        ## is value available in treemap
say 'HASVALUE Netherlands? "'tmhasvalue(map,"Netherlands")'"' ## is value available in treemap

say "Fetch all Keys of the tree"
say "--------------------------"
keys.1=''              ## init key stem
say tmkeys(map,keys)   ## fetch all keys
do i=1 to keys.0       ## report all keys
   say i keys.i
end
say "Fetch all Keys/Value pairs of the tree"
say "--------------------------------------"
keys.1=''
values.1=''
say tmdump(map,keys,values)
do i=1 to keys.0
   say keys.i' --> 'values.i
end

say "Drop Key from tree"
say "------------------"
say tmremove(map,'Berlin')
say tmremove(map,'Cairo')
say " "
say "Number of entries of the tree "tmsize(map)
say "--------------------------------"
say "Fetch all Keys/Value pairs of the tree"
say "--------------------------------------"
keys.1=''
values.1=''
say tmdump(map,keys,values)
do i=1 to keys.0
   say keys.i' --> 'values.i
end

say "Create 2. Map"
say "-------------"
map2=tmcreate()         ## create a treemap
say TMput(map2,"Fred","Flintstone")
say TMget(map2,"Fred")

say "Release trees"
say "-------------"
say tmfree(map)
say tmfree(map2)

say "Use invalid Tree"
say "----------------"
say TMput(1234,"Fred","Flintstone")
exit






