/******************************************************************
 * Copyright (c) 2017, Andrew Chang, All rights reserved.
 * File Name: AMCCoroutine.c
 * Create Time: 2017-11-23
 * License: BSD 2-clause "Simplified" License
 *
 * History:
 *     2017-11-23: File created as AMCCoroutine.c
 */

/****************/
#define __HEADERS__
#if 1

#include "AMCCoroutine.h"
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define _CFG_DEFAULT_STACK_SIZE     (64*1028)

//#define _AMC_COROUTINE_DEBUG
#ifdef _AMC_COROUTINE_DEBUG
#define	CFG_LIB_STRING
#define CFG_LIB_STDOUT
#include "AMCCommonLib.h"
#define _DEBUG(fmt, args...)    AMCPrintf("%s, %d: "fmt, __FILE__, __LINE__, ##args)
#define _DUMP(addr, len)        do{AMCPrintf("Dump " #addr ": ");AMCDataDump((addr), (len));}while(0)
#else
#define _DEBUG(fmt, args...)
#define _DUMP(addr, len) 
#endif

#endif

/****************/
#define __DATA_TYPES__
#if 1

#ifndef BOOL
#define BOOL    int
#define FALSE   (0)
#define TRUE    (!FALSE)
#endif

#ifndef NULL
#define NULL    ((void *)0)
#endif

struct _CoroutineInfo
{
    // register values
    uint64_t    reg_rbx;    // 0
    uint64_t    reg_rsp;    // 8: "stack_pointer"
    uint64_t    reg_rbp;    // 16: "base_pointer"
    uint64_t    reg_r12;    // 24
    uint64_t    reg_r13;    // 32
    uint64_t    reg_r14;    // 40
    uint64_t    reg_r15;    // 48
    uint64_t    reg_rip;    // 56: "instruction_pointer", next instruction of the coroutine

    // return address
    uint64_t    func_ret_addr;      // 64: return address of current function (not the asm func)

    // coroutine informations
    CoroutineFunc_t coroutine_func; // 72
    void        *coroutine_arg;
    void        *coroutine_ret;

    // stack resources
    uint64_t    stack_size;
    void        *p_stack_base;   // This is the address returned by mmap(). cannot be used directly
    void        *p_stack_top;    // In x86/x64, this is he smallest address of the stack. Should be equal to stack_base
    void        *p_stack_bottom; // In x86/x64, this is he biggest address of the stack.

    // pointer to generate coroutine chain
    struct _CoroutineInfo   *p_prev;
    struct _CoroutineInfo   *p_next;

    // identifier
    char        identifier[0];
};

#define _COROUTINE_INFO_REG_SIZE    (64)    // 8 registers x 8 bytes

#endif

/****************/
#define __ASSEMBLY_FUNCTIONS__
#if 1

extern void asm_amc_coroutine_dump(CoroutineInfo_st *pCtx);
extern void asm_amc_coroutine_enter(CoroutineInfo_st *pCtxTo);
extern void asm_amc_coroutine_restore(CoroutineInfo_st *pCtx);
extern void asm_amc_coroutine_switch_sp_rip_to(CoroutineInfo_st *pCtxTo);
extern void asm_amc_coroutine_return_to_main(CoroutineInfo_st *pCtxTo);

static CoroutineInfo_st *_alloc_coroutine_info(size_t stackSize, size_t identifierSize);
static int _free_coroutine_info(CoroutineInfo_st *pCtx);

#endif

/****************/
#define __GLOBAL_VARIABLES__
#if 1

static BOOL             g_Initialized = FALSE;
static CoroutineInfo_st *g_pMainThreadInfo = NULL;      // need allocate
static CoroutineInfo_st *g_pAllCoroutines = NULL;       // need allocate
static CoroutineInfo_st *g_pCurrentCoroutine = NULL;

#endif

/****************/
#define __COMMON_INTERNAL_FUNCTION__
#if 1

static int _init_main_thread()
{
    g_pMainThreadInfo = malloc(sizeof(*g_pMainThreadInfo));
    if (NULL == g_pMainThreadInfo) {
        return -__LINE__;
    }

    g_Initialized = TRUE;
    return 0;
}


static void _coroutine_did_end()
{
    _DEBUG("%s ends", g_pCurrentCoroutine->identifier);
    _DEBUG("stack: %p", AMCGetStackAddr());

    asm_amc_coroutine_switch_sp_rip_to(g_pMainThreadInfo);
    _DEBUG("Switch to main stack: %p", AMCGetStackAddr());
    _DUMP(AMCGetStackAddr(), 128);

    // no coroutine remains
    if (g_pCurrentCoroutine == g_pCurrentCoroutine->p_next)
    {
        _DEBUG("Free %s", g_pCurrentCoroutine->identifier);
        _free_coroutine_info(g_pCurrentCoroutine);
        g_pAllCoroutines = NULL;
        g_pCurrentCoroutine = NULL;
    }
    else
    {
        register CoroutineInfo_st *pCtxToDel = g_pCurrentCoroutine;
        g_pCurrentCoroutine = pCtxToDel->p_next;
        if (g_pAllCoroutines == pCtxToDel) {
            g_pAllCoroutines = pCtxToDel->p_next;
        }

        pCtxToDel->p_prev->p_next = pCtxToDel->p_next;
        pCtxToDel->p_next->p_prev = pCtxToDel->p_prev;

        _DEBUG("Free %s", pCtxToDel->identifier);
        _free_coroutine_info(pCtxToDel);

        asm_amc_coroutine_restore(g_pCurrentCoroutine);
    }

    // all coroutine ends
    _DEBUG("Now return");
    asm_amc_coroutine_return_to_main(g_pMainThreadInfo);
    return;     // this will NOT be executed
}


static CoroutineInfo_st *_alloc_coroutine_info(size_t stackSize, size_t identifierSize)
{
    void *pStack = NULL;
    CoroutineInfo_st *ret = malloc(sizeof(*ret) + identifierSize + 1);
    if (NULL == ret) {
        return ret;
    }

    if (0 == stackSize) {
        stackSize = _CFG_DEFAULT_STACK_SIZE;
    }

    int mmapFlags = MAP_PRIVATE | MAP_ANONYMOUS;
    pStack = mmap(NULL, stackSize, PROT_READ | PROT_WRITE, mmapFlags, -1, 0);
    if (NULL == pStack) {
        goto FAIL;
    }

    ret->stack_size = stackSize;
    ret->p_stack_base = pStack;
    ret->p_stack_top = pStack;
    ret->p_stack_bottom = pStack + stackSize - sizeof(uint64_t);

    return ret;

FAIL:
    if (pStack) {
        munmap(pStack, stackSize);
        pStack = NULL;
    }
    if (ret) {
        free(ret);
        ret = NULL;
    }
    return NULL;
}


static int _free_coroutine_info(CoroutineInfo_st *pCtx)
{
    if (NULL == pCtx) {
        return -__LINE__;
    }

    if (pCtx->p_stack_base) {
        munmap(pCtx->p_stack_base, pCtx->stack_size);
        pCtx->p_stack_base = NULL;
    }

    free(pCtx);
    pCtx = NULL;

    return 0;
}


#endif

/****************/
#define __PUBLIC_INTERFACES__
#if 1

CoroutineInfo_st *AMCCoroutineAdd(CoroutineFunc_t coroutineFunc, void *arg, size_t stackSize, const char *identifier)
{
    int callStat = 0;
    CoroutineInfo_st *ret = NULL;
    char identifierBuff[64] = "";
    size_t identifierSize = 0;

    if (NULL == coroutineFunc) {
        errno = EINVAL;
        return NULL;
    }

    if (NULL == identifier) {
        identifier = identifierBuff;
        snprintf(identifierBuff, sizeof(identifierBuff) - 1, "func %p", coroutineFunc);
    }
    identifierSize = strlen(identifier);

    // 0. initialization
    if (FALSE == g_Initialized)
    {
        callStat = _init_main_thread();
        if (callStat != 0) {
            return NULL;
        }
    }

    // 1. allocation
    ret = _alloc_coroutine_info(stackSize, identifierSize);
    if (NULL == ret) {
        return NULL;
    }

    // 2. list assigning
    if (NULL == g_pAllCoroutines)
    {
        g_pAllCoroutines = ret;
        ret->p_next = ret;
        ret->p_prev = ret;
    }
    else
    {
        CoroutineInfo_st *pLast = g_pAllCoroutines->p_prev;
        pLast->p_next = ret;
        ret->p_prev = pLast;
        ret->p_next = g_pAllCoroutines;
        g_pAllCoroutines->p_prev = ret;
    }

    // 3. parameter assigning
    memcpy(ret, g_pMainThreadInfo, _COROUTINE_INFO_REG_SIZE);
    ret->reg_rsp = (uint64_t)(ret->p_stack_bottom);
    ret->reg_rbp = 0;
    ret->coroutine_func = coroutineFunc;
    ret->coroutine_arg = arg;
    ret->reg_rip = (uint64_t)coroutineFunc;
    ret->func_ret_addr = (uint64_t)_coroutine_did_end;
    strcpy(ret->identifier, identifier);

    _DEBUG("Coroutine %s added", ret->identifier);
    _DEBUG("%s stask: %p", ret->identifier, ret->p_stack_bottom);
    return ret;
}


int AMCCoroutineRun()
{
    if (NULL == g_pAllCoroutines) {
        return 0;
    }

    _DUMP(AMCGetStackAddr(), 128);

    asm_amc_coroutine_dump(g_pMainThreadInfo);  // dump main thread again to get return point of this function.
    g_pMainThreadInfo->reg_rsp += 1 * sizeof(uint64_t);     // ignore return address for function "asm_amc_coroutine_dump"

    _DUMP((void *)(g_pMainThreadInfo->reg_rsp), 128);

    _DEBUG("AMCCoroutineRun return address: %p, main stack: %p", 
                (void *)(g_pMainThreadInfo->func_ret_addr),
                (void *)(g_pMainThreadInfo->reg_rsp));

    g_pCurrentCoroutine = g_pAllCoroutines;
    _DEBUG("Ready to run: %s, stack %p, entry %p, rip %p", 
                    g_pCurrentCoroutine->identifier, 
                    (void *)(g_pCurrentCoroutine->reg_rsp), 
                    (void *)(g_pCurrentCoroutine->coroutine_func),
                    (void *)(g_pCurrentCoroutine->reg_rip));
    asm_amc_coroutine_enter(g_pCurrentCoroutine);

    return -__LINE__;
}


int AMCCoroutineSchedule()
{
    if (NULL == g_pCurrentCoroutine) {
        errno = EPERM;
        return -__LINE__;
    }

    asm_amc_coroutine_dump(g_pCurrentCoroutine);
    g_pCurrentCoroutine->reg_rip = (uint64_t)(&&RETURN);
    g_pCurrentCoroutine = g_pCurrentCoroutine->p_next;
    asm_amc_coroutine_restore(g_pCurrentCoroutine);

RETURN:
    _DEBUG("go back to %s", g_pCurrentCoroutine->identifier);
    return 0;
}


#endif

/****************/
/* end of file */

