<?php
$text = "  Hello World  ";
echo "strlen(text) expected=15 result=" . strlen($text);
echo "strtoupper(text) expected=  HELLO WORLD   result=" . strtoupper($text);
echo "strtolower(text) expected=  hello world   result=" . strtolower($text);
echo "trim(text) expected=Hello World result=" . trim($text);
echo "ltrim(text) expected=Hello World   result=" . ltrim($text);
echo "rtrim(text) expected=  Hello World result=" . rtrim($text);
echo "substr(text, 2, 5) expected=Hello result=" . substr($text, 2, 5);
echo "strrev(abc) expected=cba result=" . strrev("abc");
echo "strpos(Hello World, World) expected=6 result=" . strpos("Hello World", "World");
echo "str_replace(World, CREXX, Hello World) expected=Hello CREXX result=" . str_replace("World", "CREXX", "Hello World");
?>
