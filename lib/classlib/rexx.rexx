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
    return .rxfnsb::abbrev(val,astr,len)
    
-- 		abs
  abs: method = .string
    return .rxfnsb::abs(val)

-- b2x
  b2x: method = .string
    return .rxfnsb::b2x(val)

-- b2d
  b2d: method = .string
    return .rxfnsb::b2d(val)

-- 		c2x
  c2x: method = .string
    return .rxfnsb::c2x(val)

-- 		c2d
  c2d: method = .int
    return .rxfnsb::c2d(val)

-- 		center
  center: method = .string
    arg expose centlen = .int,  pad = " "
    return .rxfnsb::center(val,centlen,pad)

-- 		centre
  centre: method = .string
    arg centlen = .int,  pad = " "
    return .rxfnsb::centre(val,centlen,pad)

-- 		changestr
  changestr: method = .string
    arg haystack = .string, nneedle = .string
    return .rxfnsb::changestr(val,haystack,needle)
      
-- 		compare
  compare: method = .int
    arg astr = .string, pad = " "
    return .rxfnsb::compare(val,astr,pad)

-- 		copies
  copies: method =.string
    arg count=.int
    return .rxfnsb::copies(val,count)

-- 		countstr
  countstr: method = .int
    arg haystack = .string /* Pass by reference */
    return .rxfnsb::countstr(val,haystack)
    
-- 		d2b
  d2b: method = .string
    return .rxfnsb::d2b(val)

-- 		d2c
  d2c: method = .string
    arg slen=-1
    return .rxfnsb::d2c(val,slen)

-- 		d2x
  d2x: method = .string
    arg xint = .int, slen=-1
    return .rxfnsb::d2x(val,slen)
      
-- 		date: no date, is a bif but not really string related
-- 		delstr
  delstr: method = .string
    arg position = .int, dellen = 0
    return .rxfnsb::delstr(val,position,dellen)

-- 		delword
  delword: method  = .string
    arg wnum = .int, wcount = -1
    return .rxfnsb::delword(val,wnum,wcount)

-- 		fileio
-- 		filter
-- 		format
  format: method = .string
    arg before = 0, after = 0, expp = 0, expt=-1
    return .rxfnsb::format(val,before,after,expp,expt)

-- 		fnv
-- 		getenv
-- 		insert
  insert: method = .string
    arg  string = .string, position = -1, len = -1, pad = " "
    return .rxfnsb::insert(val,string,position,len,pad)

-- 		length
  length: method = .int
    return .rxfnsb::length(val)
    
-- 		linesize
  linesize: method = .int
    return .rxfnsb::linesize()

-- 		lower
  lower: method = .string
    return .rxfnsb::lower(val)

-- 		max : not relevant
-- 		min : not relevant
-- 		numeric : not relevant

-- 		overlay
  overlay: method = .string
    arg string = .string, position = .int, len = 0, pad = ""
    return .rxfnsb::overlay(val,string,position,len,pad)
	
-- 		pos
  pos: method = .int
    arg haystack=.string, start=1
    return .rxfnsb::pos(val,haystack,start)
    
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
    return .rxfnsb::lastpos(val,haystack,upto)
    
-- 		left
  left: method = .string
    arg number = .int
    return .rxfnsb::left(val,number)
    
-- 		right
  right: method = .string
    arg number = .int
    return .rxfnsb::right(val,number)

-- 		raise
-- 		reverse
  reverse: method = .string
    return .rxfnsb::reverse(val)

-- 		sign
  sign: method = .string
    return .rxfnsb::sign(val)

-- 		space
  space: method = .string
    arg spacenr = 1,  pad = " "
    return .rxfnsb::space(val,spacenr,pad)
    
-- 		strip
  strip: method = .string
    arg option = "B", schar= "UTF8WSP"
    return .rxfnsb::strip(val,option,schar)

-- 		substr
  substr: method = .string
    arg start = .int, len =-256 , pad = ' '  /* set len default to -256, to avoid a call to the length function */
    return .rxfnsb::substr(val,start,len,pad)
    
-- 		symbol
symbol: method = .string
    return .rxfnsb::symbol(val)

-- 		time : not relevant for this class

-- 		translate
translate: method = .string
  arg  tochar = "?????", fromchar = "?????",pad=" "
    return .rxfnsb::translate(val,tochar,fromchar,pad)

-- 		trunc
  trunc: method = .string
    arg fraction = 0
    return .rxfnsb::trunc(fraction)

-- 		upper
upper: method = .string
    return .rxfnsb::upper(val)

-- 		value
  value: method = .string
    return .rxfnsb::value(val)

-- 		verify
verify: method = .int
    arg intab = .string, match_mode='N', spos=1
    return .rxfnsb::verify(val,intab,match_mode,spos)

-- 		version : not relevant
-- 		word
  word: method =.string
    arg wordnum=.int
    return .rxfnsb::word(val,wordnum)

-- 		words
  words: method =.int
    return .rxfnsb::words(val)

-- 		wordindex
  wordindex: method =.int
    arg wordnum=.int
    return .rxfnsb::wordindex(val,wordnum)

 -- 		subword
  subword: method =.string
    arg wordnum=.int
    return .rxfnsb::subword(val,wordnum)

    -- 		wordlength
wordlength: method  = .string
    arg wordnum = .int
    return .rxfnsb::wordlength(val,wordnum)


-- 		wordpos
  wordpos: method = .int
    arg string = .string, start = 1
    return .rxfnsb::wordpos(val,string,start)
    
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
    return .rxfnsb::x2b(val,slen)
    
-- 		x2c
  x2c: method = .string
    return .rxfnsb::x2c(val)
    
-- 		x2d
  x2d: method = .int
    arg slen=-1
    return .rxfnsb::x2d(val,slen)
    
---		reradix
  reradix: method = .string
    arg fromRadix = .int, toRadix = .int
    return ._rxsysb::reradix(val,fromRadix,toRadix)
    
    ---		sequence
  sequence: method = .string
    arg tos = .string
    return .rxfnsb::sequence(val,tos)
    
    -- 		find
    -- 		index
    -- 		xrange
  xrange: method = .string
    arg tos = .string
    say 'xrange is deprecated; use sequence'
    return 'xrange is deprecated; use sequence'
    
    -- 		parsecompile
    -- 		parsestring
    -- 		parse
    -- 		datatype
  datatype: method =.string
    arg type=""
    return .rxfnsb::datatype(val,type)
    
  toString: method = .string
    return val
    

  
