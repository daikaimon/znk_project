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
 * myfのUSERS_passwordは、ユーザからの明示的な削除キー指定をシミュレーションするための
 * ものである. pwdはpwdcの値を元に自動設定される場合があり、ユーザからの明示的な指定か
 * 否かの区別がつかないため、このような変数を別途導入してある.
 */
static void
update_pwd( ZnkVarp pwd, ZnkVarp pwdc, ZnkVarp USERS_password )
{
	if( pwd ){
		if( USERS_password && !ZnkS_empty(ZnkVar_cstr(USERS_password)) ){
			/***
			 * この場合、以後pwdcもUSERS_passwordの値として発行されるはずである.
			 */
			ZnkVar_set_val_Str( pwd, ZnkVar_cstr(USERS_password),
					Znk_MIN(ZnkVar_str_leng(USERS_password),8) );
		} else if( pwdc ){
			/***
			 * pwdcがcookie_vars上において設定されているなら、この値を採用.
			 */
			ZnkVar_set_val_Str( pwd, ZnkVar_cstr(pwdc), ZnkVar_str_leng(pwdc) );
		} else {
			/***
			 * pwdcがcookie_vars上において設定されていないなら、空値とする.
			 * この場合、以後pwdcがふたばから適当な値として自動発行されるはずである.
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
	 * dst_ptuaが以下のケースの場合、値を変更してはならない.
	 *
	 *   空値
	 *   整数
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
	 * ftr_sendにUSERS_futabaptが定義されていない場合は
	 * localStorageが無効である環境と同挙動となる.
	 */
	if( USERS_futabapt ){
		/***
		 * ptfk関数のエミュレート.
		 * オリジナルでは右辺が空のとき、左辺への代入を行わないようにしてあるが、
		 * ここでは無条件で更新する. 左辺の古い値を残してはならないためである.
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
		/* まず簡易なケースを弾く */
		if( !isUntouchable_ptua( dst_ptua ) ){
#if 0
			/***
			 * 現在ふたばではptuaの実値をtransientな仕様とすることで偽装をかわす戦略をとっている.
			 * そのため、単純な代入が通用しない.
			 */
			ZnkVar_set_val_Str( dst_ptua, ZnkVar_cstr(USERS_ptua), ZnkVar_str_leng(USERS_ptua) );
#else
			/* とりあえずbrute force algorismで十分であろう */
			ZnkStr dst_ptua_str = ZnkVar_str( dst_ptua );
			ZnkStrEx_replace_BF( dst_ptua_str, 0,
					real_ua_cstr, strlen(real_ua_cstr),
					ZnkVar_cstr(USERS_ptua), ZnkVar_str_leng(USERS_ptua),
					Znk_NPOS );
#endif
		}
	}

	/***
	 * これによりブラウザ上にあるpwdの値は遮断され、
	 * ftr_send上にある値が替わりに採用される.
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

static void
sanitizeHTMLCommentOut( ZnkStr text )
{
	/***
	 * Hi all,
	 *
	 * First, we search comment out range, and replace iframe to other pattern
	 * if it exists in the range.  We use Boyer-Moore Sunday algorism for performance.
	 * Second, adopt your myf filter to the text normally.
	 *
	 * Regard.
	 */
	ZnkSRef sref_begin;
	ZnkSRef sref_end;
	ZnkSRef sref_target;
	ZnkSRef sref_other;
	size_t occ_tbl_begin[ 256 ];
	size_t occ_tbl_end[ 256 ];
	size_t occ_tbl_target[ 256 ];
	size_t pos   = 0;
	size_t begin = 0;
	size_t end   = 0;
	size_t text_leng = ZnkStr_leng( text );
	ZnkStr tmp = ZnkStr_new( "" );

	ZnkSRef_set_literal( &sref_begin,  "<!--" );
	ZnkSRef_set_literal( &sref_end,    "-->" );
	ZnkSRef_set_literal( &sref_target, "iframe" );
	ZnkSRef_set_literal( &sref_other,  "zenkakudayoon" );

	ZnkMem_getLOccTable_forBMS( occ_tbl_begin,  (uint8_t*)sref_begin.cstr_,  sref_begin.leng_ );
	ZnkMem_getLOccTable_forBMS( occ_tbl_end,    (uint8_t*)sref_end.cstr_,    sref_end.leng_ );
	ZnkMem_getLOccTable_forBMS( occ_tbl_target, (uint8_t*)sref_target.cstr_, sref_target.leng_ );

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
				sref_target.cstr_, sref_target.leng_, occ_tbl_target,
				sref_other.cstr_,  sref_other.leng_,
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
		const char* old_ptn = "＠ふたば</span>";
		char new_ptn[ 4096 ];
		Znk_snprintf( new_ptn, sizeof(new_ptn), "＠ふたば via Moai[%s]</span>", req_urp );
		replaceStr( text, 0,
				old_ptn, Znk_NPOS,
				new_ptn, Znk_NPOS,
				Znk_NPOS );
		sanitizeHTMLCommentOut( text );
	}
	return true;
}

