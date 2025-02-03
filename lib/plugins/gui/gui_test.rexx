/* GETPI Plugin Test */
options levelb
import gui
import rxfnsb

call init_window "Pandora's Box",350,400
b1=add_button("Should I", 50, 50)
b2=add_button(  "Shouldn't I", 150, 50)
b3=add_button(  "Open It", 100, 100)
t1=add_text("Open Pandora's box, and you'll be troubleshooting forever", 10, 10)
item.1='Fred'
item.2='Mary'
item.3='Mike'
b5=add_combo(item, 170, 150)

say "edit "add_edit(200,200,"default")

say b1 b2 b3 b4 b5

list = add_list(10,200, 200, 150)  /* x, y, width, height */
say "List "list

rc=list_header(list,"My Item List","gold","black")

do i=1 to 10 by 2
   rc=list_add_item(list, "Item "i,"gold")   ## "darkgoldenrod")
   rc=list_add_item(list, "Item "i+1,"deepskyblue")   ## "darkgoldenrod")
end

/* In your event loop, check for LIST_SELECTED_TOKEN (-3) */

call show_window
say time('l')" SHOW"
/* Main loop - can do other things while window is open */
j=0
DO forever
    /* Process GTK events */
    rc=process_events(250)
    say time('l') rc
    if rc= list then do
       selected = list_get_selected(list)
       say 'List Select 'list selected
       selectedT = list_get_item(list)
       say 'List Select 'list selectedT
    end
    if rc<0 then leave
END

/*
Common X11 Colors
Basic Colors
black
white
red
green
blue
yellow
cyan
magenta
Shades of Gray
gray
lightgray
darkgray
dimgray
slategray
light slate gray
Reds
lightcoral
salmon
darksalmon
crimson
firebrick
indianred
Greens
lightgreen
mediumseagreen
seagreen
forestgreen
darkgreen
limegreen
Blues
lightblue
skyblue
deepskyblue
dodgerblue
steelblue
royalblue
navy
Yellows
lightyellow
khaki
gold
goldenrod
Purples
plum
violet
orchid
purple
mediumpurple
darkviolet
Others
orange
tan
peachpuff
lightpink
hotpink
palevioletred
*/