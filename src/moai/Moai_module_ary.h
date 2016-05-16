#ifndef INCLUDE_GUARD__Moai_module_ary_h__
#define INCLUDE_GUARD__Moai_module_ary_h__

#include <Znk_obj_ary.h>
#include <Moai_module.h>

Znk_EXTERN_C_BEGIN

Znk_DECLARE_HANDLE( MoaiModuleAry );


/**
 * @brief Ary�𐶐�����.
 *
 * @param elem_responsibility:
 *  ���ꂪtrue�̏ꍇ�A�e�v�f�̎����ɂ��Ă���Ary�͐ӔC������.
 *  �܂�destroy, clear, resize, erase, pop_bk���ɂ����āA�e�v�f�ɉ�����deleter�Ƃ��Ă��ꂪ�Ă΂��.
 *  ���̂Ƃ��A���[�U�͊O���Ŏ��������蓖�Ă�object��set, push_bk�Ȃǂœn���Ă��̂܂ܕ��u���Ă���
 *  �`�ɂȂ�(����������ΊO���ł����delete���Ă͂Ȃ�Ȃ�).
 *
 *  ���ꂪfalse�̏ꍇ�́A�P�ɂ���Ary���|�C���^�l�̈ꎞ�I�ȃR���e�i�Ƃ��Ďg�p���A
 *  �e�v�f�̎����͊O���̋@�\�ɂ���ĊǗ����Ă��邱�Ƃ��Ӗ�����.
 *  �܂�destroy, clear, resize, erase, pop_bk���ɂ����āA�e�v�f�ɉ�����deleter�͌Ă΂�Ȃ�.
 */
MoaiModuleAry
MoaiModuleAry_create( bool elem_responsibility );

/**
 * @brief Ary��j������.
 */
Znk_INLINE void
MoaiModuleAry_destroy( MoaiModuleAry ary ){
	ZnkObjAry_M_DESTROY( ary );
}

Znk_INLINE size_t
MoaiModuleAry_size( const MoaiModuleAry ary ){
	return ZnkObjAry_M_SIZE( ary );
}
Znk_INLINE MoaiModule
MoaiModuleAry_at( MoaiModuleAry ary, size_t idx ){
	return ZnkObjAry_M_AT( ary, idx, MoaiModule );
}
Znk_INLINE MoaiModule*
MoaiModuleAry_ary_ptr( MoaiModuleAry ary ){
	return ZnkObjAry_M_ARY_PTR( ary, MoaiModule );
}

Znk_INLINE bool
MoaiModuleAry_erase( MoaiModuleAry ary, MoaiModule obj ){
	return ZnkObjAry_M_ERASE( ary, obj );
}
Znk_INLINE bool
MoaiModuleAry_erase_byIdx( MoaiModuleAry ary, size_t idx ){
	return ZnkObjAry_M_ERASE_BY_IDX( ary, idx );
}
Znk_INLINE void
MoaiModuleAry_clear( MoaiModuleAry ary ){
	ZnkObjAry_M_CLEAR( ary );
}
Znk_INLINE void
MoaiModuleAry_resize( MoaiModuleAry ary, size_t size ){
	ZnkObjAry_M_RESIZE( ary, size );
}

Znk_INLINE void
MoaiModuleAry_push_bk( MoaiModuleAry ary, MoaiModule obj ){
	ZnkObjAry_M_PUSH_BK( ary, obj );
}
Znk_INLINE void
MoaiModuleAry_set( MoaiModuleAry ary, size_t idx, MoaiModule obj ){
	ZnkObjAry_M_SET( ary, idx, obj );
}

void
MoaiModuleAry_loadAllModules( MoaiModuleAry mod_ary, const ZnkMyf mtgt );

MoaiModule
MoaiModuleAry_find_byHostname( const MoaiModuleAry mod_ary, const char* hostname );

Znk_EXTERN_C_END

#endif /* INCLUDE_GUARD */
