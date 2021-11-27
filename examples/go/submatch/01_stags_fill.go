// Code generated by re2c, DO NOT EDIT.
//line "go/submatch/01_stags_fill.re":1
//go:generate re2go $INPUT -o $OUTPUT --tags
package main

import (
	"fmt"
	"os"
	"reflect"
	"testing"
)

const SIZE int = 4096

type Input struct {
	file   *os.File
	data   []byte
	cursor int
	marker int
	token  int
	limit  int
	// Tag variables must be part of the lexer state passed to YYFILL.
	// They don't correspond to tags and should be autogenerated by re2c.
	
//line "go/submatch/01_stags_fill.go":26
	yyt1 int
	yyt2 int
	yyt3 int
	yyt4 int
//line "go/submatch/01_stags_fill.re":22

	eof    bool
}

func fill(in *Input) int {
	// If nothing can be read, fail.
	if in.eof {
		return 1
	}

	// Check if at least some space can be freed.
	if in.token == 0 {
		// In real life can reallocate a larger buffer.
		panic("fill error: lexeme too long")
	}

	// Discard everything up to the start of the current lexeme,
	// shift buffer contents and adjust offsets.
	copy(in.data[0:], in.data[in.token:in.limit])
	in.cursor -= in.token
	in.marker -= in.token
	in.limit -= in.token
	// Tag variables need to be shifted like other input positions. The
	// check for -1 is only needed if some tags are nested inside of
	// alternative or repetition, so that they can have -1 value.
	
//line "go/submatch/01_stags_fill.go":58
	if in.yyt1 != -1 { in.yyt1 -= in.token }
	if in.yyt2 != -1 { in.yyt2 -= in.token }
	if in.yyt3 != -1 { in.yyt3 -= in.token }
	if in.yyt4 != -1 { in.yyt4 -= in.token }
//line "go/submatch/01_stags_fill.re":49

	in.token = 0

	// Read new data (as much as possible to fill the buffer).
	n, _ := in.file.Read(in.data[in.limit:SIZE])
	in.limit += n
	in.data[in.limit] = 0

	// If read less than expected, this is the end of input.
	in.eof = in.limit < SIZE

	// If nothing has been read, fail.
	if n == 0 {
		return 1
	}

	return 0
}

func lex(in *Input) []int {
	// User-defined local variables that store final tag values. They are
	// different from tag variables autogenerated with `stags:re2c`, as
	// they are set at the end of match and used only in semantic actions.
	var o1, o2, o3, o4 int
	var ips []int

	num := func(pos int, end int) int {
		n := 0
		for ; pos < end; pos++ {
			n = n*10 + int(in.data[pos]-'0')
		}
		return n
	}

loop:
	in.token = in.cursor
	
//line "go/submatch/01_stags_fill.go":101
{
	var yych byte
yyFillLabel0:
	yych = in.data[in.cursor]
	switch (yych) {
	case '0':
		in.yyt1 = in.cursor
		goto yy4
	case '1':
		in.yyt1 = in.cursor
		goto yy5
	case '2':
		in.yyt1 = in.cursor
		goto yy6
	case '3':
		fallthrough
	case '4':
		fallthrough
	case '5':
		fallthrough
	case '6':
		fallthrough
	case '7':
		fallthrough
	case '8':
		fallthrough
	case '9':
		in.yyt1 = in.cursor
		goto yy7
	default:
		if (in.limit <= in.cursor) {
			if (fill(in) == 0) {
				goto yyFillLabel0
			}
			goto yy32
		}
		goto yy2
	}
yy2:
	in.cursor += 1
yy3:
//line "go/submatch/01_stags_fill.re":113
	{ return nil }
//line "go/submatch/01_stags_fill.go":145
yy4:
	in.cursor += 1
	in.marker = in.cursor
yyFillLabel1:
	yych = in.data[in.cursor]
	switch (yych) {
	case '.':
		goto yy8
	default:
		if (in.limit <= in.cursor) {
			if (fill(in) == 0) {
				goto yyFillLabel1
			}
		}
		goto yy3
	}
yy5:
	in.cursor += 1
	in.marker = in.cursor
yyFillLabel2:
	yych = in.data[in.cursor]
	switch (yych) {
	case '.':
		goto yy8
	case '0':
		fallthrough
	case '1':
		fallthrough
	case '2':
		fallthrough
	case '3':
		fallthrough
	case '4':
		fallthrough
	case '5':
		fallthrough
	case '6':
		fallthrough
	case '7':
		fallthrough
	case '8':
		fallthrough
	case '9':
		goto yy10
	default:
		if (in.limit <= in.cursor) {
			if (fill(in) == 0) {
				goto yyFillLabel2
			}
		}
		goto yy3
	}
yy6:
	in.cursor += 1
	in.marker = in.cursor
yyFillLabel3:
	yych = in.data[in.cursor]
	switch (yych) {
	case '.':
		goto yy8
	case '0':
		fallthrough
	case '1':
		fallthrough
	case '2':
		fallthrough
	case '3':
		fallthrough
	case '4':
		goto yy10
	case '5':
		goto yy11
	case '6':
		fallthrough
	case '7':
		fallthrough
	case '8':
		fallthrough
	case '9':
		goto yy12
	default:
		if (in.limit <= in.cursor) {
			if (fill(in) == 0) {
				goto yyFillLabel3
			}
		}
		goto yy3
	}
yy7:
	in.cursor += 1
	in.marker = in.cursor
yyFillLabel4:
	yych = in.data[in.cursor]
	switch (yych) {
	case '.':
		goto yy8
	case '0':
		fallthrough
	case '1':
		fallthrough
	case '2':
		fallthrough
	case '3':
		fallthrough
	case '4':
		fallthrough
	case '5':
		fallthrough
	case '6':
		fallthrough
	case '7':
		fallthrough
	case '8':
		fallthrough
	case '9':
		goto yy12
	default:
		if (in.limit <= in.cursor) {
			if (fill(in) == 0) {
				goto yyFillLabel4
			}
		}
		goto yy3
	}
yy8:
	in.cursor += 1
yyFillLabel5:
	yych = in.data[in.cursor]
	switch (yych) {
	case '0':
		in.yyt2 = in.cursor
		goto yy13
	case '1':
		in.yyt2 = in.cursor
		goto yy14
	case '2':
		in.yyt2 = in.cursor
		goto yy15
	case '3':
		fallthrough
	case '4':
		fallthrough
	case '5':
		fallthrough
	case '6':
		fallthrough
	case '7':
		fallthrough
	case '8':
		fallthrough
	case '9':
		in.yyt2 = in.cursor
		goto yy16
	default:
		if (in.limit <= in.cursor) {
			if (fill(in) == 0) {
				goto yyFillLabel5
			}
		}
		goto yy9
	}
yy9:
	in.cursor = in.marker
	goto yy3
yy10:
	in.cursor += 1
yyFillLabel6:
	yych = in.data[in.cursor]
	switch (yych) {
	case '.':
		goto yy8
	case '0':
		fallthrough
	case '1':
		fallthrough
	case '2':
		fallthrough
	case '3':
		fallthrough
	case '4':
		fallthrough
	case '5':
		fallthrough
	case '6':
		fallthrough
	case '7':
		fallthrough
	case '8':
		fallthrough
	case '9':
		goto yy12
	default:
		if (in.limit <= in.cursor) {
			if (fill(in) == 0) {
				goto yyFillLabel6
			}
		}
		goto yy9
	}
yy11:
	in.cursor += 1
yyFillLabel7:
	yych = in.data[in.cursor]
	switch (yych) {
	case '.':
		goto yy8
	case '0':
		fallthrough
	case '1':
		fallthrough
	case '2':
		fallthrough
	case '3':
		fallthrough
	case '4':
		fallthrough
	case '5':
		goto yy12
	default:
		if (in.limit <= in.cursor) {
			if (fill(in) == 0) {
				goto yyFillLabel7
			}
		}
		goto yy9
	}
yy12:
	in.cursor += 1
yyFillLabel8:
	yych = in.data[in.cursor]
	switch (yych) {
	case '.':
		goto yy8
	default:
		if (in.limit <= in.cursor) {
			if (fill(in) == 0) {
				goto yyFillLabel8
			}
		}
		goto yy9
	}
yy13:
	in.cursor += 1
yyFillLabel9:
	yych = in.data[in.cursor]
	switch (yych) {
	case '.':
		goto yy17
	default:
		if (in.limit <= in.cursor) {
			if (fill(in) == 0) {
				goto yyFillLabel9
			}
		}
		goto yy9
	}
yy14:
	in.cursor += 1
yyFillLabel10:
	yych = in.data[in.cursor]
	switch (yych) {
	case '.':
		goto yy17
	case '0':
		fallthrough
	case '1':
		fallthrough
	case '2':
		fallthrough
	case '3':
		fallthrough
	case '4':
		fallthrough
	case '5':
		fallthrough
	case '6':
		fallthrough
	case '7':
		fallthrough
	case '8':
		fallthrough
	case '9':
		goto yy16
	default:
		if (in.limit <= in.cursor) {
			if (fill(in) == 0) {
				goto yyFillLabel10
			}
		}
		goto yy9
	}
yy15:
	in.cursor += 1
yyFillLabel11:
	yych = in.data[in.cursor]
	switch (yych) {
	case '.':
		goto yy17
	case '0':
		fallthrough
	case '1':
		fallthrough
	case '2':
		fallthrough
	case '3':
		fallthrough
	case '4':
		goto yy16
	case '5':
		goto yy18
	case '6':
		fallthrough
	case '7':
		fallthrough
	case '8':
		fallthrough
	case '9':
		goto yy13
	default:
		if (in.limit <= in.cursor) {
			if (fill(in) == 0) {
				goto yyFillLabel11
			}
		}
		goto yy9
	}
yy16:
	in.cursor += 1
yyFillLabel12:
	yych = in.data[in.cursor]
	switch (yych) {
	case '.':
		goto yy17
	case '0':
		fallthrough
	case '1':
		fallthrough
	case '2':
		fallthrough
	case '3':
		fallthrough
	case '4':
		fallthrough
	case '5':
		fallthrough
	case '6':
		fallthrough
	case '7':
		fallthrough
	case '8':
		fallthrough
	case '9':
		goto yy13
	default:
		if (in.limit <= in.cursor) {
			if (fill(in) == 0) {
				goto yyFillLabel12
			}
		}
		goto yy9
	}
yy17:
	in.cursor += 1
yyFillLabel13:
	yych = in.data[in.cursor]
	switch (yych) {
	case '0':
		in.yyt3 = in.cursor
		goto yy19
	case '1':
		in.yyt3 = in.cursor
		goto yy20
	case '2':
		in.yyt3 = in.cursor
		goto yy21
	case '3':
		fallthrough
	case '4':
		fallthrough
	case '5':
		fallthrough
	case '6':
		fallthrough
	case '7':
		fallthrough
	case '8':
		fallthrough
	case '9':
		in.yyt3 = in.cursor
		goto yy22
	default:
		if (in.limit <= in.cursor) {
			if (fill(in) == 0) {
				goto yyFillLabel13
			}
		}
		goto yy9
	}
yy18:
	in.cursor += 1
yyFillLabel14:
	yych = in.data[in.cursor]
	switch (yych) {
	case '.':
		goto yy17
	case '0':
		fallthrough
	case '1':
		fallthrough
	case '2':
		fallthrough
	case '3':
		fallthrough
	case '4':
		fallthrough
	case '5':
		goto yy13
	default:
		if (in.limit <= in.cursor) {
			if (fill(in) == 0) {
				goto yyFillLabel14
			}
		}
		goto yy9
	}
yy19:
	in.cursor += 1
yyFillLabel15:
	yych = in.data[in.cursor]
	switch (yych) {
	case '.':
		goto yy23
	default:
		if (in.limit <= in.cursor) {
			if (fill(in) == 0) {
				goto yyFillLabel15
			}
		}
		goto yy9
	}
yy20:
	in.cursor += 1
yyFillLabel16:
	yych = in.data[in.cursor]
	switch (yych) {
	case '.':
		goto yy23
	case '0':
		fallthrough
	case '1':
		fallthrough
	case '2':
		fallthrough
	case '3':
		fallthrough
	case '4':
		fallthrough
	case '5':
		fallthrough
	case '6':
		fallthrough
	case '7':
		fallthrough
	case '8':
		fallthrough
	case '9':
		goto yy22
	default:
		if (in.limit <= in.cursor) {
			if (fill(in) == 0) {
				goto yyFillLabel16
			}
		}
		goto yy9
	}
yy21:
	in.cursor += 1
yyFillLabel17:
	yych = in.data[in.cursor]
	switch (yych) {
	case '.':
		goto yy23
	case '0':
		fallthrough
	case '1':
		fallthrough
	case '2':
		fallthrough
	case '3':
		fallthrough
	case '4':
		goto yy22
	case '5':
		goto yy24
	case '6':
		fallthrough
	case '7':
		fallthrough
	case '8':
		fallthrough
	case '9':
		goto yy19
	default:
		if (in.limit <= in.cursor) {
			if (fill(in) == 0) {
				goto yyFillLabel17
			}
		}
		goto yy9
	}
yy22:
	in.cursor += 1
yyFillLabel18:
	yych = in.data[in.cursor]
	switch (yych) {
	case '.':
		goto yy23
	case '0':
		fallthrough
	case '1':
		fallthrough
	case '2':
		fallthrough
	case '3':
		fallthrough
	case '4':
		fallthrough
	case '5':
		fallthrough
	case '6':
		fallthrough
	case '7':
		fallthrough
	case '8':
		fallthrough
	case '9':
		goto yy19
	default:
		if (in.limit <= in.cursor) {
			if (fill(in) == 0) {
				goto yyFillLabel18
			}
		}
		goto yy9
	}
yy23:
	in.cursor += 1
yyFillLabel19:
	yych = in.data[in.cursor]
	switch (yych) {
	case '0':
		in.yyt4 = in.cursor
		goto yy25
	case '1':
		in.yyt4 = in.cursor
		goto yy26
	case '2':
		in.yyt4 = in.cursor
		goto yy27
	case '3':
		fallthrough
	case '4':
		fallthrough
	case '5':
		fallthrough
	case '6':
		fallthrough
	case '7':
		fallthrough
	case '8':
		fallthrough
	case '9':
		in.yyt4 = in.cursor
		goto yy28
	default:
		if (in.limit <= in.cursor) {
			if (fill(in) == 0) {
				goto yyFillLabel19
			}
		}
		goto yy9
	}
yy24:
	in.cursor += 1
yyFillLabel20:
	yych = in.data[in.cursor]
	switch (yych) {
	case '.':
		goto yy23
	case '0':
		fallthrough
	case '1':
		fallthrough
	case '2':
		fallthrough
	case '3':
		fallthrough
	case '4':
		fallthrough
	case '5':
		goto yy19
	default:
		if (in.limit <= in.cursor) {
			if (fill(in) == 0) {
				goto yyFillLabel20
			}
		}
		goto yy9
	}
yy25:
	in.cursor += 1
yyFillLabel21:
	yych = in.data[in.cursor]
	switch (yych) {
	case '\n':
		goto yy29
	default:
		if (in.limit <= in.cursor) {
			if (fill(in) == 0) {
				goto yyFillLabel21
			}
		}
		goto yy9
	}
yy26:
	in.cursor += 1
yyFillLabel22:
	yych = in.data[in.cursor]
	switch (yych) {
	case '\n':
		goto yy29
	case '0':
		fallthrough
	case '1':
		fallthrough
	case '2':
		fallthrough
	case '3':
		fallthrough
	case '4':
		fallthrough
	case '5':
		fallthrough
	case '6':
		fallthrough
	case '7':
		fallthrough
	case '8':
		fallthrough
	case '9':
		goto yy28
	default:
		if (in.limit <= in.cursor) {
			if (fill(in) == 0) {
				goto yyFillLabel22
			}
		}
		goto yy9
	}
yy27:
	in.cursor += 1
yyFillLabel23:
	yych = in.data[in.cursor]
	switch (yych) {
	case '\n':
		goto yy29
	case '0':
		fallthrough
	case '1':
		fallthrough
	case '2':
		fallthrough
	case '3':
		fallthrough
	case '4':
		goto yy28
	case '5':
		goto yy31
	case '6':
		fallthrough
	case '7':
		fallthrough
	case '8':
		fallthrough
	case '9':
		goto yy25
	default:
		if (in.limit <= in.cursor) {
			if (fill(in) == 0) {
				goto yyFillLabel23
			}
		}
		goto yy9
	}
yy28:
	in.cursor += 1
yyFillLabel24:
	yych = in.data[in.cursor]
	switch (yych) {
	case '\n':
		goto yy29
	case '0':
		fallthrough
	case '1':
		fallthrough
	case '2':
		fallthrough
	case '3':
		fallthrough
	case '4':
		fallthrough
	case '5':
		fallthrough
	case '6':
		fallthrough
	case '7':
		fallthrough
	case '8':
		fallthrough
	case '9':
		goto yy25
	default:
		if (in.limit <= in.cursor) {
			if (fill(in) == 0) {
				goto yyFillLabel24
			}
		}
		goto yy9
	}
yy29:
	in.cursor += 1
	o1 = in.yyt1
	o2 = in.yyt2
	o3 = in.yyt3
	o4 = in.yyt4
//line "go/submatch/01_stags_fill.re":105
	{
		ips = append(ips, num(o4, in.cursor-1)+
			(num(o3, o4-1) << 8)+
			(num(o2, o3-1) << 16)+
			(num(o1, o2-1) << 24))
		goto loop
	}
//line "go/submatch/01_stags_fill.go":889
yy31:
	in.cursor += 1
yyFillLabel25:
	yych = in.data[in.cursor]
	switch (yych) {
	case '\n':
		goto yy29
	case '0':
		fallthrough
	case '1':
		fallthrough
	case '2':
		fallthrough
	case '3':
		fallthrough
	case '4':
		fallthrough
	case '5':
		goto yy25
	default:
		if (in.limit <= in.cursor) {
			if (fill(in) == 0) {
				goto yyFillLabel25
			}
		}
		goto yy9
	}
yy32:
//line "go/submatch/01_stags_fill.re":112
	{ return ips }
//line "go/submatch/01_stags_fill.go":920
}
//line "go/submatch/01_stags_fill.re":114

}

func TestLex(t *testing.T) {
	tmpfile := "input.txt"
	var want, have []int

	// Write a few IPv4 addresses to the input file and save them to compare
	// against parse results.
	f, _ := os.Create(tmpfile)
	for i := 0; i < 256; i++ {
		fmt.Fprintf(f, "%d.%d.%d.%d\n", i, i, i, i)
		want = append(want, i + (i<<8) + (i<<16) + (i<<24));
	}
	f.Seek(0, 0)

	defer func() {
		if r := recover(); r != nil {
			have = nil
		}
		f.Close()
		os.Remove(tmpfile)
	}()

	in := &Input{
		file:   f,
		data:   make([]byte, SIZE+1),
		cursor: SIZE,
		marker: SIZE,
		token:  SIZE,
		limit:  SIZE,
		eof:    false,
	}

	have = lex(in)

	if !reflect.DeepEqual(have, want) {
		t.Errorf("have %d, want %d", have, want)
	}
}