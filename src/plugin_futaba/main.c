#include <Moai_plugin_dev.h>
#include <Znk_stdc.h>
#include <Znk_def_util.h>
#include <Znk_cookie.h>
#include <Znk_htp_hdrs.h>
#include <Znk_s_base.h>
#include <Znk_str_ptn.h>
#include <Znk_str_ex.h>
#include <Znk_missing_libc.h>
#include <Znk_mem_find.h>
#include <string.h>
#include <ctype.h>

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

static bool
isInteger( const char* cstr )
{
	const char* p = cstr;
	while( *p ){
		if( !isdigit( *p ) ){
			return false;
		}
		++p;
	}
	return (bool)( p != cstr );
}
static bool
isUntouchable_ptua( ZnkVarp dst_ptua )
{
	/***
	 * dst_ptua���ȉ��̃P�[�X�̏ꍇ�A�l��ύX���Ă͂Ȃ�Ȃ�.
	 *
	 *   ��l
	 *   ����
	 */
	return ZnkVar_str_leng(dst_ptua) == 0 || isInteger(ZnkVar_cstr(dst_ptua));
}

bool on_post( ZnkMyf ftr_send, ZnkVarpAry hdr_vars, ZnkVarpAry post_vars )
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

	if( dst_ptua ){
		ZnkVarp     real_ua      = ZnkHtpHdrs_find( hdr_vars, "User-Agent", Znk_strlen_literal("User-Agent") );
		const char* real_ua_cstr = ZnkHtpHdrs_val_cstr( real_ua, 0 );
		/* �܂��ȈՂȃP�[�X��e�� */
		if( !isUntouchable_ptua( dst_ptua ) ){
#if 0
			/***
			 * ���݂ӂ��΂ł�ptua�̎��l��transient�Ȏd�l�Ƃ��邱�ƂŋU�������킷�헪���Ƃ��Ă���.
			 * ���̂��߁A�P���ȑ�����ʗp���Ȃ�.
			 */
			ZnkVar_set_val_Str( dst_ptua, ZnkVar_cstr(USERS_ptua), ZnkVar_str_leng(USERS_ptua) );
#else
			/* �Ƃ肠����brute force algorism�ŏ\���ł��낤 */
			ZnkStr dst_ptua_str = ZnkVar_str( dst_ptua );
			ZnkStrEx_replace_BF( dst_ptua_str, 0,
					real_ua_cstr, strlen(real_ua_cstr),
					ZnkVar_cstr(USERS_ptua), ZnkVar_str_leng(USERS_ptua),
					Znk_NPOS );
#endif
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

/***
 * ����͂Ȃ��Ȃ��g�������ł���.
 * �������W���[�����[�X�Ŕėp���[�e�B���e�B�ɏ��i�\��.
 */
static void
replacePtnInnerRange( ZnkStr text, const char* begin_ptn, const char* end_ptn,
		const char* old_ptn, const char* new_ptn )
{
	ZnkSRef sref_begin;
	ZnkSRef sref_end;
	ZnkSRef sref_old;
	ZnkSRef sref_new;
	size_t occ_tbl_begin[ 256 ];
	size_t occ_tbl_end[ 256 ];
	size_t occ_tbl_target[ 256 ];
	size_t pos   = 0;
	size_t begin = 0;
	size_t end   = 0;
	size_t text_leng = ZnkStr_leng( text );
	ZnkStr tmp = ZnkStr_new( "" );

	sref_begin.cstr_ = begin_ptn;
	sref_begin.leng_ = strlen( begin_ptn );
	sref_end.cstr_   = end_ptn;
	sref_end.leng_   = strlen( end_ptn );
	sref_old.cstr_   = old_ptn;
	sref_old.leng_   = strlen( old_ptn );
	sref_new.cstr_   = new_ptn;
	sref_new.leng_   = strlen( new_ptn );

	ZnkMem_getLOccTable_forBMS( occ_tbl_begin,  (uint8_t*)sref_begin.cstr_, sref_begin.leng_ );
	ZnkMem_getLOccTable_forBMS( occ_tbl_end,    (uint8_t*)sref_end.cstr_,   sref_end.leng_ );
	ZnkMem_getLOccTable_forBMS( occ_tbl_target, (uint8_t*)sref_old.cstr_,   sref_old.leng_ );

	while( true ){
		text_leng = ZnkStr_leng( text );
		if( pos >= text_leng ){
			break;
		}
		begin = ZnkMem_lfind_data_BMS( (uint8_t*)ZnkStr_cstr(text)+pos, text_leng-pos,
				(uint8_t*)sref_begin.cstr_, sref_begin.leng_, 1, occ_tbl_begin );
		if( begin == Znk_NPOS ){
			break;
		}
		begin += pos + sref_begin.leng_;
		end   = ZnkMem_lfind_data_BMS( (uint8_t*)ZnkStr_cstr(text)+begin, text_leng-begin,
				(uint8_t*)sref_end.cstr_, sref_end.leng_, 1, occ_tbl_end );
		if( end == Znk_NPOS ){
			end = text_leng;
		} else {
			end += begin;
		}
		ZnkStr_assign( tmp, 0, ZnkStr_cstr( text ) + begin, end-begin );
		ZnkStrEx_replace_BMS( tmp, 0,
				sref_old.cstr_, sref_old.leng_, occ_tbl_target,
				sref_new.cstr_, sref_new.leng_,
				Znk_NPOS );
		ZnkBfr_replace( text, begin, end-begin, (uint8_t*)ZnkStr_cstr(tmp), ZnkStr_leng(tmp) );

		pos = begin + ZnkStr_leng(tmp) + sref_end.leng_;
	}

	ZnkStr_delete( tmp );
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

		/***
		 * Hi all,
		 *
		 * First, we search comment out range, and replace evil patterns to innocences
		 * if it exists in the range.  We use Boyer-Moore Sunday algorism for performance.
		 * Second, adopt your myf filter to the text normally.
		 *
		 * Regard.
		 */
		/* sanitize HTML comment out */
		replacePtnInnerRange( text, "<!--", "-->", "iframe",   "zenkakudayoon" );
		replacePtnInnerRange( text, "<!--", "-->", "script",   "zenkakudayoon" );
		replacePtnInnerRange( text, "<!--", "-->", "noscript", "zenkakudayoon" );
		/* sanitize HTML script or noscript tag */
		replacePtnInnerRange( text, "<script", "</script",     "iframe", "zenkakudayoon" );
		replacePtnInnerRange( text, "<noscript", "</noscript", "iframe", "zenkakudayoon" );
	}
	return true;
}

