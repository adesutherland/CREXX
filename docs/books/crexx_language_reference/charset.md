# Character Set

## Characters, text, and binary data

Character handling in cREXX involves two separate concerns:

- the characters and encoding accepted in a cREXX source program;
- the representation of text and byte data manipulated by the compiled
  program.

These concerns are related, but they are not the same. The encoding of a
source file determines how the compiler reads the program. It does not, by
itself, determine how the program interprets data read from a file, received
over a network connection, or supplied by an external library.

### Source-program encoding

In normal builds, cREXX source files are UTF-8 text. This permits comments,
string literals, and other textual source content to contain Unicode
characters.

The fixed elements of the language use the portable ASCII subset. These
include language keywords, operators, punctuation, compiler directives, and
symbols generated for RXAS. Keeping the language syntax within ASCII makes
source programs easier to exchange between development environments and
avoids dependence on locale-specific character variants.

The use of UTF-8 for source files does not imply that every Unicode character
is meaningful in every syntactic position. The lexical rules of the language
still determine which characters may occur in identifiers, numeric literals,
operators, and other language elements.

### Text values

String values are intended for textual data and may contain Unicode text.
Unicode separates characters from their encoded byte representation: the same
text may be represented differently when written using UTF-8, UTF-16, EBCDIC,
or another encoding.

Encoding and decoding therefore arise at the boundaries of a program. Input
bytes must be decoded using the encoding associated with their source, and
text must be encoded appropriately when it is written to a file, terminal,
network connection, database, or external API. The encoding of the cREXX
source file does not automatically determine the encoding of such external
data.

Operations on text should consequently be understood in terms of the cREXX
string model rather than assumed to be operations on an arbitrary sequence of
bytes. This distinction is particularly important for non-ASCII characters,
which may occupy more than one byte in UTF-8.

### Binary values

A `.binary` value represents byte-oriented data that must not be interpreted
as text. It is appropriate for data such as executable code, compressed
content, images, cryptographic material, protocol packets, and records whose
layout is defined in terms of bytes.

Binary data has no inherent character encoding. A byte sequence becomes text
only when it is decoded using a specified encoding. Conversely, converting a
string to binary data requires an encoding that defines how its characters
are to be represented as bytes.

Keeping text and binary data distinct prevents accidental character
conversion. It also makes the programmer's intention explicit: strings hold
text, whereas `.binary` values preserve byte values.

### Platform considerations

On older[^hist] or specialist platforms, source conversion may be included in the
build, transfer, or packaging workflow. For example, a development system may
store source as UTF-8 while a platform tool expects another native encoding.
Such conversion concerns the representation of the source program and should
not be confused with conversions performed by the running program on its
input and output data.

A conversion process must preserve all characters used by the source. This is
straightforward for the ASCII-based language syntax, but comments and string
literals may contain Unicode characters that are not representable in a
legacy character set. Conversion should therefore be controlled explicitly
and should not rely on an implicit system locale.

Unless stated otherwise, cREXX release documentation, source distributions,
and examples assume UTF-8 source files.


[^hist]: This can be EBCDIC for builds for historic mainframe operating systems like MVS and VM/CMS. Modern mainframe operating systems can handle UTF-8.
