
// Run in the same library as an unmodified, pinned c14 source copy.
String describe(lib.MatchResult m) {
  final tag = m is lib.SyntaxError ? 'error' : m.clause is lib.Ref ? 'Ref:${(m.clause as lib.Ref).ruleName}' : m.clause.runtimeType.toString();
  return '($tag,${m.pos},${m.len}${m.subClauseMatches.map(describe).join()})';
}
String totals(_Reading r) => 'end=${r.end},del=${r.deleted},miss=${r.missing},ev=${r.evidence},first=${r.firstDoubt},last=${r.lastDoubt},pen=${r.penalties},charge=${Squirrel._charge(r,0)}';
void main() {
  final rules = <String,lib.Clause>{
    'S': lib.Seq([lib.Ref('X'),lib.Char('f')]),
    'X': lib.First([lib.Char('e'),lib.Seq([
      lib.Char('z'),lib.Char('z'),lib.Char('z'),lib.Char('z'),
      lib.Char('a'),lib.Char('b'),lib.AnyChar(),lib.AnyChar(),lib.AnyChar()
    ])])
  };
  final engine = Squirrel(rules:rules,topRuleName:'S');
  final result = engine.recover('abcdef');
  print('normal_cost=${engine.lastCost} tree=${describe(result)}');
  engine._budget=4;
  final x=RuleRef(lib.Ref('X'),engine.rules['X']!);
  final f=Terminal(lib.Char('f'),'f');
  final a=_Reading.deleting(0,4).then(x.cleanReading(engine,4)!);
  final choices=x.readings(engine,0);
  final b=choices.firstWhere((r)=>r.end==5 && r.missing==4 && r.evidence==2);
  final tail=f.cleanReading(engine,5)!;
  print('reachable_A ${totals(a)}');
  print('reachable_B ${totals(b)}');
  print('before=${Squirrel._compare(a,b,0)} after=${Squirrel._compare(a.then(tail),b.then(tail),0)}');
  final expectedActual=engine._buildTree(a.then(tail).labeled(rules['S']!,0));
  if (describe(result)!=describe(expectedActual)) throw StateError('normal result is not A');
  print('better_complete_tree=${describe(engine._buildTree(b.then(tail).labeled(rules['S']!,0)))}');
  if (!(Squirrel._compare(a,b,0)<0 && Squirrel._compare(a.then(tail),b.then(tail),0)>0)) throw StateError('no reachable reversal');
  print('c14_reachable_pruning_counterexample=confirmed');
}
