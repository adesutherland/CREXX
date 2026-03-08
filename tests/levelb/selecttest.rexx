options levelb

/*- - S E L E C T - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --*/
i=4
select
  when i=1 then say '*** Bad *** When'
  when i=4 then ok=ok '! When'
  when i=6 then say '*** Bad *** When'
end;
select
  when i=1 then say '*** Bad *** When/Otherwise'
  when i=6 then say '*** Bad *** When/Otherwise'
  otherwise ok=ok '! Otherwise'
end;
j=3
select
  when i=1 then say '*** Bad *** When'
  when i=4 then select
     when j=1 then say '*** Bad nested when ***'
     when j=3 then ok=ok '! nested Select'
     when j=5 then say '*** Bad nested when ***'
     otherwise say '*** Bad nested when ***'
    end
  otherwise say '*** Bad *** Otherwise'
end;
say ok; ok='OK'
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/
