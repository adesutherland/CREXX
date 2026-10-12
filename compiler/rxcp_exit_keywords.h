/* cREXX License (MIT), Copyright (c) 2026 Adrian Sutherland */
#ifndef CREXX_RXCP_EXIT_KEYWORDS_H
#define CREXX_RXCP_EXIT_KEYWORDS_H
#include "rxcp_exit.h"
typedef struct CertifiedExitSpec {
    const char *primary_keyword;
    unsigned int required_flags;
} CertifiedExitSpec;

typedef struct ExitHelperRecord {
    ASTNode *file_node;
    char *helper_id;
    char *source_text;
    struct ExitHelperRecord *next;
} ExitHelperRecord;

static const CertifiedExitSpec certified_exit_specs[] = {
    { "ADDRESS",
      RXCP_EXIT_FLAG_CERTIFIED | RXCP_EXIT_FLAG_RESERVED_KEYWORD | RXCP_EXIT_FLAG_IMPLICIT_COMMAND },
    { "PARSE",
      RXCP_EXIT_FLAG_CERTIFIED | RXCP_EXIT_FLAG_RESERVED_KEYWORD },
    { "PUSH",
      RXCP_EXIT_FLAG_CERTIFIED | RXCP_EXIT_FLAG_RESERVED_KEYWORD },
    { "PULL",
      RXCP_EXIT_FLAG_CERTIFIED | RXCP_EXIT_FLAG_RESERVED_KEYWORD },
    { "QUEUE",
      RXCP_EXIT_FLAG_CERTIFIED | RXCP_EXIT_FLAG_RESERVED_KEYWORD },
    { "SIGNAL",
      RXCP_EXIT_FLAG_CERTIFIED | RXCP_EXIT_FLAG_RESERVED_KEYWORD },
    { "TRACE",
      RXCP_EXIT_FLAG_CERTIFIED | RXCP_EXIT_FLAG_RESERVED_KEYWORD },
    { NULL, 0 }
};

#endif
