#ifndef INCLUDE_GUARD__Znk_err_h__
#define INCLUDE_GUARD__Znk_err_h__

#include <Znk_base.h>
#include <Znk_str.h>
#include <Znk_s_atom.h>

Znk_EXTERN_C_BEGIN

/**
 * @brief
 *   ZnkErr�^�̒�`.
 *   ���[�U�� ���������o pr__ �ɃA�N�Z�X���Ă͂Ȃ�Ȃ�.
 */
typedef struct { ZnkSAtomPr pr__; } ZnkErr;
#define ZnkErr_D( name ) ZnkErr name = {{ 0,0 }}

/***
 * �\���̂�ZnkErr�����o�����������ꍇ�� 0 �N���A�������ꍇ�ɗp���邱�Ƃ�z�肵������.
 * ���̏������͌���K�{�ł͂Ȃ�.
 */
Znk_INLINE void ZnkErr_set_null( ZnkErr* err ){
	ZnkErr_D( tmp ); *err = tmp;
}

/**
 * @brief
 *   ZnkErr �̓���AtomDB�֐V�����G���[���b�Z�[�W��o�^���A
 *   ����ID�ƂȂ�ZnkErr(���̂�ZnkSAtomPr�^�ł���)��Ԃ�.
 *   ���̊֐����Ăяo���̂Ƀ��[�U�͂Ȃ�珀���͂���Ȃ�.
 *   (ZnkErr_initiate�֐����Ăяo���K�v���͓��ɂȂ�)
 *   internf �֐��ɂ����ẮAprintf�`���̃t�H�[�}�b�g�w����s�����Ƃ��ł��A
 *   ���̌��ʐ�������镶������L�[�Ƃ��ē���AtomDB�֓o�^�����Ƃ������̂ł���.
 *
 * @param ans_err:
 *   intern���ꂽ���ʕԂ����ZnkErr�l���i�[�����������|�C���^���w�肷��.
 *   err�����[�U���擾����K�v���Ȃ��ꍇ������NULL���w�肷�邱�Ƃ��ł���.
 *   �������A���̏ꍇ�ł�������intern����(�܂蕶�����err�̓o�^)���͍̂s����.
 *
 * @exam
 *   �o�^����������́A�ԋp���ꂽZnkErr��ZnkErr_cstr�֐��Ɉ����n�����ƂŎg�p���邱�Ƃ��ł���.
 *   �ȉ��ɁAZnkErr�̓T�^�I�Ȏg�p�������.
 *
 *     bool some_func( ZnkErr* ans_err )
 *     {
 *       // some process...
 *       if( ... ){
 *         // Error A.
 *         ZnkErr_intern( ans_err, "Error A occur." );
 *         return false;
 *       }
 *       if( ... ){
 *         // Error B. (use printf-like interface)
 *         ZnkErr_internf( ans_err, "Error B occur on idx=[%d].", idx );
 *         return false;
 *       }
 *       return true;
 *     }
 *
 *     void test()
 *     {
 *       ZnkErr_D( err ); // ����0�������͕K�{�ł͂Ȃ����A�s�������ꍇ�͂��̂悤�ɂ���.
 *       bool result = some_func( &err );
 *       if( !result ){
 *         // Error Message�̓��e��\��
 *         fprintf( stderr, "ErrorMsg is [%s]\n", ZnkErr_cstr(err) );
 *       }
 *     }
 *
 * @note
 *   ���̊֐��̓X���b�h�Z�[�t�ł���.
 */
void ZnkErr_intern(  ZnkErr* ans_err, const char* err_msg );
void ZnkErr_internf( ZnkErr* ans_err, const char* fmt, ... );

/**
 * @brief
 *   ZnkErr �������������Ԃ�.
 *   ���ꂪ����������Ƃ́AZnkErr_intern(ZnkErr_internf)�Ăяo�����ɓo�^����������ł���.
 *
 * @return
 *   ZnkErr ������������̃|�C���^�ł���AC������ł���.
 *   ���ꂪ����������|�C���^�́AZnkErr_finalize���Ă΂�Ȃ����薳���ɂȂ邱�Ƃ͂Ȃ�.
 *
 * @note
 *   ���̊֐��̓X���b�h�Z�[�t�ł���.
 */
const char* ZnkErr_cstr( const ZnkErr err );

void
ZnkErr_initiate( void );
void
ZnkErr_finalize( void );

Znk_EXTERN_C_END

#endif /* INCLUDE_GUARD */
