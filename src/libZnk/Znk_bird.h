#ifndef INCLUDE_GUARD__Znk_bird_h__
#define INCLUDE_GUARD__Znk_bird_h__

#include <Znk_str.h>

Znk_EXTERN_C_BEGIN

typedef struct ZnkBirdImpl* ZnkBird;

typedef bool (*ZnkBirdProcessFunc)( const ZnkBird bird,
		ZnkStr dst,
		const char* src,
		const char* key,
		size_t begin_idx, /* src��ɂ�����key�̊J�n�ʒu��hint�Ƃ��ė^���� */
		void* func_arg );

ZnkBird
ZnkBird_create( const char* key_beginer, const char* key_ender );

void
ZnkBird_destroy( ZnkBird bird );

void
ZnkBird_regist( ZnkBird bird, const char* key, const char* val );

void
ZnkBird_setFunc( ZnkBird bird, ZnkBirdProcessFunc func, void* func_arg );

const char*
ZnkBird_at( ZnkBird bird, const char* key );

const bool
ZnkBird_exist( ZnkBird bird, const char* key );

/**
 * @brief
 * regist�ɂ����ēo�^����key��val��S�N���A.
 */
void
ZnkBird_clear( ZnkBird bird );

/***
 * #[key]# Directive�����ߍ��܂ꂽ�������W�J����.
 *
 * ���̏�����src���p�ɂɕύX�����悤�ȏ����Ɍ����Ă���.
 * ����ŁAsrc�͏�������Œ�ł���Abird����hash��val�̒l(key�͕s��)���p�ɂɕς��悤�ȏ����̏ꍇ�A
 * ���̊֐��ł��ꉞ�Ή��ł��邪�A�O�����ɂ������ƍ����Ȏ������l���邱�Ƃ��ł���.
 * �����MsgObj�Ƃ������W���[���ŏ��������\��.
 *
 * ���Adst��src�͓����ZnkStr�ł����Ă͂Ȃ�Ȃ�.
 */
bool ZnkBird_extend( const ZnkBird bird,
		ZnkStr dst, const char* src, size_t expected_size );

bool ZnkBird_extend_self( const ZnkBird bird, ZnkStr src, size_t expected_size );


Znk_EXTERN_C_END

#endif /* INCLUDE_GUARD */
