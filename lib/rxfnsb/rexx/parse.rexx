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

namespace rxfnsb expose parse

/* ----------------------------------------------------------------------
 * Compile the given template, split it in tokens and token.ktypes
 * ----------------------------------------------------------------------
 */
parse: Procedure=.int
  arg string2Parse=.string, template=.string, expose variable=.string[], expose variable_content=.string[],option=0,upper=0
  token.1=''          ## init token array
  token_type.1=''     ## init types array
  count=parseCompile(template,token,token_type)

  call parseString string2Parse, count, token, token_type,variable,variable_content,template
  if option=1 | upper>0 then do i=1 to variable_content.0
     if option=1 then variable_content.i=strip(variable_content.i)
     if upper=1 then variable_content.i=upper(variable_content.i)
     else if upper=2 then variable_content.i=lower(variable_content.i)
  end
return variable.0
