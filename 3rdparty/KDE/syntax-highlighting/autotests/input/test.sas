/* =====================================================================
   test_sas_syntax.sas  —  minimal highlighting coverage + edge cases
   ===================================================================== */

/* ---- 1. COMMENTS --------------------------------------------------- */

/* block comment */

* statement comment at start of line;

* multi-line
  statement comment;

%* macro comment;

data _null_;
  x = 1; * inline comment after semicolon;
  y = 2; * chained; * second;
  a = 4; * comment; b = 5;   /* b=5 resumes normal highlighting */
  z = 4
  * 2;   * arithmetic * then comment on next statement;
  x2 = %* macro comment mid-expression; 4;
  y2 = /* block comment mid-expression */ 5;
run;


/* ---- 2. STRING LITERALS — all suffix types ------------------------- */

options validvarname=any;

data _null_;
  /* double-quoted */
  plain   = "hello world";
  escaped = "it's ""quoted""";
  date    = "01jan2024"d;
  dtime   = "01jan2024:12:00:00"dt;
  time    = "12:00:00"t;
  bits    = "1010"b;
  naml    = "my variable"n;
  hex     = "FFAB3C"x;

  /* single-quoted */
  sq_plain  = 'hello world';
  sq_escape = 'it''s quoted';
  sq_date   = '01jan2024'd;
  sq_dtime  = '01jan2024:12:00:00'dt;
  sq_time   = '12:00:00't;
  sq_bits   = '1010'b;
  sq_naml   = 'my variable'n;
  sq_hex    = 'FFAB3C'x;

  /* uppercase suffixes */
  uc_date  = "01jan2024"D;
  uc_dtime = "01jan2024:00:00:00"DT;
  uc_time  = "06:00:00"T;
  uc_hex   = "FF"X;
run;


/* ---- 3. NUMERIC LITERALS ------------------------------------------- */

data _null_;
  int_val  = 42;
  neg_int  = -7;
  float    = 3.14159;
  sci_pos  = 1.5e10;
  sci_neg  = 2.0E-3;
  leading  = .9;
  sas_hex  = 0c1x;
run;


/* ---- 4. FORMATS AND INFORMATS -------------------------------------- */

data _null_;
  format date_var  date9.;
  format num_var   comma12.2;
  format chr_var   $char20.;
  format best_var  best.;
  informat raw_in  $50.;
  informat dt_in   anydtdtm20.;
run;


/* ---- 5. OPERATORS -------------------------------------------------- */

data _null_;
  /* arithmetic — * and / dont trigger comment context */
  a = 2 ** 3;
  b = 15 * 3;
  c = 10 / 4;
  d = a + b - c;
  z = 15 * 3 / 4 + 100;

  /* comparison — symbolic */
  eq_  = (z =  45);
  ne_  = (z ^= 45);
  ne2  = (z ~= 45);
  gt_  = (z >  10);
  lt_  = (z <  200);
  ge_  = (z >= 10);
  le_  = (z <= 200);
  mn_  = z <> 100;   /* min operator */
  mx_  = z >< 100;   /* max operator */
  cat_ = "a" || "b"; /* concatenation */

  /* trailing-modifier operators */
  name = "Jones";
  eq_t = (name =:  "Jo");
  gt_t = (name >:  "Jo");
  ne_t = (name ^=: "Jo");

  /* mnemonic operators */
  x = 5; y = 10;
  if x gt 0 and y lt 20  then put "gt lt";
  if x ge 5 and y le 10  then put "ge le";
  if x eq 5 or  y ne 0   then put "eq ne";
  if not (x = 0)         then put "not";
  if x in (1 2 3 4 5)    then put "in";
  if name like "Jo%"     then put "like";
  if name contains "ohn" then put "contains";
  if x between 1 and 10  then put "between";
  if x is missing        then put "missing";
run;


/* ---- 6. SPECIAL / AUTOMATIC VARIABLES ------------------------------ */

data _null_;
  put _n_;
  put _error_;
  if _n_ = 1 then put "first";
  keep _all_;
  drop _character_;
  retain _numeric_;
run;


/* ---- 7. CONTROL FLOW ----------------------------------------------- */

data _null_;
  do i = 1 to 10 by 2;
    if i = 5 then continue;
    if i = 9 then leave;
    put i;
  end;

  i = 0;
  do while (i < 10);
    i + 1;
  end;

  i = 0;
  do until (i >= 10);
    i + 1;
  end;

  select (i);
    when (1)  put "one";
    when (2)  put "two";
    otherwise put "other";
  end;

  if i > 5 then put "big";
  else put "small";

  goto done;
  link sub;
  return;
  done:
  sub:
  return;
run;


/* ---- 8. DATA STEP STATEMENTS + DATASET OPTIONS --------------------- */

data work.out (keep=name age compress=yes);
  /* dataset options on SET — (opt=val) must NOT colour ds name as function */
  set sashelp.class (obs=50 where=(age > 12) keep=name age sex);
  merge work.a (in=ina) work.b (in=inb);
  by id;

  array nums{3} a b c;
  attrib name length=$20 label="Full name";
  length score 8;
  retain cumcount 0;

  /* known functions highlighted; non-function before ( is Normal Text */
  z = max(a, b);
  w = substr(name, 1, 3);
  if notafunction(z) = 1 then output;   /* notafunction -> Normal Text */

  stop;
run;


/* ---- 9. CARDS / DATALINES ------------------------------------------ */

data work.inline;
  input name $ age;
  cards;
Alice 30
Bob   25
   ;
run;

data work.cards4test;
  input line $ 1-20;
  cards4;
line with ; inside
another ; line here
;;;;
run;


/* ---- 10. PROC (GENERIC) + CONTEXT SWITCHING ------------------------ */

proc means data=sashelp.class n mean std min max;
  var age height weight;
  class sex;
  output out=work.stats mean= std= / autoname;
run;

proc sort data=sashelp.class out=work.sorted (keep=name age) nodupkey;
  by sex descending age;
run;

/* No RUN — next proc/data boundary switches context */
proc datasets library=work nolist;
  delete sorted;

data work.after_no_run;
  x = 1;
run;

/* ---- 11. PROC SQL -------------------------------------------------- */

proc sql;
  create table work.sql_out as
    select name, age, height / weight as ratio
    from   sashelp.class
    where  age > 12 and sex = 'M'
    order  by age desc;

  select count(*) as n_obs,
         sum(age * 1.0) / count(*) as mean_age
  from   sashelp.class;

  drop table work.sql_out;
quit;

/* SQL with no QUIT — next step boundary exits */
proc sql;
  select name from sashelp.class;

data work.after_sql;
  set sashelp.class;
run;


/* ---- 12. PROC PYTHON (embedded block) ------------------------------ */

proc python;
submit;
import sys, math
x = 2 ** 10 # Python comment
s = "hello"
s2 = 'world'
multi = """
triple-quoted string
"""
print(sys.version, math.pi, x)
endsubmit;
quit;


/* ---- 13. PROC LUA (embedded block) --------------------------------- */

proc lua;
submit;
-- Lua single-line comment
local x = 10 * 3     -- * is arithmetic
local s  = "hello"
local s2 = 'world'
local ml = [[multi-line string]]
--[[ multi-line Lua comment ]]
print(x, s, s2)
endsubmit;
quit;


/* ---- 14. MACRO LANGUAGE -------------------------------------------- */

%macro greet(name=World, n=1);
  %local i;
  %do i = 1 %to &n;
    %put Hello, &name! (iteration &i of &n);
  %end;
%mend greet;

%greet(name=SAS, n=3);

%let myvar = 42;
%let mystr = Hello World;

%if &myvar > 10 %then %do;
  %put myvar (&myvar) is large;
%end;
%else %do;
  %put myvar (&myvar) is small;
%end;

/* indirect macro reference */
%let prefix = abc;
%let abcval = found;
%put &prefix - simple;
%put &&prefix.val - indirect = &abcval;

/* macro quoting / utility functions */
%let q1 = %str(a semicolon: ;);
%let q2 = %nrstr(&notresolved);
%let len = %length(&mystr);
%let up  = %upcase(&mystr);
%let ev  = %eval(2 + 3);
%let sev = %sysevalf(1.5 * 2.0);

%global gvar1 gvar2;
%local  lvar1;

%* macro comment;


/* ---- 15. LIBNAME / FILENAME / OPTIONS ------------------------------ */

libname mylib "/path/to/data" access=readonly;
libname mylib clear;

filename myfile "/path/to/file.txt" lrecl=256;
filename myfile clear;

/* option=value pattern: option name -> SecKeyword, value -> Normal Text */
options obs=max nodate nonumber mprint mlogic symbolgen;
options validvarname=any compress=yes bufno=20;


/* ---- 16. MISC PROCS ------------------------------------------------ */

proc mixed data=sashelp.class;
  class sex;
  model height = age sex / solution ddfm=kr;
  random intercept / subject=name type=un;
  lsmeans sex / pdiff adjust=tukey;
run;

proc report data=sashelp.class nowd;
  column name sex age height;
  define name   / display "Student";
  define height / analysis mean format=6.2 "Avg Height";
  break after sex / summarize;
  compute after sex;
    line "--- end of group ---";
  endcomp;
run;

proc fcmp outlib=work.funcs.mathlib;
  function cube(x);
    return (x ** 3);
  endfunc;
  subroutine swap(a, b);
    outargs a, b;
    temp = a; a = b; b = temp;
  endsub;
run;

endsas;
%abort abend;
