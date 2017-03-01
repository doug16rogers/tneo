/*******************************************************************************
 *
 * TNeo: real-time kernel initially based on TNKernel
 *
 *    TNKernel:                  copyright 2004, 2013 Yuri Tiomkin.
 *    PIC32-specific routines:   copyright 2013, 2014 Anders Montonen.
 *    TNeo:                      copyright 2014       Dmitry Frank.
 *    TNeo:                      copyright 2017       Doug Rogers. (for Unix)
 *
 *    TNeo was born as a thorough review and re-implementation of
 *    TNKernel. The new kernel has well-formed code, inherited bugs are fixed
 *    as well as new features being added, and it is tested carefully with
 *    unit-tests.
 *
 *    API is changed somewhat, so it's not 100% compatible with TNKernel,
 *    hence the new name: TNeo.
 *
 *    Permission to use, copy, modify, and distribute this software in source
 *    and binary forms and its documentation for any purpose and without fee
 *    is hereby granted, provided that the above copyright notice appear
 *    in all copies and that both that copyright notice and this permission
 *    notice appear in supporting documentation.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE DMITRY FRANK AND CONTRIBUTORS "AS IS"
 *    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *    PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DMITRY FRANK OR CONTRIBUTORS BE
 *    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *    THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

/**
 *
 * \file
 *
 * TNeo-on-Unix architecture-dependent routines
 *
 */

#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef _TN_ARCH_UNIX_H_
#define _TN_ARCH_UNIX_H_

#ifdef __cplusplus
extern "C"  {     /*}*/
#endif

#define _TN_ARCH_STACK_PT_TYPE  _TN_ARCH_STACK_PT_TYPE__FULL
#define _TN_ARCH_STACK_DIR      _TN_ARCH_STACK_DIR__DESC
#define _TN_STATIC_INLINE       static inline

#define _TN_FATAL_ERROR(...)                             \
    do {                                                 \
        fprintf(stderr, "%s:%u: ", __FILE__, __LINE__);  \
        fprintf(stderr, __VA_ARGS__);                    \
        fprintf(stderr, "\n");                           \
        abort();                                         \
    } while (0)

/**
 * Minimum task's stack size, in words, not in bytes; includes a space for
 * context plus for parameters passed to task's body function.
 */
#define  TN_MIN_STACK_SIZE          (0x40 + _TN_STACK_OVERFLOW_SIZE_ADD)

/**
 * Unsigned integer type whose size is equal to the size of CPU register.
 * Typically it's plain `unsigned int`.
 */
typedef  unsigned int TN_UWord;

/**
 * Width of `int` type.
 */
#define  TN_INT_WIDTH           (sizeof(TN_UWord) * 8)

/**
 * Unsigned integer type that is able to store pointers.
 * We need it because some platforms don't define `uintptr_t`.
 * Typically it's `unsigned int`.
 */
typedef  uintptr_t TN_UIntPtr;

/**
 * This is how we enable and disable interrupts.
 */
extern void _tn_arch_unix_int_enable(int enabled);

/**
 * @return non-zero if interrupts are enabled, 0 otherwise.
 */
extern int _tn_arch_unix_int_enabled(void);

/**
 * Maximum number of priorities available, this value usually matches
 * `#TN_INT_WIDTH`.
 *
 * @see TN_PRIORITIES_CNT
 */
#define  TN_PRIORITIES_MAX_CNT      32

/**
 * Value for infinite waiting, usually matches `ULONG_MAX`,
 * because `#TN_TickCnt` is declared as `unsigned long`.
 */
#define  TN_WAIT_INFINITE           (TN_TickCnt)0xFFFFFFFF

/**
 * Value for initializing the task's stack
 */
#define  TN_FILL_STACK_VAL          0xFEEDFACE

/**
 * Invalid value for interrupt status.
 */
#define _TN_UNIX_INTSAVE_DATA_INVALID   -1

/**
 * Variable name that is used for storing interrupts state
 * by macros TN_INTSAVE_DATA and friends
 */
#define TN_INTSAVE_VAR              tn_save_status_reg

/**
 * Eh, I don't like the way Dmitry abstracted this.
 */
extern int TN_INTSAVE_VAR;

/**
 * Declares variable that is used by macros `TN_INT_DIS_SAVE()` and
 * `TN_INT_RESTORE()` for storing status register value.
 *
 * This is not needed for TNeo-on-Unix.
 *
 * @see `TN_INT_DIS_SAVE()`
 * @see `TN_INT_RESTORE()`
 */
/* #define  TN_INTSAVE_DATA        TN_UWord TN_INTSAVE_VAR = _TN_UNIX_INTSAVE_DATA_INVALID */
#define  TN_INTSAVE_DATA

/**
 * The same as `#TN_INTSAVE_DATA` but for using in ISR together with
 * `TN_INT_IDIS_SAVE()`, `TN_INT_IRESTORE()`.
 *
 * This is not needed for TNeo-on-Unix.
 *
 * @see `TN_INT_IDIS_SAVE()`
 * @see `TN_INT_IRESTORE()`
 */
#define  TN_INTSAVE_DATA_INT        TN_INTSAVE_DATA

/**
 * \def TN_INT_DIS_SAVE()
 *
 * Disable interrupts and return previous value of status register,
 * atomically. Similar `tn_arch_sr_save_int_dis()`, but implemented
 * as a macro, so it is potentially faster.
 *
 * Uses `#TN_INTSAVE_DATA` as a temporary storage.
 *
 * @see `#TN_INTSAVE_DATA`
 * @see `tn_arch_sr_save_int_dis()`
 */
#define  TN_INT_DIS_SAVE()      _tn_arch_unix_int_enable(0)

/**
 * \def TN_INT_RESTORE()
 *
 * Restore previously saved status register.
 * Similar to `tn_arch_sr_restore()`, but implemented as a macro,
 * so it is potentially faster.
 *
 * Uses `#TN_INTSAVE_DATA` as a temporary storage.
 *
 * @see `#TN_INTSAVE_DATA`
 * @see `tn_arch_sr_save_int_dis()`
 */
#define  TN_INT_RESTORE()       _tn_arch_unix_int_enable(1)

/**
 * The same as `TN_INT_DIS_SAVE()` but for using in ISR.
 *
 * Uses `#TN_INTSAVE_DATA_INT` as a temporary storage.
 *
 * @see `#TN_INTSAVE_DATA_INT`
 */
#define TN_INT_IDIS_SAVE()       TN_INT_DIS_SAVE()

/**
 * The same as `TN_INT_RESTORE()` but for using in ISR.
 *
 * Uses `#TN_INTSAVE_DATA_INT` as a temporary storage.
 *
 * @see `#TN_INTSAVE_DATA_INT`
 */
#define TN_INT_IRESTORE()        TN_INT_RESTORE()

/**
 * Returns nonzero if interrupts are disabled, zero otherwise.
 */
#define  TN_IS_INT_DISABLED()   (!_tn_arch_unix_int_enabled())

/**
 * Pend context switch from interrupt.
 */
#define _TN_CONTEXT_SWITCH_IPEND_IF_NEEDED()          \
   _tn_context_switch_pend_if_needed()

/**
 * No need for the volatile work-around for TNeo-on-Unix.
 */
#define _TN_VOLATILE_WORKAROUND

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif  // _TN_ARCH_UNIX_H_
