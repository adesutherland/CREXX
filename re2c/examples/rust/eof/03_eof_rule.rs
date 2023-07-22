/* Generated by re2c */
// re2rust $INPUT -o $OUTPUT

// Expect a null-terminated string.
fn lex(s: &[u8]) -> isize {
    let (mut cur, mut mar) = (0, 0);
    let lim = s.len() - 1; // null-terminator not included
    let mut count = 0;

    'lex: loop {
{
	#[allow(unused_assignments)]
	let mut yych : u8 = 0;
	let mut yystate : usize = 0;
	'yyl: loop {
		match yystate {
			0 => {
				yych = unsafe {*s.get_unchecked(cur)};
				match yych {
					0x20 => {
						cur += 1;
						yystate = 3;
						continue 'yyl;
					}
					0x27 => {
						cur += 1;
						yystate = 5;
						continue 'yyl;
					}
					_ => {
						if cur >= lim {
							yystate = 10;
							continue 'yyl;
						}
						cur += 1;
						yystate = 1;
						continue 'yyl;
					}
				}
			}
			1 => {
				yystate = 2;
				continue 'yyl;
			}
			2 => { return -1; }
			3 => {
				yych = unsafe {*s.get_unchecked(cur)};
				match yych {
					0x20 => {
						cur += 1;
						yystate = 3;
						continue 'yyl;
					}
					_ => {
						yystate = 4;
						continue 'yyl;
					}
				}
			}
			4 => { continue 'lex; }
			5 => {
				mar = cur;
				yych = unsafe {*s.get_unchecked(cur)};
				if yych >= 0x01 {
					yystate = 7;
					continue 'yyl;
				}
				if cur >= lim {
					yystate = 2;
					continue 'yyl;
				}
				cur += 1;
				yystate = 6;
				continue 'yyl;
			}
			6 => {
				yych = unsafe {*s.get_unchecked(cur)};
				yystate = 7;
				continue 'yyl;
			}
			7 => {
				match yych {
					0x27 => {
						cur += 1;
						yystate = 8;
						continue 'yyl;
					}
					0x5C => {
						cur += 1;
						yystate = 9;
						continue 'yyl;
					}
					_ => {
						if cur >= lim {
							yystate = 11;
							continue 'yyl;
						}
						cur += 1;
						yystate = 6;
						continue 'yyl;
					}
				}
			}
			8 => { count += 1; continue 'lex; }
			9 => {
				yych = unsafe {*s.get_unchecked(cur)};
				if yych <= 0x00 {
					if cur >= lim {
						yystate = 11;
						continue 'yyl;
					}
					cur += 1;
					yystate = 6;
					continue 'yyl;
				}
				cur += 1;
				yystate = 6;
				continue 'yyl;
			}
			10 => { return count; }
			11 => {
				cur = mar;
				yystate = 2;
				continue 'yyl;
			}
			_ => {
				panic!("internal lexer error")
			}
		}
	}
}
}
}

fn main() {
    assert_eq!(lex(b"\0"), 0);
    assert_eq!(lex(b"'qu\0tes' 'are' 'fine: \\'' \0"), 3);
    assert_eq!(lex(b"'unterminated\\'\0"), -1);
}
