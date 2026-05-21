options levelb
import array_class_lib

say "Start"
say "Main"
it1 = .Item("Hello")
say "Created it1"
ary = .Item[1]
say "Created ary"
ary[1] = it1
say "Assigned ary[1]"
say "Calling process"
say process(ary)
say "Done"
return 0

process: procedure = .string
    arg items = .Item[]
    say "Inside process, Items.0 =" items.0
    if items.0 < 1 then return ""

    it = items.1
    val = it.get_val()

    do i = 1 to items.0
        it_i = items[i]
        val_i = it_i.get_val()
        say val_i
    end
    return val
