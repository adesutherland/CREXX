// Independent pinned Squirrel core and c14 observations. cREXX owns the benchmarks.
import 'dart:collection';
import 'dart:convert';
import 'dart:io';
import 'package:squirrel_parser/squirrel_parser.dart';
import 'package:squirrel_parser/src/parser/parser_stats.dart';
import '../experiments/recovery/c14.dart' as c14;

final expectedRows = <String>[];
String canonical(MatchResult m) {
  final kind = m.clause is Ref ? 'Ref:${(m.clause as Ref).ruleName}' : '${m.clause.runtimeType}';
  return '($kind,${m.isMismatch ? 0 : 1},${m.pos},${m.len}${m.subClauseMatches.map(canonical).join()})';
}

Map<String, Object?> shape(MatchResult m) => {
  'kind': m.clause is Ref ? 'Ref:${(m.clause as Ref).ruleName}' : '${m.clause.runtimeType}',
  'ok': !m.isMismatch, 'pos': m.pos, 'len': m.len,
  'children': [for (final c in m.subClauseMatches) shape(c)],
};

void core(String name, Map<String, Clause> rules, String input, {bool tree = true}) {
  parserStats = ParserStats();
  final p = Parser(rules: rules, topRuleName: 'S', input: input);
  final raw = p.matchRule('S', 0);
  final nodes = HashSet<MatchResult>.identity();
  void visit(MatchResult m) {
    if (nodes.add(m)) { for (final c in m.subClauseMatches) { visit(c); } }
  }
  var memos = 0;
  for (final body in rules.values.toSet()) {
    for (var pos = 0; pos <= input.length; pos++) {
      final result = p.getMemoEntry(body, pos)?.result;
      if (result != null) { memos++; visit(result); }
    }
  }
  print(jsonEncode({'case': name, 'input': input, 'accepted': !raw.isMismatch && raw.len == input.length,
    'matched': !raw.isMismatch, 'len': raw.len, 'memoEvaluations': parserStats!.totalWork,
    'hits': parserStats!.cacheHits, 'expansions': parserStats!.lrExpansions,
    'memos': memos, 'reachableResults': nodes.length,
    if (tree) 'tree': shape(raw)}));
  final hex = input.codeUnits.map((c) => c.toRadixString(16).padLeft(2, '0')).join().toUpperCase();
  expectedRows.add('CORE|$name|$hex|${!raw.isMismatch && raw.len == input.length ? 1 : 0}|${raw.isMismatch ? 0 : 1}|${raw.len}|${parserStats!.totalWork}|${parserStats!.cacheHits}|${parserStats!.lrExpansions}|${tree ? canonical(raw) : ''}');
}

void main(List<String> args) {
  final a = Char('a'), b = Char('b');
  final direct = <String, Clause>{'S': First([Seq([Ref('S'), b]), a])};
  for (final text in ['', 'a', 'abbb', 'abx']) { core('direct', direct, text); }
  void enumerate(String name, Map<String, Clause> grammar, String alphabet, String prefix, int remaining) {
    if (remaining == 0) { core(name, grammar, prefix); return; }
    for (final c in alphabet.split('')) { enumerate(name, grammar, alphabet, '$prefix$c', remaining - 1); }
  }
  for (var n = 0; n <= 4; n++) { enumerate('generated-direct', direct, 'abx', '', n); }
  final mixed = <String, Clause>{'S': First([First([Seq([Seq([Ref('S'), Char('+')]), Ref('T')]),
    Seq([Seq([Ref('S'), Char('-')]), Ref('T')])]), Ref('T')]), 'T': CharSet.range('a', 'z')};
  for (final text in ['a+b-c', 'a-b+c', 'a+', 'a++b']) { core('mixed', mixed, text); }
  for (var n = 0; n <= 4; n++) { enumerate('generated-mixed', mixed, 'ab+-', '', n); }
  core('indirect', {'S': First([Ref('B'), a]), 'B': First([Seq([Ref('S'), b]), a])}, 'abbb');
  core('prefix', {'S': Seq([First([a, Str('ab')]), Char('c')])}, 'abc');
  core('literal-frontier', {'S': Str('function')}, 'fun?');
  core('predicate-frontier', {'S': Seq([a, FollowedBy(Seq([b, Char('c')]))])}, 'abx');
  core('nullable-star', {'S': ZeroOrMore(Nothing())}, '');
  core('nullable-plus', {'S': OneOrMore(Nothing())}, '');
  core('charset', {'S': Seq([Optional(Char('a')), OneOrMore(CharSet([(48,57),(65,67)]))])}, 'a19BC');
  core('inverted-set', {'S': ZeroOrMore(CharSet([(97,99)], inverted: true))}, 'XYz');
  core('negative', {'S': Seq([NotFollowedBy(Char('b')), AnyChar()])}, 'a');
  core('negative', {'S': Seq([NotFollowedBy(Char('b')), AnyChar()])}, 'b');
  core('nul', {'S': Char('\x00')}, '\x00');
  final shared = First([Seq([Ref('B'), b]), a]);
  core('shared-body', {'S': shared, 'B': shared}, 'abbb');
  for (final n in [4,8,16,32]) {
    core('overlap', {'S': First([Seq([Seq([FollowedBy(Ref('E')), Char('x')]), Ref('S')]), Char('x')]),
      'E': First([Seq([Ref('E'), Char('x')]), Char('x')])}, 'x' * n, tree: false);
  }
  final format = <String, Clause>{'S': Seq([Char('['), Ref('N'), ZeroOrMore(Seq([Char(','), Ref('N')])), Char(']')]),
    'N': OneOrMore(CharSet.range('0', '9'))};
  final recovery = c14.Squirrel(rules: format, topRuleName: 'S');
  for (final text in ['[1,2]', '[1 2]', '[1,,2]', '[1,2', '[x,2]', '[7]']) {
    final result = recovery.recover(text);
    print(jsonEncode({'case':'c14-array', 'input':text, 'cost':recovery.lastCost, 'tree':shape(result)}));
  }
  if (args.isNotEmpty) File(args[0]).writeAsStringSync('${expectedRows.join('\n')}\n');
}
