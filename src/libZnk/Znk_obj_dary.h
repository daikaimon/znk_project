#ifndef INCLUDE_GUARD__Znk_obj_dary_h__
#define INCLUDE_GUARD__Znk_obj_dary_h__

#include <Znk_base.h>

Znk_EXTERN_C_BEGIN

Znk_DECLARE_HANDLE( ZnkObj );
typedef struct ZnkObjDAryImpl* ZnkObjDAry;

typedef bool (*ZnkObjQueryFunc)( ZnkObj, void* arg );
typedef ZnkObj (*ZnkObjAllocFunc)( void* arg );
typedef void (*ZnkElemDeleterFunc)( void* );

/**
 * @brief DAry(Dynamic Array)�𐶐�����.
 *
 * @param elem_deleter:
 *  ���ꂪ��NULL�̏ꍇ�A�e�v�f�̎����ɂ��Ă���DAry�͐ӔC������.
 *  �܂�destroy, clear, resize, erase, pop_bk���ɂ����āA�e�v�f�ɉ�����deleter�Ƃ��Ă��ꂪ�Ă΂��.
 *  ���̂Ƃ��A���[�U�͊O���Ŏ��������蓖�Ă�object��set, push_bk�Ȃǂœn���Ă��̂܂ܕ��u���Ă���
 *  �`�ɂȂ�(����������ΊO���ł����delete���Ă͂Ȃ�Ȃ�). ���邢�͕ʓrregist�n�̊֐��Ȃǂ�
 *  �p�ӂ��ׂ��ł���.
 *
 *  ���ꂪNULL�̏ꍇ�́A�P�ɂ���DAry���|�C���^�l�̈ꎞ�I�ȃR���e�i�Ƃ��Ďg�p���A
 *  �e�v�f�̎����͊O���̋@�\�ɂ���ĊǗ����Ă��邱�Ƃ��Ӗ�����.
 *  �܂�destroy, clear, resize, erase, pop_bk���ɂ����āA�e�v�f�ɉ�����deleter�͌Ă΂�Ȃ�.
 *  �K�R�I�ɁA���[�U�͓����ŃI�u�W�F�N�g���蓖�Ă���������悤��regist�n�֐������̏ꍇ��
 *  �ǉ��ŗp�ӂ��ׂ��ł͂Ȃ�. ����� clear, resize, erase, push_bk, set �Ȃǂ������ČĂяo����
 *  �����Ǘ��ɕs�����������邩��ł���. �ǂ����Ă�regist�n�֐����K�v�ȏꍇ�͑f����deleter��
 *  �Z�b�g���ׂ��ł���.
 */
ZnkObjDAry
ZnkObjDAry_create( ZnkElemDeleterFunc elem_deleter );

/**
 * @brief DAry��j������.
 *  DAry��elem_deleter����NULL�ɐݒ肳��Ă���ꍇ�́A
 *  �S�v�f�ɂ��Ă���deleter���Ă΂��.
 */
void
ZnkObjDAry_destroy( ZnkObjDAry obj_dary );

size_t
ZnkObjDAry_size( const ZnkObjDAry obj_dary );

ZnkObj
ZnkObjDAry_at( const ZnkObjDAry obj_dary, size_t idx );
ZnkObj*
ZnkObjDAry_dary_ptr( ZnkObjDAry obj_dary );

ZnkElemDeleterFunc
ZnkObjDAry_getElemDeleter( const ZnkObjDAry obj_dary );

/**
 * @brief DAry�̗v�f���폜����.
 *  DAry��elem_deleter����NULL�ɐݒ肳��Ă���ꍇ�́A
 *  �Ώۗv�f�ɂ��Ă���deleter���Ă΂��.
 */
bool
ZnkObjDAry_erase( ZnkObjDAry obj_dary, ZnkObj obj );
bool
ZnkObjDAry_erase_byIdx( ZnkObjDAry obj_dary, size_t idx );
/**
 * @brief DAry�̗v�f�̂����A�����ɍ��v������̂����ׂč폜����.
 *
 * @note
 *  ���̊֐����R���p�N�V���������Ƃ��ė��p���邱�Ƃ��ł���.
 *  �����A���A���^�C�������̓r���Ȃǂ�erase���Ă΂��Ȃ�炩�̈�����邾���ɂ��Ă����A
 *  ��ŗ]�T������Ƃ��ɂ��̊֐����Ăяo���Ĉ�̂������̂��ꊇ�폜���邱�Ƃō�������
 *  �ȃ����������}���.
 */
size_t
ZnkObjDAry_erase_withQuery( ZnkObjDAry obj_dary, ZnkObjQueryFunc query_func, void* query_arg );
bool
ZnkObjDAry_pop_bk( ZnkObjDAry obj_dary );

/**
 * @brief DAry�̃T�C�Y��0�Ƃ���.
 *  DAry��elem_deleter����NULL�ɐݒ肳��Ă���ꍇ�́A
 *  ���ׂĂ̗v�f�ɂ��Ă���deleter���Ă΂��.
 */
void
ZnkObjDAry_clear( ZnkObjDAry obj_dary );
/**
 * @brief DAry�̃T�C�Y��size�Ƃ���.
 *  DAry��elem_deleter����NULL�ɐݒ肳��Ă���ꍇ�́A
 *  �����ɂ����erase����邷�ׂĂ̗v�f�ɂ��Ă���deleter���Ă΂��.
 *  ����A����ɂ����ẮA�V�K�̈�ɂ�NULL�|�C���^���Z�b�g�����.
 */
void
ZnkObjDAry_resize( ZnkObjDAry obj_dary, size_t size );

void
ZnkObjDAry_push_bk( ZnkObjDAry obj_dary, ZnkObj obj );
void
ZnkObjDAry_set( ZnkObjDAry obj_dary, size_t idx, ZnkObj obj );

void
ZnkObjDAry_swap( ZnkObjDAry obj_dary1, ZnkObjDAry obj_dary2 );

ZnkObj
ZnkObjDAry_intern( ZnkObjDAry obj_dary, size_t* ans_idx,
		ZnkObjQueryFunc query_func, void* query_arg,
		ZnkObjAllocFunc alloc_func, void* alloc_arg,
		bool* is_newly );

/***
 * helper macros.
 */
#define ZnkObjDAry_M_DESTROY( dary )            ZnkObjDAry_destroy( (ZnkObjDAry)dary )
#define ZnkObjDAry_M_SIZE( dary )               ZnkObjDAry_size( (ZnkObjDAry)dary )
#define ZnkObjDAry_M_AT( dary, idx, elem_type ) (elem_type) ZnkObjDAry_at( (ZnkObjDAry)dary, idx )
#define ZnkObjDAry_M_ARY_PTR( dary, elem_type ) (elem_type*)ZnkObjDAry_dary_ptr( (ZnkObjDAry)dary )
#define ZnkObjDAry_M_ERASE( dary, obj )         ZnkObjDAry_erase( (ZnkObjDAry)dary, (ZnkObj)obj )
#define ZnkObjDAry_M_ERASE_BY_IDX( dary, idx )  ZnkObjDAry_erase_byIdx( (ZnkObjDAry)dary, idx )
#define ZnkObjDAry_M_POP_BK( dary )             ZnkObjDAry_pop_bk( (ZnkObjDAry)dary )
#define ZnkObjDAry_M_CLEAR( dary )              ZnkObjDAry_clear( (ZnkObjDAry)dary )
#define ZnkObjDAry_M_RESIZE( dary, size )       ZnkObjDAry_resize( (ZnkObjDAry)dary, size )
#define ZnkObjDAry_M_PUSH_BK( dary, obj )       ZnkObjDAry_push_bk( (ZnkObjDAry)dary, (ZnkObj)obj )
#define ZnkObjDAry_M_SET( dary, idx, obj )      ZnkObjDAry_set( (ZnkObjDAry)dary, idx, (ZnkObj)obj )
#define ZnkObjDAry_M_SWAP( dary1, dary2 )       ZnkObjDAry_swap( (ZnkObjDAry)dary1, (ZnkObjDAry)dary2 )


Znk_EXTERN_C_END

#endif /* INCLUDE_GUARD */
