// re2rust $INPUT -o $OUTPUT -c

/*!conditions:re2c*/

const ERROR: u64 = std::u32::MAX as u64 + 1; // overflow

// Add digit with the given base, checking for overflow.
fn add(num: &mut u64, str: &[u8], cur: usize, offs: u8, base: u64) {
    let digit = unsafe { str.get_unchecked(cur - 1) } - offs;
    *num = std::cmp::min(*num * base + digit as u64, ERROR);
}

fn parse_u32(str: &[u8]) -> Option<u32> {
    let (mut cur, mut mar) = (0, 0);
    let mut cond = YYC_INIT;
    let mut num = 0u64; // Store number in u64 to simplify overflow checks.

    'lex: loop {/*!re2c
        re2c:define:YYCTYPE   = u8;
        re2c:define:YYPEEK    = "*str.get_unchecked(cur)";
        re2c:define:YYSKIP    = "cur += 1;";
        re2c:define:YYBACKUP  = "mar = cur;";
        re2c:define:YYRESTORE = "cur = mar;";
        re2c:define:YYSHIFT   = "cur = (cur as isize + @@) as usize;";
        re2c:define:YYGETCONDITION = "cond";
        re2c:define:YYSETCONDITION = "cond = @@;";
        re2c:yyfill:enable = 0;

        <INIT> '0b' / [01]        :=> BIN
        <INIT> "0"                :=> OCT
        <INIT> "" / [1-9]         :=> DEC
        <INIT> '0x' / [0-9a-fA-F] :=> HEX
        <INIT> * { return None; }

        <BIN> [01]  { add(&mut num, str, cur, 48, 2);  continue 'lex; }
        <OCT> [0-7] { add(&mut num, str, cur, 48, 8);  continue 'lex; }
        <DEC> [0-9] { add(&mut num, str, cur, 48, 10); continue 'lex; }
        <HEX> [0-9] { add(&mut num, str, cur, 48, 16); continue 'lex; }
        <HEX> [a-f] { add(&mut num, str, cur, 87, 16); continue 'lex; }
        <HEX> [A-F] { add(&mut num, str, cur, 55, 16); continue 'lex; }

        <BIN, OCT, DEC, HEX> * {
            return if num < ERROR { Some(num as u32) } else { None };
        }
    */}
}

fn main() {
    assert_eq!(parse_u32(b"\0"), None);
    assert_eq!(parse_u32(b"1234567890\0"), Some(1234567890));
    assert_eq!(parse_u32(b"0b1101\0"), Some(13));
    assert_eq!(parse_u32(b"0x7Fe\0"), Some(2046));
    assert_eq!(parse_u32(b"0644\0"), Some(420));
    assert_eq!(parse_u32(b"9999999999\0"), None);
}
