ready = 0
select
  when ready then do
    say 'ready'
  end
  otherwise say 'fallback'
end
