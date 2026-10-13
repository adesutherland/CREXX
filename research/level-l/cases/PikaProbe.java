/* Independent executable probe of the pinned, unmodified Pika reference.
 * Counts are algorithm observations; no JVM timing/product verdict here.
 */
import pikaparser.grammar.Grammar;
import pikaparser.grammar.MetaGrammar;
import pikaparser.clause.Clause;
import pikaparser.clause.terminal.Terminal;
import pikaparser.clause.nonterminal.Seq;
import pikaparser.clause.nonterminal.First;
import pikaparser.memotable.MemoKey;
import pikaparser.memotable.MemoTable;

public class PikaProbe {
    static int failures;
    static void check(boolean value, String label) {
        System.out.println((value ? "PASS: " : "FAIL: ") + label);
        if (!value) failures++;
    }
    static int length(Grammar grammar, String rule, String input) {
        var memo = grammar.parse(input);
        var match = memo.getNavigableMatches(grammar.getRule(rule).labeledClause.clause).get(0);
        return match == null ? -1 : match.len;
    }
    // A separate, anchored explanation pass for positive PEG clauses.
    // This deliberately does not claim minimum-edit repair or predicate support.
    static class Explanation {
        int position=-1;
        final java.util.Set<String> expected=new java.util.TreeSet<>();
        final java.util.Map<Clause,java.util.Set<Integer>> visited=new java.util.IdentityHashMap<>();
        final MemoTable memo;
        Explanation(MemoTable memo) { this.memo=memo; }
        void walk(Clause clause,int at) {
            if(!visited.computeIfAbsent(clause,c->new java.util.HashSet<>()).add(at)) return;
            var match=memo.lookUpBestMatch(new MemoKey(clause,at));
            if(clause instanceof Terminal) {
                if(match!=null) return;
                if(at>position) { position=at; expected.clear(); }
                if(at==position) expected.add(clause.toString());
            } else if(clause instanceof Seq) {
                int cursor=at;
                for(var child:clause.labeledSubClauses) {
                    walk(child.clause,cursor);
                    var sub=memo.lookUpBestMatch(new MemoKey(child.clause,cursor));
                    if(sub==null) break;
                    cursor+=sub.len;
                }
            } else if(clause instanceof First) {
                for(var child:clause.labeledSubClauses) walk(child.clause,at);
            } else {
                // For repetition only its mandatory first item is explanatory.
                if(clause.labeledSubClauses.length>0) walk(clause.labeledSubClauses[0].clause,at);
            }
        }
    }
    public static void main(String[] args) {
        var expressions = MetaGrammar.parse("E <- E '+' T / T; T <- T '*' A / A; A <- [0-9]+ / '(' E ')';");
        for (String input : new String[]{"1+2*3", "(1+2)*3+4+5"}) {
            check(length(expressions,"E",input)==input.length(), "reference expression " + input);
        }
        check(length(expressions,"E","1+*")<3,"reference malformed expression rejected");
        var indirect = MetaGrammar.parse("A <- B '+' [0-9] / [0-9]; B <- A;");
        check(length(indirect,"A","1+2+3")==5,"reference productive indirect recursion");
        var choice = MetaGrammar.parse("S <- ('a' / \"ab\") 'c';");
        check(length(choice,"S","abc")<3,"PEG priority differs from maximal-munch AB token");

        // These examples are acknowledged in upstream TestArithmetic.java.
        var mixed = MetaGrammar.parse("Program <- Statement+; Statement <- [a-z]+ '=' Sum ';'; Sum <- Sum '+' Term / Sum '-' Term / Term; Term <- [0-9]+ / [a-z]+;");
        int plusMinus=length(mixed,"Program","x=a+b-c;");
        int minusPlus=length(mixed,"Program","x=a-b+c;");
        System.out.println("reference_mixed_lr plus_then_minus="+plusMinus+" minus_then_plus="+minusPlus+" wanted=8");
        check(plusMinus!=8 && minusPlus==8,"upstream mixed-left-recursion limitation reproduced");
        var factored=MetaGrammar.parse("Program <- Statement+; Statement <- [a-z]+ '=' Sum ';'; Sum <- Sum ('+' / '-') Term / Term; Term <- [0-9]+ / [a-z]+;");
        check(length(factored,"Program","x=a+b-c;")==8 && length(factored,"Program","x=a-b+c;")==8,"factoring the operator choice repairs the mixed fixture");
        var cycle = MetaGrammar.parse("A <- B / 'x'; B <- A 'y' / A 'x';");
        int cycleLength=length(cycle,"A","xxyx");
        System.out.println("reference_cycle length="+cycleLength+" wanted=4");
        check(cycleLength!=4,"upstream cyclic grammar limitation reproduced");

        var statements=MetaGrammar.parse("Program <- Statement*; Statement <- [a-z]+ '=' E ';'; E <- E '+' T / T; T <- [0-9]+ / '(' E ')';");
        String broken="a=1+;b=2;c=(3+4;d=5;";
        var memo=statements.parse(broken);
        var islands=memo.getNavigableMatches(statements.getRule("Statement").labeledClause.clause);
        var gaps=memo.getSyntaxErrors("Statement");
        System.out.println("reference_islands="+islands.keySet()+" reference_gaps="+gaps);
        check(islands.containsKey(5)&&islands.containsKey(16)&&gaps.size()==2,"reference recovers statement islands and two gaps");
        for(int start:new int[]{0,9}) {
            var explanation=new Explanation(memo);
            explanation.walk(statements.getRule("Statement").labeledClause.clause,start);
            System.out.println("reference_diagnostic column="+(explanation.position+1)+" unexpected="+broken.charAt(explanation.position)+" expected="+explanation.expected);
            check(explanation.position==(start==0?4:15),"anchored Pika explanation points at the actual missing operand/parenthesis");
        }
        // Overly permissive coverage conceals errors: numbers cover broken operands.
        var coverage=MetaGrammar.parse("Program <- 'x'; Number <- [0-9]+;");
        System.out.println("reference_permissive_coverage="+coverage.parse("123").getSyntaxErrors("Program","Number"));
        check(coverage.parse("123").getSyntaxErrors("Program","Number").isEmpty(),"coverage can conceal top-level recognition failure");

        var chain=MetaGrammar.parse("A <- A 'a' / 'a';");
        for(int n:new int[]{16,32,64,128,256}) {
            var result=chain.parse("a".repeat(n));
            long slots=result.getAllNavigableMatches().values().stream().mapToLong(m->m.size()).sum();
            var seen=new java.util.IdentityHashMap<pikaparser.memotable.Match,Boolean>();
            var pending=new java.util.ArrayDeque<pikaparser.memotable.Match>();
            result.getAllNavigableMatches().values().forEach(m->pending.addAll(m.values()));
            while(!pending.isEmpty()) {
                var match=pending.removeFirst();
                if(seen.put(match,true)!=null) continue;
                for(var child:match.getSubClauseMatches()) pending.addLast(child.getValue());
            }
            System.out.println("reference_chain n="+n+" created="+result.numMatchObjectsCreated.get()+" memoized_updates="+result.numMatchObjectsMemoized.get()+" memo_slots="+slots+" reachable_match_objects="+seen.size());
        }
        System.out.println("pika_probe_failures="+failures);
        if(failures!=0) System.exit(1);
    }
}
