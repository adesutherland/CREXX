/* RXPP */
/* ----------------------------------------------------------------------
 * PRE Compiled on 31 Dec 2025  at 01:23:21
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
/* ##cflags nbuf nbuf nbuf */
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
##stem Order.indx= 'IBM'              ## this is standard CREXX Array root +1 tail
Order.indx= 'IBM'              ## this is standard CREXX Array root +1 tail
/* test stem statements which are not stems */
say '+++ STEM Edge Tests +++'
say '-----------------------'
say 1.1 'wait...'
say 1.2 '1.2.3'
say 1.3 192.168.0.1
say 1.4 'my.data.file.txt'
say 1.5 3.14159
say 1.6 1..10
/* now set valid stems with several variations */
  customer = 'IBM'
  orderid  = '123'
  customer2 = 'MS'
  orderid2  = '9001'
  customer3 = 'Unamed'
/* -- stem Customer.customer.name = 'International Business Machines' */
src=putstem("Customer."customer"."name, 'International Business Machines')
/* -- stem say 2. 'Customer 'Customer.customer.name */
_v1=getstem("Customer."customer"."name)
say 2. 'Customer '_v1
/* -- stem Customer.customer.name='International Business Machines' */
src=putstem("Customer."customer"."name,'International Business Machines')
/* -- stem Customer.customer.name    =    'International Business Machines' */
src=putstem("Customer."customer"."name,    'International Business Machines')
/* -- stem Customer.customer.name3 = Customer.customer2.name */
_v1=getstem("Customer."customer2"."name)
src=putstem("Customer."customer"."name3, _v1)
/* -- stem say 3. "stem to stem "Customer.customer.name */
_v1=getstem("Customer."customer"."name)
say 3. "stem to stem "_v1
/* -- stem Customer.customer2.name = 'Microsoft' */
src=putstem("Customer."customer2"."name, 'Microsoft')
/* -- stem Customer.123.name = 'OrderName' */
src=putstem("Customer."123"."name, 'OrderName')
/* say/if on valid stems with several variations */
/* -- stem say 4. Customer.customer.name */
_v1=getstem("Customer."customer"."name)
say 4. _v1
/* -- stem if Customer.customer.name = 'IBM' then say 3. 'ok' */
_v1=getstem("Customer."customer"."name)
if _v1 = 'IBM' then say 3. 'ok'
/* -- stem say 5. Customer.customer.name Customer.customer.city */
_v1=getstem("Customer."customer"."name)
_v2=getstem("Customer."customer"."city)
say 5. _v1 _v2
/* -- stem k = Customer.customer.name */
_v1=getstem("Customer."customer"."name)
k= _v1
say 6. 'by set of k   'k
/* -- stem say 7. 'direct say  'Customer.customer.name */
_v1=getstem("Customer."customer"."name)
say 7. 'direct say  '_v1
say 8. "Customer.customer.name"
/* -- stem say 9. Customer.customer.name 'Customer.customer.city' */
_v1=getstem("Customer."customer"."name)
say 9. _v1 'Customer.customer.city'
say 10. 'He said "Customer.customer.name" today'
/* -- stem say 11. Customer.customer.name /* Customer.customer.city */ */
_v1=getstem("Customer."customer"."name)
say 11. _v1
say 12. '/* Customer.customer.name */'
say '+++ STEM DEMO START +++'
say '-----------------------'
/* -----------------------------------------------------------------------
 * 0) Master data (“table”)
 * ---------------------------------------------------------------------- */
 customer = 'IBM'
 orderid  = '123'
 customer2 = 'MS'
 orderid2  = '9001'
/* -- stem Customer.customer.name = 'International Business Machines' */
src=putstem("Customer."customer"."name, 'International Business Machines')
/* -- stem Customer.customer.city = 'Armonk' */
src=putstem("Customer."customer"."city, 'Armonk')
/* -- stem Customer.customer2.name  = 'Microsoft' */
src=putstem("Customer."customer2"."name, 'Microsoft')
/* -- stem Customer.customer2.city  = 'Redmond' */
src=putstem("Customer."customer2"."city, 'Redmond')
/* -----------------------------------------------------------------------
* 1. Create 2 orders using dynamic segments
*    Here, customer and orderid ARE variables, so segments become dynamic.
 * ----------------------------------------------------------------------- */
say "----- 1. Create 2 orders using dynamic segments"
/* -- stem Order.customer.orderid.date   = '2025-12-20' */
src=putstem("Order."customer"."orderid"."date, '2025-12-20')
/* -- stem Order.customer.orderid.amount = '149.95' */
src=putstem("Order."customer"."orderid"."amount, '149.95')
/* -- stem Order.customer.orderid.status = 'OPEN' */
src=putstem("Order."customer"."orderid"."status, 'OPEN')
/* -- stem Order.customer2.orderid2.date   = '2025-12-21' */
src=putstem("Order."customer2"."orderid2"."date, '2025-12-21')
/* -- stem Order.customer2.orderid2.amount = '399.00' */
src=putstem("Order."customer2"."orderid2"."amount, '399.00')
/* -- stem Order.customer2.orderid2.status = 'CLOSED' */
src=putstem("Order."customer2"."orderid2"."status, 'CLOSED')
/* -----------------------------------------------------------------------
* 2) Read back the two orders, show join to Customer.<cust>.name/city
 * ----------------------------------------------------------------------- */
say "----- 2. Read back the two orders, show join to Customer.<cust>.name/city"
customer = 'IBM'
orderid  = '123'
/* -- stem say 2.1 'Order ' orderid' for 'Customer.customer.name' ( 'Customer.customer.city' ) date='Order.customer.orderid.date' amount='Order.customer.orderid.amount' status=' Order.customer.orderid.status */
_v1=getstem("Customer."customer"."name)
_v2=getstem("Customer."customer"."city)
_v3=getstem("Order."customer"."orderid"."date)
_v4=getstem("Order."customer"."orderid"."amount)
_v5=getstem("Order."customer"."orderid"."status)
say 2.1 'Order ' orderid' for '_v1' ( '_v2' ) date='_v3' amount='_v4' status=' _v5
customer = 'MS'
orderid  = '9001'
/* -- stem say 2.2 'Order ' orderid' for 'Customer.customer.name' ( 'Customer.customer.city' ) date='Order.customer.orderid.date' amount='Order.customer.orderid.amount' status=' Order.customer.orderid.status */
_v1=getstem("Customer."customer"."name)
_v2=getstem("Customer."customer"."city)
_v3=getstem("Order."customer"."orderid"."date)
_v4=getstem("Order."customer"."orderid"."amount)
_v5=getstem("Order."customer"."orderid"."status)
say 2.2 'Order ' orderid' for '_v1' ( '_v2' ) date='_v3' amount='_v4' status=' _v5
/* -----------------------------------------------------------------------
 * 3) Join-style lookup via foreign key stored in a stem:
*    Order.1001.cust = 'IBM' then Customer.<that>.name
 *
 * IMPORTANT: Here we avoid a variable named "cust" being mistaken elsewhere;
* we *do* want cust to be dynamic, so we use variable custref and then use it.
 * ----------------------------------------------------------------------- */
say "----- 3. Join-style lookup via foreign key stored in a stem"
/* -- stem Order.1001.cust = 'IBM' */
src=putstem("Order."1001"."cust, 'IBM')
/* -- stem Order.1001.sum  = '888.00' */
src=putstem("Order."1001"."sum, '888.00')
/* custref is a variable so Customer.custref.name becomes Customer.<value>.name */
/* -- stem custref = Order.1001.cust */
_v1=getstem("Order."1001"."cust)
custref= _v1
say 3.1 "Cust-REF='"custref"'"
/* -- stem say 3.2 'Join demo: Order 1001 belongs to ' Customer.custref.name', city=' Customer.custref.city', total=' Order.1001.sum */
_v1=getstem("Customer."custref"."name)
_v2=getstem("Customer."custref"."city)
_v3=getstem("Order."1001"."sum)
say 3.2 'Join demo: Order 1001 belongs to ' _v1', city=' _v2', total=' _v3
/* -----------------------------------------------------------------------
 * 4) Function usage (no nesting): SUBSTR on a stem value
 * ----------------------------------------------------------------------- */
say "----- 4. Function usage (no nesting): SUBSTR on a stem value"
/* -- stem Log.1.msg = '2025-12-19 12:31:10|WARN|disk usage 93%' */
src=putstem("Log."1"."msg, '2025-12-19 12:31:10|WARN|disk usage 93%')
/* -- stem Log.2.msg = '2025-12-20 12:31:11|INFO|cleanup done' */
src=putstem("Log."2"."msg, '2025-12-20 12:31:11|INFO|cleanup done')
/* -- stem date1 = substr(Log.1.msg, 1, 10) */
_v1=getstem("Log."1"."msg)
date1= substr(_v1, 1, 10)
/* -- stem lev1  = substr(Log.1.msg, 21, 4) */
_v1=getstem("Log."1"."msg)
lev1= substr(_v1, 21, 4)
/* -- stem  say 4.1 'Log.1 date=' date1 ' level=' lev1 ' msg="' Log.1.msg '"' */
_v1=getstem("Log."1"."msg)
say 4.1 'Log.1 date=' date1 ' level=' lev1 ' msg="' _v1 '"'
/* -----------------------------------------------------------------------
 * 5) Comparisons on stem values
 * ----------------------------------------------------------------------- */
say "----- 5.  Comparisons on stem values"
customer='IBM'
orderid='123'
/* -- stem if Order.customer.orderid.status = 'OPEN' then say 'Order is OPEN -> process' ; else say 'Order not open' */
_v1=getstem("Order."customer"."orderid"."status)
if _v1 = 'OPEN' then say 'Order is OPEN -> process' ; else say 'Order not open'
customer2='MS'
orderid2='9001'
/* -- stem if Order.customer.orderid.amount = Order.customer2.orderid2.amount then say 'Amounts equal (unlikely)' ; else say 'Amounts differ (expected)' */
_v1=getstem("Order."customer"."orderid"."amount)
_v2=getstem("Order."customer2"."orderid2"."amount)
if _v1 = _v2 then say 'Amounts equal (unlikely)' ; else say 'Amounts differ (expected)'
/* -----------------------------------------------------------------------
 * 6) Derived key: count by status (simple demo)
* NOTE: this is “demo grade” (overwrites), but shows grouping keys.
 * ----------------------------------------------------------------------- */
say "----- 6. Derived key: count by status (simple demo)"
customer='IBM'
orderid='123'
/* -- stem st = Order.customer.orderid.status */
_v1=getstem("Order."customer"."orderid"."status)
st= _v1
/* -- stem Summary.st.count = '1' */
src=putstem("Summary."st"."count, '1')
customer='MS'
orderid='9001'
/* -- stem st = Order.customer.orderid.status */
_v1=getstem("Order."customer"."orderid"."status)
st= _v1
/* -- stem Summary.st.count = '1' */
src=putstem("Summary."st"."count, '1')
/* -- stem say 'Summary OPEN count=' Summary.OPEN.count ' CLOSED count=' Summary.CLOSED.count */
_v1=getstem("Summary."OPEN"."count)
_v2=getstem("Summary."CLOSED"."count)
say 'Summary OPEN count=' _v1 ' CLOSED count=' _v2
/* ------------------------------------------------------------------ */
/* 7. liststems(): show all active stems                             */
/* ------------------------------------------------------------------ */
say ">> 7. liststems(allstems)"
allstems = .string[]
say "   Number of active stems: "liststems(allstems)
do i = 1 to allstems.0
say "   - "allstems.i
end
say ' '
/* ------------------------------------------------------------------ */
/* 7) Bulk retrieval using getall()                                   */
/* ------------------------------------------------------------------ */
tails = .string[]
vals  = .string[]
say ">> 7. getall('Customer', tails, vals) – showing first 10 entries, they are unsorted"
say getall('Customer.', tails, vals)
maxShow = min(10000, tails.0)
do i = 1 to maxShow
say "   "right(i,3)". "tails[i]" = '"vals[i]"'"
end
say "   (total entries reported by getall: "tails.0")"
say ">> 7. getall('Order', tails, vals) – showing first 10 entries, they are unsorted"
say getall('Order.', tails, vals)
maxShow = min(10000, tails.0)
do i = 1 to maxShow
say "   "right(i,3)". "tails[i]" = '"vals[i]"'"
end
say "   (total entries reported by getall: "tails.0")"
say
/* ------------------------------------------------------------------ */
/* 8) getalltails() for key enumeration                               */
/* ------------------------------------------------------------------ */
say ">> 8. getalltails('Fred', tails) – showing first 10 tails"
call getalltails 'Fred.', tails
do i = 1 to tails.0
say "   "right(i,3)". "tails[i]
end
say "   (total tails reported by getalltails: "tails.0")"
say
say '--- STEM DEMO END ---'
exit
