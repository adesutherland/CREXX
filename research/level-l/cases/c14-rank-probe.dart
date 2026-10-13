
// Appended to an untracked copy of pinned c14.dart so its actual private
// reading algebra and comparator are tested without modifying upstream.
void main() {
  final a = _Reading(5, 4, 0, 1, 0, lastDoubt: 3);
  final b = _Reading(5, 0, 4, 2, 0, lastDoubt: 0);
  final step = _Reading(6, 0, 0, 1, _preferred);
  final before = Squirrel._compare(a, b, 0);
  final after = Squirrel._compare(a.then(step), b.then(step), 0);
  print('c14_equal_end=${a.end == b.end} equal_spent=${a.spent == b.spent} before=$before after=$after');
  if (!(before < 0 && after > 0)) throw StateError('expected ranking reversal');
  print('c14_right_congruence_counterexample=confirmed');
  // This is an algebraic counterexample. It does not itself prove that a
  // particular grammar reaches both readings before the same pruning step.
}
