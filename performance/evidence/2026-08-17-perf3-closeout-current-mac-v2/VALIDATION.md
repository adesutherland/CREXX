# Validation record

| Check | Result |
| --- | --- |
| Compiler repair full Debug CTest | 2,224/2,224 pass |
| Compiler repair focused profiling-off Release | 7/7 pass; first verdict accepted by Adrian |
| Portfolio-v2 cREXX opt/no-opt under `rxvm`, `rxtvm`, `rxbvm` | 36/36 process cells pass |
| Base64-v2 cross-runtime length/checksum | cREXX, ooRexx and NetRexx pass |
| Fresh profiling-off Release closeout gate | 22/22 pass |
| Lifecycle runner focused Debug | 3/3 pass after host-platform launcher repair |
| Final combined fresh Release replay | 26/26 pass |
| Formal timing initial | 516/516 processes pass |
| Governed timing append | 150/150 processes pass; no second append |
| RSS | 86/86 processes pass |
| Lifecycle | 70/70 phase samples pass |
| Artifact inventory | 87/87 rows hashed |
| Fusion inventory | 17 optimized images; 1,248 public sites and 34 exact private sites |

The compiler-repair and portfolio-qualification details are retained in their
own sibling evidence directories. This final bundle retains the formal
same-session Mac measurements and does not duplicate sanitizer, install,
package, Linux or Windows claims.
