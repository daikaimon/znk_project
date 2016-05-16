#include <Moai_plugin_dev.h>
#include <Znk_stdc.h>
#include <Znk_def_util.h>
#include <Znk_cookie.h>
#include <Znk_htp_hdrs.h>
#include <string.h>

static ZnkVarp
refPostVar( ZnkMyf myf, const char* var_name )
{
	return ZnkMyf_refVar( myf, "post_vars", var_name );
}
static ZnkVarp
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
			 * ���̏ꍇ�A�Ȍ�pwdc���ӂ��΂���K���Ȓl�Ƃ��Ď������s���s�����͂��ł���.
			 */
			ZnkVar_set_val_Str( pwd, "", 0 );
		}
	}
}

bool on_post_before( ZnkMyf myf )
{
	ZnkVarp USERS_futabapt = refPostVar( myf, "USERS_futabapt" );
	ZnkVarp USERS_password = refPostVar( myf, "USERS_password" );
	ZnkVarp pthb = refPostVar( myf, "pthb" );
	ZnkVarp pthc = refPostVar( myf, "pthc" );
	ZnkVarp pwd  = refPostVar( myf, "pwd" );
	ZnkVarp pwdc = refCookieVar( myf, "pwdc" );

	ZnkF_printf_e( "plugin_futaba : on_post_before\n" );
	/***
	 * myf��USERS_futabapt����`����Ă��Ȃ��ꍇ��
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
	 * ����ɂ��u���E�U��ɂ���pwd�̒l�͎Ւf����A
	 * myf��ɂ���l���ւ��ɍ̗p�����.
	 */
	update_pwd( pwd, pwdc, USERS_password );
	return true;
}
bool on_response_hdr( ZnkMyf myf, ZnkVarpAry hdr_vars )
{
	ZnkVarp set_cookie = ZnkHtpHdrs_find_literal( hdr_vars, "Set-Cookie" );

	/***
	 * set_cookie�̉�͂��Amyf��ɂ�����cookie_vars���X�V����.
	 */
	if( set_cookie ){
		const size_t val_size = ZnkHtpHdrs_val_size( set_cookie );
		size_t       val_idx  = 0;
		ZnkVarpAry   cok_vars = ZnkMyf_find_vars( myf, "cookie_vars" );

		for( val_idx=0; val_idx<val_size; ++val_idx ){
			const char* p = ZnkHtpHdrs_val_cstr( set_cookie, val_idx );
			const char* e = p + strlen(p);
			const char* q = memchr( p, ';', (size_t)(e-p) );
			if( q == NULL ){
				q = e;
			}
			ZnkCookie_regist_byAssignmentStatement( cok_vars, p, (size_t)(q-p) );
		}
		return true;
	}
	return false;
}

