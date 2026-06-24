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

/* rexx compile a rexx exec to a native executable           */
/* Classic Rexx (ooRexx, REgina), NetRexx and CREXX (when    */
/* ran as a simple script from the crexx wrapper) compatible */

options levelb

parse arg crexx_home execSpec
crexx_home = strip(crexx_home)
execSpec = strip(execSpec)

if crexx_home='' | execSpec='' then do
  say 'usage: crxc.rexx crexx_home execName[.ext]'
  exit 99
end

parse var execSpec execName '.' extension
if extension<>'' then say 'filename extension ignored.'
linkedName = execName || '_linked'
socketLib = ''
crexx_home_upper = translate(crexx_home)
if pos('\', crexx_home) > 0 | substr(crexx_home,2,1) = ':' | pos('/MINGW', crexx_home_upper) > 0 | pos('/MSYS', crexx_home_upper) > 0 then socketLib = ' -lws2_32 -lsecur32 -lcrypt32'

address system '"' || crexx_home || '/rxc" -i "' || crexx_home || '" ' || execName
if rc<>0 then exit rc
address system '"' || crexx_home || '/rxas" ' || execName
if rc<>0 then exit rc
address system '"' || crexx_home || '/rxlink" -s -o "' || linkedName || '" "' || execName || '" "' || crexx_home || '/library"'
if rc<>0 then exit rc
address system '"' || crexx_home || '/rxcpack" -o "' || execName || '" "' || linkedName || '"'
if rc<>0 then exit rc
address system 'gcc -O3 -DNDEBUG -o ' || execName || ' -L "' || crexx_home || '" -lrxvml -lrxpashim -lrxvmplugin -lrxvmref -lplatform "' || crexx_home || '/rxvm_mc_decimal_manual.a" -ldecnumber -lavl_tree -lrxpa -lm' || socketLib || ' ' || execName || '.c'
if rc<>0 then exit rc
exit 0
