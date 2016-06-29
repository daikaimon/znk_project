#include <Moai_plugin_dev.h>
#include <Znk_stdc.h>
#include <Znk_def_util.h>
#include <Znk_cookie.h>
#include <Znk_htp_hdrs.h>
#include <Znk_str_ptn.h>
#include <Znk_missing_libc.h>
#include <string.h>

Znk_INLINE ZnkVarp
refPostVar( ZnkMyf myf, const char* var_name )
{
	return ZnkMyf_refVar( myf, "post_vars", var_name );
}
Znk_INLINE ZnkVarp
refCookieVar( ZnkMyf myf, const char* var_name )
{
	return ZnkMyf_refVar( myf, "cookie_vars", var_name );
}

/***
 * myf��USERS_password�́A���[�U����̖����I�ȍ폜�L�[�w����V�~�����[�V�������邽�߂�
 * ���̂ł���. pwd��pwdc�̒l�����Ɏ����ݒ肳���ꍇ������A���[�U����̖����I�Ȏw�肩
 * �ۂ��̋�ʂ����Ȃ����߁A���̂悤�ȕϐ���ʓr�������Ă���.
 */
static void
update_pwd( ZnkVarp pwd, ZnkVarp pwdc, ZnkVarp USERS_password )
{
	if( pwd ){
		if( USERS_password && !ZnkS_empty(ZnkVar_cstr(USERS_password)) ){
			/***
			 * ���̏ꍇ�A�Ȍ�pwdc��USERS_password�̒l�Ƃ��Ĕ��s�����͂��ł���.
			 */
			ZnkVar_set_val_Str( pwd, ZnkVar_cstr(USERS_password),
					Znk_MIN(ZnkVar_str_leng(USERS_password),8) );
		} else if( pwdc ){
			/***
			 * pwdc��cookie_vars��ɂ����Đݒ肳��Ă���Ȃ�A���̒l���̗p.
			 */
			ZnkVar_set_val_Str( pwd, ZnkVar_cstr(pwdc), ZnkVar_str_leng(pwdc) );
		} else {
			/***
			 * pwdc��cookie_vars��ɂ����Đݒ肳��Ă��Ȃ��Ȃ�A��l�Ƃ���.
			 * ���̏ꍇ�A�Ȍ�pwdc���ӂ��΂���K���Ȓl�Ƃ��Ď������s�����͂��ł���.
			 */
			ZnkVar_set_val_Str( pwd, "", 0 );
		}
	}
}

bool on_post( ZnkMyf ftr_send, ZnkVarpAry post_vars )
{
	ZnkVarp USERS_futabapt = refPostVar( ftr_send, "USERS_futabapt" );
	ZnkVarp USERS_password = refPostVar( ftr_send, "USERS_password" );
	ZnkVarp USERS_ptua     = refPostVar( ftr_send, "USERS_ptua" );
	ZnkVarp pthb = refPostVar( ftr_send, "pthb" );
	ZnkVarp pthc = refPostVar( ftr_send, "pthc" );
	ZnkVarp pwd  = refPostVar( ftr_send, "pwd" );
	ZnkVarp pwdc = refCookieVar( ftr_send, "pwdc" );
	ZnkVarp dst_ptua = ZnkVarpAry_find_byName( post_vars, "ptua", Znk_strlen_literal("ptua"), false );

	/***
	 * ftr_send��USERS_futabapt����`����Ă��Ȃ��ꍇ��
	 * localStorage�������ł�����Ɠ������ƂȂ�.
	 */
	if( USERS_futabapt ){
		/***
		 * ptfk�֐��̃G�~�����[�g.
		 * �I���W�i���ł͉E�ӂ���̂Ƃ��A���ӂւ̑�����s��Ȃ��悤�ɂ��Ă��邪�A
		 * �����ł͖������ōX�V����. ���ӂ̌Â��l���c���Ă͂Ȃ�Ȃ����߂ł���.
		 */
		if( pthb ){
			ZnkVar_set_val_Str( pthb, ZnkVar_cstr(USERS_futabapt), ZnkVar_str_leng(USERS_futabapt) );
		}
		if( pthc ){
			ZnkVar_set_val_Str( USERS_futabapt, ZnkVar_cstr(pthc), ZnkVar_str_leng(pthc) );
		}
	}

	/***
	 * dst_ptua�ɉ����l���ݒ肳��Ă���� ptua��USERS_ptua�ŏ���������.
	 * �����łȂ���� dst_ptua�ɋ�l��ݒ肷��.
	 */
	if( dst_ptua ){
		if( ZnkVar_str_leng(dst_ptua) ){
			ZnkVar_set_val_Str( dst_ptua, ZnkVar_cstr(USERS_ptua), ZnkVar_str_leng(USERS_ptua) );
		} else {
			ZnkVar_set_val_Str( dst_ptua, "", 0 );
		}
	}

	/***
	 * ����ɂ��u���E�U��ɂ���pwd�̒l�͎Ւf����A
	 * ftr_send��ɂ���l���ւ��ɍ̗p�����.
	 */
	update_pwd( pwd, pwdc, USERS_password );
	return true;
}

static size_t
replaceStr( ZnkStr str, size_t begin,
		const char* old_ptn, size_t old_ptn_leng,
		const char* new_ptn, size_t new_ptn_leng,
		size_t seek_depth )
{
	return ZnkStrPtn_replace( str, begin,
			(uint8_t*)old_ptn, old_ptn_leng,
			(uint8_t*)new_ptn, new_ptn_leng,
			seek_depth );
}

bool on_response( ZnkMyf ftr_send,
		ZnkVarpAry hdr_vars, ZnkStr text, const char* req_urp )
{
	if( text ){
		const char* old_ptn = "���ӂ���</span>";
		char new_ptn[ 4096 ];
		Znk_snprintf( new_ptn, sizeof(new_ptn), "���ӂ��� via Moai[%s]</span>", req_urp );
		replaceStr( text, 0,
				old_ptn, Znk_NPOS,
				new_ptn, Znk_NPOS,
				Znk_NPOS );
	}
	return true;
}

