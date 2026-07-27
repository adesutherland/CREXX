/* Generated from 'rexxcps_2_2n.nrx' 27 Jul 2026 18:39:28 [v5.10] *//* Options: Annotations Decimal Implicituses Java Logo Trace2 Verbose3 */















































public class rexxcps_2_2n{private static final netrexx.lang.Rexx $01=new netrexx.lang.Rexx((byte)20);private static final netrexx.lang.Rexx $02=new netrexx.lang.Rexx("Java method rexxcps_2_2n.nrx");private static final char[] $03={3,1,1,10,1,0,1,10,2,1,2,0};private static final netrexx.lang.Rexx $04=new netrexx.lang.Rexx("NetRexx 5.10 20 Mar 2026");private static final char[] $05={1,10,1,0,0};private static final netrexx.lang.Rexx $06=new netrexx.lang.Rexx((byte)2);private static final netrexx.lang.Rexx $07=new netrexx.lang.Rexx((byte)14);private static final netrexx.lang.Rexx $08=new netrexx.lang.Rexx(1234);private static final netrexx.lang.Rexx $09=netrexx.lang.Rexx.toRexx("5678");private static final netrexx.lang.Rexx $010=new netrexx.lang.Rexx("1.1");private static final netrexx.lang.Rexx $011=new netrexx.lang.Rexx("2.2");private static final netrexx.lang.Rexx $012=new netrexx.lang.Rexx(1);private static final netrexx.lang.Rexx $013=new netrexx.lang.Rexx(17);private static final netrexx.lang.Rexx $014=netrexx.lang.Rexx.toRexx("foobar");private static final netrexx.lang.Rexx $015=new netrexx.lang.Rexx(9);private static final netrexx.lang.Rexx $016=new netrexx.lang.Rexx('?');private static final netrexx.lang.Rexx $017=new netrexx.lang.Rexx(5);private static final netrexx.lang.Rexx $018=new netrexx.lang.Rexx(2);private static final netrexx.lang.Rexx $019=new netrexx.lang.Rexx("1.0");private static final netrexx.lang.Rexx $020=netrexx.lang.Rexx.toRexx("");private static final netrexx.lang.Rexx $021=netrexx.lang.Rexx.toRexx("string");private static final netrexx.lang.Rexx $022=new netrexx.lang.Rexx(0);private static final netrexx.lang.Rexx $023=new netrexx.lang.Rexx("99.7");private static final netrexx.lang.Rexx $024=netrexx.lang.Rexx.toRexx("ring");private static final char[] $025={4,1,5,10,1,0,1,10,2,1,2,0};private static final netrexx.lang.Rexx $026=netrexx.lang.Rexx.toRexx("with");private static final netrexx.lang.Rexx $027=netrexx.lang.Rexx.toRexx("args");private static final char[] $028={6,1,10,1,0,1,10,1,2,0};private static final netrexx.lang.Rexx $029=new netrexx.lang.Rexx("1000000000");private static final netrexx.lang.Rexx $030=new netrexx.lang.Rexx(100);private static final netrexx.lang.Rexx $031=netrexx.lang.Rexx.toRexx("iterations");private static final netrexx.lang.Rexx $032=netrexx.lang.Rexx.toRexx("s)");private static final netrexx.lang.Rexx $033=netrexx.lang.Rexx.toRexx("seconds");private static final netrexx.lang.Rexx $034=new netrexx.lang.Rexx(1000);private static final netrexx.lang.Rexx $035=new netrexx.lang.Rexx("0.5");private static final netrexx.lang.Rexx $036=netrexx.lang.Rexx.toRexx("cps)");private static final char[] $037={1,10,5,0,1,2,3,4,0};private static final char[] $038={1,10,4,0,1,2,3,0};private static final netrexx.lang.Rexx $039=new netrexx.lang.Rexx((byte)1);private static final char[] $040={1,10,3,0,1,2,0};private static final java.lang.String $0="rexxcps_2_2n.nrx";private static final netrexx.lang.RexxSet $1=new netrexx.lang.RexxSet();static{$1.setDigits($01);}@SuppressWarnings("unchecked") public static void main(java.lang.String $0s[]){netrexx.lang.Rexx mysource=null;netrexx.lang.Rexx opsystem=null;netrexx.lang.Rexx myversion=null;netrexx.lang.Rexx dstats;netrexx.lang.Rexx count;netrexx.lang.Rexx averaging;netrexx.lang.Rexx rexxcps;long empty;long start=0;netrexx.lang.Rexx fempty;netrexx.lang.Rexx trial=null;long full=0;netrexx.lang.Rexx flag=null;netrexx.lang.Rexx p0=null;netrexx.lang.Rexx lvar=null;netrexx.lang.Rexx key1=null;netrexx.lang.Rexx acompound=null;netrexx.lang.Rexx j=null;netrexx.lang.Rexx avar=null;netrexx.lang.Rexx v1=null;netrexx.lang.Rexx v2=null;netrexx.lang.Rexx address=null;netrexx.lang.Rexx rc=null;netrexx.lang.Rexx p1=null;netrexx.lang.Rexx p5=null;netrexx.lang.Rexx p2=null;netrexx.lang.Rexx p6=null;netrexx.lang.Rexx p3=null;netrexx.lang.Rexx p7=null;netrexx.lang.Rexx p4=null;netrexx.lang.Rexx p8=null;netrexx.lang.Rexx total=null;netrexx.lang.Rexx ffull;netrexx.lang.Rexx looptime;netrexx.lang.Rexx cps;{netrexx.lang.Rexx $2[]=new netrexx.lang.Rexx[3];netrexx.lang.RexxParse.parse($02,$03,$2);mysource=$2[0];opsystem=$2[1];}
mysource=mysource;


{netrexx.lang.Rexx $3[]=new netrexx.lang.Rexx[1];netrexx.lang.RexxParse.parse($04,$05,$3);myversion=$3[0];}
dstats=new netrexx.lang.Rexx((byte)1);
count=new netrexx.lang.Rexx((byte)100);

averaging=new netrexx.lang.Rexx((byte)100);

rexxcps=netrexx.lang.Rexx.toRexx("2.2n");
netrexx.lang.RexxIO.Say((netrexx.lang.Rexx.toRexx("----- REXXCPS").OpCcblank($1,rexxcps)).OpCcblank($1,netrexx.lang.Rexx.toRexx("-- Measuring NetRexx clauses/second -----")));
netrexx.lang.RexxIO.Say(netrexx.lang.Rexx.toRexx(" NetRexx version is:").OpCcblank($1,myversion));
netrexx.lang.RexxIO.Say(netrexx.lang.Rexx.toRexx("          System is:").OpCcblank($1,opsystem));


empty=new netrexx.lang.Rexx(0).tolong();
{int $4=averaging.OpPlus($1).toint();for(;$4>0;$4--){
start=java.lang.System.nanoTime();
{int $5=count.OpPlus($1).toint();for(;$5>0;$5--){}}
empty=new netrexx.lang.Rexx(empty).OpAdd($1,new netrexx.lang.Rexx(java.lang.System.nanoTime())).OpSub($1,new netrexx.lang.Rexx(start)).tolong();
}}
fempty=new netrexx.lang.Rexx(empty).OpDiv($1,averaging);



{trial=new netrexx.lang.Rexx((byte)1);trial:for(;trial.OpLtEq($1,$06);trial=trial.OpAdd($1,new netrexx.lang.Rexx(1))){

full=new netrexx.lang.Rexx(0).tolong();
{int $6=averaging.OpPlus($1).toint();for(;$6>0;$6--){
start=java.lang.System.nanoTime();
{int $7=count.OpPlus($1).toint();for(;$7>0;$7--){

flag=new netrexx.lang.Rexx((byte)0);p0=new netrexx.lang.Rexx('b');
{lvar=new netrexx.lang.Rexx((byte)1);lvar:for(;lvar.OpLtEq($1,$07);lvar=lvar.OpAdd($1,new netrexx.lang.Rexx(1))){

key1=netrexx.lang.Rexx.toRexx("Key Bee");acompound=netrexx.lang.Rexx.toRexx("");
acompound.getnode(key1).leaf.getnode(lvar).leaf=($08.OpCc($1,$09)).substr(new netrexx.lang.Rexx((byte)6),new netrexx.lang.Rexx((byte)2));
if (flag.OpEq($1,acompound.getnode(key1).leaf.getnode(lvar).leaf)) netrexx.lang.RexxIO.Say("Failed1");
{netrexx.lang.Rexx $8=$010;j=$010.OpPlus($1);j:for(;j.OpLtEq($1,$011);j=j.OpAdd($1,$8)){
if (j.OpGt($1,acompound.getnode(key1).leaf.getnode(lvar).leaf)) netrexx.lang.RexxIO.Say("Failed2");
if ($013.OpLt($1,(key1.length()).OpSub($1,$012))) netrexx.lang.RexxIO.Say("Failed3");
if (j.OpEq($1,$014)) netrexx.lang.RexxIO.Say("Failed4");
if ((key1.substr(new netrexx.lang.Rexx((byte)1),new netrexx.lang.Rexx((byte)1))).OpEq($1,$015)) netrexx.lang.RexxIO.Say("Failed5");
if ((key1.word(new netrexx.lang.Rexx((byte)1))).OpEq($1,$016)) netrexx.lang.RexxIO.Say("Failed6");
if (j.OpLt($1,$017)) {
acompound.getnode(key1).leaf.getnode(lvar).leaf=(acompound.getnode(key1).leaf.getnode(lvar).leaf).OpAdd($1,$012);
if (j.OpEq($1,$018)) break j;
}
continue j;
}}/*j*/
avar=($019.OpCc($1,$020)).OpCc($1,lvar);
{/*select*/
if (flag.OpEq($1,$021))netrexx.lang.RexxIO.Say("FailedS1");
else if ((avar.getnode(flag).leaf.getnode(new netrexx.lang.Rexx((byte)2)).leaf).OpEq($1,$022))netrexx.lang.RexxIO.Say("FailedS2");
else if (flag.OpEq($1,$017.OpAdd($1,$023)))netrexx.lang.RexxIO.Say("FailedS3");
else if (flag.toboolean())avar.getnode(new netrexx.lang.Rexx((byte)1)).leaf.getnode(new netrexx.lang.Rexx((byte)2)).leaf=(avar.getnode(new netrexx.lang.Rexx((byte)1)).leaf.getnode(new netrexx.lang.Rexx((byte)2)).leaf).OpMult($1,$010);
else if (flag.OpEqS($1,$022))flag=new netrexx.lang.Rexx((byte)0);
else{throw new netrexx.lang.NoOtherwiseException();}}
if (true) flag=new netrexx.lang.Rexx((byte)1);
{/*select*/
if (flag.OpEqS($1,$024))netrexx.lang.RexxIO.Say("FailedT1");
else if ((avar.getnode(flag).leaf.getnode(new netrexx.lang.Rexx((byte)3)).leaf).OpEq($1,$022))netrexx.lang.RexxIO.Say("FailedT2");
else if (flag.toboolean())avar.getnode(new netrexx.lang.Rexx((byte)1)).leaf.getnode(new netrexx.lang.Rexx((byte)2)).leaf=(avar.getnode(new netrexx.lang.Rexx((byte)1)).leaf.getnode(new netrexx.lang.Rexx((byte)2)).leaf).OpMult($1,$010);
else if (flag.OpEqS($1,$022))flag=new netrexx.lang.Rexx((byte)1);
else{throw new netrexx.lang.NoOtherwiseException();}}
{netrexx.lang.Rexx $9[]=new netrexx.lang.Rexx[3];netrexx.lang.RexxParse.parse(netrexx.lang.Rexx.toRexx("Foo Bar"),$025,$9);v1=$9[0];v2=$9[1];}
address=(new netrexx.lang.Rexx($1.digits));
address=address;
subroutine(($026.OpCcblank($1,$018)).OpCcblank($1,$027),((netrexx.lang.Rexx.toRexx("(This is the second)").OpCc($1,$012)).OpCc($1,$020)).OpCc($1,$012));
rc=netrexx.lang.Rexx.toRexx("This is an awfully boring program");{netrexx.lang.Rexx $10[]=new netrexx.lang.Rexx[3];$10[1]=p0;netrexx.lang.RexxParse.parse(rc,$028,$10);p1=$10[0];p5=$10[2];}
rc=netrexx.lang.Rexx.toRexx("is an awfully boring program This");{netrexx.lang.Rexx $11[]=new netrexx.lang.Rexx[3];$11[1]=p0;netrexx.lang.RexxParse.parse(rc,$028,$11);p2=$11[0];p6=$11[2];}
rc=netrexx.lang.Rexx.toRexx("an awfully boring program This is");{netrexx.lang.Rexx $12[]=new netrexx.lang.Rexx[3];$12[1]=p0;netrexx.lang.RexxParse.parse(rc,$028,$12);p3=$12[0];p7=$12[2];}
rc=netrexx.lang.Rexx.toRexx("awfully boring program This is an");{netrexx.lang.Rexx $13[]=new netrexx.lang.Rexx[3];$13[1]=p0;netrexx.lang.RexxParse.parse(rc,$028,$13);p4=$13[0];p8=$13[2];}
}}/*lvar*/

}}
full=new netrexx.lang.Rexx(full).OpAdd($1,new netrexx.lang.Rexx(java.lang.System.nanoTime())).OpSub($1,new netrexx.lang.Rexx(start)).tolong();
}}
total=new netrexx.lang.Rexx(full).OpDiv($1,$029);
if (total.OpGt($1,$012)|trial.OpEq($1,$018)) break trial;
if (total.OpEq($1,$022)) count=count.OpMult($1,$030);
else count=(($012.OpDivI($1,total).OpAdd($1,$012))).OpMult($1,count);
}}/*trial*/

ffull=new netrexx.lang.Rexx(full).OpDiv($1,averaging);
netrexx.lang.RexxIO.Say((((netrexx.lang.Rexx.toRexx("          Averaging:").OpCcblank($1,averaging)).OpCcblank($1,netrexx.lang.Rexx.toRexx("measures of"))).OpCcblank($1,count)).OpCcblank($1,$031));
netrexx.lang.RexxIO.Say((netrexx.lang.Rexx.toRexx("                       of 1000 clauses (over").OpCcblank($1,total.format((netrexx.lang.Rexx)null,(netrexx.lang.Rexx)null,new netrexx.lang.Rexx((byte)1)))).OpCcblank($1,$032));


looptime=((ffull.OpSub($1,fempty))).OpDiv($1,(count.OpMult($1,$029)));

if (dstats.toboolean()) {
netrexx.lang.RexxIO.Say("");
netrexx.lang.RexxIO.Say((((((netrexx.lang.Rexx.toRexx("Total (full DOs):").OpCcblank($1,total)).OpCcblank($1,netrexx.lang.Rexx.toRexx("secs (average of"))).OpCcblank($1,averaging)).OpCcblank($1,netrexx.lang.Rexx.toRexx("measures of"))).OpCcblank($1,count)).OpCcblank($1,netrexx.lang.Rexx.toRexx("iterations)")));

netrexx.lang.RexxIO.Say((netrexx.lang.Rexx.toRexx("Time for one iteration (1000 clauses) was:").OpCcblank($1,looptime)).OpCcblank($1,$033));
}


cps=($034.OpDiv($1,looptime).OpAdd($1,$035));
cps=cps.substr(new netrexx.lang.Rexx((byte)1),(cps.pos(new netrexx.lang.Rexx('.'))).OpSub($1,$012));
netrexx.lang.RexxIO.Say("");
netrexx.lang.RexxIO.Say((netrexx.lang.Rexx.toRexx("     Performance:").OpCcblank($1,formatThousands(cps))).OpCcblank($1,netrexx.lang.Rexx.toRexx("NetRexx clauses per second")));
netrexx.lang.RexxIO.Say((netrexx.lang.Rexx.toRexx("                (").OpCcblank($1,cps.format((netrexx.lang.Rexx)null,new netrexx.lang.Rexx((byte)3),(netrexx.lang.Rexx)null,new netrexx.lang.Rexx((byte)0),new netrexx.lang.Rexx('e')))).OpCcblank($1,$036));
netrexx.lang.RexxIO.Say("");
p1=p1;p2=p2;p3=p3;p4=p4;p5=p5;p6=p6;p7=p7;p8=p8;v1=v1;v2=v2;
netrexx.lang.RexxIO.Say("PASS: RexxCPS 2.2n NetRexx disclosed adaptation");
{System.exit(0);return;}}



@SuppressWarnings("unchecked") public static final void subroutine(netrexx.lang.Rexx arg1,netrexx.lang.Rexx arg2){netrexx.lang.Rexx a1=null;netrexx.lang.Rexx a2=null;netrexx.lang.Rexx a3=null;netrexx.lang.Rexx a4=null;netrexx.lang.Rexx b1=null;netrexx.lang.Rexx b2=null;netrexx.lang.Rexx b3=null;netrexx.lang.Rexx rc=null;netrexx.lang.Rexx c1=null;netrexx.lang.Rexx c2=null;netrexx.lang.Rexx c3=null;
{netrexx.lang.Rexx $14[]=new netrexx.lang.Rexx[5];netrexx.lang.RexxParse.parse(arg1.upper(),$037,$14);a1=$14[0];a2=$14[1];a3=$14[2];a4=$14[4];}
{netrexx.lang.Rexx $15[]=new netrexx.lang.Rexx[4];netrexx.lang.RexxParse.parse(a3,$038,$15);b1=$15[0];b2=$15[1];b3=$15[2];}

{int $16=$039.OpPlus($1).toint();for(;$16>0;$16--){rc=(a1.OpCcblank($1,a2)).OpCcblank($1,a3);{netrexx.lang.Rexx $17[]=new netrexx.lang.Rexx[3];netrexx.lang.RexxParse.parse(rc,$040,$17);c1=$17[0];c2=$17[1];c3=$17[2];}}}
b1=b1;b2=b2;b3=b3;
a1=a1;a2=a2;a3=a3;a4=a4;
c1=c1;c2=c2;c3=c3;


return;}


@SuppressWarnings("unchecked") public static netrexx.lang.Rexx formatThousands(netrexx.lang.Rexx n){netrexx.lang.Rexx mn;netrexx.lang.Rexx nm;netrexx.lang.Rexx raw;
mn=n.reverse();
nm=netrexx.lang.Rexx.toRexx("abc_def_ghi_jkl_mno_pqr_stu_vwx_yz1").translate(mn,netrexx.lang.Rexx.toRexx("abcdefghijklmnopqrstuvwxyz1"));
raw=nm.reverse().strip();
{for(;;){if(!(raw.left(new netrexx.lang.Rexx((byte)1)).datatype(new netrexx.lang.Rexx('d'))).OpNot($1))break;
raw=raw.substr(new netrexx.lang.Rexx((byte)2));
}}
return raw;}private rexxcps_2_2n(){return;}}