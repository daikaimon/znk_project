#ifndef INCLUDE_GUARD__Rano_dir_util_h__
#define INCLUDE_GUARD__Rano_dir_util_h__

#include <Znk_str.h>
#include <Znk_dir_recursive.h>

Znk_EXTERN_C_BEGIN

typedef bool (*RanoDirUtilFuncT)( const char* file_path, void* arg );

typedef bool (*RanoDirUtilFilterFuncT)( const char* in_file_path, const char* out_file_path, void* arg );

/**
 * @brief
 *   src_dir�f�B���N�g���z�����ċA�I��dst_dir�ֈړ�����.
 */
void
RanoDirUtil_moveDir( const char* src_dir, const char* dst_dir,
		const char* title, ZnkStr ermsg,
		RanoDirUtilFuncT is_processFile_func, void* is_processFile_arg );

/**
 * @brief
 *   top_dir�f�B���N�g���z�������ׂč폜����.
 */
void
RanoDirUtil_removeDir( const char* topdir,
		const char* title, ZnkStr ermsg,
		RanoDirUtilFuncT is_processFile_func, void* is_processFile_arg );

/**
 * @brief
 *   src_dir�f�B���N�g���z�����ċA�I��filt�������A
 *   filt�����ς݂̌��ʂ�dst_dir�֍ċA�I�ɏo�͂���.
 *   (src_dir�f�B���N�g���z���͉����ύX����Ȃ�)
 *
 * @param is_processFile_func:
 *   filt�����Ώۂ̃t�@�C�����ǂ����𔻒肷��֐���^����.
 *   NULL���w�肵���ꍇ�A���ׂẴt�@�C����ΏۂƂ���.
 *
 * @param is_processFile_arg:
 *   is_processFile_func �֓n������.
 *
 * @param filterFile_func:
 *   filt���������������֐���^����.
 *   ������NULL���w�肷�邱�Ƃ͂ł��Ȃ�.
 *
 * @param filterFile_arg:
 *   filterFile_func �֓n������.
 *
 * @param isIgnoreDir_func:
 *   filt�����̑ΏۂƂ������Ȃ��f�B���N�g��(IgnoreDir)�𔻒肷��֐���^����.
 *   NULL���w�肵���ꍇ�A���̂悤�ȃf�B���N�g���͂Ȃ����̂Ƃ���.
 *   ���Ȃ킿���ׂẴf�B���N�g����ΏۂƂ���.
 *
 * @param isIgnoreDir_arg:
 *   isIgnoreDir_func �֓n������.
 */
void
RanoDirUtil_filterDir( const char* src_dir, const char* dst_dir,
		const char* title, ZnkStr ermsg,
		RanoDirUtilFuncT       is_processFile_func, void* is_processFile_arg,
		RanoDirUtilFilterFuncT filterFile_func,     void* filterFile_arg,
		ZnkDirRecursiveFuncT   isIgnoreDir_func,    void* isIgnoreDir_arg );

/**
 * @brief
 *   src_dir�f�B���N�g���z�����ċA�I��dst_dir�փC���X�g�[������.
 *
 * @param is_processFile_func:
 *   copy�����Ώۂ̃t�@�C�����ǂ����𔻒肷��֐���^����.
 *   NULL���w�肵���ꍇ�A���ׂẴt�@�C����ΏۂƂ���.
 *
 * @param is_processFile_arg:
 *   is_processFile_func �֓n������.
 *
 * @param isIgnoreDir_func:
 *   copy�����̑ΏۂƂ������Ȃ��f�B���N�g��(IgnoreDir)�𔻒肷��֐���^����.
 *   NULL���w�肵���ꍇ�A���̂悤�ȃf�B���N�g���͂Ȃ����̂Ƃ���.
 *   ���Ȃ킿���ׂẴf�B���N�g����ΏۂƂ���.
 *
 * @param isIgnoreDir_arg:
 *   isIgnoreDir_func �֓n������.
 */
void
RanoDirUtil_installDir( const char* src_dir, const char* dst_dir,
		const char* title, ZnkStr ermsg,
		RanoDirUtilFuncT     is_processFile_func, void* is_processFile_arg,
		ZnkDirRecursiveFuncT isIgnoreDir_func,    void* isIgnoreDir_arg );


/**
 * @brief
 *   path��tail�������܂���f�B���N�g��(�v��tail��荶��)�����ׂ�mkdir����.
 */
bool
RanoDirUtil_mkdirOfTailContain( const char* path, ZnkStr ermsg );

Znk_EXTERN_C_END

#endif /* INCLUDE_GUARD */
