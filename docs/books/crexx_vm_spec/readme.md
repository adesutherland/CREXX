# Composing the cRexx VM Reference 

This publication is mostly generated from different sources. Until the process has been automated, this howto serves to remember how to put the different parts together.

## The SVG images of the instruction frames

## The Examples and their output

These are to be found in the 'examples' subdirectory. The instruction chapter is generated. There is no point in editing it. There is an exec called instruction_doc.rexx which puts this file together from the 'instructions.sqb' database in the 'doc/instructions' directory. The exec needs to be run, for now, with nrc -exec instruction_doc.rexx; it will be eating crexx dogfood soon.

The explanations of what the instructions do is are the 'operation' subdirectory. Both are keyed by the signature of the instructions, i.e. their names and the type of their arguments.

