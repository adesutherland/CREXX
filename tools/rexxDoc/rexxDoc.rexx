/* REXX */

options levelb
import rxfnsb

main: procedure

/* ----- add here a rexx program with rexxDOC content -----
                    v
                    v                  */
   rexx="C:\Users\PeterJ\CLionProjects\CREXX\260401\lib\classlib\TreeMap.rexx"
/* extract documentation */
   call rexxDoc(rexx,'dump')    /* format either DUMP or HTML */
return

/* ------------------------------------------------------------------
 *  Program:   RexxDoc JavaDoc Extractor
 *
 *  Purpose:
 *      Demonstrates a simple RexxDoc extraction and rendering pipeline, based on JavaDoc:
 *
 *        1. Extract raw /** ... */ blocks from a source file
 *        2. Normalize the comment body
 *        3. Split the block into description + tags
 *        4. Render the result as plain text or HTML
 *
 *  Design:
 *      - Parsing and rendering are kept separate
 *      - Parsing preserves structure
 *      - Rendering interprets inline tags and simple HTML markup
 *      - KISS-oriented: reliable extraction first, richer semantics later
 *  ------------------------------------------------------------------ */
rexxDoc: procedure
   arg file=.string, format='dump'
   format=lower(format)

   rexxContent = loadText(file)

   /* Extract all JavaDoc comment blocks including delimiters */
   founddocs = QEXTRACTALL('/**','*/',rexxContent,'incl')
   do i = 1 to founddocs.0
      do         /* Fresh arrays for each parsed block */
         tagName = .string[]
         tagArg = .string[]
         tagText = .string[]
         description = .string[]

         /* Normalize raw JavaDoc block and split into structure */
  ##       say 999 "'"founddocs.i
         part = rexxDocStrip(founddocs.i)
         tags = splittags(part, description, tagName, tagArg, tagText)

         /* HTML dump example */
         if format='html' then do
            html = dumpRexxDocHtml('RexxDoc HTML Sample', description, tagName, tagArg, tagText)
            say html
         end
         else do   /* Plain text dump */
            call dumpRexxDoc 'RexxDoc Sample', description, tagName, tagArg, tagText
            call lintRexxDoc description, tagName, tagArg, tagText
         end
      end
   end
return


/* ------------------------------------------------------------------
 *  Function:  DUMPREXXDOC
 *  Purpose:   Pretty-print one parsed JavaDoc/RexxDoc block to the
 *             console in a readable, grouped form.
 *
 *  Sections:
 *      - Description
 *      - Parameters
 *      - Returns
 *      - Throws
 *      - Other Tags
 *
 *  Notes:
 *      - Uses renderDocText() for presentation-time cleanup
 *  ------------------------------------------------------------------ */
dumpRexxDoc: procedure
   arg title=.string, description=.string[], tagName=.string[], tagArg=.string[], tagText=.string[]

   if title = '' then title = 'RexxDoc'
   say '=================================================='
   say title
   say '=================================================='

   say 'Description:'
   if description[0] = 0 then
      say '  <none>'
   else do
      do i = 1 to description[0]
         say '  'renderDocText(description[i])
      end
   end
   say

   say 'Parameters:'
   found = 0
   do i = 1 to tagName[0]
      if tagName[i] = 'param' then do
         found = 1
         say '  'tagArg[i]': 'renderDocText(tagText[i])
      end
   end
   if \found then say '  <none>'

   say

   say 'Returns:'
   found = 0
   do i = 1 to tagName[0]
      if tagName[i] = 'return' then do
         found = 1
         say '  'renderDocText(tagText[i])
      end
   end
   if \found then say '  <none>'

   say

   say 'Throws:'
   found = 0
   do i = 1 to tagName[0]
      if tagName[i] = 'throws' | tagName[i] = 'exception' then do
         found = 1
         say '  'tagArg[i]': 'renderDocText(tagText[i])
      end
   end
   if \found then say '  <none>'

   say

   say 'Other Tags:'
   found = 0
   do i = 1 to tagName[0]
      select
         when tagName[i] = 'param' then nop
         when tagName[i] = 'return' then nop
         when tagName[i] = 'throws' then nop
         when tagName[i] = 'exception' then nop
         otherwise do
            found = 1
            if tagArg[i] \= '' then
               say '  @'tagName[i] tagArg[i]' -> 'renderDocText(tagText[i])
            else
               say '  @'tagName[i]' -> 'renderDocText(tagText[i])
         end
      end
   end
   if \found then say '  <none>'
return


/* ------------------------------------------------------------------
 *  Function:  DUMPREXXDOCHTML
 *  Purpose:   Produce a minimal HTML representation of one parsed
 *             JavaDoc/RexxDoc block.
 *
 *  Output:
 *      Returns a single HTML document as a string.
 *
 *  Current Scope:
 *      - Description only
 *      - Tag sections can be added later in the same style
 *
 *  Notes:
 *      - Uses renderDocText() for normalization
 *      - Uses escapeHtml() to make content safe for HTML output
 *  ------------------------------------------------------------------ */
dumpRexxDocHtml: procedure=.string
   arg title=.string, description=.string[], tagName=.string[], tagArg=.string[], tagText=.string[]

   html = ''
   nl = '0a'x

   if title = '' then title = 'RexxDoc'

   html = html || '<html>' || nl
   html = html || '<head><meta charset="utf-8"><title>' || title || '</title></head>' || nl
   html = html || '<body>' || nl
   html = html || '<h1>' || title || '</h1>' || nl

   html = html || '<h2>Description</h2>' || nl
   if description[0] = 0 then
      html = html || '<p>&lt;none&gt;</p>' || nl
   else do
      do i = 1 to description[0]
         if strip(description[i]) \= '' then
            html = html || '<p>' || escapeHtml(renderDocText(description[i])) || '</p>' || nl
      end
   end

   html = html || '</body>' || nl || '</html>' || nl
return html


/* ------------------------------------------------------------------
 *  Function:  RENDERDOCTEXT
 *  Purpose:   Apply presentation-time rendering to one text fragment.
 *
 *  Pipeline:
 *      1. Trim trailing blanks
 *      2. Expand inline JavaDoc tags
 *      3. Remove or normalize simple embedded HTML markup
 *
 *  Notes:
 *      - This is a rendering helper only
 *      - It does not change the parsed source data
 *  ------------------------------------------------------------------ */
renderDocText: procedure=.string
   arg s=.string
return htmlDoc(inlineDoc(strip(s, 'T')))


/* ------------------------------------------------------------------
 *  Function:  HTMLDOC
 *  Purpose:   Normalize a small subset of JavaDoc/HTML markup for
 *             readable plain-text or safe downstream rendering.
 *
 *  Supported:
 *      <p>...</p>            -> paragraph markers removed
 *      <br>, <br/>, <br />   -> newline inserted
 *      <i>...</i>            -> keep inner text
 *      <b>...</b>            -> keep inner text
 *      <strong>...</strong>  -> keep inner text
 *
 *  Notes:
 *      - This is intentionally minimal
 *      - It is not a full HTML parser
 *  ------------------------------------------------------------------ */
htmlDoc: procedure=.string
   arg s=.string

   s = changestr('<p>',s,'')
   s = changestr('</p>',s,'')
   s = changestr('<br>',s,'0a'x)
   s = changestr('<br/>',s,'0a'x)
   s = changestr('<br />',s,'0a'x)

   s = stripTag(s, '<i>', '</i>')
   s = stripTag(s, '<b>', '</b>')
   s = stripTag(s, '<strong>', '</strong>')
return s


/* ------------------------------------------------------------------
 *  Function:  INLINEDOC
 *  Purpose:   Expand simple inline JavaDoc tags in a text line.
 *
 *  Supported:
 *      {@code text}         -> text
 *      {@link Target}       -> Target
 *      {@link Target Label} -> Label
 *
 *  Notes:
 *      - This is intentionally lightweight
 *      - Nested inline tags are not handled
 *  ------------------------------------------------------------------ */
inlineDoc: procedure=.string
   arg s=.string

   s = expandInlineCode(s)
   s = expandInlineLink(s)
return s


/* ------------------------------------------------------------------
 *  Function:  EXPANDINLINECODE
 *  Purpose:   Replace all {@code ...} fragments with their inner text.
 *
 *  Example:
 *      {@code containsKey} -> containsKey
 *  ------------------------------------------------------------------ */
expandInlineCode: procedure=.string
   arg s=.string
   token = '{@code '

   do forever
      p1 = pos(token, s)
      if p1 = 0 then leave

      p2 = pos('}', s, p1 + length(token))
      if p2 = 0 then leave

      inner = substr(s, p1 + length(token), p2 - p1 - length(token))
      s = left(s, p1 - 1) || inner || substr(s, p2 + 1)
   end
return s


/* ------------------------------------------------------------------
 *  Function:  EXPANDINLINELINK
 *  Purpose:   Replace all {@link ...} fragments.
 *
 *  Rules:
 *      {@link X}         -> X
 *      {@link X label}   -> label
 *
 *  Notes:
 *      - The first token is interpreted as the target
 *      - Remaining text, if any, is interpreted as the display label
 *  ------------------------------------------------------------------ */
expandInlineLink: procedure=.string
   arg s=.string

   token = '{@link '

   do forever
      p1 = pos(token, s)
      if p1 = 0 then leave

      p2 = pos('}', s, p1 + length(token))
      if p2 = 0 then leave

      inner = strip(substr(s, p1 + length(token), p2 - p1 - length(token)))

      parse var inner target label
      label = strip(label)

      if label \= '' then
         repl = label
      else
         repl = target

      s = left(s, p1 - 1) || repl || substr(s, p2 + 1)
   end

return s


/* ------------------------------------------------------------------
 *  Function:  STRIPTAG
 *  Purpose:   Remove a simple matching tag pair while preserving the
 *             enclosed text.
 *
 *  Example:
 *      <i>abc</i>  -> abc
 *
 *  Notes:
 *      - Repeats until no more matching pairs are found
 *      - Assumes simple, non-nested usage of the same tag type
 *  ------------------------------------------------------------------ */
stripTag: procedure=.string
   arg s=.string, open=.string, close=.string

   do forever
      p1 = pos(open, s)
      if p1 = 0 then leave

      p2 = pos(close, s, p1 + length(open))
      if p2 = 0 then leave

      inner = substr(s, p1 + length(open), p2 - p1 - length(open))
      s = left(s, p1 - 1) || inner || substr(s, p2 + length(close))
   end
return s


/* ------------------------------------------------------------------
 *  Function:  ESCAPEHTML
 *  Purpose:   Escape special characters so text can safely be embedded
 *             inside generated HTML.
 *
 *  Converts:
 *      &  -> &amp;
 *      <  -> &lt;
 *      >  -> &gt;
 *      "  -> &quot;
 *      '  -> &#39;
 *
 *  Notes:
 *      - Order matters: '&' must be escaped first
 *  ------------------------------------------------------------------ */
escapeHtml: procedure=.string
   arg s=.string

   s = changestr('&', s, '&amp;')
   s = changestr('<', s, '&lt;')
   s = changestr('>', s, '&gt;')
   s = changestr('"', s, '&quot;')
   s = changestr("'", s, '&#39;')
return s


/* ------------------------------------------------------------------
 *  Function:  SPLITTAGS
 *  Purpose:   Split cleaned JavaDoc text into:
 *               - description text
 *               - parallel tag arrays
 *
 *  Usage:
 *      tagCount = splittags(cleanText, description, tagName, tagArg, tagText)
 *
 *  Output:
 *      description - description block
 *      tagName[i]  - tag name (param, return, throws, ...)
 *      tagArg[i]   - optional argument (<K>, key, ExceptionName, ...)
 *      tagText[i]  - tag text with continuation lines merged
 *
 *  Tag Rules:
 *      - lines before the first @tag belong to description
 *      - a line starting with @ begins a new tag
 *      - later non-@ lines continue the current tag
 *
 *  Returns:
 *      number of tags found
 *  ------------------------------------------------------------------ */
splittags: procedure=.int
   arg text=.string, expose description=.string[], expose tagName=.string[], expose tagArg=.string[], expose tagText=.string[]

   nl = '0a'x
   inTags = 0
   curTag = 0
   rest = .string
   line = .string

   do while text \= ''
      p = pos(nl, text)
      if p = 0 then do
         line = text
         text = ''
      end
      else do
         line = left(text, p - 1)
         text = substr(text, p + 1)
      end

      if right(line,1) = '0d'x then
         line = left(line, length(line) - 1)

      raw   = line
      sline = strip(line)

      /* ------------------------------------------------------------
       * Tag mode:
       *   - blank line extends current tag as a space
       *   - @... starts a new tag
       *   - any other line continues current tag text
       * ------------------------------------------------------------ */
      if inTags then do
         if sline = '' then do
            if curTag > 0 & tagText[curTag] \= '' then
               tagText[curTag] = tagText[curTag] || ' '
            iterate
         end

         if left(sline,1) = '@' then do
            parse var sline '@' name rest
            name = lower(strip(name))
            rest = strip(rest)

            tagName[tagName[0]+1] = name
            curTag = tagName[0]
            tagArg[curTag]  = ''
            tagText[curTag] = ''

            select
               when name = 'param' then do
                  parse var rest argx restText
                  tagArg[curTag]  = argx
                  tagText[curTag] = strip(restText)
               end
               when name = 'throws' | name = 'exception' then do
                  parse var rest argx restText
                  tagArg[curTag]  = argx
                  tagText[curTag] = strip(restText)
               end
               otherwise do
                  tagText[curTag] = rest
               end
            end
            iterate
         end

         if curTag > 0 then do
            if tagText[curTag] = '' then
               tagText[curTag] = sline
            else
               tagText[curTag] = tagText[curTag] || ' ' || sline
         end
         iterate
      end

      /* ------------------------------------------------------------
       * Description mode:
       *   - blank line stays in description
       *   - first @... switches to tag mode
       *   - anything else belongs to description
       * ------------------------------------------------------------ */
      if sline = '' then do
         description[description[0]+1] = ''
         iterate
      end

      if left(sline,1) = '@' then do
         inTags = 1

         parse var sline '@' name rest
         name = lower(strip(name))
         rest = strip(rest)

         tagName[tagName[0]+1] = name
         curTag = tagName[0]
         tagArg[curTag]  = ''
         tagText[curTag] = ''

         select
            when name = 'param' then do
               parse var rest argx restText
               tagArg[curTag]  = argx
               tagText[curTag] = strip(restText)
            end
            when name = 'throws' | name = 'exception' then do
               parse var rest argx restText
               tagArg[curTag]  = argx
               tagText[curTag] = strip(restText)
            end
            otherwise do
               tagText[curTag] = rest
            end
         end
         iterate
      end

      description[description[0]+1] = strip(raw,'T')
   end

return tagName[0]


/* ------------------------------------------------------------------
 *  Function:  REXXDOCSTRIP
 *  Purpose:   Normalize one raw JavaDoc block into plain multi-line text.
 *
 *  Actions:
 *      - remove opening /** and closing */
 *      - remove decorative leading '*' on each line
 *      - preserve line structure
 *      - trim leading/trailing empty lines from the result
 *
 *  Input:
 *      full raw extracted block, including delimiters
 *
 *  Output:
 *      cleaned JavaDoc body as one multi-line string
 *  ------------------------------------------------------------------ */
rexxdocstrip: procedure=.string
   arg text=.string

   nl = '0a'x
   out = ''
   if text = '' then return ''
   ppi=pos('/**',text)
   if ppi>0 then text = substr(text, ppi+3)
   ppj=pos('*/',text)
   if ppj>0 then text = left(text, ppj - 1)

   line = .string

   do while text \= ''
      p = pos(nl, text)

      if p = 0 then do
         line = text
         text = ''
      end
      else do
         line = left(text, p - 1)
         text = substr(text, p + 1)
      end

      if right(line,1) = '0d'x then line = left(line, length(line) - 1)

      s = strip(line, 'L')

      /* Remove one decorative leading '*' if present */
      if left(s,1) = '*' then do
         s = substr(s,2)
         if left(s,1) = '*' then s = substr(s,2)
         if left(s,1) = ' ' then s = substr(s,2)
      end
      if out = '' then out = s
      else out = out || nl || s
   end

   /* Trim leading blank lines */
   do while out \= ''
      p = pos(nl, out)
      if p = 0 then leave
      first = left(out, p - 1)
      if strip(first) \= '' then leave
      out = substr(out, p + 1)
   end

   /* Trim trailing blank lines */
   do while out \= ''
      p = lastpos(nl, out)
      if p = 0 then leave
      last = substr(out, p + 1)
      if strip(last) \= '' then leave
      out = left(out, p - 1)
   end
return out

/* ------------------------------------------------------------------
 *  Function:  LINTREXXDOC
 *  Purpose:   Check one parsed RexxDoc block for likely documentation
 *             problems and print warnings.
 *
 *  Current Rules:
 *      1. Description line starts with "return ", "throws ", or "param "
 *         -> likely missing '@'
 *      2. Empty tag text
 *      3. Duplicate @return
 *      4. Empty documentation block
 *
 *  Notes:
 *      - Strict mode: does not auto-correct anything
 *      - Intended as a lightweight lint pass
 *  ------------------------------------------------------------------ */
lintRexxDoc: procedure
   arg description=.string[], tagName=.string[], tagArg=.string[], tagText=.string[],quiet=1

   warnings = 0
   returnCount = 0

   /* ------------------------------------------------------------
    * Rule 1: suspicious description lines that look like tags
    * ------------------------------------------------------------ */
   do i = 1 to description[0]
      line = strip(description[i])
      lowerLine = lower(line)

      if left(lowerLine, 7) = 'return ' then do
         if warnings = 0 then say 'Lint:'
         warnings = warnings + 1
         say '  WARNING: possible missing @return tag: ' line
      end
      else if left(lowerLine, 7) = 'throws ' then do
         if warnings = 0 then say 'Lint:'
         warnings = warnings + 1
         say '  WARNING: possible missing @throws tag: ' line
      end
      else if left(lowerLine, 6) = 'param ' then do
         if warnings = 0 then say 'Lint:'
         warnings = warnings + 1
         say '  WARNING: possible missing @param tag: ' line
      end
   end

   /* ------------------------------------------------------------
    * Rule 2 + 3: tag checks
    * ------------------------------------------------------------ */
   do i = 1 to tagName[0]
      name = lower(strip(tagName[i]))
      txt  = strip(tagText[i])

      if name = 'return' then
         returnCount = returnCount + 1

      if txt = '' then do
         if warnings = 0 then say 'Lint:'
         warnings = warnings + 1
         say '  WARNING: empty tag text for @'name
      end
   end

   if returnCount > 1 then do
      if warnings = 0 then say 'Lint:'
      warnings = warnings + 1
      say '  WARNING: multiple @return tags found'
   end

   /* ------------------------------------------------------------
    * Rule 4: completely empty block
    * ------------------------------------------------------------ */
   if description[0] = 0 & tagName[0] = 0 then do
      if warnings = 0 then say 'Lint:'
      warnings = warnings + 1
      say '  WARNING: empty documentation block'
   end

   if warnings = 0 & quiet=0 then say 'Lint: <none>'

return