// re2c $INPUT -o $OUTPUT -ir
/*!re2c "a" {} * {} */

/*!rules:re2c "b" {} * {} */

/*!use:re2c*/
/*!use:re2c re2c:flags:u = 1; *//*!re2c re2c:flags:u = 0; */

/*!rules:re2c "c" {} * {} */

/*!use:re2c*/
/*!use:re2c re2c:flags:u = 1; *//*!re2c re2c:flags:u = 0; */

/*!re2c "d" {} * {} */

/*!use:re2c*/
/*!use:re2c re2c:flags:u = 1; *//*!re2c re2c:flags:u = 0; */

/*!rules:re2c "e" {} * {} */

/*!use:re2c*/
/*!use:re2c re2c:flags:u = 1; *//*!re2c re2c:flags:u = 0; */

/*!re2c "f" {} * {} */

/*!use:re2c*/
/*!use:re2c re2c:flags:u = 1; *//*!re2c re2c:flags:u = 0; */

/*!rules:re2c "g" {} * {} */

/*!rules:re2c "h" {} * {} */

/*!use:re2c*/
/*!use:re2c re2c:flags:u = 1; *//*!re2c re2c:flags:u = 0; */
