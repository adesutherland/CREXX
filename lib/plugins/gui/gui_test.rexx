/* GUI Sample */
options levelb
import gui
import pick
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

/* Initialize the GUI */
call init_window "Sample Application", 400, 300

/* Add a label */
label_index = add_text("Click the button!", 20, 20)

/* Add a button */
button_index = add_button("Click Me", 20, 60)

/* Add a combo box with options */
combo_index = add_combo(["Option 1", "Option 2", "Option 3"], 20, 100)

/* Add a list to display selected items */
list_index = add_list(20, 150, 200, 100)

/* Show the window */
call show_window

/* Main event loop */
do forever
    event_token = process_events(500)  /* Wait for events with a timeout of 500ms */
    
    if event_token = button_index then do
        /* Update label text when button is clicked */
        call set_text label_index, "Button clicked!"
    end
    
    if event_token = combo_index then do
        /* Get selected item from combo box */
        selected_index = get_combo_index(combo_index)
        selected_text = "Selected: " || selected_index
        
        /* Add selected item to the list */
        call list_add_item list_index, selected_text, "lightyellow"
    end
    
    if event_token = 0 then leave  /* Exit loop on timeout */
end

/* Clean up the GUI */
call cleanup_gui