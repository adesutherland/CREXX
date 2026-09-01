# DECIMAL-01 D3 fixed-34 decQuad candidate

This directory contains an opt-in comparison candidate. It is not a production
provider and does not change the default decimal plugin, plugin ABI, RXAS or
RXBIN.

Configure with CREXX_BUILD_DECQUAD_DECIMAL_CANDIDATE=ON. The candidate stores
one pointer-free 16-byte decQuad directly in VM-owned decimal sidecar storage
and uses the already-vendored decimal128 implementation. It explicitly rounds
parse and arithmetic results to the admitted cREXX 9- or 18-digit context.

The boundary is intentionally narrower than mc_decimal:

- numeric contexts above 34 digits are unsupported;
- decimal power is implemented only for integral exponents by composing
  decQuad multiply/divide because decQuad has no power primitive;
- only checksum-matching 9- and 18-digit cells are admissible as D3 evidence;
- the candidate remains a fixed-precision performance ceiling, not an
  arbitrary-precision replacement proposal.

The lifecycle test covers raw byte copy, destruction of the original owner and
in-place arithmetic. The direct-core comparator separates decQuad arithmetic
from plugin and VM adapter cost.
