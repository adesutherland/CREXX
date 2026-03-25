/* rexx */
options levelb

namespace rxfnsb expose compileMask

/* ------------------------------------------------------------------
 * compileMask
 *
 * Build token arrays once from the mask.
 *
 * Token[1] type     = "LIT" / "TXT" / "NUM"
 * Token[2] text     = literal text for LIT tokens
 * Token[3] len      = field width for TXT/NUM
 * Token[4] intslots = integer slots for NUM
 * Token[5] decslots = decimal slots for NUM
 * Token[6] hasdot   = 0/1 for NUM
 * Token[7] curr     = currency marker for NUM
 * ------------------------------------------------------------------ */
compileMask: procedure=.int
    arg mask=.string, expose fmt_token=.string[],debug=0

    mask = translate(mask,"030405"x,"€£¥")
    token_count=fmt_token[1]
    if token_count="" then fmt_token[1]=1
    else fmt_token[1]=fmt_token[1]+1
    newToken = fmt_token[0]+1
    fmt_token[newToken]='$FMTOKEN'
    fmt_token[newToken+1]=newtoken
    token_entries=0
    pos  = 1
    llen = length(mask)
    one  = 1
    ch   = .string

    do while pos <= llen
       ch=substr(mask,pos,1)
        /* ----------------------------------------------------------
         * TXT token
         * ---------------------------------------------------------- */
        if ch = "X" then do
            width = readTextMaskLen(mask, pos)
            ##                       type text len   intslots, decslots hasdot curr
            call setToken fmt_token,"TXT","",  width,0,        0,       0,     ""
            token_entries=token_entries+1
            pos = pos + width
            iterate
        end
        /* ----------------------------------------------------------
         * NUM token
         * ---------------------------------------------------------- */
        if ch = "9" | ch = "$" | ch = "03"x | ch = "04"x | ch = "05"x | ch = "." then do
           info=readNumMaskInfo(mask, pos)
            call setToken fmt_token,"NUM","",  info[1],info[2],info[3],info[4],info[5]
            token_entries=token_entries+1
            pos = pos + info[1]
            iterate
        end
        /* ----------------------------------------------------------
         * LIT token
         * Batch literal run
         * ---------------------------------------------------------- */
        litstart = pos
        do while pos <= llen
           ch=substr(mask,pos,1)
           if ch = "X" | ch = "9" | ch = "$" | ch = "03"x | ch = "04"x | ch = "05"x | ch = "." then leave
           pos = pos + 1
        end
        litlen = pos - litstart

        if litlen > 0 then do
           text= substr(mask, litstart, litlen)
           text=left(text,litlen)
           call setToken fmt_token,"LIT",text,0,0,0,0,""
           if strip(text)\='' then token_entries=token_entries+1  /* don't count empty tokens, to match expectations in the debug output */
        end
    end
    fmt_token[newToken+2]=fmt_token[0]+1
    if debug=1 then do
       say "--- Compile Debug, Mask='"mask"'"
       say "    Token="newToken", Mask Entry="fmt_token[1]
       say "    Compiled Entries="token_entries
    end
return newToken

setToken: procedure
  arg expose fmt_token=.string[],toktype=.string,toktext=.string,toklen=.int,tokslot=.int,tokdslot=.int,tokhasdot=.int,tokcurr=.string
  t = fmt_token[0]+1
  fmt_token[t+1] = toktype
  fmt_token[t+2] = toktext
  fmt_token[t+3] = toklen
  fmt_token[t+4] = tokslot
  fmt_token[t+5] = tokdslot
  fmt_token[t+6] = tokhasdot
  fmt_token[t+7] = tokcurr
return
/* ------------------------------------------------------------------
 * Read contiguous X...X mask length only
 * ------------------------------------------------------------------ */
readTextMaskLen: procedure=.int
  arg s=.string, p=.int
  i    = p
  llen = length(s)
  do while i < llen
     ch=substr(s,i+1,1)
     if ch \= "X" then leave
     i = i + 1
  end
return i - (p - 1)

/* ------------------------------------------------------------------
 * Read numeric mask metadata in one pass.
 *
 * info_i[1] = seglen
 * info_i[2] = intslots
 * info_i[3] = decslots
 * info_i[4] = hasdot
 * info_s[1] = curr
 * ------------------------------------------------------------------ */
readNumMaskInfo: procedure=.string[]
  arg s=.string, p=.int
  info=.string[]
  i        = p
  seenDot  = 0
  intslots = 0
  decslots = 0
  curr     = ""
  one      = 1
  ch       = .string
  ch2      = .string
  llen     = length(s)

  do while i <= llen
     ch=substr(s,i,1)
     if ch = "9" then do
        if seenDot then decslots = decslots + 1
        else intslots = intslots + 1
        i = i + 1
        iterate
     end

     if ch = "$" | ch = "03"x | ch = "04"x | ch = "05"x then do
        if curr = "" then do
           if ch = "$" then curr = "$"
           else if ch = "03"x then curr = "€"
           else if ch = "04"x then curr = "£"
           else if ch = "05"x then curr = "¥"
        end

        if seenDot then decslots = decslots + 1
        else intslots = intslots + 1
        i = i + 1
        iterate
      end

     if ch = "." then do
        if seenDot then leave
        if i = llen then leave
        if substr(s,i+1,1)\="9" then leave
        seenDot = 1
        i = i + 1
        iterate
     end

     leave
  end

  info[1] = i - p
  info[2] = intslots
  info[3] = decslots
  info[4] = seenDot
  info[5] = curr
return info

