# MetaCatalog API

`MetaCatalog` is the metadata-catalog example implemented in
[`metaRexx-sample.crexx`](metaRexx-sample.crexx). It stores members under
`type:<member>` keys and relationships under
`ref:<relationship>:<from>:<to>` keys.

## Lifecycle

| Method | Return | Description |
|---|---|---|
| `MetaCatalog()` | `.MetaCatalog` | Create a closed catalog object. |
| `open(filename)` | `.int` | Open or create the catalog database. |
| `close()` | `.int` | Close the catalog database. |
| `isOpen()` | `.int` | Return `1` when open, otherwise `0`. |

## Members

| Method | Return | Description |
|---|---|---|
| `addMember(logicalName, sourceFile, metadataFile, memberType)` | `.int` | Add or replace a member record. `memberType` may be `rexxscript`, `bif`, `class`, `method`, `interface`, or `dummy`. |
| `containsType(logicalName)` | `.int` | Test whether a member is registered. |
| `getType(logicalName)` | `.string` | Return the raw member record, `NOT_FOUND`, or `ERROR`. |
| `removeType(logicalName)` | `.int` | Remove a member and its owned type record. |

## Relationships

| Method | Return | Description |
|---|---|---|
| `linkMembers(memberFrom, memberTo, forwardRelation, reverseRelation)` | `.int` | Store both directions of one relationship. The source must already exist. A missing target is automatically registered as a minimal `dummy` member. |
| `hasReference(relation, memberFrom, memberTo)` | `.int` | Test whether one relationship exists. |
| `removeReference(relation, memberFrom, memberTo)` | `.int` | Remove one relationship. |

Typical link:

```rexx
rc = catalog.linkMembers( ,
    "myRexx", ,
    "POS", ,
    "uses-bif", ,
    "is-used-by")
```

The relationship is stored as:

```text
myRexx --uses-bif--> POS
POS   --is-used-by--> myRexx
```

## Traversal and analysis

| Method | Return | Description |
|---|---|---|
| `billOfMaterial(member[, memberType])` | `.string[]` | Recursively list forward dependencies. An optional type filters displayed entries while retaining the root. |
| `impactAnalysis(member[, memberType])` | `.string[]` | Recursively list members affected by a member through reverse relationships. An optional type filters displayed entries while retaining the root. |
| `listDependencies(member)` | `.string[]` | List direct forward dependencies with relationship names and member types. |

The returned arrays use element `0` as their item count.

## Catalog listings

| Method | Return | Description |
|---|---|---|
| `listMembers([memberType])` | `.string[]` | List registered members and their incoming usage counts. An empty type lists all registered members. |
| `listDummyMembers()` | `.string[]` | List members registered with type `dummy`. |
| `countByType()` | `.string[]` | Return one `type = count` line for each registered member type. |

Example:

```rexx
members = catalog.listMembers("bif")
do i = 1 to members[0]
    say members[i]
end
```

## Record helpers

These methods support catalog record construction and inspection:

| Method | Return | Description |
|---|---|---|
| `makeReferenceKey(relation, memberFrom, memberTo)` | `.string` | Build a canonical relationship key. |
| `makeRecord(sourceFile, metadataFile, metadataClass, kind, interfaces)` | `.string` | Build the older extended record format used by the example. |
| `getRecordInterfaces(record)` | `.string[]` | Extract the comma-separated interface list from an extended record. |

## Internal traversal helpers

The following methods are implementation helpers rather than intended public
catalog operations:

`bomWalk`, `bomWalkEdge`, `impactWalk`, `impactWalkEdge`,
`findForwardReferences`, `findReverseReferences`, `resolveMemberName`,
`memberKind`, `alreadyVisited`, `markVisited`, and `addBomEntry`.

`addRelationship` and `makeTypeRecord` are also internal storage helpers used
by the public methods.
