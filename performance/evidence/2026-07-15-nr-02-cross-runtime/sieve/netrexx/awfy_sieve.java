/* Generated from 'awfy_sieve.nrx' 15 Jul 2026 16:34:53 [v5.10] *//* Options: Annotations Binary Decimal Implicituses Java Logo Trace2 Verbose3 */






public class awfy_sieve{private static final java.lang.String $0="awfy_sieve.nrx";

@SuppressWarnings("unchecked") public static void main(java.lang.String args[]){int repetitions;int result;int iteration=0;
repetitions=1;
if (args.length>=1) repetitions=(netrexx.lang.Rexx.toRexx(args[0])).toint();
if (repetitions<1) {
netrexx.lang.RexxIO.Say("FAIL: repetitions must be positive");
{System.exit(1);return;}
}

result=0;
{int $1=repetitions;iteration=1;iteration:for(;iteration<=$1;iteration++){
result=sieveOnce();
if (result!=669) {
netrexx.lang.RexxIO.Say("FAIL: expected 669 primes, got"+" "+result);
{System.exit(1);return;}
}
}}/*iteration*/

netrexx.lang.RexxIO.Say("benchmark=awfy_sieve repetitions="+repetitions+" "+"result="+result);
netrexx.lang.RexxIO.Say("PASS: AWFY Sieve NetRexx port");return;}

@SuppressWarnings("unchecked") public static int sieveOnce(){int flags[];int i=0;int primeCount;int k=0;
flags=new int[5001];
{i=1;i:for(;i<=5000;i++){
flags[i]=1;
}}/*i*/

primeCount=0;
{i=2;i:for(;i<=5000;i++){
if (flags[i]!=0) {
primeCount++;
{int $2=i;boolean $3=$2>=0;k=i+i;k:for(;$3?k<=5000:k>=5000;k=k+$2){
flags[k]=0;
}}/*k*/
}
}}/*i*/
return primeCount;}private awfy_sieve(){return;}}