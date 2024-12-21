/**
 * Simple XML Parser Implementation
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crexxpa.h"
#include "rxml.h"

static void xml_set_error(XMLDocument* doc, const char* error) {
    snprintf(doc->error, sizeof(doc->error), "%s", error);
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

static int xml_parse_tag(const char* xml, int pos, char* tag, int* is_closing) {
    int i = 0;
    pos++; // Skip '<'
    if (xml[pos] == '/') {
        *is_closing = 1;
        pos++;
    } else {
        *is_closing = 0;
    }

    while (xml[pos] != '>' && xml[pos] != ' ' && xml[pos] != '\0' && i < XML_MAX_TAG_LENGTH - 1) {
        tag[i++] = xml[pos++];
    }
    tag[i] = '\0';

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
            int new_pos = xml_parse_tag(xml, pos, tag, &is_closing);

            if (new_pos == -1) {
                xml_set_error(doc, "Malformed XML tag");
                return doc;
            }

            if (!is_closing) {
                // Opening tag
                if (doc->count >= XML_MAX_ELEMENTS) {
                    xml_set_error(doc, "Too many elements");
                    return doc;
                }

                current_element = doc->count++;
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
                    xml_set_error(doc, "XML nesting too deep");
                    return doc;
                }
                stack[stack_pos++] = current_element;
            } else {
                // Closing tag
                if (stack_pos <= 0 || strcmp(doc->elements[stack[stack_pos - 1]].tag, tag) != 0) {
                    xml_set_error(doc, "Mismatched closing tag");
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
        xml_set_error(doc, "Unclosed tags");
    }

    return doc;
}

void xml_free_document(XMLDocument* doc) {
    free(doc);
}

char* xml_get_error(XMLDocument* doc) {
    return doc->error;
}

int xml_find_elements(XMLDocument* doc, const char* tag, char** results, int max_results) {
    int i,count = 0;

    for (i = 0; i < doc->count && count < max_results; i++) {
        if (strcmp(doc->elements[i].tag, tag) == 0) {
            results[count++] = doc->elements[i].value;
        }
    }

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
/**
 * XML Processing Interface (rxml.c)
 * Using simple XML parser implementation
 */

/* XML processing flags and globals */
static int xml_debug = 0;
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

    // Cleanup any previous document
    if (current_doc) {
        xml_free_document(current_doc);
        current_doc = NULL;
    }

    // Parse XML string
    current_doc = xml_parse_string(xml_string);
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
    char* results[XML_MAX_ELEMENTS];
    int i,count;

    if (!current_doc) {
        RETURNINTX(-1);
    }

    count = xml_find_elements(current_doc, tag, results, XML_MAX_ELEMENTS);

    SETARRAYHI(ARG1, count);
    for (i = 0; i < count; i++) {
        SETSARRAY(ARG1, i, results[i]);
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

/* Function Registration */
LOADFUNCS
    ADDPROC(xmlflags, "rxml.xmlflags",  "b", ".int",    "flags=.string");
    ADDPROC(xmlparse, "rxml.xmlparse",  "b", ".int",    "xml=.string");
    ADDPROC(xmlfind,  "rxml.xmlfind",   "b", ".int",    "tag=.string,expose results=.string[]");
    ADDPROC(xmlbuild, "rxml.xmlbuild",  "b", ".string", "root=.string,expose elements=.string[]");
    ADDPROC(xmlerror, "rxml.xmlerror",  "b", ".string","");
ENDLOADFUNCS