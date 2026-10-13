// Independent c14 reference, full raw trees and n-ary source grammar fixtures.
import 'dart:io';
import 'package:squirrel_parser/squirrel_parser.dart';
import '../experiments/recovery/c14.dart' as c14;
String hex(String s) => s.isEmpty ? '-' : s.codeUnits.map((c) => c.toRadixString(16).padLeft(2,'0')).join().toUpperCase();
String canonical(MatchResult m) {
  final kind = m is SyntaxError ? 'error' : m.clause is Ref ? 'Ref:${(m.clause as Ref).ruleName}' : '${m.clause.runtimeType}';
  return '($kind,${m.pos},${m.len}${m.subClauseMatches.map(canonical).join()})';
}
List<String> words(String alphabet,int max) {
  var level=['']; final out=<String>[''];
  for(var n=1;n<=max;n++) { level=[for(final p in level) for(final c in alphabet.split('')) '$p$c']; out.addAll(level); }
  return out;
}
void main(List<String> args) {
  final fixtures=<String>[], expected=<String>[];
  void suite(String name,Map<String,Clause> rules,List<String> inputs) {
    fixtures.add('GRAMMAR $name');
    final records=<List<Object>>[], edges=<int,List<int>>{}, refs=<String,int>{};
    int add(int op,{int a=0,int b=0,String text='',String label='-'}) {
      final id=records.length+1;records.add([id,op,a,b,hex(text),label]);return id;
    }
    for(final name in rules.keys) {refs[name]=add(5,label:name);}
    int convert(Clause c) {
      if(c is Ref) return refs[c.ruleName]!;
      final int id;
      if(c is Seq) {id=add(2);edges[id]=c.subClauses.map(convert).toList();}
      else if(c is First) {id=add(3);edges[id]=c.subClauses.map(convert).toList();}
      else if(c is Repetition) {id=add(c.requireOne?14:7);edges[id]=[convert(c.subClause)];}
      else if(c is Optional) {id=add(15);edges[id]=[convert(c.subClause)];}
      else if(c is FollowedBy) {id=add(9);edges[id]=[convert(c.subClause)];}
      else if(c is NotFollowedBy) {id=add(8);edges[id]=[convert(c.subClause)];}
      else if(c is Char) {id=add(1,a:c.char.codeUnitAt(0));}
      else if(c is Str) {id=add(13,text:c.text);}
      else if(c is CharSet) {id=add(16,a:c.inverted?1:0,text:String.fromCharCodes([for(final (lo,hi) in c.ranges) for(var n=lo;n<=hi;n++) n]));}
      else if(c is AnyChar) {id=add(12);}
      else if(c is Nothing) {id=add(6);}
      else {throw StateError('unsupported ${c.runtimeType}');}
      return id;
    }
    for(final entry in rules.entries) { edges[refs[entry.key]!]=[convert(entry.value)]; }
    fixtures.addAll(records.map((r)=>'CLAUSE ${r.join(' ')}'));
    fixtures.addAll(edges.entries.map((e)=>'EDGES ${e.key} ${e.value.join(' ')}'));
    final engine=c14.Squirrel(rules:rules,topRuleName:'S');
    for(final input in inputs) {
      final result=engine.recover(input);
      fixtures.add('CASE ${hex(input)}');
      // A zero cost can be the fallback on empty impossible input. Use the
      // independent packaged plain result only for clean-vs-recovered status.
      final plain=Parser(rules:rules,topRuleName:'S',input:input).matchRule('S',0);
      final status=!plain.isMismatch && plain.len==input.length ? 0 : 1;
      expected.add('C14|$name|${hex(input)}|$status|${engine.lastCost}|${canonical(result)}');
    }
  }
  suite('array',{'S':Seq([Char('['),Ref('N'),ZeroOrMore(Seq([Char(','),Ref('N')])),Char(']')]),'N':OneOrMore(CharSet.range('0','9'))},
    ['[1,2]','[1 2]','[1,,2]','[1,2','[x,2]','[7]','','[',']','[12,34]','[1,2]garbage','[1,x,3]']);
  suite('direct',{'S':First([Seq([Ref('S'),Char('b')]),Char('a')])},words('abx',3));
  suite('indirect',{'S':First([Ref('B'),Char('a')]),'B':First([Seq([Ref('S'),Char('b')]),Char('a')])},['','a','abbb','abx','xab','bb']);
  suite('literal',{'S':Seq([Str('true'),Char(';')])},['true;','tru;','txue;','trrue;','true','tr','', 'xtrue;', 'false;']);
  suite('prefix',{'S':Seq([First([Char('a'),Str('ab')]),Char('c')])},['ac','abc','abbc','a','bc','']);
  suite('optional',{'S':Seq([Optional(Char('a')),OneOrMore(CharSet.range('0','9'))])},['12','a12','aa12','a','', 'x12','12x']);
  suite('space',{'S':Seq([ZeroOrMore(Char(' ')),Char('a'),ZeroOrMore(Char(' ')),Char('b')])},[' ab','a b',' a x b ',' a',' ', '']);
  suite('predicates',{'S':Seq([NotFollowedBy(Char('b')),FollowedBy(Char('a')),AnyChar(),Char(';')])},['a;','b;','ax','a','x','']);
  suite('inverted',{'S':Seq([Char('"'),ZeroOrMore(CharSet([(34,34)],inverted:true)),Char('"')])},['"abc"','"a','abc','""','']);
  suite('nullable-star',{'S':ZeroOrMore(Nothing())},['','x']);
  suite('nullable-plus',{'S':OneOrMore(Nothing())},['','x']);
  suite('bytes',{'S':Seq([Char('\x00'),CharSet([(128,128),(255,255)])])},['\x00\x80','\x00\xff','\x00','\xff','']);
  suite('pruning',{'S':Seq([Ref('X'),Char('f')]),'X':First([Char('e'),Seq([Char('z'),Char('z'),Char('z'),Char('z'),Char('a'),Char('b'),AnyChar(),AnyChar(),AnyChar()])])},['abcdef']);
  suite('mixed',{'S':First([Seq([Ref('S'),Char('+'),Ref('T')]),Seq([Ref('S'),Char('-'),Ref('T')]),Ref('T')]),'T':CharSet.range('a','z')},words('a+-',3)..addAll(['a+a-a','a-a+a','a++a','a--a']));
  suite('right',{'S':First([Seq([Char('a'),Ref('S')]),Char('b')])},words('abx',3));
  suite('empty-slots',{'S':Seq([Nothing(),Char('a'),Optional(Nothing()),Char('b'),Nothing()])},['ab','a','b','','xab','axb']);
  suite('overlap',{'S':First([Seq([FollowedBy(Ref('E')),Char('x'),Ref('S')]),Char('x')]),'E':First([Seq([Ref('E'),Char('x')]),Char('x')])},['xxxx','xxxxxx','xxqxx']);
  // D01 held-out deterministic mutation partition. No tuning to these trees.
  final damage=<String>{}; const base='[12,34,56]';
  for(var i=0;i<base.length;i++) {
    damage.add(base.substring(0,i)+base.substring(i+1));
    damage.add(base.substring(0,i)+'?'+base.substring(i+1));
    damage.add(base.substring(0,i)+','+base.substring(i));
    damage.add(base.substring(0,i));
  }
  suite('heldout-array',{'S':Seq([Char('['),Ref('N'),ZeroOrMore(Seq([Char(','),Ref('N')])),Char(']')]),'N':OneOrMore(CharSet.range('0','9'))},damage.toList());
  File(args[0]).writeAsStringSync('${fixtures.join('\n')}\n');
  File(args[1]).writeAsStringSync('${expected.join('\n')}\n');
  print('c14_reference_cases=${expected.length}');
}
