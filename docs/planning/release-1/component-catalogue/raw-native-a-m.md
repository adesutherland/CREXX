# Raw RXPA registrations: a-m

Stage 2 discovery data taken from active `ADDPROC` registrations in native source. The registration option is recorded verbatim and is not yet treated as an approved level classification.

| ID | Plugin/source area | Registered name | C procedure | Registration option | Result | Arguments | Source |
|---|---|---|---|---|---|---|---|
| `RXPA-arrays-bubble-sort` | `lib/plugins/arrays` | `arrays.bubble_sort` | `bubble_sort` | `b` | `.void` | `expose a = .string[]` | `lib/plugins/arrays/arrays.c` |
| `RXPA-arrays-copy-array` | `lib/plugins/arrays` | `arrays.copy_array` | `copy_array` | `b` | `.int` | `expose a = .string[],b=.string[],from=.int,tto=.int` | `lib/plugins/arrays/arrays.c` |
| `RXPA-arrays-delete-array` | `lib/plugins/arrays` | `arrays.delete_array` | `delete_array` | `b` | `.int` | `expose a = .string[],from=.int,tto=.int` | `lib/plugins/arrays/arrays.c` |
| `RXPA-arrays-insert-array` | `lib/plugins/arrays` | `arrays.insert_array` | `insert_array` | `b` | `.int` | `expose a = .string[],from=.int,new=.int` | `lib/plugins/arrays/arrays.c` |
| `RXPA-arrays-list-array` | `lib/plugins/arrays` | `arrays.list_array` | `list_array` | `b` | `.void` | `expose a = .string[],from=.int,tto=.int` | `lib/plugins/arrays/arrays.c` |
| `RXPA-arrays-merge-array` | `lib/plugins/arrays` | `arrays.merge_array` | `merge_array` | `b` | `.int` | `expose a = .string[],expose b=.string[]` | `lib/plugins/arrays/arrays.c` |
| `RXPA-arrays-pyarray` | `lib/plugins/arrays` | `arrays.pyarray` | `pyarray` | `b` | `.void` | `expose code = .string[]` | `lib/plugins/arrays/arrays.c` |
| `RXPA-arrays-quick-sort` | `lib/plugins/arrays` | `arrays.quick_sort` | `quick_sort` | `b` | `.void` | `expose a = .string[], offset=.int, order=.string` | `lib/plugins/arrays/arrays.c` |
| `RXPA-arrays-reverse-array` | `lib/plugins/arrays` | `arrays.reverse_array` | `reverse_array` | `b` | `.void` | `expose a = .string[]` | `lib/plugins/arrays/arrays.c` |
| `RXPA-arrays-search-array` | `lib/plugins/arrays` | `arrays.search_array` | `search_array` | `b` | `.int` | `expose a = .string[],needle=.string,startrow=.int` | `lib/plugins/arrays/arrays.c` |
| `RXPA-arrays-shell-sort` | `lib/plugins/arrays` | `arrays.shell_sort` | `shell_sort` | `b` | `.void` | `expose a = .string[], offset=.int, order=.string` | `lib/plugins/arrays/arrays.c` |
| `RXPA-cipher-md5` | `lib/plugins/cipher` | `cipher.md5` | `md5` | `b` | `.string` | `arg0=.string` | `lib/plugins/cipher/cipher.c` |
| `RXPA-console-closeconsole` | `lib/plugins/console` | `console.closeconsole` | `CLOSECONSOLE` | `b` | `.int` | `` | `lib/plugins/console/console.c` |
| `RXPA-console-clrscr` | `lib/plugins/console` | `console.clrscr` | `CLRSCR` | `b` | `.int` | `` | `lib/plugins/console/console.c` |
| `RXPA-console-console-getchar` | `lib/plugins/console` | `console.console_getchar` | `CONSOLE_GETCHAR` | `b` | `.int` | `echo=.int` | `lib/plugins/console/console.c` |
| `RXPA-console-console-input` | `lib/plugins/console` | `console.console_input` | `CONSOLE_INPUT` | `b` | `.string` | `prompt=.string,maxlen=.int` | `lib/plugins/console/console.c` |
| `RXPA-console-extendedkey` | `lib/plugins/console` | `console.extendedkey` | `EXTENDEDKEY` | `b` | `.int` | `` | `lib/plugins/console/console.c` |
| `RXPA-console-fullclear` | `lib/plugins/console` | `console.fullclear` | `FULLCLEAR` | `b` | `.int` | `` | `lib/plugins/console/console.c` |
| `RXPA-console-getchar` | `lib/plugins/console` | `console.getchar` | `GETCHAR` | `b` | `.int` | `timeout=.int` | `lib/plugins/console/console.c` |
| `RXPA-console-getkey` | `lib/plugins/console` | `console.getkey` | `GETKEY` | `b` | `.int` | `` | `lib/plugins/console/console.c` |
| `RXPA-console-kbhit` | `lib/plugins/console` | `console.kbhit` | `KBHIT` | `b` | `.int` | `` | `lib/plugins/console/console.c` |
| `RXPA-console-keyname` | `lib/plugins/console` | `console.keyname` | `KEYNAME` | `b` | `.string` | `keycode=.int` | `lib/plugins/console/console.c` |
| `RXPA-console-mydummy` | `lib/plugins/console` | `console.mydummy` | `STRIM` | `b` | `.string` | `string=.string` | `lib/plugins/console/console.c` |
| `RXPA-console-newconsole` | `lib/plugins/console` | `console.newconsole` | `NEWCONSOLE` | `b` | `.int` | `title=.string,width=.int,height=.int` | `lib/plugins/console/console.c` |
| `RXPA-console-openconsole` | `lib/plugins/console` | `console.openconsole` | `OPENCONSOLE` | `b` | `.int` | `title=.string,width=.int,height=.int` | `lib/plugins/console/console.c` |
| `RXPA-console-printat` | `lib/plugins/console` | `console.printat` | `PRINTAT` | `b` | `.int` | `row=.int,col=.int,string=.string` | `lib/plugins/console/console.c` |
| `RXPA-console-readconsole` | `lib/plugins/console` | `console.readconsole` | `READSTRING` | `b` | `.string` | `maxlen=.int,prompt=.string` | `lib/plugins/console/console.c` |
| `RXPA-console-resetcolors` | `lib/plugins/console` | `console.resetcolors` | `RESETCOLORS` | `b` | `.int` | `` | `lib/plugins/console/console.c` |
| `RXPA-console-screensize` | `lib/plugins/console` | `console.screensize` | `SCREENSIZE` | `b` | `.string` | `` | `lib/plugins/console/console.c` |
| `RXPA-console-setcolor` | `lib/plugins/console` | `console.setcolor` | `SETCOLOR` | `b` | `.int` | `fgcolor=.int,bgcolor=.int` | `lib/plugins/console/console.c (2 registration sites)` |
| `RXPA-console-setcursor` | `lib/plugins/console` | `console.setcursor` | `SETCURSOR` | `b` | `.int` | `row=.int,col=.int` | `lib/plugins/console/console.c` |
| `RXPA-console-strim` | `lib/plugins/console` | `console.strim` | `STRIM` | `b` | `.string` | `string=.string` | `lib/plugins/console/console.c` |
| `RXPA-console-wait` | `lib/plugins/console` | `console.wait` | `WAIT` | `b` | `.int` | `sleep=.int` | `lib/plugins/console/console.c` |
| `RXPA-fileio-appendall` | `lib/plugins/fileio` | `fileio.appendall` | `appendall` | `b` | `.int` | `expose array=.string[],file=.string,arg2=.int` | `lib/plugins/fileio/fileio.c` |
| `RXPA-fileio-chmod` | `lib/plugins/fileio` | `fileio.chmod` | `chmodx` | `b` | `.int` | `expose path=.string,mode=.int` | `lib/plugins/fileio/fileio.c` |
| `RXPA-fileio-copyfile` | `lib/plugins/fileio` | `fileio.copyfile` | `copyfile` | `b` | `.int` | `expose src=.string,expose dest=.string` | `lib/plugins/fileio/fileio.c` |
| `RXPA-fileio-delete` | `lib/plugins/fileio` | `fileio.delete` | `deletefile` | `b` | `.int` | `expose path=.string` | `lib/plugins/fileio/fileio.c` |
| `RXPA-fileio-exists` | `lib/plugins/fileio` | `fileio.exists` | `exists` | `b` | `.int` | `expose path=.string` | `lib/plugins/fileio/fileio.c` |
| `RXPA-fileio-filesize` | `lib/plugins/fileio` | `fileio.filesize` | `filesize` | `b` | `.int` | `expose path=.string` | `lib/plugins/fileio/fileio.c` |
| `RXPA-fileio-move` | `lib/plugins/fileio` | `fileio.move` | `movefile` | `b` | `.int` | `expose oldpath=.string,expose newpath=.string` | `lib/plugins/fileio/fileio.c` |
| `RXPA-fileio-readall` | `lib/plugins/fileio` | `fileio.readall` | `readall` | `b` | `.int` | `expose array=.string[],expose file=.string,arg2=.int` | `lib/plugins/fileio/fileio.c` |
| `RXPA-fileio-readdir` | `lib/plugins/fileio` | `fileio.readdir` | `readdir` | `b` | `.int` | `expose entries=.string[],expose dirs=.string[],file=.string` | `lib/plugins/fileio/fileio.c` |
| `RXPA-fileio-truncate` | `lib/plugins/fileio` | `fileio.truncate` | `truncatex` | `b` | `.int` | `expose path=.string,size=.int` | `lib/plugins/fileio/fileio.c` |
| `RXPA-fileio-writeall` | `lib/plugins/fileio` | `fileio.writeall` | `writeall` | `b` | `.int` | `expose array=.string[],file=.string,arg2=.int` | `lib/plugins/fileio/fileio.c` |
| `RXPA-getpi-getpi` | `lib/plugins/getpi` | `getpi.getpi` | `getpi` | `b` | `.float` | `pi_arg=.string` | `lib/plugins/getpi/getpi.c` |
| `RXPA-gui-add-button` | `lib/plugins/gui` | `gui.add_button` | `add_button` | `b` | `.int` | `button_text=.string,x=.int,y=.int` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-add-checkbox` | `lib/plugins/gui` | `gui.add_checkbox` | `add_checkbox` | `b` | `.int` | `text=.string,x=.int,y=.int` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-add-combo` | `lib/plugins/gui` | `gui.add_combo` | `add_combo` | `b` | `.int` | `expose items=.string[],x=.int,y=.int` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-add-edit` | `lib/plugins/gui` | `gui.add_edit` | `add_edit` | `b` | `.int` | `x=.int,y=.int,width=.int` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-add-graph` | `lib/plugins/gui` | `gui.add_graph` | `add_graph` | `b` | `.int` | `x=.int,y=.int,width=.int,height=.int,x1=.float[],y1=.float[],x2=.float[],y2=.float[],x3=.float[],y3=.float[]` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-add-list` | `lib/plugins/gui` | `gui.add_list` | `add_list` | `b` | `.int` | `x=.int,y=.int,width=.int,height=.int` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-add-message-area` | `lib/plugins/gui` | `gui.add_message_area` | `add_message_area` | `b` | `.int` | `x=.int,y=.int,width=.int,height=.int` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-add-r2chart` | `lib/plugins/gui` | `gui.add_r2chart` | `add_r2chart` | `b` | `.int` | `x=.int,y=.int,width=.int,height=.int,x1=.float[],y1=.float[],x2=.float[],y2=.float[],x3=.float[],y3=.float[]` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-add-status-bar` | `lib/plugins/gui` | `gui.add_status_bar` | `add_status_bar` | `b` | `.int` | `` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-add-text` | `lib/plugins/gui` | `gui.add_text` | `add_text` | `b` | `.int` | `text=.string,x=.int,y=.int` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-append-message` | `lib/plugins/gui` | `gui.append_message` | `append_message` | `b` | `.int` | `index=.int,message=.string` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-cleanup` | `lib/plugins/gui` | `gui.cleanup` | `cleanup_gui` | `b` | `.void` | `` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-combo-add-item` | `lib/plugins/gui` | `gui.combo_add_item` | `combo_add_item` | `b` | `.int` | `combo=.int,text=.string` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-combo-clear` | `lib/plugins/gui` | `gui.combo_clear` | `combo_clear` | `b` | `.int` | `combo=.int` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-combo-pick` | `lib/plugins/gui` | `gui.combo_pick` | `combo_pick` | `b` | `.string` | `title=.string,message=.string,expose items=.string[]` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-combo-remove-item` | `lib/plugins/gui` | `gui.combo_remove_item` | `combo_remove_item` | `b` | `.int` | `combo=.int,position=.int` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-copy-file` | `lib/plugins/gui` | `gui.copy_file` | `copy_file_procedure` | `b` | `.int` | `source_path=.string,destination_path=.string` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-date-pick` | `lib/plugins/gui` | `gui.date_pick` | `date_pick` | `b` | `.string` | `title=.string,show_time=.int,format=.string` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-dialog-pick` | `lib/plugins/gui` | `gui.dialog_pick` | `dialog_pick` | `b` | `.string` | `title=.string,message=.string,buttons=.string` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-file-pick` | `lib/plugins/gui` | `gui.file_pick` | `file_pick` | `b` | `.string` | `title=.string,initial_dir=.string,save_dialog=.int,pattern=.string` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-form-pick` | `lib/plugins/gui` | `gui.form_pick` | `form_pick` | `b` | `.string` | `title=.string,message=.string,expose labels=.string[],expose defaults=.string[]` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-get-combo-index` | `lib/plugins/gui` | `gui.get_combo_index` | `get_combo_index` | `b` | `.int` | `combo=.int` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-get-widget-address` | `lib/plugins/gui` | `gui.get_widget_address` | `get_widget_address` | `b` | `.int` | `index=.int` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-hide-widget` | `lib/plugins/gui` | `gui.hide_widget` | `hide_widget` | `b` | `.int` | `index=.int` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-init-window` | `lib/plugins/gui` | `gui.init_window` | `init_window` | `b` | `.int` | `title=.string, width=.int,height=.int` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-input-pick` | `lib/plugins/gui` | `gui.input_pick` | `input_pick` | `b` | `.string` | `title=.string,message=.string,default_value=.string,password_mode=.int` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-list-add-item` | `lib/plugins/gui` | `gui.list_add_item` | `list_add_item` | `b` | `.int` | `list=.int,text=.string,bg_color=.string` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-list-clear` | `lib/plugins/gui` | `gui.list_clear` | `list_clear` | `b` | `.int` | `list=.int` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-list-get-count` | `lib/plugins/gui` | `gui.list_get_count` | `list_get_count` | `b` | `.int` | `list=.int` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-list-get-selected` | `lib/plugins/gui` | `gui.list_get_selected` | `list_get_selected` | `b` | `.int` | `list=.int` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-list-get-selected-item` | `lib/plugins/gui` | `gui.list_get_selected_item` | `list_get_selected_item` | `b` | `.string` | `list=.int` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-list-header` | `lib/plugins/gui` | `gui.list_header` | `list_set_header` | `b` | `.int` | `list=.int,header=.string,text_color=.string,bg_color=.string` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-list-pick` | `lib/plugins/gui` | `gui.list_pick` | `list_pick` | `b` | `.string` | `title=.string,expose items=.string[],multi_select=.int,message=.string` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-notify-pick` | `lib/plugins/gui` | `gui.notify_pick` | `notify_pick` | `b` | `.string` | `title=.string,message=.string,type=.string` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-page-pick` | `lib/plugins/gui` | `gui.page_pick` | `page_pick` | `b` | `.string` | `title=.string,expose pages=.string[],expose labels=.string[],expose defaults=.string[]` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-path-pick` | `lib/plugins/gui` | `gui.path_pick` | `path_pick` | `b` | `.string` | `title=.string,initial_dir=.string` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-process-events` | `lib/plugins/gui` | `gui.process_events` | `process_events` | `b` | `.int` | `timeout=.int` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-report-widgets` | `lib/plugins/gui` | `gui.report_widgets` | `report_widgets` | `b` | `.string` | `` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-rgb-to-hex` | `lib/plugins/gui` | `gui.rgb_to_hex` | `rgb_to_hex` | `b` | `.string` | `r=.int,g=.int,b=.int` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-run-program` | `lib/plugins/gui` | `gui.run_program` | `run_external_program` | `b` | `.string` | `command=.string` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-run-sync` | `lib/plugins/gui` | `gui.run_sync` | `run_external_program_sync` | `b` | `.string` | `command=.string,parm=.string,wdir=.string` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-set-edit` | `lib/plugins/gui` | `gui.set_edit` | `set_edit` | `b` | `.int` | `index=.int,text=.string` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-set-sensitive` | `lib/plugins/gui` | `gui.set_sensitive` | `set_sensitive` | `b` | `.int` | `index=.int,sensitive=.int` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-set-status` | `lib/plugins/gui` | `gui.set_status` | `set_status` | `b` | `.int` | `index=.int,message=.string` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-set-text` | `lib/plugins/gui` | `gui.set_text` | `set_text` | `b` | `.int` | `index=.int,text=.string` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-show-widget` | `lib/plugins/gui` | `gui.show_widget` | `show_widget` | `b` | `.int` | `index=.int` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-show-window` | `lib/plugins/gui` | `gui.show_window` | `show_window` | `b` | `.int` | `x=.int,y=.int` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-splash-pick` | `lib/plugins/gui` | `gui.splash_pick` | `splash_pick` | `b` | `.string` | `title=.string,message=.string,duration=.int,width=.int,height=.int,image_path=.string` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-text-display` | `lib/plugins/gui` | `gui.text_display` | `text_display` | `b` | `.string` | `title=.string,message=.string,item_texts=.string` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-text-display-pick` | `lib/plugins/gui` | `gui.text_display_pick` | `text_display_pick` | `b` | `.string` | `title=.string,message=.string,expose item_texts=.string[]` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-tree-diagram` | `lib/plugins/gui` | `gui.tree_diagram` | `tree_diagram` | `b` | `.string` | `expose items=.string[],expose parents=.string[]` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-tree-pick` | `lib/plugins/gui` | `gui.tree_pick` | `tree_pick` | `b` | `.string` | `title=.string,message=.string,expose items=.string[],expose parents=.string[],multi_select=.int` | `lib/plugins/gui/gui.c` |
| `RXPA-gui-update-graph` | `lib/plugins/gui` | `gui.update_graph` | `update_graph` | `b` | `.int` | `widget=.int,gnum=.int,ftype=.int,lwidth=.int,color=.string` | `lib/plugins/gui/gui.c` |
| `RXPA-hash-sha256` | `lib/plugins/hash` | `rxhash.sha256` | `sha256` | `b` | `.binary` | `data = .binary` | `lib/plugins/hash/rx_hash.c` |
| `RXPA-id-base58` | `lib/plugins/id` | `id._base58` | `base58` | `b` | `.string` | `` | `lib/plugins/id/id.c` |
| `RXPA-id-nanoid` | `lib/plugins/id` | `id._nanoid` | `nanoid` | `b` | `.string` | `` | `lib/plugins/id/id.c` |
| `RXPA-id-snowflake` | `lib/plugins/id` | `id._snowflake` | `snowflake` | `b` | `.string` | `` | `lib/plugins/id/id.c` |
| `RXPA-id-ulid` | `lib/plugins/id` | `id._ulid` | `ulid` | `b` | `.string` | `` | `lib/plugins/id/id.c` |
| `RXPA-id-uuid` | `lib/plugins/id` | `id._uuid` | `uuid` | `b` | `.string` | `` | `lib/plugins/id/id.c` |
| `RXPA-id-uuidt` | `lib/plugins/id` | `id._uuidt` | `uuidt` | `b` | `.string` | `` | `lib/plugins/id/id.c` |
| `RXPA-id-uuidv7` | `lib/plugins/id` | `id._uuidv7` | `uuidv7` | `b` | `.string` | `` | `lib/plugins/id/id.c` |
| `RXPA-keyaccess-backup` | `lib/plugins/keyaccess` | `keyaccess._backup` | `backup` | `b` | `.int` | `handle=.int,path=.string` | `lib/plugins/keyaccess/keyaccess.c` |
| `RXPA-keyaccess-closekey` | `lib/plugins/keyaccess` | `keyaccess._closekey` | `closefile` | `b` | `.int` | `handle=.int` | `lib/plugins/keyaccess/keyaccess.c` |
| `RXPA-keyaccess-compact` | `lib/plugins/keyaccess` | `keyaccess._compact` | `compact_database` | `b` | `.int` | `handle=.int` | `lib/plugins/keyaccess/keyaccess.c` |
| `RXPA-keyaccess-deletekey` | `lib/plugins/keyaccess` | `keyaccess._deletekey` | `deletekey` | `b` | `.int` | `handle=.int,key=.string` | `lib/plugins/keyaccess/keyaccess.c` |
| `RXPA-keyaccess-listkey` | `lib/plugins/keyaccess` | `keyaccess._listkey` | `listkeys` | `b` | `.int` | `handle=.int` | `lib/plugins/keyaccess/keyaccess.c` |
| `RXPA-keyaccess-openkey` | `lib/plugins/keyaccess` | `keyaccess._openkey` | `openfile` | `b` | `.int` | `filename=.string,mode=.string` | `lib/plugins/keyaccess/keyaccess.c` |
| `RXPA-keyaccess-readkey` | `lib/plugins/keyaccess` | `keyaccess._readkey` | `readkey` | `b` | `.string` | `handle=.int,key=.string` | `lib/plugins/keyaccess/keyaccess.c` |
| `RXPA-keyaccess-stats` | `lib/plugins/keyaccess` | `keyaccess._stats` | `get_statistics` | `b` | `.string` | `handle=.int` | `lib/plugins/keyaccess/keyaccess.c` |
| `RXPA-keyaccess-txbegin` | `lib/plugins/keyaccess` | `keyaccess._txbegin` | `begin_transaction` | `b` | `.int` | `handle=.int` | `lib/plugins/keyaccess/keyaccess.c` |
| `RXPA-keyaccess-txcommit` | `lib/plugins/keyaccess` | `keyaccess._txcommit` | `commit_transaction` | `b` | `.int` | `handle=.int` | `lib/plugins/keyaccess/keyaccess.c` |
| `RXPA-keyaccess-txrollback` | `lib/plugins/keyaccess` | `keyaccess._txrollback` | `rollback_transaction` | `b` | `.int` | `handle=.int` | `lib/plugins/keyaccess/keyaccess.c` |
| `RXPA-keyaccess-validate` | `lib/plugins/keyaccess` | `keyaccess._validate` | `validate_database` | `b` | `.int` | `handle=.int` | `lib/plugins/keyaccess/keyaccess.c` |
| `RXPA-keyaccess-writekey` | `lib/plugins/keyaccess` | `keyaccess._writekey` | `writekey` | `b` | `.int` | `handle=.int,key=.string,value=.string` | `lib/plugins/keyaccess/keyaccess.c` |
| `RXPA-llist-debug` | `lib/plugins/llist` | `llist._debug` | `debug_llist` | `b` | `.int` | `qname=.int` | `lib/plugins/llist/llist.c` |
| `RXPA-llist-appendnode` | `lib/plugins/llist` | `llist.appendnode` | `appendnode` | `b` | `.int` | `qname=.int,message=.string` | `lib/plugins/llist/llist.c` |
| `RXPA-llist-appnode` | `lib/plugins/llist` | `llist.appnode` | `appendnode` | `b` | `.int` | `qname=.int,message=.string` | `lib/plugins/llist/llist.c` |
| `RXPA-llist-cleanup` | `lib/plugins/llist` | `llist.cleanup` | `cleanup` | `b` | `.int` | `` | `lib/plugins/llist/llist.c` |
| `RXPA-llist-currentnode` | `lib/plugins/llist` | `llist.currentnode` | `currentnode` | `b` | `.string` | `qname=.int` | `lib/plugins/llist/llist.c` |
| `RXPA-llist-currentnodeaddr` | `lib/plugins/llist` | `llist.currentnodeaddr` | `currentnodeaddr` | `b` | `.int` | `qname=.int` | `lib/plugins/llist/llist.c` |
| `RXPA-llist-freellist` | `lib/plugins/llist` | `llist.freellist` | `freellist` | `b` | `.int` | `qname=.int` | `lib/plugins/llist/llist.c` |
| `RXPA-llist-insertnode` | `lib/plugins/llist` | `llist.insertnode` | `insertnode` | `b` | `.int` | `qname=.int,message=.string,mode=.string` | `lib/plugins/llist/llist.c` |
| `RXPA-llist-listllist` | `lib/plugins/llist` | `llist.listllist` | `listllist` | `b` | `.int` | `qname=.int` | `lib/plugins/llist/llist.c` |
| `RXPA-llist-listnode` | `lib/plugins/llist` | `llist.listnode` | `listnode` | `b` | `.int` | `qname=.int` | `lib/plugins/llist/llist.c` |
| `RXPA-llist-nextnode` | `lib/plugins/llist` | `llist.nextnode` | `nextnode` | `b` | `.string` | `qname=.int` | `lib/plugins/llist/llist.c` |
| `RXPA-llist-prependnode` | `lib/plugins/llist` | `llist.prependnode` | `prependnode` | `b` | `.int` | `qname=.int,message=.string` | `lib/plugins/llist/llist.c` |
| `RXPA-llist-prepnode` | `lib/plugins/llist` | `llist.prepnode` | `prependnode` | `b` | `.int` | `qname=.int,message=.string` | `lib/plugins/llist/llist.c` |
| `RXPA-llist-prevnode` | `lib/plugins/llist` | `llist.prevnode` | `prevnode` | `b` | `.string` | `qname=.int` | `lib/plugins/llist/llist.c` |
| `RXPA-llist-removenode` | `lib/plugins/llist` | `llist.removenode` | `removenode` | `b` | `.int` | `qname=.int` | `lib/plugins/llist/llist.c` |
| `RXPA-llist-setnode` | `lib/plugins/llist` | `llist.setnode` | `setnode` | `b` | `.int` | `queue=.int,position=.string` | `lib/plugins/llist/llist.c` |
| `RXPA-llist-setnodeaddr` | `lib/plugins/llist` | `llist.setnodeaddr` | `setnodeaddr` | `b` | `.int` | `queue=.int,position=.int` | `lib/plugins/llist/llist.c` |
| `RXPA-rxdes-decrypt` | `lib/rxfnsb/native/des` | `rxdes.decrypt` | `RxDesDecrypt` | `b` | `.string` | `key=.string,data=.string` | `lib/rxfnsb/native/des/rxdes.c` |
| `RXPA-rxdes-encrypt` | `lib/rxfnsb/native/des` | `rxdes.encrypt` | `RxDesEncrypt` | `b` | `.string` | `key=.string,data=.string` | `lib/rxfnsb/native/des/rxdes.c` |

Discovered rows: **137**.
