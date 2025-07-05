options levelb
import gui
import rxfnsb
j=1
xmax=10

/* Define functions */
j=1
xmax=10
do i=-xmax to xmax
   x.j=i/10
   y.j=x.j*x.j    /* Quadratic function - visible */
   j=j+1
end

j=1
do i=-xmax to xmax
   a.j=i/10
   b.j=a.j+0.1        /* Linear function - initially invisible */
   j=j+1
end

j=1
do i=-xmax to xmax
   k.j=i/10
   l.j=-k.j+2        /* Linear function - initially invisible */
   j=j+1
end

/*
a.1=0.0
b.1=0.0
k.1=0.0
l.1=0.0
*/
j=1
do i=-xmax to xmax
   m.j=i/10
   n.j=1/(m.j)        /* Linear function - initially invisible */
   say m.j n.j
   if m.j=0 then iterate
   j=j+1
end
xx.1=0.0
xy.1=0.0
zx.1=0.0
zy.1=0.0

prices.1 = 100.00
prices.2 = 102.33
prices.3 = 104.13
prices.4 = 106.78
prices.5 = 107.64
prices.6 = 110.29
prices.7 = 111.56
prices.8 = 113.12
prices.9 = 115.84
prices.10 = 117.91
prices.11 = 120.28
prices.12 = 122.11
prices.13 = 123.56
prices.14 = 125.82
prices.15 = 127.16
prices.16 = 129.45
prices.17 = 131.32
prices.18 = 133.59
prices.19 = 135.78
prices.20 = 137.02
prices.21 = 139.46
prices.22 = 141.23
prices.23 = 143.98
prices.24 = 145.56
prices.25 = 148.03
prices.26 = 150.25
prices.27 = 152.41
prices.28 = 154.79
prices.29 = 157.02
prices.30 = 159.46
prices.31 = 161.35
prices.32 = 163.61
prices.33 = 165.78
prices.34 = 167.92
prices.35 = 169.49
prices.36 = 171.63
prices.37 = 173.77
prices.38 = 175.24
prices.39 = 177.88
prices.40 = 179.62
prices.41 = 181.42
prices.42 = 183.31
prices.43 = 185.47
prices.44 = 187.55
prices.45 = 189.91
prices.46 = 192.03
prices.47 = 194.16
prices.48 = 196.33
prices.49 = 198.78
prices.50 = 200.89
do i=1 to 50
   pi.i=i+0.0
end

/* Initialize the GUI */
call init_window "Mathematical Functions Display", 1200, 600
/* Add title labels */
call add_text "Quadratic Function (y = x²)", 10, 10
/* Add graph with both functions, but second one invisible */
quad_graph = add_graph(10, 30, 570, 550, x, y,a,b,k,l)          /* Both functions */
sec_graph = add_r2chart(590, 30, 570, 550, pi, prices,a,b,k,l)   /* Both functions */

call update_graph sec_graph,11,1,3,"gold"
call update_graph sec_graph,2,1,3,"yellow"
call update_graph quad_graph,1,1,3,"red"
call update_graph quad_graph,2,1,3,"blue"
call update_graph quad_graph,3,2,3,"green"
call update_graph quad_graph,-1,0,0,""
call update_graph sec_graph,-1,0,0,""


/* Show the window */
call show_window -1,-1

/* Process events with better debug output */
do forever
    say "Processing events" i
    event = process_events(1000)
    if event < 0 then leave
end

exit 0





home="C:\Users\PeterJ\CLionProjects\CREXX\CREXX\f0049B\"
pluginhome=home"\lib\plugins\"
/* ----------------------------------------------------------------------------------------
 * Initialize splash screen and store return value
 * ----------------------------------------------------------------------------------------
 */
  splash_rc = splash_pick("Let's do the Time Warp again!", ,
                       "it's a jump to the left and then a step to the right", ,
                       3, 500,500, ,
                       "CREXX.png")
/* Initialize the GUI */
  call init_window "Process CREXX File", 600, 430
/* Add a label for the input field */
  label_index = add_text("Select a CREXX File ", 10, 10)
/* Add an entry field for input */
  input_field_index = add_edit(10, 30,515)
/* Add a small button next to the input field */
  select = add_button("Dir", 530, 30)
  update = add_button("Edit  ", 530,70)
/* Add checkboxes */
  checkbox1_index = add_checkbox("Compile", 10, 70)
  checkbox2_index = add_checkbox("Assembly 2", 10, 90)
  checkbox3_index = add_checkbox("Run", 10, 110)
  submit = add_button("Submit", 10, 140)
  message_area = add_message_area(10, 190, 580, 200)  /* x, y, width, height */
  status_bar = add_status_bar()
/* Show the window */
  call show_window -1,-1
## hidemsg=hide_widget(submit)
  call set_sensitive submit,0
  call set_sensitive update,0
/* ----------------------------------------------------------------------------------------
 * Event Handler
 * ----------------------------------------------------------------------------------------
 */
do forever
    event_token = process_events(500)  /* Wait for events with a timeout of 500ms */
    if event_token < 0 then leave      /* Exit loop on timeout */
    else if event_token = select then do
       input_rexx = file_pick('Select REXX File', pluginhome,0,"*.rexx")
       if input_rexx <> "" then do     /* Check if the input text is not empty */
          call set_edit input_field_index, input_rexx
          call set_status status_bar, "Selected: "input_rexx
          ## call show_widget(submit)
          call set_sensitive submit,1
          call set_sensitive update,1
       end
    end
    else if event_token = submit then do
       call submitit input_rexx,home,message_area
    end
    else if event_token = update then do
       call run_sync "C:\Program Files\Notepad++\Notepad++.exe",input_rexx,""
    end
end
exit 0
/* ----------------------------------------------------------------------------------------
 * Event Handler
 * ----------------------------------------------------------------------------------------
 */
submitit: procedure=.int
arg isrexx=.string,home=.string,msg=.int
  getfile=translate(isrexx,,'/\')
  fpos=words(getfile)
  member=translate(word(getfile,fpos),,'.')
  member=word(member,1)
  xdir=word(getfile,fpos-1)

  build=home"\cmake-build-debug"

  plugindir=build"\lib\plugins\"xdir
  plugindir2=home"\lib\plugins\"xdir
  rxc=build"\compiler\rxc.exe"
  rxas=build"\assembler\rxas.exe"
  rxvm=build"\interpreter\rxvme.exe"

  ms1="Compile  "member": "run_sync(rxc, "-i "build"\rxfns\rxas;"plugindir" -l "plugindir2" -o "member member,"")
  ms2="Assembly "member": "run_sync(rxas,"-l "plugindir2" -o "member member,"")
## rxbin member must be copied into the BUILD part
  crx=copy_file(plugindir2"\"member".rxbin",plugindir"\"member".rxbin")
  ms3="Execute  "member": "run_sync(rxvm,member" rx_"xdir,plugindir)
  /* Append messages to it */
  call append_message msg,ms1
  call append_message msg,ms2
  call append_message msg,ms3
return 0