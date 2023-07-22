/*
 * @(#)REgen       1.0 2018/10/31
 *
 * Copyright (c) 2018 Angelo Borsotti. All Rights Reserved.
 * This computer program is protected under Copyright.
 */

import java.io.*;
import java.util.*;
import java.util.regex.*;
import java.lang.management.*;

/**
 * The <code>REgen</code> class provides regular expression benchmarks generation.
 *
 * @author  Angelo Borsotti
 * @version 1.0   31 October 2018
 */


public class REgen {

    /** A constant representing infinite. */
    private static final int INFINITE = Integer.MAX_VALUE;

    /** The generation parameters. */

    public class Parameters {

        /** The number of leaves of the RE. */
        int size;

        /** The depth of the ast of the RE. */
        int[] depth;

        /** The iterator range of the RE. */
        int[] iterrange = {3,5};          // just a default;

        /** The balance factor of the AST, true = balanced. */
        boolean balanced;

        /** The star height of the RE. */
        int[] starHeight;

        /** The maximum arity of concatenations, 0 = any. */
        int concarity;

        /** The maximum arity of alternatives, 0 = any. */
        int altarity;

        /** The leaves, whether they are single random or sets. */
        int leaves;

        /** The terminal alphabet. */
        String alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

        /** The special symbols. */
        String special = "ε|(){}*?+.[]^$\\";

        /** The allowed nesting of operators. */
        boolean[][] adjacences;

        /** Whether capturing parentheses in REs are generated. */
        boolean capturing = true;

        /** Whether epsilon in REs are generated. */
        boolean epsilon;

        /** Whether texts are generated. */
        boolean texts = true;

        /** Whether weak equivalence is measured. */
        boolean weakEq;

        /** Whether overlapping of languages is measured. */
        boolean overlap;

        /** Whether generation progress is shown on the console. */
        boolean progress = true;
    }

    /** Whether the leaves are generated in sequence (testing only). */
    private static final int LEAVES_STEP = 0;

    /** Whether the leaves are generated as terminals. */
    public static final int LEAVES_TERM = 1;

    /** Whether the leaves are generated as sets of terminals. */
    public static final int LEAVES_SET = 2;

    /** The parameters. */
    public Parameters parameters = new Parameters();


    //---------------- Tracing ---------------------

    /** The trace stream. */

    private static class Trc {
        public static PrintStream out = System.err;
    }

    /** The trace flags. */
    int trc;

    /*
     * Internal constants for trace flags
     */

    /**
     * The following trace flags are used:
     * <p><blockquote><pre>
     *    a   ast building
     *    b   getsym
     *    c   random terminals and quantities
     *    d   building BS
     *    e   building AST
     *    f   building character classes
     *    g   generating REs and texts
     *    h   decomposition in factors
     *    k   random numbers
     *    l   generating texts
     *    m   match BS
     *    r   generation of REs
     *    s   generating collections details
     *    t   generating collections, tests
     *    r   generation of REs, details
     * </pre></blockquote><p>
     */

    static final int FL_A = 1 << ('a'-0x60);
    static final int FL_B = 1 << ('b'-0x60);
    static final int FL_C = 1 << ('c'-0x60);
    static final int FL_D = 1 << ('d'-0x60);
    static final int FL_E = 1 << ('e'-0x60);
    static final int FL_F = 1 << ('f'-0x60);
    static final int FL_G = 1 << ('g'-0x60);
    static final int FL_H = 1 << ('h'-0x60);
    static final int FL_I = 1 << ('i'-0x60);
    static final int FL_K = 1 << ('k'-0x60);
    static final int FL_L = 1 << ('l'-0x60);
    static final int FL_M = 1 << ('m'-0x60);
    static final int FL_N = 1 << ('n'-0x60);
    static final int FL_R = 1 << ('r'-0x60);
    static final int FL_S = 1 << ('s'-0x60);
    static final int FL_T = 1 << ('t'-0x60);
    static final int FL_U = 1 << ('u'-0x60);

    /**
     * Set the trace flags which are specified in the argument.
     * The string must contain only lowercase letters.
     * Upon return <code>trc</code> is the powerset of flags augmented
     * with the ones specified.
     *
     * @param      s string of flags
     */

    public void settrc(String s){
        this.trc = 0;
        for (int i = 0; i < s.length(); i++){
            this.trc |= 1 << (s.charAt(i) - 0x60);
        }
    }
    /**
     * Construct a new <code>REgen</code>.
     */

    public REgen(){
    }

    // ---------- Symbols -----------------

    // Kinds of Symbols

    /* The symbols with kind >= S_CHAR denote characters; and may have a symset. */

    /** The kind of symbol for no symbol. */
    private static final int S_NONE = 0;

    /** The kind of symbol for a single character. */
    private static final int S_CHAR = 1;

    /** The kind of symbol for the character set denoted by dot. */
    private static final int S_DOT = 2;

    /** The kind of symbol for a character set. */
    private static final int S_SET = 3;

    /** The kind of symbol for a negated character set. */
    private static final int S_NSET = 4;

    /** The kind of symbol for a set of subsets of classes. */
    private static final int S_CSET = 5;

    /** The kind of symbol for class. */
    private static final int S_CLASS = 6;

    /** A symbol that represents input characters, tags and priorities. */

    private class Symbol {

        /** The kind of symbol. */
        int kind = S_NONE;

        /** The symbol if the kind is character. */
        char sym;

        /** The set of symbols, if the symbol is a set. */
        boolean[] symset;

        /** The set of classes, if the symbol is a set of classes. */
        boolean[] cset;

        /** The class, if the symbol is a class. */
        int csym;

        /**
         * Deliver a string representing this symbol.
         *
         * @return     String
         */

        public String toString(){
            String str = "";
            switch (this.kind){
            case S_NONE:
                break;
            case S_CHAR:
                str += this.sym;
                break;
            case S_DOT:
                str += '.';
                break;
            case S_SET:
            case S_NSET:
                str += '[';
                boolean pos = true;
                if (this.kind == S_NSET){
                    str += "^";
                    pos = false;
                }
                str += symSetToString(this.symset,pos);
                str += ']';
                break;
            case S_CSET:
                boolean first = true;
                for (int i = 0; i < this.cset.length; i++){
                    if (!this.cset[i]) continue;
                    if (!first) str += ", ";
                    first = false;
                    str += symClassTable[i].toString();
                }
                if (str.length() > 1){
                    str = '[' + str + ']';
                }
                break;
            case S_CLASS:
                str = symClassTable[this.csym].toString();
                if (str.length() > 1){
                    str = '[' + str + ']';
                }
                break;
            }
            return str;
        }

        /**
         * Tell if the specified symbol is equal to this one.
         *
         * @param      other symbol
         */

        public boolean equals(Symbol other){
            if (other == null) return false;
            return this.kind == other.kind && this.sym == other.sym &&
                Arrays.equals(this.symset,other.symset) &&
                Arrays.equals(this.cset,other.cset);
        }
    }

    /**
     * Deliver a new <code>Symbol</code> representing the specified character.
     *
     * @param      c character
     * @return     symbol
     */

    private Symbol newSymbolChar(char c){
        Symbol res = new Symbol();
        res.kind = S_CHAR;
        res.sym = c;
        return res;
    }

    /**
     * Deliver a new <code>Symbol</code> representing the specified character class.
     *
     * @param      c class
     * @return     symbol
     */

    private Symbol newSymbolClass(int c){
        Symbol res = new Symbol();
        res.kind = S_CLASS;
        res.csym = c;
        return res;
    }


    // ---------- AST's -----------------

    // kinds of AST nodes

    /** The kind of an AST node for a leaf. */
    private static final int A_LEA = 0;

    /** The kind of an AST node for an alternative. */
    private static final int A_ALT = 1;

    /** The kind of an AST node for a concatenation. */
    private static final int A_CON = 2;

    /** The kind of an AST node for a group. */
    private static final int A_GRO = 3;

    /** The kind of an AST node for the empty string. */
    private static final int A_EMP = 4;

    /** The kind of an AST node for the empty set. */
    private static final int A_NUL = 5;

    /** The strings representing the kinds of AST node. */
    private static final String[] astIcon = new String[]{
        "\u03a3",  // sigma
        "|",
        "\u2219",  // bullet
        "()",      // group
        "\u03b5",  // epsilon
        "\u03a6"}; // phi

    // kinds for groups

    /** The group kind of an AST node for the normal () group. */
    private static final int G_GRO = 0;

    /** The group kind of an AST node for the optional ()? group. */
    private static final int G_OPT = 1;

    /** The group kind of an AST node for the Kleene ()* group. */
    private static final int G_RE0 = 2;

    /** The group kind of an AST node for the positive ()+ group. */
    private static final int G_RE1 = 3;

    /** The group kind of an AST node for the bounded groups. */
    private static final int G_RE2 = 4;

    /** String representing group kinds. */
    private static final String[] groupKindStr = new String[]{
        "GRO","OPT","RE0","RE1","RE2"};

    /** Strings representing group symbols in RE strings. */
    private static final String[] groupSym = new String[]{
        "","?","*","+",""};

    /** String representing of capturing groups. */
    private static final String[] groupIconC = new String[]{
        "()","()?","()*","()+","(){}"};

    /** String representing of non-capturing groups. */
    private static final String[] groupIconN = new String[]{
        "()","?","*","+","{}"};

    /**
     * Deliver a string representing the group of the specified AST in compact form.
     *
     * @param      ast reference to the AST
     * @return     String
     */

    private String groupIcon(AstNode ast){
        return groupIconC[ast.groupKind];
    }

    /**
     * Deliver a string representing the kind of the specified AST in compact form.
     *
     * @param      ast reference to the AST
     * @return     String
     */

    private String astKindString(AstNode ast){
        String str = astIcon[ast.kind];
        if (ast.kind == A_GRO){
            str = groupIcon(ast);
        }
        if (ast.altnr != 0) str += " #|" + ast.altnr;
        return str;
    }

    /** An AST node, representing a subexpression. */

    private class AstNode implements Cloneable {

        /** The serial number. */
        int seq;

        /** The reference to the brother. */
        AstNode bro;

        /** The reference to the son. */
        AstNode son;

        /** The reference to the father. */
        AstNode fat;

        /** The kind of node. */
        int kind;

        /** The symbol if the kind is leaf. */
        Symbol sym;

        /** The kind of group (if group). */
        int groupKind;

        /** The lower bound of the bounded group. */
        int lowerbound;

        /** The upper bound of the bounded group (-1 = infinite, 0 = none). */
        int upperbound;

        /** The position: sequence of indexes of level numbering. */
        int[] pos = new int[0];

        /** The index in the RE as string. */
        int cursor;

        /** The number of alternative (when this node is an alternative). */
        int altnr;

        /** The level in the AST tree. */
        int lev;

        /** The maximum number of strings generated, -1 = infinite. */
        int maxStrNr;

        /** The minimum length of the strings generated. */
        int minStrLen;

        /** The maximum length of the strings generated, -1 = infinite. */
        int maxStrLen;

        /** The number of strings to generate. */
        int toGenNr;

        /** The maximum length of strings to generate. */
        int toGenLen;

        /** The number of repetitions to generate. */
        int toGenRep;

        /** The data to generate the AST. */
        GenData gendata;

        /** The length of the generated length. */
        int genLen;

        /** Whether the node generates the empty string. */
        boolean isNull;

        /** The init set. */
        Set<BSsymbol> ini = new HashSet<BSsymbol>();

        /** The finish set. */
        Set<BSsymbol> fin = new HashSet<BSsymbol>();

        /** The digrams set. */
        Set<BSsymbol> dig = new HashSet<BSsymbol>();

        /**
         * Deliver a string representing this node.
         *
         * @return     String
         */

        public String toString(){
            String str = "ast seq: " + this.seq;
            str += " pos:";
            str += posToString(this.pos);
            str += " ";
            if (this.altnr != 0) str += "|" + this.altnr + " ";
            str += " " + astKindString(this);
            if (this.kind == A_LEA){
                str += this.sym.toString();
            }
            str += " at: ";
            str += this.cursor;
            if (this.bro != null){
                str += " bro: ";
                str += this.bro.seq;
            }
            if (this.son != null){
                str += " son: ";
                str += this.son.seq;
            }
            if (this.fat != null){
                str += " fat: ";
                str += this.fat.seq;
            }
            if (this.kind == A_GRO){
                str += " {" + this.lowerbound + "," + this.upperbound + "}";
            }
            str += " nr strings: " + this.maxStrNr;
            str += " string lengths: " + this.minStrLen + "," + this.maxStrLen;
            str += " to gen n: " + this.toGenNr + ", l: " + this.toGenLen + ", r: " + this.toGenRep;
            return str;
        }

        /**
         * Deliver a string representing the tree of this node.
         *
         * @return     String
         */

        private String toTree(){
            StringBuilder sb = new StringBuilder();
            toTree(sb,this);
            return sb.toString();
        }

        /**
         * Deliver a string representing the tree of the specified node collecting
         * the string in the specified string buffer (internal method).
         *
         * @param      sb string buffer
         * @param      ast AST node
         */

        private void toTree(StringBuilder sb, AstNode ast){
            sb.append("<");
            switch (ast.kind){
            case A_LEA: sb.append(ast.sym.toString()); break;
            case A_ALT: sb.append("|"); break;
            case A_CON: sb.append("∙"); break;
            case A_GRO: sb.append(groupIcon(ast)); break;
            case A_EMP: sb.append("ε"); break;
            case A_NUL: sb.append("Φ"); break;
            }
            for (AstNode a = ast.son; a != null; a = a.bro){
                if (a != ast.son) sb.append(",");
                toTree(sb,a);
            }
            sb.append(">");
        }

        /**
         * Deliver a string representing the RE rooted in this node.
         *
         * @return     String
         */

        private String toRE(){
            StringBuilder sb = new StringBuilder();
            toRE(sb);
            return sb.toString();
        }

        /**
         * Append to the specified string builder a string representing the RE rooted
         * in this node.
         *
         * @param      sb string builder
         */

        private void toRE(StringBuilder sb){
            if (this.kind == A_LEA){          // leaf
                sb.append(this.sym.toString());
            } else if (this.kind == A_ALT){   // alt
                for (AstNode i = this.son; i != null; i = i.bro){
                    if (i != this.son) sb.append("|");
                    if (i.kind == A_ALT && parameters.capturing){
                        sb.append("(");
                    }
                    //if (i.kind == A_EMP || (i.kind == A_CON && i.son.kind == A_EMP)) {
                    //    // Avoid generating RE with empty subexpressions, like (|a) or (a||b).
                    //    sb.append("()");
                    //}
                    i.toRE(sb);
                    if (i.kind == A_ALT && parameters.capturing){
                        sb.append(")");
                    }
                }
            } else if (this.kind == A_CON){   // conc
                for (AstNode i = this.son; i != null; i = i.bro){
                    if (i.kind == A_ALT || i.kind == A_CON && parameters.capturing){
                        sb.append("(");
                    }
                    i.toRE(sb);
                    if (i.kind == A_ALT || i.kind == A_CON && parameters.capturing){
                        sb.append(")");
                    }
                }
            } else if (this.kind == A_EMP){       // empty
                if (parameters.epsilon){
                    sb.append("\u03b5");
                }
            } else if (this.kind == A_NUL){       // empty set
                sb.append("\u03a6");
            } else {                              // group
                boolean parens = true;
                if (!parameters.capturing){
                    if (this.son.kind == A_LEA){  // in a number of cases generate r*
                        int r = getRandom(10);
                        if (r < 5) parens = false;
                    }
                }
                if (this.son != null){
                    if (parens) sb.append("(");
                    this.son.toRE(sb);
                    if (parens) sb.append(")");
                }
                sb.append(groupSym[this.groupKind]);
                if (this.groupKind == G_RE2){
                    sb.append("{");
                    sb.append(this.lowerbound);
                    if (this.upperbound < 0){
                        sb.append(",");
                    } else if (this.upperbound > 0){
                        sb.append(",");
                        sb.append(this.upperbound);
                    }
                    sb.append("}");
                }
            }
        }

        /**
         * Deliver a clone of this object.
         *
         * @return     clone
         */

        public Object clone(){
            AstNode ast = null;
            try {
                ast = (AstNode)super.clone();
                ast.fat = ast;
                // these fields are used only by Berry-Sethi, and a deep clone is not needed
                ast.ini = new HashSet<BSsymbol>();
                ast.fin = new HashSet<BSsymbol>();
                ast.dig = new HashSet<BSsymbol>();
            } catch (CloneNotSupportedException e){
            }
            return ast;
        }
    }

    /**
     * Deliver the number of sons of the specified AST node.
     *
     * @param      ast AST node
     * @return     number of sons
     */

    private int nrSons(AstNode ast){
        int n = 0;
        for (AstNode a = ast.son; a != null; a = a.bro){
            n++;
        }
        return n;
    }

    /**
     * Deliver a string representing the specified character set using the bracketed
     * representation if it is a set.
     *
     * @param      symset set (array)
     * @param      pos <code>true</code> if the elements that are true in the array
     *             represent characters that are in the set, <code>false</code> if such
     *             elements represent character that are not in the set
     */

    private String symSetToString(boolean[] arr, boolean pos){
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < arr.length; i++){
            if (arr[i] != pos) continue;
            if (this.parameters.special.indexOf(this.parameters.alphabet.charAt(i)) >= 0){
                sb.append("\\");
            }
            sb.append(this.parameters.alphabet.charAt(i));
            int j = i;
            for (i++; i < arr.length; i++){
                if (arr[i] != pos) break;
            }
            if (i > j+2){                     // range (not of single character)
                sb.append("-");
                int n = i-1;
                if (this.parameters.special.indexOf(this.parameters.alphabet.charAt(n)) >= 0){
                    sb.append("\\");
                }
                sb.append(this.parameters.alphabet.charAt(n));
            } else if (i == j+2){     // range of single character
                i -= 2;
            } else {
                i--;
            }
        }
        return sb.toString();
    }

    /**
     * Deliver a string representing the specified position.
     *
     * @param      pos reference to the position
     * @return     string
     */

    private static String posToString(int[] pos){
        if (pos == null) return "null";
        if (pos.length == 0) return "\u2227";
        if (pos[0] == Integer.MAX_VALUE) return "$";
        String str = "";
        for (int i = 0; i < pos.length; i++){
            if (i > 0) str += '.';
            str += pos[i];
        }
        return str;
    }

    /** Sequence number of ast nodes. */
    private int astSeq;

    /** Map from integers to ast nodes. */
    private AstNode[] astMap;

    /**
     * Deliver a new AST node with the specified kind.
     *
     * @param      kind kind
     * @return     reference to the node
     */

    private AstNode newAstNode(int kind){
        AstNode node = new AstNode();
        node.kind = kind;
        node.cursor = this.startToken;
        node.seq = this.astSeq++;
        if ((FL_A & trc) != 0){Trc.out.printf("newAstNode %s %s\n",node.seq,astKindString(node));}
        return node;
    }

    /**
     * Trace the subtree rooted in the specified ast.
     *
     * @param      ast reference to the AST node
     */

    private void traceAst(AstNode ast){
        for (AstNode a = ast; a != null; a = a.bro){
            Trc.out.printf("node: %s\n",a);
            traceAst(a.son);
        }
    }

    /**
     * Visit all the nodes rooted in the specified one and set in them their position,
     * father and sequence number, ordered as the positions.
     *
     * @param      ast reference to the AST node
     * @param      pos reference to the position
     * @param      fat reference to the AST node father
     * @return     string
     */

    private void setPosAst(AstNode ast, int[] pos, AstNode fat){
        if (ast == null) return;
        ast.pos = pos;
        ast.fat = fat;
        if (fat != null){
            ast.lev = ast.fat.lev + 1;
        }
        ast.seq = this.astSeq++;
        this.astMap[ast.seq] = ast;
        int n = 1;
        for (AstNode a = ast.son; a != null; a = a.bro){
            int[] newpos = Arrays.copyOf(pos,pos.length+1);
            newpos[pos.length] = n;
            setPosAst(a,newpos,ast);
            n++;
        }
    }

    /**
     * Visit all the nodes rooted in the specified one and set in them their position,
     * father and sequence number, ordered as the positions; then it build the map
     * of ast nodes adding the EOF at the end.
     */

    private void setPosAst(){
        int n = countAst(this.astRoot);
        this.astMap = new AstNode[n + 1];       // for the eof
        this.astSeq = 0;
        setPosAst(this.astRoot,new int[0],null);
        this.eofAst = newAstNode(A_LEA);
        this.eofAst.sym = newSymbolChar(EOF);         // eof
        this.astMap[this.eofAst.seq] = this.eofAst;
    }

    /**
     * Visit all the nodes rooted in the specified one and deliver their number,.
     *
     * @return     number
     */

    private int countAst(AstNode ast){
        int n = 1;
        for (AstNode a = ast.son; a != null; a = a.bro){
            n += countAst(a);
        }
        return n;
    }


    // ---------- Syntax analysis of a RE -----------------

    /* Syntax
     *
     *  - the syntax of RE as reported in papers is not a good one for applications.
     *    E.g. where a RE is allowed, a single terminal or a concatenations can occur.
     *    An application must instead have only one thing, which can contain one or several
     *    elements. Something like, e.g.:
     *
     *           r -> alt {| alt}
     *           alt -> {factor}+
     *           factor -> terminal | (r) | (r)? | (r)+ | (r)* | (r){m [,[n]]} | set
     *           set -> "." | "[" {terminal | terminal "-" terminal}+ "]"
     *
     *  - when a RE is a single character, ?, +, * and {} can occur without parentheses.
     *  - conditions:
     *       {n}:    n > 0
     *       {n,m}:  n > m
     */

    /** Whether alternative operator is binary (otherwise n-ary). */
    private boolean altbin;

    /** The reference to the current node. */
    private AstNode curNode; 

    /** The reference to the root of the AST. */
    private AstNode astRoot;

    /** The index in RE (as a string). */
    private int cursor;

    /** The index of the start of a token in RE (as a string). */
    private int startToken;

    /** The RE to be analysed. */
    private String re;

    /**
     * Build the AST from the specified string containing the RE.
     *
     * @param      re string
     * @return     reference to the root of the AST
     */

    private AstNode buildAst(String re){
        this.re = re;
        if ((FL_E & trc) != 0){Trc.out.printf("buildAst %s\n",re);}
        this.astSeq = 0;
        this.error = false;

        this.cursor = 0;
        this.startToken = 0;
        this.curNode = null;
        expression();
        if (this.error || getsym() != -1){    // error or whole re not consumed
            return null;
        }
        AstNode ast = this.curNode;
        this.astRoot = ast;
        setPosAst();

        if ((FL_E & this.trc) != 0){
            traceAst(ast);
            for (int i = 0; i < this.astMap.length; i++){
                Trc.out.printf("buildAst astMap[%s] = %s\n",i,this.astMap[i]);
            }
        }
        return this.astRoot;
    }

    /** The AST representing the end of text. */
    private AstNode eofAst;

    /** The char representing the end of text. */
    private static final char EOF = '\u22a3';

    /** Whether an error has occurred. */
    private boolean error;

    /**
     * Parse an expression.
     */

    private void expression(){
        if ((FL_A & trc) != 0){Trc.out.printf("expression start at: %s\n",this.cursor);}
        this.curNode = null;
        AstNode r = null;
        AstNode altnode = null;
        doit: {
            subexpression();                         // allow also nothing
            if (this.error) return;
            if (this.curNode == null){
                this.curNode = newAstNode(A_EMP);    // return empty
                this.curNode.sym = newSymbolChar('\u03b5');
            }
            r = this.curNode;
            int n = 1;
            l: for (;;){
                gsep();
                int symv = getsym();
                char sym = (char)symv;
                if (symv < 0) break;
                if (sym != '|'){
                    this.cursor--;
                    break;
                }
                if (altnode == null){
                    altnode = newAstNode(A_ALT);
                    altnode.son = r;
                    r.altnr = n;
                }
                n++;
                if ((FL_A & trc) != 0){Trc.out.printf("expression inner at: %s\n",this.cursor);}
                subexpression();                           // alternative, require a term
                if (this.error) return;
                if (this.curNode == null){
                    this.curNode = newAstNode(A_EMP);      // return empty
                    this.curNode.sym = newSymbolChar('\u03b5');
                }
                r.bro = this.curNode;                      // store anyway
                r = this.curNode;
                r.altnr = n;
                if (this.altbin){                          // generate binary alternatives
                    if (altnode.son.bro != null &&         // three sons present
                        altnode.son.bro.bro != null){      // three sons present
                        AstNode curaltnode = newAstNode(A_ALT);
                        curaltnode.son = altnode.son;
                        altnode.son = curaltnode;
                        curaltnode.son.bro.bro = null;
                        curaltnode.bro = this.curNode;
                    }
                }
            } // l;
        } // doit
        if (altnode != null){
            this.curNode = altnode;
        }
        if ((FL_A & trc) != 0){Trc.out.printf("expression end at: %s %s re %s\n",this.cursor,this.curNode,this.curNode==null?"":this.curNode.toRE());}
    }

    /**
     * Parse a subexpression.
     */

    private void subexpression(){
        if ((FL_A & trc) != 0){Trc.out.printf("subexpression start at: %s\n",this.cursor);}
        this.curNode = null;
        factor();
        if (this.error) return;
        AstNode concnode = null;
        if (this.curNode != null){
            AstNode q = this.curNode;
            int n = 1;
            l: for (;;){
                AstNode p = this.curNode;
                n++;
                if ((FL_A & trc) != 0){Trc.out.printf("subexpression inner at: %s\n",this.cursor);}
                factor();
                if (this.error) return;
                if (this.curNode != null){    // several terms
                    if (concnode == null){
                        concnode = newAstNode(A_CON);
                        concnode.son = p;
                        q = p;
                    }
                } else {
                    this.curNode = p;
                    break;
                }
                q.bro = this.curNode;
                q = this.curNode;
            } // l
        }
        if (concnode != null){
            this.curNode = concnode;
        }
        if ((FL_A & trc) != 0){Trc.out.printf("subexpression end at: %s %s re %s\n",this.cursor,this.curNode,this.curNode==null?"":this.curNode.toRE());}
    }

    /** Whether simple groups are not represented in the AST. */
    private boolean noGRO;

    /**
     * Parse a factor.
     */

    private void factor(){
        if ((FL_A & trc) != 0){Trc.out.printf("factor start at: %s\n",this.cursor);}
        this.curNode = null;
        gsep();
        int symv = getsym();
        char sym = (char)symv;
        AstNode groupnode = null;
        if ((FL_A & trc) != 0){Trc.out.printf("factor symv %s sym %s\n",symv,sym);}
        doit: if (symv == -1){
            if ((FL_A & trc) != 0){Trc.out.printf("factor return %s\n",this.error);}
            return;
        } else if (sym == '|' || sym == ')'){
            this.cursor--;
        } else if (sym == '('){
            if ((FL_A & trc) != 0){Trc.out.printf("factor group start\n");}
            expression();
            if (this.error) return;
            groupnode = newAstNode(A_GRO);
            groupnode.groupKind = G_GRO;
            groupnode.son = this.curNode;
            groupnode.lowerbound = 1;
            groupnode.upperbound = 1;
            if (getsym() != ')'){
                this.error = true;
                return;
            }
            symv = getsym();
            sym = (char)symv;
            if ((FL_A & trc) != 0){Trc.out.printf("factor group end %s\n",sym);}
            if (sym == '*'){
                groupnode.groupKind = G_RE0;
                groupnode.lowerbound = 0;
                groupnode.upperbound = -1;
            } else if (sym == '+'){
                groupnode.groupKind = G_RE1;
                groupnode.lowerbound = 1;
                groupnode.upperbound = -1;
            } else if (sym == '?'){
                groupnode.groupKind = G_OPT;
                groupnode.lowerbound = 0;
                groupnode.upperbound = 1;
            } else if (sym == '{'){
                getbounds(groupnode);
                if (this.error){
                    return;
                }
            } else if (symv != -1){
                this.cursor--;
            }
            if (groupnode.groupKind == G_GRO && this.noGRO){
                this.astSeq--;                     // simple group: discard
                groupnode = this.curNode;
            }
            if ((FL_A & trc) != 0){Trc.out.printf("factor group capturing %s\n",groupnode);}
        } else if (sym == '['){                    // set
            if ((FL_A & trc) != 0){Trc.out.printf("factor set start\n");}
            this.curNode = newAstNode(A_LEA);
            this.curNode.sym = new Symbol();
            this.curNode.sym.kind = S_SET;
            this.curNode.sym.symset = new boolean[this.parameters.alphabet.length()];
            boolean first = true;
            int prev = -1;
            for (;;){
                symv = getsym();
                if (symv < 0){
                    this.error = true;
                    return;
                }
                sym = (char)symv;
                if (sym == ']') break;
                if (first){
                    if (sym == '^'){
                        this.curNode.sym.kind = S_NSET;
                        continue;
                    }
                    first = false;
                }
                int aidx = 0;
                if (sym == '-'){  // range
                    symv = getsym();
                    if (symv < 0){
                        this.error = true;
                        return;
                    }
                    sym = (char)symv;
                    if (sym == '-' || sym == ']'){
                        this.error = true;
                        return;
                    }
                    aidx = this.parameters.alphabet.indexOf((char)symv);
                    if (aidx < 0 || aidx < prev){             // not in alphabet, or < lower
                        this.error = true;
                        return;
                    }
                    for (int j = prev+1; j <= aidx; j++){
                        this.curNode.sym.symset[j] = true;
                    }
                } else {
                    aidx = this.parameters.alphabet.indexOf((char)symv);
                    if (aidx < 0){             // not in alphabet
                        this.error = true;
                        return;
                    }
                    this.curNode.sym.symset[aidx] = true;
                }
                if (sym != '-') prev = aidx;
            }
            if ((FL_A & trc) != 0){Trc.out.printf("factor set symset %s %s\n",symSetToString(this.curNode.sym.symset,true),this.curNode.sym.kind==S_SET);}
            if (this.curNode.sym.kind == S_NSET){
                for (int i = 0; i < this.curNode.sym.symset.length; i++){
                    this.curNode.sym.symset[i] = !this.curNode.sym.symset[i];
                }
            }
        } else if (sym == '.'){                    // dot
            this.curNode = newAstNode(A_LEA);
            this.curNode.sym = new Symbol();
            this.curNode.sym.kind = S_DOT;
            this.curNode.sym.sym = sym;
            if (dotset == null){
                dotset = new boolean[this.parameters.alphabet.length()];
                Arrays.fill(dotset,true);
                dotset['\n'] = false;
            }
            this.curNode.sym.symset = dotset;
        } else {
            if (sym == '\u03b5'){
                this.curNode = newAstNode(A_EMP);      // return empty
                this.curNode.sym = new Symbol();
                this.curNode.sym.kind = S_CHAR;
                this.curNode.sym.sym = sym;
                break doit;
            }
            if (sym == '\u03a6'){
                this.curNode = newAstNode(A_NUL);      // return empty set
                this.curNode.sym = new Symbol();
                this.curNode.sym.kind = S_CHAR;
                this.curNode.sym.sym = sym;
                break doit;
            }
            if (this.parameters.alphabet.indexOf(sym) < 0){
                this.error = true;
                return;
            }
            this.curNode = newAstNode(A_LEA);          // normal leaf
            this.curNode.sym = new Symbol();
            this.curNode.sym.kind = S_CHAR;
            this.curNode.sym.sym = sym;
        } // doit
        if (groupnode != null){
            this.curNode = groupnode;
        }
        for (;;){
            gsep();
            symv = getsym();
            sym = (char)symv;
            if (sym == '*'){
                groupnode = newAstNode(A_GRO);
                groupnode.groupKind = G_RE0;
                groupnode.lowerbound = 0;
                groupnode.upperbound = -1;
                groupnode.son = curNode;
            } else if (sym == '+'){
                groupnode = newAstNode(A_GRO);
                groupnode.groupKind = G_RE1;
                groupnode.lowerbound = 1;
                groupnode.upperbound = -1;
                groupnode.son = this.curNode;
            } else if (sym == '?'){
                groupnode = newAstNode(A_GRO);
                groupnode.groupKind = G_OPT;
                groupnode.lowerbound = 0;
                groupnode.upperbound = 1;
                groupnode.son = this.curNode;
            } else if (sym == '{'){                  // parse {n,m}
                groupnode = newAstNode(A_GRO);
                getbounds(groupnode);
                if (this.error){
                    return;
                }
            } else if (symv != -1){
                this.cursor--;
                break;
            } else {
                break;
            }
            this.curNode = groupnode;
        }
        if ((FL_A & trc) != 0){Trc.out.printf("factor end at: %s %s re %s\n",this.cursor,this.curNode,this.curNode==null?"":this.curNode.toRE());}
    }

    /** The set of characters represented by the dot */
    private static boolean[] dotset;

    /**
     * Get the bounds of a bounded group.
     */

    private AstNode getbounds(AstNode groupnode){
        if ((FL_A & trc) != 0){Trc.out.printf("getbounds start at: %s\n",this.cursor);}
        groupnode.groupKind = G_RE2;
        groupnode.son = this.curNode;
        // get the lower bound
        int bound = getint();
        if (bound < 0){                      // integer not found or illegal
            this.error = true;
            if ((FL_A & trc) != 0){Trc.out.printf("getbounds error\n");}
            return null;
        }
        groupnode.lowerbound = bound;
        int symv = getsym();
        if (symv == -1){
            this.error = true;
            if ((FL_A & trc) != 0){Trc.out.printf("getbounds error\n");}
            return null;
        }
        char sym = (char)symv;
        if (sym == '}'){                     // {n}
            groupnode.upperbound = 0;
            if (groupnode.lowerbound <= 0){
                this.error = true;
                if ((FL_A & trc) != 0){Trc.out.printf("getbounds error\n");}
                return null;
            }
        } else if (sym == ','){
            symv = getsym();
            if (symv == -1){
                this.error = true;
                if ((FL_A & trc) != 0){Trc.out.printf("getbounds error\n");}
                return null;
            }
            sym = (char)symv;
            if (sym == '}'){                 // {n,}
                groupnode.upperbound = -1;
                if (groupnode.lowerbound == 0){
                    groupnode.groupKind = G_RE0;
                } else if (groupnode.lowerbound == 1){
                    groupnode.groupKind = G_RE1;
                }
            } else {                         // {n,m}
                this.cursor--;
                // get the upper bound
                bound = getint();
                if (bound < 0){
                    this.error = true;
                    if ((FL_A & trc) != 0){Trc.out.printf("getbounds error\n");}
                    return null;
                }
                groupnode.upperbound = bound;
                if (groupnode.upperbound <= groupnode.lowerbound){
                    this.error = true;
                    if ((FL_A & trc) != 0){Trc.out.printf("getbounds error\n");}
                    return null;
                }
                symv = getsym();
                if (symv == -1){
                    this.error = true;
                    if ((FL_A & trc) != 0){Trc.out.printf("getbounds error\n");}
                    return null;
                }
                sym = (char)symv;
                if (sym != '}'){
                    this.error = true;
                    if ((FL_A & trc) != 0){Trc.out.printf("getbounds error\n");}
                    return null;
                }
            }
        } else {
            this.error = true;
            if ((FL_A & trc) != 0){Trc.out.printf("getbounds error\n");}
            return null;
        }
        if ((FL_A & trc) != 0){Trc.out.printf("getbounds end at: %s\n",this.cursor);}
        return groupnode;
    }

    /**
     * Get an integer >= 0.
     *
     * @return     value, -1 if not present
     */

    private int getint(){
        int res = 0;
        boolean found = false;
        for (;;){
            int cur = this.cursor;
            int symv = getsym();
            if (symv < 0) break;               // end of data
            if (this.cursor-cur > 1){          // number ended by spaces
                this.cursor--;
                break;
            }
            char c = (char)symv;
            if ((c < '0') || ('9' < c)){
                this.cursor--;
                break;
            }
            res *= 10;
            if (res > 0){
                found = false;
                break;
            }
            int digit = (int)c - '0';
            res -= digit;                     // use negative accumulators, ..
            if (res > 0){                     // .. which is larger than a ..
                found = false;                // .. positive one
                break;
            }
            found = true;
        }
        if (found){
            res = -res;
            if (res < 0){
                found = false;
            }
        }
        if (!found) res = -1;
        if ((FL_A & trc) != 0){Trc.out.printf("getint res: %d\n",res);}
        return res;
    }

    /**
     * Get the next character from the RE.
     *
     * @return     character, or -1 if no more available
     */

    private int getsym(){
        if ((FL_B & trc) != 0){Trc.out.printf("getsym cursor: %s re %s\n",this.cursor,this.re);}
        int res;
        for (; this.cursor < this.re.length(); this.cursor++){
             if (this.re.charAt(this.cursor) != ' ') break;
        }
        if (this.cursor >= this.re.length()){   // eof
            res = -1;
        } else {
            res = this.re.charAt(this.cursor++);
        }
        if ((FL_B & trc) != 0){Trc.out.printf("getsym ret cursor: %s re %s res %s %s\n",this.cursor,this.re,res,(char)res);}
        return res;
    }

    /**
     * Skip whitespaces.
     *
     * @return     character, or -1 if no more available
     */

    private void gsep(){
        for (; this.cursor < this.re.length(); this.cursor++){
             if (this.re.charAt(this.cursor) != ' ') break;
        }
        this.startToken = this.cursor;
    }

    /** A set of characters or character classes. */

    private class BoolSet implements Comparable {

        /** The set: each true element is a member of the set. */
        boolean[] arr;

        /**
         * Construct an empty set.
         */

        private BoolSet(){
            this.arr = new boolean[parameters.alphabet.length()];
        }

        /**
         * Add all the character in the specified range to this set.
         *
         * @param   lo lower bound, included
         * @param   hi higher bound, excluded
         */

        private void addRange(int lo, int hi){
            for (int i = lo; i < hi; i++){
                this.arr[i] = true;
            }
        }

        /**
         * Assign this set with all the elements of the specified set.
         *
         * @param   b set, make of all the true elements
         */

        private void assignArr(boolean[] b){
            for (int i = 0; i < b.length; i++){
                this.arr[i] = b[i];
            }
        }

        /**
         * Assign this set with all the elements of the specified set.
         *
         * @param   b set
         */

        private void assignSet(BoolSet b){
            for (int i = 0; i < this.arr.length; i++){
                this.arr[i] = b.arr[i];
            }
        }

        /**
         * Add or removes the specified element.
         *
         * @param   i index of the element
         * @param   v <code>true</code> to add the element, <code>false</code> to remove it
         */

        private void set(int i, boolean v){
            this.arr[i] = v;
        }

        /**
         * Empty this set.
         */

        private void clear(){
            Arrays.fill(this.arr,false);
        }

        /**
         * Complement this set.
         */

        private void complement(){
            for (int i = 0; i < this.arr.length; i++){
                this.arr[i] = !this.arr[i];
            }
        }

        /**
         * Add all the elements of the specified set to this set.
         *
         * @param   b set
         */

        private void or(BoolSet b){
            int len = b.arr.length;
            if (this.arr.length < len) len = this.arr.length;
            for (int i = 0; i < len; i++){
                this.arr[i] |= b.arr[i];
            }
        }

        /**
         * Make the intersection of this set and the specified one.
         *
         * @param   b set
         */

        private void and(BoolSet b){
            int len = b.arr.length;
            if (this.arr.length < len) len = this.arr.length;
            for (int i = 0; i < len; i++){
                this.arr[i] &= b.arr[i];
            }
            for (int i = b.arr.length; i < this.arr.length-b.arr.length; i++){
                this.arr[i] = false;
            }
        }

        /**
         * Subtracts the specified set from this one.
         *
         * @param   b set
         */

        private void sub(BoolSet b){
            int len = b.arr.length;
            if (this.arr.length < len) len = this.arr.length;
            for (int i = 0; i < len; i++){
                if (b.arr[i]) this.arr[i] = false;
            }
        }

        /**
         * Tell if this set is empty.
         *
         * @return  <code>true</code> if empty, <code>false</code> otherwise
         */

        private boolean isEmpty(){
            for (int i = 0; i < this.arr.length; i++){
                if (this.arr[i]) return false;
            }
            return true;
        }

        /**
         * Tell if this set is equal to the specified one.
         *
         * @return  <code>true</code> if equal, <code>false</code> otherwise
         */

        public boolean equals(Object other){
            return equals((BoolSet)other);
        }

        /**
         * Tell if this set is equal to the specified one.
         *
         * @return  <code>true</code> if equal, <code>false</code> otherwise
         */

        public boolean equals(BoolSet other){
            return Arrays.equals(this.arr,other.arr);
        }

        /**
         * Deliver a string representing this set.
         *
         * @return  string
         */

        public String toString(){
            return symSetToString(this.arr,true);
        }

        /**
         * Tell if this set precedes, is equal or follows the specified
         * other one, comparing the elements in their natural ordering.
         *
         * @param   other the other set
         * @return  -1 if it precedes, 0 if equal, 1 if it follows
         */

        public int compareTo(Object other){
            if (other == null) return 1;
            BoolSet set = (BoolSet)other;
            boolean thisempty = this.isEmpty();
            boolean otherempty = set.isEmpty();
            if (thisempty && !otherempty) return -1;
            if (!thisempty && otherempty) return 1;
            int len = set.arr.length;
            if (this.arr.length < len) len = this.arr.length;
            for (int i = 0; i < len; i++){
                if (this.arr[i] == set.arr[i]) continue;
                if (this.arr[i]) return -1;
                if (set.arr[i]) return 1;
            }
            for (int i = set.arr.length; i < this.arr.length-set.arr.length; i++){
                if (set.arr[i]) return 1;
            }
            return 0;
        }

        /**
         * Deliver a copy of this set.
         *
         * @return  copy
         */

        public BoolSet clone(){
            BoolSet b = new BoolSet();
            b.arr = Arrays.copyOf(this.arr,this.arr.length);
            return b;
        }

        /**
         * Deliver the array of the charactes in this set.
         *
         * @return  array
         */

        private char[] toArray(){
            int n = 0;
            for (int i = 0; i < this.arr.length; i++){
                if (this.arr[i]) n++;
            }
            char[] res = new char[n];
            n = 0;
            for (int i = 0; i < this.arr.length; i++){
                if (this.arr[i]) res[n++] = (char)i;
            }
            return res;
        }
    }

    /**
     * Compile the specified regular expression.
     *
     * @param      re string of the regular expression
     * @return     <code>true</code> if successful, <code>false</code> otherwise
     */

    public boolean compile(String re){
        this.error = false;
        buildAst(re);
        if (getsym() != -1){      // whole re not consumed
            this.error = true;
        }
        stringsNr(this.astRoot);
        return !this.error;
    }

    /** A generator of random numbers that can remember them and play back them. */

    private class RandomClass {

        /** Whether new random numbers are brand new (0) or are taken from the loaded record
         * (-1) or are delivered in sequence (-2).
         */
        int playback = -1;

        /** The record. */
        LinkedList<Double> record = null;

        /** The current position in the record. */
        int position = -1;

        /** The generator of random numbers. */
        Random ran = new Random();

        /** The step from one number to the next when deliverin in sequence. */
        double step = 0.1;

        /** The last delivered number. */
        double current = 0;

        /**
         * Deliver a random number.
         *
         * @return  number
         */

        private double get(){
            double res = 0;
            doit: {
                if (this.playback >= 0){
                    res = this.record.get(this.playback++);
                    this.current = res;
                    break doit;
                }
                if (this.playback == -2){
                    this.current += this.step;
                    if (this.current >= 1.0) this.current = 0;
                    res = this.current;
                } else {
                    res = this.ran.nextDouble();
                    this.current = res;
                }
                if (this.position >= 0){
                    this.record.add(res);
                }
            } // doit      
            if ((FL_K & trc) != 0){Trc.out.printf("get random playback %s res %s\n",this.playback,res);}
            return res;
        }

        /**
         * Tell the generator to record the numbers delivered.
         */

        private void rec(){
            this.position = 0;
            this.record = new LinkedList<Double>();
        }

        /**
         * Reset the generator to deliver random numbers without recording any.
         */

        private void reset(){
            this.playback = -1;
            this.position = -1;
            this.record = null;
            this.current = 0;
        }

        /**
         * Tell the generator to deliver numbers out of the currently loaded record.
         *
         * @return  number
         */

        private void play(){
            this.playback = 0;
        }

        /**
         * Tell the generator to deliver numbers in sequence (needed for testing).
         */

        private void test(){
            reset();
            this.playback = -2;
        }

        /**
         * Load the specified record and tell the generator to deliver numbers out of it.
         *
         * @param   arr record
         */

        private void load(double[] arr){
            this.record = new LinkedList<Double>();
            for (int i = 0; i < arr.length; i++){
                this.record.add(arr[i]);
            }
            play();
        }
    }

    /**
     * Deliver a random number from 0 (included) to the specified maximum (excluded).
     *
     * @param      max maximum
     * @return     number
     */

    private int getRandom(int max){
        double r = randomNumber.get();
        return (int)Math.floor(r * max);
    }

    /**
     * Deliver a random number from the specified minimum (included) to the specified
     * maximum (excluded).
     *
     * @param      min minimum
     * @param      max maximum
     * @return     number
     */

    private int getRandomRange(int min, int max){
        max -= min;
        double r = randomNumber.get();
        return (int)Math.floor(r * max) + min;
    }

    /** The random numbers generator. */
    private RandomClass randomNumber = new RandomClass();


    // ---------- Generation of REs -----------------

    /*
     * Balanced trees:
     *
     *     1. decompose the number of leaves into prime factors: a vector indexed by bases
     *        (which are small, e.g. from 2 to 19) containing the exponents; add also the exponent
     *        for arity 1, which is depth - number of non-zero elements
     *     2. reject if the bases are out of the allowed ranges, or (better), redo for a number
     *        of leaves that is one lower or one higher
     *     3. start from the root: take an arity randomly out of the vector and create the sons
     *     4. attach to the sons a tuple of parameters that is: the arity vector, updated for
     *        the arity just used
     *     5. do 3, 4 for each son
     *
     *   It alternates randomly the unary and non-unary nodes, avoiding unary-unary adjacences.
     *   It computes the remaining ratio between unary and non-unary, which cannot be greater
     *   than 1. When it would become > 1, a unary is taken. Otherwise it avoids to choose an
     *   unary if the father is unary.
     *
     *   The case of number of leaves that is prime, or that leads to arities that are out of
     *   range (range to be defined): it tries with decreasing numbers of leaves until an acceptable
     *   factoring is found. Then it generates the tree, and eventually it adds sons to some
     *   node in the last level that can take them (or in the last-1 if in the last there are no
     *   such nodes, but there are unary ones.
     * 
     *   This generate trees with nodes whose arity is a prime number. It we want to have also
     *   arities that are not prime, we can randomly decrement a couple of exponents and increment
     *   the one corresponding to a base that is multiplication.
     *
     *   Let's call the depth that is due to a factoring on prime factors "prime depth".
     *   We can generate trees whose depth is lower than that by decreasing the number of exponents
     *   and using bigger bases. We can also have deeper trees adding unary nodes, and we can also
     *   combine the two things.
     *   This defines a space of solutions, that is bounded by the interval of depths and those
     *   of the arities.
     *
     *   To have a factoring, we can take the prime one, and randomly pick up two exponents,
     *   multyply the bases and if the result is in the allowed range, then add this new basis
     *   and decrease the expended exponents.
     *   For each factoring we know the dept D: we then intersect the range D:2*D with the
     *   specified range and pick up randomly a depth in the intersection.
     *   If the intersection is empty, then 3 attempts are made to find a good one.
     *   Are the factorizations ordered so that we can draw a diagram with the factorizations in
     *   the abscissas and the depths in the ordinates, and the number of leaves in the z axis
     *   (obtaining a surface)?
     *   An ordering could be the lexicographical of the bases. Let's start with the prime one,
     *   and then take the two lowest bases, decrease their exponents and introduce a new basis.
     *   Then take the lowest basis and the third one, and do the same, and continue generating
     *   all the combinations (whose bases fall within the allowed interval).
     *   But for the bases whose exponent is > 1 we can also decrement the exponent by 2 and also
     *   use a basis that is the square, so the combinations are many more. And to have a nice
     *   surface I have probably to order them so that the dept is increasing or decreasing, which
     *   could be done for a number of leaves, but could provide a different result for another.
     *   It seems too complex. Probably, to take a random factorization I could pick up an exponent
     *   randomly, and then among the remaining ones another (2, otherwise the chance that the new
     *   basis is in the range is low).
     *
     *   Notes:
     *
     *    - number of leaves: we try the factorization in prime factors, up to and including 19.
     *      If not such factorization exists, then we try it for decreasing number of leaves until
     *      one is found. If it exists, then randomly we make a factorization in non-prime factors.
     *      If it does not exist, we try three times to factor.
     *      Then we compute the prime depth P, i.e. the one that of the tree that would be
     *      generated using only nodes whose arities are the factors of the factoring.
     *      We take then a random depth D in the range intersection between the one specified
     *      and P:P*2
     *      If D > P we add D-P unary nodes.
     *      Then the tree is built and eventually the missing leaves spread on nodes in the last
     *      level that can have them.
     *      Sometimes it could be impossible to add them because of the bounds on the arities.
     *      In such a case the tree has fewer leaves than the ones desired.
     *      If the chosen nodes are unary, then the last-1 level is serached for non-unary nodes,
     *      and then chains of unary nodes ending in leaves are added.
     *      N.B. we build trees with fewer leaves and grow them because it is simpler than removing
     *      leaves (it could lead to non-unary nodes that have only one leaf).
     *
     *    - we avoid unary nodes as sons of unary nodes
     *
     *    - leaves: we generate leaves according to the field "leaves" in the column "target":
     *
     *            - unspecified:   "a"
     *            - random:  a random letter
     *            - randomset:  a random letter or set
     *            - step:  increasing letters starting with "a"
     *
     *    - arities: the maximum ones can be defined, and not the minimum because it does not make
     *      much sense in practice.
     *      We allow two limits, one for concatenation and the other for alternative. This means
     *      that a factorization must satisfy both. We disregard the ones that have bases higher
     *      than the highest of the two. Then, when we choose the kind of nodes, if the current
     *      base to use is higher than the lowest of the two, we choose the other. Otherwise we
     *      choose randomly concatenation or alternative and randomly also the arity among the
     *      unexpended ones (the choice is done according to the frequency distribution of the
     *      exponents).
     *
     * Unbalanced trees
     *
     *   In order to be able to relate the depth to the number of leaves and the arities, the trees
     *   must have some kind of regularity or property.
     *   Let's start from the root and partition the number of leaves N into a number of pieces in the
     *   arity range.
     *   Let's say that there is one piece that is bigger than the others by a given factor: N/F,
     *   where F is lower than all the ones of the other pieces. Let's then do the same recursively
     *   on the sons, partitioning the biggest son always with the same criterion (and the same F).
     *   The biggest son is the central "bole", the highest one, and its height (well, depth) is approx
     *   log_F N (or more precisely, the number of elements in the progression N/F, N/F^2 ...).
     *   The number of elements in the progression from the initial one a to the n-th one is:
     *
     *       a_n = a r^(n-1)
     *
     *       since we want to have a_n = 1, and we start with a = N, and r = 1/f, we have:
     *
     *       1 = N (1/f)^(n-1),  f = N^(1/(n-1))
     *
     *       being here n the depth.
     *
     *   The other branches can have an arbitrary form.
     *   This also defines a space of solutions in which the depth depends on F: it can be increased
     *   decreasing F and/or adding unary nodes, and it can be decreased increasing F.
     *   In both cases the space of solutions is bounded by the range of arities.
     *
     *     - we take randomly a dept D in its range and a number N of not-unary nodes (D/2 <= N <= D),
     *       and compute the F to have a number of non-unary nodes in the bole N.
     *       The number of unary nodes is then D - N.
     *
     *     - partitioning leaves for a bole node determines also the number of its sons.
     *       If there are limits on the arities and a partitioning on the bole exceeds them,
     *       10 attempts are made with another partitioning
     *
     *   - when spreading the remainder, and f < 2, start with less than the remainder, otherwise
     *     we risk to have another N/F. In general, we should always get pieces that are < N/F
     *     and that must have a number of unary nodes < ratio of the bole, which can also be done
     *     by taking the previous piece and find randomly one that is smaller (e.g. picking a random
     *     value whose max is lower than the length of the previous piece). Note, however, that the
     *     problem is only with the bole, and thus we can reduce the current piece if greater.
     *   - a side branch with the same number of leaves as the bole (or a bit less) could have
     *     a depth that is higher than the bole (e.g. with lower arity). To avoid this, the ratio
     *     inherited by non-boles is set lower than the one of the bole
     *   - there is a relation between f and the number of sons: N - N/F, which is the number
     *     allotted to sons, divided by nr-sons -1 could be bigger than N/F.
     *     This means that the choosen depth and the choosen arity cannot be completely free.
     *     I partition and then see if the resulting arity is within the range, and if not try another
     *     partitioning.
     *
     *   - alternation
     *
     *     - if ratio (unary/not-unary) R > 0
     *       - the first node is not-unary
     *       - the second is unary
     *       - the third is then not-unary and the current ratio C after it is 1/3
     *       - the fourth if there are still unary to be placed, and C < R, then unary, else not-unary
     *         If there are no more unary to be placed C could be < R? No because if the number of
     *         non-unary placed is lower than the final one, then C > R.
     *         This must stop when the depth has been reached.
     *         The only peculiar case is when there is only one leaf remaining, and not all unaries
     *         have been placed, then C < R and in such a case we must continue growing.
     *
     *       - for non-bole nodes, we use a lower R
     *         But here we do not know how many unaries to place. No problem, continue until
     *         C < R (in which case place a unary) or nr-leaves = 1.
     *
     *   - depth is the number of nodes from the root (included) to the furthest leaf (excluded).
     *     Notunary is the number of not-unary nodes computed as a random number between depth/2
     *     and depth. The number of elements in the progression includes the last one, that is 1,
     *     and that is the terminal. So, it is notunary+1.
     *     Thus, the number of unary nodes is depth - notunary.
     */

    /**
     * Deliver a random set of terminals.
     *
     * @return     symbol
     */

    private Symbol randomTerminalSet(){
        Symbol sym = new Symbol();
        int r = getRandom(2);          // single terminal or set
        if ((FL_C & trc) != 0){Trc.out.printf("randomTerminalSet r %s\n",r);}
        if (r == 0){
            r = getRandom(this.parameters.alphabet.length());
            sym = newSymbolChar(this.parameters.alphabet.charAt(r));
            if ((FL_C & trc) != 0){Trc.out.printf("randomTerminalSet %s\n",sym);}
            return sym;
        }
        sym.kind = S_SET;
        sym.symset = new boolean[this.parameters.alphabet.length()];
        // add a random number of ranges, each range starts at a random char and extends for
        // a random number of char; the next range starts at a random gap from the previous one
        r = getRandom(4);          // single terminal or set
        if (r == 0) r = 1;
        int card = 0;
        int s = getRandom(this.parameters.alphabet.length()/2);    // start
        for (int k = 0; k < r; k++){
            int e = getRandom(this.parameters.alphabet.length()/4);
            e += s;
            e %= this.parameters.alphabet.length();
            for (int i = s; i < e; i++){
                sym.symset[i] = true;
                card++;
            }
            s = getRandom(this.parameters.alphabet.length()/4);
            s += e;
        }
        if (card == 0){            // empty
            sym.symset[getRandom(this.parameters.alphabet.length())] = true;
        }
        r = getRandom(3);          // negation
        // generate a negated range, but it looks odd
        if (r == 1){
            sym.kind = S_NSET;
            for (int i = 0; i < sym.symset.length; i++){
                sym.symset[i] = !sym.symset[i];
            }
        }
        if ((FL_C & trc) != 0){Trc.out.printf("randomTerminalSet %s\n",sym);}
        return sym;
    }

    /*
     * The table of allowed nestings.
     *
     *          none  alt conc ()  ()?  ()*  ()+  (){n,m}
     *   ε
     *   Σ
     *   alt         each cell is either true to indicate that
     *   conc        the kind in row is disallowed as son of the
     *   ()          kind in the column; the first column is
     *   ()?         for the root, nested in none
     *   ()*
     *   ()+
     *   (){n,m}
     *
     */

    /** The map from AST kinds to the rows of the allowed nesting matrix. */
    private static final int[] toAllowedRow = {1,2,3,0,0};

    /** The map from AST kinds to the columns of the allowed nesting matrix. */
    private static final int[] toAllowedCol = {0,1,2,0,0};

    /** The map from columns of the allowed nesting matrix to the AST non-group kinds. */
    private static final int[] toAllowedKind = {A_EMP,A_LEA,A_ALT,A_CON};

    /**
     * Tell if the specified AST can be a son of the specified father one.
     *
     * @param      son son AST 
     * @param      fat father AST
     * @return     <code>true</code> if allowed, <code>false</code> otherwise
     */

    private boolean allowedNesting(AstNode son, AstNode fat){
        return allowedNesting(son.kind,son.groupKind,fat);
    }

    /**
     * Tell if the specified AST kind can be a son of the specified father one.
     *
     * @param      kind kind of son AST
     * @param      group kind of son AST if group (irrelevant otherwise)
     * @param      fat father AST
     * @return     <code>true</code> if allowed, <code>false</code> otherwise
     */

    private boolean allowedNesting(int kind, int group, AstNode fat){
        if (this.parameters.adjacences == null) return true;
        int sonRow = toAllowedRow[kind];
        if (kind == A_GRO){
            sonRow = group+4;
        }
        int fatCol = 0;
        if (fat != null){
            fatCol = toAllowedCol[fat.kind];
            if (fat.kind == A_GRO){
                fatCol = fat.groupKind+3;
            }
        }
        return !this.parameters.adjacences[sonRow][fatCol];
    }

    /**
     * Disallow the specified AST kind as son of the specified father one.
     *
     * @param      sonkind kind of son
     * @param      songroup kind of group son, if group (irrelevant otherwise)
     * @param      fatkind kind of father, AST_EMP if no father
     * @param      fatgroup kind of group father, if group (irrelevant otherwise)
     */

    private void disallowNesting(int sonkind, int songroup, int fatkind, int fatgroup){
        if (this.parameters.adjacences == null){
            this.parameters.adjacences = new boolean[9][];
            for (int i = 0; i < 9; i++){
                this.parameters.adjacences[i] = new boolean[8];
            }
        }
        int sonRow = toAllowedRow[sonkind];
        if (sonkind == A_GRO){
            sonRow = songroup+4;
        }
        int fatCol = toAllowedCol[fatkind];
        if (fatkind == A_GRO){
            fatCol = fatgroup+3;
        }
        this.parameters.adjacences[sonRow][fatCol] = true;
        if ((FL_G & trc) != 0){Trc.out.printf("disallowNesting\n%s\n",matrixToString(this.parameters.adjacences));}
    }

    /**
     * Disallow the specified AST kind as son of any other.
     *
     * @param      sonkind kind of son
     * @param      songroup kind of group son, if group (irrelevant otherwise)
     */

    private void disallowNesting(int sonkind, int songroup){
        if (this.parameters.adjacences == null){
            this.parameters.adjacences = new boolean[9][];
            for (int i = 0; i < 9; i++){
                this.parameters.adjacences[i] = new boolean[8];
            }
        }
        int sonRow = toAllowedRow[sonkind];
        if (sonkind == A_GRO){
            sonRow = songroup+4;
        }
        for (int i = 0; i < 8; i++){
            this.parameters.adjacences[sonRow][i] = true;
        }
        if ((FL_G & trc) != 0){Trc.out.printf("disallowNesting\n%s\n",matrixToString(this.parameters.adjacences));}
    }

    /** The labels of the AST kinds. */
    private static final String[] ADJ_LABELS = new String[]{
        "ε","Σ","|","∙","()","?","*","+","{}"};

    /**
     * Disallow the specified AST kind as son of the specified father one.
     *
     * @param      son kind of son
     * @param      fat kind of father
     */

    public void disallowNesting(String son, String fat){
        if (this.parameters.adjacences == null){
            this.parameters.adjacences = new boolean[9][];
            for (int i = 0; i < 9; i++){
                this.parameters.adjacences[i] = new boolean[8];
            }
        }
        int sonRow = -1;
        for (int i = 0; i < ADJ_LABELS.length; i++){
            if (son.equals(ADJ_LABELS[i])){
                sonRow = i;
                break;
            }
        }
        if (sonRow < 0){
            throw new IllegalArgumentException("illegal son " + son);
        }
        int fatCol = -1;
        if (fat == ""){
            fatCol = 0;
        } else {
            for (int i = 2; i < ADJ_LABELS.length; i++){
                if (fat.equals(ADJ_LABELS[i])){
                    fatCol = i-1;
                    break;
                }
            }
        }
        if (fatCol < 0){
            throw new IllegalArgumentException("illegal fat " + fat);
        }
        this.parameters.adjacences[sonRow][fatCol] = true;
        if ((FL_G & trc) != 0){Trc.out.printf("disallowNesting\n%s\n",matrixToString(this.parameters.adjacences));}
    }

    /**
     * Disallow the specified AST kind as son of any other.
     *
     * @param      son kind of son
     */

    public void disallowNesting(String son){
        if (this.parameters.adjacences == null){
            this.parameters.adjacences = new boolean[9][];
            for (int i = 0; i < 9; i++){
                this.parameters.adjacences[i] = new boolean[8];
            }
        }
        int sonRow = -1;
        for (int i = 0; i < ADJ_LABELS.length; i++){
            if (son.equals(ADJ_LABELS[i])){
                sonRow = i;
                break;
            }
        }
        if (sonRow < 0){
            throw new IllegalArgumentException("illegal son " + son);
        }
        for (int i = 0; i < 8; i++){
            this.parameters.adjacences[sonRow][i] = true;
        }
        if ((FL_G & trc) != 0){Trc.out.printf("disallowNesting\n%s\n",matrixToString(this.parameters.adjacences));}
    }

    /**
     * Deliver a matrix representing the adjacences.
     *
     * @param      arr adjacences
     * @return     String
     */

    private String matrixToString(boolean[][] arr){
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < arr.length; i++){
            for (int j = 0; j < arr[i].length; j++){
                if (j > 0) sb.append(',');
                sb.append(arr[i][j]);
            }
            if (i < arr.length-1) sb.append('\n');
        }
        return sb.toString();
    }

    /** An array with the allowed nesting for each adjacences column. */
    private int[][] nestings;

    /**
     * Deliver a group of a random kind and range of repetitions in the specified AST node.
     *
     * @param      n AST node
     * @return     <code>true</code> if group allowed, <code>false</code>otherwise
     */

    private boolean randomGroup(AstNode n){
        if (this.nestings == null){
            this.nestings = new int[8][];
            int[] tmp = new int[9];
            for (int i = 0; i < 8; i++){
                int k = 0;
                for (int j = 4; j < 9; j++){
                    if (this.parameters.adjacences == null || !this.parameters.adjacences[j][i]){
                        tmp[k++] = j;
                    }
                }
                this.nestings[i] = Arrays.copyOf(tmp,k);
            }
            if ((FL_G & this.trc) != 0){
                for (int i = 0; i < 8; i++){
                    Trc.out.printf("%s: %s\n",i,Arrays.toString(nestings[i]));
                }
            }
        }
        if (this.parameters.starHeight != null){    // limit the iter depth
            // compute the number of iteration groups in the path
            int s = 0;
            for (AstNode a = n.fat; a != null; a = a.fat){
                if (a.kind == A_GRO && a.groupKind >= G_OPT) s++;
            }
            if (s >= this.parameters.starHeight[1]){
                // star height already reached, generate a simple group
                if (!allowedNesting(A_GRO,G_GRO,n.fat)) return false;
                n.kind = A_GRO;
                n.groupKind = G_GRO;
                n.lowerbound = 1;
                n.upperbound = 1;
                return true;
            }
        }
        int fatCol = 0;
        if (n.fat != null){
            fatCol = toAllowedCol[n.fat.kind];
            if (n.fat.kind == A_GRO){
                fatCol = n.fat.groupKind+3;
            }
        }
        if (nestings[fatCol].length == 0){
            return false;
        }
        int sonkind = getRandom(nestings[fatCol].length);
        n.kind = nestings[fatCol][sonkind];
        if (n.kind >= 4){
            n.groupKind = n.kind - 4;
            n.kind = A_GRO;
        } else {
            n.kind = toAllowedKind[n.kind];
        }
        if (n.kind == A_GRO){
            if (n.groupKind == G_GRO){
                n.lowerbound = 1;
                n.upperbound = 1;
            } else if (n.groupKind == G_OPT){
                n.lowerbound = 0;
                n.upperbound = 1;
            } else if (n.groupKind == G_RE0){
                n.lowerbound = 0;
                n.upperbound = -1;
            } else if (n.groupKind == G_RE1){
                n.lowerbound = 1;
                n.upperbound = -1;
            } else if (n.groupKind == G_RE2){
                double d = randomNumber.get();
                int delta = 0;
                if (d > 0.7){
                    delta = 1;
                } else if (d > 0.3){
                    delta = -1;
                }
                double k = randomNumber.get();
                n.lowerbound = this.parameters.iterrange[0] + delta;
                n.upperbound = 0;
                if (k > 0.7){                   // {m,n}
                    n.upperbound = this.parameters.iterrange[1] + delta;
                } else if (k > 0.3){            // {m,}
                    n.upperbound = -1;
                }
            }
        }
        return true;
    }

    /** The last generated terminal leaf. */
    private int stepTerminal;

    /**
     * Generate a random leaf into the specified AST.
     *
     * @param      a AST
     */

    private void getLeaf(AstNode a){
        if ((FL_G & trc) != 0){Trc.out.printf("getLeaf\n");}
        Symbol sym = null;
        if (this.parameters.leaves == LEAVES_SET){
            sym = randomTerminalSet();
        } else {
            int r = 0;
            if (this.parameters.leaves == LEAVES_TERM){
                r = getRandom(this.parameters.alphabet.length());
            } else if (this.parameters.leaves == LEAVES_STEP){
                r = stepTerminal++;
                r %= this.parameters.alphabet.length();
            }
            sym = newSymbolChar(this.parameters.alphabet.charAt(r));
        }
        if ((FL_G & trc) != 0){Trc.out.printf("getLeaf %s\n",sym);}
        a.sym = sym;
    }

    /** The table of the generated ASTs. */
    private LinkedList<AstNode> astTab;

    /**
     * Generate a RE as an AST.
     */

    private void generateRE(){
        this.astSeq = 0;
        this.astMap = null;
        this.astTab = new LinkedList<AstNode>();         // map from integers to ast nodes
        // if nr leaves = 0, then return an empty
        if (this.parameters.size == 0){
            this.astRoot = newAstNode(A_EMP);
            this.astTab.add(this.astRoot);
        } else {
            boolean balanced = this.parameters.balanced;
            if (balanced){
                generateBalanced();
            } else {
                generateUnbalanced();
            }
        }
        this.astMap = astListToArray(this.astTab);
    }

    /** The budget of generation parameters handed down to AST nodes. */

    private class GenData {

        /** The number of leaves. */
        int nrleaves;

        /** The depth range. */
        int[] depth;

        /** The arities of sons. */
        int[] ari;

        /** The ratio of unary and non-unary nodes. */
        double unRatio;

        /** The ratio of leaves reserved to the bole. */
        double bole;

        /**
         * Construct a set of generation parameters with the specified arguments.
         *
         * @param      leaves number of leaves
         * @param      depth range of depts
         * @param      arity range of number of sons
         * @param      uratio ratio of unary and non-unary nodes
         */

        private GenData(int leaves, int[] depth, int[] arity, double uratio){
            this.nrleaves = leaves;
            this.depth = depth;
            this.ari = arity;
            this.unRatio = uratio;
        }

        /**
         * Construct a set of generation parameters with the specified arguments.
         *
         * @param      leaves number of leaves
         * @param      depth range of depts
         * @param      bole ratio of leaves reserved to the bole
         * @param      uratio ratio of unary and non-unary nodes
         */

        private GenData(int leaves, int[] depth, double bole, double uratio){
            this.nrleaves = leaves;
            this.depth = depth;
            this.bole = bole;
            this.unRatio = uratio;
        }
    }

    /**
     * Convert a list of integers into an array.
     *
     * @param      list list
     * @return     array
     */

    private int[] intListToArray(LinkedList<Integer> list){
        int[] res = new int[list.size()];
        for (int i = 0; i < list.size(); i++){
            res[i] = list.get(i);
        }
        return res;
    }

    /**
     * Convert a list of AST nodes into an array.
     *
     * @param      list list
     * @return     array
     */

    private AstNode[] astListToArray(LinkedList<AstNode> list){
        AstNode[] res = new AstNode[list.size()];
        for (int i = 0; i < list.size(); i++){
            res[i] = list.get(i);
        }
        return res;
    }

    /**
     * Generate a balanded AST.
     */

    private void generateBalanced(){
        if (((FL_G|FL_R) & trc) != 0){Trc.out.printf("generateBalanced leaves %s\n",this.parameters.size);}
        int maxBase = MAX_BASES;
        if (this.parameters.concarity == 0) this.parameters.concarity = INFINITE;
        if (this.parameters.altarity == 0) this.parameters.altarity = INFINITE;
        if (this.parameters.concarity < maxBase){
            maxBase = this.parameters.concarity;
        }
        if (this.parameters.altarity < maxBase){
            maxBase = this.parameters.altarity;
        }
        int nl = this.parameters.size;     // number of leaves
        int[] arities = null;
        int remaining = 0;
        double unRatio = 0;
        int depth = 0;
        // make attempts to find a decomposition
        for (int k = 0; k < 3; k++){
            // find a decomposition in factors for n or decreasing numbers
            for (int i = 0; i < 10; i++){
                arities = decompose(nl,GEN_PRIMES,maxBase);
                if (arities != null){
                    randomFactoring(nl,arities,maxBase);
                    break;
                }
                nl--;
                // approximate
            }
            if (arities == null){               // none found
                if (k < 2) continue;
                // impossible
                this.astRoot = null;
                if ((FL_R & trc) != 0){Trc.out.printf("generateBalanced -1-\n");}
                return;
            }
            if (arities == null) continue;
            remaining = this.parameters.size - nl;
            if ((FL_G & trc) != 0){Trc.out.printf("generate arities %s\n",Arrays.toString(arities));}
            int nz = 0;
            for (int i : arities){
                if (i != 0) nz++;
            }
            // nz is the prime depth
            int k1 = this.parameters.depth[0];
            int k2 = this.parameters.depth[1] + 1;
            int nz1 = nz*2 + 1;
            // determine the intersection of the specified and possible depths
            if (k2 <= nz || nz1 <= k1){     // empty
                if (k < 2) continue;
                // impossible
                this.astRoot = null;
                if ((FL_R & trc) != 0){Trc.out.printf("generateBalanced -2-\n");}
                return;
            }
            int lo = Math.max(k1,nz);
            int hi = Math.min(k2,nz1);
            depth = lo;
            if (hi-lo > 1){
                // choose randomly a depth in the intersection
                depth = getRandomRange(lo,hi);
            }
            int unar = depth - nz;
            if (unar < 0) unar = 0;
            if ((FL_G & trc) != 0){Trc.out.printf("generate arities depth %s nz %s\n",depth,nz);}
            arities[1] = unar;            // the number of unary nodes in each path
            unRatio = 0;
            if (nz == 0){
                unRatio = Double.MAX_VALUE;
            } else {
                unRatio = unar/nz;            // of elements in the progression
            }
            break;
        }
        // convert to array of pairs base,exponent;
        LinkedList<Integer> ari = new LinkedList<Integer>();
        for (int i = 0; i < arities.length; i++){
            if (arities[i] != 0){
                ari.add(i);
                ari.add(arities[i]);
            }
        }
        if ((FL_G & trc) != 0){Trc.out.printf("generate arities pairs %s\n",ari);}
    
        this.astSeq = 0;
        this.cursor = 0;
        this.astRoot = newAstNode(A_CON);
        this.astTab.add(this.astRoot);
        this.astRoot.gendata = new GenData(this.parameters.size,this.parameters.depth,
            intListToArray(ari),unRatio);
        // this.astRoot.getIcon = newAstIcon;
        if ((FL_R & trc) != 0){Trc.out.printf("generateBalanced, arities in each path: %s\n",factorPairsToString(ari));}
        LinkedList<AstNode> buds = new LinkedList<AstNode>();
        buds.add(this.astRoot);
    
        // then grow the tree
        int maxlev = 0;
        for (int g = 0; buds.size() > 0; g++){
            /*
            if (g > 100){
                Trc.out.printf("bau\n");
                break;
            }
            */
            if ((FL_G & this.trc) != 0){
                Trc.out.printf(">>> buds");
                for (AstNode b : buds){
                    Trc.out.printf(" %s %s",b.seq,astKindString(b));
                }
                Trc.out.printf("\n");
            }
            AstNode n = buds.poll();        // get and remove first
            if ((FL_G & trc) != 0){Trc.out.printf("take node %s arities %s\n",n.seq,Arrays.toString(n.gendata.ari));}
            // grow it
    
            // choose its arity
            int ia = 0;
            choo: {
                // try to keep the unary/non-unary ratio
                int nu = 0;                 // number of unary
                int nnu = 0;                // number of non-unary
                if ((FL_G & trc) != 0){Trc.out.printf("node %s count nnu\n",n.seq);}
                for (AstNode a = n.fat; a != null; a = a.fat){
                    if (a.kind == A_GRO){
                        nu++;
                    } else {
                        nnu++;
                    }
                }
                double curRatio = 0.0;
                if (n.lev == 0){
                    curRatio = n.gendata.unRatio;
                } else if (nnu == 0){
                    curRatio = INFINITE;
                } else {
                    curRatio = nu/nnu;
                }
                boolean prevUnary = false;
                if (n.fat != null && n.fat.kind == A_GRO) prevUnary = true;
                if ((FL_G & trc) != 0){Trc.out.printf("curRatio %s nu %s nnu %s data %s prevu %s\n",curRatio,nu,nnu,n.gendata.unRatio,prevUnary);}
                if (curRatio < n.gendata.unRatio && !prevUnary){
                    ia = 0;                 // choose unary
                    break choo;
                }
    
                // then choose a random one
                // it is fairer to choose randomly in a vector in which there are a number
                // of entries that are equal to each exponent
    
                boolean nounary = n.fat != null && n.fat.kind == A_GRO;
                LinkedList<Integer> chooser = new LinkedList<Integer>();
                for (int i = 0; i < n.gendata.ari.length; i += 2){
                    if (nounary && n.gendata.ari[i] == 1) continue;    // avoid unary-unary
                    for (int j = 0; j < n.gendata.ari[i+1]; j++){
                        chooser.add(n.gendata.ari[i]);
                    }
                }
                if (chooser.size() > 0){
                    int a = chooser.get(getRandom(chooser.size()));
                    for (int i = 0; i < n.gendata.ari.length; i += 2){
                        if (n.gendata.ari[i] == a){
                            ia = i;
                        }
                    }
                    if ((FL_G & trc) != 0){Trc.out.printf("take node chooser %s: %s ia: %s\n",chooser,a,ia);}
                }
            }
            int nsons = n.gendata.ari[ia];
            if (nsons == 1){                     // then the node is unary
                if (!randomGroup(n)){
                    this.astRoot = null;
                    if ((FL_R & trc) != 0){Trc.out.printf("generateBalanced -3-\n");}
                    return;
                }
            } else if (nsons > this.parameters.concarity){
                n.kind = A_ALT;
            } else if (nsons > this.parameters.altarity){
                n.kind = A_CON;
            } else {
                n.kind = A_ALT;
                if (randomNumber.get() > 0.5) n.kind = A_CON;
                if (!allowedNesting(n,n.fat)){
                    if (n.kind == A_ALT) n.kind = A_CON;
                }
            }
            if (!allowedNesting(n,n.fat)){
                this.astRoot = null;
                if ((FL_R & trc) != 0){Trc.out.printf("generateBalanced -4-\n");}
                return;
            }
    
            LinkedList<Integer> newArity = new LinkedList<Integer>();
            for (int i = 0; i < n.gendata.ari.length; i++){
                newArity.add(n.gendata.ari[i]);
            }
            newArity.set(ia+1,newArity.get(ia+1)-1);    // remove one such node
            // and if it becomes 0, then remove the pair
            if (newArity.get(ia+1) == 0){
                newArity.remove(ia);
                newArity.remove(ia);
            }

            if ((FL_R & trc) != 0){Trc.out.printf("take node %s kind %s new arities %s sons:\n",n.seq,astKindString(n),factorPairsToString(newArity));}
            // generate sons
            if ((FL_G & trc) != 0){Trc.out.printf("grow node %s lev %s depth %s target leaves %s sons %s new arities %s\n",n.seq,n.lev,depth,n.gendata.nrleaves,nsons,newArity);}
            int[] arr = nsons == 0 ? new int[1] : new int[nsons];
            if (nsons > 1){
                partQuantity(n.gendata.nrleaves,arr,1);
            } else {
                arr[0] = n.gendata.nrleaves;
            }
            if ((FL_G & trc) != 0){Trc.out.printf("partQuantity done arr %s\n",Arrays.toString(arr));}
    
            AstNode prev = null;
            for (int i = 0; i < nsons; i++){
                if ((FL_G & trc) != 0){Trc.out.printf("father %s son %s-th\n",n.seq,i);}
                int kind = A_LEA;
                if (newArity.size() > 0){
                    kind = A_CON;
                }
                if (!allowedNesting(kind,0,n)){
                    this.astRoot = null;
                    if ((FL_R & trc) != 0){Trc.out.printf("generateBalanced -5-\n");}
                    return;
                }
                AstNode a = newAstNode(kind);
                // but it will be set properly after
                int l = arr[i];
                if (kind != A_LEA){
                    buds.add(a);
                    if ((FL_G & trc) != 0){Trc.out.printf("push %s\n",a.seq);}
                } else {
                    getLeaf(a);
                    l = 0;
                }
                a.lev = n.lev+1;
                if (a.lev > maxlev) maxlev = a.lev;
                a.fat = n;
                a.gendata = new GenData(l,new int[]{n.gendata.depth[0]-1,n.gendata.depth[1]-1},
                    intListToArray(newArity),unRatio);
                // a.getIcon = newAstIcon;
                this.astTab.add(a);
                if ((FL_R & trc) != 0){Trc.out.printf("    son %s kind %s leaves %s arities %s\n",a.seq,astKindString(a),l,factorPairsToString(newArity));}
                if (i == 0){
                    n.son = a;
                } else {
                    prev.bro = a;
                }
                prev = a;
            }
        }
        if (((FL_G|FL_R) & trc) != 0){Trc.out.printf("adjusting %s\n",remaining);}
    
        adj: if (remaining > 0){
            // adjust the tree
            // build an array of the ones at the last level that can take more sons
            LinkedList<Integer> growable = new LinkedList<Integer>();
            for (int i = 0; i < this.astTab.size(); i++){
                AstNode ast = this.astTab.get(i);
                if (ast.lev < maxlev-1) continue;
                if (ast.kind == A_ALT){
                    int ns = nrSons(ast);
                    if (ns >= this.parameters.altarity) continue;
                    int rem = this.parameters.altarity == INFINITE ? remaining : this.parameters.altarity-ns;
                    for (int j = 0; j < rem; j++){
                        growable.add(ast.seq);
                    }
                }                
                if (ast.kind == A_CON){
                    int ns = nrSons(ast);
                    if (ns >= this.parameters.concarity) continue;
                    int rem = this.parameters.concarity == INFINITE ? remaining : this.parameters.concarity-ns;
                    for (int j = 0; j < rem; j++){
                        growable.add(ast.seq);
                    }
                }                
            }
            if ((FL_G & trc) != 0){Trc.out.printf("growable %s\n",growable);}
            if (growable.size() > 0){
                for (; remaining > 0;){
                    int r = getRandom(growable.size());
                    AstNode ast = this.astTab.get(growable.get(r));
                    AstNode a = newAstNode(A_LEA);
                    if (!allowedNesting(A_LEA,0,ast)){
                        this.astRoot = null;
                        if ((FL_R & trc) != 0){Trc.out.printf("generateBalanced -6-\n");}
                        return;
                    }
                    getLeaf(a);
                    a.lev = ast.lev+1;
                    a.fat = ast;
                    a.gendata = new GenData(1,null,null,0);
                    // a.getIcon = newAstIcon;
                    this.astTab.add(a);
                    for (AstNode s = ast.son; s != null; s = s.bro){
                        if (s.bro == null){      // last
                            s.bro = a;
                            break;
                        }
                    }
                    remaining--;
                    if ((FL_G & trc) != 0){Trc.out.printf("remainder %s added to %s\n",a.seq,ast.seq);}
                    growable.remove(r);
                    if (growable.size() == 0) break;
                }
            }
            if ((FL_G & trc) != 0){Trc.out.printf("remaining again %s\n",remaining);}
            if (remaining == 0) break adj;
    
            // when remaining still > 0 we could try with the nodes in the level above that
            // are conc or alt adding a unary node with a leaf, but this also could fail
            growable = new LinkedList<Integer>();
            for (int i = 0; i < astTab.size(); i++){
                AstNode ast = this.astTab.get(i);
                if (ast.lev != maxlev-2) continue;
                if (ast.kind == A_ALT){
                    int ns = nrSons(ast);
                    if (ns >= this.parameters.altarity) continue;
                    int rem = this.parameters.altarity == INFINITE ? remaining : this.parameters.altarity-ns;
                    for (int j = 0; j < rem; j++){
                        growable.add(ast.seq);
                    }
                }                
                if (ast.kind == A_CON){
                    int ns = nrSons(ast);
                    if (ns >= this.parameters.concarity) continue;
                    int rem = this.parameters.concarity == INFINITE ? remaining : this.parameters.concarity-ns;
                    for (int j = 0; j < rem; j++){
                        growable.add(ast.seq);
                    }
                }                
            }
            if ((FL_G & trc) != 0){Trc.out.printf("growable %s\n",growable);}
            if (growable.size() > 0){
                for (; remaining > 0; remaining--){
                    int r = getRandom(growable.size());
                    AstNode ast = this.astTab.get(growable.get(r));
                    AstNode a = newAstNode(A_GRO);
                    if (!allowedNesting(A_GRO,G_GRO,ast)){
                        this.astRoot = null;
                        if ((FL_R & trc) != 0){Trc.out.printf("generateBalanced -7-\n");}
                        return;
                    }
                    a.groupKind = G_RE0;
                    a.lowerbound = 0;
                    a.upperbound = -1;
                    a.lev = ast.lev+1;
                    a.fat = ast;
                    a.gendata = new GenData(1,null,null,0);
                    // a.getIcon = newAstIcon;
                    this.astTab.add(a);
                    for (AstNode s = ast.son; s != null; s = s.bro){
                        if (s.bro == null){      // last
                            s.bro = a;
                            break;
                        }
                    }
                    AstNode l = newAstNode(A_LEA);
                    if (!allowedNesting(A_LEA,0,a)){
                        this.astRoot = null;
                        if ((FL_R & trc) != 0){Trc.out.printf("generateBalanced -8-\n");}
                        return;
                    }
                    getLeaf(l);
                    l.lev = a.lev+1;
                    l.fat = a;
                    l.gendata = new GenData(1,null,null,0);
                    //l.getIcon = newAstIcon;
                    this.astTab.add(l);
                    a.son = l;
                    if ((FL_G & trc) != 0){Trc.out.printf("remainder %s added to %s\n",a.seq,ast.seq);}
                    growable.remove(r);
                    if (growable.size() == 0) break;
                }
            }
        }
        if ((FL_R & trc) != 0){Trc.out.printf("generateBalanced done\n");}
    }

    /**
     * Partition the specified value spreading it over the elements of the specified
     * array ensuring a minimum value to each element so that the sum of the allotted
     * values is close to the specified value.
     *
     * @param      size value to be spread
     * @param      arr return array of spread values
     * @param      min minimum value
     */

    private void partQuantity(int size, int[] arr, int min){
        if ((FL_C & trc) != 0){Trc.out.printf("partQuantity size %s arr %s min %s\n",size,Arrays.toString(arr),min);}
        int q = size;
        int[] orig = Arrays.copyOf(arr,arr.length);
        int nzero = arr.length;
        // try not to generate zero pieces
        if (min * arr.length > size) min = (int)Math.floor(size/arr.length);
        for (int i = 0; i < arr.length; i++){
            arr[i] = min;
        }
        size -= nzero * min;
        if (nzero == 0){
            System.out.printf("partQuantity error %s %s %s %s %s\n",
                q,Arrays.toString(arr),Arrays.toString(orig),min,size);
            System.exit(1);
            return;
        }
        int chunk = (int)Math.floor(size/nzero);
        if (chunk == 0) chunk = 1;
        if ((FL_C & trc) != 0){Trc.out.printf("spread chunk %s size %s nzero %s\n",chunk,size,nzero);}
        int j = 0;
        while (size > 0){
            int r = getRandom(nzero);
            arr[r] += chunk;
            if ((FL_C & trc) != 0){Trc.out.printf("spread chunk added %s to index %s: %s\n",chunk,r,arr[r]);}
            size -= chunk;
            if (size < chunk) break;
        }
        // try now spreading the size orderly on each element
        if ((FL_C & trc) != 0){Trc.out.printf("partQuantity again size %s arr %s\n",size,Arrays.toString(arr));}
        ag: for (;;){
            for (int i = 0; i < arr.length; i++){
                if (size == 0) break ag;
                arr[i]++;
                if ((FL_C & trc) != 0){Trc.out.printf("spread added %s to index %s: %s\n",1,i,arr[i]);}
                size--;
            }
        }
        if ((FL_C & trc) != 0){Trc.out.printf("partQuantity done arr %s\n",Arrays.toString(arr));}
        int n = 0;
        for (int i = 0; i < arr.length; i++){
            n += arr[i];
        }
        if (n != q){
            System.out.printf("partQuantity error %s %s %s %s %s\n",
                q,Arrays.toString(arr),Arrays.toString(orig),min,size);
            System.exit(1);
        }
    }

    /**
     * Decompose the specified value in the specified prime factors.
     *
     * @param      n value to be factorized
     * @param      primes allowed prime factors
     * @return     array of the exponents of each prime factor
     */

    private int[] decompose(int n, int[] primes){
        return decompose(n,primes,Integer.MAX_VALUE);
    }

    /**
     * Decompose the specified value in the specified prime factors up to the maximum specified.
     *
     * @param      n value to be factorized
     * @param      primes prime factors
     * @param      maxBase maximum prime factor allowed 
     * @return     array of the exponents of each prime factor
     */

    private int[] decompose(int n, int[] primes, int maxBase){
        if ((FL_H & trc) != 0){Trc.out.printf("decompose %s\n",n);}
        int nr = n;
        int[] res = null;
        doit: {
            if (n == 0) break doit;
            res = new int[primes[primes.length-1] + 1];
            if (n == 1){
                res[1] = 1;
                break doit;
            }
            for (int i = 0; i < primes.length; i++){
                int base = primes[i];
                if (base > maxBase) break;
                for (;;){
                    if ((n % base) != 0) break;
                    res[base]++;
                    n /= base;
                }
            }
            if (n > 1){
                // impossible to decompose
                res = null;
                break doit;
            }
        } // doit
        if ((FL_H & this.trc) != 0){
            String str = "";
            if (res != null){
                for (int i = 0; i < res.length; i++){
                    if (res[i] != 0) str += " " + i + "^" + res[i];
                }
            }
            Trc.out.printf("decompose %s: %s\n",nr,str);
        }
        return res;
    }
    
    /**
     * Randomly change the specified factoring converting a couple of prime factors
     * into a non-prime one.
     *
     * @param      n value factorized
     * @param      exponents of the prime factors (changed upon return)
     * @param      maxBase maximum prime factor allowed 
     */

    private void randomFactoring(int n, int[] exponents, int maxBase){
        if ((FL_H & trc) != 0){Trc.out.printf("randomFactoring %s\n",n);}
        doit: if (exponents != null){
            // build the list of bases among which to choose two to collapse
            LinkedList<Integer> chooser = new LinkedList<Integer>();
            for (int i = 0; i < exponents.length; i++){
                if (exponents[i] != 0){
                    for (int j = 0; j < exponents[i]; j++){
                        chooser.add(i);
                    }
                }
            }
            if ((FL_H & trc) != 0){Trc.out.printf("exponents %s chooser %s\n",exponents,chooser);}
            // half of the times collapse
            if (randomNumber.get() < 0.5 && chooser.size() > 1){
                LinkedList<Integer> save = new LinkedList<Integer>(chooser);
                for (int i = 0; i < 10; i++){                     // try a reasonable number of times
                    chooser = new LinkedList<Integer>(save);     // use a temporary copy
                    int a = getRandom(chooser.size());
                    int b1 = chooser.get(a);
                    chooser.remove(a);            // remove
                    a = getRandom(chooser.size());
                    int b2 = chooser.get(a);
                    chooser.remove(a);            // remove
                    if (b1*b2 <= maxBase){
                        int newbase = b1*b2;
                        exponents[b1]--;
                        exponents[b2]--;
                        exponents[newbase]++;
                        if ((FL_H & trc) != 0){Trc.out.printf("new base %s new arities %s\n",newbase,exponents);}
                        break;
                    }
                }
            }
        } // doit
        if ((FL_H & this.trc) != 0){
            String str = "";
            if (exponents != null){
                for (int i = 0; i < exponents.length; i++){
                    if (exponents[i] != 0) str += " " + i + "^" + exponents[i];
                }
            }
            Trc.out.printf("randomFactoring %s: %s",n,str);
        }
    }

    /**
     * Deliver a string representing a sequence of factors as bases with exponents.
     *
     * @param      arr factors as pairs
     * @return     string
     */

    private String factorPairsToString(int[] arr){
        String str = "";
        for (int i = 0; i < arr.length; i+= 2){
            if (i > 0) str += " ";
            str += arr[i] + "^" + arr[i+1];
        }
        return str;
    }

    /**
     * Deliver a string representing a sequence of factors as bases with exponents.
     *
     * @param      list factors as pairs
     * @return     string
     */

    private String factorPairsToString(LinkedList<Integer> arr){
        String str = "";
        for (int i = 0; i < arr.size(); i+= 2){
            if (i > 0) str += " ";
            str += arr.get(i) + "^" + arr.get(i+1);
        }
        return str;
    }

    /** The max number of bases for the decomposition of factors for balanced trees.*/
    private static final int MAX_BASES = 19;

    /** The prime numbers for the decomposition of factors for balanced trees.*/
    private static final int[] GEN_PRIMES = {2,3,5,7,11,13,17,19};

    /**
     * Generate an unbalanded AST.
     */

    private void generateUnbalanced(){
        if ((FL_G & trc) != 0){Trc.out.printf("--------- generateUnbalanced ----------------\n");}
        int nr = this.parameters.size;     // number of leaves
        int k1 = this.parameters.depth[0];
        int k2 = this.parameters.depth[1] + 1;
        int depth = 0;
        double f = 0;
        double unRatio = 0;

        int maxArity = INFINITE;
        if (this.parameters.concarity != 0) maxArity = Math.max(maxArity,this.parameters.concarity);
        if (this.parameters.altarity != 0) maxArity = Math.max(maxArity,this.parameters.altarity);

        t: for (int i = 0; i < 10; i++){
            depth = getRandomRange(k1,k2);
            if ((FL_G & trc) != 0){Trc.out.printf("generateUnbalanced deph %s\n",depth);}
            int notunary = getRandomRange(depth/2,depth);
            if (notunary == 0) notunary = 1;
            if ((FL_G & trc) != 0){Trc.out.printf("generateUnbalanced try notunary %s\n",notunary);}
            f = Math.pow(nr,1.0/(double)notunary);      // i.e. notunary+1 to cater for the terminal (last
            int unary = depth - notunary;       // in the progression) and then -1 to get the number
            unRatio = (double)unary/(double)notunary;  // of elements in the progression
            if ((FL_G & trc) != 0){Trc.out.printf("generateUnbalanced new notunary %s unary %s,depth %s unRatio %s f %s\n",notunary,unary,depth,unRatio,f);}
            if (f >= 1) break;               // it cannot be 1
        }
        if ((FL_G & trc) != 0){Trc.out.printf("generateUnbalanced f %s unRatio %s\n",f,unRatio);}
    
        if (this.parameters.concarity == 0) this.parameters.concarity = INFINITE;
        if (this.parameters.altarity == 0) this.parameters.altarity = INFINITE;

        this.astSeq = 0;
        this.cursor = 0;
        this.astRoot = newAstNode(A_CON);
        this.astTab.add(this.astRoot);
        this.astRoot.gendata = new GenData(this.parameters.size,
            this.parameters.depth,f,unRatio);
        //this.astRoot.getIcon = newAstIcon;
        LinkedList<AstNode> buds = new LinkedList<AstNode>();
        buds.add(this.astRoot);
    
        // then grow the tree
        int maxlev = 0;
        for (int g = 0; buds.size() > 0; g++){
            /*
            if (g > 100){
                Trc.out.printf("bau\n");
                break;
            }
            */
            AstNode n = buds.poll();        // get and remove first
            if ((FL_G & trc) != 0){Trc.out.printf(">>>> take node %s %s father %s bole %s\n",n.seq,astKindString(n),n.fat==null?"null":n.fat.seq,n.gendata.bole);}
            // grow it
    
            // choose its arity
            int nsons = 0;
            int[] arr = null;
    
            // try to keep the unary/non-unary ratio
            int nu = 0;                 // number of unary
            int nnu = 0;                // number of non-unary
            if ((FL_U & trc) != 0){Trc.out.printf("node %s count nnu\n",n.seq);}
            for (AstNode a = n.fat; a != null; a = a.fat){
                if (a.kind >= A_GRO){
                    nu++;
                } else {
                    nnu++;
                }
            }
            double curRatio = (n.lev == 0) ? n.gendata.unRatio : (double)nu/(double)nnu;
            boolean prevUnary = false;
            if (n.fat != null && n.fat.kind == A_GRO) prevUnary = true;
            if ((FL_U & trc) != 0){Trc.out.printf("curRatio %s nu %s nnu %s\n",curRatio,nu,nnu);}
    
            if (curRatio < n.gendata.unRatio && !prevUnary){
                nsons = 1;
                if (!randomGroup(n)){
                    this.astRoot = null;
                    return;
                }
                if ((FL_G & trc) != 0){Trc.out.printf("kind n.kind %s\n",astKindString(n));}
                if (n.gendata.bole > 0){
                    arr = new int[]{-n.gendata.nrleaves};
                } else {
                    arr = new int[]{n.gendata.nrleaves};
                }
            } else if (n.gendata.bole > 0){
                if ((FL_G & trc) != 0){Trc.out.printf("bole\n");}
                if (n.gendata.nrleaves == 1){
                    nsons = 1;
                    if (!randomGroup(n)){
                        this.astRoot = null;
                        return;
                    }
                    arr = new int[]{-n.gendata.nrleaves};
                } else {
                    if ((FL_H & trc) != 0){Trc.out.printf("bole n %s lev %s f %s cur %s\n",this.parameters.size,nnu,f,this.parameters.size/Math.pow(f,nnu));}
                    for (int z = 0; z < 10; z++){
                        arr = partitionBole(this.parameters.size/Math.pow(f,nnu),n.gendata.nrleaves,f);
                        if (maxArity != INFINITE){
                            if (arr.length < maxArity) break;
                        } else {
                            break;
                        }
                    }
                    nsons = arr.length;
                    n.kind = A_ALT;
                    if (randomNumber.get() > 0.5) n.kind = A_CON;
                }
            } else {
                int max = this.parameters.concarity == INFINITE ? 0 : this.parameters.concarity;
                if (this.parameters.altarity != INFINITE){
                    if (this.parameters.altarity > max) max = this.parameters.altarity;
                }
                if (max == 0) max = 7;    // not defined, take a max
                nsons = getRandomRange(2,max);
                if (n.gendata.nrleaves == 1) nsons = 1;
                if (nsons > n.gendata.nrleaves) nsons = n.gendata.nrleaves;
                if (nsons == 1){            // then the node is unary
                    if (!randomGroup(n)){
                        this.astRoot = null;
                        return;
                    }
                } else if (nsons > this.parameters.concarity){
                    n.kind = A_ALT;
                } else if (nsons > this.parameters.altarity){
                    n.kind = A_CON;
                } else {
                    n.kind = A_ALT;
                    if (randomNumber.get() > 0.5) n.kind = A_CON;
                    if (!allowedNesting(n,n.fat)){
                        if (n.kind == A_ALT) n.kind = A_CON;
                    }
                }
                if (nsons > 1){
                    arr = partitionNormal(n.gendata.nrleaves,nsons);
                } else {
                    arr = new int[]{n.gendata.nrleaves};
                }
                if (!allowedNesting(n,n.fat)){
                    this.astRoot = null;
                    return;
                }
            }
            if ((FL_G & trc) != 0){Trc.out.printf("partition done arr %s nsons %s\n",Arrays.toString(arr),nsons);}
            // generate sons
            if ((FL_G & trc) != 0){Trc.out.printf("grow node %s lev %s depth %s target leaves %s sons %s\n",n.seq,n.lev,depth,n.gendata.nrleaves,nsons);}
    
            AstNode prev = null;
            for (int i = 0; i < nsons; i++){
                if ((FL_G & trc) != 0){Trc.out.printf("father %s son %s-th leaves %s\n",n.seq,i,arr[i]);}
                int kind = A_LEA;
                AstNode a = newAstNode(kind);
                int l = Math.abs(arr[i]);
                if (l > 1){
                    a.kind = A_CON;
                } else if (arr[i] < 0 && n.lev+1 < depth){
                    a.kind = A_CON;
                } else if (arr[i] > 0 && n.lev == 0){
                    // generate also some leaves at level 2
                    if (randomNumber.get() > 0.5){
                        if (!randomGroup(a)){
                            this.astRoot = null;
                            return;
                        }
                    }
                }
                kind = a.kind;
                if (!allowedNesting(a,n)){
                    this.astRoot = null;
                    return;
                }
                // but it will be set properly after
                if (kind != A_LEA){
                    buds.add(a);
                    if ((FL_G & trc) != 0){Trc.out.printf("push %s\n",a.seq);}
                } else {
                    getLeaf(a);
                    l = 0;
                }
                a.lev = n.lev+1;
                if (a.lev > maxlev) maxlev = a.lev;
                a.fat = n;
                a.gendata = new GenData(l,
                    new int[]{n.gendata.depth[0]-1,n.gendata.depth[1]-1},
                    (arr[i] < 0 ? f : 0),(arr[i] < 0 ? unRatio : unRatio*0.7));
                // a.getIcon = newAstIcon;
                this.astTab.add(a);
                if (i == 0){
                    n.son = a;
                } else {
                    prev.bro = a;
                }
                prev = a;
                if ((FL_G & trc) != 0){Trc.out.printf("father %s son created %s\n",n.seq,a);}
            }
            /*
            for (AstNode z : this.astTab){
                Trc.out.printf("--- %s %s\n",z.seq,astKindString(z));
            }
            for (AstNode z : buds){
                Trc.out.printf("+++ %s %s\n",z.seq,astKindString(z));
            }
            */
        }
        // checkAst(this.astRoot);
        if ((FL_G & this.trc) != 0){
            traceAst(this.astRoot);
        }
        if ((FL_G & trc) != 0){Trc.out.printf("generateUnbalanced done\n");}
    }

    /**
     * Partition the specified number into a (larger) piece that takes the specified ratio.
     * the remaining in smaller pieces.
     *
     * @param      b ....
     * @param      n number
     * @param      f ratio of b
     * @return     return array
     */

    private int[] partitionBole(double b, int n, double f){
        if ((FL_G & trc) != 0){Trc.out.printf("partitionBole b %s n %s f %s\n",b,n,f);}
        int sumn = n;
        LinkedList<Integer> arr = new LinkedList<Integer>();
        doit: {
            int v = -(int)Math.round(b/f);
            if (v == 0 || v == n){
                arr = null;
                break doit;
            }
            arr.add(v);
            n += v;
    
            double max = -v*0.7;
            if (max < 1) max = 1;
            for (; n > 0;){
                double p = Math.min(max,n);
                int m = (int)Math.round(1 + (p-1)*randomNumber.get());
                if (m == 0) break;
                arr.add(m);
                n -= m;
            }
            if ((FL_G & trc) != 0){Trc.out.printf("partitionBole unshuffled %s\n",arr);}
            // shuffle the array
            for (int i = 0; i < arr.size(); i++){
                int j = getRandom(arr.size());
                int tmp = arr.get(j);
                arr.set(j,arr.get(i));
                arr.set(i,tmp);
            }
        } // doit
        if (arr != null){
            int sum = 0;
            for (int i = 0; i < arr.size(); i++) sum += Math.abs(arr.get(i));
            if (sum != sumn){
                throw new RuntimeException("partitionBole failed");
            }
        }
        if ((FL_G & trc) != 0){Trc.out.printf("partitionBole res %s\n",arr);}
        return intListToArray(arr);
    }
    
    /**
     * Partition randomly the specified number into the specified number of pieces.
     *
     * @param      n number
     * @param      p number of pieces
     * @return     return array
     */

    private int[] partitionNormal(int n, int p){
        if ((FL_G & trc) != 0){Trc.out.printf("partitionNormal n %s p %s\n",n,p);}
        LinkedList<Integer> arr = new LinkedList<Integer>();
        doit: {
            for (int i = 0; i < p; i++){
                arr.add(1);
                n--;
            }
            if (n < 0){
                arr = null;
                break doit;
            }
            for (int i = 0; i < p; i++){
                int m = getRandom(n);
                arr.set(i,arr.get(i)+m);
                n -= m;
                if (n == 0) break;
            }
            if (n > 0) arr.set(p-1,arr.get(p-1)+n);
            if ((FL_G & trc) != 0){Trc.out.printf("partitionNormal unshuffled %s\n",arr);}
            // shuffle the array
            // shuffle the array
            for (int i = 0; i < p; i++){
                int j = getRandom(p);
                int tmp = arr.get(j);
                arr.set(j,arr.get(i));
                arr.set(i,tmp);
            }
        } // doit
        if ((FL_G & trc) != 0){Trc.out.printf("partitionNormal res %s\n",arr);}
        return intListToArray(arr);
    }

    // ---------- Generation of texts  -----------------

    /**
     * Deliver a string of as many blanks * 2 as the argument.
     *
     * @param      lev nesting depth
     * @return     string
     */

    private String indent(int lev){
        String str = "";
        for (int i = 0; i < lev; i++){
            if (i == 0){
                str += "   ";
            } else {
                str += "|  ";
            }
        }
        return str;
    }

    /**
     * Visit the specified AST and assign to each node the maximum number of strings
     * that it can generate and the minimum and maximum string lengths.
     *
     * @param      ast AST
     */

    private void stringsNr(AstNode ast){
        if ((FL_G & trc) != 0){Trc.out.printf("stringsNr start %s\n",ast);}
        int maxn = 0;
        int minl = 0;
        int maxl = 0;
        switch (ast.kind){
        case A_LEA:
            ast.maxStrNr = 1;
            ast.minStrLen = 1;
            ast.maxStrLen = 1;
            break;
        case A_EMP:
            ast.maxStrNr = 1;
            ast.minStrLen = 0;
            ast.maxStrLen = 0;
            break;
        case A_NUL:
            ast.maxStrNr = 0;
            ast.minStrLen = 0;
            ast.maxStrLen = 0;
            break;
        case A_ALT:
            maxn = 0;
            minl = Integer.MAX_VALUE;
            maxl = 0;
            for (AstNode a = ast.son; a != null; a = a.bro){
                stringsNr(a);
                if (maxn >= 0){
                    if (a.maxStrNr >= 0){
                        maxn += a.maxStrNr;
                    } else {
                        maxn = -1;
                    }
                }
                if (maxl >= 0){
                    if (a.maxStrLen >= 0){
                        if (a.maxStrLen > maxl) maxl = a.maxStrLen;
                    } else {
                        maxl = -1;
                    }
                }
                if (a.minStrLen < minl) minl = a.minStrLen;
            }
            ast.maxStrNr = maxn;
            ast.minStrLen = minl;
            ast.maxStrLen = maxl;
            break;
        case A_CON:
            maxn = 1;
            minl = 0;
            maxl = 0;
            for (AstNode a = ast.son; a != null; a = a.bro){
                stringsNr(a);
                if (maxn >= 0){
                    if (a.maxStrNr >= 0){
                        maxn *= a.maxStrNr;
                    } else {
                        maxn = -1;
                    }
                }
                if (maxl >= 0){
                    if (a.maxStrLen >= 0){
                        maxl += a.maxStrLen;
                    } else {
                        maxl = -1;
                    }
                }
                minl += a.minStrLen;
            }
            ast.maxStrNr = maxn;
            ast.minStrLen = minl;
            ast.maxStrLen = maxl;
            break;
        case A_GRO:
            stringsNr(ast.son);
            if (ast.upperbound < 0){              // infinite
                ast.maxStrNr = -1;
                ast.maxStrLen = -1;
                if (ast.son.maxStrLen == 0) ast.maxStrLen = 0;
            } else if (ast.upperbound == 0){
                if (ast.son.maxStrLen >= 0){
                    ast.maxStrNr = ast.son.maxStrNr;
                    ast.maxStrLen = ast.lowerbound * ast.son.maxStrLen;
                } else {
                    ast.maxStrNr = -1;
                    ast.maxStrLen = -1;
                }
            } else {
                if (ast.son.maxStrLen >= 0){
                    ast.maxStrNr = ast.son.maxStrNr * (ast.upperbound - ast.lowerbound + 1);
                    ast.maxStrLen = ast.upperbound * ast.son.maxStrLen;
                } else {
                    ast.maxStrNr = -1;
                    ast.maxStrLen = -1;
                }
            }
            ast.minStrLen = ast.lowerbound * ast.son.minStrLen;
            break;
        }
        if ((FL_G & trc) != 0){Trc.out.printf("stringsNr end %s\n",ast);}
    }

    /**
     * Trace the specified AST as a tree showing the number of strings, their length
     * and the repetition of groups to be generated.
     *
     * @param      ast AST
     */

    private void traceAstTree(AstNode ast){
        for (AstNode a = ast; a != null; a = a.bro){
            String s = astKindString(a);
            if (a.kind == A_LEA){
                s = a.sym.toString();
            }
            Trc.out.printf("%s%s %s len: min %s max %s |L|: %s togen l: %s",
                indent(a.lev),a.seq,s,a.minStrLen,a.maxStrLen,a.maxStrNr,
                a.toGenLen);
            if (a.toGenNr > 1){
                Trc.out.printf(" n: %s",a.toGenNr);
            }
            if (a.kind == A_GRO){
                Trc.out.printf(" r: %s",a.toGenRep);
            }
            if (a.genLen > 0) Trc.out.printf(" |str|: %s",a.genLen);
            Trc.out.printf("\n");
            traceAstTree(a.son);
        }
    }

    /**
     * Deliver a string representing the RE rooted in the specified AST with all operators
     * attributed with their budget.
     *
     * @param      ast AST
     * @return     String
     */

    private String toBudgetRE(AstNode ast){
        StringBuilder sb = new StringBuilder();
        toBudgetRE(ast,sb);
        if (sb.length() > 0 && sb.charAt(sb.length()-1) == ' '){
            sb.setLength(sb.length()-1);
        }
        return sb.toString();
    }

    /**
     * Deliver a string representing the RE rooted in the specified AST with all operators
     * attributed with their budget (internal method).
     * Each sub-re is followed by l[,n][*r], where l is the length of the string to
     * generate, n is the number of strings if not 1, and r is the repetition if not 1 and
     * if group.
     *
     * @param      ast AST
     * @param      sb string builder
     */

    private void toBudgetRE(AstNode ast, StringBuilder sb){
        if (ast.kind != A_LEA){          // leaf, remove preceding space
            if (sb.length() > 0 && sb.charAt(sb.length()-1) == ' '){
                sb.setLength(sb.length()-1);
            }
        }
        if (ast.kind == A_LEA){          // leaf
            sb.append(ast.sym.toString());
        } else if (ast.kind == A_ALT){   // alt
            sb.append("(");
            for (AstNode a = ast.son; a != null; a = a.bro){
                if (a != ast.son){
                    if (sb.length() > 0 && sb.charAt(sb.length()-1) == ' '){
                        sb.setLength(sb.length()-1);
                    }
                    sb.append("|");
                }
                toBudgetRE(a,sb);
            }
            if (sb.length() > 0 && sb.charAt(sb.length()-1) == ' '){
                sb.setLength(sb.length()-1);
            }
            sb.append(")");
            sb.append(ast.toGenLen);
            if (ast.toGenNr != 1){
                sb.append(",");
                sb.append(ast.toGenNr);
            }
            sb.append(" ");
        } else if (ast.kind == A_CON){   // conc
            sb.append("(");
            for (AstNode a = ast.son; a != null; a = a.bro){
                toBudgetRE(a,sb);
            }
            if (sb.length() > 0 && sb.charAt(sb.length()-1) == ' '){
                sb.setLength(sb.length()-1);
            }
            sb.append(")");
            sb.append(ast.toGenLen);
            if (ast.toGenNr != 1){
                sb.append(",");
                sb.append(ast.toGenNr);
            }
        } else if (ast.kind == A_EMP){   // empty
            sb.append("\u03b5");
        } else if (ast.kind == A_NUL){   // empty set
            sb.append("\u03a6");
        } else {                         // group
            sb.append("(");
            if (ast.son != null){
                toBudgetRE(ast.son,sb);
                if (sb.length() > 0 && sb.charAt(sb.length()-1) == ' '){
                    sb.setLength(sb.length()-1);
                }
                sb.append(")");
            }
            sb.append(groupSym[ast.groupKind]);
            if (ast.groupKind == G_RE2){
                sb.append("{");
                sb.append(ast.lowerbound);
                if (ast.upperbound < 0){
                    sb.append(",");
                } else if (ast.upperbound > 0){
                    sb.append(",");
                    sb.append(ast.upperbound);
                }
                sb.append("}");
            }
            sb.append(ast.toGenLen);
            if (ast.toGenNr != 1){
                sb.append(",");
                sb.append(ast.toGenNr);
            }
            if (ast.toGenRep != 1){
                sb.append("*");
                sb.append(ast.toGenRep);
            }
            sb.append(" ");
        }
    }

    /**
     * Deliver a string which is a parentetized one (i.e. a linearized form of a tree)
     * showing the substrings of each parenthetized substring at the same nesting level
     * in a dedicated tier.
     *
     * @param      str string
     */

    private String toTiers(String str){
        StringBuilder sb = new StringBuilder();
        sb.append(str);
        sb.append('\n');
        for (;;){
            int lev = 0;
            String newstr = "";
            boolean allsp = true;
            for (int i = 0; i < str.length(); i++){
                char c = str.charAt(i);
                char n = c;
                if (c == '('){
                    lev++;
                }
                if (lev <= 1) n = ' ';
                newstr += n;
                if (n != ' ') allsp = false;
                if (c == ')'){
                    if (lev > 1){
                        // add attributes of group
                        for (int j = i+1; j < str.length(); j++){
                            char a = str.charAt(j);
                            if (",{}*+?0123456789".indexOf(a) < 0){
                                if (j > i+1){
                                    newstr += str.substring(i+1,j);
                                }
                                i = j-1;
                                break;
                            }
                        }
                    }
                    lev--;
                }
            }
            if (allsp) break;
            sb.append(newstr);
            sb.append('\n');
            str = newstr;
        }
        return sb.toString();
    }

    /** A range of values. */

    private static class Range {

        /** The lower bound of the range (included). */
        int lower;

        /** The upper bound of the range (included). */
        int upper;

        /**
         * Construct a range with the specified bounds.
         *
         * @param      l lower bound (included)
         * @param      h upper bound (included)
         */

        public Range(int l, int h){
            this.lower = l;
            this.upper = h;
        }

        /**
         * Deliver a string representing this range.
         *
         * @return  string
         */

        public String toString(){
            return "(" + this.lower + "," + (this.upper==Integer.MAX_VALUE?"*":this.upper) + ")";
        }
    }

    /**
     * Partition the specified value spreading it over the elements of the specified
     * array meeting the allowed range of each element at best effort so that the
     * sum of the allotted values is close to the specified value.
     *
     * @param      n value to be spread
     * @param      ranges array of ranges
     * @param      arr return array of spread values
     */

    /* The algorithm is:
     *  - assign to each slot its minimum
     *  - if sum > n end
     *  - then take what remains and:
     *     - take a random piece and assign it to a random slot that can hold it
     *     - continue until no more remains, or no more slots can take some, in which case
     *       spread one by one over the slots
     *  - the random pieces must be small enough to allow some distribution, otherwise we end up
     *    with a slot that has much more than the others and the others almost empty.
     *    To do this we take pieces that vary randomly around the even value (which it the
     *    current amount to distribute divided by the number of pieces.
     */

    private void partSum(int n, Range[] ranges, int[] arr){
        if ((FL_H & trc) != 0){Trc.out.printf("partSum n %s ranges %s\n",n,Arrays.toString(ranges));}
        int initn = n;
        Arrays.fill(arr,0);
        doit: {
            if (n == 0) break doit;
            int maxspare = 0;
            for (int i = 0; i < arr.length; i++){
                int howmuch = ranges[i].lower;
                if (n < howmuch) howmuch = n;
                arr[i] = howmuch;
                n -= arr[i];
                if (n <= 0) break doit;
                int spare = ranges[i].upper - arr[i];
                if (spare > maxspare) maxspare = spare;
            }
            int chunkl = (int)Math.round(((double)n/ranges.length)*0.9);
            int chunkh = (int)Math.round(((double)n/ranges.length)*1.1);
            if ((FL_H & trc) != 0){Trc.out.printf("partSum min allotted, remains %s chunk %s..%s\n",n,chunkl,chunkh);}
            for (int t = 0;; t++){
                if ((FL_H & trc) != 0){Trc.out.printf("partSum loop n %s arr %s maxspare %s\n",n,Arrays.toString(arr),maxspare);}
                if (maxspare == 0) break doit;
                // assign pieces to slots
                int morsel = getRandomRange(chunkl,chunkh) + 1;
                if (morsel > n) morsel = n;
                if (morsel > maxspare) morsel = maxspare;   // make it the same as the max a slot can get
                int holders = 0;                            // nr of candidate slots
                for (int i = 0; i < arr.length; i++){
                    if (ranges[i].upper - arr[i] >= morsel) holders++;
                }
                int winner = getRandom(holders);
                if ((FL_H & trc) != 0){Trc.out.printf("partSum holders %s winner %s morsel %s\n",holders,winner,morsel);}
                maxspare = 0;
                for (int i = 0; i < arr.length; i++){
                    if (ranges[i].upper - arr[i] >= morsel){
                        if (winner == 0){
                            arr[i] += morsel;
                            if ((FL_H & trc) != 0){Trc.out.printf("partSum morsel %s at %s\n",morsel,i);}
                        }
                        winner--;
                    }
                    int spare = ranges[i].upper - arr[i];
                    if (spare > maxspare) maxspare = spare;
                }
                if ((FL_H & trc) != 0){Trc.out.printf("partSum new maxspare %s\n",maxspare);}
                n -= morsel;
                if (n == 0) break doit;
            }
        } // doit
        if (n > 0){          // there is still some more
            if ((FL_H & trc) != 0){Trc.out.printf("partSum spread remaining %s\n",n);}
            // spread it
            last: for (int t = 0;; t++){
                for (int i = 0; i < arr.length; i++){
                    arr[i]++;
                    n--;
                    if (n <= 0) break last;
                }
            }
        }
        if ((FL_H & trc) != 0){Trc.out.printf("partSum %s\n",rangesSumToString(initn,ranges,arr));}
        return;
    }

    /**
     * Deliver a string representing a sequence of ranges with their allotted values.
     *
     * @param      n value to be spread
     * @param      ranges array of ranges
     * @param      arr return array of spread values
     */

    private String rangesSumToString(int n, Range[] ranges, int[] arr){
        String str = "n: " + n;
        for (int i = 0; i < arr.length; i++){
            str += " " + ranges[i].toString() + arr[i];
        }
        return str;
    }

    /**
     * Partition the specified value spreading it over the two elements of the specified
     * array meeting the allowed two ranges and (if specified) altenative ranges for each
     * element at best effort so that the product of the allotted values is close to the
     * specified value.
     *
     * @param      n value to be spread
     * @param      ranges array of ranges
     * @param      altRanges alternative array of ranges
     * @param      arr return array of spread values
     */

    private boolean partProd2(int n, Range[] ranges, Range[] altRanges, int[] arr){
        if ((FL_H & trc) != 0){Trc.out.printf("partProd n %s ranges %s\n",n,Arrays.toString(ranges));}
        Arrays.fill(arr,1);

        // we must compute two values x and y such that x * y = n
        // x = n/y
        // Moreover, suppose that n > 0, then x >= 1 and y >= 1
        // x = n/y
        // x >= 1
        // 1 = n/y
        // 1 <= y <= n
        // but y must also satisfy: l2 <= y <= h2, so let's intersect the two intervals, and
        // then we have two border values for y, yl and yh, which determine an interval for
        // x: xl..xh
        // Since l1 <= x <= h1, we intersect it with xl..xh and then if the intersection is
        // not null we can take a value in it and then determine that of y.
        // Conditions:
        //    n > 0
        //    h1 > 0

        boolean res = false;
        doit: {
            if (n <= 0) break doit;
            int l2 = ranges[1].lower;
            int h2 = ranges[1].upper;
            int yl = Math.max(l2,1);
            if (yl == 0) yl = 1;
            int yh = Math.min(h2,n);
            if ((FL_H & trc) != 0){Trc.out.printf("partProd y intesection %s..%s and %s..%s res %s..%s\n",l2,h2,1,n,yl,yh);}
            if (yh < yl){                    // no intersection
                break doit;
            }
            int l1 = ranges[0].lower;
            int h1 = ranges[0].upper;
            int xyl = (int)Math.round((double)n / (double)yh);
            int xyh = (int)Math.round((double)n / (double)yl);
            int xl = Math.max(l1,xyl);
            if (xl == 0) xl = 1;
            int xh = Math.min(h1,xyh);
            if ((FL_H & trc) != 0){Trc.out.printf("partProd x intesection %s..%s and %s..%s res %s..%s\n",l1,h1,xyl,xyh,xl,xh);}
            if (xh < xl){                    // no intersection
                break doit;
            }

            // then choose a value for x in it, and compute y
            int x = 0;
            int y = 0;
            int delta = INFINITE;
            int bestx = 0;
            for (x = xl; x <= xh; x++){
                y = (int)Math.round((double)n / (double)x);
                if (y == 0) y = 1;
                int d = Math.abs(n-x*y);
                if (d < delta){
                    delta = d;
                    bestx = x;
                }
            }
            x = bestx;
            y = (int)Math.round((double)n / (double)x);
            int besty = y;

            alt: if (altRanges != null){
                for (int i = 0; i < altRanges.length; i++){
                    Range ar = altRanges[i];
                    if (ar.lower <= y && y <= ar.upper) break alt;
                }
                // no alternative ranges matches, try with all the possible x's if there is one
                // that produces an y that matches one such ranges
                for (x = xl; x <= xh; x++){
                    y = (int)Math.round((double)n / (double)x);
                    if (y == 0) y = 1;
                    for (int i = 0; i < altRanges.length; i++){
                        Range ar = altRanges[i];
                        if (ar.lower <= y && y <= ar.upper) break alt;
                    }
                }
                // no one, then use the computed bestx and besty
                x = bestx;
                y = besty;
            }
            arr[0] = x;
            arr[1] = y;
            res = true;
        } // doit
        if ((FL_H & trc) != 0){Trc.out.printf("partProd %s\n",rangesSumToString(n,ranges,arr));}
        return res;
    }

    /**
     * Deliver a random text from the current AST with approx the specified length.
     *
     * @param      l length
     * @return     string of the text
     */

    private String genRandomText(int l){
        if ((FL_G & trc) != 0){Trc.out.printf("genRandomText\n");}
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < this.astMap.length; i++){
            this.astMap[i].genLen = 0;
        }
        genText(this.astRoot,l,sb);
        if ((FL_G & trc) != 0){Trc.out.printf("genRandomText res %s\n",sb.toString());}
        if (((FL_G|FL_L) & this.trc) != 0){
            traceAstTree(this.astRoot);
        }
        return sb.toString();
    }

    /**
     * Deliver a random text from the specified AST node with approx the specified length
     * appending the text to the specified string buffer.
     *
     * @param      ast root of the AST subtree
     * @param      l length
     * @param      sb returned string
     */

    /* It chooses a random alternative among the ones that meet the length, or the one
     * that is closest to it.
     * It chooses a random number of repetitions and body length for groups. Note that this
     * is done each time a group is entered, which can occur several times for groups that
     * are nested in others, resulting in different, random choices.
     */

    private void genText(AstNode ast, int l, StringBuilder sb){
        if ((FL_G & trc) != 0){Trc.out.printf("%sgenText ast %s %s length %s\n",indent(ast.lev),ast.seq,astKindString(ast),l);}
        int initlen = sb.length();
        ast.toGenLen = l;
        if (l > 3){
            if ((l < ast.minStrLen && (ast.minStrLen-l)/(double)l > 0.3) ||
                (ast.maxStrLen >= 0 && l > ast.maxStrLen && (l-ast.maxStrLen)/(double)l > 0.3)){
            }
        }
        if (l < ast.minStrLen){
            l = ast.minStrLen;
        } else if (l > ast.maxStrLen && ast.maxStrLen >= 0){
            l = ast.maxStrLen;
        }
        if (l != ast.toGenLen){
            if ((FL_G & trc) != 0){Trc.out.printf("%sgenText length normalized %s->%s\n",indent(ast.lev),ast.toGenLen,l);}
            ast.toGenLen = l;
        }
        if (l == 0) return;

        int nsons = 0;
        switch (ast.kind){
        case A_LEA:
            if (ast.sym.kind == S_CHAR){
                sb.append(ast.sym);
            } else {
                // choose a random one
                int n = 0;
                for (int i = 0; i < ast.sym.symset.length; i++){
                    if (ast.sym.symset[i]) n++;
                }
                char[] arr = new char[n];
                n = 0;
                for (int i = 0; i < ast.sym.symset.length; i++){
                    if (ast.sym.symset[i]) arr[n++] = this.parameters.alphabet.charAt(i);
                }
                int r = getRandom(arr.length);
                sb.append(arr[r]);
            }
            break;
        case A_EMP:
        case A_NUL:
            break;
        case A_ALT:
            // here there are several alternatives, do not choose among the
            // ones whose length is out of range
            int na = 0;
            for (AstNode a = ast.son; a != null; a = a.bro){
                if (l < a.minStrLen ||
                    (l > a.maxStrLen && a.maxStrLen >= 0)){
                    continue;
                }
                na++;
            }
            int j = 0;
            if (na == 0){
                // there is no one that meets the desired length, take the one
                // that is closest to it
                if ((FL_G & trc) != 0){Trc.out.printf("%sgenText alt l %s no good sons\n",indent(ast.lev),l);}
                int delta = INFINITE;
                int k = 0;
                for (AstNode a = ast.son; a != null; a = a.bro){
                    if (a.maxStrLen != 0){         // not empty
                        if (l < a.minStrLen){
                            int d = a.minStrLen - l;
                            if (d < delta){
                                delta = d;
                                j = k;
                            }
                        } else if (l > a.maxStrLen && a.maxStrLen >= 0){
                            int d = l - a.maxStrLen;
                            if (d < delta){
                                delta = d;
                                j = k;
                            }
                        }
                    }
                    k++;
                }
                if ((FL_G & trc) != 0){Trc.out.printf("%sgenText alt l %s closest taken %s\n",indent(ast.lev),l,j);}
                for (AstNode a = ast.son; a != null; a = a.bro){
                    if (j == 0){
                        genText(a,l,sb);
                        break;
                    }
                    j--;
                }
            } else {
                j = getRandom(na);
                if ((FL_G & trc) != 0){Trc.out.printf("%sgenText alt l %s chosen %s\n",indent(ast.lev),l,j);}
                for (AstNode a = ast.son; a != null; a = a.bro){
                    if (l < a.minStrLen ||
                        (l > a.maxStrLen && a.maxStrLen >= 0)){
                        continue;
                    }
                    if (j == 0){
                        genText(a,l,sb);
                        break;
                    }
                    j--;
                }
            }
            break;
        case A_CON:
            // partition the budget over sons
            nsons = nrSons(ast);
            Range[] rangesl = new Range[nsons];
            int r = 0;
            for (AstNode a = ast.son; a != null; a = a.bro){
                rangesl[r++] = new Range(a.minStrLen,a.maxStrLen < 0 ? Integer.MAX_VALUE : a.maxStrLen);
            }
            int[] arrl = new int[nsons];
            partSum(l,rangesl,arrl);
            if ((FL_G & trc) != 0){Trc.out.printf("%sgenText conc budget lenghts of sons %s\n",indent(ast.lev),rangesSumToString(l,rangesl,arrl));}
            r = 0;
            for (AstNode a = ast.son; a != null; a = a.bro){
                int ln = arrl[r++];
                genText(a,ln,sb);
            }
            break;
        case A_GRO:
            calcSonLen(l,ast);           // compute the interval of lengths of the son

            int upper = 0;
            if (ast.upperbound < 0){
                upper = INFINITE;
            } else {
                upper = ast.upperbound;
                if (upper == 0){
                    upper = ast.lowerbound;
                } else {
                    upper = getRandomRange(ast.lowerbound,upper+1);
                }
            }
            if (lenlo == 0) lenlo = 1;

            if (ast.groupKind == G_GRO || ast.groupKind == G_OPT){
                int len = getRandomRange(this.lenlo,this.lenhi);
                genText(ast.son,len,sb);
                break;
            }
            int[] arr = null;
            if (ast.groupKind == G_RE2 && ast.upperbound >= 0){
                // with a bounded group there is a need to determine the lengths of the bodies
                // breaking the total length in a number of factors whose sum is the length
                // guaranteeing that each get at least some
                Range[] ranges = new Range[upper];
                Range range = new Range(this.lenlo,this.lenhi);
                for (int i = 0; i < ranges.length; i++){
                    ranges[i] = range;
                }
                arr = new int[upper];
                partSum(l,ranges,arr);
                if ((FL_G & trc) != 0){Trc.out.printf("%sgenText sons budget lengths of sons %s\n",indent(ast.lev),rangesSumToString(l,ranges,arr));}
            }
            // in all the cases in which the group is unbounded, i.e. *, +, {m,}
            // arr is null, and we terminate the loop when the length of the string
            // generated reaches the budget
            int grlen = 0;
            int rem = l;
            for (int i = 0; i < upper; i++){
                int len = 0;
                if (arr == null){
                    if (this.lenlo < rem && rem < this.lenhi){      // last piece
                        len = rem;
                    } else {
                        len = getRandomRange(this.lenlo,this.lenhi);
                    }
                } else {
                    len = arr[i];
                }
                if ((FL_G & trc) != 0){Trc.out.printf("%sgenText iteration %s len %s\n",indent(ast.lev),i,len);}
                genText(ast.son,len,sb);
                grlen = sb.length() - initlen;
                rem = l - grlen;
                if ((FL_G & trc) != 0){Trc.out.printf("%sgenText iteration %s remaining %s grlen %s l %s\n",indent(ast.lev),i,rem,grlen,l);}
                if (arr == null && rem <= 0) break;
            }
            break;
        }
        ast.genLen += sb.length() - initlen;
        if ((FL_G & trc) != 0){Trc.out.printf("%sgenText ast %s %s done length %s genLen %s cur len %s string %s\n",indent(ast.lev),ast.seq,astKindString(ast),l,ast.genLen,sb.length(),sb.substring(initlen));}
    }

    /**
     * Compute the interval of the allowed length taking into account the limits of the
     * son and those of the reptition of the father. Return the lower one in <code>lenlo</code>
     * and the higher one in <code>lenhi</code>.
     *
     * @param      l length
     * @param      ast father node
     */

    private void calcSonLen(int l, AstNode ast){
        int lenlo = 0;
        int lenhi = 0;
        int lower = ast.lowerbound;
        if (lower == 0) lower = 1;        // 0 generates no string, but we come here to have one
        int upper = ast.upperbound;
        if (upper == 0) upper = ast.lowerbound;
        int llower = l/lower;
        if (llower == 0) llower = 1;
        int lupper = l/upper;
        if (lupper == 0) lupper = 1;
        if (ast.upperbound < 0){          // infinite
            if (ast.son.maxStrLen >= 0){
                lenlo = Math.max(ast.son.minStrLen,1);
                lenhi = Math.min(ast.son.maxStrLen,llower);
                if ((FL_G & trc) != 0){Trc.out.printf("%sgenText M:inf-m:n\n",indent(ast.lev));}
            } else {                      // infinite
                lenlo = Math.max(ast.son.minStrLen,1);
                lenhi = Math.min(l,llower);
                if ((FL_G & trc) != 0){Trc.out.printf("%sgenText M:inf-m:inf\n",indent(ast.lev));}
            }
        } else {
            if (ast.son.maxStrLen >= 0){
                lenlo = Math.max(ast.son.minStrLen,lupper);
                lenhi = Math.min(ast.son.maxStrLen,llower);
                if ((FL_G & trc) != 0){Trc.out.printf("%sgenText M:N-m:n\n",indent(ast.lev));}
            } else {                      // infinite
                lenlo = Math.max(ast.son.minStrLen,lupper);
                lenhi = Math.min(l,llower);
                if ((FL_G & trc) != 0){Trc.out.printf("%sgenText M:N-inf\n",indent(ast.lev));}
            }
            if (lenhi < lenlo){
                if ((FL_G & trc) != 0){Trc.out.printf("M:N-m:n no intersection\n");}
                lenlo = ast.son.minStrLen;
                if (lenlo < 0) lenlo = 1;
                lenhi = ast.son.maxStrLen;
                if (lenhi < 0) lenhi = lenlo;
                // System.exit(1);
            }
        }
        if ((FL_G & trc) != 0){Trc.out.printf("%sgenText group {%s,%s}, son limits %s..%s len %s..%s\n",indent(ast.lev),ast.lowerbound,ast.upperbound,ast.son.minStrLen,ast.son.maxStrLen,lenlo,lenhi);}
        this.lenlo = lenlo;
        this.lenhi = lenhi;
    }

    /** Lower bound of the range of the lengths of the son of a group. */
    private int lenlo;

    /** Upper bound of the range of the lengths of the son of a group. */
    private int lenhi;

    // ---------- Generation of collections  -----------------

    /** The actual data of a generated RE AST. */

    private class Actuals {

        /** The number of leaves. */
        int leaves;

        /** The depth (length of the longest path). */
        int depth;

        /** Whether it is balanced. */
        boolean balance;

        /** The maximum number of sons of altenation nodes. */
        int altarity;

        /** The maximum number of sons of concatenation nodes. */
        int conarity;

        /** The minumum m in {m,n} groups. */
        int lowerbound;

        /** The maximum n in {m,n} groups. */
        int upperbound;

        /** The maximum number of groups with iterations > 1 among all paths. */
        int starDepth;

        /** The number of operators for each kind (ordered as ajacences, with {} expanded
         *  into {m}, {m,} {m,n}. */
        int[] nrOperators;

        /**
         * Compute the actual data of the generated AST.
         *
         * @param      actuals return data
         */

        private void calcActuals(){
            int leaves = 0;
            int minAltArity = INFINITE;
            int maxAltArity = 0;
            int minConArity = INFINITE;
            int maxConArity = 0;
            int minIterRange = INFINITE;
            int maxIterRange = 0;
            int maxStarDepth = 0;
            int d = 0;
            this.nrOperators = new int[11];
            for (int i = 0; i < astMap.length; i++){
                AstNode ast = astMap[i];
                if (ast.son == null){
                    leaves++;
                    int s = 0;
                    for (AstNode a = ast; a != null; a = a.fat){
                        if (a.kind == A_GRO && a.groupKind >= G_OPT) s++;
                    }
                    if (s > maxStarDepth) maxStarDepth = s;
                }
                if (ast.kind == A_ALT){
                    int n = 0;
                    for (AstNode s = ast.son; s != null; s = s.bro){
                        n++;
                    }
                    if (n < minAltArity) minAltArity = n;
                    if (n > maxAltArity) maxAltArity = n;
                }
                if (ast.kind == A_CON){
                    int n = 0;
                    for (AstNode s = ast.son; s != null; s = s.bro){
                        n++;
                    }
                    if (n < minConArity) minConArity = n;
                    if (n > maxConArity) maxConArity = n;
                }
                if (ast.kind == A_GRO && ast.groupKind == G_RE2){
                    if (ast.lowerbound < minIterRange) minIterRange = ast.lowerbound;
                    if (ast.upperbound > maxIterRange) maxIterRange = ast.upperbound;
                }
                if ((FL_G & trc) != 0){Trc.out.printf("calcActuals %s lev %s d %s\n",ast.seq,ast.lev,d);}
                if (ast.lev > d) d = ast.lev;

                this.nrOperators[toSeqKind(ast)]++;
            }
            this.leaves = leaves;
            this.depth = d;
            this.balance = parameters.balanced;
            this.altarity = maxAltArity;
            this.conarity = maxConArity;
            this.lowerbound = minIterRange;
            this.upperbound = maxIterRange;
            this.starDepth = maxStarDepth;
        }

        /**
         * Deliver a string representing this object.
         *
         * @return  string
         */

        public String toString(){
            StringBuilder sb = new StringBuilder();
            sb.append("leaves " + this.leaves);
            sb.append(" depth " + this.depth);
            sb.append(" balance " + parameters.balanced);
            sb.append(" altarity " + this.altarity);
            sb.append(" conarity " + this.conarity);
            sb.append(" lowerbound " + this.lowerbound);
            sb.append(" upperbound " + this.upperbound);
            sb.append(" starDepth " + this.starDepth);
            sb.append(" operators");
            for (int a = 0; a < this.nrOperators.length; a++){
                sb.append(String.format(" %s: %s",KIND_LABELS[a],this.nrOperators[a]));
            }
            return sb.toString();
        }
    }

    /** The labels of sequential AST kinds. */
    private static final String[] KIND_LABELS =
        {"epsilon","sigma","|","∙","()","?","*","+","{m}","{m,}","{m,n}"};

    /**
     * Deliver the number of the kind of the specified AST as ordered in KIND_LABELS.
     *
     * @param      ast AST node
     * @return     number
     */

    private int toSeqKind(AstNode ast){
        int kind = toAllowedRow[ast.kind];
        if (ast.kind == A_GRO){
            kind = ast.groupKind + 4;
            if (ast.groupKind == G_RE2){
                if (ast.upperbound < 0){
                    kind++;
                } else {
                    kind += 2;
                }
            }
        }
        return kind;
    }

    /** The parameters of the groups to generate. */

    public static class Groups {

        /** The number of groups. */
        int groups;

        /** The number of elements in each group. */
        int each;

        /** The increment of size from one group to the next. */
        int step;

        /**
         * Deliver a string representing this group.
         *
         * @return  string
         */

        public String toString(){
            return this.groups + " groups " + this.each + " each " + this.step + " step";
        }
    }

    /**
     * Generate a collection of REs and their texts with the specified data and
     * returns the REs and texts calling the specified emitter. The REs are guaranteed
     * to be unique in each group.
     *
     * @param      regr data of groups of REs
     * @param      textgr data of groups of texts
     * @param      out emitter
     */

    /* There are these solutions to generate texts:
     *
     *   1. generate texts with a random length in the desired interval, then place them
     *      in their proper groups (n.b. as groups get close to filled it can become
     *      more and more difficult to fill them).
     *   2. generate a given number of such texts, say 10 times the needed ones, and place
     *      them in their groups, discarding the ones that cannot be placed in their groups
     *      because full.
     *   3. generate texts group by group with a random length in the group range.
     *
     * Here 2. and 3. are implemented. They produce almost the same results.
     * Note that there is no guarantee that each group of REs and of texts is full.
     *
     * To check that each text derives from its corresponding RE, a Berry-Sethi matcher
     * it used. Note that java.regex is not used since it could take a very long amount
     * of time.
     *
     * To measure the ovelapping ones there is a need to check each RE against each other,
     * which takes a lot of time. Once an overlap is found, a RE is not checked with the
     * remaining ones.
     * Instead, to check the equivalence, a RE is checked only with the ones of the same group,
     * which takes much less.
     */



    public void generateCollection(Groups regr, Groups textgr, Emitter out){
        String initTime = new Date().toString();
        long t0 = getCpuTime();
        if ((FL_S & trc) != 0){Trc.out.printf("collection build REs and texts\n");}
        if ((FL_T & trc) != 0){Trc.out.printf("generateCollection REs: %s texts: %s balanced: %s\n",regr,textgr,this.parameters.balanced);}
        randomNumber.reset();
        String[][] res = new String[regr.groups][];            // delivered REs
        String[][][][] texts = new String[regr.groups][][][];  // delivered texts
        Actuals actuals = new Actuals();
        HashSet<String> reSet = new HashSet<String>();         // set of all REs (for duplication)
        LinkedList<String> reList = new LinkedList<String>();  // list of all REs (for overlapping)
        int reNr = 0;                // number of delivered REs
        int reGen = 0;               // number of generated REs
        int reAmbig = 0;             // number of ambiguous REs
        int reEquiv = 0;             // number of weakly equivalent REs
        int reOver = 0;              // number of overlapping REs

        int textNr = 0;              // number of delivered texts
        TextStat tstat = new TextStat();
        long reChars = 0;
        long textChars = 0;
        for (int i = 0; i < regr.groups; i++){
            if ((FL_S & trc) != 0){Trc.out.printf("benchmark RE group %s nr REs %s leaves %s..%s\n",i,regr.each,i*regr.step,(i+1)*regr.step);}
            if ((FL_T & trc) != 0){Trc.out.printf("REs group %s\n",i);}
            res[i] = new String[regr.each];
            texts[i] = new String[regr.each][][];
            boolean filled = false;
            rgr: for (int rg = 0; rg < 10; rg++){             // attempts to fill the group
                for (int j = 0; j < regr.each; j++){          // fill the RE group
                    if ((FL_S & trc) != 0){Trc.out.printf("benchmark RE %s.%s\n",i,j);}
                    if ((FL_T & trc) != 0){Trc.out.printf("REs %s.%s start\n",i,j);}
                    int c = 0;
                    int r = getRandom(regr.step);
                    this.parameters.size = i*regr.step + r;
                    if (this.parameters.size == 0){           // predictable result: and empty RE
                        this.parameters.size = 1;
                    }
                    boolean done = false;
                    String re = null;

                    REgen regex = null;
                    for (int rt = 0; rt < 100; rt++){         // attempts to generate a re
                        if (this.parameters.progress){
                            System.out.printf("RE %s.%s\n",i,j);
                        }
                        generateRE();
                        if (this.astRoot == null){
                            continue;
                        }
                        reGen++;
                        actuals.calcActuals();
                        stringsNr(this.astRoot);
                        c = actuals.leaves;
                        re = this.astRoot.toRE();
                        if (i*regr.step <= c && c < (i+1)*regr.step*1.1){
                            if (!reSet.add(re)){
                            } else {
                                done = true;
                                if (this.astRoot.maxStrLen > 0){
                                    if (this.astRoot.maxStrLen < textgr.groups*textgr.step*0.7){
                                        done = false;
                                    }
                                }
                                if (this.astRoot.maxStrNr > 0 && this.astRoot.maxStrNr < textgr.groups*textgr.each){
                                }

                            }
                        }
                        if (done){
                            regex = new REgen();
                            if (!regex.compileBS(re)){
                                System.out.printf("compile error %s\n",re);
                                Trc.out.printf("compile error %s\n",re);
                                continue;
                            }
                            break;
                        }
                    }
                    if (!done){
                        if (((FL_S|FL_T) & trc) != 0){Trc.out.printf("RE %s.%s redo group\n",i,j);}
                    }

                    res[i][j] = re;
                    this.re = re;
                    if (((FL_S|FL_T) & trc) != 0){Trc.out.printf("RE %s.%s %s done, texts now\n",i,j,re);}

                    if (parameters.texts){
                        String[][] tbdir = new String[textgr.groups][];   // allocate groups dir

                        stringsNr(this.astRoot);
                        generateTextsByGroup(tbdir,i,j,textgr,regex,tstat);
                        texts[i][j] = tbdir;
                    }
                }
                break rgr;
            }

            if (this.parameters.progress){
                System.out.printf("RE group %s done, calculating statistics\n",i);
            }
            // end of a RE group, calculate their numbers
            int regInGroup = 0;
            if (out != null) out.writeREGroup(i);
            for (int j = 0; j < res[i].length; j++){
                if (res[i][j] == null) continue;
                regInGroup++;
                reNr++;
                reChars += res[i][j].length();
                if (out != null) out.writeRE(res[i][j]);

                if (parameters.overlap){
                    // reckon how many produce overlapping languages
                    for (String rex : reList){
                        if (isOverlapping(rex,res[i][j])){
                            reOver++;
                            break;
                        }
                    }
                    reList.add(res[i][j]);
                }

                if (parameters.texts){
                    if (texts[i][j] == null) continue;
                    for (int k = 0; k < texts[i][j].length; k++){
                        if (texts[i][j][k] == null) continue;
                        if (out != null) out.writeTextGroup(k);
                        for (int l = 0; l < texts[i][j][k].length; l++){
                            if (texts[i][j][k][l] == null) continue;
                            textNr++;
                            textChars += texts[i][j][k][l].length();
                            if (out != null) out.writeText(texts[i][j][k][l]);
                        }
                    }
                }
            }
            if ((FL_T & trc) != 0){Trc.out.printf("RE group %s done, %s REs in it\n",i,regInGroup);}

            if (parameters.texts){
                // calculate the nr of texts in each group across all REs
                for (int j = 0; j < texts[i].length; j++){
                    String[][] tgroups = texts[i][j];
                    if (tgroups == null) continue;
                    for (int k = 0; k < tgroups.length; k++){
                        String[] tgroup = tgroups[k];
                        if (tgroup == null) continue;
                        int n = 0;
                        for (int l = 0; l < tgroup.length; l++){
                            String text = tgroup[l];
                            if (text == null) continue;
                            n++;
                        }
                    }
                }
            }

            if (parameters.weakEq){
                // reckon how many are weakly equivalent
                // with this it compiles and complements them only once
                REgen[] rearr = new REgen[res[i].length];
                BoolSet[] leavesSet = new BoolSet[res[i].length];
                boolean[] complemented = new boolean[res[i].length];
                for (int j = 0; j < res[i].length; j++){
                    if (res[i][j] == null) continue;
                    REgen reg1 = new REgen();
                    String r1 = res[i][j];
                    if (!reg1.compileBS(r1)){
                        System.out.printf("compile error\n");
                        continue;
                    }
                    rearr[j] = reg1;
                    leavesSet[j] = new BoolSet();
                    reg1.getLeavesSet(reg1.astRoot,leavesSet[j]);
                    for (int k = 0; k < j; k++){
                        if (!leavesSet[j].equals(leavesSet[k])) continue;
                        if (!complemented[k]){
                            rearr[k].dfaComplement();
                        }
                        BStateTable prod = dfaProduct(rearr[j],rearr[k]);
                        // then find if there is at least a path from the initial state to the final ones
                        boolean b = findPathToFinal(prod);
                        if (!b){
                            reEquiv++;
                            break;
                        }
                    }
                }
            }
            if (this.parameters.progress){
                System.out.printf("RE group %s done\n",i);
            }
            // Don't free space: RE and strings are still needed for serialization.
            //res[i] = null;   // free space
            //texts[i] = null; // free space
        }
        Trc.out.printf("collection: %s, RE: %s, texts: %s\n",
            parameters.balanced?"balanced":"unbalanced",regr,textgr);
        Trc.out.printf("%s REs (should be %s) %s %%, %s generated %s discarded\n",
            reNr,regr.groups*regr.each,((double)reNr/(regr.groups*regr.each))*100.0,
            reGen,reGen-reNr);
        if (parameters.texts){
            Trc.out.printf("%s texts (should be %s) %s %%, %s generated %s discarded\n",
                textNr,regr.groups*regr.each*textgr.groups*textgr.each,
                ((double)textNr/(regr.groups*regr.each*textgr.groups*textgr.each))*100.0,
                tstat.textGen,tstat.textGen-textNr);
        }
        Trc.out.printf("RE equivalent %s\n",reEquiv);
        Trc.out.printf("RE overlapping %s\n",reOver);
        Trc.out.printf("RE chars %s\n",reChars);

        System.out.printf("%s %s\n",initTime,new Date().toString());
        Trc.out.printf("%s %s\n",initTime,new Date().toString());
        long t1 = getCpuTime();
        System.out.printf("%s s\n",(t1-t0)/1000000000);
        Trc.out.printf("%s s\n",(t1-t0)/1000000000);

        // emit serialized file
        try {
            FileOutputStream fileOut = new FileOutputStream("samples.ser");
            ObjectOutputStream outs = new ObjectOutputStream(fileOut);
            outs.writeObject(res);
            outs.writeObject(texts);
            outs.close();
            fileOut.close();
        } catch(IOException exc){
            System.out.printf("serialization %s\n",exc);
        }
    }

    /** The statistics of texts generation. */

    private class TextStat {

        /** The number of delivered texts. */
        int textNr;

        /** The number of generated texts. */
        int textGen;

        /** The number of generated null texts. */
        int textNull;

        /** The number of random lengths that are out of the current text group. */
        int textOut;

        /** The number of times the current AST produces texts that do not belong to a text group. */
        int textNo;

        /** The number of generated texts that do not belong to the current group. */
        int textOutGr;
    }

    /**
     * Generate the groups of texts of the current AST generating one group at a time.
     *
     * @param      tbdir resulting groups
     * @param      i RE group number
     * @param      j RE number in its group
     * @param      textgr parameters of the text groups
     * @param      regex Berry-Sethi compiled RE
     * @param      tstat statistics of the texts
     */

    /* There is no check that texts are unique because there are many REs that have
     * few texts, and for them duplications are the only way to have several.
     */

    private void generateTextsByGroup(String[][] tbdir, int i, int j,
        Groups textgr, REgen regex, TextStat tstat){
        int tgen = 0;
        int tnull = 0;
        int tout = 0;
        int tgood = 0;
        int tgrredo = 0;
        for (int k = 0; k < tbdir.length; k++){
            if ((FL_S & trc) != 0){Trc.out.printf("benchmark text group %s.%s.%s\n",i,j,k);}
            tbdir[k] = new String[textgr.each];
            tgr: for (int tg = 0; tg < 100; tg++){             // attempts to fill the group
                for (int l = 0; l < tbdir[k].length; l++){
                    if ((FL_S & trc) != 0){Trc.out.printf("benchmark text %s.%s.%s.%s\n",i,j,k,l);}

                    // if the range of the current group is out of
                    // that of the ast, there is no way, leave it empty
                    int lol = Math.max(k*textgr.step,this.astRoot.minStrLen);
                    int hil = this.astRoot.maxStrLen;
                    if (hil < 0) hil = INFINITE;
                    hil = Math.min((k+1)*textgr.step,hil);
                    if ((FL_S & trc) != 0){Trc.out.printf("lol %s.. hil %s ast %s %s \n",lol,hil,this.astRoot.minStrLen,this.astRoot.maxStrLen);}
                    if (hil < lol){  // no intersection
                        if ((FL_S & trc) != 0){Trc.out.printf("AST produces strings out of this group");}
                        tstat.textNo++;
                        continue;
                    }

                    int c = 0;
                    int len = 0;
                    boolean got = false;
                    for (int ln = 0; ln < 10; ln++){
                        len = getRandomRange(lol,hil);
                        if ((FL_S & trc) != 0){Trc.out.printf("re %s len %s lol %s.. hil %s ast %s %s \n",re,len,lol,hil,this.astRoot.minStrLen,this.astRoot.maxStrLen);}
                        // this should be superfluous
                        if (this.astRoot.minStrLen <= len &&
                            (this.astRoot.maxStrLen < 0 || len <= this.astRoot.maxStrLen)){
                            got = true;
                            break;
                        }
                    }
                    if (!got){
                        tstat.textOut++;
                        tgrredo++;
                        len = k*textgr.step;     // take a value anyway
                    }
                    String text = null;
                    for (int tt = 0; tt < 1000; tt++){          // attempts to generate a text
                        if ((FL_S & trc) != 0){Trc.out.printf("benchmark text gen %s.%s.%s.%s\n",i,j,k,l);}
                        text = genRandomText(len);
                        if ((FL_S & trc) != 0){Trc.out.printf("benchmark text gen done %s.%s.%s.%s null %s\n",i,j,k,l,text==null);}
                        tgen++;
                        if (text == null){
                            tnull++;
                            tstat.textNull++;
                            continue;
                        }
                        tstat.textGen++;
                        c = text.length();
                        if (k*textgr.step*0.9 <= c && c < (k+1)*textgr.step*1.1){
                            tgood++;
                        } else {
                            tout++;
                            tstat.textOutGr++;
                            continue;
                        }

                        if (!regex.matchBS(text)){
                            System.out.printf("match error RE %s %s len %s\n",re,text,len);
                            Trc.out.printf("match error RE %s %s len %s\n",re,text,len);
                            continue;
                        }
                        break;
                    }
                    tbdir[k][l] = text;
                    if ((FL_S & trc) != 0){Trc.out.printf("benchmark text %s.%s.%s.%s done\n",i,j,k,l);}
                }
                break tgr;
            } // tgr
        }
        int tn = 0;
        for (int tk = 0; tk < tbdir.length; tk++){
            for (int tl = 0; tl < tbdir[tk].length; tl++){
                if (tbdir[tk][tl] != null) tn++;
            }
        }
        tstat.textNr = tn;
        if ((FL_T & trc) != 0){Trc.out.printf("RE %s.%s texts: %s tgen %s tgood %s tnull %s tout %s tgrredo %s\n",i,j,tn,tgen,tgood,tnull,tout,tgrredo);}
    }

    /**
     * Deliver the CPU time in nanoseconds.
     *
     * @return     CPU time
     */

    private long getCpuTime(){
        ThreadMXBean bean = ManagementFactory.getThreadMXBean();
        return bean.isCurrentThreadCpuTimeSupported() ?
            bean.getCurrentThreadCpuTime() : 0L;
    }

    /**
     * Tell if the specifed REs are weakly equivalent.
     *
     * @param      re1 first RE
     * @param      re2 second RE
     * @return     <code>true</code> if they are, <code>false</code> otherwise
     */

    /* Two REs can be equivalent only if they have the same terminals as leaves, so we
     * first visit an AST and build the set of leaves, and check it with that of the preceding
     * ones.
     * Let's take two sets, q' and q" (which denote a state in the product), and take
     * their edges. Take an edge of q': S-> r' and an exge of q" T->r" such
     * that S intersection T is not empty. Then add an edge: (q',q")-S int T->(r',r"),
     * and repeat it for all the edges of q" that have a nonempty intersection with S.
     * Repeat the same process for all other edges of q'.
     * Note that there is no need to build a new DFA that allows to recognize, the problem
     * with DFAs with sets on the edges is that the sets in all outgoing edeges of a state
     * must be non-overlapping. Having a single character class on each edge allows for this.
     * Moreover, it allows to build a direct access transition table. Here we have intersections
     * of classes in the edges, so the non-overlapping condition is satisfied. Actually, we
     * can put anything on the product edges because what we need is only to find accepting paths.
     */

    private boolean isWeaklyEquiv(String re1, String re2){
        REgen reg1 = new REgen();
        if (!reg1.compileBS(re1)){
            System.out.printf("compile error %s\n",re1);
            return false;
        }
        if ((FL_H & this.trc) != 0){
            Trc.out.printf("isWeaklyEquiv first\n");
            reg1.dfa.trace();
        }
        REgen reg2 = new REgen();
        if (!reg2.compileBS(re2)){
            System.out.printf("compile error %s\n",re2);
            return false;
        }
        reg2.dfaComplement();
        BStateTable prod = dfaProduct(reg1,reg2);
        // then find if there is at least a path from the initial state to the final ones
        boolean res = true;
        if (findPathToFinal(prod)){
            res = false;
        }
        if ((FL_H & trc) != 0){Trc.out.printf("isWeaklyEquiv %s\n",res);}
        return res;
    }

    /**
     * Tell if the specifed REs denote overlapping langueges;
     *
     * @param      re1 first RE
     * @param      re2 second RE
     * @return     <code>true</code> if they are, <code>false</code> otherwise
     */

    private boolean isOverlapping(String re1, String re2){
        REgen reg1 = new REgen();
        if (!reg1.compileBS(re1)){
            System.out.printf("compile error %s\n",re1);
            return false;
        }
        if ((FL_H & this.trc) != 0){
            Trc.out.printf("isWeaklyEquiv first\n");
            reg1.dfa.trace();
        }
        REgen reg2 = new REgen();
        if (!reg2.compileBS(re2)){
            System.out.printf("compile error %s\n",re2);
            return false;
        }
        BoolSet leavesSet = new BoolSet();
        reg1.getLeavesSet(reg1.astRoot,leavesSet);
        reg2.getLeavesSet(reg2.astRoot,leavesSet);
        if (leavesSet.isEmpty()){
            return false;
        }

        BStateTable prod = dfaProduct(reg1,reg2);
        // then find if there is at least a path from the initial state to the final ones
        boolean res = findPathToFinal(prod);
        if ((FL_H & trc) != 0){Trc.out.printf("isOverlapping %s\n",res);}
        return res;
    }

    /**
     * Deliver the set of the leaves of the specified AST, which has been expanded
     * to build a Berry-Seti DFA.
     *
     * @param      ast root of the AST
     * @param      set retur set
     */

    private void getLeavesSet(AstNode ast, BoolSet set){
        if (ast.kind == A_LEA){          // leaf
            set.or(this.symClassTable[ast.sym.csym]);
        } else {
            for (AstNode a = ast.son; a != null; a = a.bro){
                getLeavesSet(a,set);
            }
        }
    }

    /**
     * Replace the current DFA with its complement.
     */

    private void dfaComplement(){
        BStateTable dfa = this.dfa;
        BState catchall = dfa.addState();              // catch-all error state
        catchall.isFinal = true;
        for (int i = 0; i < this.symClassTable.length; i++){
            dfa.addBSEdge(catchall,catchall,i);
        }
        boolean[] compl = new boolean[this.symClassTable.length];
        for (BState h = dfa.head;                      // swap final and
            h != null; h = h.suc){                     // .. intemediate
            if (h == catchall) continue;
            if (!h.isFinal){                           // not a final,
                h.isFinal = true;                      // .. becomes final
            } else {
                h.isFinal = false;                     // no more final
            }
            Arrays.fill(compl,false);
            for (BSTrans ts = h.transList;             // edges for all
                ts != null; ts = ts.next){             // .. symbols not present
                compl[ts.sym] = true;
            }
            for (int i = 0; i < compl.length; i++){
                if (!compl[i]){
                    dfa.addBSEdge(h,catchall,i);
                }
            }
        }
        dfa.table = new BState[dfa.stateNr];
        for (BState s = dfa.head; s != null; s = s.suc){
            dfa.table[s.number] = s;
        }
        if ((FL_H & trc) != 0){Trc.out.printf("complemented\n");}
    }

    /**
     * Deliver the product of the two specified DFAs.
     *
     * @param      dfa1 first DFA
     * @param      dfa2 second DFA
     * @return     product
     */

    private BStateTable dfaProduct(REgen r1, REgen r2){
        if ((FL_H & trc) != 0){Trc.out.printf("dfaProduct\n");}
        BStateTable dfa1 = r1.dfa;
        BStateTable dfa2 = r2.dfa;
        REgen regex = new REgen();
        regex.symClassTable = r1.symClassTable;
        BStateTable prod = regex.new BStateTable();
        int[][] pairs = new int[100][];
        prod.table = new BState[dfa1.stateNr*dfa2.stateNr];
        int p = 0;
        for (BState s1 = dfa1.head; s1 != null; s1 = s1.suc){
            for (BState s2 = dfa2.head; s2 != null; s2 = s2.suc){
                BState s = prod.addState();
                if (s1.isFinal && s2.isFinal) s.isFinal = true;
                if (p >= pairs.length){
                    pairs = Arrays.copyOf(pairs,pairs.length*2);
                }
                pairs[p++] = new int[]{s1.number,s2.number};
                prod.table[s.number] = s;
                if ((FL_H & trc) != 0){Trc.out.printf("dfaProduct state %s (%s,%s)\n",s.number,s1.number,s2.number);}
            }
        }
        if ((FL_H & trc) != 0){Trc.out.printf("dfaProduct %s new states tables %s %s\n",p,dfa1.table.length,dfa2.table.length);}
        for (int i = 0; i < p; i++){
            BState s1 = dfa1.table[pairs[i][0]];
            BState s2 = dfa2.table[pairs[i][1]];
            for (BSTrans t1 = s1.transList; t1 != null; t1 = t1.next){
                for (BSTrans t2 = s2.transList; t2 != null; t2 = t2.next){
                    BoolSet b1 = r1.symClassTable[t1.sym];
                    BoolSet b2 = r2.symClassTable[t2.sym];
                    BoolSet res = new BoolSet();
                    res.assignSet(b1);
                    res.and(b2);
                    if (!res.isEmpty()){
                        int n1 = t1.nextState.number;
                        int n2 = t2.nextState.number;
                        for (int j = 0; j < p; j++){
                            if (pairs[j][0] == n1 && pairs[j][1] == n2){
                                prod.addBSEdge(prod.table[i],prod.table[j],t1.sym);
                                if ((FL_H & trc) != 0){Trc.out.printf("dfaProduct edge %s-%s->%s\n",prod.table[i].number,t1.sym,prod.table[j].number);}
                            }
                        }
                    }
                }
            }
        }
        if ((FL_H & this.trc) != 0){
            Trc.out.printf("dfaProduct first\n");
            dfa1.trace();
            Trc.out.printf("dfaProduct second\n");
            dfa2.trace();
            Trc.out.printf("dfaProduct\n");
            prod.trace();
        }
        return prod;
    }

    /**
     * Tell if there is a path from the initial state to a final state of the specified DFA.
     *
     * @param      dfa DFA
     * @return     <code>true</code> if there is, <code>false</code> otherwise
     */

    private boolean findPathToFinal(BStateTable dfa){
        boolean[] visited = new boolean[dfa.stateNr];
        return findPathToFinal(dfa.head,visited);
    }

    /**
     * Tell if there is a path from the initial state to a final state of the specified DFA
     * (internal method).
     *
     * @param      dfa DFA
     * @param      visited array of visited nodes
     * @return     <code>true</code> if there is, <code>false</code> otherwise
     */

    private boolean findPathToFinal(BState s, boolean[] visited){
        if ((FL_H & trc) != 0){Trc.out.printf("findPathToFinal %s\n",s);}
        visited[s.number] = true;
        if (s.isFinal){
            if ((FL_H & trc) != 0){Trc.out.printf("findPathToFinal found\n");}
            return true;
        }
        for (BSTrans t = s.transList; t != null; t = t.next){
            BState w = t.nextState;
            if (!visited[w.number]){            // not yet visited
                if (findPathToFinal(w,visited)){
                    if ((FL_H & trc) != 0){Trc.out.printf("findPathToFinal found\n");}
                    return true;
                }
            }
        }
        return false;
    }

    /** A writer for REs and texts. */

    public static class Emitter {

        /**
         * Emit the specified RE group.
         *
         * @param      n number of group
         */

        public void writeREGroup(int n){
            Trc.out.printf("RE group: %s\n",n);
        }

        /**
         * Emit the specified RE.
         *
         * @param      re re
         */

        public void writeRE(String re){
            Trc.out.printf("%s\n",re);
        }

        /**
         * Emit the specified text group.
         *
         * @param      n number of group
         */

        public void writeTextGroup(int n){
            Trc.out.printf("text group: %s\n",n);
        }

        /**
         * Emit the specified text.
         *
         * @param      text text
         */

        public void writeText(String text){
            Trc.out.printf("%s\n",text);
        }
    }


    // ---------- Berry-Sethi DFA ---------------

    /* This is a Berry-Sethi recognizer.
     * It supports bounded repetition groups (i.e. r{m,n}) and character sets (e.g. [a-z]).
     * Bounded repetition groups are handled by replacing them with a sequence of m bodies,
     * followed by a body* if n is undefined, or n-m body? otherwise.
     * This is so because the construction of a Berry-Seti DFA requires to compute the
     * digrams of marked symbols, and there is no other way of having the proper number of
     * marked symbols in bounded groups.
     * This change is done by changind ASTs once they are first created.
     * Character sets are supported taking the ones that are present on the AST leaves and
     * computing the subsets (character classes) that allow to represent each leaf as a set
     * of them.
     * Actually, such leaves are changed into alternatives of single character classes.
     * This is so because of the need to compute digrams, which are made of pairs of elements,
     * terminals in recognizers that do not support character sets, character classes in the
     * ones that do.
     *
     * The DFA does not contain directly accessible transition tables because its intended
     * application is only to validate the generation of REs an texts in collections, so
     * its speed is not an issue provding that it is reasoneably acceptable.
     */

    /** A symbol in the DFA */

    private class BSsymbol {

        /** An array of indexes of ast nodes. */
        int[] arr;

        /**
         * Deliver a string representing this symbol.
         *
         * @return     string
         */

        public String toString(){
            return toString(astMap,this.arr.length);
        }

        /**
         * Deliver a string long at most the specified length, representing this symbol using
         * the specified map of indexes of nodes.
         *
         * @param      astMap map
         * @param      length maximum length
         * @return     string
         */

        public String toString(AstNode[] astMap, int len){
            String str = "";
            if (this.arr != null){
                for (int i = 0; i < len; i++){
                    str += astMap[this.arr[i]].sym;
                }
            }
            return str;
        }

        /**
         * Tell if this symbol is equal to the specified one.
         *
         * @param      other other symbol
         * @return     <code>true</code> if it is equal, <code>false</code> otherwise
         */

        public boolean equals(BSsymbol other){
            return Arrays.equals(this.arr,other.arr);
        }

        /**
         * Tell if this symbol is equal to the specified one.
         *
         * @see        #equals(BSsymbol)
         */

        public boolean equals(Object other){
            return equals((BSsymbol) other);
        }

        /**
         * Return the hashcode for this symbol.
         *
         * @return     hash code value
         */

        public int hashCode(){
            int res = 0;
            for (int i = 0; i < this.arr.length; i++){
                res += arr[i];
            }
            return res;
        }

        /**
         * Compare this symbol with the specified one. It is fit only for sorting because
         * it sorts elements on their encodings.
         *
         * @param      other the symbol to compare
         * @return     &lt; = or &gt; 0 if this symbol precedes, is equal or follows the other
         */

        public int compareTo(BSsymbol other){
            int n = this.arr.length;
            if (other.arr.length < n) n = other.arr.length;
            int i = 0;
            int j = 0;
            while (n-- != 0){
                int c1 = this.arr[i++];
                int c2 = other.arr[j++];
                if (c1 != c2) return c1 - c2;
            }
            return this.arr.length - other.arr.length;
        }

        /**
         * Deliver a new symbol with the specified substring of this one.
         *
         * @param      begin start index of the string (inclusive)
         * @param      end index (exclusive)
         * @return     symbol
         */

        public BSsymbol substring(int begin, int end){
            BSsymbol sym = new BSsymbol();
            if (end < 0) end = this.arr.length + end;
            sym.arr = Arrays.copyOfRange(this.arr,begin,end);
            return sym;
        }

        /**
         * Deliver the length of this symbol.
         *
         * @return     length
         */

        public int length(){
            return this.arr.length;
        }

        /**
         * Deliver the element at the specified index.
         *
         * @param      i index
         * @return     element
         */

        public int eleAt(int i){
            if (i < 0) return this.arr[this.arr.length+i];
            return this.arr[i];
        }

        /**
         * Trace this symbol.
         */

        public void trace(){
            for (int i = 0; i < this.arr.length; i++){
                int ele = this.arr[i];
                Trc.out.printf("%s |%s|\n",i,ele);
            }
        }
    }

    /** The set of character of each character subclass. */
    private BoolSet[] symClassTable;

    /**
     * Build the partition of the character classes that label the leaves of the AST.
     * Each element of the partition is a character subclass.
     */

    private void buildSymClasses(){
        if ((FL_G & trc) != 0){Trc.out.printf("buildClasses\n");}
        if (this.parameters.alphabet.indexOf(EOF) < 0){
            this.parameters.alphabet += EOF;
        }

        BoolSet complement = new BoolSet();
        complement.complement();
        Set<BoolSet> tempSet = new HashSet<BoolSet>();
        Set<BoolSet> classSet = new HashSet<BoolSet>();
        BoolSet curSet = new BoolSet();
        BoolSet inter = new BoolSet();
        int classNr = 0;
        for (int i = 0; i < this.astMap.length; i++){
            AstNode ast = this.astMap[i];
            if (ast.kind != A_LEA) continue;
            if ((FL_G & trc) != 0){Trc.out.printf("buildClasses ast %s\n",ast);}
            if (ast.sym.kind < S_CHAR) continue;
            if (ast.sym.symset != null){
                curSet.assignArr(ast.sym.symset);
            } else {
                curSet.set(this.parameters.alphabet.indexOf(ast.sym.sym),true);
            }
            if ((FL_G & trc) != 0){Trc.out.printf("buildClasses set %s\n",curSet);}
            complement.sub(curSet);                 // update complement
            tempSet.clear();
            Iterator iter = classSet.iterator();
            while (iter.hasNext()){
                BoolSet elem = (BoolSet)(iter.next());
                if ((FL_G & trc) != 0){Trc.out.printf("buildClasses elem of table %s\n",elem);}
                if (curSet.isEmpty()) break;
                if (curSet.equals(elem)){           // already there
                    curSet.clear();
                    if ((FL_G & trc) != 0){Trc.out.printf("buildClasses found\n");}
                    break;
                }
                inter.assignSet(elem);
                inter.and(curSet);
                if (inter.isEmpty()){
                    if ((FL_G & trc) != 0){Trc.out.printf("buildClasses no intersection\n");}
                    continue;                       // no overlap
                }
                if (!inter.equals(elem)){           // not a superset
                    if ((FL_G & trc) != 0){Trc.out.printf("buildClasess inter el %s\n",elem);}
                    elem.sub(inter);                // remove common symbols
                    if ((FL_G & trc) != 0){Trc.out.printf("buildClasses split old %s split new %s\n",elem,inter);}
                    tempSet.add(inter.clone());     // insert in temporary
                }
                curSet.sub(inter);
                if ((FL_G & trc) != 0){Trc.out.printf("buildClasses new current %s\n",curSet);}
            }
            if (!curSet.isEmpty()){                 // add to list
                tempSet.add(curSet.clone());        // insert in temporary
                if ((FL_G & trc) != 0){Trc.out.printf("buildClasses add temp %s\n",curSet);}
            }
            if (!tempSet.isEmpty()){
                if ((FL_G & trc) != 0){Trc.out.printf("buildClass addes to table\n");}
                classSet.addAll(tempSet);
            }
            if ((FL_G & trc) != 0){Trc.out.printf("buildClasses end ast: %s\n",ast.seq);}
        }
        if (!complement.isEmpty()){               // assign a class to the remaining
            classSet.add(new BoolSet());          // placeholder for it, lowest
        }
        this.symClassTable = new BoolSet[classSet.size()];
        classSet.toArray(this.symClassTable);
        Arrays.sort(this.symClassTable);
        if (!complement.isEmpty()){               // class for all unspecified
            this.symClassTable[0] = complement;   // .. symbols always zero
        }
        if ((FL_F & this.trc) != 0){
            Trc.out.printf("buildClasses classes:\n");
            for (int i = 0; i < this.symClassTable.length; i++){
                Trc.out.printf("%s: %s\n",i,this.symClassTable[i]);
            }
        }
        // store then the sets of classes in AST leaves
        for (int i = 0; i < this.astMap.length; i++){
            AstNode ast = this.astMap[i];
            if (ast.kind != A_LEA) continue;
            if (ast.sym.kind < S_CHAR) continue;
            ast.sym.cset = getClassSets(ast.sym.sym,ast.sym.symset);
            ast.sym.kind = S_CSET;
        }

        // generate the map between character and character subclasses.
        this.classMap = new int[this.parameters.alphabet.length()];
        for (int i = 0; i < this.symClassTable.length; i++){
            for (int j = 0; j < this.symClassTable[i].arr.length; j++){
                if (this.symClassTable[i].arr[j]) classMap[j] = i;       // number of subset
            }
        }
        if ((FL_F & this.trc) != 0){
            Trc.out.printf("buildClasses changed ast:\n");
            traceAst(this.astRoot);
        }
    }

    /**
     * Return the set of character subclasses that make up the specified character or
     * character class.
     *
     * @param      sym single character
     * @param      symset character class
     * @return     set of subclasses
     */

    private boolean[] getClassSets(char sym, boolean[] symset){
        boolean[] res = new boolean[this.symClassTable.length];
        BoolSet cur = new BoolSet();
        BoolSet inter = new BoolSet();
        if (symset != null){
            cur.assignArr(symset);
        } else {
            cur.set(this.parameters.alphabet.indexOf(sym),true);
        }
        for (int i = 0;                               // find subclasses which
            i < this.symClassTable.length; i++){      // .. make up the set
            inter.assignSet(this.symClassTable[i]);   // .. of this transition
            inter.and(cur);
            if (!inter.isEmpty()){                    // found a subset component of the current one
                if ((FL_G & trc) != 0){Trc.out.printf("component found: %s\n",i);}
                res[i] = true;
                cur.sub(inter);
                if (cur.isEmpty()) break;
            }
        }
        if ((FL_F & trc) != 0){Trc.out.printf("getClassSets %s: %s\n",symset!=null?symSetToString(symset,true):sym,Arrays.toString(res));}
        return res;
    }

    /** The map that tells the number of character subclass of each character. */
    private int[] classMap;

    /**
     * Deliver a new BSsymbol with the specified element.
     *
     * @param      ele element
     * @return     BSsymbol
     */

    private BSsymbol newBSsymbol(int ele){
        BSsymbol sym = new BSsymbol();
        sym.arr = new int[]{ele};
        return sym;
    }

    /**
     * Deliver a new BSsymbol whose elements are obtained by concatenating the ones contained
     * in the arguments.
     *
     * @param      args BSsymbols, arrays of elements, or elements
     * @return     BSsymbol
     */

    private BSsymbol newBSsymbol(Object... args){
        BSsymbol sym = new BSsymbol();
        sym.arr = new int[0];
        for (Object arg : args){
            if (arg instanceof BSsymbol){
                BSsymbol bs = (BSsymbol)arg;
                int len = sym.arr.length;
                sym.arr = Arrays.copyOf(sym.arr,sym.arr.length+bs.arr.length);
                System.arraycopy(bs.arr,0,sym.arr,len,bs.arr.length);
            } else if (arg instanceof int[]){   // array
                int len = sym.arr.length;
                int[] arr = (int[])arg;
                sym.arr = Arrays.copyOf(sym.arr,sym.arr.length+arr.length);
                System.arraycopy(arr,0,sym.arr,len,arr.length);
            } else {
                sym.arr = Arrays.copyOf(sym.arr,sym.arr.length+1);
                sym.arr[sym.arr.length-1] = (int)arg;
            }
        }
        return sym;
    }

    /**
     * Compute the BS attributes for the subtree rooted in the specified AST node.
     *
     * @param      ast reference to the AST node
     */

    private void computeAstBS(AstNode ast){
        if ((FL_E & trc) != 0){Trc.out.printf("computeAstBS: %s sub-re: %s\n",ast,ast.toRE());}
        switch (ast.kind){
        case A_LEA:                           // terminal
            ast.isNull = false;
            ast.ini = new HashSet<BSsymbol>();
            ast.ini.add(newBSsymbol(ast.seq));
            ast.fin = new HashSet<BSsymbol>();
            ast.fin.add(newBSsymbol(ast.seq));
            ast.dig = new HashSet<BSsymbol>();
            break;
        case A_EMP:                           // empty
            ast.isNull = true;
            ast.ini = new HashSet<BSsymbol>();
            ast.fin = new HashSet<BSsymbol>();
            ast.dig = new HashSet<BSsymbol>();
            break;
        default:
            ast.ini = new HashSet<BSsymbol>();
            ast.fin = new HashSet<BSsymbol>();
            ast.dig = new HashSet<BSsymbol>();
            for (AstNode a = ast.son; a != null; a = a.bro){
                computeAstBS(a);
            }
        }
        switch (ast.kind){
        case A_ALT:                                    // alternative
            if ((FL_E & trc) != 0){Trc.out.printf("computeAstBS: alt start %s\n",ast);}
            ast.isNull = false;
            if (ast.son == null) ast.isNull = true;
            for (AstNode a = ast.son; a != null; a = a.bro){
                if (a.isNull) ast.isNull = true;
                ast.ini.addAll(a.ini);
                ast.fin.addAll(a.fin);
                ast.dig.addAll(a.dig);
            }
            if ((FL_E & trc) != 0){Trc.out.printf("computeAstBS: alt end %s\n",ast);}
            break;
        case A_CON:                                // concatenation
            if ((FL_E & trc) != 0){Trc.out.printf("computeAstBS: conc start %s\n",ast);}
            boolean iniDone = false;
            AstNode lastNotNull = ast.son;         // in case all are null
            HashSet<BSsymbol> digfin = new HashSet<BSsymbol>();
            ast.isNull = true;
            for (AstNode a = ast.son; a != null; a = a.bro){
                if ((FL_E & trc) != 0){Trc.out.printf("computeAstBS: conc ele %s\n",a);}
                if (!a.isNull) lastNotNull = a;
                if (!a.isNull) ast.isNull = false;
                if (!iniDone){
                    ast.ini.addAll(a.ini);
                    if (!a.isNull){
                        iniDone = true;
                    }
                }
                ast.dig.addAll(a.dig);
                if (a != ast.son){                // not first
                    BSsymbol[] iniarr = a.ini.toArray(new BSsymbol[0]);
                    BSsymbol[] finarr = digfin.toArray(new BSsymbol[0]);
                    for (int i = 0; i < finarr.length; i++){
                        for (int j = 0; j < iniarr.length; j++){
                            ast.dig.add(newBSsymbol(finarr[i],iniarr[j]));
                        }
                    }
                }

                // compute the fin of all elements up to and including the current one
                digfin.clear();
                for (AstNode aa = lastNotNull; aa != null; aa = aa.bro){
                    digfin.addAll(aa.fin);
                    if (aa == a) break;
                }
            }
            for (AstNode a = lastNotNull; a != null; a = a.bro){
                ast.fin.addAll(a.fin);       // union of all those of the null tail + last not null
            }
            if ((FL_E & trc) != 0){Trc.out.printf("computeAstBS: conc end\n");}
            break;
        case A_GRO:                               // sub-re
            if ((FL_E & trc) != 0){Trc.out.printf("computeAstBS: group start %s body %s\n",ast,ast.son);}
            if (ast.groupKind == G_RE1 || ast.groupKind == G_GRO ||
                (ast.groupKind == G_RE2 && ast.lowerbound > 0)){
                ast.isNull = ast.son.isNull;
            } else {
                ast.isNull = true;
            }
            ast.ini.clear();
            BSsymbol[] iniarr = ast.son.ini.toArray(new BSsymbol[0]);
            ast.ini.addAll(ast.son.ini);
            ast.dig.addAll(ast.son.dig);

            ast.fin.clear();
            BSsymbol[] finarr = ast.son.fin.toArray(new BSsymbol[0]);
            ast.fin.addAll(ast.son.fin);

            digrams: {
                if (ast.groupKind == G_RE0 || ast.groupKind == G_RE1){
                    for (int i = 0; i < finarr.length; i++){
                        for (int j = 0; j < iniarr.length; j++){
                            ast.dig.add(newBSsymbol(finarr[i],iniarr[j]));
                        }
                    }
                }
            } // digrams
            if ((FL_E & trc) != 0){Trc.out.printf("computeAstBS: group end\n");}
            break;
        }
        if (ast.pos.length == 0){           // top node
            AstNode e = this.eofAst;
            if (ast.isNull){
                ast.ini.add(newBSsymbol(e.seq));
            }
            BSsymbol[] finarr = ast.fin.toArray(new BSsymbol[0]);
            for (int i = 0; i < finarr.length; i++){
                ast.dig.add(newBSsymbol(finarr[i],e.seq));
            }
        }
        if ((FL_E & trc) != 0){Trc.out.printf("computeAstBS done: %s\n",ast);}
    }

    /**
     * Visit the specified AST and replace bounded repetition groups and character sets
     * with their expansion.
     *
     * @param      ast reference to the AST node
     */

    private void expandAst(AstNode ast){
        if ((FL_E & trc) != 0){Trc.out.printf("expandAst %s\n",ast.seq);}
        // postorder visit: build first the bottom ones so that subtrees can be cloned
        for (AstNode a = ast.son; a != null; a = a.bro){
            expandAst(a);
        }
        if (ast.kind == A_GRO && ast.groupKind == G_RE2){
            if ((FL_E & trc) != 0){Trc.out.printf("expandAst RE2 %s %s\n",ast.lowerbound,ast.upperbound);}
            AstNode body = ast.son;
            int lowerbound = ast.lowerbound;
            int upperbound = ast.upperbound;
            ast.kind = A_CON;                           // concatenation of bodies
            AstNode prev = null;
            for (int i = 0; i < lowerbound; i++){       // {m,n} or {m,}
                AstNode a = (AstNode)body.clone();      // generate m bodies
                a.bro = null;
                a.fat = ast;
                if (prev == null){
                    ast.son = a;
                } else {
                    prev.bro = a;
                }
                prev = a;
                if ((FL_E & trc) != 0){Trc.out.printf("expandAst RE2 fixed part cloned %s\n",a);}
            }
            if (upperbound < 0){                        // {m,}
                AstNode a = newAstNode(A_GRO);          // generate a body*
                a.groupKind = G_RE0;
                a.lowerbound = 0;
                a.upperbound = -1;
                a.son = (AstNode)body.clone();
                a.son.bro = null;
                a.fat = ast;
                if (prev == null){
                    ast.son = a;
                } else {
                    prev.bro = a;
                }
                if ((FL_E & trc) != 0){Trc.out.printf("expandAst RE2 * cloned %s\n",a);}
            } else if (upperbound != 0){                 //{m,n}
                for (int i = 0; i < upperbound - lowerbound; i++){
                    AstNode a = newAstNode(A_GRO);       // generate n-m body?
                    a.groupKind = G_OPT;
                    a.lowerbound = 0;
                    a.upperbound = -1;
                    a.son = (AstNode)body.clone();
                    a.son.bro = null;
                    a.fat = a;
                    if (prev == null){
                        ast.son = a;
                    } else {
                        prev.bro = a;
                    }
                    prev = a;
                    if ((FL_E & trc) != 0){Trc.out.printf("expandAst RE2 optional part cloned %s\n",a);}
                }
            }
        }

        // expand terminal sets
        if (ast.kind == A_LEA && ast.sym.kind == S_CSET){
            int n = 0;
            for (int i = 0; i < ast.sym.cset.length; i++){
                if (ast.sym.cset[i]) n++;
            }
            if ((FL_E & trc) != 0){Trc.out.printf("expandAst set %s set len %s n %s\n",ast,ast.sym.cset.length,n);}
            if (n > 1){
                boolean[] cset = ast.sym.cset;
                ast.kind = A_ALT;
                AstNode prev = null;
                for (int i = 0; i < ast.sym.cset.length; i++){
                    if (ast.sym.cset[i]){
                        AstNode a = newAstNode(A_LEA);
                        a.sym = newSymbolClass(i);
                        if (prev == null){
                            ast.son = a;
                        } else {
                            prev.bro = a;
                        }
                        prev = a;
                    }
                }
            } else {
                for (int i = 0; i < ast.sym.cset.length; i++){
                    if (ast.sym.cset[i]){
                        ast.sym = newSymbolClass(i);
                        break;
                    }
                }
            }
        }
        if ((FL_E & trc) != 0){Trc.out.printf("expandAst %s done\n",ast.seq);}
    }

    /** An edge (transition) of the BS DFA. */

    private static class BSTrans {

        /** The reference to the next transition. */
        BSTrans next;

        /** The reference to the next state (endpoint of this transition). */
        BState nextState;

        /** The symbol (character class number) that labels the arc. */
        int sym;
    }

    /** A state of the BS DFA. */

    private class BState {

        /** The reference to the next state in the list of states. */
        BState suc;

        /** The state number. */
        int number;

        /** The head of the list of transitions. */
        BSTrans transList;

        /** The items. */
        int[] items;

        /** Whether this state is final. */
        boolean isFinal;

        /**
         * Tell if this state contains the same items as the specified one.
         *
         * @param      other the other state
         * @return     <code>true</code> if it does, <code>false</code> otherwise
         */

        private boolean equals(BState other){
            if (this == other) return true;
            if (other == null) return false;
            return this.equals(other.items,other.items.length);
        }

        /**
         * Tell if this state contains the same items as the ones in the specified array.
         *
         * @param      other the array of items
         * @param      len the number of significant entries in the array
         * @return     <code>true</code> if it does, <code>false</code> otherwise
         */

        private boolean equals(int[] other, int len){
            int[] items = other;
            if (this.items.length != len){
                return false;
            }
            for (int i = 0; i < len; i++){
                if (this.items[i] != items[i]){
                    return false;
                }
            }
            return true;
        }

        /**
         * Deliver a string representing this state.
         *
         * @return     string
         */

        public String toString(){
            return this.number + (this.isFinal ? "f": "");
        }
    }

    /**
     * Create a new BS state with the specified number and items.
     *
     * @param      n state number
     * @param      items array of items
     * @param      len the number of significant entries in the array
     * @return     reference to the created state
     */

    private BState newBState(int n, int[] items, int len){
        BState s = new BState();
        s.number = n;
        if (items != null){
            s.items = Arrays.copyOfRange(items,0,len);
        }
        return s;
    }

    /** A state table for the BS DFA. */

    private class BStateTable {

        /** The head of the list of states. */
        BState head;

        /** The tail of the list. */
        BState last;

        /** The last state added. */
        BState lastAdded;

        /** The number of states. */
        int stateNr;

        /** The table of states. */
        BState[] table;

        /** The set of items of the initial state. */
        Set<BSsymbol> initSet; 

        /** The set of items that follow a terminal. */
        Set<BSsymbol> followSet;

        /**
         * Search the state containing the specified items.
         *
         * @param      items array of items
         * @param      len number of significant entries in the array
         * @return     reference to the state, null if no such state
         */

        private BState search(int[] items, int len){
            for (BState h = this.head; h != null; h = h.suc){
                if (h.equals(items,len)) return h;
            }
            return null;
        }

        /**
         * Add a new state.
         *
         * @return     state
         */

        private BState addState(){
            BState h = newBState(this.stateNr++,null,0);
            if (this.last == null) this.head = h;     // append to list
            else this.last.suc = h;
            this.last = h;
            return h;
        }

        /**
         * Add a state with the specified items, if not present.
         *
         * @param      items array of items
         * @param      len number of significant entries in the array
         * @return     <code>true</code> if the state is added, <code>false</code> if already present
         */

        private boolean addUnique(int[] items, int len){
            if ((FL_D & trc) != 0){Trc.out.printf("addUnique\n");}
            BState h = this.search(items,len);
            this.lastAdded = h;
            if (h != null){                           // found
                if ((FL_D & trc) != 0){Trc.out.printf("addUnique found\n");}
                return false;
            }
            h = newBState(this.stateNr++,items,len);  // allocate entry
            if (this.last == null) this.head = h;     // append to list
            else this.last.suc = h;
            this.last = h;
            this.lastAdded = h;
            if ((FL_D & trc) != 0){Trc.out.printf("addUnique added\n");}
            return true;
        }

        /**
         * Deliver a string representing the specified item.
         *
         * @param      item item
         * @return     string
         */

        private String bsItemToString(int item){
            StringBuilder st = new StringBuilder();
            st.append("[");
            st.append(astMap[item].sym);
            st.append("]");
            return st.toString();
        }

        /**
         * deliver a string representation of the specified state.
         *
         * @param      s reference to the state
         * @return     string
         */

        private String stateToString(BState s){
            StringBuilder st = new StringBuilder();
            st.append(s.number);
            if (s.isFinal) st.append(" final");
            st.append(": ");
            for (int i = 0; i < s.items.length; i++){
                if (i > 0) st.append(", ");
                st.append(bsItemToString(s.items[i]));
            }
            for (BSTrans t = s.transList; t != null; t = t.next){
                st.append(" ");
                st.append(symClassTable[t.sym]);
                st.append("->");
                st.append(t.nextState.number);
            }
            return st.toString();
        }

        /**
         * Trace the specified state.
         *
         * @param      s reference to the state
         * @return     string 
         */

        private void traceState(BState s){
            Trc.out.printf("state: %s %s\n",s.number,
                s.isFinal ? "final" : "");
            if (s.items != null){
                Trc.out.printf("  items:\n");
                for (int j = 0; j < s.items.length; j++){
                    Trc.out.printf("    %s: %s\n",j,bsItemToString(s.items[j]));
                }
            }
            Trc.out.printf("  transitions:\n");
            for (BSTrans t = s.transList; t != null; t = t.next){
                Trc.out.printf("    %s -> %s\n",symClassTable[t.sym],t.nextState.number);
            }
        }

        /**
         * Trace this table of states.
         */

        private void trace(){
            for (BState s = this.head; s != null; s = s.suc){
                traceState(s);
            }
        }

        /** The items of the state currently being built. */
        private int[] bsitems; 

        /** Their number. */
        private int itmNr;

        /**
         * Add an item with the specified symbol and iids to the current items, if not present.
         *
         * @param      sym ymbol
         * @return     index of the item
         */

        private int addbsItem(int sym){
            int itm = 0;
            int end = this.itmNr;
            int off = -1;
            while ((++off < end) &&          // search duplicates
                this.bsitems[off] != sym);
            if (off < end){                  // already present
                itm = off;
                if ((FL_D & trc) != 0){Trc.out.printf("add already present %s\n",bsItemToString(this.bsitems[itm]));}
                int item = this.bsitems[itm];
            } else {
                if (this.bsitems == null){
                    this.bsitems = new int[10];
                } else if (this.bsitems.length <= this.itmNr){
                    this.bsitems = Arrays.copyOf(this.bsitems,this.itmNr+10);
                }
                this.bsitems[this.itmNr] = sym;
                itm = this.itmNr;
                this.itmNr++;
                if ((FL_D & trc) != 0){Trc.out.printf("added %s\n",bsItemToString(this.bsitems[itm]));}
            }
            return itm;
        }

        /**
         * Add a new state with the items collected so far.
         *
         * @return     reference to the state
         */

        private BState addBState(){
            BState res = null;
            int[] items = Arrays.copyOf(this.bsitems,this.itmNr);
            Arrays.sort(items);
            if (addUnique(items,this.itmNr)){
                int nreof = 0;
                for (int i = 0; i < this.itmNr; i++){
                    if (items[i] == eofAst.seq){
                        this.lastAdded.isFinal = true;
                    }
                }
                if ((FL_D & trc) != 0){Trc.out.printf("added state: %s\n",stateToString(this.lastAdded));}
            } else {
                if ((FL_D & trc) != 0){Trc.out.printf("found state: %s\n",stateToString(this.lastAdded));}
            }
            res = this.lastAdded;
            return res;
        }

        /**
         * Add a transition from the specifed states with the specified symbol to the
         * specified state.
         *
         * @param      from start state
         * @param      to end state
         * @param      sym characte class
         * @return     reference to the created transition
         */

        private BSTrans addBSEdge(BState from, BState to, int sym){
            BSTrans t = null;
            sea: {
                BSTrans pr = null;
                for (t = from.transList; t != null; t = t.next){      // find edge or last
                    if ((t.nextState == to) &&                        // do not insert duplicates
                        (t.sym == sym)){
                        break sea;
                    }
                    pr = t;
                }
                t = new BSTrans();
                t.nextState = to;
                t.sym = sym;
                if (pr == null){                // append
                    from.transList = t;
                } else {
                    pr.next = t;
                }
            }
            return t;
        }
    }

    /** The current BS DFA. */
    private BStateTable dfa;

    /**
     * Compile the specified RE into a BS DFA.
     *
     * @param      re string to be parsed
     * @return     <code>true</code> if successful, <code>false</code> otherwise
     */

    private boolean compileBS(String re){
        if ((FL_D & trc) != 0){Trc.out.printf("compileBS %s\n",re);}
        this.error = false;
        this.re = re;
        this.astSeq = 0;
        this.cursor = 0;
        this.startToken = 0;
        this.curNode = null;
        expression();
        if (this.error || getsym() != -1){    // error or whole re not consumed
            return false;
        }
        AstNode ast = this.curNode;
        this.astRoot = ast;
        setPosAst();
        buildSymClasses();
        expandAst(this.astRoot);
        setPosAst();

        if ((FL_E & this.trc) != 0){
            traceAst(ast);
            for (int i = 0; i < this.astMap.length; i++){
                Trc.out.printf("buildAst astMap[%s] = %s\n",i,this.astMap[i]);
            }
        }
        computeAstBS(this.astRoot);
        this.dfa = new BStateTable();
        dfa.initSet = ast.ini;
        dfa.followSet = ast.dig;
        if ((FL_E & trc) != 0){Trc.out.printf("initSet %s\n",dfa.initSet);}
        if ((FL_E & trc) != 0){Trc.out.printf("followSet %s\n",dfa.followSet);}

        for (BSsymbol s : dfa.initSet){
            dfa.addbsItem(s.eleAt(0));
        }
        dfa.addBState();
        if ((FL_D & this.trc) != 0){
            if ((FL_D & trc) != 0){Trc.out.printf("buildBS initial state:\n");}
            dfa.trace();
        }
        // visit the initial state and the ones generated after it
        // to create all states
        BState cur = dfa.head;                    // build the next ones
        while (cur != null){
            if ((FL_D & trc) != 0){Trc.out.printf("------ processing state: %s\n",cur.number);}
            // determine the transitions to the next states
            for (int c = 0; c < this.symClassTable.length; c++){     // each alpha in Sigma
                if ((FL_D & trc) != 0){Trc.out.printf("buildBS class %s %s\n",c,this.symClassTable[c]);}
                dfa.itmNr = 0;
                for (int i = 0; i < cur.items.length; i++){   // scan its items
                    int itm = cur.items[i];
                    int sym = this.astMap[itm].sym.csym;
                    if (sym != c) continue;
                    if ((FL_D & trc) != 0){Trc.out.printf("item %s: %s symbol class %s\n",i,dfa.bsItemToString(itm),sym);}
                    for (BSsymbol j : dfa.followSet){
                        if (j.eleAt(0) == itm){
                            dfa.addbsItem(j.eleAt(1));
                        }
                    }
                }
                if (dfa.itmNr > 0){            // add the new state
                    if ((FL_D & trc) != 0){Trc.out.printf("buildBS adding next state\n");}
                    BState next = dfa.addBState();
                    if ((FL_D & trc) != 0){Trc.out.printf("buildBS adding edge to it\n");}
                    dfa.addBSEdge(cur,next,c);
                    if ((FL_D & trc) != 0){Trc.out.printf("buildBS adding next done\n");}
                }
            }
            cur = cur.suc;
        }
        dfa.table = new BState[dfa.stateNr];
        for (BState s = dfa.head; s != null; s = s.suc){
            dfa.table[s.number] = s;
        }

        if ((FL_D & this.trc) != 0){
            if ((FL_D & trc) != 0){Trc.out.printf("buildBS DFA: %s\n",ast.toRE());}
            dfa.trace();
        }
        return !this.error;
    }

    /**
     * Match the specified text.
     *
     * @param      text string to be parsed
     * @return     <code>true</code> if successful, <code>false</code> otherwise
     */

    private boolean matchBS(String text){
        this.error = false;
        if ((FL_M & this.trc) != 0){
            if ((FL_M & trc) != 0){Trc.out.printf("matchBS start RE: %s text %s\n",this.astRoot.toRE(),text);}
            this.dfa.trace();
        }
        BState state = this.dfa.head;

        for (int i = 0; i < text.length(); i++){
            if ((FL_M & this.trc) != 0){
                if ((FL_M & trc) != 0){Trc.out.printf("match at: %s sym %s\n",i,text.charAt(i));}
                this.dfa.traceState(state);
            }
            char sym = text.charAt(i);
            if (this.parameters.alphabet.indexOf(sym) < 0){
                this.error = true;
                break;
            }
            BState next = null;
            int cclass = this.classMap[this.parameters.alphabet.indexOf(sym)];
            for (BSTrans t = state.transList; t!= null; t = t.next){
                if (cclass == t.sym){
                    next = t.nextState;
                    break;
                }
            }
            if ((FL_M & trc) != 0){Trc.out.printf("match process %s next %s class %s\n",sym,next==null?"null":next.number,cclass);}
            if (next == null){
                this.error = true;
                break;
            }
            state = next;
            if ((FL_M & trc) != 0){Trc.out.printf("match nextstate %s\n",state.number);}
        }
        if ((FL_M & trc) != 0){Trc.out.printf("match end state %s\n",state.number);}
        if (!state.isFinal){
            this.error = true;
            // not recognized
        }
        if ((FL_M & trc) != 0){Trc.out.printf("matchBS end RE: %s text %s %s\nstates:\n",this.astRoot.toRE(),text,this.error?"failure":"success");}
        return !this.error;
    }


    // ---------- Testing -----------------

    /** The name of the feature under testing. */
    private static String featureName;

    /** The flag to enable the trace of the tests. */
    private static boolean ckshow = false;

    /**
     * Start the testing of a feature. Record its name and trace it
     * if the tracing of tests is enabled.
     *
     * @param      s feature name
     */

    private static void feature(String s){
        featureName = s;
        if (ckshow) Trc.out.printf("%s\n",featureName);
    }

    /**
     * Trace a test of a feature.
     *
     * @param      t number of the test
     */

    private static void showTest(int t){
        if (ckshow) Trc.out.printf("-- test %s %s ---\n",featureName,t);
    }

    /**
     * Report the failure of a test.
     *
     * @param      t number of the test
     */

    private static void trcfail(int t){
        Trc.out.printf("\n");
        Trc.out.printf("-- test %s %s failed---\n",featureName,t);
    }

    /**
     * Deliver the specified string with spaces removed.
     *
     * @param      str string
     * @return     string
     */

    private static String removeSpaces(String str){
        String s = "";
        for (int i = 0; i < str.length(); i++){
            if (str.charAt(i) == ' ') continue;
            s += str.charAt(i);
        }
        return s;
    }

    /**
     * Trace the specified message followed by the expected value and the actual one.
     *
     * @param      str message
     * @param      e expected value
     * @param      a actual value
     * @return     string
     */

    private static void trc(String s, String e, String a){
        Trc.out.printf("%s expected: |%s| actual: |%s|\n",s,e,a);
    }

    /**
     * Trace the specified message followed by the expected value and the actual one.
     *
     * @param      str message
     * @param      e expected value
     * @param      a actual value
     * @return     string
     */

    private static void trc(String s, double e, double a){
        Trc.out.printf("%s expected: |%s| actual: |%s|\n",s,e,a);
    }

    /**
     * Test the generation of the AST of the specified string.
     *
     * @param      t number of the test case
     * @param      re RE
     * @param      exp expected string generated from the AST
     */

    private void testRE(int t, String re, String exp){
        showTest(t);
        buildAst(re);
        String actual = null;
        if (this.error){
            actual = "error";
        } else {
            actual = this.astRoot.toRE();
        }
        if (!actual.equals(exp)){
            trcfail(t);
            trc("text",exp,actual);
        }
    }

    /**
     * Test the calculation of the maximum number of strings of the nodes of the
     * AST of the specified RE.
     *
     * @param      t number of the test case
     * @param      re RE
     * @param      exp expected result
     */

    private void testNrStrings(int t, String re, String exp){
        showTest(t);
        buildAst(re);
        if (this.error){
            trcfail(t);
        }
        stringsNr(this.astRoot);
        String actual = "";
        for (int i = 0; i < this.astMap.length-1; i++){
            if (actual.length() > 0) actual += " ";
            actual += i + ":" + this.astMap[i].maxStrNr;
        }
        if (!actual.equals(exp)){
            trcfail(t);
            trc("nrs",exp,actual);
            traceAst(this.astRoot);
        }
    }

    /**
     * Test the calculation of the lengths of the strings of the nodes of the
     * AST of the specified RE.
     *
     * @param      t number of the test case
     * @param      re RE
     * @param      exp expected result
     */

    private void testLenStrings(int t, String re, String exp){
        showTest(t);
        buildAst(re);
        if (this.error){
            trcfail(t);
        }
        stringsNr(this.astRoot);
        String actual = "";
        for (int i = 0; i < this.astMap.length-1; i++){
            if (actual.length() > 0) actual += " ";
            actual += i + ":" + this.astMap[i].minStrLen + "," + this.astMap[i].maxStrLen;
        }
        if (!actual.equals(exp)){
            trcfail(t);
            trc("lenghts",exp,actual);
            traceAst(this.astRoot);
            traceAstTree(this.astRoot);
        }
    }

    /**
     * Test the partitioning of a number in a sum of factors.
     *
     * @param      t number of the test case
     * @param      n number
     * @param      r ranges
     * @param      exp expected result
     */

    private void testPartSum(int t, int n, Range[] ranges, String exp){
        showTest(t);
        int[] arr = new int[ranges.length];
        partSum(n,ranges,arr);
        String actual = Arrays.toString(arr);
        if (!actual.equals(exp)){
            trcfail(t);
            trc("factors",exp,actual);
        }
        for (int i = 0; i < arr.length; i++){
            n -= arr[i];
        }
        if (n != 0){
            trcfail(t+1000);
        }
    }

    /**
     * Test the partitioning of several a numbers in a sum of factors.
     *
     * @param      t number of the test case
     * @param      r ranges
     */

    private void testPartSumStress(int t, Range[] ranges){
        showTest(t);
        int[] arr = new int[ranges.length];
        for (int i = 0; i < 1000; i++){
            partSum(i,ranges,arr);
            int n = i;
            for (int j = 0; j < arr.length; j++){
                n -= arr[j];
            }
            if (n != 0){
                trcfail(t+1000);
                break;
            }
        }
    }

    /**
     * Test the partitioning of a number in a product of two factors.
     *
     * @param      t number of the test case
     * @param      n number
     * @param      r1 first range
     * @param      r2 second range
     * @param      h3 upperbound second range
     * @param      exp expected result
     */

    private void testPartProd(int t, int n, Range r1, Range r2, String exp){
        showTest(t);
        int[] arr = new int[2];
        Range[] ranges = {r1,r2};
        String actual = "error";
        if (partProd2(n,ranges,null,arr)){
            actual = Arrays.toString(arr);
        }
        if (!actual.equals(exp)){
            trcfail(t);
            trc("factors",exp,actual);
        }
    }

    /**
     * Test the generation of a random text from the specified RE with the specified
     * length.
     *
     * @param      t number of the test case
     * @param      re RE
     * @param      len maximum string length
     * @param      exp expected result
     */

    private void testTextRandom(int t, String re, int len, String exp){
        showTest(t);
        buildAst(re);
        if (this.error){
            trcfail(t);
        }
        stringsNr(this.astRoot);
        randomNumber.test();
        String actual = genRandomText(len);
        if (!actual.equals(exp)){
            trcfail(t);
            trc("text",exp,actual);
            String str = toBudgetRE(this.astRoot);
            traceAstTree(this.astRoot);
            trc("len",len,actual.length());
        }
        REgen regex = new REgen();
        if (!regex.compileBS(re)){
            trcfail(t);
            Trc.out.printf("compile error\n");
        } else if (!regex.matchBS(actual)){
            trcfail(t);
            Trc.out.printf("match error\n");
        }
    }

    /**
     * Test the computation of the lengths of the bodies of groups.
     *
     * @param      t number of the test case
     * @param      l total group length
     * @param      M minimum repetition
     * @param      N maximum repetition
     * @param      m minimum string length of the body
     * @param      n maximum string length of the body
     * @param      expl expected lowerbound
     * @param      exph expected upperbound
     */

    private void testGroupLen(int t, int l, int M, int N, int m, int n, int expl, int exph){
        showTest(t);
        AstNode son = newAstNode(A_EMP);
        son.minStrLen = m;
        son.maxStrLen = n;
        AstNode ast = newAstNode(A_GRO);
        ast.son = son;
        ast.lowerbound = M;
        ast.upperbound = N;
        ast.minStrLen = 0;
        ast.maxStrLen = -1;
        calcSonLen(l,ast);
        if (this.lenlo != expl || this.lenhi != exph){
            trcfail(t);
            trc("low",expl,this.lenlo);
            trc("high",exph,this.lenhi);
        }
    }

    /**
     * Test if the specified two REs are weakly equivalent.
     *
     * @param      t number of the test case
     * @param      re1 first RE
     * @param      re2 second RE
     * @param      exp expected result
     */

    private void testWeaklyEquiv(int t, String re1, String re2, boolean exp){
        showTest(t);
        boolean actual = isWeaklyEquiv(re1,re2);
        if (actual != exp){
            trcfail(t);
        }
    }

    /**
     * Test the expansion of bounded groups and terminals sets.
     *
     * @param      t number of the test case
     * @param      re RE
     * @param      exp expected result
     */

    private void testExpand(int t, String re, String exp){
        showTest(t);
        if (!compile(re)){
            trcfail(t);
            return;
        }
        buildSymClasses();
        expandAst(this.astRoot);
        String actual = this.astRoot.toRE();
        if (!actual.equals(exp)){
            trcfail(t);
            trc("expanded",exp,actual);
            traceAst(this.astRoot);
        }
    }

    /**
     * Test whether the matching of a RE and text with the BS DFA is successful.
     *
     * @param      t number of the test case
     * @param      re RE
     * @param      str text
     */

    private void testMatch(int t, String re, String str){
        testMatch(t,re,str,"");
    }

    /**
     * Test the matching of a RE and text with the BS DFA.
     *
     * @param      t number of the test case
     * @param      re RE
     * @param      str text
     * @param      exp expected result: "bad" if illegal RE, "no" if match failed,
     *             otherwise match successful
     */

    private void testMatch(int t, String re, String str, String exp){
        showTest(t);
        boolean b = compileBS(re);
        if (!b){
            if (!exp.equals("bad")){
                trcfail(t);
            }
            return;
        }
        b = matchBS(str);
        if (b == exp.equals("no")){
            trcfail(t+1000);
        }
    }

    /**
     * Run all the verification tests.
     */

    private void test(){
        ckshow = true;
        this.randomNumber.test();

        feature("RE ast");
        testRE(0,"","");
        testRE(1,"a","a");
        testRE(2,"#","error");
        testRE(3,"[z-a]","error");
        testRE(4,"[a-a]","[a]");
        testRE(5,"[a-c]","[a-c]");
        testRE(6,"[#-a]","error");
        testRE(7,"[a-#]","error");
        testRE(8,"a(bc(d)*)+","a(bc(d)*)+");

        feature("max nr strings");
        testNrStrings(0,"","0:1");
        testNrStrings(1,"a","0:1");
        testNrStrings(2,"ab","0:1 1:1 2:1");
        testNrStrings(3,"ab|c","0:2 1:1 2:1 3:1 4:1");
        testNrStrings(4,"a(b|c)","0:2 1:1 2:2 3:2 4:1 5:1");
        testNrStrings(5,"a*","0:-1 1:1");
        testNrStrings(6,"a?","0:2 1:1");
        testNrStrings(7,"a+","0:-1 1:1");
        testNrStrings(8,"a{2}","0:1 1:1");
        testNrStrings(9,"a{2,3}","0:2 1:1");
        testNrStrings(10,"a{2,}","0:-1 1:1");
        testNrStrings(11,"a{0,}","0:-1 1:1");
        testNrStrings(12,"a(b*|c)","0:-1 1:1 2:-1 3:-1 4:-1 5:1 6:1");
        testNrStrings(13,"a(b|c*)","0:-1 1:1 2:-1 3:-1 4:1 5:-1 6:1");
        testNrStrings(14,"ab*","0:-1 1:1 2:-1 3:1");
        testNrStrings(15,"a*b","0:-1 1:-1 2:1 3:1");
        testNrStrings(16,"a*b*","0:-1 1:-1 2:1 3:-1 4:1");
        testNrStrings(17,"(a|b){2,3}","0:4 1:2 2:1 3:1");

        feature("range of string lengths");
        testLenStrings(0,"","0:0,0");
        testLenStrings(1,"a","0:1,1");
        testLenStrings(2,"ab","0:2,2 1:1,1 2:1,1");
        testLenStrings(3,"ab|c","0:1,2 1:2,2 2:1,1 3:1,1 4:1,1");
        testLenStrings(4,"a(b|c)","0:2,2 1:1,1 2:1,1 3:1,1 4:1,1 5:1,1");
        testLenStrings(5,"a*","0:0,-1 1:1,1");
        testLenStrings(6,"a?","0:0,1 1:1,1");
        testLenStrings(7,"a+","0:1,-1 1:1,1");
        testLenStrings(8,"a{2}","0:2,2 1:1,1");
        testLenStrings(9,"a{2,3}","0:2,3 1:1,1");
        testLenStrings(10,"a{2,}","0:2,-1 1:1,1");
        testLenStrings(11,"a{0,}","0:0,-1 1:1,1");
        testLenStrings(12,"a(b*|c)","0:1,-1 1:1,1 2:0,-1 3:0,-1 4:0,-1 5:1,1 6:1,1");
        testLenStrings(13,"a(b|c*)","0:1,-1 1:1,1 2:0,-1 3:0,-1 4:1,1 5:0,-1 6:1,1");
        testLenStrings(14,"ab*","0:1,-1 1:1,1 2:0,-1 3:1,1");
        testLenStrings(15,"a*b","0:1,-1 1:0,-1 2:1,1 3:1,1");
        testLenStrings(16,"a*b*","0:0,-1 1:0,-1 2:1,1 3:0,-1 4:1,1");
        testLenStrings(17,"(a|b){2,3}","0:2,3 1:1,1 2:1,1 3:1,1");

        feature("partition sum");
        Range[] ranges = new Range[]{
            new Range(0,1),
            new Range(1,3),
            new Range(2,5)
        };
        testPartSum(0,1,ranges,"[0, 1, 0]");
        testPartSum(1,2,ranges,"[0, 1, 1]");
        testPartSum(2,3,ranges,"[0, 1, 2]");
        testPartSum(3,4,ranges,"[1, 1, 2]");
        testPartSum(4,5,ranges,"[0, 3, 2]");
        testPartSum(5,6,ranges,"[0, 1, 5]");
        testPartSum(6,9,ranges,"[1, 3, 5]");
        testPartSum(7,10,ranges,"[2, 3, 5]");
        testPartSum(8,16,ranges,"[4, 5, 7]");
        testPartSumStress(10,ranges);
        ranges = new Range[]{
            new Range(2,5),
            new Range(0,1),
            new Range(1,3),
            new Range(1,8)
        };
        testPartSumStress(11,ranges);

        ranges = new Range[]{
            new Range(1,1),
            new Range(1,1),
            new Range(1,1),
            new Range(2,-1)
        };
        testPartSum(12,6,ranges,"[2, 1, 1, 2]");

        feature("partition product two");
        testPartProd(0,0,new Range(1,1),new Range(1,1),"error");
        testPartProd(1,1,new Range(1,1),new Range(1,1),"[1, 1]");
        testPartProd(2,5,new Range(1,10),new Range(6,8),"error");
        testPartProd(3,20,new Range(1,2),new Range(4,5),"error");
        testPartProd(4,20,new Range(1,4),new Range(2,5),"[4, 5]");
        testPartProd(5,21,new Range(1,4),new Range(2,5),"[4, 5]");
        testPartProd(6,22,new Range(1,4),new Range(2,5),"[4, 6]");
        testPartProd(7,37,new Range(1,9),new Range(2,5),"[9, 4]");

        feature("generate random texts");
        testTextRandom(0,"a",1,"a");
        testTextRandom(1,"ab",2,"ab");
        testTextRandom(2,"a*",4,"aaaa");
        testTextRandom(3,"a(b|cd)",3,"acd");
        testTextRandom(4,"a(b|cd)",4,"acd");
        testTextRandom(5,"a(b|cd)",2,"ab");
        testTextRandom(6,"a(b|cd)",1,"ab");
        testTextRandom(7,"a*b*",1,"a");
        testTextRandom(8,"(a|b)*",3,"aab");
        testTextRandom(9,"(ab)(cde)",5,"abcde");
        testTextRandom(10,"(ab)(cde)",6,"abcde");
        testTextRandom(11,"(ab)(cde)",4,"abcde");
        testTextRandom(12,"a|bc|def",1,"a");
        testTextRandom(13,"a|bc|def",2,"bc");
        testTextRandom(14,"a|bc|def",3,"def");
        testTextRandom(15,"a|bc|def",4,"def");
        testTextRandom(16,"(a|b|c|d|e)+",5,"bcdde");
        testTextRandom(17,"(ab)",2,"ab");
        //   M:inf    m:n        len in m .. n,  intersection with 1..l/M
        //   M:inf    m:inf      len in m .. l,  intersection with 1..l/M
        //   M:N      m:n        len in m .. n,  intersection with l/N .. l/M
        //   M:N      m:inf      len in m .. l,  intersection with l/N .. l/M

        testGroupLen(20,5,  0,-1,  0,3,  1,3);
        testGroupLen(21,5,  0,-1,  1,-1, 1,5);
        testGroupLen(22,12, 0,3,   1,5,  4,5);
        testGroupLen(23,12, 0,3,   1,-1, 4,12);
        testGroupLen(24,12, 2,3,   1,5,  4,5);
        testGroupLen(25,12, 2,3,   1,-1, 4,6);
        testGroupLen(26,5,  1,1,   2,6,  5,5);
        testGroupLen(27,5,  1,1,   2,-1, 5,5);
        testGroupLen(28,5,  0,1,   2,6,  5,5);
        testGroupLen(29,5,  1,1,   2,-1, 5,5);
        testGroupLen(30,5,  2,3,   1,2,  1,2);

        testTextRandom(30,"(ab)+",2,"ab");
        testTextRandom(31,"(a|bc|def)+",2,"aa");
        testTextRandom(32,"((a)*)+",3,"aaa");
        testTextRandom(33,"((a)*b)+",3,"bab");
        testTextRandom(34,"(a|bc){2,3}",5,"bcbc");
settrc("g");
settrc("");
        testTextRandom(35,"(a(b|cd)+)+",6,"ababab");
        testTextRandom(36,"abcd|ab",3,"abcd");
        testTextRandom(37,"abcd|ab",1,"ab");
        testTextRandom(38,"(a){2,}",4,"aaaa");
        testTextRandom(39,"(a){2}",2,"aa");

        feature("weak equivalence");
        testWeaklyEquiv(0,"","",true);
        testWeaklyEquiv(1,"a","a",true);
        testWeaklyEquiv(2,"a","b",false);
        testWeaklyEquiv(3,"a","aa",false);
        testWeaklyEquiv(4,"(a)","a",true);
        testWeaklyEquiv(5,"a|b","b|a",true);
        testWeaklyEquiv(6,"a*","a",false);
        testWeaklyEquiv(7,"a|((b)*)*","a|b*",true);
        testWeaklyEquiv(8,"a{3}","aaa",true);
        testWeaklyEquiv(9,"a{2,3}","aaa|aa",true);
        testWeaklyEquiv(10,"aa|ab","a(a|b)",true);

        feature("expand ast");
        testExpand(0,"","");
        testExpand(1,"a","a");
        testExpand(2,"ab","ab");
        testExpand(3,"a|b","a|b");
        testExpand(4,"(a)","(a)");
        testExpand(5,"a*","(a)*");
        testExpand(6,"(ab){2}","(ab)(ab)");
        testExpand(7,"x(ab){2}","x((ab)(ab))");
        testExpand(8,"(ab){0,2}","(ab)?(ab)?");
        testExpand(9,"(ab){2,4}","(ab)(ab)(ab)?(ab)?");
        testExpand(10,"(ab){2,}","(ab)(ab)(ab)*");
        testExpand(11,"a|bc|(d)|e(f){2,3}|g(h(i){3}){2}","a|bc|(d)|e(ff(f)?)|g((h(iii))(h(iii)))");
        testExpand(20,"[a-c]","[a-c]");
        testExpand(21,"[a-c][b-d]","(a|[bc])([bc]|d)");

        feature("match BS");
        testMatch(0,"(","","bad");
        testMatch(1,"*","","bad");
        testMatch(2,")","","bad");
        testMatch(3,"ab[","","bad");
        testMatch(4,"ab[a","","bad");
        testMatch(10,"","");
        testMatch(11,"a","a");
        testMatch(12,"ab","ab");
        testMatch(13,"ab","a","no");
        testMatch(14,"ab","ba","no");

        testMatch(102,"a|b","a");
        testMatch(103,"a|b","b");
        testMatch(104,"(a)*","");
        testMatch(105,"(a)*","a");
        testMatch(106,"(a)*","aa");
        testMatch(107,"(a)+","a");
        testMatch(108,"(a)+","aa");
        testMatch(109,"(a)","a");
        testMatch(110,"(a)?","");
        testMatch(111,"(a)?","a");
        testMatch(112,"(a)*b","b");
        testMatch(113,"(a)*b","ab");
        testMatch(114,"aaab|aaac","aaac");
        testMatch(115,"(ab|(a)*)*","aba");
        testMatch(116,"(a)?(ab)?","ab");
        testMatch(117,"(a)?((ab)?)(b)?","ab");
        testMatch(1171,"((a)?)(((ab)?)((b)?))","ab");
        testMatch(118,"((a)?((ab)?))(b)?","ab");
        testMatch(119,"(a)?(((ab)?)(b)?)","ab");
        testMatch(120,"(a)?(a)?","a");
        testMatch(121,"((a)?(a)?)((a)?(a)?)","aaa");
        testMatch(122,"((a)?(a)?)((a)?(a)?)((a)?(a)?)","aaa");
        testMatch(123,"((a)?(a)?)*","aaa");
        testMatch(124,"(a)?((ab)?)(b)?","ab");
        testMatch(125,"(ab)?((b)?a)","aba");
        testMatch(126,"(a|ab|ba)*","aba");
        testMatch(127,"(aba|a*b)(aba|a*b)","ababa");
        testMatch(128,"(aba|a*b)*","ababa");
        testMatch(129,"(aba|ab|a)(aba|ab|a)","ababa");
        testMatch(130,"(aba|ab|a)*","ababa");
        testMatch(131,"(a(b)?)(a(b)?)","aba");
        testMatch(132,"(a(b)?)+","aba");
        testMatch(133,"(a*)(a*)","aa");
        testMatch(134,"a*(a*)","aa");
        testMatch(135,"(aa(b(b))?)+","aabbaa");
        testMatch(136,"(a(b)?)+","aba");
        testMatch(137,"(a)?b","b");
        testMatch(138,"a*b*","a");
        testMatch(200,"","");
        testMatch(201,"()","");
        testMatch(202,"()*","");
        testMatch(203,"()+","");
        testMatch(204,"()?","");
        testMatch(205,"a*a*","");
        testMatch(206,".","a");
        testMatch(207,"[ab]","b");
        testMatch(208,"[abd]","b");
        testMatch(209,"[a-d]","b");
        testMatch(210,"[^a-dg-p]","e");
        testMatch(211,".a.","aaa");

        // ambiguous
        testMatch(300,"((a)*)*","aa");
        testMatch(301,"(a|b|ab)*","ab");
        testMatch(302,"(ab|a|b)*","abab");
        testMatch(303,"(a|ab)(bc|c)","abc");
        testMatch(304,"(a|a*)*","aa");
        testMatch(305,"(a*|a)*","aa");
        testMatch(306,"a*|(a|b)*","aaa");
        testMatch(307,"((a)*)|(((a)|(b))*)","aaa");
        testMatch(308,"(a|ab)(c|bcd)(de|e)","abcde");
        testMatch(309,"(a+|ba|aba)*b","abab");
        testMatch(310,"(a|b|ab)*","abab");
        testMatch(311,"a*aa","aaaa");
        testMatch(312,"(((ab))|a()b)*","ab");
        testMatch(313,"(a*|a*a*)*b","b");
        testMatch(314,"(a*a*|a*)*b","b");
        testMatch(315,"(a)*(f|j|ajf)*","aajf");
        testMatch(316,"(a|a)(b|b)","ab");
        testMatch(317,"a((b)?)*c","abbc");
        testMatch(318,"a|(a|b)","a");
        testMatch(319,"a|(a|b)","b");
        testMatch(320,"(())*","");

        testMatch(400,"","");
        testMatch(401,"a","a");
        testMatch(402,"abc","abc");
        testMatch(403,"a|bc","a");
        testMatch(404,"a|bc","bc");
        testMatch(405,"a(b)c","abc");
        testMatch(406,"a(bc)d","abcd");
        testMatch(407,"a(b|cd)e","abe");
        testMatch(408,"a(b|cd)e","acde");
        testMatch(409,"a*","a");
        testMatch(410,"(a)","a");
        testMatch(411,"a*","aa");
        testMatch(412,"ab*c","abc");
        testMatch(411,"a*","");
        testMatch(413,"a+","a");
        testMatch(414,"(a)?","");
        testMatch(415,"(a)?","a");
        testMatch(416,"a*b","b");
        testMatch(417,"((xx)(a|b|ab)*)*","xxab");
        testMatch(418,"a+","aa");
        testMatch(419,"a+b","aab");
        testMatch(420,"a+b|cd*","aab");
        testMatch(421,"a+b|cd*","cdd");
        testMatch(422,"a+a*","aaa");

        testMatch(500,"abc","ab","no");
        testMatch(501,"ab","abc","no");
        testMatch(502,"abd","acd","no");
        testMatch(503,"ab(d)?","abc","no");

        // failures
        testMatch(600,"a*bc","b","no");
        testMatch(601,"a(b)c","ac","no");
        testMatch(602,"abc*","a","no");
        testMatch(603,"a|b|c","d","no");
        testMatch(604,"abc","abcd","no");
        testMatch(605,"a+bc","bc","no");
        testMatch(606,"ab+c","ac","no");
        testMatch(607,"abc+","ab","no");
        testMatch(608,"a(b)?c","adc","no");
        testMatch(609,"a(b)?c","abbc","no");
        testMatch(610,"a(b)c","ac","no");
        testMatch(611,"a(b|d)c","ac","no");
        testMatch(612,"a(b|d)c","aac","no");
        testMatch(613,"a(b|dd)c","adc","no");
        testMatch(614,"a(b|d+)c","ac","no");
        testMatch(615,"a(b|(d)?)c","addc","no");
        testMatch(616,"a","","no");
        testMatch(617,"a(b|d)","abc","no");

        // problematic
        testMatch(700,"(a|)*","a");
        testMatch(701,"(a|())*","a");
        testMatch(702,"(a|()*)*","a");
        testMatch(703,"(a|(b)*)*","a");
        testMatch(704,"(a|(a|b)*)*","a");
        testMatch(705,"(a|(a)*|(a|c)*)*","a");
        testMatch(706,"(a|(a)*|(a|c)*)*","a");
        testMatch(707,"a***","aaa");
        testMatch(708,"((a|){2,3}){2}","aa");
        testMatch(709,"(a|)*","");

        // others
        testMatch(750,"(([ab][b]?)([bc][ac])?)*","");
        testMatch(751,"(([bc][ab][a]?)([ab][bc])?)*","");
        testMatch(752,"([a]*|([ac][c])*)*([bc][b])*","");
        testMatch(753,"([ab]*[b])([ab][ab])+","bbb");
    }

    /**
     * Main program.
     *
     * @param      args vector of the arguments
     */

    public static void main(String[] args){
        Locale.setDefault(Locale.US);

        REgen rgt = new REgen();
        rgt.test();
        //if (true) return;

        REgen rg = new REgen();
        rg.parameters.size = 12;
        rg.parameters.depth = new int[]{2,5};
        rg.parameters.balanced = false;             // parameters.balanced = false;
        rg.parameters.starHeight = null;            // parameters.starHeight = [2,6];
        rg.parameters.concarity = 0;                // parameters.concarity = 10;
        rg.parameters.altarity = 0;                 // parameters.altarity = 10;
        rg.parameters.leaves = LEAVES_TERM;         // parameters.leaves = LEAVES_TERM or LEAVES_SET
        rg.parameters.special = null;               // parameters.special = "#@";
        rg.parameters.adjacences = null;            // parameters.adjacences = [][];

        class MyEmitter extends Emitter {
            public void writeRE(String str){
                Trc.out.printf("RE %s\n",str);
            }
        }
        MyEmitter out = new MyEmitter();

        Groups regr = new Groups();
        regr.groups = 10;
        regr.each = 100;
        regr.step = 10;
        Groups textgr = new Groups();
        textgr.groups = 10;
        textgr.each = 10;
        textgr.step = 100;
        //rg.settrc("s");

        //rg.randomNumber.reset();
        //rg.settrc("g");
        //rg.parameters.texts = false;
        rg.generateCollection(regr,textgr,out);

        rg.parameters.balanced = true;
        //rg.generateCollection(regr,textgr,out);

    }
}