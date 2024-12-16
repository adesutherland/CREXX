//
// System Information Plugin for crexx/pa - Plugin Architecture
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // For POSIX systems (Linux/macOS)
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file
#include <windows.h>
#define HIGHVALUE ((char *)-1)

struct NodeStub {
    uintptr_t * sStackFirst;
    uintptr_t * sStackLast;
    uintptr_t * sStackCurrent;
    uintptr_t * sStackLastValid;
    int         sStackEntries;
    char        stackname[16];
};

HANDLE hHeap = NULL;

struct NodeStub llEntryStub[99]={0};  // Array of head pointers

struct NodeEntry {
    uintptr_t * sSaddr;  // self reference Pointer to the stack
    uintptr_t * sNext;
    uintptr_t * sPrev;
    char String[5];
};

uintptr_t getmain(int bytes) {
    // Allocate memory from the heap
    if (hHeap == NULL) {
        hHeap = GetProcessHeap();
    }
    LPVOID pMemory = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, bytes);
    if (pMemory == NULL) {
        printf("Heap allocation failed\n");
        return -8;
    }
    // Use the allocated memory
    return (uintptr_t) pMemory;
}
int freemain(LPVOID storage ) {
    if(storage==NULL) return 0;
    if (!HeapFree(hHeap, 0, storage)) return -8;
    return 0;
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

int2string(char *result, int number){
   char str[20];
   sprintf(str, "%d", number); // Convert integer to string
   trim(str);
   strcpy(result,str);
}


// Function to create a new node
struct NodeEntry * createNodeEntry(char * message) {
    struct NodeEntry* newNode;
    uintptr_t * nodeADDR= (uintptr_t *) getmain(sizeof(struct NodeEntry) + strlen(message) + 16);
    if (nodeADDR == NULL) {
        printf("Memory allocation failed\n");
        return (struct NodeEntry *) -8;
    }
    newNode= (struct NodeEntry *)  nodeADDR;
    newNode->sSaddr= nodeADDR;
    strncpy(newNode->String,message,strlen(message));

    newNode->sNext = NULL;
    return newNode;
}

// Function to add a node at the end of a list
PROCEDURE(addnode)  {
    int    nodeindx = GETINT(ARG0);
    char * message  = GETSTRING(ARG1);
    struct NodeEntry *newNode = createNodeEntry(message);            // allocate storage for new entry

    if (llEntryStub[nodeindx].sStackFirst== NULL) {                  // it will be the first entry for this stack
        llEntryStub[nodeindx].sStackFirst  = (uintptr_t *) newNode;  // save pointer to first element
        llEntryStub[nodeindx].sStackCurrent= (uintptr_t *) newNode;  // set current pointer to first element
    }
    struct NodeEntry * last = (struct NodeEntry *) llEntryStub[nodeindx].sStackLast; // address last entry, if any
    struct NodeEntry * new =  newNode;                                               // address new entry

    if(last == NULL)   new->sPrev = 0;          // this is the first element, reset pointer to previous one
    else {                                      // there is already at least one element in queue
        last->sNext = (uintptr_t *) newNode;    // forward pointer from previous element to the new one
        new->sPrev=last->sSaddr;                // backward pointer from new to previous one
    }
    llEntryStub[nodeindx].sStackEntries++;
    llEntryStub[nodeindx].sStackLast   = (uintptr_t *) newNode;  // set new last entry in stub
    new->sSaddr= (uintptr_t *) newNode;                          // create self reference address in entry
    new->sNext=0;                                                // init forward pointer
}

// Function to display a linked list
PROCEDURE(listnode) {
    int nodeindx=GETINT(ARG0);
    printf("Linked List %d:\n",GETINT(ARG0));
    if (llEntryStub[nodeindx].sStackFirst== NULL) {
        printf("Empty list\n");
        return;
    }
    struct NodeEntry * temp = (struct NodeEntry *) llEntryStub[nodeindx].sStackFirst;
    while (temp != NULL) {
        printf("%s -> ", temp->String);
        temp = (struct NodeEntry *) temp->sNext;
    }
    printf("NULL\n");
}

// Function to free memory of a linked list
PROCEDURE(freellist) {
    int nodeindx=GETINT(ARG0);
    if (llEntryStub[nodeindx].sStackFirst== NULL) {
        printf("Empty list\n");
        return;
    }
    struct NodeEntry * temp = (struct NodeEntry *) llEntryStub[nodeindx].sStackFirst;
    struct NodeEntry * next;
    while (temp != NULL) {
        next = (struct NodeEntry *) temp->sNext;
        freemain(temp);
        temp = next;
    }
}

/*
PROCEDURE(insertqueue) {
    int qname = GETINT(ARG0);
    char *message = GETSTRING(ARG1);
    int count = 0, insert = GETINT(ARG2);

    QueueStub[qname].sStackCurrent = (struct NodeEntry *) QueueStub[qname].stackptr;
    struct NodeEntry *current = (struct NodeEntry *) QueueStub[qname].sStackCurrent;
    while (current > 0) {
        if (count == insert) break;
        current = (struct NodeEntry *) current->sNext;
        count++;
    }
    printf("Entry after %d %d %p\n",count,insert,current);
    RETURNINT(0);
    ENDPROC
}
*/

PROCEDURE(nextnode) {
    int qname = GETINT(ARG0);

    struct NodeEntry *currentEntry = (struct NodeEntry *) llEntryStub[qname].sStackCurrent;
    currentEntry= (struct NodeEntry *) currentEntry->sNext;
    if (currentEntry == 0) {
        char EOLL[16];
        sprintf(EOLL, "$%s%d$", "END-OF-LLIST-",qname);
        RETURNSTR(EOLL);
    }else {
        RETURNSTR(currentEntry->String);
        if (currentEntry->sNext == 0)  llEntryStub[qname].sStackLastValid= (uintptr_t *) currentEntry;
        llEntryStub[qname].sStackCurrent= (uintptr_t *) currentEntry;
    }
        ENDPROC
}

PROCEDURE(currentnode) {
    int qname = GETINT(ARG0);

    struct NodeEntry *currentEntry = (struct NodeEntry *) llEntryStub[qname].sStackCurrent;
    if (currentEntry == 0) currentEntry = (struct NodeEntry *) llEntryStub[qname].sStackFirst;
    if (currentEntry == 0) {
        char EOLL[16];
        sprintf(EOLL, "$%s%d$", "EMPTY-LLIST-",qname);
        RETURNSTR(EOLL);
    }else {
        RETURNSTR(currentEntry->String);
        llEntryStub[qname].sStackLastValid= (uintptr_t *) currentEntry;
    }
    ENDPROC
}
PROCEDURE(currentnodeaddr) {
    int qname = GETINT(ARG0);

    struct NodeEntry *currentEntry = (struct NodeEntry *) llEntryStub[qname].sStackCurrent;
    if (currentEntry == 0) currentEntry = (struct NodeEntry *) llEntryStub[qname].sStackFirst;
    if (currentEntry == 0) {
       RETURNINT(0);
    }else {
        RETURNINT((uintptr_t) currentEntry->sSaddr);
        llEntryStub[qname].sStackLastValid= (uintptr_t *) currentEntry;
    }
    ENDPROC
}

PROCEDURE(prevnode) {
    int qname = GETINT(ARG0);

    struct NodeEntry *currentEntry = (struct NodeEntry *) llEntryStub[qname].sStackCurrent;
    currentEntry= (struct NodeEntry *) currentEntry->sPrev;
    if (currentEntry == 0) {
        char EOLL[16];
        sprintf(EOLL, "$%s%d$", "TOP-OF-LLIST-",qname);
        RETURNSTR(EOLL);
    }else {
        RETURNSTR(currentEntry->String);
        if (currentEntry->sPrev == 0)  llEntryStub[qname].sStackLastValid= (uintptr_t *) currentEntry;
        llEntryStub[qname].sStackCurrent= (uintptr_t *) currentEntry;
    }
    ENDPROC
}

void poslist(int qname,char * position){
    int i,pos,relpos=0;
    struct NodeEntry *currentEntry;
    if (strstr(position,"+")>0) relpos=1;
    else if (strstr(position,"-")>0) relpos=1;
    pos=atoi(position);
    if (relpos==1) {
        currentEntry = (struct NodeEntry *) llEntryStub[qname].sStackCurrent;
        if (currentEntry == 0)  currentEntry= (struct NodeEntry *) llEntryStub[qname].sStackLastValid;
        if (pos > 0) { // relative position to current position
            for (i = 0; i < pos; i++) {
                if (currentEntry->sNext == 0) break;
                currentEntry = (struct NodeEntry *) currentEntry->sNext;
            }
         } else {
            for (i = 0; i < -pos; i++) {
                if (currentEntry->sPrev == 0) break;
                currentEntry = (struct NodeEntry *) currentEntry->sPrev;
            }
        }
    } else {
        currentEntry = (struct NodeEntry *) llEntryStub[qname].sStackFirst;
        for (i = 1; i < pos; i++) {
            if (currentEntry->sNext == 0) break;
            currentEntry = (struct NodeEntry *) currentEntry->sNext;
        }
    }
    llEntryStub[qname].sStackCurrent = (uintptr_t *) currentEntry;
}

PROCEDURE(setnode) {
    int pos, qname=GETINT(ARG0);
    char * position=GETSTRING(ARG1);
    toUpperCase(position);
    if(strncmp(position,"FIRST",2)==0) {
       llEntryStub[qname].sStackCurrent = llEntryStub[qname].sStackFirst;
    } else if(strncmp(position,"LAST",2)==0) {
       llEntryStub[qname].sStackCurrent =  llEntryStub[qname].sStackLast;
    } else {
        poslist(qname, position);
    }
    RETURNINT(0);
    ENDPROC
}
PROCEDURE(setnodeaddr) {
    int pos, qname=GETINT(ARG0);
    char * position=GETSTRING(ARG1);
    toUpperCase(position);
    if(strncmp(position,"FIRST",2)==0) {
        llEntryStub[qname].sStackCurrent = llEntryStub[qname].sStackFirst;
    } else if(strncmp(position,"LAST",2)==0) {
        llEntryStub[qname].sStackCurrent =  llEntryStub[qname].sStackLast;
    } else {
        poslist(qname, position);
    }
    RETURNINT(0);
    ENDPROC
}
PROCEDURE(removequeue) {
    int qname = GETINT(ARG0);
    struct NodeEntry *current = (struct NodeEntry *) llEntryStub[qname].sStackFirst;
    uintptr_t * next;
    while (current > 0) {
       next=current->sNext;
       freemain(current->sSaddr);
       current = (struct NodeEntry *) next;
    }
    RETURNINT(0);
    ENDPROC
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
/* -------------------------------------------------------------------------------------
 * Functions to be provided to rexx
 * -------------------------------------------------------------------------------------
 */
LOADFUNCS
//      C Function, REXX namespace & name,      Option,Return Type, Arguments
//  !! Do not use "to" in the parm-list, make it for example "tto", else compile fails: "expose a = .string[],from=.int,tto=.int"
ADDPROC(addnode,      "llist.addnode",    "b",  ".int",    "qname=.int,message=.string" );
ADDPROC(currentnode,  "llist.currentnode","b",  ".string", "qname=.int" );
ADDPROC(currentnodeaddr,"llist.currentnodeaddr","b",".int", "qname=.int" );
ADDPROC(nextnode,     "llist.nextnode",   "b",  ".string", "qname=.int" );
ADDPROC(prevnode,     "llist.prevnode",   "b",  ".string", "qname=.int" );

ADDPROC(setnode,     "llist.setnode",     "b",  ".int", "queue=.int,position=.string");
ADDPROC(setnodeaddr, "llist.setnodeaddr", "b",  ".int", "queue=.int,position=.int");

ADDPROC(listnode,     "llist.listnode",   "b",  ".int", "qname=.int" );
ADDPROC(listllist,    "llist.listllist",  "b",  ".int", "qname=.int" );
ADDPROC(freellist,    "llist.freellist",   "b",  ".int", "qname=.int" );
ENDLOADFUNCS