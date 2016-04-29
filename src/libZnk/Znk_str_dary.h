#ifndef INCLUDE_GUARD__Znk_str_dary_h__
#define INCLUDE_GUARD__Znk_str_dary_h__

#include <Znk_obj_dary.h>
#include <Znk_str.h>

Znk_EXTERN_C_BEGIN

Znk_DECLARE_HANDLE( ZnkStrDAry );

/**
 * @brief DAry�𐶐�����.
 *
 * @param elem_responsibility:
 *  ���ꂪtrue�̏ꍇ�A�e�v�f�̎����ɂ��Ă���DAry�͐ӔC������.
 *  �܂�destroy, clear, resize, erase, pop_bk���ɂ����āA�e�v�f�ɉ�����deleter�Ƃ��Ă��ꂪ�Ă΂��.
 *  ���̂Ƃ��A���[�U�͊O���Ŏ��������蓖�Ă�object��set, push_bk�Ȃǂœn���Ă��̂܂ܕ��u���Ă���
 *  �`�ɂȂ�(����������ΊO���ł����delete���Ă͂Ȃ�Ȃ�). ���邢�͕ʓrregist�n�̊֐��Ȃǂ�
 *  �p�ӂ��ׂ��ł���.
 *
 *  ���ꂪfalse�̏ꍇ�́A�P�ɂ���DAry���|�C���^�l�̈ꎞ�I�ȃR���e�i�Ƃ��Ďg�p���A
 *  �e�v�f�̎����͊O���̋@�\�ɂ���ĊǗ����Ă��邱�Ƃ��Ӗ�����.
 *  �܂�destroy, clear, resize, erase, pop_bk���ɂ����āA�e�v�f�ɉ�����deleter�͌Ă΂�Ȃ�.
 *  �K�R�I�ɁA���[�U�͓����ŃI�u�W�F�N�g���蓖�Ă���������悤��regist�n�֐������̏ꍇ��
 *  �ǉ��ŗp�ӂ��ׂ��ł͂Ȃ�. ����� clear, resize, erase, push_bk, set �Ȃǂ������ČĂяo����
 *  �����Ǘ��ɕs�����������邩��ł���. �ǂ����Ă�regist�n�֐����K�v�ȏꍇ�͑f����deleter��
 *  �Z�b�g���ׂ��ł���.
 *
 * @note
 *   ��Lregist�n API�Ƃ� regist ������.
 */
ZnkStrDAry
ZnkStrDAry_create( bool elem_responsibility );

/**
 * @brief DAry��j������.
 */
Znk_INLINE void
ZnkStrDAry_destroy( ZnkStrDAry dary ){
	ZnkObjDAry_M_DESTROY( dary );
}

Znk_INLINE size_t
ZnkStrDAry_size( const ZnkStrDAry dary ){
	return ZnkObjDAry_M_SIZE( dary );
}
Znk_INLINE ZnkStr
ZnkStrDAry_at( ZnkStrDAry dary, size_t idx ){
	return ZnkObjDAry_M_AT( dary, idx, ZnkStr );
}
Znk_INLINE ZnkStr*
ZnkStrDAry_dary_ptr( ZnkStrDAry dary ){
	return ZnkObjDAry_M_ARY_PTR( dary, ZnkStr );
}

Znk_INLINE bool
ZnkStrDAry_erase( ZnkStrDAry dary, ZnkStr obj ){
	return ZnkObjDAry_M_ERASE( dary, obj );
}
Znk_INLINE bool
ZnkStrDAry_erase_byIdx( ZnkStrDAry dary, size_t idx ){
	return ZnkObjDAry_M_ERASE_BY_IDX( dary, idx );
}
Znk_INLINE void
ZnkStrDAry_clear( ZnkStrDAry dary ){
	ZnkObjDAry_M_CLEAR( dary );
}
Znk_INLINE void
ZnkStrDAry_resize( ZnkStrDAry dary, size_t size ){
	ZnkObjDAry_M_RESIZE( dary, size );
}

Znk_INLINE void
ZnkStrDAry_push_bk( ZnkStrDAry dary, ZnkStr obj ){
	ZnkObjDAry_M_PUSH_BK( dary, obj );
}
Znk_INLINE void
ZnkStrDAry_set( ZnkStrDAry dary, size_t idx, ZnkStr obj ){
	ZnkObjDAry_M_SET( dary, idx, obj );
}
Znk_INLINE void
ZnkStrDAry_swap( ZnkStrDAry dary1, ZnkStrDAry dary2 ){
	ZnkObjDAry_M_SWAP( dary1, dary2 );
}

int
ZnkStrDAry_push_bk_snprintf( ZnkStrDAry dary, size_t size, const char* fmt, ... );

void
ZnkStrDAry_push_bk_cstr( ZnkStrDAry dary, const char* cstr, size_t cstr_leng );

size_t
ZnkStrDAry_find( ZnkStrDAry dary, size_t pos, const char* ptn, size_t ptn_leng );

Znk_INLINE const char*
ZnkStrDAry_at_cstr( ZnkStrDAry dary, size_t idx ){
	return ZnkStr_cstr( ZnkStrDAry_at( dary, idx ) );
}
Znk_INLINE size_t
ZnkStrDAry_at_leng( ZnkStrDAry dary, size_t idx ){
	return ZnkStr_leng( ZnkStrDAry_at( dary, idx ) );
}

Znk_INLINE void
ZnkStrDAry_reserve( ZnkStrDAry dary, size_t size ){
	const size_t size_save = ZnkStrDAry_size( dary );
	ZnkObjDAry_M_RESIZE( dary, size );
	ZnkObjDAry_M_RESIZE( dary, size_save );
}

Znk_INLINE void
ZnkStrDAry_copy( ZnkStrDAry dst, const ZnkStrDAry src )
{
	const size_t size = ZnkStrDAry_size( src );
	size_t idx;
	ZnkStr str;
	ZnkStrDAry_clear( dst );
	for( idx=0; idx<size; ++idx ){
		str = ZnkStrDAry_at( src, idx );
		ZnkStrDAry_push_bk_cstr( dst, ZnkStr_cstr( str ), ZnkStr_leng( str ) );
	}
}

Znk_EXTERN_C_END

#endif /* INCLUDE_GUARD */
