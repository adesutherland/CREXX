# Linked List Documentation

## Overview
The llist plugin provides linked list functionality for REXX programs. It supports multiple independent linked lists, memory management, and debugging capabilities.

## Functions

### Basic Operations

#### appendnode(qname, message)
Appends a node to the end of the list.
- `qname`: Integer (0-98) identifying the list
- `message`: String to store in the node
- Returns: 0 on success, -8 on failure

#### prependnode(qname, message)
Adds a node to the beginning of the list.
- `qname`: Integer (0-98) identifying the list
- `message`: String to store in the node
- Returns: 0 on success, -8 on failure

#### insertnode(qname, message, mode)
Inserts a node relative to the current position.
- `qname`: Integer (0-98) identifying the list
- `message`: String to store in the node
- `mode`: "B" for before current node, "A" for after
- Returns: 0 on success, -8 on failure

#### removenode(qname)
Removes the current node from the list.
- `qname`: Integer (0-98) identifying the list
- Returns: 0 on success, -8 on failure

### Navigation

#### currentnode(qname)
Returns the string stored in the current node.
- `qname`: Integer (0-98) identifying the list
- Returns: String value or "$EMPTY-LLIST-n$" if empty

#### nextnode(qname)
Moves to the next node and returns its value.
- `qname`: Integer (0-98) identifying the list
- Returns: String value or "$END-OF-LLIST-n$" if at end

#### prevnode(qname)
Moves to the previous node and returns its value.
- `qname`: Integer (0-98) identifying the list
- Returns: String value or "$TOP-OF-LLIST-n$" if at start

#### setnode(qname, position)
Sets the current node position.
- `qname`: Integer (0-98) identifying the list
- `position`: "FIRST", "LAST", or relative position (+/-n)
- Returns: Address of new current node or 0

### Memory Management

#### freellist(qname)
Frees all memory associated with the list.
- `qname`: Integer (0-98) identifying the list
- Returns: 0 on success

### Debugging

#### debug(qname)
Performs memory validation and prints statistics.
- `qname`: Integer (0-98) identifying the list
- Returns: 0 if valid, -1 if corruption detected

#### cleanup()
Prints memory statistics and checks for leaks.
- Returns: 0

### Display Functions

#### listnode(qname)
Displays the contents of the list.
- `qname`: Integer (0-98) identifying the list
- Returns: 0

#### listllist(qname)
Displays detailed list information including pointers.
- `qname`: Integer (0-98) identifying the list
- Returns: 0

## Memory Protection Features

The plugin includes several memory protection features:
- Buffer overflow detection
- Memory leak detection
- Double-free detection
- Memory corruption detection
- Link integrity validation

## Example Usage 

## Error and Return Codes

### Error Codes
- 0: Success - Operation completed successfully
- -8: Memory/Operation failure
  - Memory allocation failed
  - Invalid operation attempted
  - Memory free operation failed
- -1: Memory corruption detected
  - Invalid memory header magic number
  - Buffer overflow detected
  - Invalid linked list pointers

### Special Return Values
For string-returning functions:
- "$EMPTY-LLIST-n$": List n is empty (currentnode)
- "$END-OF-LLIST-n$": Reached end of list n (nextnode)
- "$TOP-OF-LLIST-n$": Reached start of list n (prevnode)
where n is the list number (qname)

For address-returning functions:
- 0: Operation failed or null pointer
- non-zero: Valid memory address

## Common Error Scenarios
1. Memory Allocation (-8)
   - System out of memory
   - Invalid size requested
   - Heap corruption

2. List Operations (-8)
   - Invalid list number (qname > 98)
   - Operation on empty list
   - Invalid current node

3. Memory Corruption (-1)
   - Buffer overflow detected
   - Invalid memory header
   - Corrupted linked list pointers
   - Double-free attempted