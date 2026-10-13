/* Pinned upstream normal-form export and independent operational oracle.
 * Reflection reads the raw match DAG; getSubClauseMatches flattens plus and
 * would hide differences in the native port's storage/selection semantics.
 */
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.lang.reflect.Field;
import java.util.*;
import pikaparser.grammar.*;
import pikaparser.clause.*;
import pikaparser.clause.terminal.*;
import pikaparser.clause.nonterminal.*;
import pikaparser.memotable.*;

public class PikaCoreProbe {
    static PrintWriter fixtures, expected;
    static Field children, choice;
    static int count;
    static String hex(String s) {
        if(s.isEmpty()) return "-";
        StringBuilder out=new StringBuilder();
        for(char c:s.toCharArray()) {
            if(c>255) throw new IllegalArgumentException("byte fixture outside profile");
            out.append(String.format("%02x",(int)c));
        }
        return out.toString();
    }
    static int op(Clause c) {
        if(c instanceof CharSet) return 1;
        if(c instanceof Seq) return 2;
        if(c instanceof First) return 3;
        if(c instanceof Nothing) return 6;
        if(c instanceof NotFollowedBy) return 8;
        if(c instanceof FollowedBy) return 9;
        if(c instanceof CharSeq) return 13;
        if(c instanceof OneOrMore) return 14;
        if(c instanceof Start) return 17;
        throw new IllegalArgumentException(c.getClass().getName());
    }
    static String raw(Match m) throws Exception {
        if(m==null) return "-";
        StringBuilder s=new StringBuilder("("+(m.memoKey.clause.clauseIdx+1)+","+m.memoKey.startPos+","+m.len+","+choice.getInt(m));
        for(Match child:(Match[])children.get(m)) s.append(raw(child));
        return s.append(')').toString();
    }
    static void run(String name, Grammar g, String rule, List<String> inputs) throws Exception {
        Clause root=g.getRule(rule).labeledClause.clause;
        fixtures.println("GRAMMAR "+name+" "+g.allClauses.size()+" "+(root.clauseIdx+1));
        for(Clause c:g.allClauses) {
            String literal=""; int flags=0;
            if(c instanceof CharSeq) { literal=((CharSeq)c).str; flags=((CharSeq)c).ignoreCase?1:0; }
            if(c instanceof CharSet) {
                StringBuilder members=new StringBuilder();
                for(char symbol=0;symbol<256;symbol++) {
                    String s=""+symbol;
                    if(c.match(new MemoTable(g,s),new MemoKey(c,0),s)!=null) members.append(symbol);
                }
                literal=members.toString();
            }
            fixtures.print("CLAUSE "+(c.clauseIdx+1)+" "+op(c)+" "+flags+" "+hex(literal)+" "+(c.canMatchZeroChars?1:0));
            for(var child:c.labeledSubClauses) fixtures.print(" "+(child.clause.clauseIdx+1));
            fixtures.println();
        }
        for(String input:inputs) {
            fixtures.println("CASE "+hex(input));
            MemoTable memo=g.parse(input);
            Match selected=memo.getNavigableMatches(root).get(0);
            long entries=memo.getAllNavigableMatches().values().stream().mapToLong(Map::size).sum();
            expected.println("PIKA|"+name+"|"+hex(input)+"|"+(selected==null?-1:selected.len)+"|"+memo.numMatchObjectsCreated.get()+"|"+memo.numMatchObjectsMemoized.get()+"|"+entries+"|"+raw(selected));
            count++;
        }
    }
    static List<String> words(String alphabet,int depth) {
        List<String> out=new ArrayList<>(); out.add("");
        List<String> frontier=List.of("");
        for(int i=1;i<=depth;i++) {
            List<String> next=new ArrayList<>();
            for(String prefix:frontier) for(char c:alphabet.toCharArray()) next.add(prefix+c);
            out.addAll(next); frontier=next;
        }
        return out;
    }
    public static void main(String[] args) throws Exception {
        children=Match.class.getDeclaredField("subClauseMatches"); children.setAccessible(true);
        choice=Match.class.getDeclaredField("firstMatchingSubClauseIdx"); choice.setAccessible(true);
        fixtures=new PrintWriter(args[0],StandardCharsets.UTF_8);
        expected=new PrintWriter(args[1],StandardCharsets.UTF_8);
        run("direct",MetaGrammar.parse("A <- A 'a' / 'a';"),"A",words("ab",5));
        run("mixed",MetaGrammar.parse("E <- E '+' T / E '-' T / T; T <- [ab];"),"E",words("ab+-",4));
        run("indirect",MetaGrammar.parse("A <- B / 'x'; B <- A 'y' / A 'x';"),"A",words("xy",5));
        run("clauses",MetaGrammar.parse("S <- ^ &\"ab\" (!'x' [a-z])+ ('!' / ());"),"S",List.of("","a","ab","abc!","abx","xab","ab!x"));
        run("prefix",MetaGrammar.parse("S <- ('a' / \"ab\") 'c';"),"S",List.of("ac","abc","c","ab",""));
        run("stars",MetaGrammar.parse("S <- [a-z]* '!';"),"S",List.of("!","a!","abc!","abc","","\0!"));
        run("start",MetaGrammar.parse("S <- 'x' ^ 'a';"),"S",List.of("xa","a","xxa"));
        run("empty",new Grammar(List.of(new Rule("S",new Nothing()))),"S",List.of("","x"));
        run("empty_literal",new Grammar(List.of(new Rule("S",new CharSeq("",false)))),"S",List.of("","x"));
        run("fold",new Grammar(List.of(new Rule("S",new CharSeq("ab",true)))),"S",List.of("ab","AB","aB","xb",""));
        run("bytes",new Grammar(List.of(new Rule("S",new OneOrMore(new CharSet('\0','\u0080','\u00ff'))))),"S",List.of("\0","\u0080","\u00ff","\0\u0080\u00ff","a",""));
        run("precedence",MetaGrammar.parse("E[1,L] <- E '+' E; E[2,L] <- E '*' E; E[3] <- [0-9];"),"E[1]",List.of("1+2*3","1*2+3","1+2+3","1**2",""));
        fixtures.close(); expected.close();
        System.out.println("pika_core_reference_cases="+count);
    }
}
