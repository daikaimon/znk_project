#include "Znk_obj_dary.h"
#include "Znk_bfr.h"
#include "Znk_stdc.h"
#include "Znk_vpod.h"
#include <assert.h>


struct ZnkObjDAryImpl {
	ZnkBfr zkbfr_;
	ZnkElemDeleterFunc elem_deleter_;
};


ZnkObjDAry
ZnkObjDAry_create( ZnkElemDeleterFunc elem_deleter )
{
	ZnkObjDAry obj_dary = (ZnkObjDAry)Znk_malloc( sizeof(struct ZnkObjDAryImpl) );
	obj_dary->zkbfr_ = ZnkBfr_create_null();
	obj_dary->elem_deleter_ = elem_deleter;
	return obj_dary;
}
void
ZnkObjDAry_destroy( ZnkObjDAry obj_dary )
{
	if( obj_dary ){
		/***
		 * elem_deleter_���ݒ肳��Ă���ꍇ�͗v�f�����ɐӔC�����P�[�X�Ƃ݂Ȃ�
		 * �v�f�����ׂĔj��.
		 */
		ZnkObjDAry_clear( obj_dary );

		ZnkBfr_destroy( obj_dary->zkbfr_ );
		Znk_free( obj_dary );
	}
}

size_t
ZnkObjDAry_size( const ZnkObjDAry obj_dary )
{
	return ZnkBfr_size( obj_dary->zkbfr_ ) / sizeof(ZnkObj);
}

ZnkObj
ZnkObjDAry_at( const ZnkObjDAry obj_dary, size_t idx )
{
	assert( idx * sizeof(ZnkObj) < ZnkBfr_size( obj_dary->zkbfr_ ) );
	return (ZnkObj)ZnkBfr_at_ptr( obj_dary->zkbfr_, idx );
}
ZnkObj*
ZnkObjDAry_dary_ptr( ZnkObjDAry obj_dary )
{
	return (ZnkObj*)ZnkBfr_data( obj_dary->zkbfr_ );
}
ZnkElemDeleterFunc
ZnkObjDAry_getElemDeleter( const ZnkObjDAry obj_dary )
{
	return obj_dary->elem_deleter_;
}

bool
ZnkObjDAry_erase( ZnkObjDAry obj_dary, ZnkObj query_obj )
{
	const size_t size = ZnkBfr_size( obj_dary->zkbfr_ );
	const size_t num  = size / sizeof(ZnkObj);
	ZnkObj elem;
	size_t idx;
	for( idx=0; idx<num; ++idx ){
		elem = (ZnkObj)ZnkBfr_at_ptr( obj_dary->zkbfr_, idx );
		if( elem == query_obj ){
			/* found */
			if( obj_dary->elem_deleter_ ){
				if( elem ){
					obj_dary->elem_deleter_( elem );
				}
			}
			if( idx == num-1 ){
				ZnkBfr_pop_bk_ptr( obj_dary->zkbfr_ );
			} else {
				ZnkBfr_erase_ptr( obj_dary->zkbfr_, idx*sizeof(void*), 1 );
			}
			return true;
		}
	}
	return false;
}
bool
ZnkObjDAry_erase_byIdx( ZnkObjDAry obj_dary, size_t idx )
{
	const size_t size = ZnkBfr_size( obj_dary->zkbfr_ );
	const size_t num  = size / sizeof(ZnkObj);
	if( idx < num ){
		if( obj_dary->elem_deleter_ ){
			ZnkObj elem = (ZnkObj)ZnkBfr_at_ptr( obj_dary->zkbfr_, idx );
			if( elem ){
				obj_dary->elem_deleter_( elem );
			}
		}
		if( idx == num-1 ){
			ZnkBfr_pop_bk_ptr( obj_dary->zkbfr_ );
		} else {
			ZnkBfr_erase_ptr( obj_dary->zkbfr_, idx*sizeof(void*), 1 );
		}
		return true;
	}
	return false;
}
size_t
ZnkObjDAry_erase_withQuery( ZnkObjDAry obj_dary, ZnkObjQueryFunc query_func, void* query_arg )
{
	const size_t size = ZnkBfr_size( obj_dary->zkbfr_ );
	const size_t num  = size / sizeof(ZnkObj);

	if( num && query_func ){
		size_t count = 0;
		size_t idx;
		ZnkObj  obj_ptr;
		/***
		 * �����ƍ����Ȏ�@�����邩������Ȃ����A�����ł͋t��������
		 * �V���v����erase_byIdx���Ăяo�������ɂƂ肠�������߂Ă���.
		 */
		for( idx=num-1; idx>0; --idx ){
			obj_ptr = ZnkObjDAry_at( obj_dary, idx );
			if( query_func( obj_ptr, query_arg ) ){
				if( ZnkObjDAry_erase_byIdx( obj_dary, idx ) ){
					++count;
				}
			}
		}
		return count;
	}
	return 0;
}
bool
ZnkObjDAry_pop_bk( ZnkObjDAry obj_dary )
{
	const size_t size = ZnkBfr_size( obj_dary->zkbfr_ );
	const size_t num  = size / sizeof(ZnkObj);
	if( num ){
		const size_t idx = num-1;
		if( obj_dary->elem_deleter_ ){
			ZnkObj elem = (ZnkObj)ZnkBfr_at_ptr( obj_dary->zkbfr_, idx );
			if( elem ){
				obj_dary->elem_deleter_( elem );
			}
		}
		ZnkBfr_pop_bk_ptr( obj_dary->zkbfr_ );
		return true;
	}
	return false;
}
void
ZnkObjDAry_clear( ZnkObjDAry obj_dary )
{
	/***
	 * elem_deleter_���ݒ肳��Ă���ꍇ�͗v�f�����ɐӔC�����P�[�X�Ƃ݂Ȃ�
	 * �v�f�����ׂĔj��.
	 */
	if( obj_dary->elem_deleter_ ){
		const size_t size = ZnkBfr_size( obj_dary->zkbfr_ );
		const size_t num  = size / sizeof(ZnkObj);
		ZnkObj elem;
		size_t idx;
		for( idx=0; idx<num; ++idx ){
			elem = (ZnkObj)ZnkBfr_at_ptr( obj_dary->zkbfr_, idx );
			if( elem ){
				obj_dary->elem_deleter_( elem );
			}
		}
	}
	ZnkBfr_resize( obj_dary->zkbfr_, 0 );
}
void
ZnkObjDAry_resize( ZnkObjDAry obj_dary, size_t size )
{
	if( obj_dary->elem_deleter_ ){
		const size_t old_size = ZnkVPtr_size( obj_dary->zkbfr_ );
		if( size < old_size ){
			ZnkObj elem;
			size_t idx;
			for( idx=size; idx<old_size; ++idx ){
				elem = (ZnkObj)ZnkBfr_at_ptr( obj_dary->zkbfr_, idx );
				if( elem ){
					obj_dary->elem_deleter_( elem );
				}
			}
		}
	}
	ZnkVPtr_resize_fill( obj_dary->zkbfr_, size, NULL );
}

void
ZnkObjDAry_push_bk( ZnkObjDAry obj_dary, ZnkObj obj )
{
	ZnkBfr_push_bk_ptr( obj_dary->zkbfr_, obj );
}
void
ZnkObjDAry_set( ZnkObjDAry obj_dary, size_t idx, ZnkObj obj )
{
	ZnkObj elem = ZnkVPtr_at( obj_dary->zkbfr_, idx );
	if( elem ){
		/* ��elem���܂�delete����K�v������. */
		if( obj_dary->elem_deleter_ ){
			obj_dary->elem_deleter_( elem );
		}
	}
	ZnkVPtr_set( obj_dary->zkbfr_, idx, obj );
}

void
ZnkObjDAry_swap( ZnkObjDAry obj_dary1, ZnkObjDAry obj_dary2 )
{
	Znk_SWAP( struct ZnkObjDAryImpl, *obj_dary1, *obj_dary2 );
}

ZnkObj
ZnkObjDAry_intern( ZnkObjDAry obj_dary, size_t* ans_idx,
		ZnkObjQueryFunc query_func, void* query_arg,
		ZnkObjAllocFunc alloc_func, void* alloc_arg,
		bool* is_newly )
{
	const size_t size = ZnkObjDAry_size( obj_dary );
	ZnkObj ptr = NULL;
	size_t idx;
	size_t null_idx = Znk_NPOS; /* �ŏ���NULL�������ʒu�i�[�p */
	if( query_func ){
		for( idx=0; idx<size; ++idx ){
			ptr = ZnkObjDAry_at( obj_dary, idx );
			if( ptr ){
				if( query_func( ptr, query_arg ) ){
					/***
					 * query_func �𖞑����� ptr �����ɑ��݂���.
					 */
					if( is_newly ){ *is_newly = false; }
					goto FUNC_END;
				}
			} else if( null_idx == Znk_NPOS ){
				null_idx = idx;
			}
		}
	} else {
		/* ���̃P�[�X�ł� NULL ���i�[����Ă���ʒu�݂̂��������� */
		for( idx=0; idx<size; ++idx ){
			ptr = ZnkObjDAry_at( obj_dary, idx );
			if( ptr == NULL ){
				null_idx = idx;
				break;
			}
		}
	}

	if( null_idx != Znk_NPOS ){
		/* �r���ɑ��݂���NULL��������u�󂫁v�ł���A���̈ʒu��idx�Ƃ���. */
		idx = null_idx;
		if( alloc_func ){
			ptr = alloc_func( alloc_arg );
			ZnkObjDAry_set( obj_dary, idx, ptr );
		}
	} else {
		/* �V�K�o�^.
		 * �Ō����ptr��push_bk����. ���̂Ƃ���idx��size�̒l�ƂȂ��Ă���͂�. */
		if( alloc_func ){
			ptr = alloc_func( alloc_arg );
		}
		ZnkObjDAry_push_bk( obj_dary, ptr );
	}
	ptr = ZnkObjDAry_at( obj_dary, idx );
	if( is_newly ){ *is_newly = true; }

FUNC_END:
	if( ans_idx ){ *ans_idx = idx; }
	return ptr;
}

