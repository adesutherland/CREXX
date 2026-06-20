/* rexx */
books = 'crexx_language_reference crexx_programming_guide crexx_vm_spec crexx_library_reference'
loop while books <> ''
parse var books title books

'cd ~/apps/crexx-fork/docs/books/'title
call build_pdf title
end

exit

build_pdf: procedure
parse arg title
'mkdir -p tex/book'
'cd tex/book'
'rexx ~/apps/TextTools/build.rexx -copy'
return
