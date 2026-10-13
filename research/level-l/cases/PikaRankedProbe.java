/* Independent upstream evaluator for NATIVE-lowered ranked graphs.
 * No reference normalization is supplied to the native code in this check.
 * The reference's operational scheduler is rank-sensitive, so preserve the
 * explicit ranks rather than comparing unspecified HashSet iteration orders.
 */
import java.io.*;
import java.nio.file.*;
import java.lang.reflect.Field;
import java.util.*;
import pikaparser.grammar.*;
import pikaparser.clause.*;
import pikaparser.clause.terminal.*;
import pikaparser.clause.nonterminal.*;
import pikaparser.memotable.*;

public class PikaRankedProbe {
    static Field children, choice;
    static String decode(String h) {
        if (h.equals("-")) return "";
        StringBuilder out = new StringBuilder();
        for(int i=0;i<h.length();i+=2) out.append((char)Integer.parseInt(h.substring(i,i+2),16));
        return out.toString();
    }
    static String raw(Match m) throws Exception {
        if(m==null) return "-";
        StringBuilder out = new StringBuilder("("+(m.memoKey.clause.clauseIdx+1)+","+m.memoKey.startPos+","+m.len+","+choice.getInt(m));
        for(Match c:(Match[])children.get(m)) out.append(raw(c));
        return out.append(')').toString();
    }
    static Grammar build(List<String[]> rows) {
        Grammar g=new Grammar(List.of(new Rule("stub",new Nothing())));
        g.allClauses.clear();
        Clause placeholder=new CharSet('x');
        for(String[] r:rows) {
            int op=Integer.parseInt(r[2]); String s=decode(r[4]);
            Clause[] subs=new Clause[r.length-6]; Arrays.fill(subs,placeholder);
            Clause c=switch(op) {
                case 1 -> new CharSet(s.toCharArray());
                case 2 -> new Seq(subs);
                case 3 -> new First(subs);
                case 6 -> new Nothing();
                case 8 -> new NotFollowedBy(placeholder);
                case 9 -> new FollowedBy(placeholder);
                case 13 -> new CharSeq(s,Integer.parseInt(r[3])==1);
                case 14 -> new OneOrMore(placeholder);
                default -> throw new IllegalArgumentException("clause "+op);
            };
            c.clauseIdx=g.allClauses.size();
            c.toStringCached="rank"+(c.clauseIdx+1);
            g.allClauses.add(c);
        }
        for(int i=0;i<rows.size();i++) {
            String[] r=rows.get(i); Clause c=g.allClauses.get(i);
            for(int j=6;j<r.length;j++) c.labeledSubClauses[j-6].clause=g.allClauses.get(Integer.parseInt(r[j])-1);
        }
        boolean changed=true;
        while(changed) {
            changed=false;
            for(Clause c:g.allClauses) if(c.determineWhetherCanMatchZeroChars()) changed=true;
        }
        for(int i=0;i<rows.size();i++) {
            Clause c=g.allClauses.get(i);
            if(c.canMatchZeroChars!=(Integer.parseInt(rows.get(i)[5])==1)) throw new AssertionError("zero flag "+i);
            c.addAsSeedParentClause();
        }
        return g;
    }
    public static void main(String[] args) throws Exception {
        children=Match.class.getDeclaredField("subClauseMatches"); children.setAccessible(true);
        choice=Match.class.getDeclaredField("firstMatchingSubClauseIdx"); choice.setAccessible(true);
        List<String[]> rows=new ArrayList<>(); Grammar g=null; String name="",want=""; int root=0,cases=0,failures=0;
        for(String row:Files.readAllLines(Path.of(args[0]))) {
            if(row.startsWith("GRAMMAR ")) { String[] r=row.split(" "); name=r[1];root=Integer.parseInt(r[3]);rows.clear();g=null; }
            else if(row.startsWith("CLAUSE ")) rows.add(row.split(" "));
            else if(row.startsWith("CASE ")) {
                if(g==null) g=build(rows);
                String input=decode(row.substring(5)); MemoTable memo=g.parse(input);
                Match m=memo.getNavigableMatches(g.allClauses.get(root-1)).get(0);
                long entries=memo.getAllNavigableMatches().values().stream().mapToLong(Map::size).sum();
                want="OBSERVED|"+(m==null?-1:m.len)+"|"+memo.numMatchObjectsCreated.get()+"|"+memo.numMatchObjectsMemoized.get()+"|"+entries+"|"+raw(m);
            } else if(row.startsWith("OBSERVED|")) {
                cases++; if(!row.equals(want)) { failures++;System.out.println("DIFFERENCE "+name+" case "+cases+"\n"+want+"\n"+row); }
            }
        }
        System.out.println("pika_native_lowering_reference_cases="+cases);
        System.out.println("pika_native_lowering_reference_failures="+failures);
        if(failures!=0 || cases==0) System.exit(1);
    }
}
