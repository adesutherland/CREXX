/**
 * Simple XML Parser Implementation
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crexxpa.h"
#include "rxml.h"

#define XML_ERROR_MALFORMED    "Malformed XML tag"
#define XML_ERROR_OVERFLOW     "Too many elements"
#define XML_ERROR_DEPTH        "XML nesting too deep"
#define XML_ERROR_MISMATCH     "Mismatched closing tag"
#define XML_ERROR_UNCLOSED     "Unclosed tags"
#define XML_ERROR_NO_DOC       "No XML document loaded"
#define XML_ERROR_NO_FIND      "xmlfind must be called before accessing attributes"
#define XML_ERROR_INVALID_INST "Invalid instance number"

/* XML processing flags and globals */
static int xml_debug = 0;

#define XML_DEBUG(msg) if(xml_debug) printf("RXML: %s\n", msg)

static void xml_set_error(XMLDocument* doc, const char* error) {
    if (!doc) return;
    snprintf(doc->error, sizeof(doc->error), "%s", error);
    XML_DEBUG(error);
}

static char* xml_trim(char* str) {
    char* end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

static int xml_parse_attribute(const char* xml, int* pos, XMLElement* element) {
    char name[XML_MAX_ATTR_NAME] = {0};
    char value[XML_MAX_ATTR_VALUE] = {0};
    int name_len = 0;
    int value_len = 0;
    char quote_char = 0;

    // Skip whitespace
    while(isspace(xml[*pos])) (*pos)++;
    
    // Get attribute name
    while(xml[*pos] != '=' && xml[*pos] != '>' && xml[*pos] != '\0' && name_len < XML_MAX_ATTR_NAME - 1) {
        name[name_len++] = xml[(*pos)++];
    }
    
    if(xml[*pos] != '=') return 0;
    (*pos)++; // Skip '='
    
    // Skip whitespace
    while(isspace(xml[*pos])) (*pos)++;
    
    // Check quote type
    if(xml[*pos] == '"' || xml[*pos] == '\'') {
        quote_char = xml[*pos];
        (*pos)++;
    } else {
        return 0;  // Must have quotes
    }
    
    // Get attribute value
    while(xml[*pos] != '\0' && value_len < XML_MAX_ATTR_VALUE - 1) {
        if(xml[*pos] == quote_char) {
            // Check for escaped quotes
            if(xml[*pos + 1] == quote_char) {
                value[value_len++] = quote_char;
                (*pos) += 2;  // Skip both quote characters
            } else {
                (*pos)++;  // Skip closing quote
                break;
            }
        } else {
            value[value_len++] = xml[(*pos)++];
        }
    }
    
    // Store attribute if we have both name and value
    if(name_len > 0 && value_len >= 0) {
        name[name_len] = '\0';
        value[value_len] = '\0';
        xml_set_attribute(element, xml_trim(name), value);
        return 1;
    }
    
    return 0;
}

static int xml_parse_tag(const char* xml, int pos, char* tag, int* is_closing, XMLElement* element) {
    int i = 0;

    pos++; // Skip '<'
    if (xml[pos] == '/') {
        *is_closing = 1;
        pos++;
    } else {
        *is_closing = 0;
        element->attr_count = 0; // Reset attributes for new tag
    }

    // Get tag name
    while (xml[pos] != '>' && xml[pos] != ' ' && xml[pos] != '/' && xml[pos] != '\0' && i < XML_MAX_TAG_LENGTH - 1) {
        tag[i++] = xml[pos++];
    }
    tag[i] = '\0';

    // Parse attributes if this is an opening tag
    if (!*is_closing) {
        while (xml[pos] != '>' && xml[pos] != '/' && xml[pos] != '\0') {
            if (!xml_parse_attribute(xml, &pos, element)) {
                pos++; // Skip any problematic characters
            }
        }
    }

    // Skip to end of tag
    while (xml[pos] != '>' && xml[pos] != '\0') pos++;
    return xml[pos] == '\0' ? -1 : pos + 1;
}

XMLDocument* xml_parse_string(const char* xml) {
    XMLDocument* doc = (XMLDocument*)malloc(sizeof(XMLDocument));
    int pos = 0;
    int current_element = -1;
    int stack[XML_MAX_DEPTH];
    int stack_pos = 0;

    doc->count = 0;
    doc->error[0] = '\0';

    while (xml[pos] != '\0') {
        if (xml[pos] == '<') {
            char tag[XML_MAX_TAG_LENGTH];
            int is_closing;
            XMLElement* current = NULL;

            // For opening tags, prepare the current element
            if (xml[pos + 1] != '/') {
                current_element = doc->count;
                current = &doc->elements[current_element];
                current->attr_count = 0;  // Initialize attribute count
            }

            int new_pos = xml_parse_tag(xml, pos, tag, &is_closing, current);

            if (new_pos == -1) {
                xml_set_error(doc, XML_ERROR_MALFORMED);
                return doc;
            }

            if (!is_closing) {
                // Opening tag
                if (doc->count >= XML_MAX_ELEMENTS) {
                    xml_set_error(doc, XML_ERROR_OVERFLOW);
                    return doc;
                }

                strcpy(doc->elements[current_element].tag, tag);
                doc->elements[current_element].value[0] = '\0';
                doc->elements[current_element].has_children = 0;
                doc->elements[current_element].level = stack_pos;
                if (stack_pos > 0) {
                    doc->elements[current_element].parent_index = stack[stack_pos - 1];
                    doc->elements[stack[stack_pos - 1]].has_children = 1;
                } else {
                    doc->elements[current_element].parent_index = -1;
                }

                if (stack_pos >= XML_MAX_DEPTH) {
                    xml_set_error(doc, XML_ERROR_DEPTH);
                    return doc;
                }
                stack[stack_pos++] = current_element;
                doc->count++;
            } else {
                // Closing tag
                if (stack_pos <= 0 || strcmp(doc->elements[stack[stack_pos - 1]].tag, tag) != 0) {
                    xml_set_error(doc, XML_ERROR_MISMATCH);
                    return doc;
                }
                stack_pos--;
            }
            pos = new_pos;
        } else {
            // Text content
            if (current_element >= 0 && !doc->elements[current_element].has_children) {
                int i = 0;
                while (xml[pos] != '<' && xml[pos] != '\0' && i < XML_MAX_VALUE_LENGTH - 1) {
                    doc->elements[current_element].value[i++] = xml[pos++];
                }
                doc->elements[current_element].value[i] = '\0';
                xml_trim(doc->elements[current_element].value);
            } else {
                pos++;
            }
        }
    }

    if (stack_pos != 0) {
        xml_set_error(doc, XML_ERROR_UNCLOSED);
    }

    return doc;
}

void xml_free_document(XMLDocument* doc) {
    free(doc);
}

char* xml_get_error(XMLDocument* doc) {
    return doc->error;
}

static int found_elements[XML_MAX_ELEMENTS];
static int found_count = 0;

int xml_find_elements(XMLDocument* doc, const char* tag, char** results, int max_results) {
    int i, j, count = 0;
    char buffer[XML_MAX_VALUE_LENGTH];
    XMLElement* elem;

    found_count = 0;  // Reset found elements
    
    for (i = 0; i < doc->count && count < max_results; i++) {
        if (strcmp(doc->elements[i].tag, tag) == 0) {
            elem = &doc->elements[i];
            found_elements[count] = i;  // Store the element index
            
            // Start with tag name
            snprintf(buffer, sizeof(buffer), "%s", elem->tag);
            
            // Add value if present
            if (elem->value[0] != '\0') {
                size_t len = strlen(buffer);
                snprintf(buffer + len, sizeof(buffer) - len, "=%s", elem->value);
            }
            
            // Add attributes if present
            for (j = 0; j < elem->attr_count; j++) {
                size_t len = strlen(buffer);
                snprintf(buffer + len, sizeof(buffer) - len, " %s=%s",
                        elem->attributes[j].name,
                        elem->attributes[j].value);
            }
            
            results[count] = strdup(buffer);
            count++;
        }
    }
    
    found_count = count;
    return count;
}

char* xml_build_document(const char* root, const char** elements, int count) {
    static char buffer[XML_MAX_VALUE_LENGTH * 2];
    int i,pos = 0;

    // Opening root tag
    pos += snprintf(buffer + pos, sizeof(buffer) - pos, "<%s>", root);

    // Add elements
    for (i = 0; i < count; i += 2) {
        if (i + 1 < count) {
            pos += snprintf(buffer + pos, sizeof(buffer) - pos,
                            "<%s>%s</%s>",
                            elements[i], elements[i + 1], elements[i]);
        }
    }

    // Closing root tag
    pos += snprintf(buffer + pos, sizeof(buffer) - pos, "</%s>", root);

    return buffer;
}

int xml_get_attribute(XMLElement* element, const char* attr_name, char* value, size_t max_length) {
    int i;
    if (!element || !attr_name || !value || max_length == 0) {
        return -1;
    }

    for (i = 0; i < element->attr_count; i++) {
        if (strcmp(element->attributes[i].name, attr_name) == 0) {
            if (strlen(element->attributes[i].value) >= max_length) {
                return -1;  // Buffer too small
            }
            strcpy(value, element->attributes[i].value);
            return 0;
        }
    }
    return -1;  // Attribute not found
}

int xml_set_attribute(XMLElement* element, const char* attr_name, const char* attr_value) {
    int i;
    if (!element || !attr_name || !attr_value) {
        return -1;
    }

    // First, check if attribute already exists
    for (i = 0; i < element->attr_count && i < XML_MAX_ATTRIBUTES; i++) {
        if (strcmp(element->attributes[i].name, attr_name) == 0) {
            if (strlen(attr_value) >= XML_MAX_ATTR_VALUE) {
                return -1;  // Value too long
            }
            strncpy(element->attributes[i].value, attr_value, XML_MAX_ATTR_VALUE - 1);
            element->attributes[i].value[XML_MAX_ATTR_VALUE - 1] = '\0';
            return 0;
        }
    }

    // If we get here, we need to add a new attribute
    if (element->attr_count >= XML_MAX_ATTRIBUTES) {
        return -1;  // Too many attributes
    }

    if (strlen(attr_name) >= XML_MAX_ATTR_NAME || 
        strlen(attr_value) >= XML_MAX_ATTR_VALUE) {
        return -1;  // Name or value too long
    }

    strncpy(element->attributes[element->attr_count].name, attr_name, XML_MAX_ATTR_NAME - 1);
    strncpy(element->attributes[element->attr_count].value, attr_value, XML_MAX_ATTR_VALUE - 1);
    element->attributes[element->attr_count].name[XML_MAX_ATTR_NAME - 1] = '\0';
    element->attributes[element->attr_count].value[XML_MAX_ATTR_VALUE - 1] = '\0';
    element->attr_count++;
    return 0;
}

int xml_remove_attribute(XMLElement* element, const char* attr_name) {
    int i,j;
    if (!element || !attr_name) {
        return -1;
    }

    for (i = 0; i < element->attr_count; i++) {
        if (strcmp(element->attributes[i].name, attr_name) == 0) {
            // Shift remaining attributes left
            for (j = i; j < element->attr_count - 1; j++) {
                strcpy(element->attributes[j].name, element->attributes[j + 1].name);
                strcpy(element->attributes[j].value, element->attributes[j + 1].value);
            }
            element->attr_count--;
            return 0;
        }
    }
    return -1;  // Attribute not found
}

int xml_get_attribute_count(XMLElement* element) {
    if (!element) {
        return -1;
    }
    return element->attr_count;
}

int xml_get_attribute_at(XMLElement* element, int index, char* name, char* value, size_t max_length) {
    if (!element || !name || !value || index < 0 || index >= element->attr_count || max_length == 0) {
        return -1;
    }

    if (strlen(element->attributes[index].name) >= max_length ||
        strlen(element->attributes[index].value) >= max_length) {
        return -1;  // Buffer too small
    }

    strcpy(name, element->attributes[index].name);
    strcpy(value, element->attributes[index].value);
    return 0;
}

/* Find element and attribute by qualified path (e.g., "product.price") */
int xml_find_element_by_path(XMLDocument* doc, const char* path, char* attr_name, int instance) {
    char path_copy[XML_MAX_PATH_LENGTH];
    char *token, *saveptr, *last_token = NULL;
    int i, current_index = 0;
    
    if (!doc || !path || strlen(path) >= XML_MAX_PATH_LENGTH) {
        return -1;
    }
    
    strncpy(path_copy, path, XML_MAX_PATH_LENGTH - 1);
    path_copy[XML_MAX_PATH_LENGTH - 1] = '\0';
    
    // Get first token
    token = strtok_r(path_copy, ".", &saveptr);
    if (!token) return -1;
    
    // Keep track of the last token while processing
    while (token) {
        last_token = token;
        token = strtok_r(NULL, ".", &saveptr);
        
        if (token) { // Not the last token, process as element
            int found = 0;
            int count = 0;
            int start_idx = (current_index == 0) ? 0 : current_index + 1;
            
            for (i = start_idx; i < doc->count; i++) {
                if ((current_index == 0 && doc->elements[i].level == 0) ||
                    doc->elements[i].parent_index == current_index) {
                    if (strcmp(doc->elements[i].tag, last_token) == 0) {
                        count++;
                        if (count == instance) {
                            current_index = i;
                            found = 1;
                            break;
                        }
                    }
                }
            }
            
            if (!found) {
                return -1;
            }
        }
    }
    
    // Copy the last token as the attribute name
    if (last_token) {
        strncpy(attr_name, last_token, XML_MAX_ATTR_NAME - 1);
        attr_name[XML_MAX_ATTR_NAME - 1] = '\0';
    }
    
    return current_index;
}

/* XML Cleaning Helper Functions */
const char* remove_xml_declaration(const char* input) {
    if (strncmp(input, "<?xml", 5) == 0) {
        const char* decl_end = strstr(input, "?>");
        if (decl_end) {
            return decl_end + 2;
        }
    }
    return input;
}

void handle_open_tag(char c, int* inside_tag, int* collecting_tag, char* tag_name, size_t* tag_pos) {
    *inside_tag = 1;
    *collecting_tag = 1;
    *tag_pos = 0;
    memset(tag_name, 0, XML_MAX_TAG_NAME);
}

size_t handle_self_closing(const char* input, char* output, size_t out_pos, size_t max_output, 
                          const char* tag_name, size_t tag_pos, int* inside_tag, int* collecting_tag) {
    if (*(input + 1) == '>') {
        if (out_pos + tag_pos + 4 < max_output) {
            output[out_pos++] = '>';
            output[out_pos++] = '<';
            output[out_pos++] = '/';
            strncpy(output + out_pos, tag_name, tag_pos);
            out_pos += tag_pos;
            output[out_pos++] = '>';
        }
        *inside_tag = 0;
        *collecting_tag = 0;
    }
    else {
        output[out_pos++] = '/';
    }
    return out_pos;
}

void handle_close_tag(char c, int* inside_tag, int* collecting_tag) {
    *inside_tag = 0;
    *collecting_tag = 0;
}

size_t handle_whitespace(char c, char last_char, int inside_tag, char* output, size_t out_pos) {
    if (!inside_tag && last_char != '>') {
        output[out_pos++] = ' ';
    }
    if (inside_tag) {
        output[out_pos++] = c;
    }
    return out_pos;
}

size_t handle_character(char c, char* output, size_t out_pos, char* tag_name, 
                       size_t* tag_pos, int* collecting_tag) {
    output[out_pos++] = c;
    if (*collecting_tag && *tag_pos < XML_MAX_TAG_NAME - 1) {
        if (is_valid_tag_char(c)) {
            tag_name[*tag_pos] = c;
            (*tag_pos)++;
        }
        else if (c == ' ') {
            *collecting_tag = 0;
        }
    }
    return out_pos;
}

int is_valid_tag_char(char c) {
    return (isalnum(c) || c == '_' || c == '-');
}

void normalize_whitespace(char* text) {
    char* read = text;
    char* write = text;
    int space_pending = 0;
    
    while (*read) {
        if (isspace(*read)) {
            space_pending = 1;
        }
        else {
            if (space_pending && write > text) {
                *write++ = ' ';
            }
            *write++ = *read;
            space_pending = 0;
        }
        read++;
    }
    *write = '\0';
}

char* clean_xml(const char* input, char* output, size_t max_output) {
    size_t out_pos = 0;
    int inside_tag = 0;
    char last_char = 0;
    char tag_name[XML_MAX_TAG_NAME] = {0};
    size_t tag_pos = 0;
    int collecting_tag = 0;
    
    const char* xml_start = remove_xml_declaration(input);
    const char* p;
    for (p = xml_start; *p && out_pos < max_output - 1; p++) {
        char c = *p;
        
        if (c == '<') {
            handle_open_tag(c, &inside_tag, &collecting_tag, tag_name, &tag_pos);
            output[out_pos++] = c;
        }
        else if (c == '/' && inside_tag) {
            out_pos = handle_self_closing(p, output, out_pos, max_output, 
                                        tag_name, tag_pos, &inside_tag, &collecting_tag);
            if (*(p + 1) == '>') p++;
        }
        else if (c == '>') {
            handle_close_tag(c, &inside_tag, &collecting_tag);
            output[out_pos++] = c;
        }
        else if (isspace(c)) {
            out_pos = handle_whitespace(c, last_char, inside_tag, output, out_pos);
            collecting_tag = 0;
        }
        else {
            out_pos = handle_character(c, output, out_pos, tag_name, &tag_pos, &collecting_tag);
        }
        
        if (!isspace(c)) {
            last_char = c;
        }
    }
    
    output[out_pos] = '\0';
    normalize_whitespace(output);
    
    return output;
}

/**
 * XML Processing Interface (rxml.c)
 * Using simple XML parser implementation
 */

/* XML processing flags and globals */
static XMLDocument* current_doc = NULL;

#define XML_DEBUG(msg) if(xml_debug) printf("RXML: %s\n", msg)

/* Set XML processing flags */
PROCEDURE(xmlflags) {
    char *flags = GETSTRING(ARG0);

    if (strstr(flags, "RESET")) {
        xml_debug = 0;
        RETURNINTX(0);
    }

    if (strstr(flags, "DEBUG")) xml_debug = 1;
    if (strstr(flags, "NDEBUG")) xml_debug = 0;

    RETURNINTX(0);
    ENDPROC
}

/* Parse XML string */
PROCEDURE(xmlparse) {
    char *xml_string = GETSTRING(ARG0);
    char xml_cleansed[XML_MAX_OUTPUT];

    // Cleanup any previous document
    if (current_doc) {
        xml_free_document(current_doc);
        current_doc = NULL;
    }

    clean_xml(xml_string, xml_cleansed, XML_MAX_OUTPUT);
    // Parse XML string
    current_doc = xml_parse_string(xml_cleansed);
    if (current_doc->error[0] != '\0') {
        RETURNINTX(-1);
    }

    XML_DEBUG("XML parsed successfully");
    RETURNINTX(0);
    ENDPROC
}

/* Find XML elements by tag name */
PROCEDURE(xmlfind) {
    char *tag = GETSTRING(ARG0);
    char *results[XML_MAX_ELEMENTS];
    int i, count;

    if (!current_doc) {
        RETURNINTX(0);
    }

    count = xml_find_elements(current_doc, tag, results, XML_MAX_ELEMENTS);

    SETARRAYHI(ARG1, count);
    for (i = 0; i < count; i++) {
        SETSARRAY(ARG1, i, results[i]);
        free(results[i]);  // Free the allocated string
    }

    RETURNINTX(count);
    ENDPROC
}

/* Build XML from structured data */
PROCEDURE(xmlbuild) {
    char *root_name = GETSTRING(ARG0);
    int i,count = GETARRAYHI(ARG1);
    const char** elements = malloc(count * sizeof(char*));
    char* result;

    for (i = 0; i < count; i++) {
        elements[i] = GETSARRAY(ARG1, i);
    }

    result = xml_build_document(root_name, elements, count);
    free(elements);

    RETURNSTRX(result);
    ENDPROC
}

/* Get last error message */
PROCEDURE(xmlerror) {
    if (current_doc) {
        RETURNSTRX(xml_get_error(current_doc));
    }
    RETURNSTRX("");
    ENDPROC
}

/* Cleanup on unload */
void cleanup(void) {
    if (current_doc) {
        xml_free_document(current_doc);
        current_doc = NULL;
    }
}

static int validate_xml_state(XMLDocument* doc, int instance) {
    if (!doc) {
        xml_set_error(doc, XML_ERROR_NO_DOC);
        return -1;
    }
    
    if (doc->count == 0) {
        xml_set_error(doc, XML_ERROR_NO_FIND);
        return -1;
    }
    
    if (instance < 1 || instance > doc->count) {
        xml_set_error(doc, XML_ERROR_INVALID_INST);
        return -1;
    }
    
    return 0;
}

/* Get attribute value by name */
PROCEDURE(xmlgetattr) {
    char *attr_name = GETSTRING(ARG0);
    int instance = GETINT(ARG1);
    XMLElement* element;
    int i;
    
    if (validate_xml_state(current_doc, instance) == -1) {
        RETURNSTRX("");
    }
    
    element = &current_doc->elements[found_elements[instance - 1]];
    for (i = 0; i < element->attr_count; i++) {
        if (strcmp(element->attributes[i].name, attr_name) == 0) {
            RETURNSTRX(element->attributes[i].value);
        }
    }
    
    RETURNSTRX("");
    ENDPROC
}

/* Set attribute value */
PROCEDURE(xmlsetattr) {
    char *attr_name = GETSTRING(ARG0);
    int instance = GETINT(ARG1);
    char *value = GETSTRING(ARG2);
    XMLElement* element;
    
    if (validate_xml_state(current_doc, instance) == -1) {
        RETURNINTX(-1);
    }
    
    element = &current_doc->elements[found_elements[instance - 1]];
    RETURNINTX(xml_set_attribute(element, attr_name, value));
    ENDPROC
}

/* Remove attribute */
PROCEDURE(xmlremattr) {
    char *attr_name = GETSTRING(ARG0);
    int instance = GETINT(ARG1);
    XMLElement* element;
    
    if (validate_xml_state(current_doc, instance) == -1) {
        RETURNINTX(-1);
    }
    
    element = &current_doc->elements[found_elements[instance - 1]];
    RETURNINTX(xml_remove_attribute(element, attr_name));
    ENDPROC
}

/* Get attribute count */
PROCEDURE(xmlattrcount) {
    int instance = GETINT(ARG0);
    XMLElement* element;
    
    if (validate_xml_state(current_doc, instance) == -1) {
        RETURNINTX(-1);
    }
    
    element = &current_doc->elements[found_elements[instance - 1]];
    RETURNINTX(xml_get_attribute_count(element));
    ENDPROC
}

/* Get attribute at index */
PROCEDURE(xmlattrat) {
    int instance = GETINT(ARG0);
    int attr_index = GETINT(ARG1);
    XMLElement* element;
    char name[XML_MAX_ATTR_NAME];
    char value[XML_MAX_ATTR_VALUE];
    
    if (validate_xml_state(current_doc, instance) == -1) {
        RETURNINTX(-1);
    }
    
    element = &current_doc->elements[found_elements[instance - 1]];
    if (xml_get_attribute_at(element, attr_index, name, value, XML_MAX_ATTR_VALUE) == 0) {
        SETSTRING(ARG2, name);
        SETSTRING(ARG3, value);
        RETURNINTX(0);
    }
    
    RETURNINTX(-1);
    ENDPROC
}

/* Add REXX procedure for XML cleaning */
PROCEDURE(xmlclean) {
    char* input = GETSTRING(ARG0);
    char output[XML_MAX_OUTPUT];
    
    clean_xml(input, output, XML_MAX_OUTPUT);
    RETURNSTRX(output);
    ENDPROC
}

/* Function Registration */
LOADFUNCS
    ADDPROC(xmlflags, "rxml.xmlflags",  "b", ".int",    "flags=.string");
    ADDPROC(xmlparse, "rxml.xmlparse",  "b", ".int",    "xml=.string");
    ADDPROC(xmlfind,  "rxml.xmlfind",   "b", ".int",    "tag=.string, expose results=.string[]");
    ADDPROC(xmlbuild, "rxml.xmlbuild",  "b", ".string", "root=.string, expose elements=.string[]");
    ADDPROC(xmlerror, "rxml.xmlerror",  "b", ".string", "");
    ADDPROC(xmlgetattr, "rxml.xmlgetattr", "b", ".string", "attr_name=.string,instance=.int");
    ADDPROC(xmlsetattr, "rxml.xmlsetattr", "b", ".int", "attr_name=.string,instance=.int,value=.string");
    ADDPROC(xmlremattr, "rxml.xmlremattr", "b", ".int", "attr_name=.string,instance=.int");
    ADDPROC(xmlattrcount, "rxml.xmlattrcount", "b", ".int", "instance=.int");
    ADDPROC(xmlattrat, "rxml.xmlattrat", "b", ".int", "instance=.int,index=.int,expose name=.string,expose value=.string");
ENDLOADFUNCS
