/* Treemap Plugin Test */
options levelb
import treemap
import rxfnsb

map=tmcreate()         ## create a treemap
say 'MAPID 'map
say "Adding Key/Value pairs to the tree"
say "----------------------------------"
say 'TMPUT 'tmput(map,"London","United Kingdom")   ## Put key record into treemap
say 'TMPUT 'tmput(map,"Den Haag","Netherlands")    ## Put key record into treemap
say 'TMPUT 'tmput(map,"Berlin","Germany")          ## Put key record into treemap
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
say 'HASKEY 'tmhaskey(map,"Paris")        ## is key available in treemap
say 'HASKEY 'tmhaskey(map,"London")       ## is key available in treemap
say "is Value already define in the tree"
say "-----------------------------------"
say 'HASVALUE 'tmhasvalue(map,"Japan")        ## is value available in treemap
say 'HASVALUE 'tmhasvalue(map,"Netherlands")  ## is value available in treemap

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
say "Fetch all Keys/Value pairs of the tree"
say "--------------------------------------"
keys.1=''
values.1=''
say tmdump(map,keys,values)
do i=1 to keys.0
   say keys.i' --> 'values.i
end

say "Release tree"
say "------------"
say tmfree(map)
exit






