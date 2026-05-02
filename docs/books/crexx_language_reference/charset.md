# Character Set

There are two character-set questions in \crexx{}:

- the characters accepted in source programs
- the character and byte data manipulated by running programs

Source files are UTF-8 text in normal builds. Language keywords, operators,
directives, and generated RXAS symbols use the portable ASCII subset. String
data can contain Unicode text, and `.binary` values are available for byte
data that should not be treated as text.

For older or specialist platforms, source conversion may be part of the build
or packaging workflow, but the release documentation and examples assume UTF-8
source.
