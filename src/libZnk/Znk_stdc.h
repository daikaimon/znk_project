#ifndef INCLUDE_GUARD__Znk_c_std_h__
#define INCLUDE_GUARD__Znk_c_std_h__

#include <Znk_base.h>
#include <stdarg.h>

Znk_EXTERN_C_BEGIN

void* Znk_malloc( size_t size );
void  Znk_free( void* ptr );
void* Znk_realloc( void* ptr, size_t size );
void* Znk_calloc( size_t ary_num, size_t elem_size );
void* Znk_alloc0( size_t size );
char* Znk_strdup( const char* cstr );

void* Znk_memcpy( void* dst, const void* src, size_t size );
void* Znk_memmove( void* dst, const void* src, size_t size );
void* Znk_memset( void* dst, uint8_t c, size_t size );
int   Znk_memcmp( const void* m1, const void* m2, size_t size );
void* Znk_memchr( const void* m, int c, size_t size );

size_t  Znk_strlen( const char* cstr );
/**
 * @brief
 *   �����񃊃e�����̏ꍇ�A���̃}�N���̕���strlen��荂���ɕ����񒷂��擾�ł���.
 *   literal_cstr�ɂ͕K�������񃊃e������^���邱��.
 *
 * @note
 *   ����literal_cstr�ɕ����񃊃e�����ȊO(��ʂ̕�����ϐ���z��Ȃ�)���^����ꂽ�ꍇ�A
 *   �ꉞ�R���p�C���G���[�ƂȂ�悤�Ɍ��� "" ��A�����Ă���.
 *
 *   ���̃}�N���̕Ԃ��l��strlen�Ɗ��S�ɂ͓����łȂ����Ƃɒ��ӂ���.
 *   �Ⴆ�΁Aliteral_cstr �Ƃ��� "abcdef" ���^����ꂽ�ꍇ�͊��Ғʂ� 6 ���Ԃ���邪
 *   literal_cstr �Ƃ��� "ab\0cdef" ���^����ꂽ�ꍇ�� 2 �ł͂Ȃ� 7 ���Ԃ����.
 *   �����sizeof�������񃊃e������P���ɔz��Ƃ݂Ȃ��A���̃T�C�Y��Ԃ��Ă��邽�߂ł���.
 */
#define Znk_strlen_literal( literal_cstr ) ( sizeof( literal_cstr "" ) - 1 )

int Znk_strcmp(  const char* s1, const char* s2 );
int Znk_strncmp( const char* s1, const char* s2, size_t leng );

char* Znk_getenv( const char* varname );


Znk_DECLARE_HANDLE( ZnkFile );

ZnkFile Znk_Internal_getStdFP( int no );
void Znk_Internal_setMode( int no, bool is_binary_mode );

Znk_INLINE ZnkFile ZnkF_stdin(  void ){ return Znk_Internal_getStdFP( 0 ); }
Znk_INLINE ZnkFile ZnkF_stdout( void ){ return Znk_Internal_getStdFP( 1 ); }
Znk_INLINE ZnkFile ZnkF_stderr( void ){ return Znk_Internal_getStdFP( 2 ); }


ZnkFile
ZnkF_fopen( const char* filename, const char* mode );
void
ZnkF_fclose( ZnkFile fp );

int
ZnkF_fgetc( ZnkFile fp );
int
ZnkF_fputc( int c, ZnkFile fp );

char*
ZnkF_fgets( char* buf, size_t size, ZnkFile fp );
int
ZnkF_fputs( const char* c_str, ZnkFile fp );

size_t
ZnkF_fread( uint8_t* buf, const size_t blk_size, const size_t nmemb, ZnkFile fp );
size_t
ZnkF_fwrite( const uint8_t* buf, const size_t blk_size, const size_t nmemb, ZnkFile fp );

bool ZnkF_feof( ZnkFile fp );
bool ZnkF_fflush( ZnkFile fp );
bool ZnkF_fseek( ZnkFile fp, long offset, int whence );
bool ZnkF_fseek_i64( ZnkFile fp, int64_t offset, int whence );
int64_t ZnkF_ftell_i64( ZnkFile fp );

int
ZnkF_vfprintf( ZnkFile fp, const char* fmt, va_list ap );
int
ZnkF_fprintf( ZnkFile fp, const char* fmt, ... );
int
ZnkF_printf( const char* fmt, ... );
int
ZnkF_printf_e( const char* fmt, ... );

ZnkFile
ZnkF_freopen( const char* filename, const char* mode, ZnkFile fp );

Znk_EXTERN_C_END

#endif /* INCLUDE_GUARD */
