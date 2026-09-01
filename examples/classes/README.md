# Class demonstrations

This directory is reserved for self-contained class and interface
demonstrations. These examples may be written using Level B or Level G syntax,
but their location does not make them part of either level's supported class
library contract.

- `global_object_keys.crexx` demonstrates an immutable, copyable handle for a
  global resource, canonical Level G `<eq>` through `.ObjectEquatable`, and a
  length-framed `namespace/type/value` key strategy using `rxfnsb.fnv`. The
  handle identifies the resource; it is not a live cross-worker reference to
  the resource itself.
