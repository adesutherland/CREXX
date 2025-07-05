/**
 * Simple XML Parser Header
 */
#ifndef SIMPLE_XML_H
#define SIMPLE_XML_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifndef CREXX_RXML_H
#define CREXX_RXML_H

#define XML_MAX_ELEMENTS 1000
#define XML_MAX_TAG_LENGTH 256
#define XML_MAX_VALUE_LENGTH 1024
#define XML_MAX_DEPTH 32
#define XML_MAX_ATTRIBUTES 32
#define XML_MAX_ATTR_NAME 256
#define XML_MAX_ATTR_VALUE 1024
#define XML_MAX_PATH_LENGTH 256

typedef struct {
    char name[XML_MAX_ATTR_NAME];
    char value[XML_MAX_ATTR_VALUE];
} XMLAttribute;

typedef struct {
    char tag[XML_MAX_TAG_LENGTH];
    char value[XML_MAX_VALUE_LENGTH];
    XMLAttribute attributes[XML_MAX_ATTRIBUTES];
    int attr_count;
    int has_children;
    int level;
    int parent_index;
} XMLElement;

typedef struct {
    XMLElement elements[XML_MAX_ELEMENTS];
    int count;
    char error[XML_MAX_VALUE_LENGTH];
} XMLDocument;

#endif //CREXX_RXML_H

// Parser functions
XMLDocument* xml_parse_string(const char* xml);
void xml_free_document(XMLDocument* doc);
char* xml_get_error(XMLDocument* doc);
int xml_find_elements(XMLDocument* doc, const char* tag, char** results, int max_results);
char* xml_build_document(const char* root, const char** elements, int count);
int xml_get_attribute(XMLElement* element, const char* attr_name, char* value, size_t max_length);
int xml_set_attribute(XMLElement* element, const char* attr_name, const char* attr_value);
int xml_remove_attribute(XMLElement* element, const char* attr_name);
int xml_get_attribute_count(XMLElement* element);
int xml_get_attribute_at(XMLElement* element, int index, char* name, char* value, size_t max_length);

#endif // SIMPLE_XML_H

#ifndef RXML_CLEAN_H
#define RXML_CLEAN_H

#include <string.h>
#include <ctype.h>

/* Maximum sizes for XML processing */
#define XML_MAX_TAG_NAME 256
#define XML_MAX_OUTPUT 16384

/* Main cleaning function */
char* clean_xml(const char* input, char* output, size_t max_output);

/* Helper functions */
const char* remove_xml_declaration(const char* input);
void handle_open_tag(char c, int* inside_tag, int* collecting_tag, char* tag_name, size_t* tag_pos);
size_t handle_self_closing(const char* input, char* output, size_t out_pos, size_t max_output, 
                          const char* tag_name, size_t tag_pos, int* inside_tag, int* collecting_tag);
void handle_close_tag(char c, int* inside_tag, int* collecting_tag);
size_t handle_whitespace(char c, char last_char, int inside_tag, char* output, size_t out_pos);
size_t handle_character(char c, char* output, size_t out_pos, char* tag_name, 
                       size_t* tag_pos, int* collecting_tag);
int is_valid_tag_char(char c);
void normalize_whitespace(char* text);

#endif /* RXML_CLEAN_H */ 