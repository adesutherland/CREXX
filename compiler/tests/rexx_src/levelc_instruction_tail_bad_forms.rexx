options levelc
call on
say afterCallOnMissing
call on syntax
say afterCallOnBadWord
call off 10
say afterCallOffBadToken
call on error name
say afterCallNameMissing
call on error name 10
say afterCallNameBad
signal on
say afterSignalOnMissing
signal on banana
say afterSignalOnBadWord
signal off 10
say afterSignalOffBadToken
signal on error name
say afterSignalNameMissing
address value
say afterAddressValueMissing
address with banana
say afterAddressWithWord
address with 10
say afterAddressWithBad
address value env with 10
say afterAddressValueWithBad
numeric form 10
say afterNumericFormBad
numeric form value
say afterNumericFormValueMissing
procedure expose
say afterProcedureExposeMissing
procedure expose ()
say afterProcedureExposeEmptyParen
procedure expose (10)
say afterProcedureExposeParenBad
parse var
say afterParseVarMissing
parse var 10
say afterParseVarBad
arg (10)
say afterArgBadPattern
pull (10)
say afterPullBadPattern
parse arg (name
say afterParsePatternMissing
push +
say afterPushBadExpr
return +
say afterReturnBadExpr
signal value
say afterSignalValueMissing
address value +
say afterAddressValueBadExpr
