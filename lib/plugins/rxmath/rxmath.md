# RXMATH Library Documentation

## Overview
RXMATH is a comprehensive mathematical library providing a wide range of mathematical, statistical, and utility functions. It's designed to be used with the crexx/pa Plugin Architecture.

## Function Categories (Basic Math, Trig, Hyperbolic, etc.)

# RXMATH Library Documentation

## Overview
RXMATH is a comprehensive mathematical library providing a wide range of mathematical, statistical, and utility functions. It's designed to be used with the crexx/pa Plugin Architecture. The library provides optimized implementations of common mathematical operations, statistical analysis tools, and utility functions.

## Function Categories

### Basic Mathematical Functions

#### Exponential and Logarithmic Functions
| Function | Description | Input Parameters | Return Type | Notes |
|----------|-------------|------------------|-------------|-------|
| `exp(x)` | e^x (exponential) | x: float | float | Returns e raised to power x |
| `exp2(x)` | 2^x | x: float | float | Returns 2 raised to power x |
| `log(x)` | Natural logarithm | x: float > 0 | float | Base e logarithm |
| `log2(x)` | Base-2 logarithm | x: float > 0 | float | Base 2 logarithm |
| `log10(x)` | Base-10 logarithm | x: float > 0 | float | Base 10 logarithm |
| `pow(x,y)` | x^y | x: float, y: float | float | x raised to power y |
| `pow10(x)` | 10^x | x: float | float | 10 raised to power x |

#### Trigonometric Functions
| Function | Description | Input Parameters | Return Type | Notes |
|----------|-------------|------------------|-------------|-------|
| `sin(x)` | Sine | x: float (radians) | float | Range: [-1, 1] |
| `cos(x)` | Cosine | x: float (radians) | float | Range: [-1, 1] |
| `tan(x)` | Tangent | x: float (radians) | float | Undefined at x = π/2 + nπ |
| `asin(x)` | Arc sine | x: float [-1, 1] | float | Range: [-π/2, π/2] |
| `acos(x)` | Arc cosine | x: float [-1, 1] | float | Range: [0, π] |
| `atan(x)` | Arc tangent | x: float | float | Range: [-π/2, π/2] |

#### Hyperbolic Functions
| Function | Description | Input Parameters | Return Type | Notes |
|----------|-------------|------------------|-------------|-------|
| `sinh(x)` | Hyperbolic sine | x: float | float | |
| `cosh(x)` | Hyperbolic cosine | x: float | float | Range: [1, ∞) |
| `tanh(x)` | Hyperbolic tangent | x: float | float | Range: [-1, 1] |
| `asinh(x)` | Inverse hyperbolic sine | x: float | float | |
| `acosh(x)` | Inverse hyperbolic cosine | x: float ≥ 1 | float | Range: [0, ∞) |
| `atanh(x)` | Inverse hyperbolic tangent | x: float (-1, 1) | float | |

#### Rounding and Absolute Value Functions
| Function | Description | Input Parameters | Return Type | Notes |
|----------|-------------|------------------|-------------|-------|
| `ceil(x)` | Ceiling | x: float | float | Smallest integer ≥ x |
| `floor(x)` | Floor | x: float | float | Largest integer ≤ x |
| `fabs(x)` | Absolute value | x: float | float | |
| `round(x)` | Round | x: float | float | Nearest integer |
| `trunc(x)` | Truncate | x: float | float | Integer part of x |

#### Special Mathematical Functions
| Function | Description | Input Parameters | Return Type | Notes |
|----------|-------------|------------------|-------------|-------|
| `sqrt(x)` | Square root | x: float ≥ 0 | float | |
| `cbrt(x)` | Cube root | x: float | float | |
| `hypot(x,y)` | Hypotenuse | x: float, y: float | float | √(x² + y²) |
| `erf(x)` | Error function | x: float | float | Range: (-1, 1) |
| `erfc(x)` | Complementary error function | x: float | float | Range: (0, 2) |
| `tgamma(x)` | Gamma function | x: float > 0 | float | |
| `lgamma(x)` | Log gamma function | x: float > 0 | float | Natural log of gamma |
| `fmod(x,y)` | Floating-point remainder | x: float, y: float ≠ 0 | float | x - n*y, n = trunc(x/y) |

### Statistical Functions
| Function | Description | Input Parameters | Return Type | Notes |
|----------|-------------|------------------|-------------|-------|
| `mean(a)` | Arithmetic mean | a: float[] | float | Array average |
| `stddev(a)` | Standard deviation | a: float[] | float | Population std dev |
| `covar(arg1,arg2)` | Covariance | arg1: float[], arg2: float[] | float | |
| `correl(arg1,arg2)` | Correlation coefficient | arg1: float[], arg2: float[] | float | Range: [-1, 1] |
| `regression(arg0,arg1,arg2,arg3)` | Linear regression | arg0: float[], arg1: float[], arg2: float, arg3: float | float | y = mx + b |

### Hash Functions
| Function | Description | Input Parameters | Return Type | Notes |
|----------|-------------|------------------|-------------|-------|
| `djb2(str)` | DJB2 hash | str: string | int | 32-bit hash |
| `murmur(str,seed)` | MurmurHash3 | str: string, seed: int | int | 32-bit hash |
| `fnv1a(str)` | FNV-1a hash | str: string | int | 32-bit hash |
| `crc32(str)` | CRC32 hash | str: string | int | 32-bit hash |

### Constants
| Function | Description | Return Type | Value |
|----------|-------------|-------------|-------|
| `pi()` | π constant | float | 3.141592653589793 |
| `euler()` | e constant | float | 2.718281828459045 |

### Utility Functions
| Function | Description | Input Parameters | Return Type | Notes |
|----------|-------------|------------------|-------------|-------|
| `uuid()` | Generate UUID v4 | none | string | Random UUID |
| `inlineC(code)` | Execute C code | code: string | string | Experimental |

## Error Handling

### Error Types
1. Domain Errors (e.g., sqrt(-1))
2. Range Errors (e.g., asin(2))
3. Invalid Input Errors (e.g., null arrays)

### Error Responses
- Returns `NaN` for invalid mathematical operations
- Returns `null` for invalid array operations
- Throws exception for severe errors (memory allocation, etc.)

## Examples
### Basic Mathematics
rexx
/ Trigonometric calculations /
angle = pi() / 4
result = sin(angle)^2 + cos(angle)^2 / Should be 1 /
/ Exponential and logarithmic /
value = exp(1) / e /
logval = log(value) / Should be 1 /



### Statistical Analysis
rexx
/ Basic statistics /
numbers = [1, 2, 3, 4, 5]
avg = mean(numbers)
std = stddev(numbers)
/ Correlation /
x = [1, 2, 3, 4, 5]
y = [2, 4, 6, 8, 10]
correlation = correl(x, y)