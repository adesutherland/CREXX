# The .stem class

The `.stem` class provides the implementation of the Classic Rexx stem variables.

> Compound symbols and stems are used for more complex collections of variables, such as arrays and lists[^stem].

As a core language element this class is included in the namespace `rxfnsb`. In cRexx, a stem variable needs to be declared:

```rexx <!--stemexample.crexx-->
options levelb
import rxfnsb

a = .stem()
a.b = 'c'
a.c = 'd'
say 'a has' a.size() 'elements'
say 'a.b  =' a.b
say 'a[b] =' a[b]
say 'a.c  =' a.c
say 'a[c] =' a[c]
```

<!--splice--crexx stemexample.crexx-->

[^stem]: <!--cite-->[cowlishaw1985rexx]

Here we can see that in addtion to the . (dot) notation, cRexx supports, like Object Rexx and NetRexx, the alternative square bracket notation `[x]` for stems, and that there exists a method `.size()` which gives the current size of the stem variable.

We can loop over stem elements using the `key()` and `value()` methods, as in the next example:

```rexx <!--stemtest2.crexx-->
options levelb
import rxfnsb

countries = .stem()

countries.1	 = 'Germany'
countries.2	 = 'The Netherlands'
countries.3	 = 'The United Kingdom'

say '===================='

loop j=1 to countries.size()
  k = countries.key(j)
  v = countries.value(k)
  say k ":" v
end
```

<!--splice--crexx stemtest2.crexx-->

There is also support for the related `.stemIterator` class which makes *live* and *snapshot* iterators possible.
