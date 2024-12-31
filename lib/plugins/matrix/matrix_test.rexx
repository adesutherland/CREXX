/* GETPI Plugin Test */
options levelb
import rxfnsb
import matrix

rx=regdat()
ry=regression(rx,rx+1,'Regression')
call mprint ry

m1=mcreate(24,5,"Data Matrix")
call matdata m1

call mprint  m1
say "Mean   COL 1 "mmean(m1,1)
say "Stddev COL 1 "mstdev(m1,1)
say "Status M1 "stats(m1,'r')
say "Status M1 "stats(m1,'c')
m2=m1

m9=Mcorr(m1,'Correlation')
call mprint m9
## call mplot m9, "line"
## call mplot m9, "scatter"
## call mplot m9, "bar"
## call mplot m9, "heatmap"
##  - "line": Line plot
##  - "scatter": Scatter plot
##  - "bar": Bar chart
##  - "heatmap": Heatmap
ma=Mcov(m1,'Covariance')
call mprint ma

## lets do the calculation via Matrix operations
## 1. transpose Data Matrix
  m2=mtranspose(m1,"Transposed Data Matrix")
  call mprint m2

## 2. Standardise Data Matrix (mean=0, stddev=1)
  m4=mstandard(m1,"Standardised")
  call mprint  m4
## 3. transpose standardised Data Matrix
  m5=mtranspose(m4,"Transposed Data Matrix")
  call mprint m5
## 4. multiply Data Matrix with transposed Matrix
  m6=mmult(m5,m4,"close to Correlation Matrix")
  call mprint m6
## 5. Almost there, multiply by rows
  m7=mprod(m6,1/23,"Correlation Matrix")
  call mprint m7
## 6. other stuff: determinant
   say 'determinante 'mdet(m1)
## 7. other stuff: L and U Matrix
   mlx=mlu(m6,"L Matrix","U Matrix")
   call mprint mlx
   call mprint mlx+1
stats = mcolstats(m1, "Statistics")
call mprint(stats)

say '*** Data Matrix before Factor Analysis ***'
call mprint m1

say '*** Factor Analysis unrotated ***'
loadings1 = mfactor(m1, 2, 0,1, "Unrotated")
call mprint loadings1
loadings1T=mtranspose(loadings1,"Unrotated T")
l1mult=Mmult(loadings1,loadings1t,'Generated Correlation')
call mprint l1Mult
l1mult=Mmult(loadings1t,loadings1,'Generated Correlation')
call mprint l1Mult
call mprint loadings1+1
call mprint loadings1+2
call mprint loadings1+3
say '*** Factor Analysis Varimax ***'
loadings2 = mfactor(m1, 2, 1,1, "Rotated Varimax")
loadingsT=mtranspose(loadings2,"Rotated I")
l1mult=Mmult(loadings2,loadingst,'Generated Correlation')
call mprint l1Mult
l1mult=Mmult(loadingst,loadings2,'Generated Correlation')

call mprint loadings2
call mprint loadings2+1
call mprint loadings2+2
call mprint loadings2+3
say '*** Factor Analysis Quartimax ***'
loadings3 = mfactor(m1, 2, 2,1, "Rotated Quartimax")
call mprint loadings3
## call masciiplot m1, "hist"
## call masciiplot m1, "bar"
## call masciiplot m1, "line"
## call masciiplot m1, "heat"
## call masciiplot m1, "scatter"
## call masciiplot m1, "box"
call mprint loadings3+1
call mprint loadings3+2
call mprint loadings3+3
say '*** Factor Analysis Promox ***'
loadings4 = mfactor(m1, 2, 3,1, "Rotated Promax")
call mprint loadings4
call mprint loadings4+1
call mprint loadings4+2
call mprint loadings4+3


say "FREE m3 "mfree(m6)  ## free storage of m6
say "FREE m7 "mfree(m7)  ## free storage of m7
say "FREE m91 "mfree(91) ## free storage of m91, which is not there
call mfree -1            ## free all
exit

## Function to perform linear regression
regression: procedure=.int
  arg ind=.int, dep=.int, resultid=.string
  yrows=stats(ind,'Rows')
  xrows=stats(dep,'Rows')
  xcols=stats(ind,'COLs')

  if yrows <> Xrows then return -62; ## Dimension mismatch

 ## Step 1: Add a column of ones to X for the intercept
    augmented=mexpand(ind,1,1.0)
    call mprint augmented
 ## Step 2: calculate ('X*X)
    Xt  = mtranspose(augmented, "Xt");
    XtX = mmult(Xt, augmented,  "XtX");
 ## Step 3: Calculate (X'y)
    XtY = mmult(Xt, dep, "XtY");
 ## Step 4: Invert (X'X)
    XtX_inv = minvert(XtX, "XtX_inverted");
    if XtX_inv < 0 then return -21;             ## Singular matrix error
 ## Step 5: Calculate coefficients: result = (X'X)^(-1)(X'y)
    result = mmult(XtX_inv, XtY, "Regression Coefficients");
 ## Clean up temporary matrices
    call mfree(Xt);
    call mfree(XtX);
    call mfree(XtY);
    call mfree(XtX_inv);
return result


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