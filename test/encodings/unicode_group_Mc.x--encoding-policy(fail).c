/* Generated by re2c */
#line 1 "encodings/unicode_group_Mc.x--encoding-policy(fail).re"
// re2c $INPUT -o $OUTPUT -x --encoding-policy fail
#include <stdio.h>
#include "utf16.h"
#define YYCTYPE unsigned short
bool scan(const YYCTYPE * start, const YYCTYPE * const limit)
{
	__attribute__((unused)) const YYCTYPE * YYMARKER; // silence compiler warnings when YYMARKER is not used
#	define YYCURSOR start
Mc:
	
#line 14 "encodings/unicode_group_Mc.x--encoding-policy(fail).c"
{
	YYCTYPE yych;
	yych = *YYCURSOR;
	if (yych <= 0x17B5) {
		if (yych <= 0x0C40) {
			if (yych <= 0x0AC0) {
				if (yych <= 0x09C0) {
					if (yych <= 0x0948) {
						if (yych <= 0x093A) {
							if (yych == 0x0903) goto yy4;
						} else {
							if (yych <= 0x093B) goto yy4;
							if (yych <= 0x093D) goto yy2;
							if (yych <= 0x0940) goto yy4;
						}
					} else {
						if (yych <= 0x094F) {
							if (yych != 0x094D) goto yy4;
						} else {
							if (yych <= 0x0981) goto yy2;
							if (yych <= 0x0983) goto yy4;
							if (yych >= 0x09BE) goto yy4;
						}
					}
				} else {
					if (yych <= 0x0A02) {
						if (yych <= 0x09CA) {
							if (yych <= 0x09C6) goto yy2;
							if (yych <= 0x09C8) goto yy4;
						} else {
							if (yych <= 0x09CC) goto yy4;
							if (yych == 0x09D7) goto yy4;
						}
					} else {
						if (yych <= 0x0A40) {
							if (yych <= 0x0A03) goto yy4;
							if (yych >= 0x0A3E) goto yy4;
						} else {
							if (yych == 0x0A83) goto yy4;
							if (yych >= 0x0ABE) goto yy4;
						}
					}
				}
			} else {
				if (yych <= 0x0B4C) {
					if (yych <= 0x0B3D) {
						if (yych <= 0x0ACA) {
							if (yych == 0x0AC9) goto yy4;
						} else {
							if (yych <= 0x0ACC) goto yy4;
							if (yych <= 0x0B01) goto yy2;
							if (yych <= 0x0B03) goto yy4;
						}
					} else {
						if (yych <= 0x0B40) {
							if (yych != 0x0B3F) goto yy4;
						} else {
							if (yych <= 0x0B46) goto yy2;
							if (yych <= 0x0B48) goto yy4;
							if (yych >= 0x0B4B) goto yy4;
						}
					}
				} else {
					if (yych <= 0x0BC5) {
						if (yych <= 0x0BBD) {
							if (yych == 0x0B57) goto yy4;
						} else {
							if (yych == 0x0BC0) goto yy2;
							if (yych <= 0x0BC2) goto yy4;
						}
					} else {
						if (yych <= 0x0BD6) {
							if (yych == 0x0BC9) goto yy2;
							if (yych <= 0x0BCC) goto yy4;
						} else {
							if (yych <= 0x0BD7) goto yy4;
							if (yych <= 0x0C00) goto yy2;
							if (yych <= 0x0C03) goto yy4;
						}
					}
				}
			}
		} else {
			if (yych <= 0x0DDF) {
				if (yych <= 0x0D01) {
					if (yych <= 0x0CC4) {
						if (yych <= 0x0C83) {
							if (yych <= 0x0C44) goto yy4;
							if (yych >= 0x0C82) goto yy4;
						} else {
							if (yych == 0x0CBE) goto yy4;
							if (yych >= 0x0CC0) goto yy4;
						}
					} else {
						if (yych <= 0x0CC9) {
							if (yych <= 0x0CC6) goto yy2;
							if (yych <= 0x0CC8) goto yy4;
						} else {
							if (yych <= 0x0CCB) goto yy4;
							if (yych <= 0x0CD4) goto yy2;
							if (yych <= 0x0CD6) goto yy4;
						}
					}
				} else {
					if (yych <= 0x0D4C) {
						if (yych <= 0x0D40) {
							if (yych <= 0x0D03) goto yy4;
							if (yych >= 0x0D3E) goto yy4;
						} else {
							if (yych <= 0x0D45) goto yy2;
							if (yych != 0x0D49) goto yy4;
						}
					} else {
						if (yych <= 0x0D83) {
							if (yych == 0x0D57) goto yy4;
							if (yych >= 0x0D82) goto yy4;
						} else {
							if (yych <= 0x0DCE) goto yy2;
							if (yych <= 0x0DD1) goto yy4;
							if (yych >= 0x0DD8) goto yy4;
						}
					}
				}
			} else {
				if (yych <= 0x103C) {
					if (yych <= 0x102A) {
						if (yych <= 0x0F3D) {
							if (yych <= 0x0DF1) goto yy2;
							if (yych <= 0x0DF3) goto yy4;
						} else {
							if (yych <= 0x0F3F) goto yy4;
							if (yych == 0x0F7F) goto yy4;
						}
					} else {
						if (yych <= 0x1031) {
							if (yych <= 0x102C) goto yy4;
							if (yych >= 0x1031) goto yy4;
						} else {
							if (yych == 0x1038) goto yy4;
							if (yych >= 0x103B) goto yy4;
						}
					}
				} else {
					if (yych <= 0x1082) {
						if (yych <= 0x1061) {
							if (yych <= 0x1055) goto yy2;
							if (yych <= 0x1057) goto yy4;
						} else {
							if (yych <= 0x1064) goto yy4;
							if (yych <= 0x1066) goto yy2;
							if (yych <= 0x106D) goto yy4;
						}
					} else {
						if (yych <= 0x108E) {
							if (yych <= 0x1084) goto yy4;
							if (yych <= 0x1086) goto yy2;
							if (yych <= 0x108C) goto yy4;
						} else {
							if (yych <= 0x108F) goto yy4;
							if (yych <= 0x1099) goto yy2;
							if (yych <= 0x109C) goto yy4;
						}
					}
				}
			}
		}
	} else {
		if (yych <= 0x1C2B) {
			if (yych <= 0x1A6C) {
				if (yych <= 0x19AF) {
					if (yych <= 0x1926) {
						if (yych <= 0x17C5) {
							if (yych <= 0x17B6) goto yy4;
							if (yych >= 0x17BE) goto yy4;
						} else {
							if (yych <= 0x17C6) goto yy2;
							if (yych <= 0x17C8) goto yy4;
							if (yych >= 0x1923) goto yy4;
						}
					} else {
						if (yych <= 0x192F) {
							if (yych <= 0x1928) goto yy2;
							if (yych <= 0x192B) goto yy4;
						} else {
							if (yych == 0x1932) goto yy2;
							if (yych <= 0x1938) goto yy4;
						}
					}
				} else {
					if (yych <= 0x1A55) {
						if (yych <= 0x19C9) {
							if (yych <= 0x19C0) goto yy4;
							if (yych >= 0x19C8) goto yy4;
						} else {
							if (yych <= 0x1A18) goto yy2;
							if (yych <= 0x1A1A) goto yy4;
							if (yych >= 0x1A55) goto yy4;
						}
					} else {
						if (yych <= 0x1A60) {
							if (yych == 0x1A57) goto yy4;
						} else {
							if (yych == 0x1A62) goto yy2;
							if (yych <= 0x1A64) goto yy4;
						}
					}
				}
			} else {
				if (yych <= 0x1BA0) {
					if (yych <= 0x1B3B) {
						if (yych <= 0x1B04) {
							if (yych <= 0x1A72) goto yy4;
							if (yych >= 0x1B04) goto yy4;
						} else {
							if (yych == 0x1B35) goto yy4;
							if (yych >= 0x1B3B) goto yy4;
						}
					} else {
						if (yych <= 0x1B42) {
							if (yych <= 0x1B3C) goto yy2;
							if (yych <= 0x1B41) goto yy4;
						} else {
							if (yych <= 0x1B44) goto yy4;
							if (yych == 0x1B82) goto yy4;
						}
					}
				} else {
					if (yych <= 0x1BE7) {
						if (yych <= 0x1BA7) {
							if (yych <= 0x1BA1) goto yy4;
							if (yych >= 0x1BA6) goto yy4;
						} else {
							if (yych == 0x1BAA) goto yy4;
							if (yych >= 0x1BE7) goto yy4;
						}
					} else {
						if (yych <= 0x1BEE) {
							if (yych <= 0x1BE9) goto yy2;
							if (yych != 0x1BED) goto yy4;
						} else {
							if (yych <= 0x1BF1) goto yy2;
							if (yych <= 0x1BF3) goto yy4;
							if (yych >= 0x1C24) goto yy4;
						}
					}
				}
			}
		} else {
			if (yych <= 0xAA32) {
				if (yych <= 0xA881) {
					if (yych <= 0x302D) {
						if (yych <= 0x1CE0) {
							if (yych <= 0x1C33) goto yy2;
							if (yych <= 0x1C35) goto yy4;
						} else {
							if (yych <= 0x1CE1) goto yy4;
							if (yych <= 0x1CF1) goto yy2;
							if (yych <= 0x1CF3) goto yy4;
						}
					} else {
						if (yych <= 0xA824) {
							if (yych <= 0x302F) goto yy4;
							if (yych >= 0xA823) goto yy4;
						} else {
							if (yych == 0xA827) goto yy4;
							if (yych >= 0xA880) goto yy4;
						}
					}
				} else {
					if (yych <= 0xA9B3) {
						if (yych <= 0xA951) {
							if (yych <= 0xA8B3) goto yy2;
							if (yych <= 0xA8C3) goto yy4;
						} else {
							if (yych <= 0xA953) goto yy4;
							if (yych == 0xA983) goto yy4;
						}
					} else {
						if (yych <= 0xA9BC) {
							if (yych <= 0xA9B5) goto yy4;
							if (yych <= 0xA9B9) goto yy2;
							if (yych <= 0xA9BB) goto yy4;
						} else {
							if (yych <= 0xA9C0) goto yy4;
							if (yych <= 0xAA2E) goto yy2;
							if (yych <= 0xAA30) goto yy4;
						}
					}
				}
			} else {
				if (yych <= 0xABE2) {
					if (yych <= 0xAA7D) {
						if (yych <= 0xAA4D) {
							if (yych <= 0xAA34) goto yy4;
							if (yych >= 0xAA4D) goto yy4;
						} else {
							if (yych == 0xAA7B) goto yy4;
							if (yych >= 0xAA7D) goto yy4;
						}
					} else {
						if (yych <= 0xAAED) {
							if (yych == 0xAAEB) goto yy4;
						} else {
							if (yych <= 0xAAEF) goto yy4;
							if (yych == 0xAAF5) goto yy4;
						}
					}
				} else {
					if (yych <= 0xABEC) {
						if (yych <= 0xABE7) {
							if (yych != 0xABE5) goto yy4;
						} else {
							if (yych <= 0xABE8) goto yy2;
							if (yych != 0xABEB) goto yy4;
						}
					} else {
						if (yych <= 0xD81A) {
							if (yych <= 0xD803) goto yy2;
							if (yych <= 0xD804) goto yy6;
							if (yych <= 0xD805) goto yy7;
						} else {
							if (yych <= 0xD81B) goto yy8;
							if (yych == 0xD834) goto yy9;
						}
					}
				}
			}
		}
	}
yy2:
	++YYCURSOR;
yy3:
#line 14 "encodings/unicode_group_Mc.x--encoding-policy(fail).re"
	{ return YYCURSOR == limit; }
#line 349 "encodings/unicode_group_Mc.x--encoding-policy(fail).c"
yy4:
	++YYCURSOR;
#line 13 "encodings/unicode_group_Mc.x--encoding-policy(fail).re"
	{ goto Mc; }
#line 354 "encodings/unicode_group_Mc.x--encoding-policy(fail).c"
yy6:
	yych = *++YYCURSOR;
	if (yych <= 0xDE2E) {
		if (yych <= 0xDCB8) {
			if (yych <= 0xDC81) {
				if (yych <= 0xDC00) {
					if (yych <= 0xDBFF) goto yy3;
					goto yy4;
				} else {
					if (yych == 0xDC02) goto yy4;
					goto yy3;
				}
			} else {
				if (yych <= 0xDCAF) {
					if (yych <= 0xDC82) goto yy4;
					goto yy3;
				} else {
					if (yych <= 0xDCB2) goto yy4;
					if (yych <= 0xDCB6) goto yy3;
					goto yy4;
				}
			}
		} else {
			if (yych <= 0xDDB2) {
				if (yych <= 0xDD2C) {
					if (yych <= 0xDD2B) goto yy3;
					goto yy4;
				} else {
					if (yych == 0xDD82) goto yy4;
					goto yy3;
				}
			} else {
				if (yych <= 0xDDBE) {
					if (yych <= 0xDDB5) goto yy4;
					goto yy3;
				} else {
					if (yych <= 0xDDC0) goto yy4;
					if (yych <= 0xDE2B) goto yy3;
					goto yy4;
				}
			}
		}
	} else {
		if (yych <= 0xDF3F) {
			if (yych <= 0xDEDF) {
				if (yych <= 0xDE33) {
					if (yych <= 0xDE31) goto yy3;
					goto yy4;
				} else {
					if (yych == 0xDE35) goto yy4;
					goto yy3;
				}
			} else {
				if (yych <= 0xDF01) {
					if (yych <= 0xDEE2) goto yy4;
					goto yy3;
				} else {
					if (yych <= 0xDF03) goto yy4;
					if (yych <= 0xDF3D) goto yy3;
					goto yy4;
				}
			}
		} else {
			if (yych <= 0xDF4A) {
				if (yych <= 0xDF44) {
					if (yych <= 0xDF40) goto yy3;
					goto yy4;
				} else {
					if (yych <= 0xDF46) goto yy3;
					if (yych <= 0xDF48) goto yy4;
					goto yy3;
				}
			} else {
				if (yych <= 0xDF57) {
					if (yych <= 0xDF4D) goto yy4;
					if (yych <= 0xDF56) goto yy3;
					goto yy4;
				} else {
					if (yych <= 0xDF61) goto yy3;
					if (yych <= 0xDF63) goto yy4;
					goto yy3;
				}
			}
		}
	}
yy7:
	yych = *++YYCURSOR;
	if (yych <= 0xDDBD) {
		if (yych <= 0xDCBE) {
			if (yych <= 0xDCB8) {
				if (yych <= 0xDCAF) goto yy3;
				if (yych <= 0xDCB2) goto yy4;
				goto yy3;
			} else {
				if (yych == 0xDCBA) goto yy3;
				goto yy4;
			}
		} else {
			if (yych <= 0xDDAE) {
				if (yych == 0xDCC1) goto yy4;
				goto yy3;
			} else {
				if (yych <= 0xDDB1) goto yy4;
				if (yych <= 0xDDB7) goto yy3;
				if (yych <= 0xDDBB) goto yy4;
				goto yy3;
			}
		}
	} else {
		if (yych <= 0xDE3E) {
			if (yych <= 0xDE32) {
				if (yych <= 0xDDBE) goto yy4;
				if (yych <= 0xDE2F) goto yy3;
				goto yy4;
			} else {
				if (yych <= 0xDE3A) goto yy3;
				if (yych == 0xDE3D) goto yy3;
				goto yy4;
			}
		} else {
			if (yych <= 0xDEAD) {
				if (yych == 0xDEAC) goto yy4;
				goto yy3;
			} else {
				if (yych <= 0xDEAF) goto yy4;
				if (yych == 0xDEB6) goto yy4;
				goto yy3;
			}
		}
	}
yy8:
	yych = *++YYCURSOR;
	if (yych <= 0xDF50) goto yy3;
	if (yych <= 0xDF7E) goto yy4;
	goto yy3;
yy9:
	yych = *++YYCURSOR;
	if (yych <= 0xDD64) goto yy3;
	if (yych <= 0xDD66) goto yy4;
	if (yych <= 0xDD6C) goto yy3;
	if (yych <= 0xDD72) goto yy4;
	goto yy3;
}
#line 15 "encodings/unicode_group_Mc.x--encoding-policy(fail).re"

}
static const unsigned int chars_Mc [] = {0x903,0x903,  0x93b,0x93b,  0x93e,0x940,  0x949,0x94c,  0x94e,0x94f,  0x982,0x983,  0x9be,0x9c0,  0x9c7,0x9c8,  0x9cb,0x9cc,  0x9d7,0x9d7,  0xa03,0xa03,  0xa3e,0xa40,  0xa83,0xa83,  0xabe,0xac0,  0xac9,0xac9,  0xacb,0xacc,  0xb02,0xb03,  0xb3e,0xb3e,  0xb40,0xb40,  0xb47,0xb48,  0xb4b,0xb4c,  0xb57,0xb57,  0xbbe,0xbbf,  0xbc1,0xbc2,  0xbc6,0xbc8,  0xbca,0xbcc,  0xbd7,0xbd7,  0xc01,0xc03,  0xc41,0xc44,  0xc82,0xc83,  0xcbe,0xcbe,  0xcc0,0xcc4,  0xcc7,0xcc8,  0xcca,0xccb,  0xcd5,0xcd6,  0xd02,0xd03,  0xd3e,0xd40,  0xd46,0xd48,  0xd4a,0xd4c,  0xd57,0xd57,  0xd82,0xd83,  0xdcf,0xdd1,  0xdd8,0xddf,  0xdf2,0xdf3,  0xf3e,0xf3f,  0xf7f,0xf7f,  0x102b,0x102c,  0x1031,0x1031,  0x1038,0x1038,  0x103b,0x103c,  0x1056,0x1057,  0x1062,0x1064,  0x1067,0x106d,  0x1083,0x1084,  0x1087,0x108c,  0x108f,0x108f,  0x109a,0x109c,  0x17b6,0x17b6,  0x17be,0x17c5,  0x17c7,0x17c8,  0x1923,0x1926,  0x1929,0x192b,  0x1930,0x1931,  0x1933,0x1938,  0x19b0,0x19c0,  0x19c8,0x19c9,  0x1a19,0x1a1a,  0x1a55,0x1a55,  0x1a57,0x1a57,  0x1a61,0x1a61,  0x1a63,0x1a64,  0x1a6d,0x1a72,  0x1b04,0x1b04,  0x1b35,0x1b35,  0x1b3b,0x1b3b,  0x1b3d,0x1b41,  0x1b43,0x1b44,  0x1b82,0x1b82,  0x1ba1,0x1ba1,  0x1ba6,0x1ba7,  0x1baa,0x1baa,  0x1be7,0x1be7,  0x1bea,0x1bec,  0x1bee,0x1bee,  0x1bf2,0x1bf3,  0x1c24,0x1c2b,  0x1c34,0x1c35,  0x1ce1,0x1ce1,  0x1cf2,0x1cf3,  0x302e,0x302f,  0xa823,0xa824,  0xa827,0xa827,  0xa880,0xa881,  0xa8b4,0xa8c3,  0xa952,0xa953,  0xa983,0xa983,  0xa9b4,0xa9b5,  0xa9ba,0xa9bb,  0xa9bd,0xa9c0,  0xaa2f,0xaa30,  0xaa33,0xaa34,  0xaa4d,0xaa4d,  0xaa7b,0xaa7b,  0xaa7d,0xaa7d,  0xaaeb,0xaaeb,  0xaaee,0xaaef,  0xaaf5,0xaaf5,  0xabe3,0xabe4,  0xabe6,0xabe7,  0xabe9,0xabea,  0xabec,0xabec,  0x11000,0x11000,  0x11002,0x11002,  0x11082,0x11082,  0x110b0,0x110b2,  0x110b7,0x110b8,  0x1112c,0x1112c,  0x11182,0x11182,  0x111b3,0x111b5,  0x111bf,0x111c0,  0x1122c,0x1122e,  0x11232,0x11233,  0x11235,0x11235,  0x112e0,0x112e2,  0x11302,0x11303,  0x1133e,0x1133f,  0x11341,0x11344,  0x11347,0x11348,  0x1134b,0x1134d,  0x11357,0x11357,  0x11362,0x11363,  0x114b0,0x114b2,  0x114b9,0x114b9,  0x114bb,0x114be,  0x114c1,0x114c1,  0x115af,0x115b1,  0x115b8,0x115bb,  0x115be,0x115be,  0x11630,0x11632,  0x1163b,0x1163c,  0x1163e,0x1163e,  0x116ac,0x116ac,  0x116ae,0x116af,  0x116b6,0x116b6,  0x16f51,0x16f7e,  0x1d165,0x1d166,  0x1d16d,0x1d172,  0x0,0x0};
static unsigned int encode_utf16 (const unsigned int * ranges, unsigned int ranges_count, unsigned int * s)
{
	unsigned int * const s_start = s;
	for (unsigned int i = 0; i < ranges_count; i += 2)
		for (unsigned int j = ranges[i]; j <= ranges[i + 1]; ++j)
		{
			if (j <= re2c::utf16::MAX_1WORD_RUNE)
				*s++ = j;
			else
			{
				*s++ = re2c::utf16::lead_surr(j);
				*s++ = re2c::utf16::trail_surr(j);
			}
		}
	return s - s_start;
}

int main ()
{
	unsigned int * buffer_Mc = new unsigned int [800];
	YYCTYPE * s = (YYCTYPE *) buffer_Mc;
	unsigned int buffer_len = encode_utf16 (chars_Mc, sizeof (chars_Mc) / sizeof (unsigned int), buffer_Mc);
	/* convert 32-bit code units to YYCTYPE; reuse the same buffer */
	for (unsigned int i = 0; i < buffer_len; ++i) s[i] = buffer_Mc[i];
	if (!scan (s, s + buffer_len))
		printf("test 'Mc' failed\n");
	delete [] buffer_Mc;
	return 0;
}
