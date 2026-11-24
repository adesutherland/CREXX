# Procedural Data Dictionary Facade

User guide for examples/native/sqlite/data_dictionary_facade.crexx.

The facade hides SQLite handles, member IDs, transactions, encoded rows and
report formatting. Applications work with member names and .string[] arrays.

## Opening and closing

Open a dictionary before using any other facade command. Use :memory: for a
temporary dictionary. A persistent database rejects a repeated ddAdd for an
already defined member; an existing dummy can be completed in place.

    rc = ddOpen("orders.db")
    if rc <> 0 then do
      say "Cannot open dictionary:" ddLastError()
      return 1
    end

    rc = ddClose()
    if rc <> 0 then say "Cannot close dictionary:" ddLastError()

An unfinished active definition is not committed by ddClose.

## Definition

    ddOpen(filename)
    ddClose()
    ddBegin()
    ddAdd(name, member_type, description)
    ddReplace(name, member_type, description)
    ddAmend(name, member_type, description)
    ddRemove(name)
    ddAlias(value, alias_type)
    ddAttribute(attribute_name, attribute_value)
    ddLink(target, link_type, position, slot)
    ddEnd()

ddAdd starts a buffered definition. The next ddAdd implicitly completes the
previous definition. ddEnd is the explicit completion and error boundary.
ddBegin starts an explicit transaction block.

ddReplace replaces the complete definition while preserving member identity
and incoming references. Aliases, attributes and outgoing links are replaced.

ddAmend updates type and description, replaces only supplied attributes by
name, and replaces only supplied links by position. Unspecified attributes and
links remain unchanged. Aliases remain unchanged.

ddRemove removes aliases, attributes and outgoing links. A member still
referenced by another member remains as an untyped dummy. After its last
incoming reference disappears, the dummy and orphaned dummy targets are
removed.

## Aliases, attributes and links

Aliases are alternative names. Typed aliases are unique per member and alias
type; assigning the same typed alias type again replaces its value. Untyped
aliases are independently repeatable by value only once. Alias types are
case-insensitive, while member names and alias values are case-sensitive.
The generated UPPER alias is reserved and maintained automatically.

Attributes are name/value pairs and attribute names are unique per member.
ddAmend replaces only the named attributes.

ddLink(target, link_type, position, slot) records one outgoing positioned
relationship. A position of 0 selects the next available position. Missing
targets become reusable dummy members. The same target can occur at several
positions, and slot preserves the occurrence text used by reports.

## Transactions and error boundaries

Without ddBegin, each completed member definition is an independent
transaction. With ddBegin, several definitions form one transaction.
Nested explicit blocks are rejected. A new ddAdd completes the previous
definition, and the final ddEnd commits the explicit block.

    rc = ddBegin()
    if rc <> 0 then do
      say "Cannot begin transaction:" ddLastError()
      call ddClose
      return 1
    end
    call ddAdd "ORDER_RECORD", "RECORD", "Sales order"
      call ddLink "CUSTOMER_NUMBER", "contains", 1, "customer_number"
    call ddAdd "CUSTOMER_NUMBER", "FIELD", "Customer number"
    rc = ddEnd()

The first error within a definition is retained until ddEnd. A failed
ddBegin cannot cause a later definition to be committed accidentally.

## Listing and reports

    lines = ddList(member_type, prefix_string, any_string)
    details = ddListMembers(selected, member_type, prefix_string, any_string)

The three ddList filters are optional and default to empty strings.
member_type is a complete, case-insensitive filter; the stored type spelling
is normalized to uppercase. prefix_string
and any_string apply to the case-sensitive member name; prefix_string selects
names beginning with the prefix and any_string selects names containing the
substring. Filters combine.

ddMembers(member_type) remains as a compatibility wrapper for ddList.
ddListMembers preserves selection order, omits missing names, and returns
current details without modifying selected. Its member_type filter is
case-insensitive; its prefix and substring filters are case-sensitive.

Formatted reports:

    lines = ddBom(member_name, max_depth, link_type)
    lines = ddWhereUsed(member_name, max_depth, link_type)
    lines = ddDummies()
    lines = ddFindAlias(alias, alias_type)
    lines = ddListAliases(member_name)
    lines = ddListAllAliases()
    lines = ddMemberDetails(member_name)
    lines = ddAttributes(member_name)
    lines = ddMemberReport(member_name)

    lines = ddPerform(selected, "MEMBER_REPORT", 0, "", 1)

ddMemberDetails returns the current member name, normalized type, description,
outgoing link count and incoming link count. ddAttributes returns the current
attribute name/value rows. Both return formatted lines, set ddRc()/ddLastError(),
and fail if the member does not exist or a definition is still active.

ddMemberReport combines those details with all current attributes, direct
outgoing links and direct incoming links in one readable report. Link rows retain
member name, link type, position and slot text; the report does not recurse.
Each report begins with `REPORT OF member_name` and ends with `END OF
member_name`, which also makes repeated reports safe to concatenate.

ddPerform runs one supported report for each exact name in a selection, in
selection order. Each report command owns its `REPORT OF` / `END OF` boundaries,
so the returned reports can be concatenated safely. Supported
operations are MEMBER_REPORT, MEMBER_DETAILS, ATTRIBUTES, BOM, WHERE_USED and
LIST_ALIASES. The optional final argument prints the returned lines when nonzero;
the lines are still returned to the caller. A failure identifies the member and
operation and stops the batch.

## Graphviz DOT export

    dot_lines = ddGraph(member_name, max_depth, link_type)
    dot_lines = ddGraphMembers(selected)

`ddGraph` follows outgoing links from one exact member. Its depth and link-type
arguments follow `ddBom`: zero means unlimited depth and an empty link type means
all types. Cycles terminate safely, each node is declared once, and each stored
link occurrence is emitted, including parallel and cycle-closing links.

`ddGraphMembers` uses only current exact names in `selected`. It emits selected
members once and only links whose source and target are both selected. Missing
names are omitted without changing the input array; an empty selection produces
an empty valid graph. Both functions return DOT source lines and do not execute
Graphviz or modify the dictionary.

The output is a non-`strict` left-to-right `digraph` with quoted member names and
link-type labels. Names and labels are escaped for quotes, backslashes, tabs and
line breaks. Save one returned line per output line and render separately, for
example:

    dot -Tpng order.dot -o order.png

Exact-name query helpers, suitable for selections:

    names = ddMemberNames(member_type, prefix_string, any_string)
    names = ddBomNames(member_name, max_depth, link_type)
    names = ddWhereUsedNames(member_name, max_depth, link_type)

The names-only BOM and where-used helpers deduplicate names while preserving
first appearance order. For ddMemberNames, member_type matching is
case-insensitive and name filtering is case-sensitive. Formatted reports retain
all occurrences and paths.

## KEEP, DROP and printing

    selected = ddKeep(first, second)
    selected = ddDrop(selected, excluded)
    rc = ddPrint(lines)

ddKeep is a union. ddDrop is set difference. Both compare exact,
case-sensitive names, preserve first appearance order, remove duplicates, and
leave input arrays unchanged. ddPrint prints each array element in order.

Example:

    records = ddMemberNames("RECORD", "ORDER_", "")
    bom_names = ddBomNames("ORDER_RECORD", 0, "contains")
    selected = ddKeep(records, bom_names)
    excluded = .string[]
    excluded[1] = "ORDER_DATE"
    selected = ddDrop(selected, excluded)
    details = ddListMembers(selected)
    if ddRc() = 0 then call ddPrint details

## Errors

    rc = ddRc()
    message = ddLastError()

Check ddEnd, ddClose, and report/listing calls at their natural boundaries.
The first error in a definition is retained until ddEnd. A failed ddBegin
cannot accidentally commit a later definition.

Pure ddKeep, ddDrop and ddPrint operations do not require an open dictionary.

## SQL data model

The current SQLite implementation uses schema version 7. Opening a database
enables foreign-key enforcement and creates or migrates these tables.

### member

    member(
      member_id   INTEGER PRIMARY KEY,
      member_name TEXT NOT NULL COLLATE BINARY UNIQUE,
      member_type TEXT NOT NULL,
      description TEXT,
      is_dummy    INTEGER NOT NULL DEFAULT 0 CHECK(is_dummy IN (0,1))
    )

member_id is internal and is never required by facade callers. member_name is
the case-sensitive dictionary key. A dummy has is_dummy = 1 and an empty
member_type and description. Completing a dummy updates this same row, which
preserves its identity and incoming references.

### member_attribute

    member_attribute(
      member_id      INTEGER NOT NULL REFERENCES member(member_id)
                     ON DELETE CASCADE,
      attribute_name TEXT NOT NULL,
      attribute_value TEXT NOT NULL,
      PRIMARY KEY(member_id, attribute_name)
    )

An attribute name is unique per member. Amend uses the primary key to replace
only the named attributes.

### member_alias

    member_alias(
      member_id  INTEGER NOT NULL REFERENCES member(member_id)
                 ON DELETE CASCADE,
      alias      TEXT NOT NULL COLLATE BINARY,
      alias_type TEXT COLLATE BINARY,
      PRIMARY KEY(member_id, alias, alias_type)
    )

Typed aliases are unique per member and alias type. Untyped aliases are unique
per member and alias value. The generated UPPER alias is maintained by the
dictionary and is reserved from ddAlias.

### member_reference

    member_reference(
      caller_id   INTEGER NOT NULL REFERENCES member(member_id)
                  ON DELETE RESTRICT,
      successor_id INTEGER NOT NULL REFERENCES member(member_id)
                   ON DELETE RESTRICT,
      link_type   TEXT NOT NULL DEFAULT 'contains',
      position    INTEGER NOT NULL CHECK(position > 0),
      link_syntax TEXT NOT NULL DEFAULT '',
      PRIMARY KEY(caller_id, position)
    )

Each row is one positioned outgoing link. caller_id identifies the source
member and successor_id identifies the target. Position is unique within a
caller, so an amendment replaces a link by its position. A successor index
supports reverse where-used queries. A uniqueness index also prevents
duplicate caller/successor/link-type/position rows.

The schema also maintains indexes for typed and untyped aliases, successor
lookup and reference identity. Deleting a member is restricted while
references remain; ddRemove therefore converts a referenced member to a
dummy before eventual deletion.

## Coverage

The facade currently covers lifecycle, transactions, definitions, aliases,
attributes, links, listings, selections, BOM/where-used reports, member details,
attribute reports, dummy and alias reports, KEEP/DROP, printing and status
handling.

The storage layer additionally has direct successors, direct callers and raw
positioned-link queries. These remain lower-level APIs rather than dedicated
facade commands.

See:

- examples/native/sqlite/data_dictionary_facade_sample.crexx
- examples/native/sqlite/data_dictionary_facade_batch_sample.crexx
- examples/native/sqlite/data_dictionary_facade_report_test.crexx
- examples/native/sqlite/data_dictionary_facade_replace_test.crexx
