#include <Est_post.h>
#include <Est_base.h>
#include <Est_config.h>
#include <Est_ui.h>
#include <Est_hint_manager.h>
#include <Est_bmp_writer.h>

#include <Rano_log.h>
#include <Rano_htp_modifier.h>
#include <Rano_htp_boy.h>
#include <Rano_post.h>

#include <Znk_htp_util.h>
#include <Znk_htp_post.h>
#include <Znk_cookie.h>
#include <Znk_stdc.h>
#include <Znk_str_ex.h>
#include <Znk_dir.h>

#if defined(Znk_TARGET_WINDOWS)
#  include <windows.h>
#endif
#  include <stdlib.h>

#define SJIS_HYO "\x95\x5c" /* 表 */
#define SJIS_NOU "\x94\x5c" /* 能 */

#define IS_OK( val ) (bool)( (val) != NULL )

typedef struct {
	RanoCGIEVar* evar_; 
	ZnkHtpHdrs   htp_hdrs_;
	RanoModule   mod_; 
	ZnkStr       given_authentick_key_;
} EstPostInfo;

static const char*
getUpfileVarname( const char* target )
{
	if( ZnkS_eq( target, "futaba" ) ){
		return "upfile";
	}
	return "";
}

static void
initHtpHdr_forPost( ZnkHtpHdrs htp_hdrs, const char* hostname, const char* req_urp, const char* ua, ZnkVarpAry cookie, RanoCGIEVar* evar )
{
	ZnkVarp varp;
	ZnkSRef sref = { 0 };
	Znk_UNUSED(varp);

	RanoLog_printf( "initHtpHdr_forPost : hostname=[%s] req_urp=[%s]\n", hostname, req_urp );

	varp = ZnkHtpHdrs_regist( htp_hdrs->vars_,
			"Host",   Znk_strlen_literal("Host"),
			hostname, Znk_strlen(hostname), true );

	varp = ZnkHtpHdrs_regist( htp_hdrs->vars_,
			"User-Agent", Znk_strlen_literal("User-Agent"),
			ua, Znk_strlen(ua), true );

	ZnkSRef_set_literal( &sref, "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8" );
	varp = ZnkHtpHdrs_regist( htp_hdrs->vars_,
			"Accept", Znk_strlen_literal("Accept"),
			sref.cstr_, sref.leng_, true );

	ZnkSRef_set_literal( &sref, "ja,en-US;q=0.7,en;q=0.3" );
	varp = ZnkHtpHdrs_regist( htp_hdrs->vars_,
			"Accept-Language", Znk_strlen_literal("Accept-Language"),
			sref.cstr_, sref.leng_, true );

	ZnkSRef_set_literal( &sref, "gzip, deflate" );
	varp = ZnkHtpHdrs_regist( htp_hdrs->vars_,
			"Accept-Encoding", Znk_strlen_literal("Accept-Encoding"),
			sref.cstr_, sref.leng_, true );

	ZnkSRef_set_literal( &sref, "http://www.google.com" );
	varp = ZnkHtpHdrs_regist( htp_hdrs->vars_,
			"Referer", Znk_strlen_literal("Referer"),
			sref.cstr_, sref.leng_, true );
	if( varp ){
		ZnkStr val = ZnkHtpHdrs_val( varp, 0 );
		ZnkStr_set( val, "http://" );
		ZnkStr_add( val, hostname );
		ZnkStr_add( val, req_urp );
	}

	{
		ZnkStr     cok_stmt = ZnkStr_new( "" );
		ZnkCookie_extend_toCookieStatement( cookie, cok_stmt );
		varp = ZnkHtpHdrs_regist( htp_hdrs->vars_,
				"Cookie",   Znk_strlen_literal("Cookie"),
				ZnkStr_cstr(cok_stmt), ZnkStr_leng(cok_stmt), true );
		ZnkStr_delete( cok_stmt );
	}

	ZnkSRef_set_literal( &sref, "keep-alive" );
	varp = ZnkHtpHdrs_regist( htp_hdrs->vars_,
			"Connection", Znk_strlen_literal("Connection"),
			sref.cstr_, sref.leng_, true );

	varp = ZnkHtpHdrs_regist( htp_hdrs->vars_,
			"Content-Type", Znk_strlen_literal("Content-Type"),
			evar->content_type_, Znk_strlen(evar->content_type_), true );

	varp = ZnkHtpHdrs_regist( htp_hdrs->vars_,
			"Content-Length", Znk_strlen_literal("Content-Length"),
			evar->content_length_, Znk_strlen(evar->content_length_), true );

}


/***
 * Securityの問題をどう解決するかであるが、
 * とりあえずキャッシュ削除などと同様に指定されたディレクトリ配下以外でのファイル指定は無効とみなすことで対処する.
 */
static bool
replaceUploadFile_forMPFD( const ZnkVarpAry hpvs, const char* query_post_var_name, const char* new_filename )
{
	const size_t size = ZnkVarpAry_size( hpvs );
	uint8_t buf[ 4096 ] = { 0 };
	size_t idx;
	ZnkVarp varp;

	for( idx=0; idx<size; ++idx ){
		varp = ZnkVarpAry_at( hpvs, idx );
		if( ZnkS_eq( ZnkVar_name_cstr( varp ), query_post_var_name ) && varp->type_ == ZnkHtpPostVar_e_BinaryData ){
			/* 既に存在した場合は置き換える */
			ZnkFile fp = Znk_fopen( new_filename, "rb" );
			size_t readed_byte = 0;
			const char* q = Znk_strrchr( new_filename, '/' );
			q = q ? q + 1 : new_filename;
			//RanoLog_printf( "replaceUploadFile_forMPFD : q=[%s]\n", q );
			ZnkStr_assign( ZnkVar_misc( varp ), 0, q, Znk_NPOS );
			if( fp ){
				ZnkBfr bfr = ZnkVar_bfr( varp );
				ZnkBfr_clear( bfr );
				while( true ){
					readed_byte = Znk_fread( buf, sizeof(buf), fp );
					if( readed_byte == 0 ){
						break;
					}
					ZnkBfr_append_dfr( bfr, buf, readed_byte );
				}
				return true;
			}
		}
	}

	{
		/* 存在しない場合は新たに追加 */
		ZnkFile fp = Znk_fopen( new_filename, "rb" );
		size_t readed_byte = 0;
		if( fp ){
			ZnkBfr bfr;
			/* 新規追加 */
			const char* q = Znk_strrchr( new_filename, '/' );
			q = q ? q + 1 : new_filename;
			//RanoLog_printf( "replaceUploadFile_forMPFD : q=[%s]\n", q );
			varp = ZnkVarp_create( "", "", 0, ZnkPrim_e_Bfr, NULL );
			varp->type_ = ZnkHtpPostVar_e_BinaryData;
			ZnkStr_assign( varp->name_, 0, query_post_var_name, Znk_NPOS );
			ZnkStr_assign( ZnkVar_misc( varp ), 0, q, Znk_NPOS );
			ZnkVarpAry_push_bk( hpvs, varp );

			bfr = ZnkVar_bfr( varp );
			while( true ){
				readed_byte = Znk_fread( buf, sizeof(buf), fp );
				if( readed_byte == 0 ){
					break;
				}
				ZnkBfr_append_dfr( bfr, buf, readed_byte );
			}
			return true;
		}
	}
	return false;
}

static bool
doHyperUpload( ZnkVarpAry post_vars,
		const char* new_filename, const char* upfile_varname, const char* ua, RanoModule mod,
		ZnkStr result_filename )
{
	bool   is_updated = false;
	bool   is_result = false;
	bool   is_url   = false;

	is_url = ZnkS_isBegin( new_filename, "http://" ) ||
		ZnkS_isBegin( new_filename, "https://" ) ||
		ZnkS_isBegin( new_filename, "//" );

	/***
	 * http:// または https:// または // 以外は認めない.
	 * 即ちfile:// や ローカルファイルへの相対およびフルパスの指定は認めない.
	 * これらは「添付ファイル」より指定すべきであり、その方がよりセキュリティ的に堅牢である.
	 */
	if( is_url ){
		bool        is_https = false;
		int         status_code = -1;
		char        hostname[ 1024 ] = "";
		ZnkStr      req_urp = ZnkStr_new( "" );
		const char* src     = EstBase_getHostnameAndRequrp_fromEstVal( hostname, sizeof(hostname), req_urp, new_filename, &is_https );
		const char* unesc_req_urp = ZnkStr_cstr( req_urp );
		ZnkVarpAry  cookie  = ZnkVarpAry_create( true );
		const char* parent_proxy  = EstConfig_parent_proxy();

		Znk_UNUSED( src );
		is_result = EstBase_download( hostname, unesc_req_urp, "upload",
				ua, cookie,
				parent_proxy,
				result_filename, NULL, mod, &status_code, is_https );

		ZnkVarpAry_destroy( cookie );
		ZnkStr_delete( req_urp );

		if( is_result && status_code == 200 && replaceUploadFile_forMPFD( post_vars, upfile_varname, ZnkStr_cstr(result_filename) ) ){
			is_updated = true;
		}
	}

	return is_updated;
}


#if defined(Znk_TARGET_WINDOWS)
static bool
saveHBitmap( HBITMAP hBmp, const char* filename, ZnkStr ermsg )
{
	bool result = false;
	BITMAP bmap;
	int    nBmi = 0;
	size_t nColorTable = 0;
	int    nImage = 0;
	BITMAPINFO* pBmi = NULL;
	uint8_t* pImage = NULL;
	size_t rowbytes = 0;
	int    nBit = GetObject( hBmp, sizeof(bmap), &bmap );
	Znk_UNUSED( nBit );

	/* パレットエントリ数を判定 */
	switch( bmap.bmBitsPixel ) {
	case 1:  nColorTable =   2;    break;
	case 4:  nColorTable =  16;    break;
	case 8:  nColorTable = 256;    break;
	default: nColorTable =   0;    break;         
	}
	
	// 必要なサイズを計算し、メモリを獲得します
	nBmi     = sizeof(RGBQUAD) * nColorTable; /* ヘッダーのサイズ */
	rowbytes = ((bmap.bmWidth * bmap.bmBitsPixel) + 31) / 8;
	nImage   = rowbytes * bmap.bmHeight; /* 画素情報のサイズ */
	pBmi     = (BITMAPINFO *)Znk_malloc(nBmi + nImage);
	
	pImage = Znk_force_ptr_cast( uint8_t*, pBmi ) + nBmi;
	
	/* ヘッダー情報の作成 */
	memset( pBmi, 0, nBmi );
	pBmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pBmi->bmiHeader.biWidth  = bmap.bmWidth;
	pBmi->bmiHeader.biHeight = bmap.bmHeight;
	pBmi->bmiHeader.biPlanes = 1;
	pBmi->bmiHeader.biBitCount = bmap.bmBitsPixel;
	pBmi->bmiHeader.biCompression = BI_RGB;
	pBmi->bmiHeader.biSizeImage = 0;
	pBmi->bmiHeader.biXPelsPerMeter = 0;
	pBmi->bmiHeader.biYPelsPerMeter = 0;
	pBmi->bmiHeader.biClrUsed      = nColorTable;
	pBmi->bmiHeader.biClrImportant = nColorTable;
	
	/* 画素情報取得 */
	{
		HWND hWnd = GetDesktopWindow();
		HDC  hDC  = GetWindowDC(hWnd);
		GetDIBits( hDC, hBmp, 0, bmap.bmHeight, pImage, pBmi, DIB_RGB_COLORS );
		ReleaseDC( hWnd, hDC );
	}
	{
		ZnkFile fp = Znk_fopen( filename, "wb" );
		if( fp ){
			uint8_t argb_idx[ 4 ] = { 3, 2, 1, 0 };
			uint8_t* palette32 = NULL;
			size_t   palette32_num = 0;
			EstBmp_writeBFH( fp, bmap.bmHeight, bmap.bmBitsPixel, rowbytes, ermsg );
			EstBmp_writeBIH( fp, palette32, palette32_num,
					bmap.bmWidth, bmap.bmHeight, bmap. bmBitsPixel, rowbytes,
					argb_idx, ermsg );
			Znk_fwrite( pImage, nImage, fp );
			Znk_fclose( fp );
			result = true;
		}
	}
	Znk_free( pBmi );
	return result;
}
#endif

static bool
getClipboardData( const char* filename, ZnkStr ermsg )
{
	bool result = false;
#if defined(Znk_TARGET_WINDOWS)
	HWND hwnd = NULL;
	if( IsClipboardFormatAvailable( CF_BITMAP ) ){
		if( OpenClipboard( hwnd ) ){
			HBITMAP hBitmap = (HBITMAP)GetClipboardData( CF_BITMAP );
			result = saveHBitmap( hBitmap, filename, ermsg );
			CloseClipboard();
		} else {
			if( ermsg ){
				ZnkStr_addf( ermsg, "Easter HyperPost : Error.<br>" );
				ZnkStr_addf( ermsg, "<br>" );
				ZnkStr_addf( ermsg, "&nbsp;&nbsp;クリップボードの内容の取得に失敗しました.<br>" );
				ZnkStr_addf( ermsg, "&nbsp;&nbsp;Cannot open Clipboard.\n" );
			}
		}
	} else {
		if( ermsg ){
			ZnkStr_addf( ermsg, "Easter HyperPost : Error.<br>" );
			ZnkStr_addf( ermsg, "<br>" );
			ZnkStr_addf( ermsg, "&nbsp;&nbsp;クリップボードの内容の取得に失敗しました.<br>" );
			ZnkStr_addf( ermsg, "&nbsp;&nbsp;クリップボードに画像がありません.<br>" );
			ZnkStr_addf( ermsg, "&nbsp;&nbsp;(CF_BITMAP is not available)" );
		}
	}
#endif
	return result;
}

static bool
uploadClipboard( ZnkVarpAry post_vars, const char* upfile_varname, ZnkStr ermsg )
{
	bool   result = false;
	char filename_path[ 256 ] = "";

	Znk_snprintf( filename_path, sizeof(filename_path), "%s/clipboard_save.bmp" );

	/***
	 * Test : GetClipboardData (only windows)
	 */
	result = getClipboardData( filename_path, ermsg );
	if( !result ){
		goto FUNC_END;
	}

	result = false;

	/***
	 * さしあたってはImageMagic(あるいはそれと互換なconvertコマンド)を用いる.
	 * 尚、現時点ではまだマルチプロセス処理に対する排他制御は行っていない.
	 */
	if( ZnkDir_getType( "convert.exe" ) == ZnkDirType_e_File ){
		ZnkStr cmd = ZnkStr_newf( "convert.exe %s cachebox/upload/clipboard_save.png", filename_path );
		ZnkDir_mkdirPath( "cachebox/upload", Znk_NPOS, '/', NULL );
		ZnkDir_deleteFile_byForce( "cachebox/upload/clipboard_save.png" );
		system( ZnkStr_cstr(cmd) );
		if( ZnkDir_getType( "cachebox/upload/clipboard_save.png" ) == ZnkDirType_e_File ){
			ZnkDir_deleteFile_byForce( filename_path );
			if( replaceUploadFile_forMPFD( post_vars, upfile_varname, "cachebox/upload/clipboard_save.png" ) ){
				result = true;
			}
		}
		ZnkStr_delete( cmd );
	} else {
		ZnkStr_addf( ermsg, "Easter HyperPost : Error.<br>" );
		ZnkStr_addf( ermsg, "<br>" );
		ZnkStr_addf( ermsg, "&nbsp;&nbsp;ImageMagicのconvertコマンドがeaster配下に設置されていません.<br>" );
		ZnkStr_addf( ermsg, "&nbsp;&nbsp;クリップボードのファイル形式の変換のためにこのコマンドが必要となります.<br>" );
		ZnkStr_addf( ermsg, "&nbsp;&nbsp;<a class=MstyElemLink href=/moai2.0/easter_reference.html#EasterHyperPost_clipboard>こちら</a>を参照し、このコマンドを設置してください.<br>" );
		goto FUNC_END;
	}

FUNC_END:
	return result;
}

static void
cleanupHypeUpload( ZnkVarpAry post_vars,
		ZnkVarp hyper_upload_path, ZnkVarp clipboard_upload_mode,
		bool* is_updated )
{
	/***
	 * 以下は post_vars から狩り取る.
	 */
	if( hyper_upload_path ){
		ZnkVarpAry_erase( post_vars, hyper_upload_path );
		*is_updated = true;
	}
	if( clipboard_upload_mode ){
		ZnkVarpAry_erase( post_vars, clipboard_upload_mode );
		*is_updated = true;
	}
}

struct HyperPostInfo {
	EstPostInfo post_info_;
	ZnkStr upfile_filename_;
	ZnkStr upfile_varname_;
	ZnkStr ermsg_;
	ZnkStr est_hyper_upload_path_;
	bool*  est_clipboard_upload_mode_;
};

static bool
func_proc_post_vars( ZnkVarpAry post_vars, void* arg, const char* content_type, bool* is_updated )
{
	bool result = false;
	struct HyperPostInfo* hyper_post_info = Znk_force_ptr_cast( struct HyperPostInfo*, arg );
	EstPostInfo* post_info = &hyper_post_info->post_info_;
	ZnkStr       upfile_filename = hyper_post_info->upfile_filename_;
	ZnkHtpHdrs   htp_hdrs             = post_info->htp_hdrs_;
	RanoModule   mod                  = post_info->mod_;
	ZnkStr       given_authentick_key = post_info->given_authentick_key_;
	bool         is_authenticated     = false;
	const char*  moai_authentic_key   = EstConfig_authenticKey();
	const char*  easter_default_ua    = EstConfig_getEasterDefaultUA();
	
	if( ZnkStr_eq( given_authentick_key, moai_authentic_key ) ){
		is_authenticated = true;
	} else if( ZnkStr_empty( given_authentick_key ) ){
		ZnkVarp authkey = ZnkVarpAry_find_byName_literal( post_vars, "Moai_AuthenticKey", false );
		if( authkey ){
			if( ZnkS_eq( ZnkVar_cstr(authkey), moai_authentic_key ) ){
				is_authenticated = true;
			}
		}
	}

	/***
	 * 念のためMoai_AuthenticKeyのみ狩り取る.
	 */
	{
		const size_t idx = ZnkVarpAry_findIdx_byName( post_vars, "Moai_AuthenticKey", Znk_NPOS, false );
		if( idx != Znk_NPOS ){
			ZnkVarpAry_erase_byIdx( post_vars, idx );
			*is_updated = true;
		}
	}

	/***
	 * Easter Hyper-Upload
	 */
	if( is_authenticated ){
		ZnkVarp hyper_upload_path     = ZnkVarpAry_find_byName_literal( post_vars, "est_hyper_upload_path", false );
		ZnkVarp clipboard_upload_mode = ZnkVarpAry_find_byName_literal( post_vars, "est_clipboard_upload_mode", false );
		/***
		 * この機能は Authentic なUIからの投稿に限り有効.
		 */
		{
			const char* upfile_varname = ZnkStr_cstr(hyper_post_info->upfile_varname_);

			if( IS_OK( clipboard_upload_mode ) && ZnkS_eq( ZnkVar_cstr( clipboard_upload_mode ), "on" ) ){
				/* Clipboard uploading */
				result = uploadClipboard( post_vars, upfile_varname, hyper_post_info->ermsg_ );
				if( result ){
					ZnkStr_set( upfile_filename, "cachebox/upload/clipboard_save.png" );
					*is_updated = true;
					if( hyper_post_info->est_clipboard_upload_mode_ ){
						*hyper_post_info->est_clipboard_upload_mode_ = true;
					}
				} else {
					return false;
				}

			} else if( IS_OK( hyper_upload_path ) ){
				/* URL uploading */
				const char* new_filename = ZnkVar_cstr( hyper_upload_path );
				if( !ZnkS_empty( new_filename ) ){
					ZnkVarp     ua_var         = ZnkHtpHdrs_find_literal( htp_hdrs->vars_, "User-Agent" );

					/***
					 * header_varsにUser-Agentの項目がないか、値がUNTOUCHである場合.
					 * easter_default_uaを使う.
					 */
					const char* ua = ( ua_var == NULL ) ? easter_default_ua :
						ZnkS_eq( ZnkHtpHdrs_val_cstr(ua_var,0), "UNTOUCH" ) ? easter_default_ua : ZnkHtpHdrs_val_cstr(ua_var,0);

					result = doHyperUpload( post_vars, new_filename, upfile_varname, ua, mod, upfile_filename );
					if( !result ){
						/***
						 * Downloadに失敗したと思われる.
						 * この場合は何もせずエラーメッセージを出すべき.
						 */
						ZnkStr ermsg = hyper_post_info->ermsg_;
						ZnkStr_addf( ermsg, "Easter HyperPost : Error.<br>" );
						ZnkStr_addf( ermsg, "<br>" );
						ZnkStr_addf( ermsg, "&nbsp;&nbsp;添付URLにて指定されたURLの画像のダウンロードに失敗しました.<br>" );
						ZnkStr_addf( ermsg, "&nbsp;&nbsp;URLの指定に誤りがある可" SJIS_NOU "性もあるのでご確認ください.<br>" );
						ZnkStr_addf( ermsg, "<br>" );
						ZnkStr_addf( ermsg, "&nbsp;&nbsp;指定されたURL=[%s]", new_filename );
						return false;
					}
					if( result ){
						*is_updated = true;
						if( hyper_post_info->est_hyper_upload_path_ ){
							ZnkStr_set( hyper_post_info->est_hyper_upload_path_, new_filename );
						}
					}
				}
			}
		}
		cleanupHypeUpload( post_vars, hyper_upload_path, clipboard_upload_mode, is_updated );
	}
	return true;
}

static bool
parsePostAndCookieVars( RanoCGIEVar* evar, ZnkVarpAry post_vars, ZnkStr msg, ZnkHtpHdrs htp_hdrs,
		ZnkStr pst_str, /* XSS-safe */ const char* hostname, const char* req_urp, const char* target, RanoModule mod,
		ZnkStr upfile_preview, ZnkStr hyper_upload_path, bool* clipboard_upload_mode )
{
	static const char* varname_of_urlenc_body = NULL;
	bool   is_unescape_val = true;

	bool result = false;
	const char* tmpdir = EstConfig_getTmpDirCommon( true );

	ZnkBfr in_bfr  = ZnkBfr_create_null();
	ZnkStr given_authentick_key = ZnkStr_new( "" );
	size_t      content_length = 0;
	const char* content_type   = evar->content_type_;
	struct HyperPostInfo hyper_post_info = { { 0 } };
	RanoCGIUtilFnca_procPostVars fnca_proc_post_vars = {
		func_proc_post_vars, NULL,
	};
	const char* easter_default_ua = EstConfig_getEasterDefaultUA();

	/***
	 * 実際に使用するboundaryはContent-Type内に記載されている文字列の前に
	 * さらに -- を付加したものを使用しなければならない.
	 */
	ZnkStr src_boundary = ZnkStr_new( "--" );
	ZnkStr dst_boundary = ZnkStr_new( "--" );

	ZnkS_getSzU( &content_length, evar->content_length_ );
	RanoCGIUtil_getStdInStr( in_bfr, content_length );

	{
		EstPostInfo* post_info = &hyper_post_info.post_info_;
		post_info->evar_     = evar;
		post_info->htp_hdrs_ = htp_hdrs,
		post_info->mod_      = mod;
		post_info->given_authentick_key_ = given_authentick_key;
		hyper_post_info.upfile_filename_ = ZnkStr_new( "" );
		hyper_post_info.upfile_varname_  = ZnkStr_new( "" );
		hyper_post_info.ermsg_           = msg;
		hyper_post_info.est_hyper_upload_path_     = hyper_upload_path;
		hyper_post_info.est_clipboard_upload_mode_ = clipboard_upload_mode;
		fnca_proc_post_vars.arg_ = &hyper_post_info;

		ZnkStr_set( hyper_post_info.upfile_varname_, getUpfileVarname( target ) );
	}

	/***
	 * Moai_AuthenticKey のみ確認する.
	 * その後は URL QueryStringに含まれるMoai/Easter系で使用するPostVarをすべてクリアしておき、
	 * 必要なPostVarだけ再度構築する.
	 */
	{
		ZnkVarp authkey = ZnkVarpAry_find_byName_literal( post_vars, "Moai_AuthenticKey", false );
		if( authkey ){
			ZnkStr_set( given_authentick_key, ZnkVar_cstr(authkey) );
		}
		ZnkVarpAry_clear( post_vars );
	}

	/***
	 * HtpHdrをでっち上げる.
	 */
	{
		ZnkVarpAry cookie   = ZnkVarpAry_create( true );
		char cookie_filename[ 256 ] = "";

		Znk_snprintf( cookie_filename, sizeof(cookie_filename), "%s%s/cookie.txt", tmpdir, target );
		ZnkCookie_load( cookie, cookie_filename );
		initHtpHdr_forPost( htp_hdrs, hostname, req_urp, easter_default_ua, cookie, evar );

		ZnkVarpAry_destroy( cookie );
	}

	ZnkHtpHdrs_scanContentTypeVal( content_type, src_boundary );

	/***
	 * boundaryを先に決めるため、RanoHtpModifier_modifySendHdrsは先立って行っておく必要がある.
	 */
	if( mod ){
		ZnkMyf ftr_send = RanoModule_ftrSend( mod );
		const ZnkVarpAry ftr_vars = ZnkMyf_find_vars( ftr_send, "header_vars" );
		if( ftr_vars ){
			const ZnkVarp ua_var = ZnkVarpAry_find_byName( ftr_vars, "User-Agent", Znk_strlen_literal("User-Agent"), false );

			/***
			 * header_varsにUser-Agentの項目がないか、値がUNTOUCHである場合.
			 * easter_default_uaを使う.
			 */
			const char* ua = ( ua_var == NULL ) ? easter_default_ua :
				ZnkS_eq( ZnkVar_cstr(ua_var), "UNTOUCH" ) ? easter_default_ua : ZnkVar_cstr(ua_var);

			if( RanoHtpModifier_modifySendHdrs( htp_hdrs->vars_, ua, pst_str ) ){
				ZnkStr_add( pst_str, "  RanoHtpModifier_modifySendHdrs is true\n" );
			} else {
				ZnkStr_add( pst_str, "  RanoHtpModifier_modifySendHdrs is false\n" );
			}
		}
	}
	ZnkHtpHdrs_scanContentType( htp_hdrs->vars_, dst_boundary );

	result = RanoCGIUtil_getPostVars_fromInBfr( post_vars, mod, htp_hdrs,
			in_bfr, content_length,
			content_type, ZnkStr_cstr(src_boundary), ZnkStr_cstr(dst_boundary),
			pst_str, varname_of_urlenc_body, is_unescape_val, &fnca_proc_post_vars );

	if( !result ){
		if( upfile_preview ){
			ZnkStr_setf( upfile_preview, "<div class=MstyComment>%s</div>", ZnkStr_cstr(hyper_post_info.ermsg_) );
		}
	} else if( upfile_preview ){
		const char* xhr_dmz = EstConfig_XhrDMZ();
		ZnkStr upfile_filename = hyper_post_info.upfile_filename_;
		if( ZnkStr_empty(upfile_filename) ){
			/* ディスクからファイルを選択 */
			const char* upfile_varname = ZnkStr_cstr(hyper_post_info.upfile_varname_);
			ZnkVarp varp = ZnkVarpAry_findObj_byName( post_vars, upfile_varname, Znk_NPOS, false );
			if( varp ){
				if( ZnkVar_prim_type( varp ) == ZnkPrim_e_Bfr ){
					const char* filename = ZnkVar_misc_cstr( varp, "filename" );
					if( !ZnkS_empty(filename) ){
						ZnkFile fp = NULL;
						ZnkDir_mkdirPath( "cachebox/upload", Znk_NPOS, '/', NULL );
						ZnkStr_setf( upfile_filename, "cachebox/upload/%s", filename );
						fp = Znk_fopen( ZnkStr_cstr(upfile_filename), "wb" );
						if( fp ){
							Znk_fwrite( ZnkVar_data(varp), ZnkVar_data_size(varp), fp );
							Znk_fclose( fp );
							if( hyper_upload_path ){
								ZnkStr_setf( hyper_upload_path, "http://%s/cgis/easter/%s",
										xhr_dmz, ZnkStr_cstr(upfile_filename) );
							}
						}
					}
				}
			}
		} else {
			/* 添付URL or 添付クリップボード */
			if( ZnkStr_isBegin( upfile_filename, "./" ) ){
				ZnkStr_cut_front( upfile_filename, 2 );
			}
		}
		if( ZnkStr_leng(upfile_filename) ){
			ZnkStr_setf( upfile_preview,
					"<div class=MstyComment>"
					"<a href='http://%s/cgis/easter/%s' target=_blank>"
					"<img src='http://%s/cgis/easter/%s' width=350>"
					"</a>"
					"</div>",
					xhr_dmz, ZnkStr_cstr(upfile_filename),
					xhr_dmz, ZnkStr_cstr(upfile_filename) );
		} else {
			ZnkStr_setf( upfile_preview,
					"<div class=MstyComment>"
					"以下のうちのいずれかを指定した状態で「送信画像の確認」ボタンを押すと、<br>"
					"これから送信される画像の内容をプレビューで確認できます.<br>"
					"(このボタンでは実際の送信は行われません)<br>"
					"<div style=\"padding:0.4em 1em\">"
					"・ディスクからファイルを選択<br>"
					"・添付URL<br>"
					"・添付クリップボード<br>"
					"</div>"
					"これら複数を同時に指定した場合は、下にある項目の指定が優先されます.<br>"
					"(いずれも指定されていない場合は、このメッセージが" SJIS_HYO "示されます)<br>"
					"</div>" );
		}
	}

	/***
	 * Header and Cookie filtering
	 * PostVar filteringにおけるon_post関数の呼び出しで、headerやcookie_varsの値を
	 * 参照/加工するような処理にも対応するため、この filteringをここで呼び出す.
	 */
	if( mod ){
		bool is_all_replace = true;
		ZnkVarpAry extra_vars     = NULL;
		RanoModule_filtHtpHeader(  mod, htp_hdrs->vars_ );
		RanoModule_filtCookieVars( mod, htp_hdrs->vars_, is_all_replace, extra_vars );
	}

	ZnkStr_delete( given_authentick_key );

	ZnkStr_delete( src_boundary );
	ZnkStr_delete( dst_boundary );
	ZnkBfr_destroy( in_bfr );

	ZnkStr_delete( hyper_post_info.upfile_filename_ );
	ZnkStr_delete( hyper_post_info.upfile_varname_ );

	return result;
}

void
EstPost_procPost( RanoCGIEVar* evar, ZnkVarpAry post_vars, const char* est_val, ZnkStr msg,
		EstPostFuncT_filterMain filterMain )
{
	const char* tmpdir = EstConfig_getTmpDirPID( true );
	struct ZnkHtpHdrs_tag htp_hdrs_instance = { 0 };
	ZnkHtpHdrs htp_hdrs = &htp_hdrs_instance;
	ZnkStr pst_str  = ZnkStr_new( "" );
	ZnkStr hostname = ZnkStr_new( "" );
	ZnkStr req_urp  = ZnkStr_new( "" );
	ZnkStr src      = ZnkStr_new( "" );
	const char* target = NULL;
	RanoModule  mod    = NULL;
	bool is_https = false;
	bool parse_post_and_cookie_result = false;

	ZnkHtpHdrs_compose( htp_hdrs );

	RanoLog_printf( "Easter : EstPost_procPost : est_val=[%s]\n", est_val );
	{
		char hostname_buf[ 1024 ] = "";
		ZnkStr_set( src, EstBase_getHostnameAndRequrp_fromEstVal( hostname_buf, sizeof(hostname_buf), req_urp, est_val, &is_https ) );
		ZnkStr_set( hostname, hostname_buf );
	}

	target = EstConfig_getTargetAndModule( &mod, ZnkStr_cstr(hostname) );
	if( target == NULL ){
		goto FUNC_END;
	}

	parse_post_and_cookie_result = parsePostAndCookieVars( evar, post_vars, msg, htp_hdrs,
			pst_str, ZnkStr_cstr(hostname), ZnkStr_cstr(req_urp), target, mod,
			NULL, NULL, NULL );

	/***
	 * confirm用
	 * メッセージ内容を正確なものとするため、filtering処理などが全て終った後に
	 * 全文を構築しなければならない.
	 */
#if 1
	{
		RanoLog_printf( "%s", ZnkStr_cstr(pst_str) );
	}
#endif

	if( parse_post_and_cookie_result ){
		RanoTextType txt_type = RanoText_Binary;
		char content_type[ 1024 ] = "application/octet-stream";
		const char* parent_proxy  = EstConfig_parent_proxy();
		bool result = false;
		ZnkVarpAry  cookie        = ZnkVarpAry_create( true );
		ZnkStr result_filename = ZnkStr_newf( "%sresult.dat", tmpdir );
		const char* cachebox = "./cachebox/";
		ZnkStr console_msg = ZnkStr_new( "" );

		{
			result = RanoHtpBoy_cipher_post( ZnkStr_cstr(hostname), ZnkStr_cstr(req_urp), target,
					htp_hdrs, post_vars,
					parent_proxy,
					tmpdir, cachebox, result_filename, is_https, msg );
			if( !result && msg ){
				ZnkStr_addf( msg,
						"  RanoHtpBoy_cipher_post : result=[%d] hostname=[%s] req_urp=[%s]<br>"
						"                     : target=[%s] parent_proxy=[%s] tmpdir=[%s]<br>"
						"                     : result_filename=[%s].<br>",
						result, ZnkStr_cstr(hostname), ZnkStr_cstr(req_urp), target, parent_proxy, tmpdir, ZnkStr_cstr(result_filename) );
			}

		}

		EstConfig_readRecvHdrSetCookie( mod, target );
		RanoHtpBoy_readRecvHdrVar( "Content-Type", content_type, sizeof(content_type), target, tmpdir );
		ZnkVarpAry_destroy( cookie );
		//Znk_printf( "ResultFile=[%s] src=[%s]\n", result_file, ZnkStr_cstr(src) );

		txt_type = RanoCGIUtil_getTextType( content_type );
		switch( txt_type ){
		case RanoText_HTML:
		case RanoText_CSS:
			ZnkStr_add( console_msg, "est_post success.\n" );
			EstBase_addConsoleMsg_HttpCookie( console_msg, evar->http_cookie_ );
			filterMain( ZnkStr_cstr(result_filename), ZnkStr_cstr(src), target, txt_type, console_msg, false, is_https );
			break;
		case RanoText_JS:
			//filterJS( ZnkStr_cstr(result_filename), ZnkStr_cstr(src), target, txt_type );
			break;
		default:
			break;
		}
		RanoCGIUtil_displayBinary( ZnkStr_cstr(result_filename), content_type );

		ZnkStr_delete( result_filename );
		ZnkStr_delete( console_msg );
	}

FUNC_END:
	ZnkStr_delete( pst_str );
	ZnkStr_delete( hostname );
	ZnkStr_delete( req_urp );
	ZnkStr_delete( src );
	ZnkHtpHdrs_dispose( htp_hdrs );
}


void
EstPost_procPostEx( RanoCGIEVar* evar, ZnkVarpAry post_vars, const char* est_val, ZnkStr msg )
{
	ZnkVarp post_dst_varp = ZnkVarpAry_find_byName_literal( post_vars, "post_dst", false );
	if( post_dst_varp == NULL ){
		/* EstHyperPost_main 仕様に変換すべく、QueryStringにpost_dst変数を追加し、
		 * est_post_exの値であるest_valをその値とする. */
		post_dst_varp = ZnkVarp_create( "post_dst", "", 0, ZnkPrim_e_Str, NULL );
		ZnkVar_set_val_Str( post_dst_varp, est_val, Znk_NPOS );
		ZnkVarpAry_push_bk( post_vars, post_dst_varp );
	}
	EstHyperPost_main( evar, post_vars, msg );
}

/***
 * Ver2.1.3
 * こちらに移行予定.
 */
void
EstHyperPost_main( RanoCGIEVar* evar, ZnkVarpAry post_vars, ZnkStr msg )
{
	struct ZnkHtpHdrs_tag htp_hdrs_instance = { 0 };
	ZnkHtpHdrs htp_hdrs = &htp_hdrs_instance;
	ZnkStr pst_str  = ZnkStr_new( "" );
	ZnkStr hostname = ZnkStr_new( "" );
	ZnkStr req_urp  = ZnkStr_new( "" );
	ZnkStr src      = ZnkStr_new( "" );
	ZnkStr original_url = ZnkStr_new( "" );
	ZnkStr upfile_preview = ZnkStr_new( "" );
	const char* target = NULL;
	RanoModule  mod    = NULL;
	ZnkVarp post_dst_varp = NULL;
	bool is_authenticated = true;
	char command[ 256 ] = "";
	ZnkStr hyper_upload_path     = ZnkStr_new( "" );
	bool   clipboard_upload_mode = false;

	ZnkHtpHdrs_compose( htp_hdrs );

	{
		ZnkVarp authkey = ZnkVarpAry_find_byName_literal( post_vars, "Moai_AuthenticKey", false );
		if( authkey ){
			const char* moai_authentic_key = EstConfig_authenticKey();
			is_authenticated = ZnkS_eq( ZnkVar_cstr(authkey), moai_authentic_key );
		}
	}

	{
		ZnkVarp command_varp = ZnkVarpAry_find_byName_literal( post_vars, "command", false );
		if( command_varp ){
			ZnkS_copy( command, sizeof(command), ZnkVar_cstr( command_varp ), Znk_NPOS );
			if( ZnkS_eq( command, "confirm" ) ){
			}
		}
	}

	if( IS_OK( post_dst_varp = ZnkVarpAry_find_byName_literal( post_vars, "post_dst", false ) )){
		char hostname_buf[ 1024 ] = "";
		const char* post_dst = ZnkVar_cstr( post_dst_varp );
		ZnkStr_set( src, EstBase_getHostnameAndRequrp_fromEstVal( hostname_buf, sizeof(hostname_buf), req_urp, post_dst, NULL ) );
		ZnkStr_set( hostname, hostname_buf );
	}

	target = EstConfig_getTargetAndModule( &mod, ZnkStr_cstr(hostname) );
	if( target == NULL ){
		goto FUNC_END;
	}

	{
		ZnkVarp varp = ZnkVarpAry_find_byName_literal( post_vars, "est_original_url", false );
		if( varp ){
			ZnkStr_set( original_url, ZnkVar_cstr(varp) );
		}
	}

	parsePostAndCookieVars( evar, post_vars, msg, htp_hdrs,
			pst_str, ZnkStr_cstr(hostname), ZnkStr_cstr(req_urp), target, mod,
			upfile_preview, hyper_upload_path, &clipboard_upload_mode );

	/***
	 * HyperPost UI
	 */
	{
		ZnkStr str = ZnkStr_new( "" );
		size_t idx;
		const size_t size = ZnkVarpAry_size( post_vars );
		ZnkBird bird = ZnkBird_create( "#[", "]#" );
		ZnkStr  val  = ZnkStr_new( "" );
		ZnkStr  input_hiddens = ZnkStr_new( "" );
		ZnkVarp varp = NULL;
		bool has_binary_attachment = false;
		bool has_title = false;
		ZnkStrAry tkns = ZnkStrAry_create( true );

		ZnkStr_add( str, "HTTP Header Confirmation.\n" );
		RanoCGIUtil_addHdrVarsStr( str, htp_hdrs->vars_ ); /* XSS-safe */
		ZnkStr_add( str, "\n" );

		ZnkBird_regist( bird, "xhr_dmz",  EstConfig_XhrDMZ() );
		ZnkBird_regist( bird, "post_dst", ZnkStr_cstr(src) );
		if( ZnkS_isBegin( evar->content_type_, "multipart/form-data;" ) ){
			ZnkBird_regist( bird, "post_enctype", "multipart/form-data" );
		} else {
			ZnkBird_regist( bird, "post_enctype", "application/x-www-form-urlencoded" );
		}

		{
			ZnkSRef old_ptn = { 0 };
			ZnkSRef new_ptn = { 0 };
			ZnkSRef_set_literal( &old_ptn, "\n" );
			ZnkSRef_set_literal( &new_ptn, "<br>\n" );
			ZnkStrEx_replace_BF( str,     0, old_ptn.cstr_, old_ptn.leng_, new_ptn.cstr_, new_ptn.leng_, Znk_NPOS, Znk_NPOS ); 
			ZnkStrEx_replace_BF( pst_str, 0, old_ptn.cstr_, old_ptn.leng_, new_ptn.cstr_, new_ptn.leng_, Znk_NPOS, Znk_NPOS ); 
		}
		ZnkBird_regist( bird, "hdr_vars_str", ZnkStr_cstr(str) );
		ZnkBird_regist( bird, "pst_vars_str", ZnkStr_cstr(pst_str) );
		ZnkBird_regist( bird, "Moai_AuthenticKey", EstConfig_authenticKey() );

		if( ZnkS_eq( target, "futaba" ) ){
			if( ZnkVarpAry_find_byName_literal( post_vars, "resto", false ) ){
				ZnkBird_regist( bird, "send_button_name", "返信する" );
			} else {
				ZnkBird_regist( bird, "send_button_name", "スレッドを立てる" );
			}
		}

		for( idx=0; idx<size; ++idx ){
			const char* key  = NULL;
			varp = ZnkVarpAry_at( post_vars, idx );
			key  = ZnkVar_name_cstr( varp );
			ZnkStrAry_clear( tkns );
			if( EstConfig_queryInputHiddens( target, key, tkns ) ){
				/* this is hiddens post var */
				if( ZnkVar_prim_type( varp ) == ZnkPrim_e_Str ){
					ZnkStr_set( val, ZnkVar_cstr( varp ) );
					ZnkHtpURL_negateHtmlTagEffection( val ); /* for XSS */
					/***
					 * javascript 側から document.getElementById でアクセスしているケースがある.
					 * 例えばfutabaのtegaki機能でbaseformへ画像データをセットするなど.
					 * よってここではidの指定も行うモードをつける.
					 */
					if( ZnkStrAry_size(tkns) >= 2 ){
						const char* id = ZnkStrAry_at_cstr( tkns, 1 );
						ZnkStr_addf( input_hiddens, "<input type=hidden name=%s id=%s value=\"%s\">\n",
								key, id, ZnkStr_cstr(val) );
					} else {
						ZnkStr_addf( input_hiddens, "<input type=hidden name=%s value=\"%s\">\n",
								key, ZnkStr_cstr(val) );
					}
				}
			} else {
				/* this is not hiddens post var */
				if( ZnkS_eq( key, "sub" ) ){
					has_title = true;
				}
				if( ZnkVar_prim_type( varp ) == ZnkPrim_e_Str ){
					ZnkStr_set( val, ZnkVar_cstr( varp ) );
				} else if( ZnkVar_prim_type( varp ) == ZnkPrim_e_Bfr ){
					ZnkStr_set( val, ZnkVar_misc_cstr( varp, "filename" ) ); /* filename */
					has_binary_attachment = true;
				}
				ZnkHtpURL_negateHtmlTagEffection( val ); /* for XSS */
				ZnkBird_regist( bird, key, ZnkStr_cstr(val) );
			}
		}
		ZnkBird_regist( bird, "input_hiddens", ZnkStr_cstr(input_hiddens) );
		ZnkBird_regist( bird, "upfile_preview",  ZnkStr_cstr(upfile_preview) );
		ZnkBird_regist( bird, "est_hyper_upload_path", ZnkStr_cstr(hyper_upload_path) );
		ZnkBird_regist( bird, "est_clipboard_upload_mode", clipboard_upload_mode ? "checked" : "" );

		if( ZnkStr_empty(original_url) ){
			ZnkBird_regist( bird, "original_url", "Unknown" );
			ZnkBird_regist( bird, "summary",      "" );
		} else {
			char cache_path[ 256 ] = "";
			if( ZnkStr_isBegin( original_url, "http://" ) ){
				ZnkStr_cut_front( original_url, Znk_strlen_literal("http://") );
			}
			if( ZnkStr_isBegin( original_url, "https://" ) ){
				ZnkStr_cut_front( original_url, Znk_strlen_literal("https://") );
			}
			ZnkBird_regist( bird, "original_url", ZnkStr_cstr(original_url) );
			Znk_snprintf( cache_path, sizeof(cache_path), "cachebox/%s/%s", target, ZnkStr_cstr(original_url) );
			EstUI_getSummary( bird, cache_path );
		}

		{
			ZnkStr hint_table = EstHint_getHintTable( "hyper_post" );
			if( hint_table ){
				ZnkBird_regist( bird, "hint_table", ZnkStr_cstr(hint_table) );
			} else {
				ZnkBird_regist( bird, "hint_table", "" );
			}
		}

		ZnkHtpURL_negateHtmlTagEffection( msg ); /* for XSS */
		{
			ZnkSRef old_ptn = { 0 };
			ZnkSRef new_ptn = { 0 };
			ZnkSRef_set_literal( &old_ptn, "\n" );
			ZnkSRef_set_literal( &new_ptn, "<br>\n" );
			ZnkStrEx_replace_BF( msg, 0, old_ptn.cstr_, old_ptn.leng_, new_ptn.cstr_, new_ptn.leng_, Znk_NPOS, Znk_NPOS ); 
		}
		ZnkBird_regist( bird, "msg",           ZnkStr_cstr(msg) );

		if( has_binary_attachment ){
			if( has_title ){
				RanoCGIUtil_printTemplateHTML( evar, bird, "templates/post_futaba.html" );
			} else {
				RanoCGIUtil_printTemplateHTML( evar, bird, "templates/post_futaba_img_thre.html" );
			}
		} else {
			/* futaba img タイプとみなす */
			RanoCGIUtil_printTemplateHTML( evar, bird, "templates/post_futaba_img.html" );
		}
		ZnkBird_destroy( bird );

		ZnkStr_delete( input_hiddens );
		ZnkStr_delete( val );
		ZnkStr_delete( str );
		ZnkStrAry_destroy( tkns );
	}

FUNC_END:
	ZnkStr_delete( pst_str );
	ZnkStr_delete( hostname );
	ZnkStr_delete( req_urp );
	ZnkStr_delete( src );
	ZnkStr_delete( original_url );
	ZnkStr_delete( upfile_preview );
	ZnkStr_delete( hyper_upload_path );
	ZnkHtpHdrs_dispose( htp_hdrs );
}
