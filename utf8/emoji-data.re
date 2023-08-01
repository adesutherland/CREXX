/*!re2c
Emoji = [\x23\x2a\x30-\x39\xa9\xae\u203c\u2049\u2122\u2139\u2194-\u2199\u21a9-\u21aa\u231a-\u231b\u2328\u23cf\u23e9-\u23ec\u23ed-\u23ee\u23ef\u23f0\u23f1-\u23f2\u23f3\u23f8-\u23fa\u24c2\u25aa-\u25ab\u25b6\u25c0\u25fb-\u25fe\u2600-\u2601\u2602-\u2603\u2604\u260e\u2611\u2614-\u2615\u2618\u261d\u2620\u2622-\u2623\u2626\u262a\u262e\u262f\u2638-\u2639\u263a\u2640\u2642\u2648-\u2653\u265f\u2660\u2663\u2665-\u2666\u2668\u267b\u267e\u267f\u2692\u2693\u2694\u2695\u2696-\u2697\u2699\u269b-\u269c\u26a0-\u26a1\u26a7\u26aa-\u26ab\u26b0-\u26b1\u26bd-\u26be\u26c4-\u26c5\u26c8\u26ce\u26cf\u26d1\u26d3\u26d4\u26e9\u26ea\u26f0-\u26f1\u26f2-\u26f3\u26f4\u26f5\u26f7-\u26f9\u26fa\u26fd\u2702\u2705\u2708-\u270c\u270d\u270f\u2712\u2714\u2716\u271d\u2721\u2728\u2733-\u2734\u2744\u2747\u274c\u274e\u2753-\u2755\u2757\u2763\u2764\u2795-\u2797\u27a1\u27b0\u27bf\u2934-\u2935\u2b05-\u2b07\u2b1b-\u2b1c\u2b50\u2b55\u3030\u303d\u3297\u3299\U0001f004\U0001f0cf\U0001f170-\U0001f171\U0001f17e-\U0001f17f\U0001f18e\U0001f191-\U0001f19a\U0001f1e6-\U0001f1ff\U0001f201-\U0001f202\U0001f21a\U0001f22f\U0001f232-\U0001f23a\U0001f250-\U0001f251\U0001f300-\U0001f30c\U0001f30d-\U0001f30e\U0001f30f\U0001f310\U0001f311\U0001f312\U0001f313-\U0001f315\U0001f316-\U0001f318\U0001f319\U0001f31a\U0001f31b\U0001f31c\U0001f31d-\U0001f31e\U0001f31f-\U0001f320\U0001f321\U0001f324-\U0001f32c\U0001f32d-\U0001f32f\U0001f330-\U0001f331\U0001f332-\U0001f333\U0001f334-\U0001f335\U0001f336\U0001f337-\U0001f34a\U0001f34b\U0001f34c-\U0001f34f\U0001f350\U0001f351-\U0001f37b\U0001f37c\U0001f37d\U0001f37e-\U0001f37f\U0001f380-\U0001f393\U0001f396-\U0001f397\U0001f399-\U0001f39b\U0001f39e-\U0001f39f\U0001f3a0-\U0001f3c4\U0001f3c5\U0001f3c6\U0001f3c7\U0001f3c8\U0001f3c9\U0001f3ca\U0001f3cb-\U0001f3ce\U0001f3cf-\U0001f3d3\U0001f3d4-\U0001f3df\U0001f3e0-\U0001f3e3\U0001f3e4\U0001f3e5-\U0001f3f0\U0001f3f3\U0001f3f4\U0001f3f5\U0001f3f7\U0001f3f8-\U0001f407\U0001f408\U0001f409-\U0001f40b\U0001f40c-\U0001f40e\U0001f40f-\U0001f410\U0001f411-\U0001f412\U0001f413\U0001f414\U0001f415\U0001f416\U0001f417-\U0001f429\U0001f42a\U0001f42b-\U0001f43e\U0001f43f\U0001f440\U0001f441\U0001f442-\U0001f464\U0001f465\U0001f466-\U0001f46b\U0001f46c-\U0001f46d\U0001f46e-\U0001f4ac\U0001f4ad\U0001f4ae-\U0001f4b5\U0001f4b6-\U0001f4b7\U0001f4b8-\U0001f4eb\U0001f4ec-\U0001f4ed\U0001f4ee\U0001f4ef\U0001f4f0-\U0001f4f4\U0001f4f5\U0001f4f6-\U0001f4f7\U0001f4f8\U0001f4f9-\U0001f4fc\U0001f4fd\U0001f4ff-\U0001f502\U0001f503\U0001f504-\U0001f507\U0001f508\U0001f509\U0001f50a-\U0001f514\U0001f515\U0001f516-\U0001f52b\U0001f52c-\U0001f52d\U0001f52e-\U0001f53d\U0001f549-\U0001f54a\U0001f54b-\U0001f54e\U0001f550-\U0001f55b\U0001f55c-\U0001f567\U0001f56f-\U0001f570\U0001f573-\U0001f579\U0001f57a\U0001f587\U0001f58a-\U0001f58d\U0001f590\U0001f595-\U0001f596\U0001f5a4\U0001f5a5\U0001f5a8\U0001f5b1-\U0001f5b2\U0001f5bc\U0001f5c2-\U0001f5c4\U0001f5d1-\U0001f5d3\U0001f5dc-\U0001f5de\U0001f5e1\U0001f5e3\U0001f5e8\U0001f5ef\U0001f5f3\U0001f5fa\U0001f5fb-\U0001f5ff\U0001f600\U0001f601-\U0001f606\U0001f607-\U0001f608\U0001f609-\U0001f60d\U0001f60e\U0001f60f\U0001f610\U0001f611\U0001f612-\U0001f614\U0001f615\U0001f616\U0001f617\U0001f618\U0001f619\U0001f61a\U0001f61b\U0001f61c-\U0001f61e\U0001f61f\U0001f620-\U0001f625\U0001f626-\U0001f627\U0001f628-\U0001f62b\U0001f62c\U0001f62d\U0001f62e-\U0001f62f\U0001f630-\U0001f633\U0001f634\U0001f635\U0001f636\U0001f637-\U0001f640\U0001f641-\U0001f644\U0001f645-\U0001f64f\U0001f680\U0001f681-\U0001f682\U0001f683-\U0001f685\U0001f686\U0001f687\U0001f688\U0001f689\U0001f68a-\U0001f68b\U0001f68c\U0001f68d\U0001f68e\U0001f68f\U0001f690\U0001f691-\U0001f693\U0001f694\U0001f695\U0001f696\U0001f697\U0001f698\U0001f699-\U0001f69a\U0001f69b-\U0001f6a1\U0001f6a2\U0001f6a3\U0001f6a4-\U0001f6a5\U0001f6a6\U0001f6a7-\U0001f6ad\U0001f6ae-\U0001f6b1\U0001f6b2\U0001f6b3-\U0001f6b5\U0001f6b6\U0001f6b7-\U0001f6b8\U0001f6b9-\U0001f6be\U0001f6bf\U0001f6c0\U0001f6c1-\U0001f6c5\U0001f6cb\U0001f6cc\U0001f6cd-\U0001f6cf\U0001f6d0\U0001f6d1-\U0001f6d2\U0001f6d5\U0001f6d6-\U0001f6d7\U0001f6dc\U0001f6dd-\U0001f6df\U0001f6e0-\U0001f6e5\U0001f6e9\U0001f6eb-\U0001f6ec\U0001f6f0\U0001f6f3\U0001f6f4-\U0001f6f6\U0001f6f7-\U0001f6f8\U0001f6f9\U0001f6fa\U0001f6fb-\U0001f6fc\U0001f7e0-\U0001f7eb\U0001f7f0\U0001f90c\U0001f90d-\U0001f90f\U0001f910-\U0001f918\U0001f919-\U0001f91e\U0001f91f\U0001f920-\U0001f927\U0001f928-\U0001f92f\U0001f930\U0001f931-\U0001f932\U0001f933-\U0001f93a\U0001f93c-\U0001f93e\U0001f93f\U0001f940-\U0001f945\U0001f947-\U0001f94b\U0001f94c\U0001f94d-\U0001f94f\U0001f950-\U0001f95e\U0001f95f-\U0001f96b\U0001f96c-\U0001f970\U0001f971\U0001f972\U0001f973-\U0001f976\U0001f977-\U0001f978\U0001f979\U0001f97a\U0001f97b\U0001f97c-\U0001f97f\U0001f980-\U0001f984\U0001f985-\U0001f991\U0001f992-\U0001f997\U0001f998-\U0001f9a2\U0001f9a3-\U0001f9a4\U0001f9a5-\U0001f9aa\U0001f9ab-\U0001f9ad\U0001f9ae-\U0001f9af\U0001f9b0-\U0001f9b9\U0001f9ba-\U0001f9bf\U0001f9c0\U0001f9c1-\U0001f9c2\U0001f9c3-\U0001f9ca\U0001f9cb\U0001f9cc\U0001f9cd-\U0001f9cf\U0001f9d0-\U0001f9e6\U0001f9e7-\U0001f9ff\U0001fa70-\U0001fa73\U0001fa74\U0001fa75-\U0001fa77\U0001fa78-\U0001fa7a\U0001fa7b-\U0001fa7c\U0001fa80-\U0001fa82\U0001fa83-\U0001fa86\U0001fa87-\U0001fa88\U0001fa90-\U0001fa95\U0001fa96-\U0001faa8\U0001faa9-\U0001faac\U0001faad-\U0001faaf\U0001fab0-\U0001fab6\U0001fab7-\U0001faba\U0001fabb-\U0001fabd\U0001fabf\U0001fac0-\U0001fac2\U0001fac3-\U0001fac5\U0001face-\U0001facf\U0001fad0-\U0001fad6\U0001fad7-\U0001fad9\U0001fada-\U0001fadb\U0001fae0-\U0001fae7\U0001fae8\U0001faf0-\U0001faf6\U0001faf7-\U0001faf8];
Emoji_Presentation = [\u231a-\u231b\u23e9-\u23ec\u23f0\u23f3\u25fd-\u25fe\u2614-\u2615\u2648-\u2653\u267f\u2693\u26a1\u26aa-\u26ab\u26bd-\u26be\u26c4-\u26c5\u26ce\u26d4\u26ea\u26f2-\u26f3\u26f5\u26fa\u26fd\u2705\u270a-\u270b\u2728\u274c\u274e\u2753-\u2755\u2757\u2795-\u2797\u27b0\u27bf\u2b1b-\u2b1c\u2b50\u2b55\U0001f004\U0001f0cf\U0001f18e\U0001f191-\U0001f19a\U0001f1e6-\U0001f1ff\U0001f201\U0001f21a\U0001f22f\U0001f232-\U0001f236\U0001f238-\U0001f23a\U0001f250-\U0001f251\U0001f300-\U0001f30c\U0001f30d-\U0001f30e\U0001f30f\U0001f310\U0001f311\U0001f312\U0001f313-\U0001f315\U0001f316-\U0001f318\U0001f319\U0001f31a\U0001f31b\U0001f31c\U0001f31d-\U0001f31e\U0001f31f-\U0001f320\U0001f32d-\U0001f32f\U0001f330-\U0001f331\U0001f332-\U0001f333\U0001f334-\U0001f335\U0001f337-\U0001f34a\U0001f34b\U0001f34c-\U0001f34f\U0001f350\U0001f351-\U0001f37b\U0001f37c\U0001f37e-\U0001f37f\U0001f380-\U0001f393\U0001f3a0-\U0001f3c4\U0001f3c5\U0001f3c6\U0001f3c7\U0001f3c8\U0001f3c9\U0001f3ca\U0001f3cf-\U0001f3d3\U0001f3e0-\U0001f3e3\U0001f3e4\U0001f3e5-\U0001f3f0\U0001f3f4\U0001f3f8-\U0001f407\U0001f408\U0001f409-\U0001f40b\U0001f40c-\U0001f40e\U0001f40f-\U0001f410\U0001f411-\U0001f412\U0001f413\U0001f414\U0001f415\U0001f416\U0001f417-\U0001f429\U0001f42a\U0001f42b-\U0001f43e\U0001f440\U0001f442-\U0001f464\U0001f465\U0001f466-\U0001f46b\U0001f46c-\U0001f46d\U0001f46e-\U0001f4ac\U0001f4ad\U0001f4ae-\U0001f4b5\U0001f4b6-\U0001f4b7\U0001f4b8-\U0001f4eb\U0001f4ec-\U0001f4ed\U0001f4ee\U0001f4ef\U0001f4f0-\U0001f4f4\U0001f4f5\U0001f4f6-\U0001f4f7\U0001f4f8\U0001f4f9-\U0001f4fc\U0001f4ff-\U0001f502\U0001f503\U0001f504-\U0001f507\U0001f508\U0001f509\U0001f50a-\U0001f514\U0001f515\U0001f516-\U0001f52b\U0001f52c-\U0001f52d\U0001f52e-\U0001f53d\U0001f54b-\U0001f54e\U0001f550-\U0001f55b\U0001f55c-\U0001f567\U0001f57a\U0001f595-\U0001f596\U0001f5a4\U0001f5fb-\U0001f5ff\U0001f600\U0001f601-\U0001f606\U0001f607-\U0001f608\U0001f609-\U0001f60d\U0001f60e\U0001f60f\U0001f610\U0001f611\U0001f612-\U0001f614\U0001f615\U0001f616\U0001f617\U0001f618\U0001f619\U0001f61a\U0001f61b\U0001f61c-\U0001f61e\U0001f61f\U0001f620-\U0001f625\U0001f626-\U0001f627\U0001f628-\U0001f62b\U0001f62c\U0001f62d\U0001f62e-\U0001f62f\U0001f630-\U0001f633\U0001f634\U0001f635\U0001f636\U0001f637-\U0001f640\U0001f641-\U0001f644\U0001f645-\U0001f64f\U0001f680\U0001f681-\U0001f682\U0001f683-\U0001f685\U0001f686\U0001f687\U0001f688\U0001f689\U0001f68a-\U0001f68b\U0001f68c\U0001f68d\U0001f68e\U0001f68f\U0001f690\U0001f691-\U0001f693\U0001f694\U0001f695\U0001f696\U0001f697\U0001f698\U0001f699-\U0001f69a\U0001f69b-\U0001f6a1\U0001f6a2\U0001f6a3\U0001f6a4-\U0001f6a5\U0001f6a6\U0001f6a7-\U0001f6ad\U0001f6ae-\U0001f6b1\U0001f6b2\U0001f6b3-\U0001f6b5\U0001f6b6\U0001f6b7-\U0001f6b8\U0001f6b9-\U0001f6be\U0001f6bf\U0001f6c0\U0001f6c1-\U0001f6c5\U0001f6cc\U0001f6d0\U0001f6d1-\U0001f6d2\U0001f6d5\U0001f6d6-\U0001f6d7\U0001f6dc\U0001f6dd-\U0001f6df\U0001f6eb-\U0001f6ec\U0001f6f4-\U0001f6f6\U0001f6f7-\U0001f6f8\U0001f6f9\U0001f6fa\U0001f6fb-\U0001f6fc\U0001f7e0-\U0001f7eb\U0001f7f0\U0001f90c\U0001f90d-\U0001f90f\U0001f910-\U0001f918\U0001f919-\U0001f91e\U0001f91f\U0001f920-\U0001f927\U0001f928-\U0001f92f\U0001f930\U0001f931-\U0001f932\U0001f933-\U0001f93a\U0001f93c-\U0001f93e\U0001f93f\U0001f940-\U0001f945\U0001f947-\U0001f94b\U0001f94c\U0001f94d-\U0001f94f\U0001f950-\U0001f95e\U0001f95f-\U0001f96b\U0001f96c-\U0001f970\U0001f971\U0001f972\U0001f973-\U0001f976\U0001f977-\U0001f978\U0001f979\U0001f97a\U0001f97b\U0001f97c-\U0001f97f\U0001f980-\U0001f984\U0001f985-\U0001f991\U0001f992-\U0001f997\U0001f998-\U0001f9a2\U0001f9a3-\U0001f9a4\U0001f9a5-\U0001f9aa\U0001f9ab-\U0001f9ad\U0001f9ae-\U0001f9af\U0001f9b0-\U0001f9b9\U0001f9ba-\U0001f9bf\U0001f9c0\U0001f9c1-\U0001f9c2\U0001f9c3-\U0001f9ca\U0001f9cb\U0001f9cc\U0001f9cd-\U0001f9cf\U0001f9d0-\U0001f9e6\U0001f9e7-\U0001f9ff\U0001fa70-\U0001fa73\U0001fa74\U0001fa75-\U0001fa77\U0001fa78-\U0001fa7a\U0001fa7b-\U0001fa7c\U0001fa80-\U0001fa82\U0001fa83-\U0001fa86\U0001fa87-\U0001fa88\U0001fa90-\U0001fa95\U0001fa96-\U0001faa8\U0001faa9-\U0001faac\U0001faad-\U0001faaf\U0001fab0-\U0001fab6\U0001fab7-\U0001faba\U0001fabb-\U0001fabd\U0001fabf\U0001fac0-\U0001fac2\U0001fac3-\U0001fac5\U0001face-\U0001facf\U0001fad0-\U0001fad6\U0001fad7-\U0001fad9\U0001fada-\U0001fadb\U0001fae0-\U0001fae7\U0001fae8\U0001faf0-\U0001faf6\U0001faf7-\U0001faf8];
Emoji_Modifier = [\U0001f3fb-\U0001f3ff];
Emoji_Modifier_Base = [\u261d\u26f9\u270a-\u270c\u270d\U0001f385\U0001f3c2-\U0001f3c4\U0001f3c7\U0001f3ca\U0001f3cb-\U0001f3cc\U0001f442-\U0001f443\U0001f446-\U0001f450\U0001f466-\U0001f46b\U0001f46c-\U0001f46d\U0001f46e-\U0001f478\U0001f47c\U0001f481-\U0001f483\U0001f485-\U0001f487\U0001f48f\U0001f491\U0001f4aa\U0001f574-\U0001f575\U0001f57a\U0001f590\U0001f595-\U0001f596\U0001f645-\U0001f647\U0001f64b-\U0001f64f\U0001f6a3\U0001f6b4-\U0001f6b5\U0001f6b6\U0001f6c0\U0001f6cc\U0001f90c\U0001f90f\U0001f918\U0001f919-\U0001f91e\U0001f91f\U0001f926\U0001f930\U0001f931-\U0001f932\U0001f933-\U0001f939\U0001f93c-\U0001f93e\U0001f977\U0001f9b5-\U0001f9b6\U0001f9b8-\U0001f9b9\U0001f9bb\U0001f9cd-\U0001f9cf\U0001f9d1-\U0001f9dd\U0001fac3-\U0001fac5\U0001faf0-\U0001faf6\U0001faf7-\U0001faf8];
Emoji_Component = [\x23\x2a\x30-\x39\u200d\u20e3\ufe0f\U0001f1e6-\U0001f1ff\U0001f3fb-\U0001f3ff\U0001f9b0-\U0001f9b3\U000e0020-\U000e007f];
Extended_Pictographic = [\xa9\xae\u203c\u2049\u2122\u2139\u2194-\u2199\u21a9-\u21aa\u231a-\u231b\u2328\u2388\u23cf\u23e9-\u23ec\u23ed-\u23ee\u23ef\u23f0\u23f1-\u23f2\u23f3\u23f8-\u23fa\u24c2\u25aa-\u25ab\u25b6\u25c0\u25fb-\u25fe\u2600-\u2601\u2602-\u2603\u2604\u2605\u2607-\u260d\u260e\u260f-\u2610\u2611\u2612\u2614-\u2615\u2616-\u2617\u2618\u2619-\u261c\u261d\u261e-\u261f\u2620\u2621\u2622-\u2623\u2624-\u2625\u2626\u2627-\u2629\u262a\u262b-\u262d\u262e\u262f\u2630-\u2637\u2638-\u2639\u263a\u263b-\u263f\u2640\u2641\u2642\u2643-\u2647\u2648-\u2653\u2654-\u265e\u265f\u2660\u2661-\u2662\u2663\u2664\u2665-\u2666\u2667\u2668\u2669-\u267a\u267b\u267c-\u267d\u267e\u267f\u2680-\u2685\u2690-\u2691\u2692\u2693\u2694\u2695\u2696-\u2697\u2698\u2699\u269a\u269b-\u269c\u269d-\u269f\u26a0-\u26a1\u26a2-\u26a6\u26a7\u26a8-\u26a9\u26aa-\u26ab\u26ac-\u26af\u26b0-\u26b1\u26b2-\u26bc\u26bd-\u26be\u26bf-\u26c3\u26c4-\u26c5\u26c6-\u26c7\u26c8\u26c9-\u26cd\u26ce\u26cf\u26d0\u26d1\u26d2\u26d3\u26d4\u26d5-\u26e8\u26e9\u26ea\u26eb-\u26ef\u26f0-\u26f1\u26f2-\u26f3\u26f4\u26f5\u26f6\u26f7-\u26f9\u26fa\u26fb-\u26fc\u26fd\u26fe-\u2701\u2702\u2703-\u2704\u2705\u2708-\u270c\u270d\u270e\u270f\u2710-\u2711\u2712\u2714\u2716\u271d\u2721\u2728\u2733-\u2734\u2744\u2747\u274c\u274e\u2753-\u2755\u2757\u2763\u2764\u2765-\u2767\u2795-\u2797\u27a1\u27b0\u27bf\u2934-\u2935\u2b05-\u2b07\u2b1b-\u2b1c\u2b50\u2b55\u3030\u303d\u3297\u3299\U0001f000-\U0001f003\U0001f004\U0001f005-\U0001f0ce\U0001f0cf\U0001f0d0-\U0001f0ff\U0001f10d-\U0001f10f\U0001f12f\U0001f16c-\U0001f16f\U0001f170-\U0001f171\U0001f17e-\U0001f17f\U0001f18e\U0001f191-\U0001f19a\U0001f1ad-\U0001f1e5\U0001f201-\U0001f202\U0001f203-\U0001f20f\U0001f21a\U0001f22f\U0001f232-\U0001f23a\U0001f23c-\U0001f23f\U0001f249-\U0001f24f\U0001f250-\U0001f251\U0001f252-\U0001f2ff\U0001f300-\U0001f30c\U0001f30d-\U0001f30e\U0001f30f\U0001f310\U0001f311\U0001f312\U0001f313-\U0001f315\U0001f316-\U0001f318\U0001f319\U0001f31a\U0001f31b\U0001f31c\U0001f31d-\U0001f31e\U0001f31f-\U0001f320\U0001f321\U0001f322-\U0001f323\U0001f324-\U0001f32c\U0001f32d-\U0001f32f\U0001f330-\U0001f331\U0001f332-\U0001f333\U0001f334-\U0001f335\U0001f336\U0001f337-\U0001f34a\U0001f34b\U0001f34c-\U0001f34f\U0001f350\U0001f351-\U0001f37b\U0001f37c\U0001f37d\U0001f37e-\U0001f37f\U0001f380-\U0001f393\U0001f394-\U0001f395\U0001f396-\U0001f397\U0001f398\U0001f399-\U0001f39b\U0001f39c-\U0001f39d\U0001f39e-\U0001f39f\U0001f3a0-\U0001f3c4\U0001f3c5\U0001f3c6\U0001f3c7\U0001f3c8\U0001f3c9\U0001f3ca\U0001f3cb-\U0001f3ce\U0001f3cf-\U0001f3d3\U0001f3d4-\U0001f3df\U0001f3e0-\U0001f3e3\U0001f3e4\U0001f3e5-\U0001f3f0\U0001f3f1-\U0001f3f2\U0001f3f3\U0001f3f4\U0001f3f5\U0001f3f6\U0001f3f7\U0001f3f8-\U0001f3fa\U0001f400-\U0001f407\U0001f408\U0001f409-\U0001f40b\U0001f40c-\U0001f40e\U0001f40f-\U0001f410\U0001f411-\U0001f412\U0001f413\U0001f414\U0001f415\U0001f416\U0001f417-\U0001f429\U0001f42a\U0001f42b-\U0001f43e\U0001f43f\U0001f440\U0001f441\U0001f442-\U0001f464\U0001f465\U0001f466-\U0001f46b\U0001f46c-\U0001f46d\U0001f46e-\U0001f4ac\U0001f4ad\U0001f4ae-\U0001f4b5\U0001f4b6-\U0001f4b7\U0001f4b8-\U0001f4eb\U0001f4ec-\U0001f4ed\U0001f4ee\U0001f4ef\U0001f4f0-\U0001f4f4\U0001f4f5\U0001f4f6-\U0001f4f7\U0001f4f8\U0001f4f9-\U0001f4fc\U0001f4fd\U0001f4fe\U0001f4ff-\U0001f502\U0001f503\U0001f504-\U0001f507\U0001f508\U0001f509\U0001f50a-\U0001f514\U0001f515\U0001f516-\U0001f52b\U0001f52c-\U0001f52d\U0001f52e-\U0001f53d\U0001f546-\U0001f548\U0001f549-\U0001f54a\U0001f54b-\U0001f54e\U0001f54f\U0001f550-\U0001f55b\U0001f55c-\U0001f567\U0001f568-\U0001f56e\U0001f56f-\U0001f570\U0001f571-\U0001f572\U0001f573-\U0001f579\U0001f57a\U0001f57b-\U0001f586\U0001f587\U0001f588-\U0001f589\U0001f58a-\U0001f58d\U0001f58e-\U0001f58f\U0001f590\U0001f591-\U0001f594\U0001f595-\U0001f596\U0001f597-\U0001f5a3\U0001f5a4\U0001f5a5\U0001f5a6-\U0001f5a7\U0001f5a8\U0001f5a9-\U0001f5b0\U0001f5b1-\U0001f5b2\U0001f5b3-\U0001f5bb\U0001f5bc\U0001f5bd-\U0001f5c1\U0001f5c2-\U0001f5c4\U0001f5c5-\U0001f5d0\U0001f5d1-\U0001f5d3\U0001f5d4-\U0001f5db\U0001f5dc-\U0001f5de\U0001f5df-\U0001f5e0\U0001f5e1\U0001f5e2\U0001f5e3\U0001f5e4-\U0001f5e7\U0001f5e8\U0001f5e9-\U0001f5ee\U0001f5ef\U0001f5f0-\U0001f5f2\U0001f5f3\U0001f5f4-\U0001f5f9\U0001f5fa\U0001f5fb-\U0001f5ff\U0001f600\U0001f601-\U0001f606\U0001f607-\U0001f608\U0001f609-\U0001f60d\U0001f60e\U0001f60f\U0001f610\U0001f611\U0001f612-\U0001f614\U0001f615\U0001f616\U0001f617\U0001f618\U0001f619\U0001f61a\U0001f61b\U0001f61c-\U0001f61e\U0001f61f\U0001f620-\U0001f625\U0001f626-\U0001f627\U0001f628-\U0001f62b\U0001f62c\U0001f62d\U0001f62e-\U0001f62f\U0001f630-\U0001f633\U0001f634\U0001f635\U0001f636\U0001f637-\U0001f640\U0001f641-\U0001f644\U0001f645-\U0001f64f\U0001f680\U0001f681-\U0001f682\U0001f683-\U0001f685\U0001f686\U0001f687\U0001f688\U0001f689\U0001f68a-\U0001f68b\U0001f68c\U0001f68d\U0001f68e\U0001f68f\U0001f690\U0001f691-\U0001f693\U0001f694\U0001f695\U0001f696\U0001f697\U0001f698\U0001f699-\U0001f69a\U0001f69b-\U0001f6a1\U0001f6a2\U0001f6a3\U0001f6a4-\U0001f6a5\U0001f6a6\U0001f6a7-\U0001f6ad\U0001f6ae-\U0001f6b1\U0001f6b2\U0001f6b3-\U0001f6b5\U0001f6b6\U0001f6b7-\U0001f6b8\U0001f6b9-\U0001f6be\U0001f6bf\U0001f6c0\U0001f6c1-\U0001f6c5\U0001f6c6-\U0001f6ca\U0001f6cb\U0001f6cc\U0001f6cd-\U0001f6cf\U0001f6d0\U0001f6d1-\U0001f6d2\U0001f6d3-\U0001f6d4\U0001f6d5\U0001f6d6-\U0001f6d7\U0001f6d8-\U0001f6db\U0001f6dc\U0001f6dd-\U0001f6df\U0001f6e0-\U0001f6e5\U0001f6e6-\U0001f6e8\U0001f6e9\U0001f6ea\U0001f6eb-\U0001f6ec\U0001f6ed-\U0001f6ef\U0001f6f0\U0001f6f1-\U0001f6f2\U0001f6f3\U0001f6f4-\U0001f6f6\U0001f6f7-\U0001f6f8\U0001f6f9\U0001f6fa\U0001f6fb-\U0001f6fc\U0001f6fd-\U0001f6ff\U0001f774-\U0001f77f\U0001f7d5-\U0001f7df\U0001f7e0-\U0001f7eb\U0001f7ec-\U0001f7ef\U0001f7f0\U0001f7f1-\U0001f7ff\U0001f80c-\U0001f80f\U0001f848-\U0001f84f\U0001f85a-\U0001f85f\U0001f888-\U0001f88f\U0001f8ae-\U0001f8ff\U0001f90c\U0001f90d-\U0001f90f\U0001f910-\U0001f918\U0001f919-\U0001f91e\U0001f91f\U0001f920-\U0001f927\U0001f928-\U0001f92f\U0001f930\U0001f931-\U0001f932\U0001f933-\U0001f93a\U0001f93c-\U0001f93e\U0001f93f\U0001f940-\U0001f945\U0001f947-\U0001f94b\U0001f94c\U0001f94d-\U0001f94f\U0001f950-\U0001f95e\U0001f95f-\U0001f96b\U0001f96c-\U0001f970\U0001f971\U0001f972\U0001f973-\U0001f976\U0001f977-\U0001f978\U0001f979\U0001f97a\U0001f97b\U0001f97c-\U0001f97f\U0001f980-\U0001f984\U0001f985-\U0001f991\U0001f992-\U0001f997\U0001f998-\U0001f9a2\U0001f9a3-\U0001f9a4\U0001f9a5-\U0001f9aa\U0001f9ab-\U0001f9ad\U0001f9ae-\U0001f9af\U0001f9b0-\U0001f9b9\U0001f9ba-\U0001f9bf\U0001f9c0\U0001f9c1-\U0001f9c2\U0001f9c3-\U0001f9ca\U0001f9cb\U0001f9cc\U0001f9cd-\U0001f9cf\U0001f9d0-\U0001f9e6\U0001f9e7-\U0001f9ff\U0001fa00-\U0001fa6f\U0001fa70-\U0001fa73\U0001fa74\U0001fa75-\U0001fa77\U0001fa78-\U0001fa7a\U0001fa7b-\U0001fa7c\U0001fa7d-\U0001fa7f\U0001fa80-\U0001fa82\U0001fa83-\U0001fa86\U0001fa87-\U0001fa88\U0001fa89-\U0001fa8f\U0001fa90-\U0001fa95\U0001fa96-\U0001faa8\U0001faa9-\U0001faac\U0001faad-\U0001faaf\U0001fab0-\U0001fab6\U0001fab7-\U0001faba\U0001fabb-\U0001fabd\U0001fabe\U0001fabf\U0001fac0-\U0001fac2\U0001fac3-\U0001fac5\U0001fac6-\U0001facd\U0001face-\U0001facf\U0001fad0-\U0001fad6\U0001fad7-\U0001fad9\U0001fada-\U0001fadb\U0001fadc-\U0001fadf\U0001fae0-\U0001fae7\U0001fae8\U0001fae9-\U0001faef\U0001faf0-\U0001faf6\U0001faf7-\U0001faf8\U0001faf9-\U0001faff\U0001fc00-\U0001fffd];
*/
