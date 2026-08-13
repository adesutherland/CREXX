# Collection Classes

The collection classes provide an object-oriented idiom for handling (adding, deleting, sorting, searching) of data. All are part of the data namespace. They implement interfaces like `.Iterable`, `.Iterator`, `.List`, `.Map` and `.Tree`. In this chapter these interfaces will be discussed and explained; the details are in the reference section of this book.

## The interfaces

These interfaces distinguish between three ways of managing data.

- **List** — an ordered sequence of elements.
- **Set** — a collection of unique elements.
- **Map** — a collection of key/value associations.

A useful analogy is:

- List → a shopping list
- Set → a bag of unique stamps
- Map → a dictionary

`Map` stores *pairs* rather than individual objects.



## List

A `List` is an ordered collection of elements. Unlike a `Set`, which is concerned solely with membership, a list preserves the position of every element and allows the same value to appear more than once. In many respects, a list corresponds to the intuitive notion of a sequence: the order in which elements are inserted is significant, and each element occupies a well-defined position within that sequence.

The concept of a list predates modern programming languages by many decades. In mathematics, ordered collections appear as sequences, tuples, and vectors, where the arrangement of the elements is as important as the elements themselves. The sequence *(2, 3, 5)* is fundamentally different from *(5, 3, 2)*, even though both contain the same values. Computer science adopted this notion early on, and lists became one of its most important abstract data types. They form the basis of countless algorithms and data structures, from symbol tables and compiler token streams to document models and graphical user interfaces.

The defining characteristic of a list is that every element has a position, usually referred to as its **index**. In cRexx, indexing begins at one, so the first element has index `1`, the second index `2`, and so forth. Because every element occupies a unique position, a list provides efficient access to the *n*th element, making it possible to retrieve or replace an element directly by its index. Unlike a set, duplicate elements are perfectly acceptable; if the same value occurs several times, each occurrence occupies its own position in the sequence.

The operations provided by the `List` interface revolve around this notion of ordered storage. Elements may be appended to the end of the list or inserted at an arbitrary position, causing subsequent elements to shift to make room. Existing elements can be replaced without altering the size of the list, or removed entirely, in which case later elements move forward to close the gap. Lists also support searching for a value, determining the size of the collection, and traversing the elements in their stored order. Because ordering is intrinsic to the abstraction, iteration always proceeds from the first element to the last unless explicitly reversed.

An important consequence of positional storage is that a list preserves duplicates exactly as they were inserted. Consider a compiler that records every identifier encountered in a source file. If the identifier `count` appears twenty-three times, a list faithfully records all twenty-three occurrences in their original order. A set, by contrast, would retain only a single instance because its purpose is to represent the unique identifiers appearing in the program rather than every occurrence.

Although the `List` interface defines a common abstraction, different implementations are optimized for different patterns of use. The most widely used implementation is `ArrayList`, which stores its elements in a dynamically resized array. Because the elements occupy contiguous memory locations, random access by index is extremely efficient and requires constant time. Appending new elements to the end of the list is likewise very efficient, with only occasional resizing of the underlying array. Inserting or removing elements in the middle of the list, however, requires subsequent elements to be shifted, making these operations proportional to the size of the list.

`LinkedList` takes a fundamentally different approach by storing each element in a separate node connected by references to its neighbours. This representation makes insertions and removals at either end of the list, or at a location already reached by an iterator, very efficient because only a small number of references must be updated. Random access, however, is considerably slower, since locating the *n*th element requires traversing the list from one end or the other. Consequently, a linked list is most appropriate when the application performs frequent insertions and deletions but comparatively little indexed access.

cRexx also provides a `Stack` class. It implements the familiar last-in, first-out behaviour.

Several useful operations are defined specifically for lists because of their ordered nature. A list may be traversed in either direction using a `ListIterator`, sorted according to the natural ordering of its elements or a supplied comparator, reversed, or partitioned into a view representing a contiguous range of elements. Searching operations can locate the first or last occurrence of a value, reflecting the fact that duplicate elements may exist at multiple positions. These operations have no meaningful counterparts for sets, where the absence of ordering makes concepts such as "first" and "last" undefined.

Lists are among the most versatile data structures in software development and are frequently used whenever information has a natural sequence. Examples include the lines of a source program, the tokens produced by a lexical analyser, the statements of an abstract syntax tree, the history of commands entered into a shell, the items displayed in a graphical menu, or the chapters of a book. In each of these cases, the order of the elements carries essential information that would be lost if they were stored in an unordered collection.

In practice, a `List` should be chosen whenever the relative order of elements is significant or when access by position is an important requirement. If duplicates must be preserved, if the *n*th element must be retrieved efficiently, or if the collection naturally represents a sequence rather than merely a set of distinct values, a `List` is the appropriate abstraction. The central question answered by a list is therefore not simply *whether* an element exists, but also *where* it occurs within the sequence. That emphasis on order and position distinguishes the `List` interface from every other collection type in cRexx class library.

### Summary

A `List` is an ordered collection.

Properties:

- preserves insertion order
- allows duplicate elements
- elements have an index
- random access may or may not be efficient depending on implementation

Example:

```
["apple", "banana", "apple", "pear"]
```

Duplicates are perfectly legal.

You can retrieve by position:

```
list.get(0)   -> "apple"
list.get(2)   -> "apple"
```

Typical operations

```
add(E e)
add(int index, E e)

get(int index)

set(int index, E e)

remove(int index)

contains(E e)

size()
```

Typical implementations

```
ArrayList
LinkedList
Stack (legacy)
```

### Complexity

For `ArrayList`

```
get(index)        O(1)

append            O(1) amortized

insert middle     O(n)

remove middle     O(n)
```

For `LinkedList`

```
get(index)        O(n)

insert front      O(1)

remove front      O(1)
```

cRexx also offers a `Stack` class which actually is limited form of a `list`, in which elements can only be added and removed from one end. It has `push` and `pop` methods but no `get`. A stack follows the LIFO (Last In, First Out) principle. Most explanations of the `Stack` type conjure up a picture of a stack of plates in a cafeteria.

## Set

A `Set` is a collection of **distinct elements**. Unlike a `List`, a set has no notion of position or index; what matters is whether an element is a member of the set, not where it appears. This seemingly simple idea has deep mathematical roots and has influenced many areas of computer science.

The concept of a set forms one of the foundations of modern mathematics. In the late nineteenth century, **Georg Cantor** developed set theory as a rigorous way of reasoning about collections of objects. Since then, sets have become fundamental to nearly every branch of mathematics. Computer science inherited these ideas, and many of its central concepts—including relations, functions, graphs, formal languages, and databases—can ultimately be described in terms of sets.

A mathematical set is simply a collection of distinct objects, called *elements* or *members*. The defining characteristic is that an element either belongs to the set or it does not; multiple occurrences of the same element have no meaning. Thus the sets `{2, 3, 5, 7}` and `{7, 2, 5, 3}` are considered identical because they contain exactly the same members, and `{2, 3, 3, 5, 7}` represents the very same set because duplicate elements are ignored.

The `Set` interface follows this mathematical model closely. When an element is added that is already present, the operation has no effect and the set remains unchanged. Consequently, the interface does not provide positional operations such as `get(int index)`, because the concept of an index is incompatible with the notion of a set. Instead, the fundamental operations revolve around membership: adding elements, removing them, testing whether a particular element is present, determining the number of elements, and iterating over the members.

One of the greatest strengths of the `Set` abstraction is that it naturally supports the operations of elementary set theory. The most familiar of these is the **union**, which combines two sets into a new set containing every element that appears in either of them. Equally important is the **intersection**, which retains only those elements that the two sets have in common. The **difference** of two sets consists of the elements that belong to one set but not the other, while the **symmetric difference** contains precisely those elements that occur in one set or the other, but not both.

Another important concept is that of a **subset**. A set *A* is said to be a subset of another set *B* if every element of *A* is also an element of *B*. If, in addition, the two sets are not identical, *A* is called a **proper subset** of *B*. Subset relationships occur frequently in computing, for example when verifying that a user possesses all required permissions, that one collection of compiler options is contained within another, or that the symbols used in an expression belong to a language's alphabet.

<!-- The mathematical definition of set equality also differs from that of a list. Two sets are equal whenever they contain exactly the same elements, irrespective of the order in which those elements were inserted. This property is reflected directly in cRexx's `equals()` method for sets, which compares membership rather than ordering. -->

The cRexx class library provides three principal implementations of the `Set` interface, each optimized for different requirements. `HashSet` stores its elements in a hash table and offers constant-time performance for insertion, removal, and membership testing on average. Because hashing does not preserve any ordering, the iteration order of a `HashSet` is intentionally unspecified. It is therefore the preferred implementation whenever ordering is irrelevant and maximum performance is desired.

`LinkedHashSet` extends this design by maintaining a linked list of its elements in insertion order. This preserves the efficiency of hash-based lookup while ensuring that iteration visits the elements in the order in which they were originally added. The additional bookkeeping incurs only a modest memory overhead, making `LinkedHashSet` an attractive choice when predictable iteration order is required.

`TreeSet` takes a different approach by storing its elements in a self-balancing binary search tree, specifically an AVL tree[^avl]. Rather than preserving insertion order, it maintains its elements in sorted order according to their natural ordering or a user-supplied comparator. Because the tree remains balanced, insertion, removal, and lookup all require logarithmic time. The ordered representation also enables a rich collection of navigation operations, including finding the smallest or largest element, locating the nearest element greater or less than a given value, and obtaining subsets that fall within a specified range. These capabilities make `TreeSet` particularly well suited for applications where maintaining sorted data is more important than achieving the absolute fastest lookup time.

In practice, a `Set` should be chosen whenever the uniqueness of elements is the primary concern. Typical applications include maintaining the collection of identifiers encountered during compilation, recording the set of visited nodes during graph traversal, representing the keywords of a programming language, storing the enabled options of a compiler, or eliminating duplicate entries from an input stream. In each of these cases, the essential question is not *where* an element occurs, but simply *whether* it is present. That distinction lies at the heart of the `Set` abstraction and explains why it occupies such a central place in both mathematics and cRexx class library.

### Summary

A `Set` stores unique elements.

There is no concept of position.

Example

```
{"apple", "banana", "pear"}
```

Adding another `"apple"` changes nothing.

```
set.add("apple")
```

returns

```
false
```

because it already exists.

Typical operations

```
add(E e)

remove(E e)

contains(E e)

size()
```

Notice there is no

```
get(index)
```

because elements have no index.

## HashSet

Internally based on a hash table.

Properties

- no duplicates
- very fast lookup
- iteration order unspecified

Typical complexity

```
add()         O(1)

contains()    O(1)

remove()      O(1)
```

## LinkedHashSet

Same uniqueness guarantee.

Additionally

- preserves insertion order

```
add red
add green
add blue
```

Iteration always produces

```
red
green
blue
```

## TreeSet

Internally a balanced binary search tree (specifically an AVL tree).

Properties

- unique elements
- automatically sorted

Example

Insert

```
pear
apple
orange
```

Iteration becomes

```
apple
orange
pear
```

Complexity

```
add()         O(log n)

contains()    O(log n)

remove()      O(log n)
```

## Map

A `Map` is a collection that associates **keys** with **values**. Unlike a `List`, which stores elements in a sequence, or a `Set`, which stores a collection of unique elements, a map stores *relationships*. Each key identifies exactly one value, allowing the value to be retrieved efficiently whenever its corresponding key is known. In many programming languages this abstraction is also known as an *associative array*, a *dictionary*, or a *symbol table*.

The concept of a map is closely related to the mathematical notion of a **function**. In mathematics, a function assigns every element of one set, called the *domain*, to a single element of another set, called the *codomain*. While cRexx maps are generally more flexible than mathematical functions—they are mutable, need not define a value for every possible key, and may associate different keys with the same value—the underlying idea is remarkably similar. A map defines a correspondence between one collection of objects and another.

This distinction is important because a map is fundamentally different from the other collection interfaces. A list answers the question *"What is the element at position* *n* *?"* A set answers *"Is this element present?"* A map answers *"Which value belongs to this key?"* Rather than storing isolated objects, a map stores associations, and those associations become the primary object of interest.

The defining characteristic of a map is that each key is unique. Attempting to insert a new key-value pair whose key already exists does not create a duplicate entry; instead, the new value replaces the old one. Values, however, need not be unique. Several different keys may legitimately refer to the same value. For example, a compiler may associate multiple identifiers with the same data type, or several employees may belong to the same department. The uniqueness constraint therefore applies only to the keys.

The operations provided by the `Map` interface reflect this model of association. New mappings can be inserted, existing mappings updated, and mappings removed entirely. Given a key, the associated value can be retrieved efficiently, or the map can be queried to determine whether a particular key or value is present. The interface also provides views of the map's keys, values, and entries, allowing the contents to be traversed in a variety of ways. These views illustrate the close relationship between maps and collections: while a map is not itself a `Collection`, it can expose its keys as a `Set`, its values as a `Collection`, and its key-value associations as a `Set` of `Map.Entry` objects.

From a mathematical perspective, maps support many operations analogous to those defined for functions and relations. Two maps may be compared for equality by determining whether they contain precisely the same key-value associations. A map may be copied, merged with another map, filtered according to its keys or values, or transformed by applying a function to each value.

The cRexx class library provides several implementations of the `Map` interface, each optimized for different requirements. The most frequently used implementation is `HashMap`, which stores its mappings in a hash table. Under normal circumstances, insertion, lookup, and removal all require constant time on average, making `HashMap` the preferred choice whenever maximum performance is desired and the order of the keys is unimportant. Because the placement of entries is determined by their hash codes, iteration order is intentionally unspecified and may vary as the map grows.

`LinkedHashMap` extends `HashMap` by maintaining a linked list of its entries. This additional structure preserves a predictable iteration order, usually the order in which mappings were inserted, while retaining nearly the same performance characteristics as a hash table. The class also supports access-order iteration, allowing recently accessed entries to migrate toward the end of the sequence. This feature makes `LinkedHashMap` particularly useful for implementing least-recently-used (LRU) caches and similar data structures.

`TreeMap` stores its entries in a self-balancing binary search tree, specifically an AVL tree. Rather than preserving insertion order, it maintains the keys in sorted order according to their natural ordering or a user-supplied comparator. As a consequence, insertion, removal, and lookup require logarithmic rather than constant time. In return, the map gains a rich collection of navigational operations, including finding the smallest or largest key, locating the nearest key greater or less than a given value, and obtaining views representing contiguous ranges of keys. These capabilities make `TreeMap` particularly attractive when ordered traversal or range queries are more important than raw lookup speed.

<!-- Several specialized implementations complement these three principal classes. `EnumMap` provides an exceptionally compact and efficient representation for keys drawn from an enumeration, while `IdentityHashMap` compares keys by object identity rather than logical equality. `WeakHashMap` stores keys through weak references, allowing entries to disappear automatically when their keys are no longer referenced elsewhere in the application. Finally, `ConcurrentHashMap` supports safe, highly scalable concurrent access by multiple threads and has become the standard choice for shared mutable maps in multithreaded applications. -->

Maps are among the most frequently used abstractions in software engineering because many problems naturally involve associations rather than sequences. A compiler maintains a symbol table that associates identifiers with their declarations, an interpreter maps variable names to their current values, a database index relates primary keys to stored records, and a web server associates URL paths with request handlers. Configuration systems map property names to configuration values, while caches associate computed results with the requests that produced them. In every case, the central concern is not the order in which objects were inserted, nor merely whether they exist, but the relationship between one object and another.

In practice, a `Map` should be chosen whenever information is naturally expressed as a collection of key-value associations. If each object has a unique identifier, if rapid lookup by that identifier is essential, or if the problem itself is fundamentally one of representing relationships, a map is almost always the appropriate abstraction. Whereas a `List` organizes information by position and a `Set` organizes it by membership, a `Map` organizes it by correspondence. That ability to model associations efficiently and naturally explains why the `Map` interface occupies such a central place in the cRexx class library.


A `Map` associates keys with values, as a dictionary does.

```
"NL" -> "Netherlands"

"UK" -> "United Kingdom"

"D" -> "Germany"
```

Each key is unique, but values need not be unique.

Example

```
Peter -> Muenchen

Adrian -> Birmingham

René -> Amsterdam

Rony -> Vienna

Michael -> Vienna
```

The city appears more than once.

Typical operations

```
put(K key, V value)

get(K key)

remove(K key)

containsKey(K key)

containsValue(V value)

keySet()

values()

entrySet()
```

Example

```
map.put("apple", 5);

map.put("banana", 8);

map.get("apple")
```

returns

```
5
```

If

```
map.put("apple", 12)
```

the previous value is replaced.

Result

```
apple -> 12
banana -> 8
```

## HashMap

Internally a hash table.

Complexity

```
put()          O(1)

get()          O(1)

remove()       O(1)
```

Order is unspecified.

## LinkedHashMap

Like `HashMap` but preserves insertion order.

```
A
B
C
```

will iterate

```
A
B
C
```

## TreeMap

Keys are automatically sorted.

Insert

```
orange
pear
apple
```

Iteration

```
apple
orange
pear
```

Complexity

```
put()          O(log n)

get()          O(log n)

remove()       O(log n)
```



## Comparing List, Set and Map

| Feature | List | Set | Map |
|----------|------|-----|-----|
| Stores | elements | unique elements | key/value pairs |
| Duplicate elements | Yes | No | Keys: No, Values: Yes |
| Ordered | Yes | Depends | Depends |
| Indexed | Yes | No | No |
| Lookup by key | No | No | Yes |
| Sorted variant | No | TreeSet | TreeMap |



## Choosing the Right Interface

Use a `List` when:

- order matters
- duplicates are allowed
- you need positional access

Example

```
shopping list

chapter list

playlist
```

Use a `Set` when:

- uniqueness matters
- fast membership testing is important

Example

```
keywords

visited URLs

registered usernames
```

Use a `Map` when:

- every object has an identifier
- you need fast lookup by key

Example

```
employee ID -> employee

word -> definition

country code -> country name
```



## Relationships between interfaces

Suppose you have

```
a = HashMap()
```

Internally it conceptually stores entries such as

```
("apple",5)

("banana",8)

("pear",2)
```

From this map you can obtain:

```
keySet()
```

which is a `Set of .string`

```
apple
banana
pear
```

or

```
values()
```

which is an array of integers.

```
5
8
2
```

or

```
entrySet()
```

which is a `Set` of keys

```
(apple,5)

(banana,8)

(pear,2)
```

Thus `Map` provides collection views of its keys, values, and entries.



## Summary

```
List
    Ordered sequence
    Duplicates allowed
    Indexed access

Set
    Unique elements
    No indexing
    Fast membership testing

Map
    Key/value associations
    Unique keys
    Fast lookup by key
```

A good rule of thumb is:

- If you ask "What is the *nth* item?" use a `List`.
- If you ask "Have I seen this item before?" use a `Set`.
- If you ask "Given this key, what value belongs to it?" use a `Map`.

## The Iterable interface

The `Iterable` interface is the smallest and most fundamental abstraction in the cRexx class library. It represents nothing more than the ability to traverse a collection of objects one element at a time. Any class that implements `Iterable` promises that its contents can be visited sequentially, regardless of how those contents are actually stored.

Unlike `List`, `Set`, or `Map`, `Iterable` says nothing about ordering, uniqueness, indexing, or searching. It merely states that there exists a way to obtain an iterator capable of visiting every element in the object. This modest contract makes `Iterable` one of the cornerstones, as it provides a common mechanism for traversing virtually every cRexx collection.

Conceptually, `Iterable` separates **traversal** from **representation**. A collection may internally be implemented as an array, a linked list, a balanced tree, a hash table, or even a structure that generates elements on demand. As long as it can produce an iterator, clients need not know or care about its internal organization.

The interface itself is remarkably small. Its only required method is `iterator`. As an example:

```rexx
options levelb comments_dash
namespace data expose StringIterable

/**
 * StringIterable is the interface which defines the contract
 * the implementing StringIterable classes (Set, Map, List)
 * should keep to.
 */

StringIterable: interface
  iterator: method = .StringIterator
```

which returns an object implementing the `Iterator` interface. Every time `iterator()` is called, a new iterator is created, allowing multiple independent traversals of the same collection.

It enables code that obtains an iterator and repeatedly calls its `hasNext()` and `next()` methods.

This design illustrates one of the central principles of object-oriented programming: clients should depend upon capabilities rather than concrete implementations. The enhanced `for` loop does not require an `ArrayList`, a `LinkedList`, or a `HashSet`. It requires only something that can be iterated.

One of the strengths of `Iterable` is that it allows iteration over objects that are not traditional collections. A directory of files, the lines of a text file, the nodes of a syntax tree, the neighbours of a graph vertex, or the results of a database query can all naturally be represented as iterable objects. In such cases, the object does not necessarily *contain* its elements permanently; it merely provides a mechanism for visiting them one after another.

Implementing `Iterable` is often straightforward. A class need only provide an implementation of the `iterator()` method that returns an appropriate iterator. This iterator encapsulates whatever knowledge is necessary to traverse the underlying data structure. The collection itself remains responsible for storing its data, while the iterator becomes responsible for navigating through it. This separation of responsibilities is one of the reasons the iterator pattern has proven so successful.

From a design perspective, `Iterable` occupies an important position in the hierarchy of abstractions. It is intentionally minimal, expressing only the capability of sequential traversal. More specialized interfaces such as `List` and `Set` build upon this foundation by adding operations for insertion, removal, searching, and ordering.

In practice, a class should implement `Iterable` whenever its contents have a natural sequential traversal.

Notice that `Map` is deliberately separate. A map is not a collection of elements but a collection of associations between keys and values. Nevertheless, the collections returned by `keySet()`, `values()`, and `entrySet()` all implement `Iterable`, allowing their contents to be traversed using exactly the same mechanisms as any other collection.

## The Iterator Interface

The `Iterator` interface provides a uniform mechanism for traversing the elements of a collection one at a time. It embodies the **Iterator** design pattern, one of the behavioural design patterns described by the "Gang of Four[^gof]," whose central purpose is to allow sequential access to the contents of an aggregate object without exposing its internal representation.

[^gof]: <!--cite-->[gamma1993design]

An iterator acts as a cursor positioned between the elements of a collection. Initially, the cursor is located before the first element. Repeated calls to `next()` advance the cursor through the collection, returning each element in turn until the end of the sequence is reached. This model allows clients to process arbitrarily large collections without requiring random access or exposing the collection's internal data structures.

The interface itself is deliberately compact. Its two essential methods are `hasNext()`, which determines whether another element is available, and `next()`, which returns that element while advancing the iterator to the next position. Together these methods define the basic protocol for sequential traversal.

One of the principal advantages of the iterator abstraction is that it completely decouples traversal from storage. The client need not know whether the underlying collection is implemented as a dynamic array, a linked list, a balanced tree, a hash table, or some more specialized data structure. The iterator encapsulates all knowledge of how to move from one element to the next, allowing the collection to change its internal representation without affecting client code.

Each call to an `Iterable` object's `iterator()` method produces a new iterator with its own independent traversal state. Consequently, multiple iterators may traverse the same collection simultaneously, each maintaining its own current position. One iterator may have reached the middle of the collection while another has only just begun, without interfering with one another. This independence is an important property of the design and enables many algorithms that require multiple concurrent traversals.

The order in which an iterator visits elements depends entirely upon the semantics of the underlying collection. A list iterator visits elements according to their positional order, a `LinkedHashSet` iterator follows insertion order, and a `TreeSet` iterator produces elements in sorted order. By contrast, the iteration order of a `HashSet` or `HashMap` is intentionally unspecified. The iterator therefore reflects the logical ordering defined by the collection rather than imposing one of its own.

Because iteration is encapsulated in a separate object, collections are free to provide specialized iterators when appropriate. The `List` interface, for example, defines the richer `ListIterator` interface, which extends `Iterator` with bidirectional traversal, indexed positioning, and the ability to insert or replace elements during iteration. Other specialized collections may likewise provide iterators with additional capabilities while remaining compatible with the basic `Iterator` abstraction.

In practice, iterators should be preferred whenever the objective is to process every element of a collection sequentially without requiring indexed access. They provide a uniform programming model that applies equally well to lists, sets, queues, trees, and many user-defined collections, allowing algorithms to be written independently of the particular data structure being traversed. This separation of traversal from representation is one of the key design principles underlying the cRexx class library.

### Live and Snapshot Iterators

An iterator provides a sequential view of the elements contained in a collection. While the basic purpose of an iterator is always the same—to traverse the elements of a collection without exposing its internal representation—not all iterators view the collection in the same way. The principal distinction is between **live iterators**, which remain connected to the underlying collection, and **snapshot iterators**, which traverse an immutable copy of the collection as it existed when the iterator was created.

A **live iterator** reflects the current state of its underlying collection. As the collection changes, the iterator may observe those changes, depending on the semantics of the particular collection. The iterator therefore represents a moving window onto the collection rather than a fixed picture of it. This approach is memory-efficient because no copy of the collection is required, and it is well suited to situations where the collection is not modified during iteration.

The disadvantage of a live iterator is that structural modifications to the collection may invalidate the iterator. Consider a list containing the elements *A*, *B*, *C*, and *D*. If an iterator has just returned *B* and another operation removes *C*, the iterator must somehow determine where to continue. Likewise, if an element is inserted before the iterator's current position, should the new element be visited or skipped? Different answers lead to different traversal semantics, and some situations make it impossible to continue safely.

A **snapshot iterator** takes a fundamentally different approach. Rather than traversing the original collection directly, it traverses a private copy created when the iterator is constructed. Once the snapshot has been taken, subsequent modifications to the original collection have no effect on the iterator. The sequence of elements returned by the iterator is therefore stable and deterministic, regardless of any later insertions, removals, or updates.

The principal advantage of snapshot iteration is safety. Because the iterator operates on an immutable view, it cannot be invalidated by changes to the underlying collection. Multiple threads may modify the original collection freely while existing iterators continue to traverse their own independent snapshots. The cost, however, is that creating the snapshot requires additional time and memory proportional to the size of the collection. Furthermore, the iterator does not reflect recent updates, since it represents the collection exactly as it existed at the instant the snapshot was taken.

The distinction between live and snapshot iterators mirrors a broader distinction found in database systems. A live iterator resembles reading directly from a table while updates are occurring, whereas a snapshot iterator resembles reading from a transaction snapshot that presents a consistent view of the database at a particular instant. Both approaches are valuable, but each is appropriate under different circumstances.

The choice between these two approaches ultimately reflects a trade-off between **freshness** and **stability**. A live iterator observes the current collection but may be invalidated by structural changes. A snapshot iterator guarantees a consistent traversal but observes only the past. Understanding this distinction is essential when designing collections and selecting the most appropriate iteration strategy for a particular application.

The cRexx `.stemIterator` offers a choice of **live** and **snapshot**-based iterators.

## The .rexx class as a string container

The `.rexx` class offers functionality that is on par with the Object Rexx and NetRexx object-oriented notation for strings. It returns `.rexx` objects so calls can be chained.

## The .stem class implements Rexx stem variables

The `.stem` class is recast as a collection class, a content-addressable container of `.string` while keeping the traditional Classic Rexx (dot) and the Object Rexx and \nr{} bracket notations, and extended with some readily usable container methods like `size()` and `.iterator`.

## Notes on performance

The `StringTreeMap` class is based on an AVL Tree for optimal performance. This is a balanced binary tree with guaranteed $Olog N$ performance for all operations. This class is written in cRexx and its performance has been benchmarked against a *red-black tree* implementation in C, which is what most class libraries use. Its performance is nearly identical to that native implementation; the native TreeMap, which is faster than the JVM version, is available but requires more investment in the build process of an application[^build]. Also, its availability cannot be guaranteed for every platform cRexx runs on.

[^avl]: <!--cite-->[knuth1998art]
[^build]: See the chapter on building application software in the Programming Guide.


## Bag collections

A `Bag` is a collection that can contain more than one occurrence of the same
object. In computer science this type of collection is also commonly called a
*multiset*. The term `Bag` emphasizes an important property of the collection:
objects are placed in the collection without assigning them a particular
position, and the same object may be placed in it more than once.

Consider a bag containing the following objects:

```text
apple
apple
orange
apple
pear
```

The bag contains five objects, of which three are distinct. The object
`apple` occurs three times, while `orange` and `pear` each occur once. The
number of occurrences is part of the state of the bag. A bag containing three
apples is therefore different from a bag containing two apples, even when all
other objects are the same.

This distinguishes a bag from both a `List` and a `Set`. A list also permits
duplicate objects, but associates each occurrence with a position. A set does
not associate objects with positions, but permits each object to occur only
once. A bag occupies the useful middle ground: duplicate objects are retained,
but their positions are not significant.

## Relationship to the collection interfaces

`Bag` is a sibling of `Set`, rather
than a specialization of it. A simplified view of the collection hierarchy is:

<!-- Todo: picture -->

At first sight it might appear reasonable for `Bag` to implement `Set`,
because both abstractions are concerned primarily with membership rather than
position. Their contracts are nevertheless incompatible. The defining
property of a set is that an object can occur at most once. Adding an object
which is already a member of a set does not create another occurrence.

A bag has the opposite behaviour. For example:

```text
bag.add("apple")
bag.add("apple")
bag.add("pear")
```

creates a collection containing three objects. The count of `apple` is two
and the count of `pear` is one. Performing the equivalent operations on a set
would produce a collection of only two objects.

Making `Bag` implement `Set` would consequently violate the meaning of the
`Set` interface. Code receiving a `Set` is entitled to assume that every
member occurs at most once. A bag cannot satisfy that assumption while still
providing its defining behaviour.

Both abstractions therefore implement `Collection` independently. This also
allows algorithms that require only general collection operations to work
with sets, bags and lists without depending on their different rules for
duplicate objects.

## Size and occurrence counts

For a bag, `size()` denotes the total number of occurrences in the collection,
not merely the number of distinct objects. If a bag contains three apples,
two oranges and one pear, its size is six.

The number of occurrences of a particular object is obtained with `count()`:

```text
bag.count("apple")     -- 3
bag.count("orange")    -- 2
bag.count("pear")      -- 1
bag.count("banana")    -- 0
```

A count of zero means that the object is not present. Consequently,
`contains(object)` can be understood as testing whether:

```text
bag.count(object) > 0
```

This relationship illustrates one of the differences between bags and sets.
For a set, the count of an object could only ever be zero or one, so a separate
count operation would add little information. For a bag, the count is an
essential property of every member.

It is useful to distinguish the *size* of a bag from its number of distinct
objects. A bag containing one thousand occurrences of the same object has a
size of one thousand, although it contains only one distinct object. This
distinction also has consequences for the way a bag can be implemented
efficiently.

## Adding and removing objects

Adding an object to a bag adds one occurrence. If the object is not already
present, its count becomes one. If it is present, its count is increased by
one. Repeated calls to `add()` therefore accumulate occurrences:

```text
bag.add("apple")
bag.add("apple")
bag.add("apple")
```

After these operations the bag contains three occurrences of `apple`.

Removing an object performs the corresponding operation in the other
direction. `remove()` removes one occurrence rather than necessarily removing
the object completely. If an object has a count greater than one, its count is
decreased. When its last occurrence is removed, the object is no longer a
member of the bag.

Suppose a bag contains:

```text
apple     3
orange    1
```

After:

```text
bag.remove("apple")
```

the bag still contains `apple`, but its count is now two. Removing `orange`,
on the other hand, causes `orange` to disappear from the bag because its count
becomes zero.

This behaviour is important when using the methods inherited from
`Collection`. Although a bag supports the familiar collection operations,
their effects must be interpreted in terms of occurrences.

## Iteration

Iteration over a bag visits the occurrences represented by the collection.
A bag containing three apples, two oranges and one pear therefore supplies six
objects through its iterator:

```text
apple
apple
apple
orange
orange
pear
```

The order shown here is only illustrative. A bag does not inherently define
an ordering. A hash-based implementation may return the objects in an
implementation-dependent order, while an ordered implementation may return
them according to a key or comparator.

The important property is that iteration reflects multiplicity. If an object
has a count of three, an ordinary collection iterator returns that object
three times. This keeps iteration consistent with `size()`: the number of
objects produced by a complete iteration is the size of the bag.

Sometimes an application is interested only in the distinct objects. Those
objects naturally form a set. For example, the bag:

```text
apple
apple
orange
apple
pear
orange
```

has the distinct members:

```text
apple
orange
pear
```

A bag implementation can therefore provide an operation that exposes its
distinct members as a `Set`. This does not make the bag itself a set; it is a
set-valued view of one aspect of the bag.

## Equality

The occurrence counts are part of the value of a bag and must consequently
be taken into account when bags are compared. The order in which objects were
added is not significant.

For example, these two bags are equal:

```text
Bag 1                 Bag 2

apple                  orange
apple                  apple
orange                 apple
```

Both contain two occurrences of `apple` and one occurrence of `orange`.

By contrast, a bag containing two apples and one orange is not equal to a bag
containing one apple and two oranges. The two bags have the same size and the
same set of distinct objects, but their occurrence counts differ.

This is another reason why a bag should not be regarded as a set. Comparing
only the distinct members would discard information that is an essential part
of the bag.

## Implementation

A bag does not need to store every occurrence as a separate entry. A
hash-based implementation can instead maintain a mapping from each distinct
object to its occurrence count. Conceptually, a bag containing three apples,
two oranges and one pear can be represented as:

```text
apple  -> 3
orange -> 2
pear   -> 1
```

Adding an existing object increments its stored count, while removing an
occurrence decrements it. When the count reaches zero, the map entry itself is
removed.

This representation has an important advantage when duplicate objects are
common. Its storage requirement is determined primarily by the number of
distinct objects rather than by the total size of the bag. A bag containing
one million occurrences of the same object needs only one map entry together
with its count.

When the underlying map is a hash map, `add()`, `remove()`, `contains()` and
`count()` normally have constant average-time complexity. An ordered bag can
instead be based on a tree map when predictable ordering of the distinct
objects is required.

The map used for the implementation remains an internal detail. From the
outside, the bag behaves as a collection of objects and occurrences rather
than as a map from objects to integers. This distinction should also be
preserved by facilities such as `Printable`: the external representation
describes the contents of the bag, not the data structure used to maintain
its counts.

## Choosing between a Bag, Set and List

The choice between these collection types follows directly from the
information that an application needs to preserve. A `Set` is appropriate
when only membership matters and duplicate objects must be eliminated. A
`List` is appropriate when occurrences may be duplicated and their positions
or sequence are significant.

A `Bag` is appropriate when duplicate occurrences are significant but their
positions are not. It answers not only the question *is this object present?*
but also *how many times is this object present?*

This makes bags particularly useful for frequency tables, inventories,
histograms, word counts and similar applications. In all of these cases the
number of occurrences carries information, while storing an explicit
position for every occurrence would add semantics that the application does
not require.
