options levelb
import gui
import rxfnsb

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