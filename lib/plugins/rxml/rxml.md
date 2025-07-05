# XML Processing Functions

## Function Reference

### xmlflags - Sets XML processing flags for debugging
```
parm 1 flags         Debug flags (DEBUG, NDEBUG, RESET)
returns 0
```

### xmlparse - Parse XML string into internal document structure  
```
parm 1 xml_string    XML content to parse
returns -1 or 0      0 on success, -1 on error
```

### xmlfind - Find elements by tag name and store in array
```
parm 1 tag          XML tag to search for
parm 2 results      Array to store matching elements
returns count       Number of elements found
```

### xmlbuild - Build XML document from root and element array
```
parm 1 root_name    Name of root element
parm 2 elements     Array of element names and values
returns string      Generated XML document
```

### xmlerror - Get last error message from XML processing
```
returns string      Last error message or empty string
```

### xmlgetattr - Get attribute value from specified element
```
parm 1 attribute    Name of attribute to retrieve
parm 2 instance     Element instance number (1-based)
returns string      Retrieved value or empty if not found
```

### xmlsetattr - Set attribute value for specified element
```
parm 1 attribute    Name of attribute to set
parm 2 instance     Element instance number (1-based)
parm 3 value        New attribute value
returns -1 or 0     -1 on error, 0 on success
```

### xmlremattr - Remove attribute from specified element
```
parm 1 attribute    Name of attribute to remove
parm 2 instance     Element instance number (1-based)
returns -1 or 0     -1 on error, 0 on success
```

### xmlattrcount - Get number of attributes for specified element
```
parm 1 instance     Element instance number (1-based)
returns -1 or count -1 on error, count on success
```

### xmlattrat - Get attribute name and value at specified index
```
parm 1 instance     Element instance number (1-based)
parm 2 index        Attribute index (0-based)
parm 3 name         Output: attribute name
parm 4 value        Output: attribute value
returns -1 or 0     -1 on error, 0 on success
```

### Examples

#### Using xmlbuild
```xml
// Input array structure:
elements = [
    {name: "person", value: null, attrs: {id: "1"}},
    {name: "name", value: "John"}
]

// Result:
<person id="1">
    <name>John</name>
</person>
```

#### Using xmlfind
```xml
// Input XML:
<root>
    <person>John</person>
    <person>Jane</person>
</root>

// Usage:
results = []
count = xmlfind("person", results)
// results now contains both person elements
```

### Error Handling

Each function that can encounter errors follows these conventions:

1. Return Values:
   - `-1`: Indicates an error occurred
   - `0` or positive number: Indicates success
   - Empty string: For string-returning functions when errors occur

2. Error States:
   The `xmlerror()` function will return one of these messages:
   - "Invalid XML syntax" - Malformed XML structure
   - "Element not found" - Referenced element doesn't exist
   - "Invalid attribute" - Attribute name is invalid or doesn't exist
   - "Invalid instance number" - Element instance number out of range
   - "Memory allocation failed" - Internal memory allocation error
   - "Invalid parameter" - Function parameter has invalid value/type
   - "Document not initialized" - No XML document has been parsed

3. Best Practices:
```xml
// Always check return values
if (xmlparse(content) == -1) {
    error_msg = xmlerror()
    // handle error
}

// For attribute operations, verify element exists
count = xmlfind(tag, results)
if (count > 0) {
    if (xmlsetattr("id", 1, "new_value") == -1) {
        error_msg = xmlerror()
        // handle error
    }
}
```

4. Error Recovery:
   - After an error, the XML document state remains unchanged
   - Use `xmlflags(RESET)` to clear error state and reset processing
   - Subsequent operations will work on the last successfully parsed document

# RXML Error Handling

## Error Messages

### 1. Parsing Errors
- `"Malformed XML tag"` - Invalid XML syntax or structure
- `"Too many elements"` - Exceeded maximum element count
- `"XML nesting too deep"` - Exceeded maximum nesting depth
- `"Mismatched closing tag"` - Closing tag doesn't match opening tag
- `"Unclosed tags"` - Missing closing tags

### 2. State Errors
- `"No XML document loaded"` - Operation attempted without parsed document
- `"xmlfind must be called before accessing attributes"` - Attribute access sequence error
- `"Invalid instance number"` - Referenced element instance doesn't exist

## Return Values
All XML functions follow these conventions:
- `-1`: Error occurred (check xmlerror() for details)
- `0`: Success for operations without return data
- `>0`: Success with a count or size value
- Empty string: For string operations when an error occurs

## Error Checking
```c
// Basic error checking
if (xmlparse(content) == -1) {
    char* error = xmlerror();
    // handle error
}

// Attribute operations require xmlfind first
if (xmlfind("element", results) > 0) {
    if (xmlgetattr("id", 1, value, sizeof(value)) == -1) {
        char* error = xmlerror();
        // handle error
    }
}
```

## Debug Support
Enable debug mode to see error messages as they occur:
```c
// Enable debug output
xmlflags("DEBUG");

// Debug messages will show as:
// RXML: error message here
```

## Error Recovery
1. After an error, the document state remains unchanged
2. Use xmlflags("RESET") to clear error state
3. Parse a new document to reset all state

## Best Practices
1. Always check return values
2. Call xmlfind before attribute operations
3. Enable debug mode during development
4. Use xmlerror() to get detailed error information
5. Clear errors with xmlflags("RESET") when retrying operations

# RXML API Reference

## Core Functions

### xmlfind
```c
int xmlfind(const char* tag, int* results)
```
Finds all elements with the specified tag name. Must be called before any attribute operations.
- **Returns**: Number of elements found or -1 on error
- **Note**: This function establishes the context for subsequent attribute operations

### xmlgetattr
```c
int xmlgetattr(const char* name, int instance, char* value, int maxlen)
```
Gets the value of an attribute from a previously found element.
- **Requires**: Prior successful call to xmlfind()
- **Parameters**:
  - name: Attribute name to retrieve
  - instance: Element number (1 to N) from previous xmlfind results
  - value: Buffer to store result
  - maxlen: Maximum length of value buffer
- **Returns**: Length of attribute value or -1 on error

## Usage Example
```c
// Correct usage pattern:
int results[100];
int count;

// First find the elements
count = xmlfind("person", results);
if (count > 0) {
    // Then access attributes
    char value[100];
    if (xmlgetattr("name", 1, value, sizeof(value)) != -1) {
        // Use attribute value
    }
}

// Incorrect usage - will fail:
char value[100];
xmlgetattr("name", 1, value, sizeof(value));  // Error: xmlfind not called
```

## Function Dependencies
```
xmlgetattr  → Requires → xmlfind
xmlsetattr  → Requires → xmlfind
xmlremattr  → Requires → xmlfind
xmlattrcount → Requires → xmlfind
xmlattrat   → Requires → xmlfind
```

## Error Handling
See [Error Handling Documentation](error-handling.md) for complete details.
