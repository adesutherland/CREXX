#!/usr/bin/env python3
"""Independent full-format recognizer and flat-path source transformation."""
from pathlib import Path
import json,re
ROOT=Path(__file__).resolve().parents[1]
class Invalid(Exception): pass
class Reader:
    def __init__(self,s): self.s=s; self.i=0; self.flat=[]
    def peek(self): return self.s[self.i:self.i+1]
    def space(self):
        while True:
            if self.peek() and self.peek() in ' \r\n\t': self.i+=1
            elif self.s.startswith('/*',self.i):
                end=self.s.find('*/',self.i+2)
                if end<0 or any(ord(c)>127 for c in self.s[self.i:end+2]): raise Invalid()
                self.i=end+2
            else: return
    def need(self,c):
        if self.peek()!=c: raise Invalid()
        self.i+=1
    def name(self):
        m=re.match(r'[A-Za-z_][A-Za-z_0-9]*',self.s[self.i:])
        if not m: raise Invalid()
        self.i+=len(m[0]); return m[0]
    def value(self):
        start=self.i; c=self.peek()
        if c=='[':
            self.i+=1; self.space()
            if self.peek()!=']':
                self.value(); self.space()
                while self.peek()==',':
                    self.i+=1; self.space(); self.value(); self.space()
            self.need(']')
        elif c=='"':
            self.i+=1
            while self.peek()!='"':
                c=self.peek()
                if not c or not 32<=ord(c)<=126: raise Invalid()
                self.i+=1
                if c=='\\':
                    if self.peek() not in ['"','\\']: raise Invalid()
                    self.i+=1
            self.i+=1
        else:
            m=re.match(r'-?[0-9]+|true|false',self.s[self.i:])
            if not m: raise Invalid()
            self.i+=len(m[0])
        return self.s[start:self.i]
    def entries(self,prefix=''):
        self.space()
        while self.peek() and self.peek()!='}':
            name=self.name(); self.space()
            if self.peek()=='{':
                self.i+=1; self.entries(prefix+name+'.'); self.need('}')
            else:
                self.need('='); self.space(); value=self.value(); self.space(); self.need(';')
                self.flat.append(prefix+name+'='+value+'\n')
            self.space()
    def document(self):
        self.entries()
        if self.i!=len(self.s): raise Invalid()
        return ''.join(self.flat)
seed='a{x=1;}'
cases={'', '/* only comment */', seed,
       '/* header */ site { title="Demo"; data { count=12; flags=[true, /*flag*/ false, [1,-2]]; } } tail="done";',
       'true { false=true; } false="a\\"b";',
       'x="/* string, not trivia */"; /***/ empty{}',
       'a { x="\\\\"; }', 'a { /* nested /* is text */ x=1; }',
       'a{x=1;} /* open', 'a{x=1;', 'a{x=1;} trailing', 'x=[1,];', 'x="é";',
       'x=1;/*é*/', 'x=1;/*\0*/', 'x=1;\0', 'x=truefalse;', 'x="unterminated;', 'x=-;'}
for i in range(len(seed)):
    cases.update((seed[:i]+seed[i+1:],seed[:i]+'?'+seed[i+1:],seed[:i]))
for i in range(len(seed)+1):
    for c in (' ', '}', ';'): cases.add(seed[:i]+c+seed[i:])
for depth in range(1,5):
    cases.add('n{'*depth+'x="value";'+'}'*depth)
rows=[]
for text in sorted(cases):
    try: flat=Reader(text).document(); status=0
    except Invalid: flat=''; status=1
    rows.append(f'{text.encode().hex().upper()}|{status}|{flat.encode().hex().upper()}\n')
f=ROOT/'evidence/config-sections-reference.txt'; f.write_text(''.join(rows))
print('Independent section/comment format cases:',len(rows))
