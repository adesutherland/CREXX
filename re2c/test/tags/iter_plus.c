/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -i --tags
// ensure 'r+' (one or more repetitions) expansion does not duplicate 'r'
// this is crucial if 'r' contains tags (tag duplication is forbidden)


{
	YYCTYPE yych;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'a':
			yyt1 = YYCURSOR;
			goto yy2;
		default: goto yy1;
	}
yy1:
	++YYCURSOR;
	{ d }
yy2:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 'a':
			yyt1 = YYCURSOR;
			goto yy2;
		default: goto yy3;
	}
yy3:
	p = yyt1;
	{ p }
}

