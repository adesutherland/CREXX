#line 1 "../../../benchmarks/submatch_dfa_aot/src/ragel/10__alt1_2.rl"
#include <assert.h>
#include "ragel/common.c"

const char *delim = "\n";


#line 22 "../../../benchmarks/submatch_dfa_aot/src/ragel/10__alt1_2.rl"



#line 13 "gen/ragel/10__alt1_2.c"
static const int m_start = 3;
static const int m_first_final = 3;
static const int m_error = 0;

static const int m_en_main = 3;


#line 24 "../../../benchmarks/submatch_dfa_aot/src/ragel/10__alt1_2.rl"


static void lex(Input *in, Output *out)
{
	char *p = in->p;
	char *pe = in->pe;
	const char
	*a1, *b1,
	*a2, *b2;
	int cs;
	
	
#line 34 "gen/ragel/10__alt1_2.c"
	{
		cs = (int)m_start;
	}
	
#line 35 "../../../benchmarks/submatch_dfa_aot/src/ragel/10__alt1_2.rl"
	
	
#line 42 "gen/ragel/10__alt1_2.c"
	{
		switch ( cs ) {
			case 3:
			goto st_case_3;
			case 0:
			goto st_case_0;
			case 1:
			goto st_case_1;
			case 2:
			goto st_case_2;
		}
		_ctr2:
		{
#line 13 "../../../benchmarks/submatch_dfa_aot/src/ragel/10__alt1_2.rl"
			a2 = p; b1 = b2 = NULL; }
		
#line 59 "gen/ragel/10__alt1_2.c"
		
		{
#line 15 "../../../benchmarks/submatch_dfa_aot/src/ragel/10__alt1_2.rl"
			
			if (a1)      { outc(out, 'A'); outs(out, a1, a2); }
			else if (b1) { outc(out, 'B'); outs(out, b1, b2); }
			outc(out, '\n');
		}
		
#line 69 "gen/ragel/10__alt1_2.c"
		
		goto _st3;
		_ctr4:
		{
#line 12 "../../../benchmarks/submatch_dfa_aot/src/ragel/10__alt1_2.rl"
			b2 = p; a1 = a2 = NULL; }
		
#line 77 "gen/ragel/10__alt1_2.c"
		
		{
#line 15 "../../../benchmarks/submatch_dfa_aot/src/ragel/10__alt1_2.rl"
			
			if (a1)      { outc(out, 'A'); outs(out, a1, a2); }
			else if (b1) { outc(out, 'B'); outs(out, b1, b2); }
			outc(out, '\n');
		}
		
#line 87 "gen/ragel/10__alt1_2.c"
		
		goto _st3;
		_ctr6:
		{
#line 12 "../../../benchmarks/submatch_dfa_aot/src/ragel/10__alt1_2.rl"
			b1 = p; }
		
#line 95 "gen/ragel/10__alt1_2.c"
		
		{
#line 12 "../../../benchmarks/submatch_dfa_aot/src/ragel/10__alt1_2.rl"
			b2 = p; a1 = a2 = NULL; }
		
#line 101 "gen/ragel/10__alt1_2.c"
		
		{
#line 13 "../../../benchmarks/submatch_dfa_aot/src/ragel/10__alt1_2.rl"
			a1 = p; }
		
#line 107 "gen/ragel/10__alt1_2.c"
		
		{
#line 13 "../../../benchmarks/submatch_dfa_aot/src/ragel/10__alt1_2.rl"
			a2 = p; b1 = b2 = NULL; }
		
#line 113 "gen/ragel/10__alt1_2.c"
		
		{
#line 15 "../../../benchmarks/submatch_dfa_aot/src/ragel/10__alt1_2.rl"
			
			if (a1)      { outc(out, 'A'); outs(out, a1, a2); }
			else if (b1) { outc(out, 'B'); outs(out, b1, b2); }
			outc(out, '\n');
		}
		
#line 123 "gen/ragel/10__alt1_2.c"
		
		goto _st3;
		_st3:
		p+= 1;
		st_case_3:
		if ( p == pe )
			goto _out3;
		switch( ( (*( p))) ) {
			case 10: {
				goto _ctr6;
			}
			case 97: {
				goto _ctr7;
			}
			case 98: {
				goto _ctr8;
			}
		}
		goto _st0;
		_st0:
		st_case_0:
		goto _out0;
		_ctr7:
		{
#line 13 "../../../benchmarks/submatch_dfa_aot/src/ragel/10__alt1_2.rl"
			a1 = p; }
		
#line 151 "gen/ragel/10__alt1_2.c"
		
		goto _st1;
		_st1:
		p+= 1;
		st_case_1:
		if ( p == pe )
			goto _out1;
		switch( ( (*( p))) ) {
			case 10: {
				goto _ctr2;
			}
			case 97: {
				goto _st1;
			}
		}
		goto _st0;
		_ctr8:
		{
#line 12 "../../../benchmarks/submatch_dfa_aot/src/ragel/10__alt1_2.rl"
			b1 = p; }
		
#line 173 "gen/ragel/10__alt1_2.c"
		
		goto _st2;
		_st2:
		p+= 1;
		st_case_2:
		if ( p == pe )
			goto _out2;
		switch( ( (*( p))) ) {
			case 10: {
				goto _ctr4;
			}
			case 98: {
				goto _st2;
			}
		}
		goto _st0;
		_out3: cs = 3; goto _out; 
		_out0: cs = 0; goto _out; 
		_out1: cs = 1; goto _out; 
		_out2: cs = 2; goto _out; 
		_out: {}
	}
	
#line 36 "../../../benchmarks/submatch_dfa_aot/src/ragel/10__alt1_2.rl"
	
	
	in->p = p;
	in->pe = pe;
}
