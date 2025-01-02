/* Matrix Plugin Test */
options levelb
import rxfnsb
import matrix

say '*** Regression Test ***'
rx=regdat()
ry=regression(rx,rx+1,'Regression')
call mprint ry,""
say '*** End of Regression Test ***'

say '*** loading new data, some basic statistical tests ***'
m1=mcreate(24,5,"Data Matrix")
call matdata m1

call mprint  m1,""
say "Mean   COL 1 "mmean(m1,1)
say "Stddev COL 1 "mstdev(m1,1)
say "Rows of M1 "stats(m1,'r')
say "Cols of M1 "stats(m1,'c')
m2=m1
say '*** end of basic tests ***'

say '*** Create Correlation Matrix ***'
m9=Mcorr(m1,'Correlation')
call mprint m9,""
say '*** end of correlation ***'

say '*** Create Covariance Matrix ***'
ma=Mcov(m1,'Covariance')
call mprint ma,""
say '*** end of covariance ***'

say '*** Create Correlation Matrix by Matrix operations ***'
## lets do the calculation via Matrix operations
## 1. transpose Data Matrix
  m2=mtranspose(m1,"Transposed Data Matrix")
  call mprint m2,""
say "************* End of Transpose I *******************"
## 2. Standardise Data Matrix (mean=0, stddev=1)
  m4=mstandard(m1,"Standardised")
  call mprint  m4,""
## 3. transpose standardised Data Matrix
  m5=mtranspose(m4,"Transposed Data Matrix")
  call mprint m5,""
  say "************* End of Transpose II *******************"
## 4. multiply Data Matrix with transposed Matrix
  m6=mmult(m5,m4,"close to Correlation Matrix")
  call mprint m6,""
   say "************* End of Mult II *******************"
## 5. Almost there, multiply by rows
  m7=mprod(m6,1/23,"Correlation Matrix")
  call mprint m7,""
 say '*** end of correlation ***'

say '*** other functions ***'
   say 'determinante 'mdet(m1)
## 7. other stuff: L and U Matrix
   mlx=mlu(m6,"L Matrix","U Matrix")
   call mprint mlx,""
   call mprint mlx+1,""
   stats = mcolstats(m1, "Statistics")
   call mprint stats,""
say copies('-',72)
say 'Factor Analysis'
say copies('-',72)
say '*** Data Matrix before Factor Analysis ***'

mfdata=mcreate(20,5,'factorial')
call factdata mfdata    ## load new data

call mprint mfdata,""      ##print it
mfc=mCorr(mfdata,'Correlation')
call mprint mfc,'Correlation of loaded data'
## now do some different facto analysis
loadings2 = mfactor(mfdata, 2, "Factor Analysis Varimax","Rotate=Varimax,SCORE,diag=3")
  call analyseFact loadings2,mfc
loadings2a = mfactor(mfdata, 1, "Factor Analysis Varimax","Rotate=Varimax,SCORE,diag=3")
  call analyseFact loadings2a,mfc
loadings1 = mfactor(mfdata, 2, "Factor Analysis unrotated", "Rotate=none,SCORES,diag=3")
  call analyseFact loadings1,mfc
loadings3 = mfactor(mfdata, 2, "Factor Analysis Quartimax","diag=3")
  call analyseFact loadings3,mfc
loadings4 = mfactor(mfdata, 2, "Factor Analysis Promax", "Rotate=promax,SCORE,DIAG=5")
  call analyseFact loadings4,mfc
call mfree -1            ## free all
exit

analyseFact: procedure
  arg fact=.int,cor=.int
  say '*** Factors are *** '
  call mprint fact,""
  factT=mtranspose(fact,"Transposed factorial")
  factmult=Mmult(fact,factT,'Generated Correlation')
  call mprint factMult,""
  call mprint cor,""
##  call checkCorr cor
  mdif=mcreate(5,5,'Difference FCorrell and Correll')

  rows=stats(cor,'Rows')
  cols=stats(cor,'cols')
  maxdif=0.0
  do i=1 to  rows
     do j=i+1 to cols
        diff=mget(cor,i,j)-mget(factmult,i,j)
        call mset mdif,i,j,diff
        maxdif=maxdif+abs(diff)
     end
  end
  say "cumulated differences "maxdif
##  call mprint mdif,""
return



## Function to perform linear regression
regression: procedure=.int
  arg ind=.int, dep=.int, resultid=.string
  yrows=stats(ind,'Rows')
  xrows=stats(dep,'Rows')
  xcols=stats(ind,'COLs')

  if yrows <> Xrows then return -62; ## Dimension mismatch

 ## Step 1: Add a column of ones to X for the intercept
    augmented=mexpand(ind,1,1.0)
    call mprint augmented,""
 ## Step 2: calculate ('X*X)
    Xt  = mtranspose(augmented, "Xt");
    XtX = mmult(Xt, augmented,  "XtX");
 ## Step 3: Calculate (X'y)
    XtY = mmult(Xt, dep, "XtY");
    call mprint xty,""
 ## Step 4: Invert (X'X)
    XtX_inv = minvert(XtX, "XtX_inverted");
    if XtX_inv < 0 then return -21;             ## Singular matrix error
    call mprint xtx_inv,""
 ## Step 5: Calculate coefficients: result = (X'X)^(-1)(X'y)
    result = mmult(XtX_inv, XtY, "Regression Coefficients");
    call mprint result,""
 ## Clean up temporary matrices
 say '*************** cleanup *******************'
    say "FREE XT      "xt  mfree(Xt);
    say "FREE XTX     "xtx mfree(XtX);
    say "FREE XTY     "xty mfree(XtY);
    say "FREE XTX_INV "xtx_inv mfree(XtX_inv);
return result

/* Define thresholds for categorization */
checkCorr: procedure
  arg cor=.int
  strong_threshold = 0.7
  moderate_threshold = 0.3

  rows=stats(cor,'Rows')
  cols=stats(cor,'COLs')

/* Interpret the matrix */
do row = 1 to rows
    do col = 1 to cols
        if row = col then iterate
        corr_value = mget(cor,row,col)
        if corr_value > strong_threshold then  ,
           say "Variables "row" and " col "have a strong positive correlation of "corr_value
        else if corr_value < -strong_threshold then  ,
           say "Variables "row" and " col "have a strong negative correlation of "corr_value
        else if corr_value > moderate_threshold then  ,
           say "Variables "row" and " col "have a moderate positive correlation of "corr_value
        else if corr_value < -moderate_threshold then  ,
             say "Variables "row" and " col "have a moderate negative correlation of "corr_value
    end
end
return

factdata:procedure=.int
  arg fact=.int
call mset fact,1,1,4.2
call mset fact,2,1,3.9
call mset fact,3,1,4.5
call mset fact,4,1,4.0
call mset fact,5,1,4.3
call mset fact,6,1,4.1
call mset fact,7,1,4.4
call mset fact,8,1,3.8
call mset fact,9,1,4.6
call mset fact,10,1,4.2
call mset fact,11,1,4.3
call mset fact,12,1,4.1
call mset fact,13,1,4.5
call mset fact,14,1,3.9
call mset fact,15,1,4.4
call mset fact,16,1,4.0
call mset fact,17,1,4.3
call mset fact,18,1,4.2
call mset fact,19,1,4.6
call mset fact,20,1,3.8
call mset fact,1,2,3.8
call mset fact,2,2,4.1
call mset fact,3,2,3.7
call mset fact,4,2,4.0
call mset fact,5,2,3.9
call mset fact,6,2,4.2
call mset fact,7,2,3.6
call mset fact,8,2,4.3
call mset fact,9,2,3.5
call mset fact,10,2,4.1
call mset fact,11,2,3.8
call mset fact,12,2,4.0
call mset fact,13,2,3.9
call mset fact,14,2,4.1
call mset fact,15,2,3.7
call mset fact,16,2,4.2
call mset fact,17,2,3.6
call mset fact,18,2,4.1
call mset fact,19,2,3.5
call mset fact,20,2,4.3
call mset fact,1,3,6.3
call mset fact,2,3,5.8
call mset fact,3,3,6.5
call mset fact,4,3,5.9
call mset fact,5,3,6.2
call mset fact,6,3,6.0
call mset fact,7,3,6.4
call mset fact,8,3,5.7
call mset fact,9,3,6.6
call mset fact,10,3,6.1
call mset fact,11,3,6.2
call mset fact,12,3,6.0
call mset fact,13,3,6.4
call mset fact,14,3,5.8
call mset fact,15,3,6.3
call mset fact,16,3,5.9
call mset fact,17,3,6.2
call mset fact,18,3,6.1
call mset fact,19,3,6.6
call mset fact,20,3,5.7
call mset fact,1,4,6.8
call mset fact,2,4,6.1
call mset fact,3,4,6.9
call mset fact,4,4,6.3
call mset fact,5,4,6.7
call mset fact,6,4,6.4
call mset fact,7,4,6.8
call mset fact,8,4,6.0
call mset fact,9,4,7.0
call mset fact,10,4,6.5
call mset fact,11,4,6.7
call mset fact,12,4,6.4
call mset fact,13,4,6.9
call mset fact,14,4,6.2
call mset fact,15,4,6.8
call mset fact,16,4,6.3
call mset fact,17,4,6.7
call mset fact,18,4,6.5
call mset fact,19,4,7.0
call mset fact,20,4,6.0
call mset fact,1,5,5.7
call mset fact,2,5,5.3
call mset fact,3,5,5.8
call mset fact,4,5,5.4
call mset fact,5,5,5.6
call mset fact,6,5,5.5
call mset fact,7,5,5.7
call mset fact,8,5,5.2
call mset fact,9,5,5.9
call mset fact,10,5,5.6
call mset fact,11,5,5.7
call mset fact,12,5,5.5
call mset fact,13,5,5.8
call mset fact,14,5,5.4
call mset fact,15,5,5.7
call mset fact,16,5,5.4
call mset fact,17,5,5.6
call mset fact,18,5,5.6
call mset fact,19,5,5.9
call mset fact,20,5,5.2
return 0



matdata: procedure=.int
arg m9=.int
rc=mset(m9, 1, 1, 0.37454012)
rc=mset(m9, 1, 2, 0.95071431)
rc=mset(m9, 1, 3, 0.73199394)
rc=mset(m9, 1, 4, 0.59865848)
rc=mset(m9, 1, 5, 0.15601864)
rc=mset(m9, 2, 1, 0.15599452)
rc=mset(m9, 2, 2, 0.05808361)
rc=mset(m9, 2, 3, 0.86617615)
rc=mset(m9, 2, 4, 0.60111501)
rc=mset(m9, 2, 5, 0.70807258)
rc=mset(m9, 3, 1, 0.02058449)
rc=mset(m9, 3, 2, 0.96990985)
rc=mset(m9, 3, 3, 0.83244264)
rc=mset(m9, 3, 4, 0.21233911)
rc=mset(m9, 3, 5, 0.18182497)
rc=mset(m9, 4, 1, 0.18340451)
rc=mset(m9, 4, 2, 0.30424224)
rc=mset(m9, 4, 3, 0.52475643)
rc=mset(m9, 4, 4, 0.43194502)
rc=mset(m9, 4, 5, 0.29122914)
rc=mset(m9, 5, 1, 0.61185289)
rc=mset(m9, 5, 2, 0.13949386)
rc=mset(m9, 5, 3, 0.29214465)
rc=mset(m9, 5, 4, 0.36636184)
rc=mset(m9, 5, 5, 0.45606998)
rc=mset(m9, 6, 1, 0.78517596)
rc=mset(m9, 6, 2, 0.19967378)
rc=mset(m9, 6, 3, 0.51423444)
rc=mset(m9, 6, 4, 0.59241457)
rc=mset(m9, 6, 5, 0.04645041)
rc=mset(m9, 7, 1, 0.60754485)
rc=mset(m9, 7, 2, 0.17052412)
rc=mset(m9, 7, 3, 0.06505159)
rc=mset(m9, 7, 4, 0.94888554)
rc=mset(m9, 7, 5, 0.96563203)
rc=mset(m9, 8, 1, 0.80839735)
rc=mset(m9, 8, 2, 0.30461377)
rc=mset(m9, 8, 3, 0.09767211)
rc=mset(m9, 8, 4, 0.68423303)
rc=mset(m9, 8, 5, 0.44015249)
rc=mset(m9, 9, 1, 0.12203823)
rc=mset(m9, 9, 2, 0.49517691)
rc=mset(m9, 9, 3, 0.03438852)
rc=mset(m9, 9, 4, 0.90932040)
rc=mset(m9, 9, 5, 0.25877998)
rc=mset(m9, 10, 1, 0.66252228)
rc=mset(m9, 10, 2, 0.31171108)
rc=mset(m9, 10, 3, 0.52006802)
rc=mset(m9, 10, 4, 0.54671028)
rc=mset(m9, 10, 5, 0.18485446)
rc=mset(m9, 11, 1, 0.96958463)
rc=mset(m9, 11, 2, 0.77513282)
rc=mset(m9, 11, 3, 0.93949894)
rc=mset(m9, 11, 4, 0.89482735)
rc=mset(m9, 11, 5, 0.59789998)
rc=mset(m9, 12, 1, 0.92187424)
rc=mset(m9, 12, 2, 0.08849250)
rc=mset(m9, 12, 3, 0.19598286)
rc=mset(m9, 12, 4, 0.04522729)
rc=mset(m9, 12, 5, 0.32533033)
rc=mset(m9, 13, 1, 0.38867729)
rc=mset(m9, 13, 2, 0.27134903)
rc=mset(m9, 13, 3, 0.82873751)
rc=mset(m9, 13, 4, 0.35675333)
rc=mset(m9, 13, 5, 0.28093451)
rc=mset(m9, 14, 1, 0.54269608)
rc=mset(m9, 14, 2, 0.14092422)
rc=mset(m9, 14, 3, 0.80219698)
rc=mset(m9, 14, 4, 0.07455064)
rc=mset(m9, 14, 5, 0.98688694)
rc=mset(m9, 15, 1, 0.77224477)
rc=mset(m9, 15, 2, 0.19871568)
rc=mset(m9, 15, 3, 0.00552212)
rc=mset(m9, 15, 4, 0.81546143)
rc=mset(m9, 15, 5, 0.70685734)
rc=mset(m9, 16, 1, 0.72900717)
rc=mset(m9, 16, 2, 0.77127035)
rc=mset(m9, 16, 3, 0.07404465)
rc=mset(m9, 16, 4, 0.35846573)
rc=mset(m9, 16, 5, 0.11586906)
rc=mset(m9, 17, 1, 0.86310343)
rc=mset(m9, 17, 2, 0.62329813)
rc=mset(m9, 17, 3, 0.33089802)
rc=mset(m9, 17, 4, 0.06355835)
rc=mset(m9, 17, 5, 0.31098232)
rc=mset(m9, 18, 1, 0.32518332)
rc=mset(m9, 18, 2, 0.72960618)
rc=mset(m9, 18, 3, 0.63755747)
rc=mset(m9, 18, 4, 0.88721274)
rc=mset(m9, 18, 5, 0.47221493)
rc=mset(m9, 19, 1, 0.11959425)
rc=mset(m9, 19, 2, 0.71324479)
rc=mset(m9, 19, 3, 0.76078505)
rc=mset(m9, 19, 4, 0.56127720)
rc=mset(m9, 19, 5, 0.77096718)
rc=mset(m9, 20, 1, 0.49379560)
rc=mset(m9, 20, 2, 0.52273283)
rc=mset(m9, 20, 3, 0.42754102)
rc=mset(m9, 20, 4, 0.02541913)
rc=mset(m9, 20, 5, 0.10789143)
rc=mset(m9, 21, 1, 0.03142919)
rc=mset(m9, 21, 2, 0.63641041)
rc=mset(m9, 21, 3, 0.31435598)
rc=mset(m9, 21, 4, 0.50857069)
rc=mset(m9, 21, 5, 0.90756647)
rc=mset(m9, 22, 1, 0.24929223)
rc=mset(m9, 22, 2, 0.41038292)
rc=mset(m9, 22, 3, 0.75555114)
rc=mset(m9, 22, 4, 0.22879817)
rc=mset(m9, 22, 5, 0.07697991)
rc=mset(m9, 23, 1, 0.28975145)
rc=mset(m9, 23, 2, 0.16122129)
rc=mset(m9, 23, 3, 0.92969765)
rc=mset(m9, 23, 4, 0.80812038)
rc=mset(m9, 23, 5, 0.63340376)
rc=mset(m9, 24, 1, 0.87146059)
rc=mset(m9, 24, 2, 0.80367208)
rc=mset(m9, 24, 3, 0.18657006)
rc=mset(m9, 24, 4, 0.89255900)
rc=mset(m9, 24, 5, 0.53934224)
return 0
regdat: procedure=.int
reg.1='5.1 3.5 1.4 0.2 0.8 10.3'
reg.2='7.0 3.2 4.7 1.4 1.5 15.8'
reg.3='6.3 3.3 6.0 2.5 1.7 20.1'
reg.4='5.8 2.7 5.1 1.9 1.2 17.5'
reg.5='4.6 3.1 1.5 0.2 0.9 11.0'
reg.6='6.9 3.1 5.4 2.1 1.8 19.7'
reg.7='5.0 3.6 1.4 0.2 0.7 10.1'
reg.8='6.7 3.0 5.2 2.3 1.6 18.5'
reg.9='4.9 3.1 1.5 0.1 0.8 10.4'
reg.10='5.7 2.8 4.1 1.3 1.1 14.8'
r1=mcreate(10,5,'Independant')
r2=mcreate(10,1,'Dependant')

do i=1 to 10
   do j=1 to 5
      val=word(reg.i,j)
      call mset r1,i,j,val
   end
   val=word(reg.i,6)
   caLL mset r2,i,1,val
end
return r1