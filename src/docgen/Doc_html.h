#ifndef INCLUDE_GUARD__Doc_html_make_h__
#define INCLUDE_GUARD__Doc_html_make_h__

#include <Znk_str.h>

Znk_EXTERN_C_BEGIN

/**
 * @brief
 *   in����HTML���쐬����.
 */
bool
DocHtml_make( const char* in_dir, const char* out_dir, ZnkStr ermsg );

/***
 * �_�~�[�R�����g.
 */

/**
 * @brief
 *   �Ō�Ƀ|�[�Y���邩�ۂ���Ԃ�.
 */
bool
DocHtml_isPauseOnExit( void );

Znk_EXTERN_C_END

#endif /* INCLUDE_GUARD */
