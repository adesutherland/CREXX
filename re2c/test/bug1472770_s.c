/* Generated by re2c */
#line 1 "bug1472770_s.re"
// re2c $INPUT -o $OUTPUT -s
#define NULL ((char*) 0)
#define YYCTYPE char
#define YYCURSOR p
#define YYLIMIT p
#define YYMARKER q
#define YYFILL(n)

#include <stdio.h>

int scan(char *p)
{
	int n = 0;
	char *q;
	
	printf("[--------------\n");
	printf("%s\n", p);
	printf("]--------------\n");
start:

#line 24 "bug1472770_s.c"
{
	YYCTYPE yych;
	goto yy0;
yy1:
	++YYCURSOR;
yy0:
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= 0x00) goto yy3;
	if (yych == '\n') goto yy5;
	goto yy1;
yy3:
	++YYCURSOR;
#line 27 "bug1472770_s.re"
	{
		return n;
	}
#line 42 "bug1472770_s.c"
yy5:
	++YYCURSOR;
#line 22 "bug1472770_s.re"
	{
		++n;
		goto start;
	}
#line 50 "bug1472770_s.c"
}
#line 30 "bug1472770_s.re"

}

int main(int argc, char **argv)
{
	int n = 0;
	char *largv[4];

	if (argc < 2)
	{
		argc = 4;
		argv = largv;
		argv[1] = "";
		argv[2] = "1\n\n";
		argv[3] = "1\n2\n";
	}
	while(++n < argc)
	{
		printf("%d\n", scan(argv[n]));
	}
	return 0;
}
