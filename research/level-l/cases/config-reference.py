#!/usr/bin/env python3
"""Independent complete TinyConfig consumer, not derived from llfront's graph."""
from pathlib import Path
from fixture_support import write_fixture
class Invalid(Exception): pass
class Reader:
    def __init__(self, text): self.s=text; self.i=0
    def peek(self): return self.s[self.i:self.i+1]
    def space(self):
        while self.peek() and self.peek() in ' \t\r\n': self.i+=1
    def need(self,c):
        if self.peek()!=c: raise Invalid()
        self.i+=1
    def node(self,tag,start,children=''):
        return f'{tag}@{start}:{self.i}[{children}]'
    def name(self):
        start=self.i
        if not self.peek() or self.peek() not in 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_': raise Invalid()
        self.i+=1
        while self.peek() and self.peek() in 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789': self.i+=1
        return self.node('name',start)
    def value(self):
        start=self.i; ch=self.peek()
        if not ch: raise Invalid()
        if ch=='[':
            self.i+=1; self.space(); children=''
            if self.peek()!=']':
                children+=self.value(); self.space()
                while self.peek()==',':
                    self.i+=1; self.space(); children+=self.value(); self.space()
            self.need(']'); return self.node('array',start,children)
        if ch=='"':
            self.i+=1
            while self.peek()!='"':
                ch=self.peek()
                if not ch or not 32<=ord(ch)<=126: raise Invalid()
                self.i+=1
                if ch=='\\':
                    if self.peek() not in ['"','\\']: raise Invalid()
                    self.i+=1
            self.i+=1; return self.node('string',start)
        if ch=='-' or ch.isascii() and ch.isdigit():
            if ch=='-': self.i+=1
            digits=self.i
            while self.peek() and self.peek() in '0123456789': self.i+=1
            if self.i==digits: raise Invalid()
            return self.node('number',start)
        for word in ['true','false']:
            if self.s.startswith(word,self.i):
                self.i+=len(word); return self.node('boolean',start)
        raise Invalid()
    def document(self):
        children=''; self.space()
        while self.i<len(self.s):
            start=self.i; child=self.name(); self.space(); self.need('='); self.space()
            child+=self.value(); self.space(); self.need(';')
            children+=self.node('entry',start,child); self.space()
        return self.node('document',0,children)
seed='a=[1,"x"];'
cases={'',seed,'port = 8080; name="demo"; flags=[true, false, [1,-2], "a\\"b"];','x=[]; y=[[]]; x=1;','z="\\\\";','x=0;\n  y=-001;','nul="\0";','x="é";'}
for i in range(len(seed)):
    cases.add(seed[:i]+seed[i+1:])
    for c in '?],': cases.add(seed[:i]+c+seed[i+1:])
for i in range(len(seed)+1):
    for c in '[,;': cases.add(seed[:i]+c+seed[i:])
for i in range(len(seed)): cases.add(seed[:i])
rows=[]
for text in sorted(cases):
    try: tree=Reader(text).document(); status=0
    except Invalid: tree=''; status=1
    rows.append(f'{text.encode().hex().upper()}|{status}|{tree}\n')
out=Path(__file__).resolve().parents[1]/'cases/fixtures/config-reference.txt'
write_fixture(out,''.join(rows)); print(f'{len(rows)} independent TinyConfig cases')
