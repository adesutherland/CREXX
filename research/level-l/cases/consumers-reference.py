#!/usr/bin/env python3
"""Independent regex lexer and recursive readers for five pinned-source fragments.

This is a correctness oracle, not a benchmark or a claim of dialect conformance.
The subset/gaps are listed in CONSUMERS.md. No native grammar is read here.
"""
from pathlib import Path
from fixture_support import write_fixture
import json,re
R=Path(__file__).resolve().parents[1]
class Invalid(Exception): pass
KEYS={'pascal':{'BEGIN':20,'END':21},'plm':{'DO':20,'END':21,'DECLARE':22,'BYTE':23},
      'basic':{'LET':20,'PRINT':21,'FOR':22,'TO':23,'NEXT':24},'lisp':{},'prolog':{}}
def lex(lang,s):
    # Regex implementation is independent of the native Thompson/subset code.
    # Ordered ties are spelled out here, rather than imported from its rules.
    if lang in ('pascal','plm','basic'):
        pats=[(-1,r'[ \t\r\n]+' if lang!='basic' else r'[ \t\r]+')]
        if lang=='pascal':
            pats += [(-2,r'\{[^}]*\}'),(-3,r'\(\*(?:[^*]|\*+[^*)])*\*+\)'),(-4,r'//[^\n]*'),
                     (1,r'[A-Za-z_][A-Za-z_0-9]*'),(2,r'(?:[0-9]+(?:\.[0-9]+)?(?:[eE][+-]?[0-9]+)?|\$[0-9A-Fa-f]+)'),
                     (3,r"(?:'(?:[^']|'')*'|#[0-9]+)+"),(4,':='),(10,r'\.\.')]
        elif lang=='plm':
            pats[0]=(-1,r'[ \t\r\n\f]+')
            pats += [(-2,r'/\*(?:[^*]|\*+[^*/])*\*+/'),(1,r'[A-Za-z_][A-Za-z_0-9$]*'),
                     (2,r'[0-9][A-Za-z_0-9$]*'),(3,r"'(?:[^']|'')*'"),(4,'=')]
        else:
            pats += [(-2,r"'[^\n]*"),(1,r'[A-Za-z_][A-Za-z_0-9]*[$%!#]?'),
                     (2,r'[0-9]+(?:\.[0-9]*)?(?:[eEdD][+-]?[0-9]+)?[%!#]?'),
                     (3,r'"(?:[^"]|"")*"'),(4,'='),(10,'\n')]
        pats += [(5,';'),(6,r'\+'),(7,r'\*'),(8,r'\('),(9,r'\)')]
    elif lang=='lisp':
        pats=[(-1,r'[ \t\r\n]+'),(-2,r';[^\n]*'),(2,r'-?[0-9]+'),
              (1,r'''[^ \t\r\n()'";|.]+'''),(3,r'"(?:[^"\\]|\\.)*"'),
              (4,r'\('),(5,r'\)'),(6,"'"),(7,r'\.'),(8,r'\|[^|]*\|')]
    else:
        pats=[(-1,r'[ \t\r\n]+'),(-2,r'%[^\n]*'),(-3,r'/\*(?:[^*]|\*+[^*/])*\*+/'),
              (1,r'[a-z][A-Za-z_0-9]*'),(2,r'[A-Z_][A-Za-z_0-9]*'),(3,r'[0-9]+'),
              (4,r"'(?:[^'\\]|''|\\.)*'"),(5,r'\('),(6,r'\)'),(7,','),(8,r'\['),(9,r'\]'),
              (10,r'\|'),(11,':-'),(12,r'\.')]
    at=0;raw=[];tokens=[]
    while at<len(s):
        choices=[]
        for priority,(kind,pattern) in enumerate(pats):
            m=re.match(pattern,s[at:])
            if m: choices.append((len(m[0]),-priority,kind))
        if not choices or ord(s[at])>127: raise Invalid()
        n,_,kind=max(choices)
        if any(ord(c)>127 for c in s[at:at+n]): raise Invalid()
        if kind>0:
            raw.append((kind,at,at+n))
            if kind==1: kind=KEYS[lang].get(s[at:at+n].upper(),kind)
            tokens.append((kind,at,at+n))
        at+=n
    if lang=='basic':
        clean=[];start=True
        for token in tokens:
            kind=token[0]
            if start and kind==2:start=False;continue
            clean.append(token);start=kind==10
        tokens=clean
    return raw,tokens
class Reader:
    def __init__(self,lang,tokens): self.lang=lang;self.t=tokens;self.i=0;self.nodes=[]
    def peek(self): return self.t[self.i][0] if self.i<len(self.t) else 0
    def take(self,k):
        if self.peek()!=k: raise Invalid()
        self.i+=1
    def node(self,label,fn):
        start=self.i; result=fn()
        if self.i>start: self.nodes.append((label,self.t[start][1],self.t[self.i-1][2]))
        return result
    def expression(self):
        self.product()
        while self.peek()==6: self.i+=1;self.product()
    def product(self):
        self.atom()
        while self.peek()==7: self.i+=1;self.atom()
    def atom(self):
        if self.peek()==1: self.node('name',lambda:self.take(1))
        elif self.peek() in (2,3): self.node('literal',lambda:self.take(self.peek()))
        elif self.peek()==8: self.i+=1;self.expression();self.take(9)
        else: raise Invalid()
    def assignment(self):
        def body(): self.node('name',lambda:self.take(1));self.take(4);self.expression()
        self.node('assignment',body)
    def block(self):
        def body():
            self.take(20)
            if self.lang=='plm': self.take(5)
            while self.peek()!=21:
                if self.peek()==20: self.block()
                elif self.lang=='plm' and self.peek()==22:
                    def declaration(): self.take(22);self.node('name',lambda:self.take(1));self.take(23);self.take(5)
                    self.node('declaration',declaration)
                else:
                    self.assignment()
                    if self.lang=='plm': self.take(5)
                if self.lang=='pascal': self.take(5)
            self.take(21)
            if self.lang=='plm': self.take(5)
        self.node('block',body)
    def line(self):
        if self.peek()==10: self.i+=1;return
        if self.peek()==2: self.i+=1
        if self.peek()==22:
            def loop():
                self.take(22);self.node('name',lambda:self.take(1));self.take(4);self.expression();self.take(23);self.expression();self.take(10)
                while self.peek()!=24 and not (self.peek()==2 and self.i+1<len(self.t) and self.t[self.i+1][0]==24): self.line()
                if self.peek()==2:self.i+=1
                self.take(24);self.node('name',lambda:self.take(1));self.take(10)
            self.node('loop',loop)
        elif self.peek()==21:
            def printing():
                self.take(21);self.expression()
                while self.peek()==5:self.i+=1;self.expression()
            self.node('print.statement',printing);self.take(10)
        else:
            if self.peek()==20:self.i+=1
            self.assignment();self.take(10)
    def form(self):
        if self.peek()==6:
            def quote(): self.take(6);self.form()
            self.node('quoted.form',quote)
        elif self.peek()==4:
            def listing():
                self.take(4)
                if self.peek()!=5:
                    self.form()
                    while self.peek() not in (5,7):self.form()
                    if self.peek()==7:self.i+=1;self.form()
                self.take(5)
            self.node('list',listing)
        elif self.peek() in (1,2,3,8):self.node('atom',lambda:self.take(self.peek()))
        else:raise Invalid()
    def term(self):
        if self.peek() in (1,4):
            if self.i+1<len(self.t) and self.t[self.i+1][0]==5:
                def compound():
                    self.node('name',lambda:self.take(self.peek()));self.take(5);self.arguments();self.take(6)
                self.node('compound',compound)
            else:self.node('name',lambda:self.take(self.peek()))
        elif self.peek() in (2,3):self.i+=1
        elif self.peek()==8:
            def listing():
                self.take(8)
                if self.peek()!=9:
                    self.arguments()
                    if self.peek()==10:self.i+=1;self.term()
                self.take(9)
            self.node('list',listing)
        else:raise Invalid()
    def arguments(self):
        self.term()
        while self.peek()==7:self.i+=1;self.term()
    def clause(self):
        def body():
            self.term()
            if self.peek()==11:self.i+=1;self.arguments()
            self.take(12)
        self.node('clause',body)
    def document(self):
        if self.lang in ('pascal','plm'):self.block()
        else:
            while self.peek(): {'basic':self.line,'lisp':self.form,'prolog':self.clause}[self.lang]()
        if self.peek():raise Invalid()
        return ''.join(sorted(f'{label}@{start}:{end};' for label,start,end in self.nodes))
SEEDS={
 'pascal':['begin x:=1+2*3; begin value:=x; end; end',"BeGiN s:='can''t'#13#10'stop'; x:=$ff; END",'begin (* note *) x:=1.25e+2; {rest} end', 'begin end', 'begin x:=1..2; end'],
 'plm':['DO; DECLARE A BYTE; A=0FFH; DO; A$B=A+1010B; END; END;',"do; word='can''t'; /* trivia */ end;",'DO; END;'],
 'basic':['10 FOR I=1 TO 2\n20 FOR J=1 TO 3\n30 PRINT I;J\n40 NEXT J\n50 NEXT I\n',
          'LET total%=1D+2\nPRINT "hello ""world""";total%\n',"10 x=1 'tail\n20 PRINT x\n", 'FORX=3\n', ''],
 'lisp':["'(a (b 12) . tail)", '(define (square x) (* x x)) ; end\n', '(|Case Sensitive| "escaped\\\" quote" -12)', '()', "'", '(a . b c)'],
 'prolog':["parent('O''Brien',X). ancestor(X,Y):-parent(X,Y),parent(Y,Z).",'member(X,[X|_]). member(X,[H|T]):-member(X,T).',"p('A',A,_A). % atom versus variables\n",'p([a,b,[c]]).', 'p.q.']}
rows=[];records=[]
for lang,seeds in SEEDS.items():
    candidates={s:'development' for s in seeds}
    seed=seeds[0]
    for i in range(len(seed)):
        # Deterministic frozen held-out partition; no repair-policy tuning.
        split='heldout' if i%3==1 else 'development'
        candidates.setdefault(seed[:i]+seed[i+1:],split)
        if i%4==0:candidates.setdefault(seed[:i]+'@'+seed[i+1:],split)
    for s in [seed[:-1],seed[:len(seed)//2],seed+'\0',seed+'é',"'unclosed",'/* open']:
        candidates.setdefault(s,'heldout')
    for n,(s,split) in enumerate(sorted(candidates.items())):
        raw=[];tokens=[];selected='';lexstatus=0;parse=1
        try:
            raw,tokens=lex(lang,s)
            # A Prolog full stop terminates only before layout, % or EOF.
            if lang=='prolog' and any(k==12 and end<len(s) and s[end] not in ' \t\r\n%' for k,start,end in tokens):raise Invalid()
        except Invalid:lexstatus=1
        if not lexstatus:
            try:selected=Reader(lang,tokens).document();parse=0
            except (Invalid,RecursionError):pass
        ident=f'{lang}-{n:03}'
        canon=lambda ts:''.join(f'{k}@{a}:{b};' for k,a,b in ts)
        row=[lang,ident,split,s.encode().hex().upper(),str(lexstatus),str(parse),canon(raw) if not lexstatus else '',canon(tokens) if not lexstatus else '',selected]
        rows.append('|'.join(row)+'\n');records.append(dict(zip(['language','id','split','hex','lexical','parse','raw','tokens','selected'],row)))
path=R/'cases/fixtures/consumers-reference.txt'
if path.exists() and path.read_text()!=''.join(rows):raise SystemExit('Frozen corpus differs; retain old version and explain revision first')
write_fixture(path,''.join(rows))
for lang in SEEDS:
    write_fixture((R/f'cases/fixtures/consumers-{lang}.txt'),''.join(row for row in rows if row.startswith(lang+'|')))
print('Independent consumer fragment fixtures:',len(rows))
