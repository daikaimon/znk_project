#ifndef Znk_bfr_dary_h__INCLUDED__
#define Znk_bfr_dary_h__INCLUDED__

#include <Znk_obj_dary.h>
#include <Znk_bfr.h>

Znk_EXTERN_C_BEGIN

Znk_DECLARE_HANDLE( ZnkBfrDAry );


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
ZnkBfrDAry
ZnkBfrDAry_create( bool elem_responsibility );

/**
 * @brief DAry��j������.
 */
Znk_INLINE void
ZnkBfrDAry_destroy( ZnkBfrDAry dary ){
	ZnkObjDAry_M_DESTROY( dary );
}

Znk_INLINE size_t
ZnkBfrDAry_size( const ZnkBfrDAry dary ){
	return ZnkObjDAry_M_SIZE( dary );
}
Znk_INLINE ZnkBfr
ZnkBfrDAry_at( ZnkBfrDAry dary, size_t idx ){
	return ZnkObjDAry_M_AT( dary, idx, ZnkBfr );
}
Znk_INLINE ZnkBfr*
ZnkBfrDAry_dary_ptr( ZnkBfrDAry dary ){
	return ZnkObjDAry_M_ARY_PTR( dary, ZnkBfr );
}

Znk_INLINE bool
ZnkBfrDAry_erase( ZnkBfrDAry dary, ZnkBfr obj ){
	return ZnkObjDAry_M_ERASE( dary, obj );
}
Znk_INLINE bool
ZnkBfrDAry_erase_byIdx( ZnkBfrDAry dary, size_t idx ){
	return ZnkObjDAry_M_ERASE_BY_IDX( dary, idx );
}
Znk_INLINE void
ZnkBfrDAry_clear( ZnkBfrDAry dary ){
	ZnkObjDAry_M_CLEAR( dary );
}
Znk_INLINE void
ZnkBfrDAry_resize( ZnkBfrDAry dary, size_t size ){
	ZnkObjDAry_M_RESIZE( dary, size );
}

Znk_INLINE void
ZnkBfrDAry_push_bk( ZnkBfrDAry dary, ZnkBfr obj ){
	ZnkObjDAry_M_PUSH_BK( dary, obj );
}
Znk_INLINE void
ZnkBfrDAry_set( ZnkBfrDAry dary, size_t idx, ZnkBfr obj ){
	ZnkObjDAry_M_SET( dary, idx, obj );
}


Znk_EXTERN_C_END

#endif /* INCLUDE_GUARD */
