# Raw RXPA registrations: n-z

Stage 2 discovery data taken from active `ADDPROC` registrations in native source. The registration option is recorded verbatim and is not yet treated as an approved level classification.

| ID | Plugin/source area | Registered name | C procedure | Registration option | Result | Arguments | Source |
|---|---|---|---|---|---|---|---|
| `RXPA-map-clonestem` | `lib/plugins/map` | `map.clonestem` | `clonestem` | `b` | `.int` | `source=.string,target=.string` | `lib/plugins/map/map.c` |
| `RXPA-map-dropentry` | `lib/plugins/map` | `map.dropentry` | `dropentry` | `b` | `.int` | `stem=.string,key=.string` | `lib/plugins/map/map.c` |
| `RXPA-map-dropstem` | `lib/plugins/map` | `map.dropstem` | `dropstem` | `b` | `.int` | `stem=.string` | `lib/plugins/map/map.c` |
| `RXPA-map-getall` | `lib/plugins/map` | `map.getall` | `getall` | `b` | `.int` | `stem=.string, expose keys=.string[], expose values=.string[]` | `lib/plugins/map/map.c` |
| `RXPA-map-getalltails` | `lib/plugins/map` | `map.getalltails` | `getalltails` | `b` | `.int` | `stem=.string, expose keys=.string[]` | `lib/plugins/map/map.c` |
| `RXPA-map-getstem` | `lib/plugins/map` | `map.getstem` | `getstem` | `b` | `.string` | `stem=.string` | `lib/plugins/map/map.c` |
| `RXPA-map-getstemmsg` | `lib/plugins/map` | `map.getstemmsg` | `getstemmsg` | `b` | `.string` | `` | `lib/plugins/map/map.c` |
| `RXPA-map-getstemtail` | `lib/plugins/map` | `map.getstemtail` | `gettail` | `b` | `.string` | `stem=.string,ordinal=.int` | `lib/plugins/map/map.c` |
| `RXPA-map-liststems` | `lib/plugins/map` | `map.liststems` | `liststems` | `b` | `.int` | `expose stems=.string[]` | `lib/plugins/map/map.c` |
| `RXPA-map-mapflag` | `lib/plugins/map` | `map.mapflag` | `mapflagset` | `b` | `.string` | `flag=.int` | `lib/plugins/map/map.c` |
| `RXPA-map-putstem` | `lib/plugins/map` | `map.putstem` | `setstem` | `b` | `.int` | `stem=.string,value=.string` | `lib/plugins/map/map.c` |
| `RXPA-map-reorgstem` | `lib/plugins/map` | `map.reorgstem` | `compactstem` | `b` | `.int` | `` | `lib/plugins/map/map.c` |
| `RXPA-map-reservestem` | `lib/plugins/map` | `map.reservestem` | `reservestem` | `b` | `.int` | `stem=.string,size=.int` | `lib/plugins/map/map.c` |
| `RXPA-map-stemquote` | `lib/plugins/map` | `map.stemquote` | `stemquote` | `b` | `.string` | `path=.string` | `lib/plugins/map/map.c` |
| `RXPA-map-stemstat` | `lib/plugins/map` | `map.stemstat` | `statsstem` | `b` | `.string` | `stem=.string` | `lib/plugins/map/map.c` |
| `RXPA-matrix-masciiplot` | `lib/plugins/matrix` | `matrix.masciiplot` | `masciiplot` | `b` | `.int` | `m0=.int, plot_type=.string` | `lib/plugins/matrix/matrix.c` |
| `RXPA-matrix-mcolstats` | `lib/plugins/matrix` | `matrix.mcolstats` | `mcolstats` | `b` | `.int` | `m0=.int, mid=.string` | `lib/plugins/matrix/matrix.c` |
| `RXPA-matrix-mcorr` | `lib/plugins/matrix` | `matrix.mcorr` | `mcorr` | `b` | `.int` | `m0=.int, mid=.string` | `lib/plugins/matrix/matrix.c` |
| `RXPA-matrix-mcov` | `lib/plugins/matrix` | `matrix.mcov` | `mcov` | `b` | `.int` | `m0=.int, mid=.string` | `lib/plugins/matrix/matrix.c` |
| `RXPA-matrix-mcreate` | `lib/plugins/matrix` | `matrix.mcreate` | `mcreate` | `b` | `.int` | `rows=.int,cols=.int,id=.string` | `lib/plugins/matrix/matrix.c` |
| `RXPA-matrix-mdet` | `lib/plugins/matrix` | `matrix.mdet` | `mdet` | `b` | `.int` | `m0=.int` | `lib/plugins/matrix/matrix.c` |
| `RXPA-matrix-mexpand` | `lib/plugins/matrix` | `matrix.mexpand` | `mexpand` | `b` | `.int` | `m0=.int, newrows=.int, init=.float` | `lib/plugins/matrix/matrix.c` |
| `RXPA-matrix-mfactor` | `lib/plugins/matrix` | `matrix.mfactor` | `mfactor` | `b` | `.int` | `m0=.int, factors=.int, mid=.string, parms=.string` | `lib/plugins/matrix/matrix.c` |
| `RXPA-matrix-mfaplot` | `lib/plugins/matrix` | `matrix.mfaplot` | `mfaplot` | `b` | `.int` | `m0=.int, plot_type=.string` | `lib/plugins/matrix/matrix.c` |
| `RXPA-matrix-mfree` | `lib/plugins/matrix` | `matrix.mfree` | `mfree` | `b` | `.int` | `m0=.int` | `lib/plugins/matrix/matrix.c` |
| `RXPA-matrix-mget` | `lib/plugins/matrix` | `matrix.mget` | `mget` | `b` | `.float` | `m0=.int, row=.int, col=.int` | `lib/plugins/matrix/matrix.c` |
| `RXPA-matrix-minvert` | `lib/plugins/matrix` | `matrix.minvert` | `minvert` | `b` | `.int` | `m0=.int, mid=.string` | `lib/plugins/matrix/matrix.c` |
| `RXPA-matrix-mlu` | `lib/plugins/matrix` | `matrix.mlu` | `mlu` | `b` | `.int` | `m0=.int, L=.string, U=.string` | `lib/plugins/matrix/matrix.c` |
| `RXPA-matrix-mmean` | `lib/plugins/matrix` | `matrix.mmean` | `mmean` | `b` | `.float` | `m0=.int, axis=.int` | `lib/plugins/matrix/matrix.c` |
| `RXPA-matrix-mmult` | `lib/plugins/matrix` | `matrix.mmult` | `mmultiply` | `b` | `.int` | `m0=.int, m1=.int,mid=.string` | `lib/plugins/matrix/matrix.c` |
| `RXPA-matrix-mplot` | `lib/plugins/matrix` | `matrix.mplot` | `mplot` | `b` | `.int` | `m0=.int, plot_type=.string` | `lib/plugins/matrix/matrix.c` |
| `RXPA-matrix-mprint` | `lib/plugins/matrix` | `matrix.mprint` | `mprint` | `b` | `.int` | `m0=.int,hdr=.string` | `lib/plugins/matrix/matrix.c` |
| `RXPA-matrix-mprod` | `lib/plugins/matrix` | `matrix.mprod` | `mprod` | `b` | `.int` | `m0=.int, prod=.float,mid=.string` | `lib/plugins/matrix/matrix.c` |
| `RXPA-matrix-mrank` | `lib/plugins/matrix` | `matrix.mrank` | `mrank` | `b` | `.int` | `m0=.int` | `lib/plugins/matrix/matrix.c` |
| `RXPA-matrix-mset` | `lib/plugins/matrix` | `matrix.mset` | `mset` | `b` | `.int` | `m0=.int, row=.int, col=.int,value=.float` | `lib/plugins/matrix/matrix.c` |
| `RXPA-matrix-mstandard` | `lib/plugins/matrix` | `matrix.mstandard` | `mstandard` | `b` | `.int` | `m0=.int, mid=.string` | `lib/plugins/matrix/matrix.c` |
| `RXPA-matrix-mstdev` | `lib/plugins/matrix` | `matrix.mstdev` | `mstdev` | `b` | `.float` | `m0=.int, axis=.int` | `lib/plugins/matrix/matrix.c` |
| `RXPA-matrix-mtranspose` | `lib/plugins/matrix` | `matrix.mtranspose` | `mtranspose` | `b` | `.int` | `m0=.int, mid=.string` | `lib/plugins/matrix/matrix.c` |
| `RXPA-matrix-stats` | `lib/plugins/matrix` | `matrix.stats` | `stats` | `b` | `.int` | `m0=.int, mode=.string` | `lib/plugins/matrix/matrix.c` |
| `RXPA-odbc-odbc-begin-transaction` | `lib/plugins/odbc` | `odbc.odbc_begin_transaction` | `odbc_begin_transaction` | `b` | `.int` | `` | `lib/plugins/odbc/odbc.c` |
| `RXPA-odbc-odbc-bind-param` | `lib/plugins/odbc` | `odbc.odbc_bind_param` | `odbc_bind_param` | `b` | `.int` | `paramNum=.int,value=.string` | `lib/plugins/odbc/odbc.c` |
| `RXPA-odbc-odbc-colname` | `lib/plugins/odbc` | `odbc.odbc_colname` | `odbc_column_name` | `b` | `.string` | `column=.int` | `lib/plugins/odbc/odbc.c` |
| `RXPA-odbc-odbc-coltype` | `lib/plugins/odbc` | `odbc.odbc_coltype` | `odbc_column_type` | `b` | `.int` | `column=.int` | `lib/plugins/odbc/odbc.c` |
| `RXPA-odbc-odbc-column-info` | `lib/plugins/odbc` | `odbc.odbc_column_info` | `odbc_column_info` | `b` | `.string` | `column=.int` | `lib/plugins/odbc/odbc.c` |
| `RXPA-odbc-odbc-columns` | `lib/plugins/odbc` | `odbc.odbc_columns` | `odbc_columns` | `b` | `.int` | `` | `lib/plugins/odbc/odbc.c` |
| `RXPA-odbc-odbc-commit` | `lib/plugins/odbc` | `odbc.odbc_commit` | `odbc_commit` | `b` | `.int` | `` | `lib/plugins/odbc/odbc.c` |
| `RXPA-odbc-odbc-connect` | `lib/plugins/odbc` | `odbc.odbc_connect` | `odbc_connect` | `b` | `.int` | `dsn=.string,user=.string,password=.string` | `lib/plugins/odbc/odbc.c` |
| `RXPA-odbc-odbc-database` | `lib/plugins/odbc` | `odbc.odbc_database` | `odbc_database` | `b` | `.string` | `newdb=.string` | `lib/plugins/odbc/odbc.c` |
| `RXPA-odbc-odbc-disconnect` | `lib/plugins/odbc` | `odbc.odbc_disconnect` | `odbc_disconnect` | `b` | `.int` | `` | `lib/plugins/odbc/odbc.c` |
| `RXPA-odbc-odbc-error-message` | `lib/plugins/odbc` | `odbc.odbc_error_message` | `odbc_error_message` | `b` | `.string` | `` | `lib/plugins/odbc/odbc.c` |
| `RXPA-odbc-odbc-execute` | `lib/plugins/odbc` | `odbc.odbc_execute` | `odbc_execute` | `b` | `.int` | `sql=.string` | `lib/plugins/odbc/odbc.c` |
| `RXPA-odbc-odbc-execute-batch` | `lib/plugins/odbc` | `odbc.odbc_execute_batch` | `odbc_execute_batch` | `b` | `.int` | `sql=.string,delimiter=.string` | `lib/plugins/odbc/odbc.c` |
| `RXPA-odbc-odbc-fetch` | `lib/plugins/odbc` | `odbc.odbc_fetch` | `odbc_fetch` | `b` | `.int` | `row=.int` | `lib/plugins/odbc/odbc.c` |
| `RXPA-odbc-odbc-get-connection-attr` | `lib/plugins/odbc` | `odbc.odbc_get_connection_attr` | `odbc_get_connection_attr` | `b` | `.int` | `attr=.int` | `lib/plugins/odbc/odbc.c` |
| `RXPA-odbc-odbc-get-diagnostics` | `lib/plugins/odbc` | `odbc.odbc_get_diagnostics` | `odbc_get_diagnostics` | `b` | `.string` | `` | `lib/plugins/odbc/odbc.c` |
| `RXPA-odbc-odbc-get-info` | `lib/plugins/odbc` | `odbc.odbc_get_info` | `odbc_get_info` | `b` | `.string` | `` | `lib/plugins/odbc/odbc.c` |
| `RXPA-odbc-odbc-getcolumn` | `lib/plugins/odbc` | `odbc.odbc_getcolumn` | `odbc_get_column` | `b` | `.string` | `column=.int` | `lib/plugins/odbc/odbc.c` |
| `RXPA-odbc-odbc-move-to` | `lib/plugins/odbc` | `odbc.odbc_move_to` | `odbc_move_to` | `b` | `.int` | `row=.int` | `lib/plugins/odbc/odbc.c` |
| `RXPA-odbc-odbc-prepare` | `lib/plugins/odbc` | `odbc.odbc_prepare` | `odbc_prepare` | `b` | `.int` | `sql=.string` | `lib/plugins/odbc/odbc.c` |
| `RXPA-odbc-odbc-primary-keys` | `lib/plugins/odbc` | `odbc.odbc_primary_keys` | `odbc_primary_keys` | `b` | `.string` | `table=.string` | `lib/plugins/odbc/odbc.c` |
| `RXPA-odbc-odbc-rollback` | `lib/plugins/odbc` | `odbc.odbc_rollback` | `odbc_rollback` | `b` | `.int` | `` | `lib/plugins/odbc/odbc.c` |
| `RXPA-odbc-odbc-row-count` | `lib/plugins/odbc` | `odbc.odbc_row_count` | `odbc_row_count` | `b` | `.int` | `` | `lib/plugins/odbc/odbc.c` |
| `RXPA-odbc-odbc-tables` | `lib/plugins/odbc` | `odbc.odbc_tables` | `odbc_tables` | `b` | `.string` | `` | `lib/plugins/odbc/odbc.c` |
| `RXPA-odbc-show-message` | `lib/plugins/odbc` | `odbc.show_message` | `show_message` | `b` | `.string` | `line1=.string,line2=.string,line3=.string,line4=.string,` | `lib/plugins/odbc/odbc.c` |
| `RXPA-pick-combo-pick` | `lib/plugins/pick` | `pick.combo_pick` | `combo_pick` | `b` | `.string` | `title=.string,message=.string,expose items=.string[]` | `lib/plugins/pick/pick.c` |
| `RXPA-pick-date-pick` | `lib/plugins/pick` | `pick.date_pick` | `date_pick` | `b` | `.string` | `title=.string,show_time=.int,format=.string` | `lib/plugins/pick/pick.c` |
| `RXPA-pick-dialog-pick` | `lib/plugins/pick` | `pick.dialog_pick` | `dialog_pick` | `b` | `.string` | `title=.string,message=.string,buttons=.string` | `lib/plugins/pick/pick.c` |
| `RXPA-pick-file-pick` | `lib/plugins/pick` | `pick.file_pick` | `file_pick` | `b` | `.string` | `title=.string,initial_dir=.string,save_dialog=.int` | `lib/plugins/pick/pick.c` |
| `RXPA-pick-form-pick` | `lib/plugins/pick` | `pick.form_pick` | `form_pick` | `b` | `.string` | `title=.string,message=.string,expose labels=.string[],expose defaults=.string[]` | `lib/plugins/pick/pick.c` |
| `RXPA-pick-input-pick` | `lib/plugins/pick` | `pick.input_pick` | `input_pick` | `b` | `.string` | `title=.string,message=.string,default_value=.string,password_mode=.int` | `lib/plugins/pick/pick.c` |
| `RXPA-pick-list-pick` | `lib/plugins/pick` | `pick.list_pick` | `list_pick` | `b` | `.string` | `title=.string,expose items=.string[],multi_select=.int,message=.string` | `lib/plugins/pick/pick.c` |
| `RXPA-pick-notify-pick` | `lib/plugins/pick` | `pick.notify_pick` | `notify_pick` | `b` | `.string` | `title=.string,message=.string,type=.string` | `lib/plugins/pick/pick.c` |
| `RXPA-pick-page-pick` | `lib/plugins/pick` | `pick.page_pick` | `page_pick` | `b` | `.string` | `title=.string,expose pages=.string[],expose labels=.string[],expose defaults=.string[]` | `lib/plugins/pick/pick.c` |
| `RXPA-pick-path-pick` | `lib/plugins/pick` | `pick.path_pick` | `path_pick` | `b` | `.string` | `title=.string,initial_dir=.string` | `lib/plugins/pick/pick.c` |
| `RXPA-pick-splash-pick` | `lib/plugins/pick` | `pick.splash_pick` | `splash_pick` | `b` | `.string` | `title=.string,message=.string,duration=.int,image_path=.string` | `lib/plugins/pick/pick.c` |
| `RXPA-pick-text-display` | `lib/plugins/pick` | `pick.text_display` | `text_display` | `b` | `.string` | `title=.string,message=.string,item_texts=.string` | `lib/plugins/pick/pick.c` |
| `RXPA-pick-text-display-pick` | `lib/plugins/pick` | `pick.text_display_pick` | `text_display_pick` | `b` | `.string` | `title=.string,message=.string,expose item_texts=.string[]` | `lib/plugins/pick/pick.c` |
| `RXPA-pick-tree-diagram` | `lib/plugins/pick` | `pick.tree_diagram` | `tree_diagram` | `b` | `.string` | `expose items=.string[],expose parents=.string[]` | `lib/plugins/pick/pick.c` |
| `RXPA-pick-tree-pick` | `lib/plugins/pick` | `pick.tree_pick` | `tree_pick` | `b` | `.string` | `title=.string,message=.string,expose items=.string[],expose parents=.string[],multi_select=.int` | `lib/plugins/pick/pick.c` |
| `RXPA-pipe-pipecancel` | `lib/plugins/pipe` | `pipe.pipecancel` | `pipecancel` | `b` | `.int` | `proc=.int` | `lib/plugins/pipe/pipe.c` |
| `RXPA-pipe-pipeclose` | `lib/plugins/pipe` | `pipe.pipeclose` | `pipeclose` | `b` | `.int` | `proc=.int` | `lib/plugins/pipe/pipe.c` |
| `RXPA-pipe-pipecreate` | `lib/plugins/pipe` | `pipe.pipecreate` | `pipecreate` | `b` | `.int` | `mode=r` | `lib/plugins/pipe/pipe.c` |
| `RXPA-pipe-pipeexitcode` | `lib/plugins/pipe` | `pipe.pipeexitcode` | `pipeexitcode` | `b` | `.int` | `proc=.int` | `lib/plugins/pipe/pipe.c` |
| `RXPA-pipe-pipeget` | `lib/plugins/pipe` | `pipe.pipeget` | `pipeget` | `b` | `.int` | `proc=.int, expose array=.string[],timeout=0` | `lib/plugins/pipe/pipe.c` |
| `RXPA-pipe-piperun` | `lib/plugins/pipe` | `pipe.piperun` | `piperun` | `b` | `.int` | `proc=.int,cmd=.string,mode='R'` | `lib/plugins/pipe/pipe.c` |
| `RXPA-pipe-pipesend` | `lib/plugins/pipe` | `pipe.pipesend` | `pipesend` | `b` | `.int` | `proc=.int,string=.string` | `lib/plugins/pipe/pipe.c` |
| `RXPA-pipe-pipestatus` | `lib/plugins/pipe` | `pipe.pipestatus` | `pipestatus` | `b` | `.string` | `proc=.int` | `lib/plugins/pipe/pipe.c` |
| `RXPA-process-processcloseinput` | `lib/plugins/process` | `process.processcloseinput` | `processcloseinput` | `b` | `.int` | `handle=.int` | `lib/plugins/process/process.c` |
| `RXPA-process-processcreate` | `lib/plugins/process` | `process.processcreate` | `processcreate` | `b` | `.int` | `cmdline=.string,capture_output=.int` | `lib/plugins/process/process.c` |
| `RXPA-process-processfree` | `lib/plugins/process` | `process.processfree` | `processfree` | `b` | `.int` | `handle=.int` | `lib/plugins/process/process.c` |
| `RXPA-process-processfreeall` | `lib/plugins/process` | `process.processfreeall` | `processfreeall` | `b` | `.int` | `` | `lib/plugins/process/process.c` |
| `RXPA-process-processgetexitcode` | `lib/plugins/process` | `process.processgetexitcode` | `processgetexitcode` | `b` | `.int` | `handle=.int` | `lib/plugins/process/process.c` |
| `RXPA-process-processgetoutput` | `lib/plugins/process` | `process.processgetoutput` | `processgetoutput` | `b` | `.string` | `handle=.int` | `lib/plugins/process/process.c` |
| `RXPA-process-processisrunning` | `lib/plugins/process` | `process.processisrunning` | `processisrunning` | `b` | `.int` | `handle=.int` | `lib/plugins/process/process.c` |
| `RXPA-process-processkill` | `lib/plugins/process` | `process.processkill` | `processkill` | `b` | `.int` | `handle=.int` | `lib/plugins/process/process.c` |
| `RXPA-process-processlasterror` | `lib/plugins/process` | `process.processlasterror` | `processlasterror` | `b` | `.string` | `handle=.int` | `lib/plugins/process/process.c` |
| `RXPA-process-processlist` | `lib/plugins/process` | `process.processlist` | `processlist` | `b` | `.int[]` | `` | `lib/plugins/process/process.c` |
| `RXPA-process-processpeekoutput` | `lib/plugins/process` | `process.processpeekoutput` | `processpeekoutput` | `b` | `.string` | `handle=.int` | `lib/plugins/process/process.c` |
| `RXPA-process-processreadoutput` | `lib/plugins/process` | `process.processreadoutput` | `processreadoutput` | `b` | `.string` | `handle=.int` | `lib/plugins/process/process.c` |
| `RXPA-process-processsendinput` | `lib/plugins/process` | `process.processsendinput` | `processsendinput` | `b` | `.int` | `handle=.int,data=.string` | `lib/plugins/process/process.c` |
| `RXPA-process-processsettimeout` | `lib/plugins/process` | `process.processsettimeout` | `processsettimeout` | `b` | `.int` | `handle=.int,timeout_ms=.int` | `lib/plugins/process/process.c` |
| `RXPA-process-processstackinfo` | `lib/plugins/process` | `process.processstackinfo` | `processstackinfo` | `b` | `.int` | `` | `lib/plugins/process/process.c` |
| `RXPA-process-processwait` | `lib/plugins/process` | `process.processwait` | `processwait` | `b` | `.int` | `handle=.int,timeout_ms=.int` | `lib/plugins/process/process.c` |
| `RXPA-process-wait` | `lib/plugins/process` | `process.wait` | `sleep` | `b` | `.int` | `wait=.int` | `lib/plugins/process/process.c` |
| `RXPA-process-waitkill` | `lib/plugins/process` | `process.waitkill` | `processwaitkill` | `b` | `.int` | `handle=.int,timeout_ms=.int` | `lib/plugins/process/process.c` |
| `RXPA-recv390-xmitcleanup` | `lib/plugins/recv390` | `recv390.xmitcleanup` | `xmit_cleanup` | `b` | `.int` | `` | `lib/plugins/recv390/recv390.c` |
| `RXPA-recv390-xmitdirlist` | `lib/plugins/recv390` | `recv390.xmitdirlist` | `xmit_procdir` | `b` | `.int` | `memberpath=.string,array=.string[]` | `lib/plugins/recv390/recv390.c` |
| `RXPA-recv390-xmitextract` | `lib/plugins/recv390` | `recv390.xmitextract` | `xmit_extract` | `b` | `.int` | `memberpath=.string,member=.string` | `lib/plugins/recv390/recv390.c` |
| `RXPA-recv390-xmitunpack` | `lib/plugins/recv390` | `recv390.xmitunpack` | `xmit_unpack` | `b` | `.int` | `infile=.string` | `lib/plugins/recv390/recv390.c` |
| `RXPA-regex-hamming` | `lib/plugins/regex` | `regex.hamming` | `hamming` | `b` | `.int` | `string1=.string,string2=.string,uppercase=.int` | `lib/plugins/regex/regex.c` |
| `RXPA-regex-levenshtein` | `lib/plugins/regex` | `regex.levenshtein` | `levenshtein` | `b` | `.int` | `string1=.string,string2=.string` | `lib/plugins/regex/regex.c` |
| `RXPA-regex-rxcompile` | `lib/plugins/regex` | `regex.rxcompile` | `compile_pattern` | `b` | `.int` | `pattern=.string,flags=.int` | `lib/plugins/regex/regex.c` |
| `RXPA-regex-rxerror` | `lib/plugins/regex` | `regex.rxerror` | `get_error` | `b` | `.string` | `handle=.int` | `lib/plugins/regex/regex.c` |
| `RXPA-regex-rxfree` | `lib/plugins/regex` | `regex.rxfree` | `free_pattern` | `b` | `.int` | `handle=.int` | `lib/plugins/regex/regex.c` |
| `RXPA-regex-rxmatch` | `lib/plugins/regex` | `regex.rxmatch` | `match_pattern` | `b` | `.int` | `handle=.int,string=.string,flags=.int` | `lib/plugins/regex/regex.c` |
| `RXPA-rxmath-correl` | `lib/plugins/rxmath` | `rxmath.correl` | `correl` | `b` | `.float` | `expose arg1=.float[],expose arg2=.float[]` | `lib/plugins/rxmath/rxmath.c` |
| `RXPA-rxmath-covar` | `lib/plugins/rxmath` | `rxmath.covar` | `covar` | `b` | `.float` | `expose arg1=.float[],arg2=.float[]` | `lib/plugins/rxmath/rxmath.c` |
| `RXPA-rxmath-crc32` | `lib/plugins/rxmath` | `rxmath.crc32` | `crc32` | `b` | `.int` | `arg0=.string` | `lib/plugins/rxmath/rxmath.c` |
| `RXPA-rxmath-djb2` | `lib/plugins/rxmath` | `rxmath.djb2` | `djb2` | `b` | `.int` | `arg0=.string` | `lib/plugins/rxmath/rxmath.c` |
| `RXPA-rxmath-euler` | `lib/plugins/rxmath` | `rxmath.euler` | `euler` | `b` | `.float` | `` | `lib/plugins/rxmath/rxmath.c` |
| `RXPA-rxmath-fmod` | `lib/plugins/rxmath` | `rxmath.fmod` | `xfmod` | `b` | `.float` | `arg1=.float,arg2=.float` | `lib/plugins/rxmath/rxmath.c` |
| `RXPA-rxmath-fnv1a` | `lib/plugins/rxmath` | `rxmath.fnv1a` | `fnv1a` | `b` | `.int` | `arg0=.string` | `lib/plugins/rxmath/rxmath.c` |
| `RXPA-rxmath-hypot` | `lib/plugins/rxmath` | `rxmath.hypot` | `xhypot` | `b` | `.float` | `arg1=.float,arg2=.float` | `lib/plugins/rxmath/rxmath.c` |
| `RXPA-rxmath-inlinec` | `lib/plugins/rxmath` | `rxmath.inlinec` | `inlineC` | `b` | `.string` | `arg0=.string` | `lib/plugins/rxmath/rxmath.c` |
| `RXPA-rxmath-mean` | `lib/plugins/rxmath` | `rxmath.mean` | `mean` | `b` | `.float` | `expose a = .float[]` | `lib/plugins/rxmath/rxmath.c` |
| `RXPA-rxmath-murmur` | `lib/plugins/rxmath` | `rxmath.murmur` | `murmur` | `b` | `.int` | `arg0=.string, seed=.int` | `lib/plugins/rxmath/rxmath.c` |
| `RXPA-rxmath-pi` | `lib/plugins/rxmath` | `rxmath.pi` | `pi` | `b` | `.float` | `` | `lib/plugins/rxmath/rxmath.c` |
| `RXPA-rxmath-pow` | `lib/plugins/rxmath` | `rxmath.pow` | `xpow` | `b` | `.float` | `arg1=.float,arg2=.float` | `lib/plugins/rxmath/rxmath.c` |
| `RXPA-rxmath-regression` | `lib/plugins/rxmath` | `rxmath.regression` | `regression` | `b` | `.float` | `expose arg0=.float[],expose arg1=.float[],expose arg2=.float,expose arg3=.float` | `lib/plugins/rxmath/rxmath.c` |
| `RXPA-rxmath-stddev` | `lib/plugins/rxmath` | `rxmath.stddev` | `stddev` | `b` | `.float` | `expose a = .float[]` | `lib/plugins/rxmath/rxmath.c` |
| `RXPA-rxmath-uuid` | `lib/plugins/rxmath` | `rxmath.uuid` | `uuid` | `b` | `.string` | `` | `lib/plugins/rxmath/rxmath.c` |
| `RXPA-rxml-xmlattrat` | `lib/plugins/rxml` | `rxml.xmlattrat` | `xmlattrat` | `b` | `.int` | `instance=.int,index=.int,expose name=.string,expose value=.string` | `lib/plugins/rxml/rxml.c` |
| `RXPA-rxml-xmlattrcount` | `lib/plugins/rxml` | `rxml.xmlattrcount` | `xmlattrcount` | `b` | `.int` | `instance=.int` | `lib/plugins/rxml/rxml.c` |
| `RXPA-rxml-xmlbuild` | `lib/plugins/rxml` | `rxml.xmlbuild` | `xmlbuild` | `b` | `.string` | `root=.string, expose elements=.string[]` | `lib/plugins/rxml/rxml.c` |
| `RXPA-rxml-xmlerror` | `lib/plugins/rxml` | `rxml.xmlerror` | `xmlerror` | `b` | `.string` | `` | `lib/plugins/rxml/rxml.c` |
| `RXPA-rxml-xmlfind` | `lib/plugins/rxml` | `rxml.xmlfind` | `xmlfind` | `b` | `.int` | `tag=.string, expose results=.string[]` | `lib/plugins/rxml/rxml.c` |
| `RXPA-rxml-xmlflags` | `lib/plugins/rxml` | `rxml.xmlflags` | `xmlflags` | `b` | `.int` | `flags=.string` | `lib/plugins/rxml/rxml.c` |
| `RXPA-rxml-xmlgetattr` | `lib/plugins/rxml` | `rxml.xmlgetattr` | `xmlgetattr` | `b` | `.string` | `attr_name=.string,instance=.int` | `lib/plugins/rxml/rxml.c` |
| `RXPA-rxml-xmlparse` | `lib/plugins/rxml` | `rxml.xmlparse` | `xmlparse` | `b` | `.int` | `xml=.string` | `lib/plugins/rxml/rxml.c` |
| `RXPA-rxml-xmlremattr` | `lib/plugins/rxml` | `rxml.xmlremattr` | `xmlremattr` | `b` | `.int` | `attr_name=.string,instance=.int` | `lib/plugins/rxml/rxml.c` |
| `RXPA-rxml-xmlsetattr` | `lib/plugins/rxml` | `rxml.xmlsetattr` | `xmlsetattr` | `b` | `.int` | `attr_name=.string,instance=.int,value=.string` | `lib/plugins/rxml/rxml.c` |
| `RXPA-rxtcp-tcpclose` | `lib/plugins/rxtcp` | `rxtcp.tcpclose` | `tcpclose` | `b` | `.int` | `socket=.int` | `lib/plugins/rxtcp/rxtcp.c` |
| `RXPA-rxtcp-tcpflags` | `lib/plugins/rxtcp` | `rxtcp.tcpflags` | `tcpflags` | `b` | `.int` | `flags=.string` | `lib/plugins/rxtcp/rxtcp.c` |
| `RXPA-rxtcp-tcpopen` | `lib/plugins/rxtcp` | `rxtcp.tcpopen` | `tcpopen` | `b` | `.int` | `ip=.string,port=.int` | `lib/plugins/rxtcp/rxtcp.c` |
| `RXPA-rxtcp-tcpreceive` | `lib/plugins/rxtcp` | `rxtcp.tcpreceive` | `tcpreceive` | `b` | `.string` | `socket=.int,timeout=.int` | `lib/plugins/rxtcp/rxtcp.c` |
| `RXPA-rxtcp-tcpsend` | `lib/plugins/rxtcp` | `rxtcp.tcpsend` | `tcpsend` | `b` | `.int` | `socket=.int,message=.string` | `lib/plugins/rxtcp/rxtcp.c` |
| `RXPA-rxtcp-tcpserver` | `lib/plugins/rxtcp` | `rxtcp.tcpserver` | `tcpserver` | `b` | `.int` | `port=.int,expose sockets=.string[]` | `lib/plugins/rxtcp/rxtcp.c` |
| `RXPA-rxtcp-tcpwait` | `lib/plugins/rxtcp` | `rxtcp.tcpwait` | `tcpwait` | `b` | `.int` | `server=.int,timeout=.int,expose sockets=.string[]` | `lib/plugins/rxtcp/rxtcp.c` |
| `RXPA-rxtcp-wait` | `lib/plugins/rxtcp` | `rxtcp.wait` | `waitX` | `b` | `.int` | `timeout=.int` | `lib/plugins/rxtcp/rxtcp.c` |
| `RXPA-socket-socketaccept` | `lib/plugins/socket` | `socket.socketaccept` | `socketaccept` | `b` | `.int` | `sock=.int` | `lib/plugins/socket/socket.c` |
| `RXPA-socket-socketbind` | `lib/plugins/socket` | `socket.socketbind` | `socketbind` | `b` | `.int` | `sock=.int,ip=.string,port=.int` | `lib/plugins/socket/socket.c` |
| `RXPA-socket-socketclose` | `lib/plugins/socket` | `socket.socketclose` | `socketclose` | `b` | `.int` | `sock=.int` | `lib/plugins/socket/socket.c` |
| `RXPA-socket-socketconnect` | `lib/plugins/socket` | `socket.socketconnect` | `socketconnect` | `b` | `.int` | `sock=.int,host=.string,port=.int` | `lib/plugins/socket/socket.c` |
| `RXPA-socket-socketcreate` | `lib/plugins/socket` | `socket.socketcreate` | `socketcreate` | `b` | `.int` | `` | `lib/plugins/socket/socket.c` |
| `RXPA-socket-socketenabletls` | `lib/plugins/socket` | `socket.socketenabletls` | `socketenabletls` | `b` | `.int` | `sock=.int,hostname=.string` | `lib/plugins/socket/socket.c` |
| `RXPA-socket-socketisconnected` | `lib/plugins/socket` | `socket.socketisconnected` | `socketisconnected` | `b` | `.int` | `sock=.int` | `lib/plugins/socket/socket.c` |
| `RXPA-socket-socketkeepalive` | `lib/plugins/socket` | `socket.socketkeepalive` | `socketkeepalive` | `b` | `.int` | `sock=.int,enable=.int` | `lib/plugins/socket/socket.c` |
| `RXPA-socket-socketlasterror` | `lib/plugins/socket` | `socket.socketlasterror` | `socketlasterror` | `b` | `.string` | `sock=.int` | `lib/plugins/socket/socket.c` |
| `RXPA-socket-socketlisten` | `lib/plugins/socket` | `socket.socketlisten` | `socketlisten` | `b` | `.int` | `sock=.int,backlog=.int` | `lib/plugins/socket/socket.c` |
| `RXPA-socket-socketlocalinfo` | `lib/plugins/socket` | `socket.socketlocalinfo` | `socketlocalinfo` | `b` | `.string` | `sock=.int` | `lib/plugins/socket/socket.c` |
| `RXPA-socket-socketnodelay` | `lib/plugins/socket` | `socket.socketnodelay` | `socketnodelay` | `b` | `.int` | `sock=.int,enable=.int` | `lib/plugins/socket/socket.c` |
| `RXPA-socket-socketpeerinfo` | `lib/plugins/socket` | `socket.socketpeerinfo` | `socketpeerinfo` | `b` | `.string` | `sock=.int` | `lib/plugins/socket/socket.c` |
| `RXPA-socket-socketpendingbytes` | `lib/plugins/socket` | `socket.socketpendingbytes` | `socketpendingbytes` | `b` | `.int` | `sock=.int` | `lib/plugins/socket/socket.c` |
| `RXPA-socket-socketrecv` | `lib/plugins/socket` | `socket.socketrecv` | `socketrecv` | `b` | `.string` | `sock=.int,size=.int` | `lib/plugins/socket/socket.c` |
| `RXPA-socket-socketrecvline` | `lib/plugins/socket` | `socket.socketrecvline` | `socketrecvline` | `b` | `.string` | `sock=.int` | `lib/plugins/socket/socket.c` |
| `RXPA-socket-socketsend` | `lib/plugins/socket` | `socket.socketsend` | `socketsend` | `b` | `.int` | `sock=.int,data=.string` | `lib/plugins/socket/socket.c` |
| `RXPA-socket-socketsendall` | `lib/plugins/socket` | `socket.socketsendall` | `socketsendall` | `b` | `.int` | `sock=.int,data=.string` | `lib/plugins/socket/socket.c` |
| `RXPA-socket-socketsetblocking` | `lib/plugins/socket` | `socket.socketsetblocking` | `socketsetblocking` | `b` | `.int` | `sock=.int,blocking=.int` | `lib/plugins/socket/socket.c` |
| `RXPA-socket-socketsettimeout` | `lib/plugins/socket` | `socket.socketsettimeout` | `socketsettimeout` | `b` | `.int` | `sock=.int,timeout=.int` | `lib/plugins/socket/socket.c` |
| `RXPA-socket-socketshutdown` | `lib/plugins/socket` | `socket.socketshutdown` | `socketshutdown` | `b` | `.int` | `sock=.int,how=.int` | `lib/plugins/socket/socket.c` |
| `RXPA-stack-additem` | `lib/plugins/stack` | `stack.additem` | `additem` | `b` | `.int` | `expose list=.string[],ll_arg=.string` | `lib/plugins/stack/stack.c` |
| `RXPA-stack-createll` | `lib/plugins/stack` | `stack.createll` | `create` | `b` | `.int` | `expose list=.string[],ll_arg=.string` | `lib/plugins/stack/stack.c` |
| `RXPA-stack-delitem` | `lib/plugins/stack` | `stack.delitem` | `delitem` | `b` | `.int` | `expose list=.string[],ll_index=.int` | `lib/plugins/stack/stack.c` |
| `RXPA-stack-finditem` | `lib/plugins/stack` | `stack.finditem` | `finditem` | `b` | `.int` | `expose list=.string[],ll_arg=.string,ll_index=.int` | `lib/plugins/stack/stack.c` |
| `RXPA-stack-insertitem` | `lib/plugins/stack` | `stack.insertitem` | `insertitem` | `b` | `.int` | `expose list=.string[],ll_index=.int,ll_arg=.string` | `lib/plugins/stack/stack.c` |
| `RXPA-stack-listitems` | `lib/plugins/stack` | `stack.listitems` | `list` | `b` | `.int` | `expose list=.string[]` | `lib/plugins/stack/stack.c` |
| `RXPA-stack-moveitem` | `lib/plugins/stack` | `stack.moveitem` | `moveitem` | `b` | `.int` | `expose list=.string[],ll_from=.int,ll_to=.int` | `lib/plugins/stack/stack.c` |
| `RXPA-stack-pull` | `lib/plugins/stack` | `stack.pull` | `pull` | `b` | `.string` | `expose list=.string[]` | `lib/plugins/stack/stack.c` |
| `RXPA-stack-pullq` | `lib/plugins/stack` | `stack.pullq` | `pullq` | `b` | `.string` | `expose list=.string[]` | `lib/plugins/stack/stack.c` |
| `RXPA-stack-push` | `lib/plugins/stack` | `stack.push` | `additem` | `b` | `.int` | `expose list=.string[],ll_arg=.string` | `lib/plugins/stack/stack.c` |
| `RXPA-stack-queue` | `lib/plugins/stack` | `stack.queue` | `queue` | `b` | `.string` | `expose list=.string[],ll_arg=.string` | `lib/plugins/stack/stack.c` |
| `RXPA-stack-swapitem` | `lib/plugins/stack` | `stack.swapitem` | `swapitem` | `b` | `.int` | `expose list=.string[],ll_indx1=.int,ll_indx2=.int` | `lib/plugins/stack/stack.c` |
| `RXPA-strings-eval` | `lib/plugins/strings` | `strings.eval` | `eval` | `b` | `.int` | `expression=.string` | `lib/plugins/strings/strings.c` |
| `RXPA-strings-find-quoted` | `lib/plugins/strings` | `strings.find_quoted` | `fquoted` | `b` | `.int` | `string=.string, expose tokens=.string[],expose types=.string[]` | `lib/plugins/strings/strings.c` |
| `RXPA-strings-iff` | `lib/plugins/strings` | `strings.iff` | `iff` | `b` | `.string` | `expression=.string,true=.string,false=.string` | `lib/plugins/strings/strings.c` |
| `RXPA-strings-parse` | `lib/plugins/strings` | `strings.parse` | `PARSE` | `b` | `.int` | `input_string=.string, parse_template=.string, expose varnames=.string[], expose varvalues=.string[]` | `lib/plugins/strings/strings.c` |
| `RXPA-strings-set-items` | `lib/plugins/strings` | `strings.set_items` | `add_items` | `b` | `.int` | `expose target_array=.string[],items_string=.string` | `lib/plugins/strings/strings.c` |
| `RXPA-strings-xabbrev` | `lib/plugins/strings` | `strings.xabbrev` | `ABBREV` | `i` | `.int` | `target=.string,source=.string,min=.int` | `lib/plugins/strings/strings.c` |
| `RXPA-strings-xdelstr` | `lib/plugins/strings` | `strings.xdelstr` | `delstr` | `b` | `.string` | `string = .string,start=.int,length=.int` | `lib/plugins/strings/strings.c` |
| `RXPA-strings-xlastword` | `lib/plugins/strings` | `strings.xlastword` | `lastword` | `b` | `.string` | `string = .string,delim=.string` | `lib/plugins/strings/strings.c` |
| `RXPA-strings-xpos` | `lib/plugins/strings` | `strings.xpos` | `pos` | `i` | `.int` | `string = .string,substring=.string,offset=.int` | `lib/plugins/strings/strings.c` |
| `RXPA-strings-xstrip` | `lib/plugins/strings` | `strings.xstrip` | `strip` | `b` | `.string` | `string = .string,option=.string,char=.string` | `lib/plugins/strings/strings.c` |
| `RXPA-strings-xsubstr` | `lib/plugins/strings` | `strings.xsubstr` | `substring` | `b` | `.string` | `string = .string,position=.int,length=.int` | `lib/plugins/strings/strings.c` |
| `RXPA-strings-xsubword` | `lib/plugins/strings` | `strings.xsubword` | `SUBWORD` | `b` | `.string` | `string = .string,position=.int,numberOfWords=.int,delim=.string` | `lib/plugins/strings/strings.c` |
| `RXPA-strings-xword` | `lib/plugins/strings` | `strings.xword` | `word` | `b` | `.string` | `string = .string,indx=.int,delim=.string` | `lib/plugins/strings/strings.c` |
| `RXPA-strings-xwordindex` | `lib/plugins/strings` | `strings.xwordindex` | `wordindex` | `b` | `.int` | `string = .string,indx=.int,delim=.string` | `lib/plugins/strings/strings.c` |
| `RXPA-strings-xwordlen` | `lib/plugins/strings` | `strings.xwordlen` | `WORDLEN` | `i` | `.int` | `string = .string,wordNumber=.int,delim=.string` | `lib/plugins/strings/strings.c` |
| `RXPA-strings-xwordpos` | `lib/plugins/strings` | `strings.xwordpos` | `WORDPOS` | `i` | `.int` | `searchWord=.string,phrase=.string` | `lib/plugins/strings/strings.c` |
| `RXPA-strings-xwords` | `lib/plugins/strings` | `strings.xwords` | `words` | `b` | `.int` | `string = .string,delim=.string` | `lib/plugins/strings/strings.c` |
| `RXPA-system-beep` | `lib/plugins/system` | `system._beep` | `beep` | `b` | `.int` | `` | `lib/plugins/system/system.c` |
| `RXPA-system-listdir` | `lib/plugins/system` | `system._listdir` | `listdir` | `b` | `.int` | `file=.string,expose entries=.string[]` | `lib/plugins/system/system.c` |
| `RXPA-system-wait` | `lib/plugins/system` | `system._wait` | `waitX` | `b` | `.int` | `time=.int` | `lib/plugins/system/system.c` |
| `RXPA-system-append` | `lib/plugins/system` | `system.append` | `append_binary_file` | `b` | `.int` | `source=.string,target=.string` | `lib/plugins/system/system.c` |
| `RXPA-system-createdir` | `lib/plugins/system` | `system.createdir` | `createdir` | `b` | `.int` | `arg0=.string` | `lib/plugins/system/system.c` |
| `RXPA-system-deletefile` | `lib/plugins/system` | `system.deletefile` | `deletefile` | `b` | `.int` | `arg0=.string` | `lib/plugins/system/system.c` |
| `RXPA-system-getclipboard` | `lib/plugins/system` | `system.getclipboard` | `getclipboard` | `b` | `.string` | `` | `lib/plugins/system/system.c` |
| `RXPA-system-getcwd` | `lib/plugins/system` | `system.getCWD` | `getdir` | `b` | `.string` | `` | `lib/plugins/system/system.c` |
| `RXPA-system-getdir` | `lib/plugins/system` | `system.getdir` | `getdir` | `b` | `.string` | `` | `lib/plugins/system/system.c` |
| `RXPA-system-getglobal` | `lib/plugins/system` | `system.getglobal` | `getglobal` | `b` | `.string` | `key=.string` | `lib/plugins/system/system.c` |
| `RXPA-system-getloadpath` | `lib/plugins/system` | `system.getloadpath` | `getLoadPath` | `b` | `.string` | `` | `lib/plugins/system/system.c` |
| `RXPA-system-host` | `lib/plugins/system` | `system.host` | `getcomputer` | `b` | `.string` | `` | `lib/plugins/system/system.c` |
| `RXPA-system-lmodules` | `lib/plugins/system` | `system.lmodules` | `rxbin_modules` | `b` | `.int` | `source=.string` | `lib/plugins/system/system.c` |
| `RXPA-system-opsys` | `lib/plugins/system` | `system.opsys` | `opsys` | `b` | `.string` | `` | `lib/plugins/system/system.c` |
| `RXPA-system-parse` | `lib/plugins/system` | `system.parse` | `parse` | `b` | `.int` | `string=.string,pattern=.string,expose variable=.string[],expose value=.string[]` | `lib/plugins/system/system.c` |
| `RXPA-system-parsex` | `lib/plugins/system` | `system.parsex` | `parsex` | `b` | `.int` | `string=.string,pattern=.string,expose entries=.string[]` | `lib/plugins/system/system.c` |
| `RXPA-system-pipecancel` | `lib/plugins/system` | `system.pipecancel` | `pipecancel` | `b` | `.int` | `proc=.int` | `lib/plugins/system/system.c` |
| `RXPA-system-pipeclose` | `lib/plugins/system` | `system.pipeclose` | `pipeclose` | `b` | `.int` | `proc=.int` | `lib/plugins/system/system.c` |
| `RXPA-system-pipecreate` | `lib/plugins/system` | `system.pipecreate` | `pipecreate` | `b` | `.int` | `` | `lib/plugins/system/system.c` |
| `RXPA-system-pipeget` | `lib/plugins/system` | `system.pipeget` | `pipeget` | `b` | `.int` | `proc=.int, expose array=.string[]` | `lib/plugins/system/system.c` |
| `RXPA-system-pipesend` | `lib/plugins/system` | `system.pipesend` | `piperun` | `b` | `.int` | `proc=.int,cmd=.string` | `lib/plugins/system/system.c` |
| `RXPA-system-pipestatus` | `lib/plugins/system` | `system.pipestatus` | `pipestatus` | `b` | `.int` | `proc=.int` | `lib/plugins/system/system.c` |
| `RXPA-system-pipewait` | `lib/plugins/system` | `system.pipewait` | `pipewait` | `b` | `.int` | `proc=.int,mwait=10000` | `lib/plugins/system/system.c` |
| `RXPA-system-removedir` | `lib/plugins/system` | `system.removedir` | `removedir` | `b` | `.int` | `arg0=.string` | `lib/plugins/system/system.c` |
| `RXPA-system-renamefile` | `lib/plugins/system` | `system.renamefile` | `renamefileP` | `b` | `.int` | `arg0=.string,arg1=.string` | `lib/plugins/system/system.c` |
| `RXPA-system-setclipboard` | `lib/plugins/system` | `system.setclipboard` | `setclipboard` | `b` | `.int` | `arg0=.string` | `lib/plugins/system/system.c` |
| `RXPA-system-setdir` | `lib/plugins/system` | `system.setdir` | `setdir` | `b` | `.int` | `arg0=.string` | `lib/plugins/system/system.c` |
| `RXPA-system-setglobal` | `lib/plugins/system` | `system.setglobal` | `setglobal` | `b` | `.int` | `key=.string,value=.string` | `lib/plugins/system/system.c` |
| `RXPA-system-sysuptime` | `lib/plugins/system` | `system.sysuptime` | `sysuptime` | `b` | `.int` | `` | `lib/plugins/system/system.c` |
| `RXPA-system-testdir` | `lib/plugins/system` | `system.testdir` | `testdir` | `b` | `.int` | `arg0=.string` | `lib/plugins/system/system.c` |
| `RXPA-system-testfile` | `lib/plugins/system` | `system.testfile` | `testfile` | `b` | `.int` | `arg0=.string` | `lib/plugins/system/system.c` |
| `RXPA-system-userid` | `lib/plugins/system` | `system.userid` | `getuser` | `b` | `.string` | `` | `lib/plugins/system/system.c` |
| `RXPA-treemap-stemcontainskey` | `lib/plugins/treemap` | `treemap.stemcontainskey` | `stem_containsKey` | `b` | `.int` | `token=.int, key=.string` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-stemcreate` | `lib/plugins/treemap` | `treemap.stemcreate` | `stem_create` | `b` | `.int` | `items=.int, root=.string` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-stemfree` | `lib/plugins/treemap` | `treemap.stemfree` | `stem_destroy` | `b` | `.int` | `token=.int` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-stemget` | `lib/plugins/treemap` | `treemap.stemget` | `stem_get` | `b` | `.string` | `token=.int, key=.string` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-stemiterate` | `lib/plugins/treemap` | `treemap.stemiterate` | `stem_iterate` | `b` | `.int` | `token=.int, expose keys=.string[], expose values=.string[]` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-stemitercreate` | `lib/plugins/treemap` | `treemap.stemitercreate` | `stem_itercreate` | `b` | `.int` | `token=.int` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-stemiterfree` | `lib/plugins/treemap` | `treemap.stemiterfree` | `stem_iterfree` | `b` | `.int` | `token=.int` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-stemiterhasnext` | `lib/plugins/treemap` | `treemap.stemiterhasnext` | `stem_iterhasnext` | `b` | `.int` | `token=.int` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-stemiternext` | `lib/plugins/treemap` | `treemap.stemiternext` | `stem_iternext` | `b` | `.string` | `token=.int` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-stemitervalue` | `lib/plugins/treemap` | `treemap.stemitervalue` | `stem_itervalue` | `b` | `.string` | `token=.int` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-stemput` | `lib/plugins/treemap` | `treemap.stemput` | `stem_put` | `b` | `.int` | `token=.int, key=.string, value=.string` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-stemremove` | `lib/plugins/treemap` | `treemap.stemremove` | `stem_remove` | `b` | `.int` | `token=.int, key=.string` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-stemsize` | `lib/plugins/treemap` | `treemap.stemsize` | `stem_hi` | `b` | `.int` | `token=.int` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-stemstats` | `lib/plugins/treemap` | `treemap.stemstats` | `stem_stats` | `b` | `.int` | `token=.int` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-tmcontainskey` | `lib/plugins/treemap` | `treemap.tmcontainskey` | `tmap_containsKey` | `b` | `.int` | `map=.int, key=.string` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-tmcontainsvalue` | `lib/plugins/treemap` | `treemap.tmcontainsvalue` | `tmap_containsValue` | `b` | `.string` | `map=.string, value=.string` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-tmcreate` | `lib/plugins/treemap` | `treemap.tmcreate` | `tmap_create` | `b` | `.int` | `name=''` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-tmdump` | `lib/plugins/treemap` | `treemap.tmdump` | `tmap_dump` | `b` | `.int` | `map=.int, expose keys=.string[], expose values=.string[]` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-tmfirstkey` | `lib/plugins/treemap` | `treemap.tmfirstkey` | `tmap_firstkey` | `b` | `.string` | `map=.int` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-tmfree` | `lib/plugins/treemap` | `treemap.tmfree` | `tmap_free` | `b` | `.int` | `map=.int` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-tmget` | `lib/plugins/treemap` | `treemap.tmget` | `tmap_get` | `b` | `.string` | `map=.int, key=.string` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-tmhaskey` | `lib/plugins/treemap` | `treemap.tmhaskey` | `tmap_haskey` | `b` | `.int` | `map=.int, key=.string` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-tmhasvalue` | `lib/plugins/treemap` | `treemap.tmhasvalue` | `tmap_hasvalue` | `b` | `.string` | `map=.string, value=.string` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-tmitercreate` | `lib/plugins/treemap` | `treemap.tmitercreate` | `tmap_itercreate` | `b` | `.int` | `map=.int` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-tmiterfree` | `lib/plugins/treemap` | `treemap.tmiterfree` | `tmap_iterfree` | `b` | `.int` | `token=.int` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-tmiterhasnext` | `lib/plugins/treemap` | `treemap.tmiterhasnext` | `tmap_iterhasnext` | `b` | `.int` | `token=.int` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-tmiternext` | `lib/plugins/treemap` | `treemap.tmiternext` | `tmap_iternext` | `b` | `.string` | `token=.int` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-tmkeys` | `lib/plugins/treemap` | `treemap.tmkeys` | `tmap_keys` | `b` | `.int` | `map=.int, expose list=.string[]` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-tmlastkey` | `lib/plugins/treemap` | `treemap.tmlastkey` | `tmap_lastkey` | `b` | `.string` | `map=.int` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-tmlookup` | `lib/plugins/treemap` | `treemap.tmlookup` | `tmap_lookup` | `b` | `.int` | `name=.string` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-tmput` | `lib/plugins/treemap` | `treemap.tmput` | `tmap_put` | `b` | `.int` | `map=.int, key=.string, value=.string` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-tmremove` | `lib/plugins/treemap` | `treemap.tmremove` | `tmap_remove` | `b` | `.int` | `map=.int, key=.string` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-tmsize` | `lib/plugins/treemap` | `treemap.tmsize` | `tmap_size` | `b` | `.int` | `map=.int` | `lib/plugins/treemap/treemap.c` |
| `RXPA-treemap-tmtostring` | `lib/plugins/treemap` | `treemap.tmtostring` | `tmap_tostring` | `b` | `.string` | `map=.int` | `lib/plugins/treemap/treemap.c` |
| `RXPA-precomp-copy-arrayx` | `preprocessor` | `precomp.copy_arrayx` | `copy_array` | `b` | `.int` | `expose a = .string[],b=.string[],from=.int,tto=.int` | `preprocessor/precomp.c` |
| `RXPA-precomp-drop-arrayx` | `preprocessor` | `precomp.drop_arrayX` | `drop_array` | `b` | `.int` | `expose a = .string[]` | `preprocessor/precomp.c` |
| `RXPA-precomp-ffindx` | `preprocessor` | `precomp.ffindx` | `ffind` | `b` | `.int` | `expose array=.string[],pos=.int,str1=.string` | `preprocessor/precomp.c` |
| `RXPA-precomp-find-quotedx` | `preprocessor` | `precomp.find_quotedx` | `fquoted` | `b` | `.int` | `string=.string, expose tokens=.string[],expose types=.string[]` | `preprocessor/precomp.c` |
| `RXPA-precomp-fpos` | `preprocessor` | `precomp.fpos` | `fpos` | `b` | `.int` | `string = .string,substring=.string,offset=.int` | `preprocessor/precomp.c` |
| `RXPA-precomp-fsearch` | `preprocessor` | `precomp.fsearch` | `fsearch` | `b` | `.int` | `expose array=.string[],pos=.int,str1=.string,str2=.string,str3=.string,expose item=.int` | `preprocessor/precomp.c` |
| `RXPA-precomp-hasmacro` | `preprocessor` | `precomp.hasmacro` | `hasmacro` | `b` | `.int` | `line=.string,maclist=.string[],from=.int` | `preprocessor/precomp.c` |
| `RXPA-precomp-insert-arrayx` | `preprocessor` | `precomp.insert_arrayX` | `insert_array` | `b` | `.int` | `expose a = .string[],from=.int,new=.int` | `preprocessor/precomp.c` |
| `RXPA-precomp-insertatcx` | `preprocessor` | `precomp.insertatcx` | `insertat` | `b` | `.string` | `haystack = .string,needle=.string,offset=.int,len=.int` | `preprocessor/precomp.c` |
| `RXPA-precomp-list-array` | `preprocessor` | `precomp.list_array` | `list_array` | `b` | `.void` | `expose a = .string[],from=.int,tto=.int,hdr=.string` | `preprocessor/precomp.c` |
| `RXPA-precomp-readallx` | `preprocessor` | `precomp.readallx` | `readall` | `b` | `.int` | `expose array=.string[],expose file=.string,arg2=.int` | `preprocessor/precomp.c` |
| `RXPA-precomp-safe-quote` | `preprocessor` | `precomp.safe_quote` | `safe_quote` | `b` | `.string` | `expose string=.string` | `preprocessor/precomp.c` |
| `RXPA-precomp-search-arrayx` | `preprocessor` | `precomp.search_arrayx` | `search_array` | `b` | `.int` | `expose a = .string[],needle=.string,startrow=.int,match=.int` | `preprocessor/precomp.c` |
| `RXPA-precomp-shell-sortx` | `preprocessor` | `precomp.shell_sortX` | `shell_sort` | `b` | `.void` | `expose a = .string[], offset=.int, order=.string` | `preprocessor/precomp.c` |
| `RXPA-precomp-sort-bylen` | `preprocessor` | `precomp.sort_bylen` | `sort_bylen` | `b` | `.void` | `expose a = .string[],expose b = .string[],expose c = .string[],expose d = .int[]` | `preprocessor/precomp.c` |
| `RXPA-precomp-splitargs` | `preprocessor` | `precomp.splitargs` | `splitargs` | `b` | `.int` | `string=.string, expose tokens=.string[]` | `preprocessor/precomp.c` |
| `RXPA-precomp-stemquote` | `preprocessor` | `precomp.stemquote` | `stemquote` | `b` | `.string` | `path=.string` | `preprocessor/precomp.c` |
| `RXPA-precomp-templistx` | `preprocessor` | `precomp.templistx` | `templist` | `b` | `.string` | `mode=.string,index=.int,string=.string` | `preprocessor/precomp.c` |
| `RXPA-precomp-writeall` | `preprocessor` | `precomp.writeall` | `writeall` | `b` | `.int` | `expose array=.string[],file=.string,arg2=.int` | `preprocessor/precomp.c` |
| `RXPA-precomp-xlogx` | `preprocessor` | `precomp.xlogx` | `xlog` | `b` | `.void` | `string = .string` | `preprocessor/precomp.c` |

Discovered rows: **285**.
