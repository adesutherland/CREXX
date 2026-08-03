## floatabs

```rexx
floatabs(number = .float) = .float
```

Returns the binary64 absolute value without decimal conversion. Positive and
negative zero both return positive zero, either infinity returns positive
infinity, and NaN propagates. NaN payload/sign details are not part of the
public contract.

Invalid dynamic conversion to `.float` signals `CONVERSION_ERROR`.
