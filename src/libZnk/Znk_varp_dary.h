#ifndef Znk_varp_dary_h__INCLUDED__
#define Znk_varp_dary_h__INCLUDED__

#include <Znk_obj_dary.h>
#include <Znk_var.h>

Znk_EXTERN_C_BEGIN

Znk_DECLARE_HANDLE( ZnkVarpDAry );


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
ZnkVarpDAry
ZnkVarpDAry_create( bool elem_responsibility );

/**
 * @brief DAry��j������.
 */
Znk_INLINE void
ZnkVarpDAry_destroy( ZnkVarpDAry dary ){
	ZnkObjDAry_M_DESTROY( dary );
}

Znk_INLINE size_t
ZnkVarpDAry_size( const ZnkVarpDAry dary ){
	return ZnkObjDAry_M_SIZE( dary );
}
Znk_INLINE ZnkVarp
ZnkVarpDAry_at( ZnkVarpDAry dary, size_t idx ){
	return ZnkObjDAry_M_AT( dary, idx, ZnkVarp );
}
Znk_INLINE ZnkVarp*
ZnkVarpDAry_dary_ptr( ZnkVarpDAry dary ){
	return ZnkObjDAry_M_ARY_PTR( dary, ZnkVarp );
}

Znk_INLINE bool
ZnkVarpDAry_erase( ZnkVarpDAry dary, ZnkVarp obj ){
	return ZnkObjDAry_M_ERASE( dary, obj );
}
Znk_INLINE bool
ZnkVarpDAry_erase_byIdx( ZnkVarpDAry dary, size_t idx ){
	return ZnkObjDAry_M_ERASE_BY_IDX( dary, idx );
}
Znk_INLINE void
ZnkVarpDAry_clear( ZnkVarpDAry dary ){
	ZnkObjDAry_M_CLEAR( dary );
}
Znk_INLINE void
ZnkVarpDAry_resize( ZnkVarpDAry dary, size_t size ){
	ZnkObjDAry_M_RESIZE( dary, size );
}

Znk_INLINE void
ZnkVarpDAry_push_bk( ZnkVarpDAry dary, ZnkVarp obj ){
	ZnkObjDAry_M_PUSH_BK( dary, obj );
}
Znk_INLINE void
ZnkVarpDAry_set( ZnkVarpDAry dary, size_t idx, ZnkVarp obj ){
	ZnkObjDAry_M_SET( dary, idx, obj );
}
ZnkVarp
ZnkVarpDAry_find_byName( ZnkVarpDAry dary,
		const char* query_name, size_t query_name_leng, bool use_eqCase );

#define ZnkVarpDAry_find_byName_literal( dary, query_name, use_eqCase ) \
	ZnkVarpDAry_find_byName( dary, query_name, Znk_strlen_literal(query_name), use_eqCase )

Znk_EXTERN_C_END

#endif /* INCLUDE_GUARD */
