/* REXX LEVEL B SYSTEM FUNCTIONS */
options levelb
namespace _rxsysb expose _open _close _exit
#import rxfnsb

/* EXIT Function */
_exit: procedure
    arg return_value = "0" /* A string (as per the REXX standard) which defaults to 0 */

    ret_code = .int

    /* We can call exit handlers here */

    /* TODO Catch conversion error signal (when implemented!) */
    ret_code =  return_value

    assembler exit ret_code /* Real Exit! */
    return /* Never Reached */

/* Return the FILEID of a file - Opening it as needed */
_open: procedure = .int expose _open_file_names _open_file_ids _open_file_modes
    arg name = .string, mode = .string

    _open_file_names = .string[]
    _open_file_ids = .int[]
    _open_file_modes = .string[]
    fileid = 0
    empty_slot = 0

    if mode = 'r' then c_mode = "r"
    else c_mode = 'a'

    do i = 1 to _open_file_names.0 while fileid = 0
        if name = _open_file_names.i then do
            if mode <> _open_file_modes.i then do
                /* need to close the file so it can be opened again in the right mode */
                assembler fclose rc,_open_file_ids.i
                _open_file_ids.i = 0
                _open_file_names.i = ""
                _open_file_modes.i = ""
            end
            else fileid = _open_file_ids.i
        end
        if _open_file_names.i = "" then empty_slot = i
    end

    if fileid = 0 then do
        if empty_slot = 0 then empty_slot = _open_file_names.0 + 1

        assembler fopen fileid,name,c_mode

        _open_file_names.empty_slot = name
        _open_file_modes.empty_slot = mode
        _open_file_ids.empty_slot = fileid
    end

    return fileid

/* Close a file */
_close: procedure = .void expose _open_file_names _open_file_ids _open_file_modes
    arg name = .string

    found_slot = 0

    do i = 1 to _open_file_names.0 while found_slot = 0
        if name = _open_file_names.i then found_slot = i
    end

    rc = 0
    if found_slot > 0 then do
        assembler fclose rc,_open_file_ids.found_slot
        _open_file_ids.found_slot = 0
        _open_file_names.found_slot = ""
        _open_file_modes.found_slot = ""
        if rc <> 0 then say "ERROR _close() - rc =" rc
    end
    else say "ERROR _close() - file not opened"

    return