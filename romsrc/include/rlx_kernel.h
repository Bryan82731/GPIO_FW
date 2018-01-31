/*
 * Realtek Semiconductor Corp.
 *
 * rlx_kernel.h
 *   OS abstraction definition for kernel services
 *
 * Viller Hsiao (villerhsiao@realtek.com.tw)
 * May. 29, 2008
 */
#ifndef  _RLX_KERNEL_H_
#define  _RLX_KERNEL_H_

#include <ucos_ii.h> /* uC/OS-II Kernel service prototype */

#ifdef __cplusplus
extern "C" {
#endif

/* unconditional failure  */
# define RT_FAIL( _msg_ )                          \
  RT_MACRO_START                                   \
  rt_assert_fail(__FILE__, __LINE__, _msg_ );      \
  RT_MACRO_END

/* conditioned assert; if the condition is false, fail. */
# define RT_ASSERT( _bool_, _msg_ )                \
  RT_MACRO_START                                   \
  if ( ! ( _bool_ ) )                              \
    rt_assert_fail(__FILE__, __LINE__, _msg_);     \
  RT_MACRO_END

# define RT_ASSERTC( _bool_ )                      \
  RT_MACRO_START                                   \
  if ( ! ( _bool_ ) )                              \
    rt_assert_fail(__FILE__, __LINE__, #_bool_ );  \
  RT_MACRO_END





/*
 *****************************************************************************************
 * rt_assert_fail()
 *
 * Description: Assertion handler.
 *
 * Arguments  : file -- The source file where assertion happened
 *              line -- The source line where assertion happened
 *              msg  -- Message to print out
 *
 * Returns    : None
 *
 *****************************************************************************************
 */
extern void
rt_assert_fail(const char *file, INT32U line, const char *msg);


#ifdef __cplusplus
}
#endif


#endif /* _RLX_KERNEL_H_ */
