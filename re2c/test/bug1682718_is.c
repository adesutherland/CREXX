/* Generated by re2c */
// re2c $INPUT -o $OUTPUT -is
char *scan(char *p)
{

	{
		unsigned char yych;
		yych = (unsigned char)*p;
		if (yych <= '/') goto yy2;
		if (yych <= '9') goto yy4;
yy2:
		++p;
		{return (char*)0;}
yy4:
		yych = (unsigned char)*++p;
		if (yych <= '/') goto yy6;
		if (yych <= '9') goto yy4;
yy6:
		{return p;}
	}

}

