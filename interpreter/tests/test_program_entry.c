/* Ordinary API refusal precedes initialization; rxvm_run owns root entry. */
#include "rxvm.h"
#include "rxvmintp.h"
#include "rxvml.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(x,m) do { if (!(x)) { fprintf(stderr,"FAIL: %s\n",m); failures++; } } while (0)

int main(int argc, char **argv) {
    rxvm_context *context;
    rxvml_context *library_context;
    module *root = NULL;
    proc_runtime *main_procedure = NULL;
    size_t i;
    int program_rc = -1;
    if (argc != 2) return 2;
    context = rxvm_create(); CHECK(context != NULL,"create program-entry context");
    if (!context) return 1;
    CHECK(rxvm_load_file(context,argv[1]) != NULL,"load marked program image");
    for (i = 0u; i < context->num_modules; i++)
        if (context->modules[i]->file && !strcmp(context->modules[i]->file->name,"entry-root.rxas")) root = context->modules[i];
    CHECK(root != NULL,"locate exact program module");
    if (!root) { rxvm_destroy(context); return 1; }
    CHECK(rxvm_call(context,NULL,0,NULL) == -1,"ordinary implicit main API rejected");
    CHECK(root->globals[0]->int_value == 0 && root->globals[1]->int_value == 0 &&
          root->initializer_state == RXVM_INIT_UNINITIALIZED,"implicit refusal does not execute initializer or main");
    CHECK(rxvm_call(context,"main",0,NULL) == -1,"ordinary named main API rejected");
    CHECK(root->globals[0]->int_value == 0 && root->globals[1]->int_value == 0 &&
          root->initializer_state == RXVM_INIT_UNINITIALIZED,"named refusal does not execute initializer or main");
    CHECK(rxvm_call(context,"entry.start",0,NULL) == -1,"ordinary exposed main API rejected");
    CHECK(root->globals[0]->int_value == 0 && root->globals[1]->int_value == 0 &&
          root->initializer_state == RXVM_INIT_UNINITIALIZED,"exposed refusal has no startup side effects");
    for (i = 0u; i < root->procedure_count; i++)
        if (!strcmp(root->procedures[i].name,"main")) main_procedure = &root->procedures[i];
    CHECK(main_procedure && main_procedure->entry_state_kind == RXBIN_ENTRY_PROGRAM_DEFAULT,"typed contract bound to exact runtime main");
    if (main_procedure) rxvml_set_int((rxvml_value *)root->globals[2],(rxinteger)(uintptr_t)main_procedure);
    CHECK(rxvm_run(context,0,NULL) == 0,"checked program entry establishes defaults and rejects dynamic child before result clear");
    CHECK(root->globals[0]->int_value == 1 && root->globals[1]->int_value == 1,"one root body and one successful default initializer");
    CHECK(rxvm_call(context,"entry.start",0,NULL) == -1,"ready marked main still requires checked entry");
    CHECK(root->globals[0]->int_value == 1,"rejected ready ingress cannot repeat main");
    rxvm_destroy(context);
    library_context = rxvml_create(NULL,0);
    CHECK(library_context != NULL,"create independent rxvml program context");
    if (library_context) {
        CHECK(rxvml_load_module_file(library_context,argv[1]) > 0,"load marked rxvml program");
        CHECK(rxvml_run(library_context,0,NULL,&program_rc) == 0 && program_rc == 0,
              "rxvml_run uses checked marked-root handshake");
        rxvml_destroy(library_context);
    }
    printf("PROGRAM_ENTRY_VM|failures=%d\n",failures);
    return failures ? 1 : 0;
}
