/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* rexx */
options levelb

namespace rxfnsb expose qsplitsafe

/* ------------------------------------------------------------------
 * Function: QSPLITSAFE
 * ------------------------------------------------------------------ */
qsplitsafe: procedure=.string[]
  arg text=.string, sep=.string, start=1, pairs='()'

  parts = .string[]

  if sep = '' then do
     parts.1 = text
     return parts
  end

  if start < 1 then start = 1

  tlen   = length(text)
  seplen = length(sep)

  /* validate pairs string: must be even length */
  if (length(pairs)%2) \= 0 then pairs = ''

  idx  = 0
  from = start
  pos  = start

  /* quote state */
  inq = 0
  qch = ''

  /* delimiter pair stack (MUST persist across loop!) */
  stack = .string[]
  depth = 0
  sp=0

  do while pos <= tlen - seplen + 1
     ch = substr(text, pos, 1)

     /* --- quote handling --- */
     if inq then do
        if ch = qch then inq = 0
        pos = pos + 1
        iterate
     end
     else do
        if ch = "'" | ch = '"' then do
           inq = 1
           qch = ch
           pos = pos + 1
           iterate
        end
     end

     /* --- delimiter pair tracking --- */
     depth = __qsafe_pair_step(ch, pairs, stack, depth,sp)

     /* --- split only when not inside pairs --- */
     if depth = 0 then do
        if substr(text, pos, seplen) = sep then do
          idx = idx + 1
          parts[idx] = substr(text, from, pos - from)
          from = pos + seplen
          pos  = from
          iterate
        end
     end

     pos = pos + 1
  end

  /* trailing part */
  idx = idx + 1
  parts[idx] = substr(text, from)

return parts


/* ------------------------------------------------------------------
 * Internal: __qsafe_pair_step
 *
 * Updates `stack` and returns new depth.
 * stack[0] holds sp (stack pointer).
 * ------------------------------------------------------------------ */
__qsafe_pair_step: procedure=.int
  arg ch=.string, pairs=.string, expose stack=.string[], depth=.int, expose sp=.int

  /* if no pairs configured, don't change depth */
  if pairs = '' then return depth

  /* opening delimiter? */
  oi = __qsafe_open_index(ch, pairs)
  if oi > 0 then do
     closer = substr(pairs, oi + 1, 1)
     sp = sp + 1
     stack[sp] = closer
     return depth + 1
  end

  /* closing delimiter? must match top-of-stack */
  if depth > 0 & sp > 0 then do
     if ch = stack[sp] then do
        stack[sp] = ''
        sp = sp - 1
        return depth - 1
     end
  end

return depth


__qsafe_open_index: procedure=.int
  arg ch=.string, pairs=.string
  plen = length(pairs)
  i = 1
  do while i <= plen
     if substr(pairs, i, 1) = ch then return i
     i = i + 2
  end
return 0