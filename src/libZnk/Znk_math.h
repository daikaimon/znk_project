#ifndef INCLUDE_GUARD__Znk_math_h__
#define INCLUDE_GUARD__Znk_math_h__

#include <Znk_base.h>
#include <math.h>

/***
 * VC���A���ꂪ��`����Ă��Ȃ�math.h������.
 */
#ifndef   M_PI
#  define M_PI   3.14159265358979323846
#endif

#ifndef   M_LN2
#  define M_LN2  0.69314718055994530942
#endif

#ifndef   M_LN10
#  define M_LN10 2.30258509299404568402
#endif


/***
 * �K���}�֐�
 */
double
ZnkMath_gamma( double z );

#endif /* INCLUDE_GUARD */
