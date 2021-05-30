// re2c $INPUT -o $OUTPUT -8 --encoding-policy fail
#include <stdio.h>
#include "utf8.h"
#define YYCTYPE unsigned char
bool scan(const YYCTYPE * start, const YYCTYPE * const limit)
{
	__attribute__((unused)) const YYCTYPE * YYMARKER; // silence compiler warnings when YYMARKER is not used
#	define YYCURSOR start
L:
	/*!re2c
		re2c:yyfill:enable = 0;
		L = [\x41-\x5a\x61-\x7a\xaa-\xaa\xb5-\xb5\xba-\xba\xc0-\xd6\xd8-\xf6\xf8-\u02c1\u02c6-\u02d1\u02e0-\u02e4\u02ec-\u02ec\u02ee-\u02ee\u0370-\u0374\u0376-\u0377\u037a-\u037d\u037f-\u037f\u0386-\u0386\u0388-\u038a\u038c-\u038c\u038e-\u03a1\u03a3-\u03f5\u03f7-\u0481\u048a-\u052f\u0531-\u0556\u0559-\u0559\u0561-\u0587\u05d0-\u05ea\u05f0-\u05f2\u0620-\u064a\u066e-\u066f\u0671-\u06d3\u06d5-\u06d5\u06e5-\u06e6\u06ee-\u06ef\u06fa-\u06fc\u06ff-\u06ff\u0710-\u0710\u0712-\u072f\u074d-\u07a5\u07b1-\u07b1\u07ca-\u07ea\u07f4-\u07f5\u07fa-\u07fa\u0800-\u0815\u081a-\u081a\u0824-\u0824\u0828-\u0828\u0840-\u0858\u08a0-\u08b2\u0904-\u0939\u093d-\u093d\u0950-\u0950\u0958-\u0961\u0971-\u0980\u0985-\u098c\u098f-\u0990\u0993-\u09a8\u09aa-\u09b0\u09b2-\u09b2\u09b6-\u09b9\u09bd-\u09bd\u09ce-\u09ce\u09dc-\u09dd\u09df-\u09e1\u09f0-\u09f1\u0a05-\u0a0a\u0a0f-\u0a10\u0a13-\u0a28\u0a2a-\u0a30\u0a32-\u0a33\u0a35-\u0a36\u0a38-\u0a39\u0a59-\u0a5c\u0a5e-\u0a5e\u0a72-\u0a74\u0a85-\u0a8d\u0a8f-\u0a91\u0a93-\u0aa8\u0aaa-\u0ab0\u0ab2-\u0ab3\u0ab5-\u0ab9\u0abd-\u0abd\u0ad0-\u0ad0\u0ae0-\u0ae1\u0b05-\u0b0c\u0b0f-\u0b10\u0b13-\u0b28\u0b2a-\u0b30\u0b32-\u0b33\u0b35-\u0b39\u0b3d-\u0b3d\u0b5c-\u0b5d\u0b5f-\u0b61\u0b71-\u0b71\u0b83-\u0b83\u0b85-\u0b8a\u0b8e-\u0b90\u0b92-\u0b95\u0b99-\u0b9a\u0b9c-\u0b9c\u0b9e-\u0b9f\u0ba3-\u0ba4\u0ba8-\u0baa\u0bae-\u0bb9\u0bd0-\u0bd0\u0c05-\u0c0c\u0c0e-\u0c10\u0c12-\u0c28\u0c2a-\u0c39\u0c3d-\u0c3d\u0c58-\u0c59\u0c60-\u0c61\u0c85-\u0c8c\u0c8e-\u0c90\u0c92-\u0ca8\u0caa-\u0cb3\u0cb5-\u0cb9\u0cbd-\u0cbd\u0cde-\u0cde\u0ce0-\u0ce1\u0cf1-\u0cf2\u0d05-\u0d0c\u0d0e-\u0d10\u0d12-\u0d3a\u0d3d-\u0d3d\u0d4e-\u0d4e\u0d60-\u0d61\u0d7a-\u0d7f\u0d85-\u0d96\u0d9a-\u0db1\u0db3-\u0dbb\u0dbd-\u0dbd\u0dc0-\u0dc6\u0e01-\u0e30\u0e32-\u0e33\u0e40-\u0e46\u0e81-\u0e82\u0e84-\u0e84\u0e87-\u0e88\u0e8a-\u0e8a\u0e8d-\u0e8d\u0e94-\u0e97\u0e99-\u0e9f\u0ea1-\u0ea3\u0ea5-\u0ea5\u0ea7-\u0ea7\u0eaa-\u0eab\u0ead-\u0eb0\u0eb2-\u0eb3\u0ebd-\u0ebd\u0ec0-\u0ec4\u0ec6-\u0ec6\u0edc-\u0edf\u0f00-\u0f00\u0f40-\u0f47\u0f49-\u0f6c\u0f88-\u0f8c\u1000-\u102a\u103f-\u103f\u1050-\u1055\u105a-\u105d\u1061-\u1061\u1065-\u1066\u106e-\u1070\u1075-\u1081\u108e-\u108e\u10a0-\u10c5\u10c7-\u10c7\u10cd-\u10cd\u10d0-\u10fa\u10fc-\u1248\u124a-\u124d\u1250-\u1256\u1258-\u1258\u125a-\u125d\u1260-\u1288\u128a-\u128d\u1290-\u12b0\u12b2-\u12b5\u12b8-\u12be\u12c0-\u12c0\u12c2-\u12c5\u12c8-\u12d6\u12d8-\u1310\u1312-\u1315\u1318-\u135a\u1380-\u138f\u13a0-\u13f4\u1401-\u166c\u166f-\u167f\u1681-\u169a\u16a0-\u16ea\u16f1-\u16f8\u1700-\u170c\u170e-\u1711\u1720-\u1731\u1740-\u1751\u1760-\u176c\u176e-\u1770\u1780-\u17b3\u17d7-\u17d7\u17dc-\u17dc\u1820-\u1877\u1880-\u18a8\u18aa-\u18aa\u18b0-\u18f5\u1900-\u191e\u1950-\u196d\u1970-\u1974\u1980-\u19ab\u19c1-\u19c7\u1a00-\u1a16\u1a20-\u1a54\u1aa7-\u1aa7\u1b05-\u1b33\u1b45-\u1b4b\u1b83-\u1ba0\u1bae-\u1baf\u1bba-\u1be5\u1c00-\u1c23\u1c4d-\u1c4f\u1c5a-\u1c7d\u1ce9-\u1cec\u1cee-\u1cf1\u1cf5-\u1cf6\u1d00-\u1dbf\u1e00-\u1f15\u1f18-\u1f1d\u1f20-\u1f45\u1f48-\u1f4d\u1f50-\u1f57\u1f59-\u1f59\u1f5b-\u1f5b\u1f5d-\u1f5d\u1f5f-\u1f7d\u1f80-\u1fb4\u1fb6-\u1fbc\u1fbe-\u1fbe\u1fc2-\u1fc4\u1fc6-\u1fcc\u1fd0-\u1fd3\u1fd6-\u1fdb\u1fe0-\u1fec\u1ff2-\u1ff4\u1ff6-\u1ffc\u2071-\u2071\u207f-\u207f\u2090-\u209c\u2102-\u2102\u2107-\u2107\u210a-\u2113\u2115-\u2115\u2119-\u211d\u2124-\u2124\u2126-\u2126\u2128-\u2128\u212a-\u212d\u212f-\u2139\u213c-\u213f\u2145-\u2149\u214e-\u214e\u2183-\u2184\u2c00-\u2c2e\u2c30-\u2c5e\u2c60-\u2ce4\u2ceb-\u2cee\u2cf2-\u2cf3\u2d00-\u2d25\u2d27-\u2d27\u2d2d-\u2d2d\u2d30-\u2d67\u2d6f-\u2d6f\u2d80-\u2d96\u2da0-\u2da6\u2da8-\u2dae\u2db0-\u2db6\u2db8-\u2dbe\u2dc0-\u2dc6\u2dc8-\u2dce\u2dd0-\u2dd6\u2dd8-\u2dde\u2e2f-\u2e2f\u3005-\u3006\u3031-\u3035\u303b-\u303c\u3041-\u3096\u309d-\u309f\u30a1-\u30fa\u30fc-\u30ff\u3105-\u312d\u3131-\u318e\u31a0-\u31ba\u31f0-\u31ff\u3400-\u4db5\u4e00-\u9fcc\ua000-\ua48c\ua4d0-\ua4fd\ua500-\ua60c\ua610-\ua61f\ua62a-\ua62b\ua640-\ua66e\ua67f-\ua69d\ua6a0-\ua6e5\ua717-\ua71f\ua722-\ua788\ua78b-\ua78e\ua790-\ua7ad\ua7b0-\ua7b1\ua7f7-\ua801\ua803-\ua805\ua807-\ua80a\ua80c-\ua822\ua840-\ua873\ua882-\ua8b3\ua8f2-\ua8f7\ua8fb-\ua8fb\ua90a-\ua925\ua930-\ua946\ua960-\ua97c\ua984-\ua9b2\ua9cf-\ua9cf\ua9e0-\ua9e4\ua9e6-\ua9ef\ua9fa-\ua9fe\uaa00-\uaa28\uaa40-\uaa42\uaa44-\uaa4b\uaa60-\uaa76\uaa7a-\uaa7a\uaa7e-\uaaaf\uaab1-\uaab1\uaab5-\uaab6\uaab9-\uaabd\uaac0-\uaac0\uaac2-\uaac2\uaadb-\uaadd\uaae0-\uaaea\uaaf2-\uaaf4\uab01-\uab06\uab09-\uab0e\uab11-\uab16\uab20-\uab26\uab28-\uab2e\uab30-\uab5a\uab5c-\uab5f\uab64-\uab65\uabc0-\uabe2\uac00-\ud7a3\ud7b0-\ud7c6\ud7cb-\ud7fb\uf900-\ufa6d\ufa70-\ufad9\ufb00-\ufb06\ufb13-\ufb17\ufb1d-\ufb1d\ufb1f-\ufb28\ufb2a-\ufb36\ufb38-\ufb3c\ufb3e-\ufb3e\ufb40-\ufb41\ufb43-\ufb44\ufb46-\ufbb1\ufbd3-\ufd3d\ufd50-\ufd8f\ufd92-\ufdc7\ufdf0-\ufdfb\ufe70-\ufe74\ufe76-\ufefc\uff21-\uff3a\uff41-\uff5a\uff66-\uffbe\uffc2-\uffc7\uffca-\uffcf\uffd2-\uffd7\uffda-\uffdc\U00010000-\U0001000b\U0001000d-\U00010026\U00010028-\U0001003a\U0001003c-\U0001003d\U0001003f-\U0001004d\U00010050-\U0001005d\U00010080-\U000100fa\U00010280-\U0001029c\U000102a0-\U000102d0\U00010300-\U0001031f\U00010330-\U00010340\U00010342-\U00010349\U00010350-\U00010375\U00010380-\U0001039d\U000103a0-\U000103c3\U000103c8-\U000103cf\U00010400-\U0001049d\U00010500-\U00010527\U00010530-\U00010563\U00010600-\U00010736\U00010740-\U00010755\U00010760-\U00010767\U00010800-\U00010805\U00010808-\U00010808\U0001080a-\U00010835\U00010837-\U00010838\U0001083c-\U0001083c\U0001083f-\U00010855\U00010860-\U00010876\U00010880-\U0001089e\U00010900-\U00010915\U00010920-\U00010939\U00010980-\U000109b7\U000109be-\U000109bf\U00010a00-\U00010a00\U00010a10-\U00010a13\U00010a15-\U00010a17\U00010a19-\U00010a33\U00010a60-\U00010a7c\U00010a80-\U00010a9c\U00010ac0-\U00010ac7\U00010ac9-\U00010ae4\U00010b00-\U00010b35\U00010b40-\U00010b55\U00010b60-\U00010b72\U00010b80-\U00010b91\U00010c00-\U00010c48\U00011003-\U00011037\U00011083-\U000110af\U000110d0-\U000110e8\U00011103-\U00011126\U00011150-\U00011172\U00011176-\U00011176\U00011183-\U000111b2\U000111c1-\U000111c4\U000111da-\U000111da\U00011200-\U00011211\U00011213-\U0001122b\U000112b0-\U000112de\U00011305-\U0001130c\U0001130f-\U00011310\U00011313-\U00011328\U0001132a-\U00011330\U00011332-\U00011333\U00011335-\U00011339\U0001133d-\U0001133d\U0001135d-\U00011361\U00011480-\U000114af\U000114c4-\U000114c5\U000114c7-\U000114c7\U00011580-\U000115ae\U00011600-\U0001162f\U00011644-\U00011644\U00011680-\U000116aa\U000118a0-\U000118df\U000118ff-\U000118ff\U00011ac0-\U00011af8\U00012000-\U00012398\U00013000-\U0001342e\U00016800-\U00016a38\U00016a40-\U00016a5e\U00016ad0-\U00016aed\U00016b00-\U00016b2f\U00016b40-\U00016b43\U00016b63-\U00016b77\U00016b7d-\U00016b8f\U00016f00-\U00016f44\U00016f50-\U00016f50\U00016f93-\U00016f9f\U0001b000-\U0001b001\U0001bc00-\U0001bc6a\U0001bc70-\U0001bc7c\U0001bc80-\U0001bc88\U0001bc90-\U0001bc99\U0001d400-\U0001d454\U0001d456-\U0001d49c\U0001d49e-\U0001d49f\U0001d4a2-\U0001d4a2\U0001d4a5-\U0001d4a6\U0001d4a9-\U0001d4ac\U0001d4ae-\U0001d4b9\U0001d4bb-\U0001d4bb\U0001d4bd-\U0001d4c3\U0001d4c5-\U0001d505\U0001d507-\U0001d50a\U0001d50d-\U0001d514\U0001d516-\U0001d51c\U0001d51e-\U0001d539\U0001d53b-\U0001d53e\U0001d540-\U0001d544\U0001d546-\U0001d546\U0001d54a-\U0001d550\U0001d552-\U0001d6a5\U0001d6a8-\U0001d6c0\U0001d6c2-\U0001d6da\U0001d6dc-\U0001d6fa\U0001d6fc-\U0001d714\U0001d716-\U0001d734\U0001d736-\U0001d74e\U0001d750-\U0001d76e\U0001d770-\U0001d788\U0001d78a-\U0001d7a8\U0001d7aa-\U0001d7c2\U0001d7c4-\U0001d7cb\U0001e800-\U0001e8c4\U0001ee00-\U0001ee03\U0001ee05-\U0001ee1f\U0001ee21-\U0001ee22\U0001ee24-\U0001ee24\U0001ee27-\U0001ee27\U0001ee29-\U0001ee32\U0001ee34-\U0001ee37\U0001ee39-\U0001ee39\U0001ee3b-\U0001ee3b\U0001ee42-\U0001ee42\U0001ee47-\U0001ee47\U0001ee49-\U0001ee49\U0001ee4b-\U0001ee4b\U0001ee4d-\U0001ee4f\U0001ee51-\U0001ee52\U0001ee54-\U0001ee54\U0001ee57-\U0001ee57\U0001ee59-\U0001ee59\U0001ee5b-\U0001ee5b\U0001ee5d-\U0001ee5d\U0001ee5f-\U0001ee5f\U0001ee61-\U0001ee62\U0001ee64-\U0001ee64\U0001ee67-\U0001ee6a\U0001ee6c-\U0001ee72\U0001ee74-\U0001ee77\U0001ee79-\U0001ee7c\U0001ee7e-\U0001ee7e\U0001ee80-\U0001ee89\U0001ee8b-\U0001ee9b\U0001eea1-\U0001eea3\U0001eea5-\U0001eea9\U0001eeab-\U0001eebb\U00020000-\U0002a6d6\U0002a700-\U0002b734\U0002b740-\U0002b81d\U0002f800-\U0002fa1d];
		L { goto L; }
		* { return YYCURSOR == limit; }
	*/
}
static const unsigned int chars_L [] = {0x41,0x5a,  0x61,0x7a,  0xaa,0xaa,  0xb5,0xb5,  0xba,0xba,  0xc0,0xd6,  0xd8,0xf6,  0xf8,0x2c1,  0x2c6,0x2d1,  0x2e0,0x2e4,  0x2ec,0x2ec,  0x2ee,0x2ee,  0x370,0x374,  0x376,0x377,  0x37a,0x37d,  0x37f,0x37f,  0x386,0x386,  0x388,0x38a,  0x38c,0x38c,  0x38e,0x3a1,  0x3a3,0x3f5,  0x3f7,0x481,  0x48a,0x52f,  0x531,0x556,  0x559,0x559,  0x561,0x587,  0x5d0,0x5ea,  0x5f0,0x5f2,  0x620,0x64a,  0x66e,0x66f,  0x671,0x6d3,  0x6d5,0x6d5,  0x6e5,0x6e6,  0x6ee,0x6ef,  0x6fa,0x6fc,  0x6ff,0x6ff,  0x710,0x710,  0x712,0x72f,  0x74d,0x7a5,  0x7b1,0x7b1,  0x7ca,0x7ea,  0x7f4,0x7f5,  0x7fa,0x7fa,  0x800,0x815,  0x81a,0x81a,  0x824,0x824,  0x828,0x828,  0x840,0x858,  0x8a0,0x8b2,  0x904,0x939,  0x93d,0x93d,  0x950,0x950,  0x958,0x961,  0x971,0x980,  0x985,0x98c,  0x98f,0x990,  0x993,0x9a8,  0x9aa,0x9b0,  0x9b2,0x9b2,  0x9b6,0x9b9,  0x9bd,0x9bd,  0x9ce,0x9ce,  0x9dc,0x9dd,  0x9df,0x9e1,  0x9f0,0x9f1,  0xa05,0xa0a,  0xa0f,0xa10,  0xa13,0xa28,  0xa2a,0xa30,  0xa32,0xa33,  0xa35,0xa36,  0xa38,0xa39,  0xa59,0xa5c,  0xa5e,0xa5e,  0xa72,0xa74,  0xa85,0xa8d,  0xa8f,0xa91,  0xa93,0xaa8,  0xaaa,0xab0,  0xab2,0xab3,  0xab5,0xab9,  0xabd,0xabd,  0xad0,0xad0,  0xae0,0xae1,  0xb05,0xb0c,  0xb0f,0xb10,  0xb13,0xb28,  0xb2a,0xb30,  0xb32,0xb33,  0xb35,0xb39,  0xb3d,0xb3d,  0xb5c,0xb5d,  0xb5f,0xb61,  0xb71,0xb71,  0xb83,0xb83,  0xb85,0xb8a,  0xb8e,0xb90,  0xb92,0xb95,  0xb99,0xb9a,  0xb9c,0xb9c,  0xb9e,0xb9f,  0xba3,0xba4,  0xba8,0xbaa,  0xbae,0xbb9,  0xbd0,0xbd0,  0xc05,0xc0c,  0xc0e,0xc10,  0xc12,0xc28,  0xc2a,0xc39,  0xc3d,0xc3d,  0xc58,0xc59,  0xc60,0xc61,  0xc85,0xc8c,  0xc8e,0xc90,  0xc92,0xca8,  0xcaa,0xcb3,  0xcb5,0xcb9,  0xcbd,0xcbd,  0xcde,0xcde,  0xce0,0xce1,  0xcf1,0xcf2,  0xd05,0xd0c,  0xd0e,0xd10,  0xd12,0xd3a,  0xd3d,0xd3d,  0xd4e,0xd4e,  0xd60,0xd61,  0xd7a,0xd7f,  0xd85,0xd96,  0xd9a,0xdb1,  0xdb3,0xdbb,  0xdbd,0xdbd,  0xdc0,0xdc6,  0xe01,0xe30,  0xe32,0xe33,  0xe40,0xe46,  0xe81,0xe82,  0xe84,0xe84,  0xe87,0xe88,  0xe8a,0xe8a,  0xe8d,0xe8d,  0xe94,0xe97,  0xe99,0xe9f,  0xea1,0xea3,  0xea5,0xea5,  0xea7,0xea7,  0xeaa,0xeab,  0xead,0xeb0,  0xeb2,0xeb3,  0xebd,0xebd,  0xec0,0xec4,  0xec6,0xec6,  0xedc,0xedf,  0xf00,0xf00,  0xf40,0xf47,  0xf49,0xf6c,  0xf88,0xf8c,  0x1000,0x102a,  0x103f,0x103f,  0x1050,0x1055,  0x105a,0x105d,  0x1061,0x1061,  0x1065,0x1066,  0x106e,0x1070,  0x1075,0x1081,  0x108e,0x108e,  0x10a0,0x10c5,  0x10c7,0x10c7,  0x10cd,0x10cd,  0x10d0,0x10fa,  0x10fc,0x1248,  0x124a,0x124d,  0x1250,0x1256,  0x1258,0x1258,  0x125a,0x125d,  0x1260,0x1288,  0x128a,0x128d,  0x1290,0x12b0,  0x12b2,0x12b5,  0x12b8,0x12be,  0x12c0,0x12c0,  0x12c2,0x12c5,  0x12c8,0x12d6,  0x12d8,0x1310,  0x1312,0x1315,  0x1318,0x135a,  0x1380,0x138f,  0x13a0,0x13f4,  0x1401,0x166c,  0x166f,0x167f,  0x1681,0x169a,  0x16a0,0x16ea,  0x16f1,0x16f8,  0x1700,0x170c,  0x170e,0x1711,  0x1720,0x1731,  0x1740,0x1751,  0x1760,0x176c,  0x176e,0x1770,  0x1780,0x17b3,  0x17d7,0x17d7,  0x17dc,0x17dc,  0x1820,0x1877,  0x1880,0x18a8,  0x18aa,0x18aa,  0x18b0,0x18f5,  0x1900,0x191e,  0x1950,0x196d,  0x1970,0x1974,  0x1980,0x19ab,  0x19c1,0x19c7,  0x1a00,0x1a16,  0x1a20,0x1a54,  0x1aa7,0x1aa7,  0x1b05,0x1b33,  0x1b45,0x1b4b,  0x1b83,0x1ba0,  0x1bae,0x1baf,  0x1bba,0x1be5,  0x1c00,0x1c23,  0x1c4d,0x1c4f,  0x1c5a,0x1c7d,  0x1ce9,0x1cec,  0x1cee,0x1cf1,  0x1cf5,0x1cf6,  0x1d00,0x1dbf,  0x1e00,0x1f15,  0x1f18,0x1f1d,  0x1f20,0x1f45,  0x1f48,0x1f4d,  0x1f50,0x1f57,  0x1f59,0x1f59,  0x1f5b,0x1f5b,  0x1f5d,0x1f5d,  0x1f5f,0x1f7d,  0x1f80,0x1fb4,  0x1fb6,0x1fbc,  0x1fbe,0x1fbe,  0x1fc2,0x1fc4,  0x1fc6,0x1fcc,  0x1fd0,0x1fd3,  0x1fd6,0x1fdb,  0x1fe0,0x1fec,  0x1ff2,0x1ff4,  0x1ff6,0x1ffc,  0x2071,0x2071,  0x207f,0x207f,  0x2090,0x209c,  0x2102,0x2102,  0x2107,0x2107,  0x210a,0x2113,  0x2115,0x2115,  0x2119,0x211d,  0x2124,0x2124,  0x2126,0x2126,  0x2128,0x2128,  0x212a,0x212d,  0x212f,0x2139,  0x213c,0x213f,  0x2145,0x2149,  0x214e,0x214e,  0x2183,0x2184,  0x2c00,0x2c2e,  0x2c30,0x2c5e,  0x2c60,0x2ce4,  0x2ceb,0x2cee,  0x2cf2,0x2cf3,  0x2d00,0x2d25,  0x2d27,0x2d27,  0x2d2d,0x2d2d,  0x2d30,0x2d67,  0x2d6f,0x2d6f,  0x2d80,0x2d96,  0x2da0,0x2da6,  0x2da8,0x2dae,  0x2db0,0x2db6,  0x2db8,0x2dbe,  0x2dc0,0x2dc6,  0x2dc8,0x2dce,  0x2dd0,0x2dd6,  0x2dd8,0x2dde,  0x2e2f,0x2e2f,  0x3005,0x3006,  0x3031,0x3035,  0x303b,0x303c,  0x3041,0x3096,  0x309d,0x309f,  0x30a1,0x30fa,  0x30fc,0x30ff,  0x3105,0x312d,  0x3131,0x318e,  0x31a0,0x31ba,  0x31f0,0x31ff,  0x3400,0x4db5,  0x4e00,0x9fcc,  0xa000,0xa48c,  0xa4d0,0xa4fd,  0xa500,0xa60c,  0xa610,0xa61f,  0xa62a,0xa62b,  0xa640,0xa66e,  0xa67f,0xa69d,  0xa6a0,0xa6e5,  0xa717,0xa71f,  0xa722,0xa788,  0xa78b,0xa78e,  0xa790,0xa7ad,  0xa7b0,0xa7b1,  0xa7f7,0xa801,  0xa803,0xa805,  0xa807,0xa80a,  0xa80c,0xa822,  0xa840,0xa873,  0xa882,0xa8b3,  0xa8f2,0xa8f7,  0xa8fb,0xa8fb,  0xa90a,0xa925,  0xa930,0xa946,  0xa960,0xa97c,  0xa984,0xa9b2,  0xa9cf,0xa9cf,  0xa9e0,0xa9e4,  0xa9e6,0xa9ef,  0xa9fa,0xa9fe,  0xaa00,0xaa28,  0xaa40,0xaa42,  0xaa44,0xaa4b,  0xaa60,0xaa76,  0xaa7a,0xaa7a,  0xaa7e,0xaaaf,  0xaab1,0xaab1,  0xaab5,0xaab6,  0xaab9,0xaabd,  0xaac0,0xaac0,  0xaac2,0xaac2,  0xaadb,0xaadd,  0xaae0,0xaaea,  0xaaf2,0xaaf4,  0xab01,0xab06,  0xab09,0xab0e,  0xab11,0xab16,  0xab20,0xab26,  0xab28,0xab2e,  0xab30,0xab5a,  0xab5c,0xab5f,  0xab64,0xab65,  0xabc0,0xabe2,  0xac00,0xd7a3,  0xd7b0,0xd7c6,  0xd7cb,0xd7fb,  0xf900,0xfa6d,  0xfa70,0xfad9,  0xfb00,0xfb06,  0xfb13,0xfb17,  0xfb1d,0xfb1d,  0xfb1f,0xfb28,  0xfb2a,0xfb36,  0xfb38,0xfb3c,  0xfb3e,0xfb3e,  0xfb40,0xfb41,  0xfb43,0xfb44,  0xfb46,0xfbb1,  0xfbd3,0xfd3d,  0xfd50,0xfd8f,  0xfd92,0xfdc7,  0xfdf0,0xfdfb,  0xfe70,0xfe74,  0xfe76,0xfefc,  0xff21,0xff3a,  0xff41,0xff5a,  0xff66,0xffbe,  0xffc2,0xffc7,  0xffca,0xffcf,  0xffd2,0xffd7,  0xffda,0xffdc,  0x10000,0x1000b,  0x1000d,0x10026,  0x10028,0x1003a,  0x1003c,0x1003d,  0x1003f,0x1004d,  0x10050,0x1005d,  0x10080,0x100fa,  0x10280,0x1029c,  0x102a0,0x102d0,  0x10300,0x1031f,  0x10330,0x10340,  0x10342,0x10349,  0x10350,0x10375,  0x10380,0x1039d,  0x103a0,0x103c3,  0x103c8,0x103cf,  0x10400,0x1049d,  0x10500,0x10527,  0x10530,0x10563,  0x10600,0x10736,  0x10740,0x10755,  0x10760,0x10767,  0x10800,0x10805,  0x10808,0x10808,  0x1080a,0x10835,  0x10837,0x10838,  0x1083c,0x1083c,  0x1083f,0x10855,  0x10860,0x10876,  0x10880,0x1089e,  0x10900,0x10915,  0x10920,0x10939,  0x10980,0x109b7,  0x109be,0x109bf,  0x10a00,0x10a00,  0x10a10,0x10a13,  0x10a15,0x10a17,  0x10a19,0x10a33,  0x10a60,0x10a7c,  0x10a80,0x10a9c,  0x10ac0,0x10ac7,  0x10ac9,0x10ae4,  0x10b00,0x10b35,  0x10b40,0x10b55,  0x10b60,0x10b72,  0x10b80,0x10b91,  0x10c00,0x10c48,  0x11003,0x11037,  0x11083,0x110af,  0x110d0,0x110e8,  0x11103,0x11126,  0x11150,0x11172,  0x11176,0x11176,  0x11183,0x111b2,  0x111c1,0x111c4,  0x111da,0x111da,  0x11200,0x11211,  0x11213,0x1122b,  0x112b0,0x112de,  0x11305,0x1130c,  0x1130f,0x11310,  0x11313,0x11328,  0x1132a,0x11330,  0x11332,0x11333,  0x11335,0x11339,  0x1133d,0x1133d,  0x1135d,0x11361,  0x11480,0x114af,  0x114c4,0x114c5,  0x114c7,0x114c7,  0x11580,0x115ae,  0x11600,0x1162f,  0x11644,0x11644,  0x11680,0x116aa,  0x118a0,0x118df,  0x118ff,0x118ff,  0x11ac0,0x11af8,  0x12000,0x12398,  0x13000,0x1342e,  0x16800,0x16a38,  0x16a40,0x16a5e,  0x16ad0,0x16aed,  0x16b00,0x16b2f,  0x16b40,0x16b43,  0x16b63,0x16b77,  0x16b7d,0x16b8f,  0x16f00,0x16f44,  0x16f50,0x16f50,  0x16f93,0x16f9f,  0x1b000,0x1b001,  0x1bc00,0x1bc6a,  0x1bc70,0x1bc7c,  0x1bc80,0x1bc88,  0x1bc90,0x1bc99,  0x1d400,0x1d454,  0x1d456,0x1d49c,  0x1d49e,0x1d49f,  0x1d4a2,0x1d4a2,  0x1d4a5,0x1d4a6,  0x1d4a9,0x1d4ac,  0x1d4ae,0x1d4b9,  0x1d4bb,0x1d4bb,  0x1d4bd,0x1d4c3,  0x1d4c5,0x1d505,  0x1d507,0x1d50a,  0x1d50d,0x1d514,  0x1d516,0x1d51c,  0x1d51e,0x1d539,  0x1d53b,0x1d53e,  0x1d540,0x1d544,  0x1d546,0x1d546,  0x1d54a,0x1d550,  0x1d552,0x1d6a5,  0x1d6a8,0x1d6c0,  0x1d6c2,0x1d6da,  0x1d6dc,0x1d6fa,  0x1d6fc,0x1d714,  0x1d716,0x1d734,  0x1d736,0x1d74e,  0x1d750,0x1d76e,  0x1d770,0x1d788,  0x1d78a,0x1d7a8,  0x1d7aa,0x1d7c2,  0x1d7c4,0x1d7cb,  0x1e800,0x1e8c4,  0x1ee00,0x1ee03,  0x1ee05,0x1ee1f,  0x1ee21,0x1ee22,  0x1ee24,0x1ee24,  0x1ee27,0x1ee27,  0x1ee29,0x1ee32,  0x1ee34,0x1ee37,  0x1ee39,0x1ee39,  0x1ee3b,0x1ee3b,  0x1ee42,0x1ee42,  0x1ee47,0x1ee47,  0x1ee49,0x1ee49,  0x1ee4b,0x1ee4b,  0x1ee4d,0x1ee4f,  0x1ee51,0x1ee52,  0x1ee54,0x1ee54,  0x1ee57,0x1ee57,  0x1ee59,0x1ee59,  0x1ee5b,0x1ee5b,  0x1ee5d,0x1ee5d,  0x1ee5f,0x1ee5f,  0x1ee61,0x1ee62,  0x1ee64,0x1ee64,  0x1ee67,0x1ee6a,  0x1ee6c,0x1ee72,  0x1ee74,0x1ee77,  0x1ee79,0x1ee7c,  0x1ee7e,0x1ee7e,  0x1ee80,0x1ee89,  0x1ee8b,0x1ee9b,  0x1eea1,0x1eea3,  0x1eea5,0x1eea9,  0x1eeab,0x1eebb,  0x20000,0x2a6d6,  0x2a700,0x2b734,  0x2b740,0x2b81d,  0x2f800,0x2fa1d,  0x0,0x0};
static unsigned int encode_utf8 (const unsigned int * ranges, unsigned int ranges_count, unsigned int * s)
{
	unsigned int * const s_start = s;
	for (unsigned int i = 0; i < ranges_count - 2; i += 2)
		for (unsigned int j = ranges[i]; j <= ranges[i + 1]; ++j)
			s += re2c::utf8::rune_to_bytes (s, j);
	re2c::utf8::rune_to_bytes (s, ranges[ranges_count - 1]);
	return s - s_start + 1;
}

int main ()
{
	unsigned int * buffer_L = new unsigned int [410904];
	YYCTYPE * s = (YYCTYPE *) buffer_L;
	unsigned int buffer_len = encode_utf8 (chars_L, sizeof (chars_L) / sizeof (unsigned int), buffer_L);
	/* convert 32-bit code units to YYCTYPE; reuse the same buffer */
	for (unsigned int i = 0; i < buffer_len; ++i) s[i] = buffer_L[i];
	if (!scan (s, s + buffer_len))
		printf("test 'L' failed\n");
	delete [] buffer_L;
	return 0;
}
