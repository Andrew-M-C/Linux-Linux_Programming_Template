/* file encoding: UTF-8 */

#if defined (__cplusplus)
extern "C" {
#endif

#define DEBUG
#define CFG_LIB_STDOUT
#define CFG_LIB_FILE
#define CFG_LIB_ERRNO
#define CFG_LIB_DAEMON
#define CFG_LIB_SLEEP
#define CFG_LIB_PID
#define CFG_LIB_MMAP
#define CFG_LIB_STRING
#include "AMCCommonLib.h"

#include <setjmp.h>
#include <stdint.h>

#if defined (__cplusplus)
}
#endif

#include <string>
#include <vector>


/*******************************************************
 * software information
 */
#define	CFG_SOFTWARE_NAME_STR		"(no name)"
#define	CFG_SOFTWARE_DISCRIPT_STR	"An application to test det(A)"
#define	CFG_SOFTWARE_VER_STR		"0.0.1"
#define	CFG_AUTHOR_STR		"Andrew Chang"

/*******************************************************
 * coroutines
 */
typedef struct Context
{
    uint64_t    rbx;    // 0
    uint64_t    rsp;    // 8
    uint64_t    rbp;    // 16
    uint64_t    r12;    // 24
    uint64_t    r13;    // 32
    uint64_t    r14;    // 40
    uint64_t    r15;    // 48
    uint64_t    rip;    // 56: return address for coroutine
    uint64_t    break_point;    // 64
} Context_st;


typedef struct Stack
{
    uint8_t     stack[1024];
} Stack_st;

extern "C" void asm_init_context(Context_st *pCtx);
extern "C" void asm_save_context(Context_st *pCtx);
extern "C" void asm_save_main_and_go(Context_st *pMainCtx, Context_st *pCorCtx);
extern "C" void *asm_get_stack_addr(void);


static void _run_coroutines(Context_st *pMainCtx, Context_st *pSubCtx)
{
    AMCPrintf("Now go with %p and %p", pMainCtx, pSubCtx);
    asm_save_main_and_go(pMainCtx, pSubCtx);
    AMCPrintf("go done");
    return;
}


static void _schedule(unsigned to)
{
    
    return;
}


static void _coroutine_did_end()
{
    AMCPrintf("A coroutine ends");

    return;
}


static void _func_A(void)
{
    for (unsigned tmp = 0; tmp < 5; tmp++)
    {
        AMCPrintf("A: %02u", tmp);
        _schedule(1);
    }
    return;
}


static void _func_B(void)
{
    for (unsigned tmp = 0; tmp < 5; tmp++)
    {
        AMCPrintf("B: %02u", tmp);
        _schedule(0);
    }
    return;
}

static void _init_contexts(Context_st pContexts[2])
{
    int mmapFlags = MAP_PRIVATE | MAP_ANONYMOUS;
    //int mmapFlags = MAP_SHARED;
    const size_t STACK_SIZE = 128;

    AMCPrintf("&(g_contexts[0]) = %p", &(pContexts[0]));
    AMCPrintf("&(g_contexts[1]) = %p", &(pContexts[1]));

    asm_init_context(&(pContexts[0]));
    AMCPrintf("Before: g_contexts[0].rsp: %p", (void*)(pContexts[0].rsp));
    pContexts[0].rip = (uint64_t)_coroutine_did_end;
    pContexts[0].break_point = (uint64_t)_func_A;
    pContexts[0].rsp = (uint64_t)mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE, mmapFlags, -1, 0);
    pContexts[0].rsp += STACK_SIZE - sizeof(uint64_t);

    asm_init_context(&(pContexts[1]));
    pContexts[1].rip = (uint64_t)_coroutine_did_end;
    pContexts[1].break_point = (uint64_t)_func_B;
    pContexts[1].rsp = (uint64_t)mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE, mmapFlags, -1, 0);
    pContexts[1].rsp += STACK_SIZE - sizeof(uint64_t);

    //mprotect((void*)(g_contexts[0].rsp), STACK_SIZE, PROT_NONE);
    //mprotect((void*)(g_contexts[1].rsp), STACK_SIZE, PROT_NONE);

    uint64_t *pTest = (uint64_t*)(pContexts[0].rsp - STACK_SIZE);
    pTest[0] = 0x123456789ABCDEF0;
    pTest[(STACK_SIZE / sizeof(uint64_t)) - 1] = 0x123456789ABCDEF0;
    AMCPrintf("After:  g_contexts[0].rsp: %p", (void*)(pContexts[0].rsp));
    AMCPrintf("After:  g_contexts[1].rsp: %p", (void*)(pContexts[1].rsp));
    AMCPrintf("main rsp: %p", asm_get_stack_addr());

    AMCDataDump((const void *)pTest, STACK_SIZE);

    return;
}





/**********/
/* main */
static int _true_main(int argc, char* argv[])
{
#if defined (__amd64__)
    AMCPrintf("Hello AMD64");
#elif defined (__x86_64__)
    AMCPrintf("Hello x86_64");
#elif defined(__i386__)
    AMCPrintf("Hello i386");
#else
#error Upsupported CPU archetecture.
#endif

    Context_st contexts[2];
    Context_st mainContexts;

    AMCPrintf("sizeof(void *) = %u", (unsigned)sizeof(void *));
    AMCPrintf("sizeof(Context_st *) = %u", (unsigned)sizeof(Context_st *));
    _init_contexts(contexts);
    _run_coroutines(&mainContexts, contexts);
    AMCPrintf("coroutine ends");

	/****/
	/* ENDS */
	return 0;
}


int main(int argc, char* argv[])
{
	int mainRet;

	AMCPrintf (CFG_SOFTWARE_DISCRIPT_STR);
	AMCPrintf ("Version "CFG_SOFTWARE_VER_STR);
	AMCPrintf ("Author: "CFG_AUTHOR_STR);
	AMCPrintf ("main() build time: "__TIME__", "__DATE__);
	AMCPrintf ("----START----");

	mainRet = _true_main(argc, argv);
	TEST_VALUE(mainRet);
	
	AMCPrintf ("----ENDS-----");
	exit(mainRet);
	return 0;
}

