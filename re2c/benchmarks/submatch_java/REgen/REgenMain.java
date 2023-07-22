/*
 * @(#)REgenMain       1.0 2018/10/31
 *
 * Copyright (c) 2018 Angelo Borsotti. All Rights Reserved.
 * This computer program is protected under Copyright.
 */

import java.io.*;
import java.util.*;

/**
 * The <code>REgenMain</code> class provides an example of REgen use.
 *
 * @author  Angelo Borsotti
 * @version 1.0   31 October 2018
 */

/* Generation:
 * 
 * javac REgenMain.java
 *
 * Run
 *
 * java REgenMain fileName-of-collection
 *
 */

public class REgenMain {

    /** A subclass of the Emitter defined in REgen. */
    static class MyEmitter extends REgen.Emitter {
        PrintStream writer;
        public void writeREGroup(int n){
            writer.printf("RE group: %s\n",n);
        }
        public void writeRE(String str){
            writer.printf("RE %s\n",str);
        }
        public void writeTextGroup(int n){
            writer.printf("text group: %s\n",n);
        }
        public void writeText(String text){
            writer.printf("%s\n",text);
        }
    }

    /**
     * Main program.
     *
     * @param      args vector of the arguments
     */

    public static void main(String[] args){
        Locale.setDefault(Locale.US);

        REgen rg = new REgen();

        rg.parameters.size = 5;
        rg.parameters.depth = new int[]{2,5};
        rg.parameters.balanced = true;              // parameters.balanced = false;
        rg.parameters.starHeight = null;            // parameters.starHeight = [2,6];
        rg.parameters.concarity = 0;                // parameters.concarity = 10;
        rg.parameters.altarity = 0;                 // parameters.altarity = 10;
        rg.parameters.leaves = REgen.LEAVES_TERM;   // parameters.leaves = LEAVES_TERM or LEAVES_SET
        rg.parameters.special = null;               // parameters.special = "#@";
        rg.parameters.adjacences = null;            // parameters.adjacences = [][];
        rg.parameters.alphabet = "abcdefghijklmnopqrstuvwxyz";
        rg.parameters.capturing = false;

        PrintStream writer = null;
        try {
            writer = new PrintStream(args[0]);
        } catch (FileNotFoundException exc){
            System.err.printf("no such file: " + args[0]);
            System.exit(1);
        }
        MyEmitter out = new MyEmitter();
        out.writer = writer;

        REgen.Groups regr = new REgen.Groups();
        regr.groups = 10;
        regr.each = 100;
        regr.step = 5;
        REgen.Groups textgr = new REgen.Groups();
        textgr.groups = 10;
        textgr.each = 10;
        textgr.step = 100;

        rg.disallowNesting("?");
        rg.disallowNesting("{}");

        rg.generateCollection(regr,textgr,out);

        writer.close();
    }
}
