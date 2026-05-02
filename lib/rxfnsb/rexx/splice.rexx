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

namespace rxfnsb expose splice

/* ----------------------------------------------------------------------
 * SPLICE(needle, haystack, at, len)
 *
 * Replaces a substring of HAYSTACK with NEEDLE at a given position.
 *
 * Starting at the 1-based character position AT, exactly LEN characters
 * are removed from HAYSTACK and replaced with NEEDLE. The resulting
 * string may grow, shrink, or remain the same length depending on the
 * length of NEEDLE.
 *
 * This is a purely positional operation; no searching is performed.
 * No padding is applied at any time.
 *
 * PARAMETERS
 *   needle   : string   Replacement string
 *   haystack : string   Original string to be modified
 *   at       : integer  1-based start position for the replacement
 *   len      : integer  Number of characters to remove
 *
 * SEMANTICS / EDGE CASES
 *   - If AT < 1, the operation is a no-op and HAYSTACK is returned unchanged
 *   - If AT > length(HAYSTACK) + 1, AT is clamped to length(HAYSTACK) + 1
 *     (i.e. the replacement is appended)
 *   - If LEN < 0, it is treated as 0 (pure insertion)
 *   - If LEN exceeds the remaining string length, all remaining characters
 *     are removed
 *   - If NEEDLE is empty, the operation deletes LEN characters
 *
 * NOTES
 *   - This function does not pad with blanks
 *   - Behaviour is deterministic and dialect-independent
 *   - Conceptually equivalent to: left + replacement + right
 *
 * RETURNS
 *   The resulting string after replacement
 *
 * COMPARISON TO OVERLAY()
 *   OVERLAY() overwrites a fixed-width region (padding if required)
 *   SPLICE() removes a region and inserts a replacement, shifting the
 *   remainder of the string with no padding.
 * ----------------------------------------------------------------------
 */
splice: procedure=.string
  arg needle=.string,haystack=.string, at=1, len=.int
  if at < 1 then return haystack
  if len < 0 then len = 0

  hlen = length(haystack)
  if at > hlen + 1 then at = hlen + 1   /* clamp to append */
return left(haystack, at-1, '') || needle || substr(haystack, at+len)
