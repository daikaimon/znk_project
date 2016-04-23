#ifndef INCLUDE_GUARD__Moai_module_ary_h__
#define INCLUDE_GUARD__Moai_module_ary_h__

#include <Znk_obj_dary.h>
#include <Moai_module.h>

Znk_EXTERN_C_BEGIN

Znk_DECLARE_HANDLE( MoaiModuleAry );


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
MoaiModuleAry
MoaiModuleAry_create( bool elem_responsibility );

/**
 * @brief DAry��j������.
 */
Znk_INLINE void
MoaiModuleAry_destroy( MoaiModuleAry dary ){
	ZnkObjDAry_M_DESTROY( dary );
}

Znk_INLINE size_t
MoaiModuleAry_size( const MoaiModuleAry dary ){
	return ZnkObjDAry_M_SIZE( dary );
}
Znk_INLINE MoaiModule
MoaiModuleAry_at( MoaiModuleAry dary, size_t idx ){
	return ZnkObjDAry_M_AT( dary, idx, MoaiModule );
}
Znk_INLINE MoaiModule*
MoaiModuleAry_dary_ptr( MoaiModuleAry dary ){
	return ZnkObjDAry_M_ARY_PTR( dary, MoaiModule );
}

Znk_INLINE bool
MoaiModuleAry_erase( MoaiModuleAry dary, MoaiModule obj ){
	return ZnkObjDAry_M_ERASE( dary, obj );
}
Znk_INLINE bool
MoaiModuleAry_erase_byIdx( MoaiModuleAry dary, size_t idx ){
	return ZnkObjDAry_M_ERASE_BY_IDX( dary, idx );
}
Znk_INLINE void
MoaiModuleAry_clear( MoaiModuleAry dary ){
	ZnkObjDAry_M_CLEAR( dary );
}
Znk_INLINE void
MoaiModuleAry_resize( MoaiModuleAry dary, size_t size ){
	ZnkObjDAry_M_RESIZE( dary, size );
}

Znk_INLINE void
MoaiModuleAry_push_bk( MoaiModuleAry dary, MoaiModule obj ){
	ZnkObjDAry_M_PUSH_BK( dary, obj );
}
Znk_INLINE void
MoaiModuleAry_set( MoaiModuleAry dary, size_t idx, MoaiModule obj ){
	ZnkObjDAry_M_SET( dary, idx, obj );
}

void
MoaiModuleAry_loadAllModules( MoaiModuleAry mod_ary, const ZnkMyf mtgt );

MoaiModule
MoaiModuleAry_find_byHostname( const MoaiModuleAry mod_ary, const ZnkMyf mtgt, const char* hostname );

Znk_EXTERN_C_END

#endif /* INCLUDE_GUARD */
