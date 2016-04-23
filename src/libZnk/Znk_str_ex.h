#ifndef INCLUDE_GUARD__Znk_str_ex_h__
#define INCLUDE_GUARD__Znk_str_ex_h__

#include <Znk_base.h>
#include <Znk_str.h>
#include <Znk_str_dary.h>
#include <Znk_stdc.h>

Znk_EXTERN_C_BEGIN


void
ZnkStrEx_addSplitC( ZnkStrDAry ans_list,
		const char* str, size_t str_leng,
		char delimit_c, const bool count_void_str, size_t expect_size );


void
ZnkStrEx_addSplitCSet( ZnkStrDAry ans_list,
		const char* str,   size_t str_leng,
		const char* chset, size_t chset_leng,
		size_t expect_size );

void
ZnkStrEx_addSplitStr( ZnkStrDAry ans_vstr, const char* str, size_t str_leng,
		const char* delimiter, size_t delimiter_leng,
		const bool count_void_str, size_t expect_size );

void
ZnkStrEx_addJoin( ZnkStr ans, const ZnkStrDAry str_list,
		const char* connector, size_t connector_leng, size_t expect_elem_leng );

void
ZnkStrEx_addQuotedTokens( ZnkStrDAry ans_vstr,
		const char* str, size_t str_leng,
		const char* begin_quot, size_t begin_quot_leng,
		const char* end_quot,   size_t end_quot_leng,
		size_t expect_size );

/**
 * str ��^���A���� begin�Ŏw�肵���ʒu�ȍ~���X�L�������Aptn �Ŏw�肵��
 * �p�^�[���������ŏ��̈ʒu�� end �Ƃ���. ���̂Ƃ� str �ɂ�����
 * [ begin, end ) �͈̔͂̕�����������ł� Front �ƌĂсA���̊֐��ł�
 * ������擾����. ������ ptn �����݂��Ȃ��ꍇ�� end �� Znk_NPOS�ƂȂ�A
 * ���̏ꍇ Front �� [ begin, str_leng ) �܂ł͈̔͂��������̂Ƃ���.
 *
 * @param begin
 *  str �ɂ�����X�L�����J�n�ʒu.
 *  Znk_NPOS �� str_leng ���z����l���w�肵���ꍇ�AFront�͋�Ƃ݂Ȃ����.
 *  ���Ȃ킿 begin�̒l���̂��̂��Ԃ�Aans�ɂ͉����A������Ȃ�.
 *
 * @param ans:
 *  ������ ��NULL�� ZnkStr�l���w�肵���ꍇ�́A��L�ɂ����� Front �̕�����
 *  ans �̌��ɘA�������. ans �� NULL�̏ꍇ�͂��̏����͏ȗ������.
 *
 * @return 
 *  end ��Ԃ�.
 *  �������Aptn�����݂��Ȃ������ꍇ�� Znk_NPOS ��Ԃ�.
 *  ���̂Ƃ� ans �ɂ͏�L�Ɏ���Front�͈̔͂��A���������̂Ƃ���.
 *  �����ŁAZnk_NPOS �̑���� str_leng��Ԃ��d�l�ɂ��Ă��悩�������A���̏ꍇ
 *  �Ăяo������ str �̎��ۂ̒�������ɔc�����Ă����K�v����������. ����͏ꍇ��
 *  ����Ă͖��ʂȂ̂ŁAZnk_NPOS��Ԃ��d�l�ɗ��������`�ƂȂ���.
 *
 *  �����l�Ƃ��� begin < str_leng && ptn_leng ����������Ȃ������ꍇ�́A
 *  ptn �̒l�����ł���Abegin �̒l���̂��̂��Ԃ�(���̂Ƃ� ans �ɂ�
 *  �����A������Ȃ�).
 *
 * @exam
 *  str = 'ABCdefghiJKLmn'
 *  begin=3,
 *  pattern='JKL'
 *  end(�߂�l) = 9
 *  ans = 'defghi'
 */
size_t
ZnkStrEx_getFront_byPattern( const char* str, size_t str_leng,
		size_t begin, const char* ptn, size_t ptn_leng,
		ZnkStr ans );

size_t
ZnkStrEx_getBack_byPattern( const char* str, size_t str_leng,
		size_t begin, const char* ptn, size_t ptn_leng,
		ZnkStr ans );


size_t
ZnkStrEx_getKeyAndValEx( const char* str, size_t begin, size_t end,
		size_t* ans_key_begin, size_t* ans_key_end,
		size_t* ans_val_begin, size_t* ans_val_end,
		const char* delimiter,  size_t delimiter_leng,
		const char* skip_chset, size_t skip_chset_leng,
		ZnkStr ans_key, ZnkStr ans_val );
Znk_INLINE size_t
ZnkStrEx_getKeyAndVal( const char* str, size_t begin, size_t end,
		const char* delimiter, const char* skip_chset,
		ZnkStr ans_key, ZnkStr ans_val )
{
	return ZnkStrEx_getKeyAndValEx( str, begin, end,
			NULL, NULL, NULL, NULL,
			delimiter,  Znk_NPOS,
			skip_chset, Znk_NPOS,
			ans_key, ans_val );
}

bool
ZnkStrEx_removeFrontCSet( ZnkStr str, const char* skip_chset, size_t skip_chset_leng );
bool
ZnkStrEx_removeBackCSet( ZnkStr str, const char* skip_chset, size_t skip_chset_leng );
/**
 * @brief
 * ZnkStrEx_removeBackCSet �� ZnkStrEx_removeFrontCSet �����̏��Ԃōs��.
 */
Znk_INLINE void 
ZnkStrEx_removeSideCSet( ZnkStr str, const char* skip_chset, size_t skip_chset_leng ){
	ZnkStrEx_removeBackCSet ( str, skip_chset, skip_chset_leng );
	ZnkStrEx_removeFrontCSet( str, skip_chset, skip_chset_leng );
}

bool 
ZnkStrEx_chompStr( ZnkStr str, size_t pos, const char* ptn, size_t ptn_leng );
Znk_INLINE bool 
ZnkStrEx_chompFirstStr( ZnkStr str, const char* ptn, size_t ptn_leng ){
	return ZnkStrEx_chompStr( str, 0, ptn, ptn_leng );
}
Znk_INLINE bool 
ZnkStrEx_chompLastStr( ZnkStr str, const char* ptn, size_t ptn_leng ){
	return ZnkStrEx_chompStr( str, Znk_NPOS, ptn, ptn_leng );
}

void
ZnkStrEx_addRepeatC( ZnkStr ans, char c, size_t num );

void
ZnkStrEx_addEmbededStr( ZnkStr ans,
		char c, size_t num,
		const char* emb_str, size_t emb_str_leng,
		char positioning_mode );

size_t
ZnkStrEx_replace_BF( ZnkStr str, size_t begin,
		const char* old_ptn, size_t old_ptn_leng,
		const char* new_ptn, size_t new_ptn_leng,
		size_t seek_depth );

size_t
ZnkStrEx_replace_BMS( ZnkStr str, size_t begin,
		const char* old_ptn, size_t old_ptn_leng, const size_t* old_ptn_occ_table,
		const char* new_ptn, size_t new_ptn_leng,
		size_t seek_depth );

Znk_EXTERN_C_END

#endif /* INCLUDE_GUARD */
