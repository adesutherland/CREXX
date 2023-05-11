/* Count lines demo */
options levelb
import rxfnsb

/*
 * Expecting (e.g.) "countlines .rexx .rxas"
 * This should count the total lines of files in the directory with names containing *.rexx and *.rxas and so on!
 */
arg searches = .string[]

/* Add a default search if none are given */
if searches.0 < 1 then searches.2 = ".rexx"

/* Sort the search strings - just because we can! */
ordered_searches = .string[]
address cmd "sort" input searches output ordered_searches
if rc <> 0 then say "RC = "rc "when doing sort"

result = .int[]

/* Count the files */
do i = 1 to ordered_searches.0
    result.i = do_search(ordered_searches[i])
end

/* Print Results */
call print_results ordered_searches, result

/* That's all folks! */

print_results: procedure
    arg name = .string[], count = .int[]
    total = 0

    do i = 1 to name.0 /* name.1 is the program name argument */
        Say "Search" name.i "has" count.i "lines"
        total = total + count.i
    end

    say "Grant total is" total

    return

/* Searches the directory for files and counts the lines */
do_search: procedure = .int
    arg search = .string

    files = .string[]
    lines_in_files = 0;

    /* Environments don't do anything yet ... */
    address cmd "ls" output files
    if rc <> 0 then say "RC = "rc "when doing :" "ls" search

    /* Count the lines in each file */
    do i = 1 to files.0
        if right(files.i,length(search)) = search then do
            do while lines(files.i)
                call linein files.i
                lines_in_files = lines_in_files + 1
            end
            /* Close file */
            call lineout files.i
        end
    end

    return lines_in_files
