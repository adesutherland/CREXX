 /************************************************************************
 *.  Global_Vars
 ************************************************************************/
 Global_Vars:

    !DatatypeResult=''
    !AllBlanks=' '
    !Pool = 13 /* Say */
    !Limit_Digits=999
    !Limit_String = 999999
    !Limit_Literal = 250
    !Limit_Name = 250
    !Limit_EnvironmentName = 250
    !Limit_ExponentDigits = 9
    !Digits. = 777 /* Will be fixed. */
    !level = 99 /* Perhaps */
    !ArgExists.='0'
    !Arg.=''
    !ArgExists.!Level.0=2 /* Perhaps */
    !ArgExists.!Level.2='1' /* Perhaps */
    !Arg.!Level.2='Whatever' /* Perhaps */
    index = 0
    condition = ''
    !StartTime.=''
    !ClauseTime.=''
    !Condition.!Level = ''
    !ConditionDescription.!Level = ''
    !ConditionInstruction.!Level = ''
    !ConditionState.!Level = ''

    !Config_Digits.=9 /* Pretend Config_Digits call */
    !tracing.!Level = trace()
    !Env_Name.!Level= address()
    !Env_SourceType.!Level='ECST'
    !Env_SourcePosition.!Level='ECSP'
    !Env_SourceName.!Level='ECSN'
    !Env_DestinationType.!Level='ECDT'
    !Env_DestinationPosition.!Level='ECDP'
    !Env_DestinationName.!Level='ECDN'

    !Charin_Position.=1
    !Charout_Position.=1
    !Linein_Position.=1
    !Lineout_Position.=1
    !enabling.!Level.ERROR    = 'OFF'
    !enabling.!Level.FAILURE  = 'OFF'
    !enabling.!Level.HALT     = 'OFF'
    !enabling.!Level.NOVALUE  = 'OFF'
    !enabling.!Level.NOTREADY = 'OFF'
    !enabling.!Level.SYNTAX   = 'OFF'
    !condition.!Level         = ''
    !description.!Level       = ''
    !instruction.!Level       = ''
    !state.!Level             = ''
    !trapname.!Level.ERROR    = 'ERROR'
    !trapname.!Level.FAILURE  = 'FAILURE'
    !trapname.level.HALT     = 'HALT'
    !trapname.!Level.NOVALUE  = 'NOVALUE'
    !trapname.!Level.NOTREADY = 'NOTREADY'
    !trapname.!Level.SYNTAX   = 'SYNTAX'

   !ErrorText.    = ''

   !ErrorText.0.1 = 'Error <value> running <source>, line <linenumber>:'
   !ErrorText.0.2 = 'Error <value> in interactive trace:'
   !ErrorText.0.3 = 'Interactive trace.  "Trace Off" to end debug. ',
                    'ENTER to continue.'
   !ErrorText.2   = 'Failure during finalization'
   !ErrorText.2.1 = 'Failure during finalization: <description>'

   !ErrorText.3   = 'Failure during initialization'
   !ErrorText.3.1 = 'Failure during initialization: <description>'

   !ErrorText.4   = 'Program interrupted'
   !ErrorText.4.1 = 'Program interrupted with HALT condition'

   !ErrorText.5   = 'System resources exhausted'
   !ErrorText.5.1 = 'System resources exhausted: <description>'

   !ErrorText.6   = 'Unmatched "/*" or quote'
   !ErrorText.6.1 = 'Unmatched comment delimiter ("/*")'
   !ErrorText.6.2 = "Unmatched single quote (')"
   !ErrorText.6.3 = 'Unmatched double quote (")'

   !ErrorText.7   = 'WHEN or OTHERWISE expected'
   !ErrorText.7.1 = 'SELECT on line <linenumber> requires WHEN;',
                    'found "<token>"'
   !ErrorText.7.2 = 'SELECT on line <linenumber> requires WHEN, OTHERWISE,',
                    'or END; found "<token>"'
   !ErrorText.7.3 = 'All WHEN expressions of SELECT on line <linenumber> are',
                    'false; OTHERWISE expected'

   !ErrorText.8   = 'Unexpected THEN or ELSE'
   !ErrorText.8.1 = 'THEN has no corresponding IF or WHEN clause'
   !ErrorText.8.2 = 'ELSE has no corresponding THEN clause'

   !ErrorText.9   = 'Unexpected WHEN or OTHERWISE'
   !ErrorText.9.1 = 'WHEN has no corresponding SELECT'
   !ErrorText.9.2 = 'OTHERWISE has no corresponding SELECT'

   !ErrorText.10  = 'Unexpected or unmatched END'
   !ErrorText.10.1= 'END has no corresponding DO or SELECT'
   !ErrorText.10.2= 'END corresponding to DO on line <linenumber>',
                    'must have a symbol following that matches',
                    'the control variable (or no symbol);',
                    'found "<token>"'
   !ErrorText.10.3= 'END corresponding to DO on line <linenumber>',
                    'must not have a symbol following it because',
                    'there is no control variable;',
                    'found "<token>"'
   !ErrorText.10.4= 'END corresponding to SELECT on line <linenumber>',
                    'must not have a symbol following;',
                    'found "<token>"'
   !ErrorText.10.5= 'END must not immediately follow THEN'
   !ErrorText.10.6= 'END must not immediately follow ELSE'

   !ErrorText.13  = 'Invalid character in program'
   !ErrorText.13.1= 'Invalid character in program "<character>"',
                    "('<hex-encoding>'X)"

   !ErrorText.14  = 'Incomplete DO/SELECT/IF'
   !ErrorText.14.1= 'DO instruction requires a matching END'
   !ErrorText.14.2= 'SELECT instruction requires a matching END'
   !ErrorText.14.3= 'THEN requires a following instruction'
   !ErrorText.14.4= 'ELSE requires a following instruction'

   !ErrorText.15  = 'Invalid hexadecimal or binary string'
   !ErrorText.15.1= 'Invalid location of blank in position',
                    '<position> in hexadecimal string'
   !ErrorText.15.2= 'Invalid location of blank in position',
                    '<position> in binary string'
   !ErrorText.15.3= 'Only 0-9, a-f, A-F, and blank are valid in a',
                    'hexadecimal string; found "<char>"'
   !ErrorText.15.4= 'Only 0, 1, and blank are valid in a',
                    'binary string; found "<char>"'

   !ErrorText.16  = 'Label not found'
   !ErrorText.16.1= 'Label "<name>" not found'
   !ErrorText.16.2= 'Cannot SIGNAL to label "<name>" because it is',
                    'inside an IF, SELECT or DO group'
   !ErrorText.16.3= 'Cannot invoke label "<name>" because it is',
                    'inside an IF, SELECT or DO group'

   !ErrorText.17  = 'Unexpected PROCEDURE'
   !ErrorText.17.1= 'PROCEDURE is valid only when it is the first',
                    'instruction executed after an internal CALL',
                    'or function invocation'

   !ErrorText.18  = 'THEN expected'
   !ErrorText.18.1= 'IF keyword on line <linenumber> requires',
                    'matching THEN clause; found "<token>"'
   !ErrorText.18.2= 'WHEN keyword on line <linenumber> requires',
                    'matching THEN clause; found "<token>"'

   !ErrorText.19  = 'String or symbol expected'
   !ErrorText.19.1= 'String or symbol expected after ADDRESS keyword;',
                    'found "<token>"'
   !ErrorText.19.2= 'String or symbol expected after CALL keyword;',
                    'found "<token>"'
   !ErrorText.19.3= 'String or symbol expected after NAME keyword;',
                    'found "<token>"'
   !ErrorText.19.4= 'String or symbol expected after SIGNAL keyword;',
                    'found "<token>"'
   !ErrorText.19.6= 'String or symbol expected after TRACE keyword;',
                    'found "<token>"'
   !ErrorText.19.7= 'Symbol expected in parsing pattern;',
                    'found "<token>"'

   !ErrorText.20  = 'Name expected'
   !ErrorText.20.1= 'Name required; found "<token>"'
   !ErrorText.20.2= 'Found "<token>" where only a name is valid'

   !ErrorText.21  = 'Invalid data on end of clause'
   !ErrorText.21.1= 'The clause ended at an unexpected token;',
                         'found "<token>"'

   !ErrorText.22  = 'Invalid character string'
   !ErrorText.22.1= "Invalid character string '<hex-encoding>'X"

   !ErrorText.23  = 'Invalid data string'
   !ErrorText.23.1= "Invalid data string '<hex-encoding>'X"

   !ErrorText.24  = 'Invalid TRACE request'
   !ErrorText.24.1= 'TRACE request must be one character of',
                    '"ACEFILNOR"; found "<value>"'

   !ErrorText.25  = 'Invalid sub-keyword found'
   !ErrorText.25.1= 'CALL ON must be followed by one of the',
                    'keywords <keywords>; found "<token>"'
   !ErrorText.25.2= 'CALL OFF must be followed by one of the',
                    'keywords <keywords>; found "<token>"'
   !ErrorText.25.3= 'SIGNAL ON must be followed by one of the',
                    'keywords <keywords>; found "<token>"'
   !ErrorText.25.4= 'SIGNAL OFF must be followed by one of the',
                    'keywords <keywords>; found "<token>"'
   !ErrorText.25.5= 'ADDRESS WITH must be followed by one of the',
                    'keywords <keywords>; found "<token>"'
   !ErrorText.25.6= 'INPUT must be followed by one of the',
                    'keywords <keywords>; found "<token>"'
   !ErrorText.25.7= 'OUTPUT must be followed by one of the',
                    'keywords <keywords>; found "<token>"'
   !ErrorText.25.8= 'APPEND must be followed by one of the',
                    'keywords <keywords>; found "<token>"'
   !ErrorText.25.9= 'REPLACE must be followed by one of the',
                    'keywords <keywords>; found "<token>"'
   !ErrorText.25.11='NUMERIC FORM must be followed by one of the',
                    'keywords <keywords>; found "<token>"'
   !ErrorText.25.12='PARSE must be followed by one of the',
                    'keywords <keywords>; found "<token>"'
   !ErrorText.25.13='UPPER must be followed by one of the',
                    'keywords <keywords>; found "<token>"'
   !ErrorText.25.14='ERROR must be followed by one of the',
                    'keywords <keywords>; found "<token>"'
   !ErrorText.25.15='NUMERIC must be followed by one of the',
                    'keywords <keywords>; found "<token>"'
   !ErrorText.25.16='FOREVER must be followed by one of the',
                    'keywords <keywords> or nothing; found "<token>"'
   !ErrorText.25.17='PROCEDURE must be followed by the keyword',
                    'EXPOSE or nothing; found "<token>"'

   !ErrorText.26  = 'Invalid whole number'
   !ErrorText.26.1= 'Whole numbers must fit within current DIGITS',
                    'setting(<value>); found "<value>"'
   !ErrorText.26.2= 'Value of repetition count expression in DO instruction',
                    'must be zero or a positive whole number;',
                    'found "<value>"'
   !ErrorText.26.3= 'Value of FOR expression in DO instruction',
                    'must be zero or a positive whole number;',
                    'found "<value>"'
   !ErrorText.26.4= 'Positional pattern of parsing template',
                    'must be a whole number; found "<value>"'
   !ErrorText.26.5= 'NUMERIC DIGITS value',
                    'must be a positive whole number; found "<value>"'
   !ErrorText.26.6= 'NUMERIC FUZZ value',
                    'must be zero or a positive whole number;',
                    'found "<value>"'
   !ErrorText.26.7= 'Number used in TRACE setting',
                    'must be a whole number; found "<value>"'
   !ErrorText.26.8= 'Operand to right of the power operator ("**")',
                    'must be a whole number; found "<value>"'
   !ErrorText.26.11='Result of <value> % <value> operation would need',
                    'exponential notation at current NUMERIC DIGITS <value>'
   !ErrorText.26.12='Result of % operation used for <value> // <value>',
                    'operation would need',
                    'exponential notation at current NUMERIC DIGITS <value>'

   !ErrorText.27  = 'Invalid DO syntax'
   !ErrorText.27.1= 'Invalid use of keyword <token> in DO clause'

   !ErrorText.28  = 'Invalid LEAVE or ITERATE'
   !ErrorText.28.1= 'LEAVE is valid only within a repetitive DO loop'
   !ErrorText.28.2= 'ITERATE is valid only within a repetitive DO loop'
   !ErrorText.28.3= 'Symbol following LEAVE ("<token>") must',
                    'either match control variable of a current',
                    'DO loop or be omitted'
   !ErrorText.28.4= 'Symbol following ITERATE ("<token>") must',
                    'either match control variable of a current',
                    'DO loop or be omitted'

   !ErrorText.29  = 'Environment name longer than',
                    !Limit_EnvironmentName 'characters'
   !ErrorText.29.1= 'Environment name exceeds',
                    !Limit_EnvironmentName 'characters "<name>"'

   !ErrorText.30  = 'Name or string too long'
   !ErrorText.30.1= 'Name exceeds' !Limit_Name 'characters'
   !ErrorText.30.2= 'Literal string exceeds' !Limit_Literal 'characters'

   !ErrorText.31  = 'Name starts with number or "."'
   !ErrorText.31.1= 'A value cannot be assigned to a number;',
                    'found "<token>"'
   !ErrorText.31.2= 'Variable symbol must not start with a number;',
                    'found "<token>"'
   !ErrorText.31.3= 'Variable symbol must not start with a ".";',
                    'found "<token>"'

   !ErrorText.33  = 'Invalid expression result'
   !ErrorText.33.1= 'Value of NUMERIC DIGITS ("<value>")',
                    'must exceed value of NUMERIC FUZZ "(<value>)"'
   !ErrorText.33.2= 'Value of NUMERIC DIGITS ("<value>")',
                    'must not exceed' !Limit_Digits
   !ErrorText.33.6= 'Result of expression following NUMERIC FORM',
                    'must start with "E" or "S"; found "<value>"'

   !ErrorText.34  = 'Logical value not "0" or "1"'
   !ErrorText.34.1= 'Value of expression following IF keyword',
                    'must be exactly "0" or "1"; found "<value>"'
   !ErrorText.34.2= 'Value of expression following WHEN keyword',
                    'must be exactly "0" or "1"; found "<value>"'
   !ErrorText.34.3= 'Value of expression following WHILE keyword',
                    'must be exactly "0" or "1"; found "<value>"'
   !ErrorText.34.4= 'Value of expression following UNTIL keyword',
                    'must be exactly "0" or "1"; found "<value>"'
   !ErrorText.34.5= 'Value of expression to left',
                    'of logical operator "<operator>"',
                    'must be exactly "0" or "1"; found "<value>"'
   !ErrorText.34.6= 'Value of expression to right',
                    'of logical operator "<operator>"',
                    'must be exactly "0" or "1"; found "<value>"'

   !ErrorText.35  = 'Invalid expression'
   !ErrorText.35.1= 'Invalid expression detected at "<token>"'

   !ErrorText.36  = 'Unmatched "(" in expression'

   !ErrorText.37  = 'Unexpected "," or ")"'
   !ErrorText.37.1= 'Unexpected ","'
   !ErrorText.37.2= 'Unmatched ")" in expression'

   !ErrorText.38  = 'Invalid template or pattern'
   !ErrorText.38.1= 'Invalid parsing template detected at "<token>"'
   !ErrorText.38.2= 'Invalid parsing position detected at "<token>"'
   !ErrorText.38.3= 'PARSE VALUE instruction requires WITH keyword'

   !ErrorText.40  = 'Incorrect call to routine'
   !ErrorText.40.1= 'External routine <name> failed'
   !ErrorText.40.3= 'Not enough arguments in invocation of <bif>;',
                    'minimum expected is <argnumber>'
   !ErrorText.40.4= 'Too many arguments in invocation of <bif>;',
                    'maximum expected is <argnumber>'
   !ErrorText.40.5= 'Missing argument in invocation of <bif>;',
                    'argument <argnumber> is required'
   !ErrorText.40.9= '<bif> argument <argnumber>',
                    'Exponent exceeds' !Limit_ExponentDigits 'digits'
   !ErrorText.40.11='<bif> argument <argnumber>',
                    'must be a number; found "<value>"'
   !ErrorText.40.12='<bif> argument <argnumber>',
                    'must be a whole number; found "<value>"'
   !ErrorText.40.13='<bif> argument <argnumber>',
                    'must be zero or positive; found "<value>"'
   !ErrorText.40.14='<bif> argument <argnumber>',
                    'must be positive; found "<value>"'
   !ErrorText.40.15='<bif> argument <argnumber>',
                    'must fit in <value> digits; found "<value>"'
   !ErrorText.40.16='<bif> argument <argnumber>',
                    'Requires a whole number fitting within',
                    'DIGITS(<value>); found "<value>"'
   !ErrorText.40.17='<bif> argument 1',
                    'must have an integer part in the range 0:90 and a',
                    'decimal part no larger than .9; found "<value>"'
   !ErrorText.40.18='<bif> conversion must',
                    'have a year in the range 0001 to 9999'
   !ErrorText.40.19='<bif> argument 2, "<value>", is not in the format',
                     'described by argument 3, "<value>"'
   !ErrorText.40.21='<bif> argument <argnumber> must not be null'
   !ErrorText.40.22='<bif> argument <argnumber>',
                    'must be a single character or null;',
                    'found "<value>"'
   !ErrorText.40.23='<bif> argument <argnumber>',
                    'must be a single character; found "<value>"'
   !ErrorText.40.24='<bif> argument 1',
                    'must be a binary string; found "<value>"'
   !ErrorText.40.25='<bif> argument 1',
                    'must be a hexadecimal string; found "<value>"'
   !ErrorText.40.26='<bif> argument 1',
                    'must be a valid symbol; found "<value>"'
   !ErrorText.40.27='<bif> argument 1',
                    'must be a valid stream name; found "<value>"'
   !ErrorText.40.28='<bif> argument <argnumber>',
                    'must start with one of "<optionslist>";',
                    'found "<value>"'
   !ErrorText.40.29='<bif> conversion to format "<value>" is not allowed'
   !ErrorText.40.31='<bif> argument 1 ("<value>") must not exceed 100000'
   !ErrorText.40.32='<bif> the difference between argument 1 ("<value>") and',
                    'argument 2 ("<value>") must not exceed 100000'
   !ErrorText.40.33='<bif> argument 1 ("<value>") must be less than',
                    'or equal to argument 2 ("<value>")'
   !ErrorText.40.34='<bif> argument 1 ("<value>") must be less than',
                    'or equal to the number of lines',
                    'in the program (<sourceline()>)'
   !ErrorText.40.35='<bif> argument 1 cannot be expressed as a whole number;',
                    'found "<value>"'
   !ErrorText.40.36='<bif> argument 1',
                    'must be the name of a variable in the pool;',
                    'found "<value>"'
   !ErrorText.40.37='<bif> argument 3',
                    'must be the name of a pool; found "<value>"'
   !ErrorText.40.38='<bif> argument <argnumber>',
                    'is not large enough to format "<value>"'
   !ErrorText.40.41='<bif> argument <argnumber>',
                    'must be within the bounds of the stream;',
                    'found "<value>"'
   !ErrorText.40.42='<bif> argument 3 is not zero or one; found "<value>"'

   !ErrorText.41  = 'Bad arithmetic conversion'
   !ErrorText.41.1= 'Non-numeric value ("<value>")',
                    'to left of arithmetic operation "<operator>"'
   !ErrorText.41.2= 'Non-numeric value ("<value>")',
                    'to right of arithmetic operation "<operator>"'
   !ErrorText.41.3= 'Non-numeric value ("<value>")',
                    'used with prefix operator "<operator>"'
   !ErrorText.41.4= 'Value of TO expression in DO instruction',
                    'must be numeric; found "<value>"'
   !ErrorText.41.5= 'Value of BY expression in DO instruction',
                    'must be numeric; found "<value>"'
   !ErrorText.41.6= 'Value of control variable expression of DO instruction',
                    'must be numeric; found "<value>"'
   !ErrorText.41.7= 'Exponent exceeds' !Limit_ExponentDigits 'digits;',
                    'found "<value>"'

   !ErrorText.42  = 'Arithmetic overflow/underflow'
   !ErrorText.42.1= 'Arithmetic overflow detected at',
                    '"<value> <operation> <value>";',
                    'exponent of result requires more than',
                    !Limit_ExponentDigits 'digits'
   !ErrorText.42.2= 'Arithmetic underflow detected at',
                    '"<value> <operation> <value>";',
                    'exponent of result requires more than',
                    !Limit_ExponentDigits 'digits'
   !ErrorText.42.3= 'Arithmetic overflow; divisor must not be zero'

   !ErrorText.43  = 'Routine not found'
   !ErrorText.43.1= 'Could not find routine "<name>"'

   !ErrorText.44  = 'Function did not return data'
   !ErrorText.44.1= 'No data returned from function "<name>"'

   !ErrorText.45  = 'No data specified on function RETURN'
   !ErrorText.45.1= 'Data expected on RETURN instruction because',
                    'routine "<name>" was called as a function'

   !ErrorText.46  = 'Invalid variable reference'
   !ErrorText.46.1= 'Extra token ("<token>") found in variable',
                    'reference; ")" expected'

   !ErrorText.47  = 'Unexpected label'
   !ErrorText.47.1= 'INTERPRET data must not contain labels;',
                    'found "<name>"'

   !ErrorText.48  = 'Failure in system service'
   !ErrorText.48.1= 'Failure in system service: <description>'

   !ErrorText.49  = 'Interpretation Error'
   !ErrorText.49.1= 'Interpretation Error: <description>'

   !ErrorText.50  = 'Unrecognized reserved symbol'
   !ErrorText.50.1= 'Unrecognized reserved symbol "<token>"'

   !ErrorText.51  = 'Invalid function name'
   !ErrorText.51.1= 'Unquoted function names must not end with a period;',
                    'found "<token>"'

   !ErrorText.52  = 'Result returned by <name> is longer than',
                    !Limit_String 'characters'

   !ErrorText.53  = 'Invalid option'
   !ErrorText.53.1= 'String or symbol or variable reference expected',
                    'after STREAM keyword; found "<token>"'
   !ErrorText.53.2= 'Symbol or variable reference expected',
                    'after STEM keyword; found "<token>"'
   !ErrorText.53.3= 'Argument to STEM must have one period,',
                    'as its last character; found "<name>"'
   !ErrorText.54  = 'Invalid STEM value'
   !ErrorText.54.1= 'For this STEM APPEND, the value of "<name>" must be a',
                    'count of lines; found: "<value>"'
    return 0

