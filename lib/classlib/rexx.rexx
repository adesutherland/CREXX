options levelb comments_dash numeric_classic
namespace rexx expose rexx
import _rxsysb
import rxfnsb

/* 
 * class rexx is a wrapper for .string
 * enabling oo style incvocation of its bifs
 * note that for the moment we have the source
 * of the bifs herein because the recursion
 * issue that happens when we cannot call
 * prefixed by namespace.
 * 
 */ 


rexx: class
val = .string

/** 
 * the class factory returns an instance of the rexx class
 */
  *: factory
    arg s = .string
    val = s
    return
-- 		abbrev
  abbrev: method = .string
    arg astr = .string, len = 0
    string = val
    i = .int
    slen=0
    alen=0
    char1=0
    char2=0
    assembler strlen slen,string
    assembler strlen alen,astr
    do i=0 to alen-1
      assembler strchar char1,string,i
      assembler itos char1
      assembler strchar char2,astr,i
      assembler itos char2
      if char1\=char2 then return 0
    end
    /* is the minimum match of chars satisfied */
    if len>0 then do
      if i<len then return 0
    end
    return 1
    
-- 		abs
  abs: method = .string
    if left(val,1) = '-' then number = substr(val,2)
    return number

-- 		c2x
  c2x: method = .string
    from = val
    stx=""
    fz=""
    len=0
    assembler strlen len,from
    if len=0 then return "00"
    do  i=0 to len-1
      assembler hexchar fz,from,i
      stx=stx||fz
    end
    return stx
-- 		c2d
  c2d: method = .int
    from = val
    stx=""
    fz=""
    len=0
    assembler strlen len,from
    if len=0 then return 0
    do  i=0 to len-1
      assembler hexchar fz,from,i
      stx=stx||fz
    end
    return _x2d(stx)
-- 		center
  center: method = .string
    arg expose centlen = .int,  pad = " "
    
    string = val
    padstr=""
    offset=0
    slen=0
    cpad=""
    
    /* make sure just to take first char */
    assembler strchar cpad,pad,offset
    assembler load pad,""
    assembler appendchar pad,cpad
    /* calculate number of pad chars to added as prefix and suffix */
    assembler strlen slen,string
    padlen=centlen-slen
    assembler idiv padlen,padlen,2
    if padlen=0 then return string   /* if nothing to add return original string */
    
    if padlen<0 then newstr=_substr(string,-padlen+1,centlen," ")
    else do
      /* create padding string */
      padstr=_copies(pad,padlen)
      newstr=padstr||string||padstr
      assembler strlen slen,newstr
      if slen<centlen then newstr=newstr||pad  /* in case of uneven center length */
    end
    
    return newstr
-- 		centre
  centre: method = .string
    arg centlen = .int,  pad = " "
    string = val
    padstr=""
    offset=0
    slen=0
    cpad=""
    
    /* make sure just to take first char */
    assembler strchar cpad,pad,offset
    assembler load pad,""
    assembler appendchar pad,cpad
    /* calculate number of pad chars to added as prefix and suffix */
    assembler strlen slen,string
    padlen=centlen-slen
    assembler idiv padlen,padlen,2
    if padlen=0 then return string   /* if nothing to add return original string */
    
    if padlen<0 then newstr=_substr(string,-padlen+1,centlen," ")
    else do
      /* create padding string */
      padstr=_copies(pad,padlen)
      newstr=padstr||string||padstr
      assembler strlen slen,newstr
      if slen<centlen then newstr=newstr||pad  /* in case of uneven center length */
    end
    
    return newstr
-- 		changestr
  changestr: method = .string
    arg haystack = .string, nneedle = .string /* Pass by reference */
    needle = val
    offset=1
    nlen=0
    assembler strlen nlen,needle
    nnlen=0
    assembler strlen nnlen,nneedle
    newstr=haystack
    
    do forever      /* use a large do loop until we get a do forever */
      offset=_pos(needle,newstr,offset)
      if offset=0 then return newstr
      if offset=1 then newstr=nneedle||substr(newstr,offset+nlen)
      else newstr=substr(newstr,1,offset-1)||nneedle||substr(newstr,offset+nlen)
	offset=offset+nnlen
      end
      return newstr
      
-- 		compare
  compare: method = .string
    arg astr = .string, pad = " "
    string = val
    slen=0
    alen=0
    char1=0
    char2=0
    assembler strlen slen,string
    assembler strlen alen,astr
    clen=slen
    if slen>alen then do
      astr=_substr(astr,1,slen,pad)
      clen=slen
    end
    else do
      if alen>slen then string=_substr(string,1,alen,pad)
      clen=alen
    end
    
    do i=0 to clen-1
      assembler strchar char1,string,i
      assembler strchar char2,astr,i
      if char1\=char2 then return i+1
    end
    
    return 0
-- 		copies
  copies: method =.string
    arg cstring=.string, count=.int
    result=''
    do i = 1 to count
      result = result || cstring
    end
    return result
-- 		countstr
  countstr: method = .int
    arg haystack = .string /* Pass by reference */
    needle = val
    count=0
    offset=1   ## strpos is 1-based offset for best performance
    nlen=0
    hlen=0
    assembler strlen nlen,needle
    if nlen<1 then return 0   /* empty needle, nothing to count */
    assembler strlen hlen,haystack
    if hlen<1 then return 0   /* empty haystack, nothing to count */
    
    
    do forever
      assembler strpos offset,needle,haystack
      if offset<=0 then return count
      offset=offset+nlen
      count=count+1
    end
    return count
-- 		d2b
d2b: method = .string
    arg dec = .int
    hex=d2x(dec)
    bit=x2b(hex)
    return bit
-- 		d2c
  d2c: method = .string
    arg from = .int, slen=-1
    xlen=0
    if slen=0 then return ''
    if slen>0 then xstr=d2x(from,slen+slen)
    else xstr=d2x(from)
      
      xstr=_x2c(xstr)
      assembler strlen xlen,xstr
      
      if slen>0 then do
	slen=slen+2  /* double bytes */
	assembler strlen xlen,xstr
	xlen=slen-xlen
      end
      return xstr

-- 		d2x
  d2x: method = .string
    arg xint = .int, slen=-1
    if slen=0 then return ""
    if slen>20 then slen=20
    trtab="0123456789ABCDEF"
    hlen=0
    sign=0
    yint=0
    rint=0
    rstr=""
    xstr=""
    sint=""
    
    if xint<0 then do
      sign=1
      dc=8
      do j=1 to 3
        dc=dc*16
      end
      xint=dc+xint
    end
    do i=0 to 64
      assembler idiv yint,xint,16
      assembler imod rint,xint,16
      xint=yint
      
      if xint>0 then do
        assembler strchar sint,trtab,rint
        assembler appendchar rstr,sint
      end
      else do       /* xint=0, this is the highest number then!   */
        if sign>0 then assembler ior rint,rint,8
        assembler strchar sint,trtab,rint
        assembler appendchar rstr,sint
        assembler strlen hlen,rstr
	
        do i=1 to hlen   /* now reverse the string */
          j=hlen-i
          assembler concchar xstr,rstr,j
        end
	/*     if hlen//2=1 then xstr='0'xstr */ /* dropped 2 bytes adjustment */
        if slen>0 then do
          if sign>0 then xstr=right(xstr,slen,'F')
          else xstr=right(xstr,slen,'0')
          end
          return xstr
	end
      end
      return ""
      
-- 		date: no date, is a bif but not really string related
-- 		delstr
  delstr: method = .string
    arg position = .int, dellen = 0
    string = val
    retstr = ""
    ## if position<1 then call raise "syntax", "40.13", position /* Invalid start */
    if position<1 then position=1
    len=length(string)
    if position>len then return string
    if position+dellen>len then dellen=0
    if position=1 then do
      if dellen>0 then retstr=substr(string,dellen+1)
      else retstr=''
      end
      else do
	if dellen=0 then retstr=substr(string,1,position-1)
	else retstr=substr(string,1,position-1)||substr(string,position+dellen)
	end
	return retstr

-- 		delword
  delword: method  = .string
    arg wnum = .int, wcount = -1
    string = val
    if wnum<1    then return string
    if wcount=0  then return string
    
    wrds=_words(string)
    if wnum>wrds then return string
    
    wdel=0
    wlen=0
    slen=0
    wpos=0
    retstr = ""
    
    if wcount<0 then do
      wpos=_wordindex(string,wnum)    /* locate position of word x */
      if wpos=0 then return string
      if wpos=1 then return ""
      return substr(string,1,wpos-1)
    end
    
    do forever                   /* temporary solution, until we have a do forever */
      assembler strlen slen,string   /* length of original string */
      if slen<1 then return ""
      wpos=_wordindex(string,wnum)    /* locate position of word x */
      if wpos=0 then return string
      wrd= _word(string,wnum)         /* load word      */
      assembler strlen wlen,wrd      /* length of word */
      xpos=wpos+wlen                 /* next position following word */
      if xpos<slen then do           /* if still in source string, check if char is blank */
        if substr(string,xpos,1)=' ' then wlen=wlen+1 /* if so, take it as part of word, increase word length */
      end
      if wpos+wlen>slen then do      /* if next byte after word exceeds length, it's last word */
        if wpos=1 then retstr=''    /* if start position 1: empty string remains              */
        else retstr=substr(string,1,wpos-1)  /* else take string prior to word                */
	end
	else do                        /* next word is within string        */
          if wpos=1 then retstr=substr(string,wpos+wlen)   /* take remain string after word     */
          else do                     /* drop word and re-construct string */
            if wnum<wrds then retstr=substr(string,1,wpos-1)||substr(string,wpos+wlen)  /* drop was in the middle */
            else retstr=substr(string,1,wpos-1)  /* drop was last word  */
            end
	  end
	  string=retstr         /* move back to original string and loop    */
	    wdel=wdel+1           /* increase deleted count                   */
	    if wdel>=wcount then return string /* match it with the requested count? Then return */
	  end
	  return string
	  
-- 		fileio
-- 		filter
-- 		format
  format: method = .string
    arg before = 0, after = 0, expp = 0, expt=-1
    innum = val
    /* --------------------------------------------------------------------------------------
    * Formatting without Exponent
    * --------------------------------------------------------------------------------------
    */
    if expt<0 then do  /* format numbers without exponent */
      ilen=0
      formatx=_itrunc(innum)   /* integer part of number */
      formaty=_ftrunc(innum)   /* fraction part of number*/
      if before>0 then do      /* formatting of integer? */
        assembler strlen ilen,formatx  /* get length    */
        if ilen>before then return 'integer part number to large'
        formatx=right(formatx,before,' ')
      end
      if after>0 then formaty=left(formaty,after,'0') /* formatting of fraction? */
      return formatx'.'formaty
    end
    /* --------------------------------------------------------------------------------------
    * Formatting with Exponent
    * --------------------------------------------------------------------------------------
    */
    else do /* expt>=0 format numbers with exponent */
      rs1=""
      float=0.0+innum    /* force input into float */
      f1="%1.14E"        /* format it as exponential with 14 fraction digits */
      assembler fformat rs1,float,f1  /* perform conversion */
      if substr(rs1,2,1)='.' then dp=2  /* seek decimal point, either 2 or 3 */
      else dp=3                 /* format:  +9.12345678901234E+123 */
	formatx=substr(rs1,1,dp-1)   /* select integer part      */
	  formaty=substr(rs1,dp+1,14)  /* select fractional part   */
	  formatz=substr(rs1,dp+17)    /* select exponent          */
	  sign=substr(rs1,dp+16,1)     /* sign of  exponent        */
	  if before>0 then formatx=right(formatx,before,' ')
	  if after=0 then after=3      /* default precision is 3   */
	  formaty=left(formaty,after,'0') /* format fractional part*/
	  formatx=formatx'.'formaty    /* concatenate  both        */
	  if expp=0 then expp=1        /* default exponent length 1*/
	  formatz1=formatz             /* check if exponent fits   */
	  formatz=right(formatz,expp,'0') /* format exponent       */
	  formatz2=formatz             /* recheck exponent         */
	  assembler stoi formatz1      /* convert to integer       */
	  assembler stoi formatz2      /* convert to integer       */
	  formatz1=formatz1+0   /* complete conversion (else string content appears) */
	  formatz2=formatz2+0
	  if formatz1>formatz2 then return "Exponent exceeds maximum formatted Exponent"
	  return formatx"E"||sign||formatz /* return result          */
	end
	return ''

-- 		fnv
-- 		getenv
-- 		insert
  insert: method = .string
    arg  string = .string, position = -1, len = -1, pad = " "
    insstr = val
    /* len=-1 or pos -1 mean, the parameters are not set in the function call */
    slen=length(string)
    if pad='' then pad=' '
    
    if len>0 then insstr=_substr(insstr,1,len,pad) /* was there an insert string length?      */
    /* format insert string to requested length*/
    else if len=0 then insstr=""
      if position<=0 then return insstr||string
      
      if position>slen then string=_substr(string,1,position,pad)  /* is position>string length, extend string */
      
      str1=_substr(string,1,position)             /* extract first part of string       */
      if position>=slen then str2=''             /* nothing remains of string          */
      else str2=_substr(string,position+1)        /*    else create str2                */
	return str1||insstr||str2                   /* return newly constructed string    */

-- 		length
  length: method = .int
    arg expose string1 = .string 
    string1 = val
    result = 0
    assembler strlen result,string1
    return result
-- 		linesize
  linesize: method = .int
    result = 999999999
    return result

-- 		lower
  lower: method = .string
    string = val
    newstr=""
    assembler strlower newstr,string
    return newstr

-- 		max : not relevant
-- 		min : not relevant
-- 		numeric : not relevant

-- 		overlay
  overlay: method = .string
    arg string = .string, position = .int, len = 0, pad = ""
    insstr = val
    padlen=0
    slen=0
    ilen=0
    assembler strlen padlen,pad      /* padding char */
    assembler strlen slen,string     /* source string  */
    if padlen=0 then pad=" " /* define default pad char, just in case we need one */
    
    if slen=0 then do
      string=pad
      slen=1
    end
    
    assembler strlen ilen,insstr
    if ilen=0 then do         /* insert string */
      if padlen=0 then return string
      insstr=pad
    end
    
    if len>0 then do                           /* was there an insert string length?      */
      if padlen>1 then pad=substr(pad,1,1)
      newins=_substr(insstr,1,len,pad)         /* format insert string to requested length*/
      insstr=newins
    end
    assembler strlen ilen,insstr
    if position+ilen>slen then string=_substr(string,1,position+ilen,pad)  /* is position>string length , extend string */
    
    if position=1 then str1=''                /* if insert position=1 then str1 is empty */
    else str1=_substr(string,1,position-1)  /*    else split string at insert position */
      if position+ilen>slen then str2=''             /* if string was extended str2 is empty    */
      else str2=_substr(string,position+ilen) /*    else create str2                     */
	
	return str1||insstr||str2                   /* return newly constructed string         */
	
-- 		pos
  pos: method = .int
    arg haystack=.string, start=1
    needle = val
    if needle='' then return 0
    if haystack='' then return 0
    foundpos=start   ## use return variable to input start position
    assembler strpos foundpos,needle,haystack
    return foundpos
-- 		qpos - later
-- 		qsplit "
-- 		qsplitsafe "
-- 		qextractall "
-- 		qextractpair "
-- 		qstripcomment "
-- 		qremoveall "
-- 		fsayfmt
-- 		lastpos
  lastpos: method = .int
    arg haystack=.string, upto=0
    needle = val
    if upto<1 then do
      assembler strlen upto,haystack
    end
    nlen=0
    assembler strlen nlen,needle
    if nlen=0      then return 0
    if haystack='' then return 0
    nlen=nlen-1
    foundpos=1
    lastfound=0
    do forever
      assembler strpos foundpos,needle,haystack
      if foundpos = 0 then leave
      if foundpos + nlen> upto then leave    ## nlen-1 calculated above
      lastfound = foundpos
      foundpos=foundpos + 1
    end
    return lastfound
    
-- 		left
-- 		right
-- 		raise
-- 		reverse
-- 		sign
-- 		space
-- 		strip
-- 		substr
  substr: method = .string
    arg start = .int, len =-256 , pad = ' '  /* set len default to -256, to avoid a call to the length function */
    string1 = val
    inputLength  = 0
    assembler strlen inputLength, string1          /* this is the total string length */
    if len=-256 then len=inputLength + 1 - start   /* len was not set, use the faster assembler instruction to retrieve and calculate the length */
    
    if start < 1 then call raise "syntax", "40.13", start
    ## if len   < 0 then call raise "syntax", "40.13", len
    if len   <= 0 then return ''
    /* Convert to 0-based character offset */
    start = start - 1
    
    /* fast path if start offset is 0, string can be cut off after length */
    if start=0 then do
      if len = inputLength then return string1  /* same length, return immediately */
      if len<=inputLength then do
        assembler substcut string1,len         /* no length checking necessary, handled in substcut */
        return string1
      end
    end
    /* fast path if length doesn't exceed string length, there is no padding required, substring can return directly */
    outputstring = .string
    if start+len<=inputLength then do
      assembler SETSTRPOS string1,start
      assembler substring outputstring,string1,len
      return outputstring
    end
    
    /* Initialize registers */
    padchar      = 0  /* Holds Unicode codepoint */
    padstring    = ""
    padLength    = 0
    
    /* Pad character validation and preparation */
    if pad = '' then pad = ' '
    assembler strlen padLength, pad
    if padLength > 1 then call raise "syntax", "40.23", pad
    assembler strchar padchar, pad   /* padchar= utf8codepoint(pad) */
    
    /* Calculate remaining input length */
    inputLength = inputLength - start              /* calculate usable length after positioning substr */
    
    /* Set byte offset for VM based on character index (UTF-8 safe) */
    assembler SETSTRPOS string1, start
    /* Copy or pad */
    if inputLength <= 0 then do                      /* just padding required */
      assembler padstr outputstring, padchar, len
    end
    else if len <= inputLength then do
      assembler substring outputstring, string1, len
    end
    else do
      assembler substring outputstring, string1, inputLength
      pcount = len - inputLength
      assembler padstr outputstring, padchar, pcount
    end
    return outputstring

    -- 		substro
-- 		symbol
symbol: method = .string

    inputsymbol = val
    if inputsymbol = "" then return "BAD"

    result = ""
    reg = _strip(_lower(inputsymbol))
    sym = ""
    type = ""
    module = 0
    addr = 0
    meta_array = 0
    meta_entry = ""

    /* First check if it is a valid symbol */
    /* Characters must be a letter, number, full stop or underscore */
    /* Note this included 0.23.4 as a valid symbol ... should we change this for level b? */
    do i = 1 to length(inputsymbol)
        c = substr(inputsymbol, i, 1)
        if _pos(c, 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.') = 0 then return 'BAD'
    end

    /* Check if it is a reserved word - level b */
    keywords = "address arg assembler by call digits do drop else end error exit expose for forever if import input interpret",
               "iterate leave loop namespace nop numeric off on options otherwise output parse procedure pull push queue return",
               "say select signal then to trace until upper value var version void when while with "
    if _pos(_lower(inputsymbol) || " ", keywords) > 0 then return 'BAD'

    /* Look for the symbol in the metadata */
    address_object = 0
    assembler metaloadcalleraddr address_object /* Address from where are we called */
    assembler linkattr1 module,address_object,1  /* 1 = Module number */
    assembler linkattr1 addr,address_object,2 /* 2 = Address in module */

    /* Read the addresses backwards */
    result = "LIT" /* Assume a unused variable name */
    do a = addr to 0 by -1
        /* Get the metadata for that address */
        assembler metaloaddata meta_array,module,a
        do i = 1 to meta_array
            assembler linkattr1 meta_entry,meta_array,i
            if meta_entry = ".meta_clear" then do /* Object type */
                assembler linkattr1 sym,meta_entry,1
                if pos("."reg"@",sym"@") > 0 then do /* Rough and ready find */
                    /* result = "LIT" */ /* We have found a unused variable name */
                    leave a /* Leave the address loop - we have found the previous procedure in the module */
                end
            end

            else if meta_entry = ".meta_const" then do /* Object type */
                assembler linkattr1 sym,meta_entry,1
                if pos("."reg"@",sym"@") > 0 then do /* Rough and ready find */
                    result = "VAR" /* We have found a constant - but in this context it is a variable */
                    leave a
                end
            end

            else if meta_entry = ".meta_reg" then do /* Object type */
                assembler linkattr1 sym,meta_entry,1
                if pos("."reg"@",sym"@") > 0 then do /* Rough and ready find */
                    result = "VAR" /* We have found a variable */
                    leave a
                end
            end
        end
    end

    return result


-- 		time : not relevant
-- 		translate
translate: method = .string
  arg  tochar = "?????", fromchar = "?????",pad=" "
  source = val
  slen=0
  flen=0
  tlen=0
  result=""
  schar=0

  /* !!! fromchar or tochar containing '?????' mean it is not set, while '' mean it is empty */
  if pad=' ' then do
     if fromchar="?????" & tochar="?????" then return _upper(source)
     if fromchar="" & tochar="" then return source
  end
  assembler strlen slen,source                 /* get length of souce string     */
  if fromchar="?????" then do
     if tochar="?????" then  return copies(pad,slen)
     if tochar="" then return copies(pad,slen)
     fromchar=" "
  end
  if tochar="?????" then tochar=" "
  if fromchar="?????" then fromchar=" "

  assembler strlen flen,fromchar               /* get length of from list        */
  assembler strlen tlen,tochar                 /* get length of to   list        */
  if flen>tlen then tochar=left(tochar,flen,pad)  /* from and to list must be equal */
  else if tlen>flen then fromchar=left(fromchar,tlen,pad)

  do i=0 to slen-1                             /* run through from source string */
     assembler strchar schar,source,i
     assembler transchar schar,tochar,fromchar /* translate source char (if necessary) */
     assembler appendchar result,schar         /* append it to result  */
  end

return result

-- 		trunc
  trunc: method = .string
    arg fraction = 0
    innum = val
    retnum=""
    hlen=0
    i=0
    ifrac=0
    dp=0
    schar='.'
    char=""
    assembler strchar schar,schar,i   /* get decimal point */
    assembler itos schar
    
    /*  assembler ftos innum      */        /* translate input from float to string */
    /* translate exponentional format to integer */
    ppi=pos('E',innum)
    if ppi>0 then do
      num=substr(innum,1,ppi-1)
      exp=substr(innum,ppi+1)
      assembler stof num
      assembler stoi exp
      xnum=num*(10**exp)
      assembler itos xnum
      innum=xnum||'.0000000000000000'
    end
    
    Assembler strlen hlen,innum       /* determine string length              */
    
    /* step 1 transfer all digits before decimal point */
    do i=0 to hlen-1                  /* transfer byte by byte, until decimal point */
      assembler strchar char,innum,i /* get next input byte  */
      assembler itos char
      if char=schar then do          /* is it decimal point  */
        dp=i                        /* if yes, save offset  */
        leave                       /* set i hlen, to abort do loop */
	end
	else assembler appendchar retnum,char /* if no, transfer byte  */
	end
	if i=0 then retnum="0"
	if fraction=0 then return retnum  /* no fraction digits requested */
	/* step 2 transfer all fraction digits (up to max fraction) */
	assembler appendchar retnum,schar /* set decimal point   */
	do i=dp+1 to hlen-1               /* loop through remaining string */
	  assembler strchar char,innum,i /* fetch next char     */
	  assembler itos char
	  assembler appendchar retnum,char /* append return string */
	  ifrac=ifrac+1                    /* increase fraction count */
	  if ifrac>=fraction then leave     /* if limit reached leave loop */
	  end
	  /* step 3 add padding bytes (if necessary) */
	  if ifrac< fraction then do
	    retnum=left(retnum,fraction+dp+1,'0')
	  end

return retnum

-- 		upper
upper: method = .string
    string = val
    newstr=""
    assembler strupper newstr,string
    return newstr

-- 		value
  value: method = .string
    inputstring = val
    
    if inputstring = "" then return "" /* ? */
    
    result = ""
    ires = 0
    fres = 0.0
    sres = ""
    reg = _lower(inputstring)
    symbol = ""
    type = ""
    module = 0
    addr = 0
    meta_array = 0
    meta_entry = ""
    v = ""
    r_num = 0
    
    address_object = 0
    assembler metaloadcalleraddr address_object /* Address from where are we called */
    assembler linkattr1 module,address_object,1  /* 1 = Module number */
    assembler linkattr1 addr,address_object,2 /* 2 = Address in module */
    
    /* Read the addresses backwards */
    do a = addr to 0 by -1
      /* Get the metadata for that address */
      assembler metaloaddata meta_array,module,a
      do i = 1 to meta_array
        assembler linkattr1 meta_entry,meta_array,i
        if meta_entry = ".meta_clear" then do /* Object type */
          assembler linkattr1 symbol,meta_entry,1
          if pos("."reg"@",symbol"@") > 0 then do /* TODO - Rough and ready find */
            leave a
          end
        end
	
        else if meta_entry = ".meta_const" then do /* Object type */
          assembler linkattr1 symbol,meta_entry,1
          if pos("."reg"@",symbol"@") > 0 then do /* TODO - Rough and ready find */
            assembler linkattr1 type,meta_entry,3
            assembler linkattr1 v,meta_entry,4
            result = v
            leave a
          end
        end
	
        else if meta_entry = ".meta_reg" then do /* Object type */
          assembler linkattr1 symbol,meta_entry,1
          if pos("."reg"@",symbol"@") > 0 then do /* TODO - Rough and ready find */
            assembler linkattr1 type,meta_entry,3
            assembler linkattr1 r_num,meta_entry,4
	    
            if type = ".int" then do
              assembler metalinkpreg ires,r_num       /* Link parent-frame-register */
              ires_copy = ires /* Don't want to alter ires with any side effects */
              assembler unlink ires
              result = ires_copy
            end
	    
            else if type = ".float" then do
              assembler metalinkpreg fres,r_num       /* Link parent-frame-register */
              fres_copy = fres
              assembler unlink fres
              result = fres_copy
            end
	    
            else do
              assembler metalinkpreg sres,r_num       /* Link parent-frame-register */
              sres_copy = sres
              assembler unlink sres
              result = sres_copy
            end
          end
        end
      end
    end
    if result = "" then result = _upper(inputstring)
    
    return result

-- 		verify
verify: method = .int
    arg intab = .string, match='N', spos=1
    instring = val
    ilen=0
    tlen=0
    char=""
    tab=""
    pos=0
    
    if match='N' then imatch=0
    else if match='n' then imatch=0
      else if match='M' then imatch=1
	else if match='m' then imatch=1
	  
	  Assembler strlen ilen,instring        /* determine string length              */
	  Assembler strlen tlen,intab           /* determine table  length              */
	  if ilen=0 then return 0
	  if tlen=0 then return spos
	  
	  spos=spos-1
	  if spos<0 then spos=0
	  
	  do i=spos to ilen-1                  /* check each byte of input string          */
	    assembler strchar char,instring,i /* get next input byte  */
	    assembler itos char
	    fnd=0
	    do j=0 to tlen-1                  /* check each byte of verify table       */
              assembler strchar tab,intab,j  /* get next input byte  */
              assembler itos tab
              if char=tab then do            /* char found in table, check next input char */
		fnd=1                       /* set found  */
		j=tlen                      /* leave loop by setting it to upper limit */
		end
	      end
	      if fnd=1 & imatch=1 then do       /* if found & match=1 set position and leave outer loop */
		pos=i+1                        /* set found offset to position            */
		i=ilen                         /* leave outer loop */
		end
		if fnd=0 & imatch=0 then do       /* if not found and match=0 set position and leave outer loop */
		  pos=i+1                        /* not found offset to position            */
		  i=ilen                         /* leave outer loop */
		  end
		end
		
		return pos

-- 		version : not relevant
-- 		word
  word: method =.string
    arg wordnum=.int
    string = val
    len = length(string)
    offset = 0
    count = 0
    if wordnum<1 then return ""
    if len<1 then return ""
    
    do while offset < len
      assembler fndnblnk offset,string,offset     /* Find the next non-blank character */
      if offset < 0 then return ""   /* No more words */
      count = count + 1
      wordStart = offset
      assembler fndblnk offset,string,offset    /* Find the next blank after the word */
      if count = wordnum then do
        if offset < 0 then return substr(string, wordStart + 1)
        else return substr(string, wordStart + 1, offset - wordStart)
	end
	if offset < 0 then leave
      end
      return ""  /* wordnum not found */

-- 		words
  words: method =.int
    string= val
    len = length(string)
    offset = 0
    count = 0
    if wordnum<1 then return 0
    if len<1 then return 0
    
    do while offset < len
      /* Find next non-blank character (start of a word) */
      assembler fndnblnk offset,string,offset
      if offset < 0 then leave
      
      count = count + 1  /* Found a word */
      
      /* Find next blank after this word (to skip it) */
      assembler fndblnk offset,string,offset
      if offset < 0 then leave
    end
    return count  /* wordnum not found */

-- 		wordindex
  wordindex: method =.int
    arg wordnum=.int
    string = val
    len = length(string)
    offset = 0
    count = 0
    if wordnum<1 then return 0
    if len<1     then return 0
    
    do while offset < len
      assembler fndnblnk offset,string,offset     /* Find the next non-blank character */
      if offset < 0 then return 0                 /* No more words, negative value to distinguish from offset=0 which is valid */
      count = count + 1
      wordStart = offset
      assembler fndblnk offset,string,offset     /* Find the next blank after the word */
      if count = wordnum then return wordstart+1
      if offset < 0 then return 0                /* No more words, negative value to distinguish from offset=0 which is valid */
    end
    return 0  /* wordnum not found */
    -- 		subword
  subword: method =.string
    arg wordnum=.int
    string = val
    if wordnum<1 then return ''
    spos=_wordindex(string,wordnum)              /* start position of selected word */
    if spos=0 then return ""
    return substr(string,spos)

    -- 		wordlength
wordlength: method  = .string
    arg wordnum = .int
    string = val
    wlen=0
    wordstr=_word(string,wordnum)
    if wrdstr="" then return 0
    assembler strlen wlen,wordstr
    return wlen

-- 		wordpos
  wordpos: method = .int
    arg string = .string, start = 1
    search = val
    tlen=0
    assembler strlen tlen,string
    if tlen=0 then return 0
    assembler strlen tlen,search
    if tlen=0 then return 0
    
    wnum=_words(string)
    if wnum=0 then return 0
    snum=_words(search)
    if snum=0 then return 0
    
    search=_strip(search)
    
    startpos=_wordindex(string,start)  /* calculate wordpos */
    do i=start to wnum
      if snum=1 then do
	if _abbrev(_word(string,i),search)>0 then return i
      end
      else do
	if _pos(search,string,startpos) = startpos then return i
	startpos=_wordindex(string,i+1)
	if startpos=0 then leave
      end
    end
    return 0
    
-- 		qword
-- 		qwordlength
-- 		qwords
-- 		qwordindex
-- 		qwordpos
-- 		qsubword
-- 		regex
    -- 		x2b
x2b: method = .string
    arg slen=-1
    hex = val
    if hex="" then return ""
    /*      0000 0001 0010 0011 0100 0101 0110 0111 1000 1001 1010 1011 1100 1101 1110 1111 */
    bittab="0000000100100011010001010110011110001001101010111100110111101111"
    trtab="0123456789ABCDEFabcdef "
    char=""
    rstr=""
    hlen=0
    offset=0
    added=0
    assembler strlen hlen,hex
    do i=0 to hlen-1
      assembler strchar char,hex,i
      assembler poschar offset,trtab,char  /* position in hex table   */
      if offset<0 then do
	say "hex string contains invalid character "hex
	return ""
      end
      if added=0 & offset=0 then iterate  /* ignore first leading zero */
      added=added+1
      if offset=22 then iterate       /* it's a blank ignore it */
      if offset>15 then offset=offset-6
      offset=offset*4
      /* rstr\="" then rstr=rstr||" " */
      do k=offset to offset+3
	assembler concchar rstr,bittab,k
      end
    end
    if added=0 then rstr='0000'
    return rstr
-- 		x2c
  x2c: method = .string
    hex = val
    trtab="0123456789ABCDEFabcdef"
    slen=0
    char1=0
    char2=0
    off1=0
    off2=0
    binary=""
    byte=0
    hexi=""
    blank=" "
    assembler dropchar hexi,hex,blank
    assembler strlen slen,hexi    /* get length of hex string */
    
    if slen//2=1 then do
      hexi='0'hexi
      slen=slen+1
    end
    
    do i=0 to slen-1 by 2                   /* loop through hex string */
      assembler strchar char1,hexi,i        /* fetch one byte          */
      assembler poschar off1,trtab,char1   /* position in hex table   */
      j=i+1
      assembler strchar char2,hexi,j        /* fetch one byte          */
      assembler poschar off2,trtab,char2   /* position in hex table   */
      
      if off1<0 | off2<0 then do           /* no hex char? error!     */
        say "hex string contains invalid character "hexi
        return 0
      end
      if off1>15 then off1=off1-6    /* translate lower case to upper case hex */
      if off2>15 then off2=off2-6    /* translate lower case to upper case hex */
      assembler ishl byte,off1,4     /* move 1. char to left hand part of byte */
      byte=byte+off2                 /* move 2. char to right hand part of byte*/
      assembler appendchar binary,byte /* append to result                     */
    end
    return binary  /* return result */
    
-- 		x2d
  x2d: method = .int
    arg slen=-1
    hex = val
    trtab="0123456789ABCDEFabcdef"
    hlen=0
    char=0
    dec=0
    sign=0
    offset=0
    len = 0
    if slen=0 then return 0      /* no hex input return 0    */
    if slen>20 then slen=20      /* may be up to 20 chars, inluding sign */
    assembler strlen hlen,hex    /* get length of hex string */
    if slen > -1 then do
      hex=right(hex,slen,"0")  /* if length parm set, treat hex as signed integer */
      len = slen
    end
    else len=hlen                          /* else it is an unsigned integer */
      do i=0 to len-1                        /* loop through hex string */
	assembler strchar char,hex,i         /* fetch one byte          */
	assembler poschar offset,trtab,char  /* position in hex table   */
	if offset<0 then do
          say "hex string contains invalid character "hex
          return 0
	end
	if offset>15 then offset=offset-6    /* translate lower case to upper case hex */
	char=offset
	/* if it is first byte then check if sign is set, and convert number */
	if i=0 then do
          if char>=8 & slen > -1 then do /* does it contain sign? */
            sign=1                   /* keep sign             */
            char=char-8              /* switch off sign       */
          end
	end
	dec=dec*16+char        /* cumulate fetched char in result integer */
      end
      /* hex string has been processed, check if there was a sign */
      if sign=1 then do        /* was sign set?    */
	char=8                /* build complement */
	do i=1 to len-1
          char=char*16
	end
	dec=dec-char          /* create integer out of complement */
      end
      return dec
    
---		reradix

reradix: method = .string
arg FromRadix = .int, ToRadix = .int
subject = val
subject = _upper(subject)
 integer=0
 do j=1 to _length(subject)
   /* Individual digits have already been checked for range. */
   integer=integer*FromRadix+_pos(_substr(subject,j,1),'0123456789ABCDEF')-1
   /* This test not for standard. */
    if _pos('E',integer)>0 then do
      say "ReRadix unable"
      return "?"
    end
 end
 r=''
 if integer=0 then r='0'
 do while integer>0
    r=_substr('0123456789ABCDEF',1+integer//ToRadix,1)||r
   integer=integer%ToRadix
 end
 /* When between 2 and 16, there is no zero suppression. */
 if FromRadix=2 & ToRadix=16 then
   r=_right(r,(_length(subject)+3)%4,'0')
 else if FromRadix=16 & ToRadix=2 then
   r=_right(r,_length(subject)*4,'0')
 return r

---		sequence
  sequence: method = .string
    arg tos = .string
    from = val
    /* TODO: support hex notation */
    
    fromVal=_c2d(from)
    toVal  =_c2d(tos)
    
    /* as opposed to xrange, there is no wraparound */
    if fromVal > toVal then do
      say 'starting value must be less than end value'
      return "BAD"
    end
    
    resultString = ""
    diff=0
    
    assembler isub diff,toVal,fromVal
    
    loop i=0 to diff
      assembler itos fromVal
      val=_reradix(fromVal,10,16)
      resultString=resultString||_x2c(val)
      assembler inc fromVal
    end
    
    return resultString
      
-- 		find
-- 		index
-- 		xrange
  xrange: method = .string
    arg tos = .string
    say 'xrange is deprecated; using sequence'
    return sequence(tos)
    
-- 		parsecompile
-- 		parsestring
-- 		parse
-- 		datatype
datatype: method =.string
    arg type=""
    return _datatype(val,type)
    
  toString: method = .string
    return val
    

  
