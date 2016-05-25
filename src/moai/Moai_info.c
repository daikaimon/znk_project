#include "Moai_info.h"
#include "Moai_log.h"
#include "Moai_parent_proxy.h"
#include "Moai_post_vars.h"
#include "Moai_myf.h"
#include <Znk_stdc.h>
#include <Znk_s_base.h>
#include <Znk_missing_libc.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


static ZnkObjAry st_moai_info_ary;

static void
deleteMoaiInfo( void* elem )
{
	if( elem ){
		MoaiInfo* info = (MoaiInfo*)elem;
		ZnkHtpHdrs_dispose( &info->hdrs_ );
		ZnkVarpAry_destroy( info->vars_ );
		ZnkBfr_destroy( info->stream_ );
		ZnkStr_delete( info->req_urp_ );
		ZnkStr_delete( info->proxy_hostname_ );
		Znk_free( info );
	}
}

static ZnkObj
allocMoaiInfo( void* arg )
{
	MoaiInfo* info = (MoaiInfo*)Znk_alloc0( sizeof( MoaiInfo ) );
	ZnkHtpHdrs_compose( &info->hdrs_ );
	info->vars_   = ZnkVarpAry_create( true );
	info->stream_ = ZnkBfr_create_null();
	info->req_method_     = ZnkHtpReqMethod_e_Unknown;
	info->req_urp_        = ZnkStr_new( "" );
	info->proxy_hostname_ = ZnkStr_new( "" );
	return (ZnkObj)info;
}

void
MoaiInfo_initiate( void )
{
	if( st_moai_info_ary == NULL ){
		st_moai_info_ary = ZnkObjAry_create( deleteMoaiInfo );
	}
}

static void
copyPostVars( ZnkVarpAry dst_vars, const ZnkVarpAry src_vars )
{
	const size_t size = ZnkVarpAry_size( src_vars );
	size_t idx;
	ZnkVarp src_varp;
	ZnkVarp dst_varp;

	ZnkVarpAry_clear( dst_vars );
	for( idx=0; idx<size; ++idx ){
		src_varp = ZnkVarpAry_at( src_vars, idx );
		dst_varp = ZnkVarp_create( "", "", (int)src_varp->type_, ZnkPrim_type(&src_varp->prim_) ); 
		ZnkVarpAry_push_bk( dst_vars, dst_varp );
		ZnkStr_set( dst_varp->name_, ZnkStr_cstr(src_varp->name_) );
		ZnkStr_set( dst_varp->filename_, ZnkStr_cstr(src_varp->filename_) );
		if( src_varp->type_ == MoaiPostVar_e_BinaryData ){
			ZnkBfr_set_dfr( dst_varp->prim_.u_.bfr_,
					ZnkBfr_data(src_varp->prim_.u_.bfr_), ZnkBfr_size(src_varp->prim_.u_.bfr_) );
		} else {
			ZnkStr_set( dst_varp->prim_.u_.str_, ZnkStr_cstr(src_varp->prim_.u_.str_) );
		}
	}
}

static MoaiInfoID
issueNewID( void )
{
	static uint32_t st_next_seq = 0;
	time_t t;
	uint32_t t32;
	uint64_t ans = 0;

	/***
	 * �P�ɃV�[�P���V�����Ȑ����݂̂Ȃ炸�����Ɋւ���l�����ߍ���.
	 * ����ɂ��A�O��Moai�N�����ɔ��s���ꂽID���A����N������Moai��
	 * ������n���ꂽ�ꍇ�ł��AID�����̂��m���ɖh�����Ƃ��ł���.
	 *
	 * �܂��A����ŃV�[�P���V�����Ȑ������܂߂�̂́A�~���b�P�ʂ�
	 * ���s���W�������ꍇ�ł�ID�����̂��m���ɖh�����߂ł���.
	 */
	time(&t); /* ���ݎ����̎擾 */
	t32 = (uint32_t)t;
	ans = t32;
	ans <<= 32;
	ans |= st_next_seq;
	++st_next_seq;

	return (MoaiInfoID)ans;
}

MoaiInfoID
MoaiInfo_regist( const MoaiInfo* info )
{
	size_t ans_idx  = Znk_NPOS;
	bool   is_newly = false;
	MoaiInfo* new_info = NULL;

	MoaiInfo_initiate();
	new_info = (MoaiInfo*)ZnkObjAry_intern( st_moai_info_ary, &ans_idx,
			NULL, NULL,
			allocMoaiInfo, NULL,
			&is_newly );

	new_info->id_ = issueNewID();

	/* deep-copy all members except id */
	if( info ){
		/* deep-copy hdrs */
		ZnkStrAry_copy( new_info->hdrs_.hdr1st_, info->hdrs_.hdr1st_ );
		ZnkHtpHdrs_copyVars( new_info->hdrs_.vars_, info->hdrs_.vars_ );
	
		/* deep-copy POST vars */
		copyPostVars( new_info->vars_, info->vars_ );
	
		/* deep-copy stream */
		ZnkBfr_set_dfr( new_info->stream_,
				ZnkBfr_data(info->stream_), ZnkBfr_size(info->stream_) );
	
		/* deep-copy others */
		new_info->hdr_size_   = info->hdr_size_;
		ZnkStr_set( new_info->req_urp_,        ZnkStr_cstr(info->req_urp_) );
		ZnkStr_set( new_info->proxy_hostname_, ZnkStr_cstr(info->proxy_hostname_) );
		new_info->proxy_port_ = info->proxy_port_;
	}
	return new_info->id_;
}
void
MoaiInfo_clear( MoaiInfo* info )
{
	ZnkHtpHdrs_clear( &info->hdrs_ );
	ZnkVarpAry_clear( info->vars_ );
	ZnkBfr_clear( info->stream_ );
	ZnkStr_clear( info->req_urp_ );
	ZnkStr_clear( info->proxy_hostname_ );
	info->hdr_size_   = 0;
	info->req_method_ = ZnkHtpReqMethod_e_Unknown;
}

MoaiInfo*
MoaiInfo_find( MoaiInfoID query_id )
{
	const size_t size = ZnkObjAry_size( st_moai_info_ary );
	size_t idx;
	MoaiInfo* info;
	for( idx=0; idx<size; ++idx ){
		info = (MoaiInfo*)ZnkObjAry_at( st_moai_info_ary, idx );
		if( info->id_ == query_id ){
			/* found */
			return info;
		}
	}
	/* not found */
	return NULL;
}

static bool queryMoaiInfoID( ZnkObj obj, void* arg )
{
	MoaiInfoID query_id = Znk_force_ptr_cast( MoaiInfoID, arg );
	MoaiInfo*  info     = (MoaiInfo*)obj;
	return (bool)( info->id_ == query_id );
}
void
MoaiInfo_erase( MoaiInfoID query_id )
{
	ZnkObjAry_erase_withQuery( st_moai_info_ary,
			queryMoaiInfoID, Znk_force_ptr_cast( void*, query_id ) );
}

void
MoaiInfoID_getStr( MoaiInfoID id, MoaiInfoIDStr* id_str )
{
	Znk_snprintf( id_str->buf_, sizeof(id_str->buf_), "%08" Znk_PFMD_64 "x", (uint64_t)id );
}
MoaiInfoID
MoaiInfoID_scanIDStr( MoaiInfoIDStr* id_str )
{
	uint64_t ans = 0;
	if( !ZnkS_getU64X( &ans, id_str->buf_ ) ){
		assert(0);
	}
	return (MoaiInfoID)ans;
}


static bool
isRelayToMe( const char* my_hostname, size_t my_hostname_leng,
		uint16_t my_port, const char* host_begin )
{
	if( ZnkS_isBeginEx( host_begin, Znk_NPOS, my_hostname, my_hostname_leng ) ){
		if( host_begin[ my_hostname_leng ] == ':' ){
			uint16_t dst_port = 80; /* default HTTP port */
			ZnkS_getU16U( &dst_port, host_begin+my_hostname_leng+1 );
			if( dst_port == my_port ){
				/***
				 * Browser�͂��̃v���O������LocalProxy�Ƃ��Ďg���Ă���ɂ��ւ�炸
				 * ���̐ڑ���͍Ă�local�Ȃ��̃v���O����(WebServer�Ƃ��Đڑ�)�ł���.
				 * ���̏ꍇ�ALocalProxy�Ƃ��Ă̒��p�͖��ʂł���̂ŁA
				 * WebServer�Ƃ��Ă̐ڑ��ɋ������ׂ��ł��낤.
				 */
				return true;
			}
		}
	}
	return false;
}


static bool
decideLocalProxy_or_WebServer( ZnkStr str, ZnkStr req_urp, uint16_t my_port, const char* server_name )
{
	const char* host_begin = NULL;
	bool as_local_proxy = false;

	/***
	 * �����ł̓z�X�g������n�܂�ꍇ�́AUserAgent��LocalProxy�Ƃ���
	 * ���̃v���O�����ɐڑ����悤�Ƃ������̂Ƃ݂Ȃ��A/ ����n�܂�ꍇ�́A
	 * WebServer�Ƃ��Ă��̃v���O�����ɐڑ����悤�Ƃ������̂Ƃ݂Ȃ�.
	 *
	 * ���AHost���Ƃ��Ďw�肳��Ă���l�� localhost:P��127.0.0.1:P
	 * (P�͂��̃v���O�������g���Ă���port�ԍ�)�ł��邩�ۂ��𒲂ׂ�΁A
	 * ���̎�̔��肪�ł������Ɏv���邩������Ȃ����Alocalhost������
	 * Host��������ȊO�̕�����ł���ꍇ���L�蓾�邽�߁A����ł͕s���S�ł���.
	 * �Ⴆ��/etc/hosts�� 127.0.0.1 �̕ʖ��� localhost�ȊO�ɂ��w�肵�Ă�����A
	 * �����I�ɕʂ̃}�V�����炱�̃v���O�������N�����Ă���}�V����
	 * IP�A�h���X���w�肵�Đڑ������ꍇ(����IP�A�h���X�͈قȂ邽�ߓ��R�ł��邪)
	 * �Ȃǂł����ȊO�̕�����ɐ��蓾��.
	 */
	if( ZnkS_isBegin_literal( ZnkStr_cstr(str), "http://" ) ){
		/* http://����n�܂�ꍇ. */
		host_begin = ZnkStr_cstr( str ) + Znk_strlen_literal( "http://" );
		/* �{����LocalProxy�Ƃ��Ď� */
		as_local_proxy = true;
	} else if( ZnkStr_first( str ) == '/' ){
		/* / ����n�܂�ꍇ. */
		host_begin = NULL;
		/* �{����WebServer�Ƃ��Ď� */
		as_local_proxy = false;
		ZnkStr_set( req_urp, ZnkStr_cstr(str) );
	} else if( ZnkStr_first( str ) == '*' ){
		/**
		 * OPTIONS method�Ȃǂ̏ꍇ�A���̒l�ɂȂ��Ă���P�[�X���L�蓾��.
		 * �Ƃ肠���������_�ł�LocalProxy�Ƃ��ď���.
		 */
		host_begin = NULL;
		/* �{����LocalProxy�Ƃ��Ď� */
		as_local_proxy = true;
	} else {
		/* Host������n�܂�ꍇ. */
		host_begin = ZnkStr_cstr( str );
		/* �{����LocalProxy�Ƃ��Ď� */
		as_local_proxy = true;
	}

	if( host_begin ){
		ZnkSRef sref = { 0 };
		ZnkSRef_set_literal( &sref, "localhost" );
		if( isRelayToMe( sref.cstr_, sref.leng_, my_port, host_begin ) ){
			/* �{����WebServer�Ƃ��Ă̎󗝂ɋ��� */
			as_local_proxy = false;
		}
		ZnkSRef_set_literal( &sref, "127.0.0.1" );
		if( isRelayToMe( sref.cstr_, sref.leng_, my_port, host_begin ) ){
			/* �{����WebServer�Ƃ��Ă̎󗝂ɋ��� */
			as_local_proxy = false;
		}
		if( isRelayToMe( server_name, Znk_strlen(server_name), my_port, host_begin ) ){
			/* �{����WebServer�Ƃ��Ă̎󗝂ɋ��� */
			as_local_proxy = false;
		}

		/***
		 * �őO�ɂ���Host���̕����͏������Ȃ���΂Ȃ�Ȃ�.
		 * �z�X�g�����܂܂�Ă��Ă����܂��\�������T�C�g�����邪�A�����Ȃ�Ȃ��T�C�g������.
		 * �Ⴆ�� www.nicovideo.jp�Ȃǂ́A���̕�����host�������邱�Ƃ�z�肵�Ă��炸
		 * Redirect�p��Location���\�����ĕԂ��Ă���. ��̓I�ɂ͈ȉ��̂悤�ȗ���ɂȂ�.
		 *
		 * 1. GET http://www.nicovideo.jp/watch/sm0123456 HTTP/1.1 ��Request�����Ƃ���.
		 * 2. nicovideo��Web�T�[�o�́A��L��2������ / ���n�܂�filepath�ł���Ƃ݂Ȃ�.
		 *    �����܂��ŏ��� / �������ꏊ���������A���� / �̎�����n�܂镶�����
		 *    filepath2 �Ƃ����ꍇ�AHost�f�B���N�e�B�u�Ɏw�肳�ꂽ������ + / + filepath2
		 *    �Ƃ��������������Ă�������_�C���N�g�p��Location�f�B���N�e�B�u�ɐݒ肵�����̂�
		 *    Response�Ƃ��ĕԂ�.
		 * 3. ���ʓI�Ƀ��[�U����Browser�� GET www.nicovideo.jphttp/www.nicovideo.jp/watch/sm0123456 HTTP/1.1
		 *    �Ȃ�Request������ɔ��s���Ă��܂�.
		 *    ����͓��R�Awww.nicovideo.jphttp �Ƃ����z�X�g�ւ̐ڑ��Ƃ݂Ȃ���NotFound�ɂȂ�.
		 *
		 * �܂��t�ɐڑ���Ƃ��Ă���ɕʂ�proxy����ꍇ�́A
		 * ���R�Ȃ��珜���͍s���Ă͂Ȃ�Ȃ�.
		 */
		{
			/***
			 * TODO:
			 * ���̏����͂��̊֐��̊O�֒ǂ��o�������]�܂���.
			 */
			const char* end = strchr( host_begin, '/' );
			if( end == NULL ){
				end = host_begin + Znk_strlen( host_begin );
			}
			/***
			 * ZnkStr_erase����O�ɍς܂��Ă���.
			 */
			ZnkStr_assign( req_urp, 0, end, Znk_NPOS );
			{
				ZnkMyf config = MoaiMyf_theConfig();
				size_t hostname_leng = end - host_begin;
				const bool is_applied_host = MoaiParentProxy_isAppliedHost( config, host_begin, hostname_leng );
				if( !is_applied_host ){
					size_t cut_size = end - ZnkStr_cstr( str );
					ZnkStr_erase( str, 0, cut_size );
				}
			}
		}
	}
	return as_local_proxy;
}

void
MoaiInfo_parseHdr( MoaiInfo* info, MoaiBodyInfo* body_info,
		bool* as_local_proxy, uint16_t my_port, bool is_request,
		const char* response_hostname,
		MoaiModuleAry mod_ary, const char* server_name )
{
	size_t key_begin; size_t key_end;
	size_t val_begin; size_t val_end;
	const char* key = "";
	const char* val = "";
	ZnkVarp varp = NULL;
	const char* line_begin;
	const char* line_end;
	const char* p;
	const char* q;
	size_t arg_idx = 0;
	ZnkHtpHdrs hdrs   = &info->hdrs_;
	ZnkBfr     stream = info->stream_;

	/* first line */
	line_begin = (char*)ZnkBfr_data( stream );

	/***
	 * MoaiInfo hdrs��ݒ�.
	 */
	p = line_begin;
	while( *p != '\r' ){
		while( *p == ' ' || *p == '\t' ) ++p;
		q = p;
		if( arg_idx < 2 ){
			while( *q != ' ' && *q != '\t' ) ++q;
		} else {
			while( *q != '\r' ) ++q;
		}
		ZnkStrAry_push_bk_cstr( hdrs->hdr1st_, p, q-p );
		p = q;
		++arg_idx;
	}

	line_begin = p + 2;

	/***
	 * status line ��Request Line�̏ꍇ�A
	 * ��Q������Request-URI�ƂȂ�.
	 */
	if( is_request ){
		ZnkStr str = ZnkStrAry_at( hdrs->hdr1st_, 1 );
		*as_local_proxy = decideLocalProxy_or_WebServer( str, info->req_urp_, my_port, server_name );
	}

	while( line_begin - (char*)ZnkBfr_data( stream ) < (int)info->hdr_size_ ){
		line_end = Znk_memstr_literal( (uint8_t*)line_begin, info->hdr_size_ - ((uint8_t*)line_begin-ZnkBfr_data(stream)), "\r\n" );
		if( line_end == line_begin ){
			/* end of header */
			break;
		}
		ZnkS_find_key_and_val( line_begin, 0, line_end-line_begin,
				&key_begin, &key_end,
				&val_begin, &val_end,
				":", 1,
				" \t", 2 );
		if( key_begin == Znk_NPOS || key_end == Znk_NPOS || val_begin == Znk_NPOS ){
			MoaiLog_printf( "  MoaiInfo_parseHdr : Error : key_begin=[%u] key_end=[%u] val_begin=[%u]\n",
					key_begin, key_end, val_begin );
			break;
		}
		varp = ZnkHtpHdrs_regist( hdrs->vars_,
				line_begin+key_begin, key_end-key_begin,
				line_begin+val_begin, val_end-val_begin );
		if( varp == NULL ){
			/* memory error */
			exit( EXIT_FAILURE );
		}
		key = ZnkHtpHdrs_key_cstr( varp );
		val = ZnkHtpHdrs_val_cstr( varp, 0 );

		if( ZnkS_eqCase( key, "Referer" ) ){
			ZnkStr str = ZnkHtpHdrs_val( varp, 0 );
			MoaiLog_printf( "  Referer=[%s]\n", ZnkStr_cstr( str ) );
			if( strstr( ZnkStr_cstr( str ), "web_start_page" ) ){
				assert( 0 );
			}
		}
		if( ZnkS_eqCase( key, "Transfer-Encoding" ) ){
			if( ZnkS_eq( val, "chunked" ) ){
				body_info->is_chunked_ = true;
			}
		}
		if( ZnkS_eqCase( key, "Content-Encoding" ) ){
			if( ZnkS_eq( val, "gzip" ) ){
				body_info->is_gzip_ = true;
			}
		}
		if( ZnkS_eqCase( key, "Content-Type" ) ){
			if( ZnkS_isBegin( val, "text/html" ) ){
				body_info->txt_type_ = MoaiText_HTML;
				MoaiLog_printf( "  MoaiInfo_parseHdr : txt_type=HTML\n" );
			} else if(
			    ZnkS_isBegin( val, "text/css" )
			){
				body_info->txt_type_ = MoaiText_CSS;
				MoaiLog_printf( "  MoaiInfo_parseHdr : txt_type=CSS\n" );
			} else if(
			    ZnkS_isBegin( val, "application/javascript" )
			 || ZnkS_isBegin( val, "application/x-javascript" )
			 || ZnkS_isBegin( val, "text/javascript" )
			){
				body_info->txt_type_ = MoaiText_JS;
				MoaiLog_printf( "  MoaiInfo_parseHdr : txt_type=JS\n" );
			} else {
				body_info->txt_type_ = MoaiText_Binary;
				MoaiLog_printf( "  MoaiInfo_parseHdr : txt_type=Binary\n" );
			}
		}
		if( ZnkS_eqCase( key, "Content-Length" ) ){
			ZnkS_getSzU( &body_info->content_length_, val );
		}
		line_begin = line_end + 2;
	}

	{
		const char* hostname = NULL;
		if( is_request ){
			varp = ZnkHtpHdrs_find_literal( hdrs->vars_, "Host" );
			if( varp ){
				hostname = ZnkHtpHdrs_val_cstr( varp, 0 );
			}
		} else {
			hostname = response_hostname;
		}
		if( hostname ){
			MoaiModule mod = MoaiModuleAry_find_byHostname( mod_ary, hostname );
			if( mod ){
				MoaiModule_filtHtpHeader( mod, hdrs->vars_ );
			}
		}
	}


	/***
	 * www1.axfc.net�Ń_�E�����[�h����ۂȂǁAis_unlimited_�ƂȂ��Ă���CGI���X�|���X������.
	 * �����������Content-Type���w�肳��Ă��Ȃ�HTML�ł�����.
	 * ���̏ꍇ�A���̓��e����txt_type�𐄒肵�Ȃ���΂Ȃ�Ȃ�.
	 * �Ƃ͂����A���̒i�K�ł͂܂��w�b�_�����ǂݍ���ł��Ȃ����߁A���̐��菈���͌�܂킵�ɂȂ�.
	 */
	if( !body_info->is_chunked_ && body_info->content_length_ == 0 ){
		/***
		 * chunked����Content-Length���ǂ�����w�肳��Ă��Ȃ��ɂ��ւ�炸
		 * body���Ǝv�킵���]�肪���݂���ꍇ.
		 * �c�O�Ȃ���A���̂悤�ȍs�V�̈����T�[�o�����݂���.
		 * ����Proxy�ȂǂɌ�����.
		 */
		if( line_begin - (char*)ZnkBfr_data( stream ) + 2 < (int)ZnkBfr_size( stream ) ){
			body_info->is_unlimited_ = true;
		}
	}
}
