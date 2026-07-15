/* Generated from 'awfy_permute.nrx' 15 Jul 2026 16:35:38 [v5.10] *//* Options: Annotations Binary Decimal Implicituses Java Logo Trace2 Verbose3 */






public class awfy_permute{private static final java.lang.String $0="awfy_permute.nrx";

@SuppressWarnings("unchecked") public static void main(java.lang.String args[]){int repetitions;int result;int iteration=0;PermuteBenchmark benchmark=null;
repetitions=1;
if (args.length>=1) repetitions=(netrexx.lang.Rexx.toRexx(args[0])).toint();
if (repetitions<1) {
netrexx.lang.RexxIO.Say("FAIL: repetitions must be positive");
{System.exit(1);return;}
}

result=0;
{int $1=repetitions;iteration=1;iteration:for(;iteration<=$1;iteration++){
benchmark=new PermuteBenchmark();
result=benchmark.run();
if (result!=8660) {
netrexx.lang.RexxIO.Say("FAIL: expected 8660 calls, got"+" "+result);
{System.exit(1);return;}
}
}}/*iteration*/

netrexx.lang.RexxIO.Say("benchmark=awfy_permute repetitions="+repetitions+" "+"result="+result);
netrexx.lang.RexxIO.Say("PASS: AWFY Permute NetRexx port");return;}

private awfy_permute(){return;}}class PermuteBenchmark{private static final java.lang.String $0="awfy_permute.nrx";

/* properties private */
private int count=0;
private int values[]=new int[6];

@SuppressWarnings("unchecked") public PermuteBenchmark(){super();return;}

@SuppressWarnings("unchecked") public int run(){
count=0;
permute(6);
return count;}

@SuppressWarnings("unchecked") public void permute(int n){int n1=0;int i=0;
count++;
if (n!=0) {
n1=n-1;
permute(n1);
{i=n1;i:for(;i>=0;i--){
swap(n1,i);
permute(n1);
swap(n1,i);
}}/*i*/
}return;}

@SuppressWarnings("unchecked") public void swap(int i,int j){int temporary;
temporary=values[i];
values[i]=values[j];
values[j]=temporary;return;}}