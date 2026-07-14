# Level C `WORDS`

The standalone direct entry point is:

```rexx
rexxclassicbif_words(
  context = reference .RexxBifCallContext) = .RexxValue
```

It implements `WORDS(string)`, returning the number of whitespace-delimited
words as a RexxValue integer string. Empty and blank-only strings return `0`.
The direct implementation uses the VM's Unicode whitespace classification.

The CheckArgs contract is `rANY`. Standard context errors are `RXC-LC-40.3`,
`40.4`, and `40.5` for argument count and a missing required argument.

The implementation performs one allocation-free scan. It does not call the
Level B selector or dispatch through the compatibility controller. The old
common body remains deprecated only for current generated artifacts.

Supporting a configured `Config_OtherBlankCharacters` set is tracked as the
shared dependency for the word-family Level C BIFs.

`lib/rxfnsc/tests_functional/testRexxClassicBifWords.crexx` calls the entry
directly in optimized and unoptimized overlays. It covers ordinary, empty,
blank-only, repeated, leading/trailing, and Unicode whitespace; source
non-mutation; and all standard argument errors.
