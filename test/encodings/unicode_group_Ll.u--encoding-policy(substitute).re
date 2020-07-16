// re2c $INPUT -o $OUTPUT -u --encoding-policy substitute
#include <stdio.h>

#define YYCTYPE unsigned int
bool scan(const YYCTYPE * start, const YYCTYPE * const limit)
{
	__attribute__((unused)) const YYCTYPE * YYMARKER; // silence compiler warnings when YYMARKER is not used
#	define YYCURSOR start
Ll:
	/*!re2c
		re2c:yyfill:enable = 0;
		Ll = [\x61-\x7a\xb5-\xb5\xdf-\xf6\xf8-\u00ff\u0101-\u0101\u0103-\u0103\u0105-\u0105\u0107-\u0107\u0109-\u0109\u010b-\u010b\u010d-\u010d\u010f-\u010f\u0111-\u0111\u0113-\u0113\u0115-\u0115\u0117-\u0117\u0119-\u0119\u011b-\u011b\u011d-\u011d\u011f-\u011f\u0121-\u0121\u0123-\u0123\u0125-\u0125\u0127-\u0127\u0129-\u0129\u012b-\u012b\u012d-\u012d\u012f-\u012f\u0131-\u0131\u0133-\u0133\u0135-\u0135\u0137-\u0138\u013a-\u013a\u013c-\u013c\u013e-\u013e\u0140-\u0140\u0142-\u0142\u0144-\u0144\u0146-\u0146\u0148-\u0149\u014b-\u014b\u014d-\u014d\u014f-\u014f\u0151-\u0151\u0153-\u0153\u0155-\u0155\u0157-\u0157\u0159-\u0159\u015b-\u015b\u015d-\u015d\u015f-\u015f\u0161-\u0161\u0163-\u0163\u0165-\u0165\u0167-\u0167\u0169-\u0169\u016b-\u016b\u016d-\u016d\u016f-\u016f\u0171-\u0171\u0173-\u0173\u0175-\u0175\u0177-\u0177\u017a-\u017a\u017c-\u017c\u017e-\u0180\u0183-\u0183\u0185-\u0185\u0188-\u0188\u018c-\u018d\u0192-\u0192\u0195-\u0195\u0199-\u019b\u019e-\u019e\u01a1-\u01a1\u01a3-\u01a3\u01a5-\u01a5\u01a8-\u01a8\u01aa-\u01ab\u01ad-\u01ad\u01b0-\u01b0\u01b4-\u01b4\u01b6-\u01b6\u01b9-\u01ba\u01bd-\u01bf\u01c6-\u01c6\u01c9-\u01c9\u01cc-\u01cc\u01ce-\u01ce\u01d0-\u01d0\u01d2-\u01d2\u01d4-\u01d4\u01d6-\u01d6\u01d8-\u01d8\u01da-\u01da\u01dc-\u01dd\u01df-\u01df\u01e1-\u01e1\u01e3-\u01e3\u01e5-\u01e5\u01e7-\u01e7\u01e9-\u01e9\u01eb-\u01eb\u01ed-\u01ed\u01ef-\u01f0\u01f3-\u01f3\u01f5-\u01f5\u01f9-\u01f9\u01fb-\u01fb\u01fd-\u01fd\u01ff-\u01ff\u0201-\u0201\u0203-\u0203\u0205-\u0205\u0207-\u0207\u0209-\u0209\u020b-\u020b\u020d-\u020d\u020f-\u020f\u0211-\u0211\u0213-\u0213\u0215-\u0215\u0217-\u0217\u0219-\u0219\u021b-\u021b\u021d-\u021d\u021f-\u021f\u0221-\u0221\u0223-\u0223\u0225-\u0225\u0227-\u0227\u0229-\u0229\u022b-\u022b\u022d-\u022d\u022f-\u022f\u0231-\u0231\u0233-\u0239\u023c-\u023c\u023f-\u0240\u0242-\u0242\u0247-\u0247\u0249-\u0249\u024b-\u024b\u024d-\u024d\u024f-\u0293\u0295-\u02af\u0371-\u0371\u0373-\u0373\u0377-\u0377\u037b-\u037d\u0390-\u0390\u03ac-\u03ce\u03d0-\u03d1\u03d5-\u03d7\u03d9-\u03d9\u03db-\u03db\u03dd-\u03dd\u03df-\u03df\u03e1-\u03e1\u03e3-\u03e3\u03e5-\u03e5\u03e7-\u03e7\u03e9-\u03e9\u03eb-\u03eb\u03ed-\u03ed\u03ef-\u03f3\u03f5-\u03f5\u03f8-\u03f8\u03fb-\u03fc\u0430-\u045f\u0461-\u0461\u0463-\u0463\u0465-\u0465\u0467-\u0467\u0469-\u0469\u046b-\u046b\u046d-\u046d\u046f-\u046f\u0471-\u0471\u0473-\u0473\u0475-\u0475\u0477-\u0477\u0479-\u0479\u047b-\u047b\u047d-\u047d\u047f-\u047f\u0481-\u0481\u048b-\u048b\u048d-\u048d\u048f-\u048f\u0491-\u0491\u0493-\u0493\u0495-\u0495\u0497-\u0497\u0499-\u0499\u049b-\u049b\u049d-\u049d\u049f-\u049f\u04a1-\u04a1\u04a3-\u04a3\u04a5-\u04a5\u04a7-\u04a7\u04a9-\u04a9\u04ab-\u04ab\u04ad-\u04ad\u04af-\u04af\u04b1-\u04b1\u04b3-\u04b3\u04b5-\u04b5\u04b7-\u04b7\u04b9-\u04b9\u04bb-\u04bb\u04bd-\u04bd\u04bf-\u04bf\u04c2-\u04c2\u04c4-\u04c4\u04c6-\u04c6\u04c8-\u04c8\u04ca-\u04ca\u04cc-\u04cc\u04ce-\u04cf\u04d1-\u04d1\u04d3-\u04d3\u04d5-\u04d5\u04d7-\u04d7\u04d9-\u04d9\u04db-\u04db\u04dd-\u04dd\u04df-\u04df\u04e1-\u04e1\u04e3-\u04e3\u04e5-\u04e5\u04e7-\u04e7\u04e9-\u04e9\u04eb-\u04eb\u04ed-\u04ed\u04ef-\u04ef\u04f1-\u04f1\u04f3-\u04f3\u04f5-\u04f5\u04f7-\u04f7\u04f9-\u04f9\u04fb-\u04fb\u04fd-\u04fd\u04ff-\u04ff\u0501-\u0501\u0503-\u0503\u0505-\u0505\u0507-\u0507\u0509-\u0509\u050b-\u050b\u050d-\u050d\u050f-\u050f\u0511-\u0511\u0513-\u0513\u0515-\u0515\u0517-\u0517\u0519-\u0519\u051b-\u051b\u051d-\u051d\u051f-\u051f\u0521-\u0521\u0523-\u0523\u0525-\u0525\u0527-\u0527\u0529-\u0529\u052b-\u052b\u052d-\u052d\u052f-\u052f\u0561-\u0587\u1d00-\u1d2b\u1d6b-\u1d77\u1d79-\u1d9a\u1e01-\u1e01\u1e03-\u1e03\u1e05-\u1e05\u1e07-\u1e07\u1e09-\u1e09\u1e0b-\u1e0b\u1e0d-\u1e0d\u1e0f-\u1e0f\u1e11-\u1e11\u1e13-\u1e13\u1e15-\u1e15\u1e17-\u1e17\u1e19-\u1e19\u1e1b-\u1e1b\u1e1d-\u1e1d\u1e1f-\u1e1f\u1e21-\u1e21\u1e23-\u1e23\u1e25-\u1e25\u1e27-\u1e27\u1e29-\u1e29\u1e2b-\u1e2b\u1e2d-\u1e2d\u1e2f-\u1e2f\u1e31-\u1e31\u1e33-\u1e33\u1e35-\u1e35\u1e37-\u1e37\u1e39-\u1e39\u1e3b-\u1e3b\u1e3d-\u1e3d\u1e3f-\u1e3f\u1e41-\u1e41\u1e43-\u1e43\u1e45-\u1e45\u1e47-\u1e47\u1e49-\u1e49\u1e4b-\u1e4b\u1e4d-\u1e4d\u1e4f-\u1e4f\u1e51-\u1e51\u1e53-\u1e53\u1e55-\u1e55\u1e57-\u1e57\u1e59-\u1e59\u1e5b-\u1e5b\u1e5d-\u1e5d\u1e5f-\u1e5f\u1e61-\u1e61\u1e63-\u1e63\u1e65-\u1e65\u1e67-\u1e67\u1e69-\u1e69\u1e6b-\u1e6b\u1e6d-\u1e6d\u1e6f-\u1e6f\u1e71-\u1e71\u1e73-\u1e73\u1e75-\u1e75\u1e77-\u1e77\u1e79-\u1e79\u1e7b-\u1e7b\u1e7d-\u1e7d\u1e7f-\u1e7f\u1e81-\u1e81\u1e83-\u1e83\u1e85-\u1e85\u1e87-\u1e87\u1e89-\u1e89\u1e8b-\u1e8b\u1e8d-\u1e8d\u1e8f-\u1e8f\u1e91-\u1e91\u1e93-\u1e93\u1e95-\u1e9d\u1e9f-\u1e9f\u1ea1-\u1ea1\u1ea3-\u1ea3\u1ea5-\u1ea5\u1ea7-\u1ea7\u1ea9-\u1ea9\u1eab-\u1eab\u1ead-\u1ead\u1eaf-\u1eaf\u1eb1-\u1eb1\u1eb3-\u1eb3\u1eb5-\u1eb5\u1eb7-\u1eb7\u1eb9-\u1eb9\u1ebb-\u1ebb\u1ebd-\u1ebd\u1ebf-\u1ebf\u1ec1-\u1ec1\u1ec3-\u1ec3\u1ec5-\u1ec5\u1ec7-\u1ec7\u1ec9-\u1ec9\u1ecb-\u1ecb\u1ecd-\u1ecd\u1ecf-\u1ecf\u1ed1-\u1ed1\u1ed3-\u1ed3\u1ed5-\u1ed5\u1ed7-\u1ed7\u1ed9-\u1ed9\u1edb-\u1edb\u1edd-\u1edd\u1edf-\u1edf\u1ee1-\u1ee1\u1ee3-\u1ee3\u1ee5-\u1ee5\u1ee7-\u1ee7\u1ee9-\u1ee9\u1eeb-\u1eeb\u1eed-\u1eed\u1eef-\u1eef\u1ef1-\u1ef1\u1ef3-\u1ef3\u1ef5-\u1ef5\u1ef7-\u1ef7\u1ef9-\u1ef9\u1efb-\u1efb\u1efd-\u1efd\u1eff-\u1f07\u1f10-\u1f15\u1f20-\u1f27\u1f30-\u1f37\u1f40-\u1f45\u1f50-\u1f57\u1f60-\u1f67\u1f70-\u1f7d\u1f80-\u1f87\u1f90-\u1f97\u1fa0-\u1fa7\u1fb0-\u1fb4\u1fb6-\u1fb7\u1fbe-\u1fbe\u1fc2-\u1fc4\u1fc6-\u1fc7\u1fd0-\u1fd3\u1fd6-\u1fd7\u1fe0-\u1fe7\u1ff2-\u1ff4\u1ff6-\u1ff7\u210a-\u210a\u210e-\u210f\u2113-\u2113\u212f-\u212f\u2134-\u2134\u2139-\u2139\u213c-\u213d\u2146-\u2149\u214e-\u214e\u2184-\u2184\u2c30-\u2c5e\u2c61-\u2c61\u2c65-\u2c66\u2c68-\u2c68\u2c6a-\u2c6a\u2c6c-\u2c6c\u2c71-\u2c71\u2c73-\u2c74\u2c76-\u2c7b\u2c81-\u2c81\u2c83-\u2c83\u2c85-\u2c85\u2c87-\u2c87\u2c89-\u2c89\u2c8b-\u2c8b\u2c8d-\u2c8d\u2c8f-\u2c8f\u2c91-\u2c91\u2c93-\u2c93\u2c95-\u2c95\u2c97-\u2c97\u2c99-\u2c99\u2c9b-\u2c9b\u2c9d-\u2c9d\u2c9f-\u2c9f\u2ca1-\u2ca1\u2ca3-\u2ca3\u2ca5-\u2ca5\u2ca7-\u2ca7\u2ca9-\u2ca9\u2cab-\u2cab\u2cad-\u2cad\u2caf-\u2caf\u2cb1-\u2cb1\u2cb3-\u2cb3\u2cb5-\u2cb5\u2cb7-\u2cb7\u2cb9-\u2cb9\u2cbb-\u2cbb\u2cbd-\u2cbd\u2cbf-\u2cbf\u2cc1-\u2cc1\u2cc3-\u2cc3\u2cc5-\u2cc5\u2cc7-\u2cc7\u2cc9-\u2cc9\u2ccb-\u2ccb\u2ccd-\u2ccd\u2ccf-\u2ccf\u2cd1-\u2cd1\u2cd3-\u2cd3\u2cd5-\u2cd5\u2cd7-\u2cd7\u2cd9-\u2cd9\u2cdb-\u2cdb\u2cdd-\u2cdd\u2cdf-\u2cdf\u2ce1-\u2ce1\u2ce3-\u2ce4\u2cec-\u2cec\u2cee-\u2cee\u2cf3-\u2cf3\u2d00-\u2d25\u2d27-\u2d27\u2d2d-\u2d2d\ua641-\ua641\ua643-\ua643\ua645-\ua645\ua647-\ua647\ua649-\ua649\ua64b-\ua64b\ua64d-\ua64d\ua64f-\ua64f\ua651-\ua651\ua653-\ua653\ua655-\ua655\ua657-\ua657\ua659-\ua659\ua65b-\ua65b\ua65d-\ua65d\ua65f-\ua65f\ua661-\ua661\ua663-\ua663\ua665-\ua665\ua667-\ua667\ua669-\ua669\ua66b-\ua66b\ua66d-\ua66d\ua681-\ua681\ua683-\ua683\ua685-\ua685\ua687-\ua687\ua689-\ua689\ua68b-\ua68b\ua68d-\ua68d\ua68f-\ua68f\ua691-\ua691\ua693-\ua693\ua695-\ua695\ua697-\ua697\ua699-\ua699\ua69b-\ua69b\ua723-\ua723\ua725-\ua725\ua727-\ua727\ua729-\ua729\ua72b-\ua72b\ua72d-\ua72d\ua72f-\ua731\ua733-\ua733\ua735-\ua735\ua737-\ua737\ua739-\ua739\ua73b-\ua73b\ua73d-\ua73d\ua73f-\ua73f\ua741-\ua741\ua743-\ua743\ua745-\ua745\ua747-\ua747\ua749-\ua749\ua74b-\ua74b\ua74d-\ua74d\ua74f-\ua74f\ua751-\ua751\ua753-\ua753\ua755-\ua755\ua757-\ua757\ua759-\ua759\ua75b-\ua75b\ua75d-\ua75d\ua75f-\ua75f\ua761-\ua761\ua763-\ua763\ua765-\ua765\ua767-\ua767\ua769-\ua769\ua76b-\ua76b\ua76d-\ua76d\ua76f-\ua76f\ua771-\ua778\ua77a-\ua77a\ua77c-\ua77c\ua77f-\ua77f\ua781-\ua781\ua783-\ua783\ua785-\ua785\ua787-\ua787\ua78c-\ua78c\ua78e-\ua78e\ua791-\ua791\ua793-\ua795\ua797-\ua797\ua799-\ua799\ua79b-\ua79b\ua79d-\ua79d\ua79f-\ua79f\ua7a1-\ua7a1\ua7a3-\ua7a3\ua7a5-\ua7a5\ua7a7-\ua7a7\ua7a9-\ua7a9\ua7fa-\ua7fa\uab30-\uab5a\uab64-\uab65\ufb00-\ufb06\ufb13-\ufb17\uff41-\uff5a\U00010428-\U0001044f\U000118c0-\U000118df\U0001d41a-\U0001d433\U0001d44e-\U0001d454\U0001d456-\U0001d467\U0001d482-\U0001d49b\U0001d4b6-\U0001d4b9\U0001d4bb-\U0001d4bb\U0001d4bd-\U0001d4c3\U0001d4c5-\U0001d4cf\U0001d4ea-\U0001d503\U0001d51e-\U0001d537\U0001d552-\U0001d56b\U0001d586-\U0001d59f\U0001d5ba-\U0001d5d3\U0001d5ee-\U0001d607\U0001d622-\U0001d63b\U0001d656-\U0001d66f\U0001d68a-\U0001d6a5\U0001d6c2-\U0001d6da\U0001d6dc-\U0001d6e1\U0001d6fc-\U0001d714\U0001d716-\U0001d71b\U0001d736-\U0001d74e\U0001d750-\U0001d755\U0001d770-\U0001d788\U0001d78a-\U0001d78f\U0001d7aa-\U0001d7c2\U0001d7c4-\U0001d7c9\U0001d7cb-\U0001d7cb];
		Ll { goto Ll; }
		* { return YYCURSOR == limit; }
	*/
}
static const unsigned int chars_Ll [] = {0x61,0x7a,  0xb5,0xb5,  0xdf,0xf6,  0xf8,0xff,  0x101,0x101,  0x103,0x103,  0x105,0x105,  0x107,0x107,  0x109,0x109,  0x10b,0x10b,  0x10d,0x10d,  0x10f,0x10f,  0x111,0x111,  0x113,0x113,  0x115,0x115,  0x117,0x117,  0x119,0x119,  0x11b,0x11b,  0x11d,0x11d,  0x11f,0x11f,  0x121,0x121,  0x123,0x123,  0x125,0x125,  0x127,0x127,  0x129,0x129,  0x12b,0x12b,  0x12d,0x12d,  0x12f,0x12f,  0x131,0x131,  0x133,0x133,  0x135,0x135,  0x137,0x138,  0x13a,0x13a,  0x13c,0x13c,  0x13e,0x13e,  0x140,0x140,  0x142,0x142,  0x144,0x144,  0x146,0x146,  0x148,0x149,  0x14b,0x14b,  0x14d,0x14d,  0x14f,0x14f,  0x151,0x151,  0x153,0x153,  0x155,0x155,  0x157,0x157,  0x159,0x159,  0x15b,0x15b,  0x15d,0x15d,  0x15f,0x15f,  0x161,0x161,  0x163,0x163,  0x165,0x165,  0x167,0x167,  0x169,0x169,  0x16b,0x16b,  0x16d,0x16d,  0x16f,0x16f,  0x171,0x171,  0x173,0x173,  0x175,0x175,  0x177,0x177,  0x17a,0x17a,  0x17c,0x17c,  0x17e,0x180,  0x183,0x183,  0x185,0x185,  0x188,0x188,  0x18c,0x18d,  0x192,0x192,  0x195,0x195,  0x199,0x19b,  0x19e,0x19e,  0x1a1,0x1a1,  0x1a3,0x1a3,  0x1a5,0x1a5,  0x1a8,0x1a8,  0x1aa,0x1ab,  0x1ad,0x1ad,  0x1b0,0x1b0,  0x1b4,0x1b4,  0x1b6,0x1b6,  0x1b9,0x1ba,  0x1bd,0x1bf,  0x1c6,0x1c6,  0x1c9,0x1c9,  0x1cc,0x1cc,  0x1ce,0x1ce,  0x1d0,0x1d0,  0x1d2,0x1d2,  0x1d4,0x1d4,  0x1d6,0x1d6,  0x1d8,0x1d8,  0x1da,0x1da,  0x1dc,0x1dd,  0x1df,0x1df,  0x1e1,0x1e1,  0x1e3,0x1e3,  0x1e5,0x1e5,  0x1e7,0x1e7,  0x1e9,0x1e9,  0x1eb,0x1eb,  0x1ed,0x1ed,  0x1ef,0x1f0,  0x1f3,0x1f3,  0x1f5,0x1f5,  0x1f9,0x1f9,  0x1fb,0x1fb,  0x1fd,0x1fd,  0x1ff,0x1ff,  0x201,0x201,  0x203,0x203,  0x205,0x205,  0x207,0x207,  0x209,0x209,  0x20b,0x20b,  0x20d,0x20d,  0x20f,0x20f,  0x211,0x211,  0x213,0x213,  0x215,0x215,  0x217,0x217,  0x219,0x219,  0x21b,0x21b,  0x21d,0x21d,  0x21f,0x21f,  0x221,0x221,  0x223,0x223,  0x225,0x225,  0x227,0x227,  0x229,0x229,  0x22b,0x22b,  0x22d,0x22d,  0x22f,0x22f,  0x231,0x231,  0x233,0x239,  0x23c,0x23c,  0x23f,0x240,  0x242,0x242,  0x247,0x247,  0x249,0x249,  0x24b,0x24b,  0x24d,0x24d,  0x24f,0x293,  0x295,0x2af,  0x371,0x371,  0x373,0x373,  0x377,0x377,  0x37b,0x37d,  0x390,0x390,  0x3ac,0x3ce,  0x3d0,0x3d1,  0x3d5,0x3d7,  0x3d9,0x3d9,  0x3db,0x3db,  0x3dd,0x3dd,  0x3df,0x3df,  0x3e1,0x3e1,  0x3e3,0x3e3,  0x3e5,0x3e5,  0x3e7,0x3e7,  0x3e9,0x3e9,  0x3eb,0x3eb,  0x3ed,0x3ed,  0x3ef,0x3f3,  0x3f5,0x3f5,  0x3f8,0x3f8,  0x3fb,0x3fc,  0x430,0x45f,  0x461,0x461,  0x463,0x463,  0x465,0x465,  0x467,0x467,  0x469,0x469,  0x46b,0x46b,  0x46d,0x46d,  0x46f,0x46f,  0x471,0x471,  0x473,0x473,  0x475,0x475,  0x477,0x477,  0x479,0x479,  0x47b,0x47b,  0x47d,0x47d,  0x47f,0x47f,  0x481,0x481,  0x48b,0x48b,  0x48d,0x48d,  0x48f,0x48f,  0x491,0x491,  0x493,0x493,  0x495,0x495,  0x497,0x497,  0x499,0x499,  0x49b,0x49b,  0x49d,0x49d,  0x49f,0x49f,  0x4a1,0x4a1,  0x4a3,0x4a3,  0x4a5,0x4a5,  0x4a7,0x4a7,  0x4a9,0x4a9,  0x4ab,0x4ab,  0x4ad,0x4ad,  0x4af,0x4af,  0x4b1,0x4b1,  0x4b3,0x4b3,  0x4b5,0x4b5,  0x4b7,0x4b7,  0x4b9,0x4b9,  0x4bb,0x4bb,  0x4bd,0x4bd,  0x4bf,0x4bf,  0x4c2,0x4c2,  0x4c4,0x4c4,  0x4c6,0x4c6,  0x4c8,0x4c8,  0x4ca,0x4ca,  0x4cc,0x4cc,  0x4ce,0x4cf,  0x4d1,0x4d1,  0x4d3,0x4d3,  0x4d5,0x4d5,  0x4d7,0x4d7,  0x4d9,0x4d9,  0x4db,0x4db,  0x4dd,0x4dd,  0x4df,0x4df,  0x4e1,0x4e1,  0x4e3,0x4e3,  0x4e5,0x4e5,  0x4e7,0x4e7,  0x4e9,0x4e9,  0x4eb,0x4eb,  0x4ed,0x4ed,  0x4ef,0x4ef,  0x4f1,0x4f1,  0x4f3,0x4f3,  0x4f5,0x4f5,  0x4f7,0x4f7,  0x4f9,0x4f9,  0x4fb,0x4fb,  0x4fd,0x4fd,  0x4ff,0x4ff,  0x501,0x501,  0x503,0x503,  0x505,0x505,  0x507,0x507,  0x509,0x509,  0x50b,0x50b,  0x50d,0x50d,  0x50f,0x50f,  0x511,0x511,  0x513,0x513,  0x515,0x515,  0x517,0x517,  0x519,0x519,  0x51b,0x51b,  0x51d,0x51d,  0x51f,0x51f,  0x521,0x521,  0x523,0x523,  0x525,0x525,  0x527,0x527,  0x529,0x529,  0x52b,0x52b,  0x52d,0x52d,  0x52f,0x52f,  0x561,0x587,  0x1d00,0x1d2b,  0x1d6b,0x1d77,  0x1d79,0x1d9a,  0x1e01,0x1e01,  0x1e03,0x1e03,  0x1e05,0x1e05,  0x1e07,0x1e07,  0x1e09,0x1e09,  0x1e0b,0x1e0b,  0x1e0d,0x1e0d,  0x1e0f,0x1e0f,  0x1e11,0x1e11,  0x1e13,0x1e13,  0x1e15,0x1e15,  0x1e17,0x1e17,  0x1e19,0x1e19,  0x1e1b,0x1e1b,  0x1e1d,0x1e1d,  0x1e1f,0x1e1f,  0x1e21,0x1e21,  0x1e23,0x1e23,  0x1e25,0x1e25,  0x1e27,0x1e27,  0x1e29,0x1e29,  0x1e2b,0x1e2b,  0x1e2d,0x1e2d,  0x1e2f,0x1e2f,  0x1e31,0x1e31,  0x1e33,0x1e33,  0x1e35,0x1e35,  0x1e37,0x1e37,  0x1e39,0x1e39,  0x1e3b,0x1e3b,  0x1e3d,0x1e3d,  0x1e3f,0x1e3f,  0x1e41,0x1e41,  0x1e43,0x1e43,  0x1e45,0x1e45,  0x1e47,0x1e47,  0x1e49,0x1e49,  0x1e4b,0x1e4b,  0x1e4d,0x1e4d,  0x1e4f,0x1e4f,  0x1e51,0x1e51,  0x1e53,0x1e53,  0x1e55,0x1e55,  0x1e57,0x1e57,  0x1e59,0x1e59,  0x1e5b,0x1e5b,  0x1e5d,0x1e5d,  0x1e5f,0x1e5f,  0x1e61,0x1e61,  0x1e63,0x1e63,  0x1e65,0x1e65,  0x1e67,0x1e67,  0x1e69,0x1e69,  0x1e6b,0x1e6b,  0x1e6d,0x1e6d,  0x1e6f,0x1e6f,  0x1e71,0x1e71,  0x1e73,0x1e73,  0x1e75,0x1e75,  0x1e77,0x1e77,  0x1e79,0x1e79,  0x1e7b,0x1e7b,  0x1e7d,0x1e7d,  0x1e7f,0x1e7f,  0x1e81,0x1e81,  0x1e83,0x1e83,  0x1e85,0x1e85,  0x1e87,0x1e87,  0x1e89,0x1e89,  0x1e8b,0x1e8b,  0x1e8d,0x1e8d,  0x1e8f,0x1e8f,  0x1e91,0x1e91,  0x1e93,0x1e93,  0x1e95,0x1e9d,  0x1e9f,0x1e9f,  0x1ea1,0x1ea1,  0x1ea3,0x1ea3,  0x1ea5,0x1ea5,  0x1ea7,0x1ea7,  0x1ea9,0x1ea9,  0x1eab,0x1eab,  0x1ead,0x1ead,  0x1eaf,0x1eaf,  0x1eb1,0x1eb1,  0x1eb3,0x1eb3,  0x1eb5,0x1eb5,  0x1eb7,0x1eb7,  0x1eb9,0x1eb9,  0x1ebb,0x1ebb,  0x1ebd,0x1ebd,  0x1ebf,0x1ebf,  0x1ec1,0x1ec1,  0x1ec3,0x1ec3,  0x1ec5,0x1ec5,  0x1ec7,0x1ec7,  0x1ec9,0x1ec9,  0x1ecb,0x1ecb,  0x1ecd,0x1ecd,  0x1ecf,0x1ecf,  0x1ed1,0x1ed1,  0x1ed3,0x1ed3,  0x1ed5,0x1ed5,  0x1ed7,0x1ed7,  0x1ed9,0x1ed9,  0x1edb,0x1edb,  0x1edd,0x1edd,  0x1edf,0x1edf,  0x1ee1,0x1ee1,  0x1ee3,0x1ee3,  0x1ee5,0x1ee5,  0x1ee7,0x1ee7,  0x1ee9,0x1ee9,  0x1eeb,0x1eeb,  0x1eed,0x1eed,  0x1eef,0x1eef,  0x1ef1,0x1ef1,  0x1ef3,0x1ef3,  0x1ef5,0x1ef5,  0x1ef7,0x1ef7,  0x1ef9,0x1ef9,  0x1efb,0x1efb,  0x1efd,0x1efd,  0x1eff,0x1f07,  0x1f10,0x1f15,  0x1f20,0x1f27,  0x1f30,0x1f37,  0x1f40,0x1f45,  0x1f50,0x1f57,  0x1f60,0x1f67,  0x1f70,0x1f7d,  0x1f80,0x1f87,  0x1f90,0x1f97,  0x1fa0,0x1fa7,  0x1fb0,0x1fb4,  0x1fb6,0x1fb7,  0x1fbe,0x1fbe,  0x1fc2,0x1fc4,  0x1fc6,0x1fc7,  0x1fd0,0x1fd3,  0x1fd6,0x1fd7,  0x1fe0,0x1fe7,  0x1ff2,0x1ff4,  0x1ff6,0x1ff7,  0x210a,0x210a,  0x210e,0x210f,  0x2113,0x2113,  0x212f,0x212f,  0x2134,0x2134,  0x2139,0x2139,  0x213c,0x213d,  0x2146,0x2149,  0x214e,0x214e,  0x2184,0x2184,  0x2c30,0x2c5e,  0x2c61,0x2c61,  0x2c65,0x2c66,  0x2c68,0x2c68,  0x2c6a,0x2c6a,  0x2c6c,0x2c6c,  0x2c71,0x2c71,  0x2c73,0x2c74,  0x2c76,0x2c7b,  0x2c81,0x2c81,  0x2c83,0x2c83,  0x2c85,0x2c85,  0x2c87,0x2c87,  0x2c89,0x2c89,  0x2c8b,0x2c8b,  0x2c8d,0x2c8d,  0x2c8f,0x2c8f,  0x2c91,0x2c91,  0x2c93,0x2c93,  0x2c95,0x2c95,  0x2c97,0x2c97,  0x2c99,0x2c99,  0x2c9b,0x2c9b,  0x2c9d,0x2c9d,  0x2c9f,0x2c9f,  0x2ca1,0x2ca1,  0x2ca3,0x2ca3,  0x2ca5,0x2ca5,  0x2ca7,0x2ca7,  0x2ca9,0x2ca9,  0x2cab,0x2cab,  0x2cad,0x2cad,  0x2caf,0x2caf,  0x2cb1,0x2cb1,  0x2cb3,0x2cb3,  0x2cb5,0x2cb5,  0x2cb7,0x2cb7,  0x2cb9,0x2cb9,  0x2cbb,0x2cbb,  0x2cbd,0x2cbd,  0x2cbf,0x2cbf,  0x2cc1,0x2cc1,  0x2cc3,0x2cc3,  0x2cc5,0x2cc5,  0x2cc7,0x2cc7,  0x2cc9,0x2cc9,  0x2ccb,0x2ccb,  0x2ccd,0x2ccd,  0x2ccf,0x2ccf,  0x2cd1,0x2cd1,  0x2cd3,0x2cd3,  0x2cd5,0x2cd5,  0x2cd7,0x2cd7,  0x2cd9,0x2cd9,  0x2cdb,0x2cdb,  0x2cdd,0x2cdd,  0x2cdf,0x2cdf,  0x2ce1,0x2ce1,  0x2ce3,0x2ce4,  0x2cec,0x2cec,  0x2cee,0x2cee,  0x2cf3,0x2cf3,  0x2d00,0x2d25,  0x2d27,0x2d27,  0x2d2d,0x2d2d,  0xa641,0xa641,  0xa643,0xa643,  0xa645,0xa645,  0xa647,0xa647,  0xa649,0xa649,  0xa64b,0xa64b,  0xa64d,0xa64d,  0xa64f,0xa64f,  0xa651,0xa651,  0xa653,0xa653,  0xa655,0xa655,  0xa657,0xa657,  0xa659,0xa659,  0xa65b,0xa65b,  0xa65d,0xa65d,  0xa65f,0xa65f,  0xa661,0xa661,  0xa663,0xa663,  0xa665,0xa665,  0xa667,0xa667,  0xa669,0xa669,  0xa66b,0xa66b,  0xa66d,0xa66d,  0xa681,0xa681,  0xa683,0xa683,  0xa685,0xa685,  0xa687,0xa687,  0xa689,0xa689,  0xa68b,0xa68b,  0xa68d,0xa68d,  0xa68f,0xa68f,  0xa691,0xa691,  0xa693,0xa693,  0xa695,0xa695,  0xa697,0xa697,  0xa699,0xa699,  0xa69b,0xa69b,  0xa723,0xa723,  0xa725,0xa725,  0xa727,0xa727,  0xa729,0xa729,  0xa72b,0xa72b,  0xa72d,0xa72d,  0xa72f,0xa731,  0xa733,0xa733,  0xa735,0xa735,  0xa737,0xa737,  0xa739,0xa739,  0xa73b,0xa73b,  0xa73d,0xa73d,  0xa73f,0xa73f,  0xa741,0xa741,  0xa743,0xa743,  0xa745,0xa745,  0xa747,0xa747,  0xa749,0xa749,  0xa74b,0xa74b,  0xa74d,0xa74d,  0xa74f,0xa74f,  0xa751,0xa751,  0xa753,0xa753,  0xa755,0xa755,  0xa757,0xa757,  0xa759,0xa759,  0xa75b,0xa75b,  0xa75d,0xa75d,  0xa75f,0xa75f,  0xa761,0xa761,  0xa763,0xa763,  0xa765,0xa765,  0xa767,0xa767,  0xa769,0xa769,  0xa76b,0xa76b,  0xa76d,0xa76d,  0xa76f,0xa76f,  0xa771,0xa778,  0xa77a,0xa77a,  0xa77c,0xa77c,  0xa77f,0xa77f,  0xa781,0xa781,  0xa783,0xa783,  0xa785,0xa785,  0xa787,0xa787,  0xa78c,0xa78c,  0xa78e,0xa78e,  0xa791,0xa791,  0xa793,0xa795,  0xa797,0xa797,  0xa799,0xa799,  0xa79b,0xa79b,  0xa79d,0xa79d,  0xa79f,0xa79f,  0xa7a1,0xa7a1,  0xa7a3,0xa7a3,  0xa7a5,0xa7a5,  0xa7a7,0xa7a7,  0xa7a9,0xa7a9,  0xa7fa,0xa7fa,  0xab30,0xab5a,  0xab64,0xab65,  0xfb00,0xfb06,  0xfb13,0xfb17,  0xff41,0xff5a,  0x10428,0x1044f,  0x118c0,0x118df,  0x1d41a,0x1d433,  0x1d44e,0x1d454,  0x1d456,0x1d467,  0x1d482,0x1d49b,  0x1d4b6,0x1d4b9,  0x1d4bb,0x1d4bb,  0x1d4bd,0x1d4c3,  0x1d4c5,0x1d4cf,  0x1d4ea,0x1d503,  0x1d51e,0x1d537,  0x1d552,0x1d56b,  0x1d586,0x1d59f,  0x1d5ba,0x1d5d3,  0x1d5ee,0x1d607,  0x1d622,0x1d63b,  0x1d656,0x1d66f,  0x1d68a,0x1d6a5,  0x1d6c2,0x1d6da,  0x1d6dc,0x1d6e1,  0x1d6fc,0x1d714,  0x1d716,0x1d71b,  0x1d736,0x1d74e,  0x1d750,0x1d755,  0x1d770,0x1d788,  0x1d78a,0x1d78f,  0x1d7aa,0x1d7c2,  0x1d7c4,0x1d7c9,  0x1d7cb,0x1d7cb,  0x0,0x0};
static unsigned int encode_utf32 (const unsigned int * ranges, unsigned int ranges_count, unsigned int * s)
{
	unsigned int * const s_start = s;
	for (unsigned int i = 0; i < ranges_count; i += 2)
		for (unsigned int j = ranges[i]; j <= ranges[i + 1]; ++j)
			*s++ = j;
	return s - s_start;
}

int main ()
{
	unsigned int * buffer_Ll = new unsigned int [1842];
	YYCTYPE * s = (YYCTYPE *) buffer_Ll;
	unsigned int buffer_len = encode_utf32 (chars_Ll, sizeof (chars_Ll) / sizeof (unsigned int), buffer_Ll);
	/* convert 32-bit code units to YYCTYPE; reuse the same buffer */
	for (unsigned int i = 0; i < buffer_len; ++i) s[i] = buffer_Ll[i];
	if (!scan (s, s + buffer_len))
		printf("test 'Ll' failed\n");
	delete [] buffer_Ll;
	return 0;
}
