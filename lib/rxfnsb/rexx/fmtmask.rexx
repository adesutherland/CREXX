/* rexx */
options levelb

namespace rxfnsb expose fmtmask
/* ============================================================================
 * Function:  fmtmask
 * ----------------------------------------------------------------------------
 * Purpose
 *   Formats a line using a simple picture-mask syntax and a positional list
 *   of values.
 *
 *   fmtmask is inspired by classic report-writer and COBOL-style picture layouts,
 *   but intentionally implements only a small, practical subset suitable for
 *   CREXX / REXX-style text reporting.
 *
 * Usage
 *   line = fmtmask(mask, value1, value2, ...)
 *
 * Example
 *   say fmtmask("Name: XXXXXXXXXX  Price: $$$$9.99  Qty: 999", ,
 *            "Fred", 64.31, 12)
 *
 * Result
 *   Name: Fred        Price:   $64.31  Qty:  12
 *
 * ----------------------------------------------------------------------------
 * Mask Model
 * ----------------------------------------------------------------------------
 * The mask string is scanned from left to right and split into:
 *
 *   1. literal text
 *   2. text fields
 *   3. numeric fields
 *
 * Literal text is copied unchanged.
 * Each field consumes the next positional value argument.
 *
 * ----------------------------------------------------------------------------
 * Supported Field Types (v1)
 * ----------------------------------------------------------------------------
 *
 * 1. Text field
 *      X...X
 *
 *    A contiguous run of 'X' characters defines a text field.
 *
 *    Example:
 *      XXXXXXXX
 *
 *    Semantics:
 *      - field width = number of X characters
 *      - value is left-aligned
 *      - value is truncated if too long
 *      - value is padded on the right if shorter
 *
 *    Example:
 *      mask  : "XXXXXXXXXX"
 *      value : "Fred"
 *      out   : "Fred      "
 *
 * ----------------------------------------------------------------------------
 *
 * 2. Integer numeric field
 *      9...9
 *
 *    A contiguous run of '9' characters with no decimal point defines an
 *    integer-style numeric field.
 *
 *    Example:
 *      999
 *
 *    Semantics:
 *      - field width = number of mask characters
 *      - value is truncated to integer via TRUNC()
 *      - value is right-aligned
 *      - overflow fills the whole field with '*'
 *
 *    Example:
 *      mask  : "999"
 *      value : 12
 *      out   : " 12"
 *
 * ----------------------------------------------------------------------------
 *
 * 3. Decimal numeric field
 *      9...9.9...9
 *
 *    A numeric mask containing a decimal point followed by one or more '9'
 *    characters defines a fixed-decimal numeric field.
 *
 *    Example:
 *      9999.99
 *
 *    Semantics:
 *      - total field width = full mask length
 *      - decimal precision = number of 9 characters after '.'
 *      - numeric formatting uses FORMAT()
 *      - result is right-aligned
 *      - overflow fills the whole field with '*'
 *
 *    Example:
 *      mask  : "9999.99"
 *      value : 64.31
 *      out   : "  64.31"
 *
 * ----------------------------------------------------------------------------
 *
 * 4. Currency numeric field
 *      $...$9.99
 *      €...€9.99
 *      £...£9.99
 *      ¥...¥9.99
 *
 *    A numeric mask may include a repeated leading currency symbol. In v1,
 *    currency handling is intentionally simple:
 *
 *      - the currency character contributes to total field width
 *      - the numeric value is formatted as a normal numeric field
 *      - the currency symbol is overlaid into the formatted field
 *      - overflow still produces only '*' characters
 *
 *    Example:
 *      mask  : "$$$$9.99"
 *      value : 64.31
 *      out   : "  $64.31"
 *
 * Notes:
 *      - This is not intended to be full COBOL currency-editing semantics.
 *      - Mixed currency symbols within one field are undefined in v1.
 *
 * ----------------------------------------------------------------------------
 * Overflow Handling
 * ----------------------------------------------------------------------------
 * If the formatted value does not fit into the field width, the entire field
 * is replaced by '*' characters of exactly the same width.
 *
 * Examples:
 *   mask  : "999"
 *   value : 12345
 *   out   : "***"
 *
 *   mask  : "$$$9.99"
 *   value : 123333.3
 *   out   : "*******"
 *
 * ----------------------------------------------------------------------------
 * Parsing Rules
 * ----------------------------------------------------------------------------
 * 1. Text masks are contiguous runs of 'X'.
 * 2. Numeric masks are contiguous runs of:
 *      - '9'
 *      - '.'
 *      - supported currency characters
 * 3. A '.' belongs to a numeric mask only if followed by at least one '9'.
 *    Therefore:
 *      999.99   -> numeric field
 *      999.     -> numeric field "999" followed by literal '.'
 *
 * ----------------------------------------------------------------------------
 * Value Consumption
 * ----------------------------------------------------------------------------
 * Values are consumed strictly left-to-right in encounter order.
 *
 * Example:
 *   mask:
 *      "Name: XXXXX  Qty: 999  Price: 999.99"
 *
 *   values:
 *      "Tea", 7, 12.45
 *
 *   mapping:
 *      XXXXX   -> "Tea"
 *      999     -> 7
 *      999.99  -> 12.45
 *
 * ----------------------------------------------------------------------------
 * Design Scope
 * ----------------------------------------------------------------------------
 * fmtmask v1 is intentionally small and practical.
 *
 * It is:
 *   - a lightweight picture-mask formatter
 *   - suitable for report-like textual output
 *   - inspired by COBOL / report-writer ideas
 *
 * It is not:
 *   - a full COBOL PIC implementation
 *   - a locale-aware currency formatter
 *   - a complete numeric editing language
 *
 * ----------------------------------------------------------------------------
 * Implementation Notes
 * ----------------------------------------------------------------------------
 * The current implementation is split conceptually into:
 *   - mask scanning
 *   - field classification
 *   - field-specific formatting
 *
 * Useful helper functions include:
 *   - readTextMask()
 *   - readNumMask()
 *   - fmtTextMask()
 *   - fmtNumMask()
 *   - isCurrencyMaskChar()
 *   - getCurrencyMaskChar()
 *
 * A later compiler-exit version may compile the mask once and emit a direct
 * SAY expansion or helper-call sequence.
 * ==========================================================================
 */
fmtmask: procedure=.string
    /* ------------------------------------------------------------------
     * fmtmask1
     * Simple COBOL-style formatting function.
     *
     * mask : formatting picture
     * a1..a15 : values to be inserted into the mask fields
     *
     * Supports:
     *   X...X     → text field
     *   9...9     → numeric field
     *   .         → decimal point in numeric mask
     *   currency  → $, €, £, ¥ (handled internally via placeholders)
     *
     * Strategy:
     *   UTF-8 currency symbols (€,£,¥) are replaced with single-byte
     *   placeholders (03,04,05 hex) to simplify mask scanning.
     *   After formatting the placeholders are translated back.
     * ------------------------------------------------------------------
     */
    arg mask=.string, a1=.string, a2="", a3="", a4="", a5="", a6="", a7="", a8="", a9="", a10="",a11="", a12="", a13="", a14="", a15=""
    /* ------------------------------------------------------------------
     * Replace UTF-8 currency symbols with internal single-byte markers.
     * This avoids problems when scanning the mask character-by-character.
     * ------------------------------------------------------------------
     */
    mask = translate(mask,"030405"x,"€£¥")
    /* ------------------------------------------------------------------
     * Copy arguments into a simple indexed array for easier access
     * during mask processing.
     * ------------------------------------------------------------------
     */
    vals.1  = a1
    vals.2  = a2
    vals.3  = a3
    vals.4  = a4
    vals.5  = a5
    vals.6  = a6
    vals.7  = a7
    vals.8  = a8
    vals.9  = a9
    vals.10 = a10
    vals.11 = a11
    vals.12 = a12
    vals.13 = a13
    vals.14 = a14
    vals.15 = a15
    out  = ""      /* resulting output string */
    pos  = 1       /* current scan position inside mask */
    argi = 1       /* index of next argument to consume */
    one=1
    /* ------------------------------------------------------------------
     * Scan mask from left to right and emit output.
     * ------------------------------------------------------------------
     */
     ch=.string
 /*  llen=length(mask) */
/* rxpp: strlen(llen,mask) */
     llen=.int; assembler strlen llen,mask
    do while pos <= llen
/* rxpp: subst1Char(ch,mask, pos)    /* current mask character */ */
       __jpos=pos-1; assembler SETSTRPOS mask,__jpos; assembler substr ch,mask,one    /* current mask character */
        /* --------------------------------------------------------------
         * TEXT FIELD
         * Example: XXXXXXXX
         * --------------------------------------------------------------
         */
        if ch = "X" then do
       /*    width = readTextMaskLen(mask, pos,llen)  */  /* read full mask segment */
/* rxpp: masklen(width,mask,pos,llen) */
/* rxpp: __indx=.int; do __indx=pos to llen; subst1Char(ch,mask,__indx); if ch\= "X" then leave; end; width=__indx-pos */
           __indx=.int; do __indx=pos to llen; __jpos=__indx-1; assembler SETSTRPOS mask,__jpos; assembler substr ch,mask,one; if ch\= "X" then leave; end; width=__indx-pos
           out = out || left(vals.argi, width)        /* format text value */
           argi = argi + 1
           pos  = pos + width                         /* skip entire mask segment */
           iterate
        end
        /* --------------------------------------------------------------
         * NUMERIC FIELD
         * Example: 9999.99 or $$$9.99 etc.
         * --------------------------------------------------------------
         */
   /*     if isNumMaskChar(ch) then do */
/* rxpp: if ch = "9" | ch = "." | CurrencySymbol() then do */
        if ch = "9" | ch = "." | ch = "$" | ch = "03"x | ch = "04"x | ch = "05"x then do
            seg = readNumMask(mask, pos,llen)               /* read numeric mask */
            /* debug example:
               say "NUMMASK=["seg"] NEXT=["substr(mask,pos+length(seg),1)"] ARGI="argi" VALUE="vals.argi
            */
            out = out || fmtNumMask(vals.argi, seg)    /* format numeric value */
            argi = argi + 1
/* rxpp: strlen(slen,seg) */
            slen=.int; assembler strlen slen,seg
            /* pos  = pos + length(seg)    */              /* skip mask segment */
            pos  = pos + slen                          /* skip mask segment */
            iterate
        end
        /* --------------------------------------------------------------
         * LITERAL CHARACTER
         * Any character that is not part of a mask is copied directly.
         * --------------------------------------------------------------
         */
        out = out || ch
        pos = pos + 1
    end
    /* ------------------------------------------------------------------
     * Restore UTF-8 currency symbols from internal placeholders.
     * ------------------------------------------------------------------
     */
return translate(out,"€£¥","030405"x)
/* ------------------------------------------------------------------
 * Read contiguous numeric mask: 9 $ .
 *
 * Rule:
 *   '.' belongs to the mask only if at least one '9' follows it.
 *   So:
 *      999.99  -> numeric mask
 *      999.    -> mask ends before '.'
 * ------------------------------------------------------------------ */
readNumMask: procedure=.string
  arg s=.string, p=.int,llen=.int
    i = p
    seenDot = 0
    one=1
    ch=.string
    do while i <= llen
/* rxpp: subst1Char(ch,s, i) */
       __jpos=i-1; assembler SETSTRPOS s,__jpos; assembler substr ch,s,one
   /*    ch = substr(s, i, 1) */
       if ch = "9" then i = i + 1
/* rxpp: else if CurrencySymbol() then i = i + 1 */
       else if ch = "$" | ch = "03"x | ch = "04"x | ch = "05"x then i = i + 1
       else if ch = "." then do
         /* only one decimal point allowed, and it must be followed by 9 */
          if seenDot then leave
          if i = llen then leave
/* rxpp: subst1Char(ch,s, i+1) */
          __jpos=i+1-1; assembler SETSTRPOS s,__jpos; assembler substr ch,s,one
          if ch \= "9" then leave
       /*  if  substr(s, i+1, 1)="9" then leave */
          seenDot = 1
          i = i + 1
       end
       else leave
    end
return substr(s, p, i - p)
/* ------------------------------------------------------------------
 * Format numeric field for 9/$/.
 *
 * V1 rule:
 *   - width = total mask length
 *   - decimals = digits after '.'
 *   - numbers are formatted with FORMAT()
 *   - result is right aligned to mask width
 *   - if '$' occurs, place '$' at leftmost occupied position in a simple way
 *
 * This is intentionally simple, not COBOL-exact.
 * ------------------------------------------------------------------
 */
fmtNumMask: procedure=.string
   arg value=.string, mask=.string
 /*  width = length(mask) */
/* rxpp: strlen(width,mask) */
   width=.int; assembler strlen width,mask
  /* dot = pos(".", mask)    */
/* rxpp: fastpos(dot,".", mask) */
   dot = 1; __srch="."; assembler strpos dot,__srch,mask;
   curr = getCurrencyMaskChar(mask)
   /* integer-style mask: no decimal point in picture */
   if dot = 0 then do
      txt = strip(trunc(value))
/* rxpp: strlen(slen,txt) */
      slen=.int; assembler strlen slen,txt
    /* if length(txt) > width then return copies("*", width) */
      if slen > width then return copies("*", width)
      txt = right(txt, width)
      if curr \= "" then do
    /* numStart = pos(strip(txt), txt) */
/* rxpp: fastpos(numStart,strip(txt),txt) */
         numStart = 1; __srch=strip(txt); assembler strpos numStart,__srch,txt;
         if numStart > 1 then txt = overlay(curr, txt, numStart - 1, 1)
      end
      return txt
   end
   /* decimal-style mask */
   /* llen=length(mask) */
/* rxpp: strlen(llen,mask) */
   llen=.int; assembler strlen llen,mask
   decs = llen - dot
   intslots = 0
   decslots = 0
   seenDot = 0
   ch=.string
   one=1
   do i = 1 to llen
/* rxpp: subst1Char(ch,mask, i) */
      __jpos=i-1; assembler SETSTRPOS mask,__jpos; assembler substr ch,mask,one
/*      ch = substr(mask, i, 1) */
      if ch = "." then do
         seenDot = 1
         iterate
      end
/* rxpp: if ch = "9" | CurrencySymbol() then do */
      if ch = "9" | ch = "$" | ch = "03"x | ch = "04"x | ch = "05"x then do
         if seenDot then decslots = decslots + 1
         else intslots = intslots + 1
      end
   end
   txt = strip(format(value, intslots, decs))
/* rxpp: strlen(slen,txt) */
   slen=.int; assembler strlen slen,txt
   if slen > width then return copies("*", width)
   /*   if length(txt) > width then return copies("*", width) */
   txt = right(txt, width)
   if curr \= "" then do
    /*  numStart = pos(strip(txt), txt) */
/* rxpp: fastpos(numStart,strip(txt), txt) */
      numStart = 1; __srch=strip(txt); assembler strpos numStart,__srch,txt;
      if numStart > 1 then txt = overlay(curr, txt, numStart - 1, 1)
   end
return txt
getCurrencyMaskChar: procedure=.string
  arg mask=.string
  /* llen=length(mask) */
/* rxpp: strlen(llen,mask) */
  llen=.int; assembler strlen llen,mask
  one=1
  ch=.string
  do i = 1 to llen
/* rxpp: subst1Char(ch,mask,i) */
     __jpos=i-1; assembler SETSTRPOS mask,__jpos; assembler substr ch,mask,one
/*     ch = substr(mask, i, 1) */
/* rxpp: if CurrencySymbol() then return ch */
     if ch = "$" | ch = "03"x | ch = "04"x | ch = "05"x then return ch
  end
return ""

/* ##define subst1Char(into,string,at)    {__jpos=at-1; assembler SETSTRPOS string,__jpos; assembler substr into,string,one} D*/
/* ##define CurrencySymbol()              {ch = "$" | ch = "03"x | ch = "04"x | ch = "05"x} D*/
/* ##define MaskLen(into,string,offset,llen)   {__indx=.int; do __indx=offset to llen; subst1Char(ch,string,__indx); if ch\= "X" then leave; end; into=__indx-offset} D*/
