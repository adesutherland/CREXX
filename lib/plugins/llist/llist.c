//
// System Information Plugin for crexx/pa - Plugin Architecture
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>   // For POSIX systems (Linux/macOS)
#endif


#define HIGHVALUE ((char *)-1)

// Memory tracking and debugging
#define MEMORY_MAGIC 0xDEADBEEF
#define MEMORY_PADDING 16
#define DEBUG_MEMORY 1

// Memory block header structure
struct MemoryHeader {
    unsigned int magic;          // Magic number to detect corruption
    size_t size;                // Size of allocated block
    const char* file;           // Source file of allocation
    int line;                   // Line number of allocation
    struct MemoryHeader* next;  // Linked list of allocations
    struct MemoryHeader* prev;  // Previous block
    unsigned char padding[MEMORY_PADDING]; // Padding to detect buffer overflows
};

// Global memory tracking
static struct {
    size_t total_allocated;
    size_t peak_allocated;
    size_t allocation_count;
    struct MemoryHeader* first_block;
    struct MemoryHeader* last_block;
} memory_stats = {0};

struct NodeStub {
    uintptr_t * sStackFirst;
    uintptr_t * sStackLast;
    uintptr_t * sStackCurrent;
    uintptr_t * sStackLastValid;
    int         sStackEntries;
};
struct NodeStub llEntryStub[99]={0};  // Array of head pointers

struct NodeEntry {
    unsigned int frontFence;    // Memory fence at front
    uintptr_t * sSaddr;        // self reference Pointer to the stack
    uintptr_t * sNext;
    uintptr_t * sPrev;
    char String[1];
    // backFence will be placed after the string
};

// Memory management functions
#ifdef _WIN32
    HANDLE hHeap = NULL;
    
    static void* internal_alloc(size_t size, const char* file, int line) {
        if (hHeap == NULL) {
            hHeap = GetProcessHeap();
            if (hHeap == NULL) {
                fprintf(stderr, "Failed to get process heap: %lu\n", GetLastError());
                return NULL;
            }
        }

        size_t total_size = size + sizeof(struct MemoryHeader);
        struct MemoryHeader* header = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, total_size);
        
        if (header == NULL) {
            fprintf(stderr, "Memory allocation failed: %lu\n", GetLastError());
            return NULL;
        }

        return header;
    }

    static void internal_free(void* ptr) {
        if (!HeapFree(hHeap, 0, ptr)) {
            fprintf(stderr, "Memory free failed: %lu\n", GetLastError());
        }
    }
#else
    static void* internal_alloc(size_t size, const char* file, int line) {
        size_t total_size = size + sizeof(struct MemoryHeader);
        struct MemoryHeader* header = calloc(1, total_size);
        
        if (header == NULL) {
            fprintf(stderr, "Memory allocation failed: %s\n", strerror(errno));
            return NULL;
        }

        return header;
    }

    static void internal_free(void* ptr) {
        free(ptr);
    }
#endif

// Enhanced memory allocation function
#define getmain(size) _getmain(size, __FILE__, __LINE__)
uintptr_t _getmain(size_t size, const char* file, int line) {
    struct MemoryHeader* header = internal_alloc(size, file, line);
    if (header == NULL) {
        return -8;
    }
    // Initialize header
    header->magic = MEMORY_MAGIC;
    header->size = size;
    header->file = file;
    header->line = line;
    memset(header->padding, 0xCC, MEMORY_PADDING); // Set padding pattern
    // Update memory tracking
    memory_stats.total_allocated += size;
    memory_stats.allocation_count++;
    if (memory_stats.total_allocated > memory_stats.peak_allocated) {
        memory_stats.peak_allocated = memory_stats.total_allocated;
    }
    // Add to tracking list
    header->prev = memory_stats.last_block;
    header->next = NULL;
    if (memory_stats.last_block) {
        memory_stats.last_block->next = header;
    } else {
        memory_stats.first_block = header;
    }
    memory_stats.last_block = header;

    // Return pointer to usable memory
    return (uintptr_t)(header + 1);
}
// Enhanced memory free function
int freemain(void* ptr) {
    if (ptr == NULL) return 0;
    int i;

    struct MemoryHeader* header = ((struct MemoryHeader*)ptr) - 1;

    // Validate memory block
    if (header->magic != MEMORY_MAGIC) {
        fprintf(stderr, "Memory corruption detected! Invalid magic number at %p\n", ptr);
        return -8;
    }

    // Check padding
    for (i = 0; i < MEMORY_PADDING; i++) {
        if (header->padding[i] != 0xCC) {
            fprintf(stderr, "Buffer overflow detected in block allocated at %s:%d\n",
                    header->file, header->line);
            break;
        }
    }

    // Update tracking list
    if (header->prev) {
        header->prev->next = header->next;
    } else {
        memory_stats.first_block = header->next;
    }
    if (header->next) {
        header->next->prev = header->prev;
    } else {
        memory_stats.last_block = header->prev;
    }

    // Update statistics
    memory_stats.total_allocated -= header->size;
    memory_stats.allocation_count--;

    // Clear magic to catch double-frees
    header->magic = 0;

    internal_free(header);
    return 0;
}

// Add these utility functions for memory debugging
void print_memory_stats(void) {
    printf("\nMemory Statistics:\n");
    printf("Current allocations: %zu bytes\n", memory_stats.total_allocated);
    printf("Peak allocation: %zu bytes\n", memory_stats.peak_allocated);
    printf("Active allocations: %zu\n", memory_stats.allocation_count);
}

void check_memory_leaks(void) {
    if (memory_stats.allocation_count > 0) {
        fprintf(stderr, "\nMemory Leaks Detected!\n");
        struct MemoryHeader* current = memory_stats.first_block;
        while (current) {
            fprintf(stderr, "Leak: %zu bytes allocated at %s:%d\n",
                    current->size, current->file, current->line);
            current = current->next;
        }
    }
}

void toUpperCase(char *str) {
    if (str == NULL) return; // Handle null pointer
    while (*str) {
        *str = toupper((unsigned char)*str); // Convert current character to uppercase
        str++;
    }
}

void trim(char *str) {
    char *start = str;  // Pointer auf den Anfang des Strings
    char *end;
    while (isspace((unsigned char)*start)) {   // skip leading blanks
        start++;
    }
    if (start != str) {            // shift chars to the beginning
        memmove(str, start, strlen(start) + 1);
    }
    end = str + strlen(str) - 1;   // set new end pointer
    while (end > str && isspace((unsigned char)*end)) { // skip trailing blanks
        *end = '\0'; // Nullify
        end--;
    }
}

// Add at the top with other defines
#define NODE_FENCE_VALUE 0xFE0CEFE


// Update createNodeEntry to initialize fences
struct NodeEntry* createNodeEntry(char *message) {
    size_t messageLen = strlen(message);
    size_t totalSize = sizeof(struct NodeEntry) + messageLen + 1 + sizeof(unsigned int);
    uintptr_t *nodeAddr = (uintptr_t *)getmain(totalSize);
    
    if (nodeAddr == NULL) {
        printf("Memory allocation failed\n");
        return (struct NodeEntry *)-8;
    }
    
    struct NodeEntry *newNode = (struct NodeEntry *)nodeAddr;
    newNode->frontFence = NODE_FENCE_VALUE;
    newNode->sSaddr = nodeAddr;
    
    strncpy(newNode->String, message, messageLen + 1);
    newNode->sNext = NULL;
    
    // Set back fence after string
    unsigned int *backFence = (unsigned int*)(newNode->String + messageLen + 1);
    *backFence = NODE_FENCE_VALUE;
    
    return newNode;
}

int addnode(int mode, int nodeIndx,char * message) {

    struct NodeEntry *newNode = createNodeEntry(message);  // Allocate storage for new entry

    if (llEntryStub[nodeIndx].sStackFirst == NULL) {       // First entry in the stack
        llEntryStub[nodeIndx].sStackFirst   = (uintptr_t *) newNode;  // Set first element
        llEntryStub[nodeIndx].sStackCurrent = (uintptr_t *) newNode;  // Set current pointer
        llEntryStub[nodeIndx].sStackLast    = (uintptr_t *) newNode;  // Set last element
        llEntryStub[nodeIndx].sStackEntries++;
        newNode->sSaddr = (uintptr_t *) newNode;           // Self-reference address
        newNode->sPrev = 0;                                // Initialize backward pointer
        newNode->sNext = 0;                                // Initialize forward pointer
        return 0;
    }

    struct NodeEntry *currentFirst = (struct NodeEntry *) llEntryStub[nodeIndx].sStackFirst;
    struct NodeEntry *currentLast  = (struct NodeEntry *) llEntryStub[nodeIndx].sStackLast;

    if (mode == 1) {  // APPEND MODE
        // Add node to the end of the list
        currentLast->sNext = (uintptr_t *) newNode;    // Forward pointer from last to new node
        newNode->sPrev = currentLast->sSaddr;         // Backward pointer to last node
        llEntryStub[nodeIndx].sStackLast = (uintptr_t *) newNode;  // Update last node
    }
    else {  // PREPEND MODE
        // Add node to the beginning of the list
        currentFirst->sPrev = (uintptr_t *) newNode;   // Backward pointer from first to new node
        newNode->sNext = currentFirst->sSaddr;        // Forward pointer to first node
        llEntryStub[nodeIndx].sStackFirst = (uintptr_t *) newNode; // Update first node
    }

    // Update common fields
    llEntryStub[nodeIndx].sStackEntries++;
    newNode->sSaddr = (uintptr_t *) newNode;  // Create self-reference address
    newNode->sPrev = (mode == 1) ? currentLast->sSaddr : 0;  // Backward pointer
    newNode->sNext = (mode == 1) ? 0 : currentFirst->sSaddr; // Forward pointer

    return 0;
}

// Add this to your cleanup/exit code
PROCEDURE(cleanup) {
        print_memory_stats();
        check_memory_leaks();
        RETURNINT(0);
        ENDPROC
}

PROCEDURE(appendnode) {
    int    nodeIndx = GETINT(ARG0);
    char * message  = GETSTRING(ARG1);
    int rc=addnode(1, nodeIndx, message);
    RETURNINTX((rc))
ENDPROC
}

PROCEDURE(prependnode) {
    int    nodeIndx = GETINT(ARG0);
    char * message  = GETSTRING(ARG1);
    int rc=addnode(0, nodeIndx, message);
    RETURNINTX(rc);
    ENDPROC
}

PROCEDURE(insertnode) {
    // Extract input arguments
    int nodeIndex = GETINT(ARG0);
    char *message = GETSTRING(ARG1);
    char *mode = GETSTRING(ARG2);
    int before = 0;

 // Allocate memory for the new node
    struct NodeEntry *newNode = createNodeEntry(message);

 // Normalize mode to uppercase
    toUpperCase(mode);
    before = (mode[0] == 'B') ? 1 : 0;

 // Handle case where the stack is empty
    if (llEntryStub[nodeIndex].sStackFirst == NULL) {
        llEntryStub[nodeIndex].sStackFirst     = (uintptr_t *)newNode;   // Set first node
        llEntryStub[nodeIndex].sStackCurrent   = (uintptr_t *)newNode;   // Set current node
    }
 // Prepare pointers for insertion
    struct NodeEntry *current = (struct NodeEntry *)llEntryStub[nodeIndex].sStackCurrent; // Current node
    struct NodeEntry *new = newNode;                                                      // New node

 // Insert the new node into an empty list
    if (current == NULL) {              // First element: reset previous pointer
        new->sPrev = 0;
        new->sNext = 0;
    } else {                            // Queue already contains elements
        if (before==0) goto addAfter;  // add item after current item  !! use goto to have a readable structure
        else goto addPrior;            // add item prior current item  !! use goto to have a readable structure
    }
inserted: ;
 // Update stack metadata
    llEntryStub[nodeIndex].sStackEntries++;
    new->sSaddr = (uintptr_t *)newNode; // Self-reference for the new node
    llEntryStub[nodeIndex].sStackCurrent   = (uintptr_t *)newNode;   // Set new current node
RETURNINTX(0);
/* ------------------------------------------------------------------------------------------------
 * add item after current item  !! use goto to have a readable structure
 * ------------------------------------------------------------------------------------------------
 */
 addAfter: {
    struct NodeEntry *oldNext;
    uintptr_t oldPrev;
    oldNext = (struct NodeEntry *) current->sNext;
    current->sNext = (uintptr_t *) newNode; // Link current to new node
    if (oldNext > 0) {
       oldPrev = (uintptr_t) oldNext->sPrev;
       oldNext->sPrev = (uintptr_t *) newNode; // Link next node back to new node
       new->sNext = (uintptr_t *) oldNext;     // Link new node forward to old next node
    } else {
        new->sNext = 0;                        // set to zero, it's the last item
    }
    new->sPrev = (uintptr_t *) oldPrev;       // Link new node back to old previous node
    goto inserted;
}
/* ------------------------------------------------------------------------------------------------
 * add item prior current item  !! use goto to have a readable structure
 * ------------------------------------------------------------------------------------------------
 */
addPrior: {
    struct NodeEntry *oldPrev;
    uintptr_t oldNext;
    oldPrev = (struct NodeEntry *) current->sPrev;
    current->sPrev = (uintptr_t *) newNode;     // Link current to new node
    if (oldPrev > 0) {
        oldNext = (uintptr_t) oldPrev->sNext;
        oldPrev->sNext = (uintptr_t *) newNode; // Link next node back to new node
        new->sPrev = (uintptr_t *) oldPrev;     // Link new node back to old previous node
        new->sNext = (uintptr_t *) oldNext;     // Link new node forward to old next node
    } else {                                    // this is a new first entry
        new->sPrev = 0;                         // set to zero, it's the first item
        new->sNext = (uintptr_t *) current;
        llEntryStub[nodeIndex].sStackFirst = (uintptr_t *) newNode;
    }
    goto inserted;
}
ENDPROC
}

PROCEDURE(removenode) {
    int nodeIndex = GETINT(ARG0);

    // Check if the list (stack) is empty
    if (llEntryStub[nodeIndex].sStackFirst == NULL) {
        RETURNINTX(-8);  // Stack is empty
    }

    // Get the current node
    struct NodeEntry *current = (struct NodeEntry *)llEntryStub[nodeIndex].sStackCurrent;

    // Check if the current node is valid
    if (current == NULL) {
        RETURNINTX(-8);  // Invalid current node
    }

    // Extract neighboring nodes
    struct NodeEntry *next     = (struct NodeEntry *)current->sNext;
    struct NodeEntry *previous = (struct NodeEntry *)current->sPrev;

    // Update pointers for the next node
    if (next != NULL) {
        next->sPrev = (previous != NULL) ? previous->sSaddr : 0;  // Link next node back to previous
    } else {
        // Current is the last node; adjust the "last" pointer
        llEntryStub[nodeIndex].sStackLast = (uintptr_t *)previous;
    }

    // Update pointers for the previous node
    if (previous != NULL) {
        previous->sNext = (next != NULL) ? next->sSaddr : 0;  // Link previous node forward to next
        llEntryStub[nodeIndex].sStackCurrent = (uintptr_t *)previous;  // Move current pointer to previous node
    } else {
        // Current is the first node; adjust the "first" pointer
        llEntryStub[nodeIndex].sStackFirst   = (uintptr_t *)next;
        llEntryStub[nodeIndex].sStackCurrent = (uintptr_t *)next;  // Move current pointer to next node
    }

    // Free memory associated with the current node
    freemain(current->sSaddr);

    // Decrement the stack entry count
    llEntryStub[nodeIndex].sStackEntries--;

    RETURNINTX(0);
ENDPROC
}

// Function to display a linked list
PROCEDURE(listnode) {
    int nodeindx=GETINT(ARG0);
    printf("Linked List %d:\n",GETINT(ARG0));
    if (llEntryStub[nodeindx].sStackFirst== NULL) {
        printf("Empty list\n");
        RETURNINTX(0);
    }
    struct NodeEntry * temp = (struct NodeEntry *) llEntryStub[nodeindx].sStackFirst;
    while (temp != NULL) {
        printf("%s -> ", temp->String);
        temp = (struct NodeEntry *) temp->sNext;
    }
    printf("NULL\n");
    RETURNINTX(0);
ENDPROC
}

// Function to free memory of a linked list
PROCEDURE(freellist) {
    int nodeIndex = GETINT(ARG0);

    // Check if the list is already empty
    if (llEntryStub[nodeIndex].sStackFirst == NULL) {
        printf("List is already empty.\n");
        return;
    }

    // Initialize pointers for traversal
    struct NodeEntry *current = (struct NodeEntry *)llEntryStub[nodeIndex].sStackFirst;
    struct NodeEntry *nextNode;

    // Traverse and free all nodes
    while (current != NULL) {
        nextNode = (struct NodeEntry *)current->sNext;  // Save the next node
        freemain(current);                              // Free the current node
        current = nextNode;                             // Move to the next node
    }

    // Reset the list metadata
    llEntryStub[nodeIndex].sStackFirst   = NULL;
    llEntryStub[nodeIndex].sStackLast    = NULL;
    llEntryStub[nodeIndex].sStackCurrent = NULL;
    llEntryStub[nodeIndex].sStackLastValid = NULL;
    llEntryStub[nodeIndex].sStackEntries = 0;

    printf("List successfully freed.\n");
    RETURNINT(0);
}


PROCEDURE(nextnode) {
    int qname = GETINT(ARG0);

    // Retrieve the current node
    struct NodeEntry *currentEntry = (struct NodeEntry *)llEntryStub[qname].sStackCurrent;

    // Move to the next node
    currentEntry = (struct NodeEntry *)currentEntry->sNext;

    // Check if we've reached the end of the list
    if (currentEntry == NULL) {
        char endOfListMessage[32];
        sprintf(endOfListMessage, "$END-OF-LLIST-%d$", qname);
        RETURNSTR(endOfListMessage);
    }

    // Return the string stored in the current node
    RETURNSTR(currentEntry->String);

    // Update the current node pointer
    llEntryStub[qname].sStackCurrent = (uintptr_t *)currentEntry;

    // Mark the current node as the last valid node, if applicable
    if (currentEntry->sNext == NULL) {
        llEntryStub[qname].sStackLastValid = (uintptr_t *)currentEntry;
    }
}


PROCEDURE(currentnode) {
    int qname = GETINT(ARG0);

    // Retrieve the current node; default to the first node if current is NULL
    struct NodeEntry *currentEntry = (struct NodeEntry *)llEntryStub[qname].sStackCurrent;
    if (currentEntry == NULL) {
        currentEntry = (struct NodeEntry *)llEntryStub[qname].sStackFirst;
    }

    // Check if the list is empty
    if (currentEntry == NULL) {
        char emptyListMsg[32];
        sprintf(emptyListMsg, "$EMPTY-LLIST-%d$", qname);
        RETURNSTR(emptyListMsg);  // Return empty list marker
    }

    // Update the last valid node pointer and return the node's string
    llEntryStub[qname].sStackLastValid = (uintptr_t *)currentEntry;
    RETURNSTR(currentEntry->String);
}

PROCEDURE(currentnodeaddr) {
    int qname = GETINT(ARG0);

    // Retrieve the current node; default to the first node if current is NULL
    struct NodeEntry *currentEntry = (struct NodeEntry *)llEntryStub[qname].sStackCurrent;
    if (currentEntry == NULL) {
        currentEntry = (struct NodeEntry *)llEntryStub[qname].sStackFirst;
    }

    // Return 0 if the list is empty
    if (currentEntry == NULL) {
        RETURNINT(0);
    }

    // Update the last valid node pointer and return the address
    llEntryStub[qname].sStackLastValid = (uintptr_t *)currentEntry;
    RETURNINT((uintptr_t)currentEntry->sSaddr);
}


PROCEDURE(prevnode) {
    int qname = GETINT(ARG0);

    struct NodeEntry *currentEntry = (struct NodeEntry *) llEntryStub[qname].sStackCurrent;
    if (currentEntry == NULL) {
        char EOLL[16];
        sprintf(EOLL, "$%s%d$", "TOP-OF-LLIST-",qname);
        RETURNSTR(EOLL);
    }
    
    currentEntry = (struct NodeEntry *) currentEntry->sPrev;
    if (currentEntry == NULL) {
        char EOLL[16];
        sprintf(EOLL, "$%s%d$", "TOP-OF-LLIST-",qname);
        RETURNSTR(EOLL);
    } else {
        RETURNSTR(currentEntry->String);
        if (currentEntry->sPrev == NULL)  llEntryStub[qname].sStackLastValid= (uintptr_t *) currentEntry;
        llEntryStub[qname].sStackCurrent= (uintptr_t *) currentEntry;
    }
    ENDPROC
}

uintptr_t *poslist(int qname, char *position) {
    int i,pos = 0, steps = 0;
    int isRelative = 0;
    struct NodeEntry *currentEntry = NULL;

    // Determine if position is relative and convert it to an integer
    if (strchr(position, '+') || strchr(position, '-')) {
        isRelative = 1;
    }
    pos = atoi(position);

    // Determine the starting point
    if (isRelative) {
        currentEntry = (struct NodeEntry *)llEntryStub[qname].sStackCurrent;
        if (currentEntry == NULL) {
            currentEntry = (struct NodeEntry *)llEntryStub[qname].sStackLastValid;
        }
    } else {
        currentEntry = (struct NodeEntry *)llEntryStub[qname].sStackFirst;
    }

    // Traverse the list based on position
    if (currentEntry != NULL) {
        steps = (pos > 0) ? pos : -pos;

        for (i = 0; i < steps; i++) {
            if (pos > 0) { // Move forward
                if (currentEntry->sNext == NULL) break;
                currentEntry = (struct NodeEntry *)currentEntry->sNext;
            } else {       // Move backward
                if (currentEntry->sPrev == NULL) break;
                currentEntry = (struct NodeEntry *)currentEntry->sPrev;
            }
        }
    }
    return (uintptr_t *)currentEntry;
}

PROCEDURE(setnode) {
    int qname = GETINT(ARG0);
    char *position = GETSTRING(ARG1);
    toUpperCase(position);

    uintptr_t *current = NULL;

    // Determine the node position based on the input string
    if (strncmp(position, "FIRST", 2) == 0) {
        current = llEntryStub[qname].sStackFirst;
    } else if (strncmp(position, "LAST", 2) == 0) {
        current = llEntryStub[qname].sStackLast;
    } else {
        current = poslist(qname, position);
    }

    // Update the current node and return its address
    llEntryStub[qname].sStackCurrent = current;
    RETURNINT((uintptr_t)current);
}

PROCEDURE(setnodeaddr) {
    int qname = GETINT(ARG0);
    uintptr_t *current = (uintptr_t *)GETINT(ARG1);

    // Set the current node address and return it
    llEntryStub[qname].sStackCurrent = current;
    RETURNINT((uintptr_t)current);
}


PROCEDURE(listllist) {
    int qname = GETINT(ARG0);
    struct NodeEntry *current = (struct NodeEntry *) llEntryStub[qname].sStackFirst;
    while (current > 0) {
        printf("Entry:       %s \n",current->String);
        printf("Pointer: %p<-%p->%p \n",current->sPrev,current->sSaddr,current->sNext);
        current = (struct NodeEntry *) current->sNext;
    }
    RETURNINT(0);
    ENDPROC
}

// Add these functions for detailed memory reporting
        void print_llist_stats(int nodeIndex) {
            printf("\nLinked List %d Statistics:\n", nodeIndex);
            printf("Number of nodes: %d\n", llEntryStub[nodeIndex].sStackEntries);
            size_t total_memory = 0;
            struct NodeEntry* current = (struct NodeEntry*)llEntryStub[nodeIndex].sStackFirst;

            while (current) {
                total_memory += sizeof(struct NodeEntry) + strlen(current->String);
                current = (struct NodeEntry*)current->sNext;
            }

            printf("Total memory used: %zu bytes\n", total_memory);
        }

// Add memory validation function
        int validate_llist(int nodeIndex) {
            struct NodeEntry* current = (struct NodeEntry*)llEntryStub[nodeIndex].sStackFirst;
            int node_count = 0;
            struct NodeEntry* last = NULL;

            while (current) {
                // Validate memory header
                struct MemoryHeader* header = ((struct MemoryHeader*)current) - 1;
                if (header->magic != MEMORY_MAGIC) {
                    fprintf(stderr, "Memory corruption detected in node %d!\n", node_count);
                    return -1;
                }

                // Validate links
                if (current->sPrev != (last ? last->sSaddr : 0)) {
                    fprintf(stderr, "Invalid prev pointer in node %d!\n", node_count);
                    return -1;
                }

                last = current;
                current = (struct NodeEntry*)current->sNext;
                node_count++;
            }

            if (node_count != llEntryStub[nodeIndex].sStackEntries) {
                fprintf(stderr, "Node count mismatch! Expected %d, found %d\n",
                        llEntryStub[nodeIndex].sStackEntries, node_count);
                return -1;
            }

            return 0;
        }

// Add debug procedure for REXX
        PROCEDURE(debug_llist) {
            int nodeIndex = GETINT(ARG0);
            print_llist_stats(nodeIndex);
            int rc = validate_llist(nodeIndex);
            RETURNINT(rc);
            ENDPROC
        }
/* -------------------------------------------------------------------------------------
 * Functions to be provided to rexx
 * -------------------------------------------------------------------------------------
 */
LOADFUNCS
//      C Function, REXX namespace & name,      Option,Return Type, Arguments
//  !! Do not use "to" in the parm-list, make it for example "tto", else compile fails: "expose a = .string[],from=.int,tto=.int"
ADDPROC(appendnode,   "llist.appendnode", "b",  ".int",    "qname=.int,message=.string" );
ADDPROC(prependnode,  "llist.prependnode","b",  ".int",    "qname=.int,message=.string" );
ADDPROC(appendnode,   "llist.appnode",    "b",  ".int",    "qname=.int,message=.string" );
ADDPROC(prependnode,  "llist.prepnode",   "b",  ".int",    "qname=.int,message=.string" );

ADDPROC(insertnode,   "llist.insertnode", "b",  ".int",    "qname=.int,message=.string,mode=.string" );
ADDPROC(removenode,   "llist.removenode", "b",  ".int",    "qname=.int");

ADDPROC(currentnode,  "llist.currentnode","b",  ".string", "qname=.int" );
ADDPROC(currentnodeaddr,"llist.currentnodeaddr","b",".int","qname=.int" );
ADDPROC(nextnode,     "llist.nextnode",   "b",  ".string", "qname=.int" );
ADDPROC(prevnode,     "llist.prevnode",   "b",  ".string", "qname=.int" );

ADDPROC(setnode,     "llist.setnode",     "b",  ".int", "queue=.int,position=.string");
ADDPROC(setnodeaddr, "llist.setnodeaddr", "b",  ".int", "queue=.int,position=.int");

ADDPROC(listnode,     "llist.listnode",   "b",  ".int", "qname=.int" );
ADDPROC(listllist,    "llist.listllist",  "b",  ".int", "qname=.int" );
ADDPROC(freellist,    "llist.freellist",  "b",  ".int", "qname=.int" );
ADDPROC(debug_llist,   "llist.debug",    "b",  ".int",    "qname=.int" );
ADDPROC(cleanup,       "llist.cleanup",  "b",  ".int",    "" );
ENDLOADFUNCS
