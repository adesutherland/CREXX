/**
 * Simple XML Parser Header
 */
#ifndef SIMPLE_XML_H
#define SIMPLE_XML_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define XML_MAX_TAG_LENGTH 256
#define XML_MAX_VALUE_LENGTH 1024
#define XML_MAX_ELEMENTS 100
#define XML_MAX_DEPTH 32

typedef struct {
    char tag[XML_MAX_TAG_LENGTH];
    char value[XML_MAX_VALUE_LENGTH];
    int has_children;
    int parent_index;
    int level;
} XMLElement;

typedef struct {
    XMLElement elements[XML_MAX_ELEMENTS];
    int count;
    char error[256];
} XMLDocument;

// Parser functions
XMLDocument* xml_parse_string(const char* xml);
void xml_free_document(XMLDocument* doc);
char* xml_get_error(XMLDocument* doc);
int xml_find_elements(XMLDocument* doc, const char* tag, char** results, int max_results);
char* xml_build_document(const char* root, const char** elements, int count);

#endif // SIMPLE_XML_H