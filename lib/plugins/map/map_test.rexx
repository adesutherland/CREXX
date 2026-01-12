/* RXPP */
/* ----------------------------------------------------------------------
 * PRE Compiled on 12 Jan 2026  at 18:13:41
 * ----------------------------------------------------------------------
 */
options levelb
import rxfnsb
/* RXPP
 * ----------------------------------------------------------------------
 *  CREXX Stem Engine / Robin Hood Hash Demo
 *
 *  Demonstrates:
 *    - Multi-level stems (a.b.c.n)
 *    - setstem / getstem
 *    - stemstat / liststems
 *    - dropstem / dropEntry and error messages via getstemmsg
 *    - Iteration via getstemtail / getall / getalltails
 *    - clonestem (deep copy of a stem)
 *
 * ----------------------------------------------------------------------
 */
import map
/* ##cflags nbuf nbuf nbuf dotisstem docstem */
/* (1) Two stems on one line: condition + then-part */
/* ++++++++++++++++ if List.(213+4711).(harry||rose)\='' then say "OK is "List.(213+4711).(harry||Mary) */
_expr_1=harry||rose""
_expr_2=213+4711""
_tmp_rxpp_1 =getstem("List."_expr_2"."_expr_1)
_expr_1=harry||Mary""
_expr_2=213+4711""
_tmp_rxpp_2 =getstem("List."_expr_2"."_expr_1)
if _tmp_rxpp_1\='' then say "OK is "_tmp_rxpp_2
say "'"_expr_2"'"
say "End of Stem Test 1"
/* (2) Nested computed tail */
/* ++++++++++++++++ say a.(b.(c.d)).e.f */
_tmp_rxpp_3 =getstem("c."d)
_expr_1=_tmp_rxpp_3""
_tmp_rxpp_4 =getstem("b."_expr_1)
_expr_2=_tmp_rxpp_4""
_tmp_rxpp_5 =getstem("a."_expr_2"."e"."f)
say "End of Stem Test 2"
say _tmp_rxpp_5
/* (3) Function call with dots inside computed tail */
/* ++++++++++++++++ say stem.(substr(another.tail1.xyz,7,9)).tail.xend */
_tmp_rxpp_6 =getstem("another."tail1"."xyz)
_expr_1=substr(_tmp_rxpp_6,7,9)""
_tmp_rxpp_7 =getstem("stem."_expr_1"."tail"."xend)
say _tmp_rxpp_7
say "End of Stem Test 3"
/*
/* (4) Illegal: bare parentheses after computed tail (must reject) */
say stem.(abc)(def).x.y
say "End of Stem Test 4"
*/
/* =======================================================================
 * CREXX Preprocessor STEM Demo (All-in-One) — matches CURRENT semantics
 *
 * Current semantics:
 *   Record.customer.addr
 *     - segment "customer" is substituted with the VALUE of variable customer
 *       IF the variable customer exists, else it remains literal "customer"
 *
 * This demo:
 *   1) Defines 2 customers and 2 orders using dynamic segments (via existing vars)
 *   2) Shows “join” lookup: Order.1001.cust -> Customer.<cust>.name
 *   3) Uses SUBSTR on a stem value
 *   4) Uses concatenation (||) and quoted strings
 *   5) Shows comparisons and a small “derived key” example
 * ======================================================================= */
/* ================================================================
 * STEM detection test cases (1–31)
 * Designed to stress the stem detection
 * ================================================================ */
/* Customer.customer.name = 'IBM' */  ## stem within comments
## Customer.customer.name = 'IBM'     ## stem within comments
indx=1
/* ++++++++++++++++ Order.indx= 'IBM'              ## this is standard CREXX Array root +1 tail */
src=putstem("Order."indx, 'IBM'              )
/* test stem statements which are not stems */
say '+++ STEM Edge Tests +++'
say '-----------------------'
say '1.1' 'wait...'
say '1.2' '1.2.3'
say '1.3' 192.168.0.1
say '1.4' 'my.data.file.txt'
say '1.5' 3.14159
say '1.6' 1..10
/* \\\\\ Stem expansion warning:  bad-name-segment: say '1.6' 1..10 */
/* now set valid stems with several variations */
  customer = 'IBM'
  orderid  = '123'
  customer2 = 'MS'
  orderid2  = '9001'
  customer3 = 'Unamed'
/* ++++++++++++++++ Customer.(abc||4711).name = 'Microsoft' */
_expr_1=abc||4711""
src=putstem("Customer."_expr_1"."name, 'Microsoft')
/* ++++++++++++++++ Customer.customer.name = 'International Business Machines' */
src=putstem("Customer."customer"."name, 'International Business Machines')
/* ----------------------------------------------------------------- */
/* ++++++++++++++++ say '2.' 'Customer 'Customer.customer.name */
_tmp_rxpp_8 =getstem("Customer."customer"."name)
say '2.' 'Customer '_tmp_rxpp_8
/* ++++++++++++++++ Customer.customer.name='International Business Machines' */
src=putstem("Customer."customer"."name,'International Business Machines')
/* ++++++++++++++++ Customer.customer.name    =    'International Business Machines' */
src=putstem("Customer."customer"."name,    'International Business Machines')
/* ++++++++++++++++ Customer.customer.name3 = Customer.customer2.name */
_tmp_rxpp_9 =getstem("Customer."customer2"."name)
src=putstem("Customer."customer"."name3, _tmp_rxpp_9)
/* ----------------------------------------------------------------- */
/* ++++++++++++++++ say '3.' "stem to stem "Customer.customer.name */
_tmp_rxpp_10 =getstem("Customer."customer"."name)
say '3.' "stem to stem "_tmp_rxpp_10
/* ++++++++++++++++ Customer.customer2.name = 'Microsoft' */
src=putstem("Customer."customer2"."name, 'Microsoft')
/* ++++++++++++++++ Customer.123.name = 'OrderName' */
src=putstem("Customer."123"."name, 'OrderName')
/* say/if on valid stems with several variations */
/* ----------------------------------------------------------------- */
/* ++++++++++++++++ say '4.' Customer.customer.name */
_tmp_rxpp_11 =getstem("Customer."customer"."name)
say '4.' _tmp_rxpp_11
/* ++++++++++++++++ if Customer.customer.name = 'IBM' then say 3. 'ok' */
_tmp_rxpp_12 =getstem("Customer."customer"."name)
if _tmp_rxpp_12 = 'IBM' then say 3. 'ok'
/* \\\\\ Stem expansion warning:  bad-name-segment: if Customer.customer.name = 'IBM' then say 3. 'ok' */
/* ----------------------------------------------------------------- */
/* ++++++++++++++++ say '5.' Customer.customer.name Customer.customer.city */
_tmp_rxpp_13 =getstem("Customer."customer"."name)
_tmp_rxpp_14 =getstem("Customer."customer"."city)
say '5.' _tmp_rxpp_13 _tmp_rxpp_14
/* ----------------------------------------------------------------- */
/* ++++++++++++++++ k = Customer.customer.name */
_tmp_rxpp_15 =getstem("Customer."customer"."name)
k= _tmp_rxpp_15
say '6.' 'by set of k   'k
/* ----------------------------------------------------------------- */
/* ++++++++++++++++ say '7.' 'direct say  'Customer.customer.name */
_tmp_rxpp_16 =getstem("Customer."customer"."name)
say '7.' 'direct say  '_tmp_rxpp_16
/* ----------------------------------------------------------------- */
say '8.' "Customer.customer.name"
/* ----------------------------------------------------------------- */
/* ++++++++++++++++ say '9.' Customer.customer.name 'Customer.customer.city' */
_tmp_rxpp_17 =getstem("Customer."customer"."name)
say '9.' _tmp_rxpp_17 'Customer.customer.city'
/* ----------------------------------------------------------------- */
say '10.' 'He said "Customer.customer.name" today'
/* ----------------------------------------------------------------- */
/* ++++++++++++++++ say '11.' Customer.customer.name /* Customer.customer.city */ */
_tmp_rxpp_18 =getstem("Customer."customer"."name)
say '11.' _tmp_rxpp_18 
/* ----------------------------------------------------------------- */
say '12.' '/* Customer.customer.name */'
say '+++ STEM DEMO START +++'
say '-----------------------'
/* -----------------------------------------------------------------------
 * 0) Master data (“table”)
 * ---------------------------------------------------------------------- */
 customer = 'IBM'
 orderid  = '123'
 customer2 = 'MS'
 orderid2  = '9001'
/* ++++++++++++++++ Customer.customer.name = 'International Business Machines' */
src=putstem("Customer."customer"."name, 'International Business Machines')
/* ++++++++++++++++ Customer.customer.city = 'Armonk' */
src=putstem("Customer."customer"."city, 'Armonk')
/* ++++++++++++++++ Customer.customer2.name  = 'Microsoft' */
src=putstem("Customer."customer2"."name, 'Microsoft')
/* ++++++++++++++++ Customer.customer2.city  = 'Redmond' */
src=putstem("Customer."customer2"."city, 'Redmond')
/* -----------------------------------------------------------------------
 * 1. Create 2 orders using dynamic segments
 *    Here, customer and orderid ARE variables, so segments become dynamic.
 * ----------------------------------------------------------------------- */
say "----- 1. Create 2 orders using dynamic segments"
/* ++++++++++++++++ Order.customer.orderid.date   = '2025-12-20' */
src=putstem("Order."customer"."orderid"."date, '2025-12-20')
/* ++++++++++++++++ Order.customer.orderid.amount = '149.95' */
src=putstem("Order."customer"."orderid"."amount, '149.95')
/* ++++++++++++++++ Order.customer.orderid.status = 'OPEN' */
src=putstem("Order."customer"."orderid"."status, 'OPEN')
/* ++++++++++++++++ Order.customer2.orderid2.date   = '2025-12-21' */
src=putstem("Order."customer2"."orderid2"."date, '2025-12-21')
/* ++++++++++++++++ Order.customer2.orderid2.amount = '399.00' */
src=putstem("Order."customer2"."orderid2"."amount, '399.00')
/* ++++++++++++++++ Order.customer2.orderid2.status = 'CLOSED' */
src=putstem("Order."customer2"."orderid2"."status, 'CLOSED')
/* -----------------------------------------------------------------------
 * 2) Read back the two orders, show join to Customer.<cust>.name/city
 * ----------------------------------------------------------------------- */
say "----- 2. Read back the two orders, show join to Customer.<cust>.name/city"
customer = 'IBM'
orderid  = '123'
/* ++++++++++++++++ say '2.1' 'Order ' orderid' for 'Customer.customer.name' ( 'Customer.customer.city' ) date='Order.customer.orderid.date' amount='Order.customer.orderid.amount' status=' Order.customer.orderid.status */
_tmp_rxpp_19 =getstem("Customer."customer"."name)
_tmp_rxpp_20 =getstem("Customer."customer"."city)
_tmp_rxpp_21 =getstem("Order."customer"."orderid"."date)
_tmp_rxpp_22 =getstem("Order."customer"."orderid"."amount)
_tmp_rxpp_23 =getstem("Order."customer"."orderid"."status)
say '2.1' 'Order ' orderid' for '_tmp_rxpp_19' ( '_tmp_rxpp_20' ) date='_tmp_rxpp_21' amount='_tmp_rxpp_22' status=' _tmp_rxpp_23
customer = 'MS'
orderid  = '9001'
/* ++++++++++++++++ say '2.2' 'Order ' orderid' for 'Customer.customer.name' ( 'Customer.customer.city' ) date='Order.customer.orderid.date' amount='Order.customer.orderid.amount' status=' Order.customer.orderid.status */
_tmp_rxpp_24 =getstem("Customer."customer"."name)
_tmp_rxpp_25 =getstem("Customer."customer"."city)
_tmp_rxpp_26 =getstem("Order."customer"."orderid"."date)
_tmp_rxpp_27 =getstem("Order."customer"."orderid"."amount)
_tmp_rxpp_28 =getstem("Order."customer"."orderid"."status)
say '2.2' 'Order ' orderid' for '_tmp_rxpp_24' ( '_tmp_rxpp_25' ) date='_tmp_rxpp_26' amount='_tmp_rxpp_27' status=' _tmp_rxpp_28
/* -----------------------------------------------------------------------
 * 3) Join-style lookup via foreign key stored in a stem:
 *    Order.1001.cust = 'IBM' then Customer.<that>.name
 *
 * IMPORTANT: Here we avoid a variable named "cust" being mistaken elsewhere;
 * we *do* want cust to be dynamic, so we use variable custref and then use it.
 * ----------------------------------------------------------------------- */
say "----- 3. Join-style lookup via foreign key stored in a stem"
/* ++++++++++++++++ Order.1001.cust = 'IBM' */
src=putstem("Order."1001"."cust, 'IBM')
/* ++++++++++++++++ Order.1001.sum  = '888.00' */
src=putstem("Order."1001"."sum, '888.00')
/* custref is a variable so Customer.custref.name becomes Customer.<value>.name */
/* ++++++++++++++++ custref = Order.1001.cust */
_tmp_rxpp_29 =getstem("Order."1001"."cust)
custref= _tmp_rxpp_29
say '3.1' "Cust-REF='"custref"'"
/* ++++++++++++++++ say '3.2' 'Join demo: Order 1001 belongs to ' Customer.custref.name', city=' Customer.custref.city', total=' Order.1001.sum */
_tmp_rxpp_30 =getstem("Customer."custref"."name)
_tmp_rxpp_31 =getstem("Customer."custref"."city)
_tmp_rxpp_32 =getstem("Order."1001"."sum)
say '3.2' 'Join demo: Order 1001 belongs to ' _tmp_rxpp_30', city=' _tmp_rxpp_31', total=' _tmp_rxpp_32
/* -----------------------------------------------------------------------
 * 4) Function usage (no nesting): SUBSTR on a stem value
 * ----------------------------------------------------------------------- */
say "----- 4. Function usage (no nesting): SUBSTR on a stem value"
/* ++++++++++++++++ Log.1.msg = '2025-12-19 12:31:10|WARN|disk usage 93%' */
src=putstem("Log."1"."msg, '2025-12-19 12:31:10|WARN|disk usage 93%')
/* ++++++++++++++++ Log.2.msg = '2025-12-20 12:31:11|INFO|cleanup done' */
src=putstem("Log."2"."msg, '2025-12-20 12:31:11|INFO|cleanup done')
/* ++++++++++++++++ date1 = substr(Log.1.msg, 1, 10) */
_tmp_rxpp_33 =getstem("Log."1"."msg)
date1= substr(_tmp_rxpp_33, 1, 10)
/* ++++++++++++++++ lev1  = substr(Log.1.msg, 21, 4) */
_tmp_rxpp_34 =getstem("Log."1"."msg)
lev1= substr(_tmp_rxpp_34, 21, 4)
/* ++++++++++++++++  say '4.1' 'Log.1 date=' date1 ' level=' lev1 ' msg="' Log.1.msg '"' */
_tmp_rxpp_35 =getstem("Log."1"."msg)
 say '4.1' 'Log.1 date=' date1 ' level=' lev1 ' msg="' _tmp_rxpp_35 '"'
/* -----------------------------------------------------------------------
 * 5) Comparisons on stem values
 * ----------------------------------------------------------------------- */
say "----- 5.  Comparisons on stem values"
customer='IBM'
orderid='123'
/* ++++++++++++++++ if Order.customer.orderid.status = 'OPEN' then say 'Order is OPEN -> process' ; else say 'Order not open' */
_tmp_rxpp_36 =getstem("Order."customer"."orderid"."status)
if _tmp_rxpp_36 = 'OPEN' then say 'Order is OPEN -> process' ; else say 'Order not open'
customer2='MS'
orderid2='9001'
/* ++++++++++++++++ if Order.customer.orderid.amount = Order.customer2.orderid2.amount then say 'Amounts equal (unlikely)' ; else say 'Amounts differ (expected)' */
_tmp_rxpp_37 =getstem("Order."customer"."orderid"."amount)
_tmp_rxpp_38 =getstem("Order."customer2"."orderid2"."amount)
if _tmp_rxpp_37 = _tmp_rxpp_38 then say 'Amounts equal (unlikely)' ; else say 'Amounts differ (expected)'
/* -----------------------------------------------------------------------
 * 6) Derived key: count by status (simple demo)
 * NOTE: this is “demo grade” (overwrites), but shows grouping keys.
 * ----------------------------------------------------------------------- */
 say "----- 6. Derived key: count by status (simple demo)"
customer='IBM'
orderid='123'
/* ++++++++++++++++ st = Order.customer.orderid.status */
_tmp_rxpp_39 =getstem("Order."customer"."orderid"."status)
st= _tmp_rxpp_39
/* ++++++++++++++++ Summary.st.count = '1' */
src=putstem("Summary."st"."count, '1')
customer='MS'
orderid='9001'
/* ++++++++++++++++ st = Order.customer.orderid.status */
_tmp_rxpp_40 =getstem("Order."customer"."orderid"."status)
st= _tmp_rxpp_40
/* ++++++++++++++++ Summary.st.count = '1' */
src=putstem("Summary."st"."count, '1')
/* ++++++++++++++++ say 'Summary OPEN count=' Summary.OPEN.count ' CLOSED count=' Summary.CLOSED.count */
_tmp_rxpp_41 =getstem("Summary."OPEN"."count)
_tmp_rxpp_42 =getstem("Summary."CLOSED"."count)
say 'Summary OPEN count=' _tmp_rxpp_41 ' CLOSED count=' _tmp_rxpp_42
/* ------------------------------------------------------------------ */
/* 7. liststems(): show all active stems                             */
/* ------------------------------------------------------------------ */
say ">> 7. liststems(allstems)"
allstems = .string[]
say "   Number of active stems: "liststems(allstems)
do i = 1 to allstems[0]
   say "   - "allstems[i]
end
say ' '
/* ------------------------------------------------------------------ */
/* 7) Bulk retrieval using getall()                                   */
/* ------------------------------------------------------------------ */
tails = .string[]
vals  = .string[]
say ">> 7. getall('Customer', tails, vals) – showing first 10 entries, they are unsorted"
say getall('Customer.', tails, vals)
maxShow = min(10000, tails[0])
do i = 1 to maxShow
   say "   "right(i,3)". "tails[i]" = '"vals[i]"'"
end
say "   (total entries reported by getall: "tails[0]")"
say ">> 7. getall('Order', tails, vals) – showing first 10 entries, they are unsorted"
say getall('Order.', tails, vals)
maxShow = min(10000, tails[0])
do i = 1 to maxShow
   say "   "right(i,3)". "tails[i]" = '"vals[i]"'"
end
say "   (total entries reported by getall: "tails[0]")"
say
/* ------------------------------------------------------------------ */
/* 8) getalltails() for key enumeration                               */
/* ------------------------------------------------------------------ */
say ">> 8. getalltails('Fred', tails) – showing first 10 tails"
call getalltails 'Fred.', tails
do i = 1 to tails[0]
   say "   "right(i,3)". "tails[i]
end
say "   (total tails reported by getalltails: "tails[0]")"
say
say ">> 8. getalltails('Fred', tails) – showing first 10 tails"
call getalltails 'List.', tails
do i = 1 to tails[0]
   say "   "right(i,3)". "tails[i]
end
say "   (total tails reported by getalltails: "tails[0]")"
say
say '--- STEM DEMO END ---'
exit
