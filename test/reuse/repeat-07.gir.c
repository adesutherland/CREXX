/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -gir
// multiple scanners, additional rules, char width change

void scan(unsigned char* in)
{

	{
		unsigned char yych;
		if (limit1 <= cursor1) fill1(1);
		yych = *cursor1;
		{
			static void *yytarget[256] = {
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy2,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy5,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy7,  &&yy9,  &&yy11, &&yy13, &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,
				&&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3,  &&yy3
			};
			goto *yytarget[yych];
		}
yy2:
yy3:
		++cursor1;
		return ".";
yy5:
		++cursor1;
		return "1";
yy7:
		++cursor1;
		return "a";
yy9:
		++cursor1;
		return "b";
yy11:
		++cursor1;
		return "c";
yy13:
		++cursor1;
		return "d";
	}

}

void scan(unsigned short* in)
{

	{
		unsigned short yych;
		if (limit2 <= cursor2) fill2(1);
		yych = *cursor2;
		if (yych & ~0xFF) {
			goto yy18;
		} else {
			static void *yytarget[256] = {
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy17, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy20, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy22, &&yy24, &&yy26, &&yy28, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18,
				&&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18, &&yy18
			};
			goto *yytarget[yych];
		}
yy17:
yy18:
		++cursor2;
		return ".";
yy20:
		++cursor2;
		return "2";
yy22:
		++cursor2;
		return "a";
yy24:
		++cursor2;
		return "b";
yy26:
		++cursor2;
		return "c";
yy28:
		++cursor2;
		return "d";
	}

}

void scan(unsigned int* in)
{

	{
		unsigned int yych;
		if (limit3 <= cursor3) fill3(1);
		yych = *cursor3;
		if (yych & ~0xFF) {
			goto yy33;
		} else {
			static void *yytarget[256] = {
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy32, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy35, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy37, &&yy39, &&yy41, &&yy43, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33,
				&&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33, &&yy33
			};
			goto *yytarget[yych];
		}
yy32:
yy33:
		++cursor3;
		return ".";
yy35:
		++cursor3;
		return "3";
yy37:
		++cursor3;
		return "a";
yy39:
		++cursor3;
		return "b";
yy41:
		++cursor3;
		return "c";
yy43:
		++cursor3;
		return "d";
	}

}
reuse/repeat-07.gir.re:28:2: warning: control flow is undefined for strings that match '\xA', use default rule '*' [-Wundefined-control-flow]
reuse/repeat-07.gir.re:46:2: warning: control flow is undefined for strings that match '\xA', use default rule '*' [-Wundefined-control-flow]
reuse/repeat-07.gir.re:64:2: warning: control flow is undefined for strings that match '\xA', use default rule '*' [-Wundefined-control-flow]
