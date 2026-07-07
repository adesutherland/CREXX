# Glossary

BIF
: Built-in-function. The traditional name for Rexx string functions.

Capability
: A cohesive unit of business or technical functionality provided by a system. Unlike a service or module, a capability is defined by what it accomplishes rather than how it is implemented.

Cognitive Load
: The mental effort required to understand, modify, or operate a system. Modern architectural practice attempts to minimize unnecessary cognitive load by limiting dependencies, simplifying interfaces, and exposing only essential details.

Fitness Function
: An automated test or measurement that continuously verifies that an architectural property remains true. Examples include limits on coupling, response time, dependency counts, or code complexity.

Lowering
: The process of translating a high-level representation into a more concrete or implementation-oriented one while preserving its meaning. In compilers, lowering typically refers to successive transformations from an abstract syntax tree through intermediate representations to machine instructions. More recently, the term has been adopted in software architecture and AI systems to describe the translation of abstract intent into executable actions.

Ratchet
: A mechanism that permits progress in one direction while preventing regression. In software engineering, a ratchet is usually an automated process, such as a test, quality gate, or architectural rule which ensures improvements cannot silently be undone. Examples include increasing minimum test coverage, reducing dependency counts, or preventing the introduction of new compiler warnings.

Scalar
: A scalar is a single, indivisible value that represents one quantity. Unlike compound values (such as arrays, lists, records, or objects), a scalar has no internal components that are directly accessible.

Shape
: The overall structural form of a system, data model, API, or solution, emphasizing relationships rather than implementation details. The term is intentionally informal and generally refers to architectural organization rather than specific software constructs.

Surface
: The externally visible portion of a software component through which other components interact with it. Depending on context, this may refer to an API surface, user interface surface, or attack surface. Architects generally seek to minimize exposed surface area in order to reduce complexity, coupling, and maintenance effort.
