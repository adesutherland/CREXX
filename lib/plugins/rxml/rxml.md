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
