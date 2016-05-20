/***
 * MoaiServer
 */
#include "Moai_server.h"
#include "Moai_post.h"
#include "Moai_context.h"
#include "Moai_module_ary.h"
#include "Moai_io_base.h"
#include "Moai_connection.h"
#include "Moai_info.h"
#include "Moai_log.h"
#include "Moai_fdset.h"
#include "Moai_parent_proxy.h"
#include "Moai_myf.h"

#include <Znk_server.h>
#include <Znk_socket_ary.h>
#include <Znk_fdset.h>
#include <Znk_err.h>
#include <Znk_net_base.h>
#include <Znk_socket.h>
#include <Znk_sys_errno.h>
#include <Znk_s_base.h>
#include <Znk_missing_libc.h>
#include <Znk_str_ary.h>
#include <Znk_str_fio.h>
#include <Znk_zlib.h>
#include <Znk_def_util.h>
#include <Znk_htp_hdrs.h>
#include <Znk_dlink.h>
#include <Znk_cookie.h>
#include <Znk_txt_filter.h>
#include <Znk_net_ip.h>

#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

static uint16_t    st_moai_port = 8124;
static bool        st_enable_log_file = true;
static bool        st_enable_log_verbose = true;
static char        st_acceptable_host[ 256 ] = "";
static char        st_acceptable_host_prev[ 256 ] = "";
static char        st_server_name[ 256 ] = "";
static uint32_t    st_private_ip = 0;
static MoaiModuleAry st_mod_ary = NULL;
static bool        st_you_need_to_restart_moai = false;
static ZnkStrAry   st_access_allow_ips = NULL;
static ZnkStrAry   st_access_deny_ips  = NULL;



static bool
scanHeaderVar( const char* var_name, const uint8_t* buf,
		size_t result_size, char* val_buf, size_t val_buf_size )
{
	/***
	 * ������Var: �w�b�_�[���C�������o.
	 */
	char var_ptn[ 4096 ];
	char* begin = (char*)buf;
	char* end;
	size_t remain_size = result_size;
	size_t var_ptn_leng;
	char* val_begin;
	Znk_snprintf( var_ptn, sizeof(var_ptn), "%s: ", var_name );
	var_ptn_leng = strlen( var_ptn );
	while( remain_size ){
		end = memchr( begin, '\r', remain_size );
		if( end == NULL ){
			/* Var line not found or broken */
			return false;
		}
		end++;
		if( end - begin >= (ptrdiff_t)remain_size ){
			/* Var line not found or broken */
			return false;
		}
		if( *end != '\n' ){
			/* broken line */
			return false;
		}
		end++;
		if( ZnkS_isCaseBegin( begin, var_ptn ) ){
			val_begin = begin + var_ptn_leng;
			ZnkS_copy( val_buf, val_buf_size, val_begin, end-2 - val_begin );
			/* found */
			return true;
		}
		remain_size -= end - begin;
		begin = end;
	}
	return false;
}


typedef enum {
	 HtpScan_e_None=0
	,HtpScan_e_Request
	,HtpScan_e_Response
	,HtpScan_e_RecvZero
	,HtpScan_e_RecvError
	,HtpScan_e_NotHttpStart
	,HtpScan_e_UnknownMethod
	,HtpScan_e_CannotGetHost
	,HtpScan_e_HostBloking
} HtpScanType;


typedef struct {
	const uint8_t* src_;
	size_t         src_size_;
	ZnkStr         ans_;
} GZipInfo;

static unsigned int
supplyDst( uint8_t* dst_buf, size_t dst_size, void* arg )
{
	GZipInfo* info = (GZipInfo*)arg;
	ZnkStr_append( info->ans_, (char*)dst_buf, dst_size );
	return dst_size;
}
static unsigned int
demandSrc( uint8_t* src_buf, size_t src_size, void* arg )
{
	GZipInfo* info = (GZipInfo*)arg;
	const size_t cpy_size = Znk_MIN( info->src_size_, src_size );
	memmove( src_buf, info->src_, cpy_size );
	info->src_      += cpy_size;
	info->src_size_ -= cpy_size;
	return (unsigned int)cpy_size;
}

static void
processResponse_forText( ZnkSocket O_sock, MoaiContext ctx, MoaiFdSet mfds,
		size_t* content_length_remain, MoaiModule mod )
{
	bool is_direct_download = false;
	MoaiInfo*     info      = ctx->draft_info_;
	MoaiBodyInfo* body_info = &ctx->body_info_;

	size_t result_size = 0;
	if( body_info->is_chunked_ ){
		size_t      dbfr_size;
		const char* chunk_size_cstr_begin;
		const char* chunk_size_cstr_end;
		char        chunk_size_cstr_buf[ 1024 ];
		size_t      chunk_size = 0;
		size_t      chunk_size_cstr_leng = 0;
		size_t      chunk_remain = 0;
		size_t      chunk_begin = info->hdr_size_;

		while( true ){
			dbfr_size = ZnkBfr_size( info->stream_ );
			chunk_size_cstr_begin = (char*)ZnkBfr_data( info->stream_ ) + chunk_begin;
			chunk_size_cstr_end   = Znk_memmem( chunk_size_cstr_begin, dbfr_size-chunk_begin, "\r\n", 2 );
			if( chunk_size_cstr_end == NULL ){
				MoaiIO_recvByPtn( info->stream_, O_sock, mfds, "\r\n", &result_size );
				/* for realloc */
				dbfr_size = ZnkBfr_size( info->stream_ );
				chunk_size_cstr_begin = (char*)ZnkBfr_data( info->stream_ ) + chunk_begin;
				chunk_size_cstr_end   = Znk_memmem( chunk_size_cstr_begin, dbfr_size-chunk_begin, "\r\n", 2 );
				assert( chunk_size_cstr_end != NULL );
			}
			chunk_size_cstr_leng = chunk_size_cstr_end-chunk_size_cstr_begin;
			ZnkS_copy( chunk_size_cstr_buf, sizeof(chunk_size_cstr_buf),
					chunk_size_cstr_begin, chunk_size_cstr_leng );
			if( !ZnkS_getSzX( &chunk_size, chunk_size_cstr_buf ) ){
				MoaiLog_printf( "chunk_size_cstr is broken. dump dbfr_dump.dat\n" );
				{
					ZnkFile fp = ZnkF_fopen( "dbfr_dump.dat", "wb" );
					if( fp ){
						const uint8_t* data = ZnkBfr_data( info->stream_ );
						const size_t   size = ZnkBfr_size( info->stream_ );
						ZnkF_fwrite( data, 1, size, fp );
						ZnkF_fclose( fp );
					}
					assert( 0 );
				}
			} else {
				//MoaiLog_printf( "chunk_size=[%u]\n", chunk_size );
			}
			/***
			 * dbfr�ɂ�����chunk_size_cstr�̗̈�����.
			 */
			ZnkBfr_erase( info->stream_, chunk_begin, chunk_size_cstr_leng+2 );

			if( ZnkBfr_size( info->stream_ ) < chunk_begin + chunk_size + 2 ){
				chunk_remain = chunk_begin + chunk_size + 2 - ZnkBfr_size( info->stream_ );
			} else {
				chunk_remain = 0;
			}
			if( chunk_remain ){
				if( !MoaiIO_recvBySize( info->stream_, O_sock, mfds, chunk_remain, &result_size ) ){
					MoaiLog_printf( "MoaiIO_recvBySize : error[%s]\n", ZnkStr_cstr(ctx->msgs_) );
				}
			}
			/***
			 * dbfr�ɂ�����chunk�̍Ō�ɂ���\r\n�����.
			 * ( chunk_size_cstr�� 0 �̏ꍇ�ł����ɋ󔒍s������݂��邽�߁A���ꂪ�K�v.
			 * ����Ɍ����Ɍ����΁A���̑O�Ƀt�b�^�[�s������\�������邪�A���͍l�����Ȃ�)
			 */
			ZnkBfr_erase( info->stream_, chunk_begin+chunk_size, 2 );
			if( chunk_size == 0 ){
				break;
			}
			chunk_begin += chunk_size;
		}
	} else if( ctx->body_info_.is_unlimited_ ){
		MoaiLog_printf( "  MoaiIO_recvByZero Mode Begin\n" );
		MoaiIO_recvByZero( info->stream_, O_sock, mfds, &result_size );
		MoaiLog_printf( "  MoaiIO_recvByZero Mode End\n" );
	} else {
		while( *content_length_remain ){
			MoaiIO_recvBySize( info->stream_, O_sock, mfds, *content_length_remain, &result_size );
			if( *content_length_remain >= result_size ){
				*content_length_remain -= result_size;
			} else {
				assert(0);
			}
		}
	}

	{
		const uint8_t* body_data = ZnkBfr_data( info->stream_ ) + info->hdr_size_;
		const size_t   body_size = ZnkBfr_size( info->stream_ ) - info->hdr_size_;

		ZnkStr_clear( ctx->text_ );
		if( body_info->is_gzip_ ){
			static const bool method1 = false;
			uint8_t dst_buf[ 4096 ];
			ZnkZStream zst = ZnkZStream_create();
			ZnkZStream_inflateInit( zst );

			if( method1 ){
				size_t expanded_dst_size = 0;
				size_t expanded_src_size = 0;
				const uint8_t* src;
				size_t src_size;

				src      = body_data;
				src_size = body_size;
				while( src_size ){
					if( !ZnkZStream_inflate( zst, dst_buf, sizeof(dst_buf), src, src_size,
							&expanded_dst_size, &expanded_src_size ) ){
						MoaiLog_printf( "expanded_src_size=[%zu]\n", expanded_src_size );
					}
					assert( expanded_src_size );
					ZnkStr_append( ctx->text_, (char*)dst_buf, expanded_dst_size );
					src_size -= expanded_src_size;
					src += expanded_src_size;
				}
			} else {
				uint8_t src_buf[ 4096 ];
				GZipInfo gzip_info = { 0 };

				gzip_info.src_      = body_data;
				gzip_info.src_size_ = body_size;
				gzip_info.ans_      = ctx->text_;

				while( gzip_info.src_size_ ){
					if( !ZnkZStream_inflate2( zst,
								dst_buf, sizeof(dst_buf), supplyDst, &gzip_info,
								src_buf, sizeof(src_buf), demandSrc, &gzip_info ) )
					{
						assert( 0 );
					}
				}
			}
			ZnkZStream_inflateEnd( zst );
			ZnkZStream_destroy( zst );

		} else {
			ZnkStr_append( ctx->text_, (char*)body_data, body_size );
		}
	}

	if( mod ){
		ZnkTxtFilterAry txt_ftr = NULL;
		switch( body_info->txt_type_ ){
		case MoaiText_HTML:
			txt_ftr = MoaiModule_ftrHtml( mod );
			break;
		case MoaiText_JS:
			txt_ftr = MoaiModule_ftrJS( mod );
			break;
		case MoaiText_CSS:
			txt_ftr = MoaiModule_ftrCSS( mod );
			break;
		default:
			break;
		}
		MoaiModule_invokeOnResponse( mod, info->hdrs_.vars_, ctx->text_, ZnkStr_cstr(info->req_urp_) );
		if( txt_ftr ){
			ZnkTxtFilterAry_exec( txt_ftr, ctx->text_ );
		}
	}

	if( is_direct_download ){
		/***
		 * ����ł͂܂��Œ薼�Ȃ̂Ŗ��Ȃ����A
		 * ����������host��ɂ���t�@�C�����ŕۑ�����ꍇ��
		 * �J�����g�f�B���N�g���֒��ɕۑ�����̂͊댯�ł���̂�
		 * �Ⴆ��downloads �f�B���N�g���Ȃǂ��쐬���Ă��̔z���֕ۑ�����悤��
		 * �`�ɂ��������悢.
		 */
		ZnkFile fp = ZnkF_fopen( "result.html", "wb" );
		if( fp ){
			const uint8_t* body_data = ZnkBfr_data( info->stream_ ) + info->hdr_size_;
			const size_t   body_size = ZnkBfr_size( info->stream_ ) - info->hdr_size_;

			if( body_info->is_gzip_ ){
				uint8_t dst_buf[ 4096 ];
				size_t expanded_dst_size = 0;
				size_t expanded_src_size = 0;
				const uint8_t* src;
				size_t src_size;
				ZnkZStream zst = ZnkZStream_create();

				ZnkZStream_inflateInit( zst );
				src      = body_data;
				src_size = body_size;
				while( src_size ){
					ZnkZStream_inflate( zst, dst_buf, sizeof(dst_buf), src, src_size,
							&expanded_dst_size, &expanded_src_size );
					assert( expanded_src_size );
					ZnkF_fwrite( dst_buf, 1, expanded_dst_size, fp );
					src_size -= expanded_src_size;
					src += expanded_src_size;
				}
				ZnkZStream_inflateEnd( zst );

				ZnkZStream_destroy( zst );
			} else {
				ZnkF_fwrite( body_data, 1, body_size, fp );
			}

			ZnkF_fclose( fp );
		}
	} else {
		ZnkFile fp = NULL;
		switch( body_info->txt_type_ ){
		case MoaiText_HTML:
			fp = ZnkF_fopen( "result.html", "wb" );
			break;
		case MoaiText_JS:
			fp = ZnkF_fopen( "result.js", "wb" );
			break;
		case MoaiText_CSS:
			fp = ZnkF_fopen( "result.css", "wb" );
			break;
		default:
			break;
		}
		if( fp ){
			const uint8_t* html_data = (uint8_t*)ZnkStr_cstr( ctx->text_ );
			const size_t   html_size = ZnkStr_leng( ctx->text_ );
			ZnkF_fwrite( html_data, 1, html_size, fp );
			ZnkF_fclose( fp );
		}
	}

	/***
	 * I���ɂ�ctx->text_�̒l�ŕԐM����.
	 * ����ɔ����AHeader�ɂ�����Content-Length�̒l���C������.
	 */
	{
		ZnkHtpHdrs hdrs = &info->hdrs_;
		const char* key;
		bool exist_content_length = false;
		size_t size;
		size_t idx;
		ZnkVarp varp;

		ZnkHtpHdrs_erase( hdrs->vars_, "Transfer-Encoding" );

		size = ZnkVarpAry_size( hdrs->vars_ );
		for( idx=0; idx<size; ++idx ){
			varp = ZnkVarpAry_at( hdrs->vars_, idx );
			key = ZnkHtpHdrs_key_cstr( varp );

			if( ZnkS_eqCase( key, "Content-Length" ) ){
				ZnkStr str = ZnkHtpHdrs_val( varp, 0 );
				size_t new_contet_length = ZnkStr_leng( ctx->text_ );
				ZnkStr_sprintf( str, 0, "%u", new_contet_length );
				exist_content_length = true;
			}
			if( ZnkS_eqCase( key, "Content-Encoding" ) ){
				ZnkStr str = ZnkHtpHdrs_val( varp, 0 );
				/* identity�͓��ʂ�Encoding�͂Ȃ������� */
				ZnkStr_set( str, "identity" );
			}
		}
		if( !exist_content_length ){
			char buf[ 1024 ];
			size_t new_contet_length = ZnkStr_leng( ctx->text_ );
			key = "Content-Length";
			Znk_snprintf( buf, sizeof(buf), "%u", new_contet_length );
			ZnkHtpHdrs_regist( hdrs->vars_,
					key, strlen(key),
					buf, strlen(buf) );
		}
	}
}

static void
sendHdrs( ZnkSocket sock, ZnkStrAry hdr1st, const ZnkVarpAry vars )
{
	ZnkBfr bfr = ZnkBfr_create_null();
	ZnkHtpHdrs_extendToStream( hdr1st, vars, bfr, true );
	ZnkSocket_send( sock, ZnkBfr_data(bfr), ZnkBfr_size(bfr) );
}

static void
debugHdrVars( const ZnkVarpAry vars, const char* prefix_tab )
{
	const size_t size = ZnkVarpAry_size( vars );
	size_t  idx;
	size_t  val_idx;
	size_t  val_size;
	ZnkVarp varp = NULL;
	for( idx=0; idx<size; ++idx ){
		varp = ZnkVarpAry_at( vars, idx );
		val_size = ZnkHtpHdrs_val_size( varp );
		MoaiLog_printf( "%s", prefix_tab );
		MoaiLog_printf( "[%s]: ", ZnkHtpHdrs_key_cstr(varp) );
		for( val_idx=0; val_idx<val_size; ++val_idx ){
			MoaiLog_printf( "[%s]",  ZnkHtpHdrs_val_cstr(varp,val_idx) );
		}
		MoaiLog_printf( "\n" );
	}
}


static HtpScanType
scanHttpFirst( MoaiContext ctx, MoaiConnection mcn,
		ZnkSocket sock, MoaiSockType sock_type, ZnkFdSet fdst_observe_r, MoaiFdSet mfds )
{
	MoaiHtpType htp_type = MoaiHtpType_e_None;
	MoaiInfo* draft_info = ctx->draft_info_;
	char sock_str[ 4096 ];

	/***
	 * �����ł̓p�C�v���C��������A�������݂���ꍇ�͍Ō�̗v�f��Ԃ����ƂɂȂ邪�A
	 * ���������p�C�v���C������HEAD/GET�Ȃǂ��g�ݍ��킳���Ă���悤�ȏꍇ�݂̂ɋN����.
	 * POST/CONNECT�ɂ��ẮA�u�������p�C�v���C�������Ă͂Ȃ�Ȃ��v�Ƃ���HTTP��̋K�������邽�߁A
	 * ���̉\����S�z���Ȃ��Ă��悢.
	 * �����Ĉȉ��ł�last_invoked_info��CONNECT�̔���݂̂Ɏg�p����.
	 */
	MoaiInfo*       last_invoked_info = MoaiConnection_getInvokedInfo( mcn, Znk_NPOS );
	ZnkHtpReqMethod last_req_method   = last_invoked_info ? last_invoked_info->req_method_ : ZnkHtpReqMethod_e_Unknown;

	ctx->buf_[0] = '\0';
	ctx->result_size_ = 0;
	ctx->recv_executed_ = false;
	ctx->req_method_ = ZnkHtpReqMethod_e_Unknown;
	ZnkSocket_getPeerIPandPort( sock, &ctx->peer_ipaddr_, NULL );

	MoaiBodyInfo_clear( &ctx->body_info_  );
	ZnkStr_clear( ctx->msgs_ );
	ZnkStr_clear( ctx->text_ );
	MoaiInfo_clear( draft_info );

	MoaiLog_printf( "Moai : scanHttpFirst[%s] %s\n",
			sock_type == MoaiSockType_e_Inner ? "Inner" : sock_type == MoaiSockType_e_Outer ? "Outer" : "None",
			MoaiIO_makeSockStr( sock_str, sizeof(sock_str), sock, true ) );
	{
		size_t arg_pos[ 3 ];
		size_t arg_leng[ 3 ];
		char   arg_tkns[ 3 ][ 4096 ];
		int http_version = 0;
		bool check_Host = false;

		ctx->result_size_ = ZnkSocket_recv( sock, ctx->buf_, sizeof(ctx->buf_) );
		if( ctx->result_size_ <= 0 ){
			MoaiIO_addAnalyzeLabel( ctx->msgs_, sock, ctx->result_size_, "scanHttpFirst:first_recv" );
			/***
			 * ����CONNECT�̏ꍇ�ARecvZero�����o�����̂�I���ł���AO���ł���A
			 * CONNECT�ʐM�̏I�����Ӗ����A���̑��葤�����Ȃ���΂Ȃ�Ȃ����Ƃɒ��ӂ���kj.
			 */
			MoaiConnection_erase( mcn, mfds );
			if( ctx->result_size_ == 0 ){
				ZnkStr_addf( ctx->msgs_, "RecvZero.\n" );
			} else {
				char errmsg_buf[ 4096 ];
				int  err_code = ZnkNetBase_getLastErrCode();
				ZnkNetBase_getErrMsg( errmsg_buf, sizeof(errmsg_buf), err_code );
				ZnkStr_addf( ctx->msgs_, "RecvError.\n" );
				ZnkStr_addf( ctx->msgs_, "  errmsg=[%s]\n", errmsg_buf );
			}
			MoaiLog_printf( "%s", ZnkStr_cstr( ctx->msgs_ ) );
			return ctx->result_size_ == 0 ? HtpScan_e_RecvZero : HtpScan_e_RecvError;
		}
		ZnkBfr_append_dfr( draft_info->stream_, ctx->buf_, ctx->result_size_ );
		ctx->recv_executed_ = true;

		if( last_req_method == ZnkHtpReqMethod_e_CONNECT ){
			/***
			 * �����CONNECT�ɂ��Unknown�ȒʐM�ł���.
			 * ���̏ꍇ��������NotHttpStart�Ƃ��Ă悢.
			 */
			ZnkStr_addf( ctx->msgs_, "  NotHttpStart (CONNECT tunneling : result_size=[%d]).\n", ctx->result_size_ );
			MoaiLog_printf( "%s", ZnkStr_cstr( ctx->msgs_ ) );
			return HtpScan_e_NotHttpStart;
		}

		/***
		 * �ʏ��Http�ɂ��ʐM�ł���͂�.
		 * ���̏ꍇ�Acontent_length_remain_����zero�ł���Ȃ�A
		 * Http�̊J�n�ł͂Ȃ�.
		 *
		 * TODO:
		 * ���Http�̏I���̃^�C�~���O��RecvZero�ɗ���킯�ɂ͂����Ȃ�.
		 * Http�ɂ����Ă͂��ꂪ�����Ă���ۏ�͂Ȃ��A����ǂ��납
		 * �����\�P�b�g�����̑S���قȂ�T�[�o�ւ̐V����Http�ڑ������̗\�����Ȃ���
		 * �s���Ă��邱�Ƃ��炠��(SrcRecycle).
		 * ����āA���̏I���̃^�C�~���O�͂����Ŋ��S�ɏ������Ă����K�v������.
		 * ���̔���́AContent-Length��Trasfer-Encoding�̎w��ɗ��鑼�Ȃ�.
		 * Content-Length���w�肳��Ă��Ȃ����邢��0�ł���ɂ��ւ�炸
		 * Content body�����݂���ꍇ��Trasfer-Encoding��chunked���w�肳��Ă���P�[�X�ł���.
		 * �������܂Ƃ߂ď������邽�߁AContent-Length���w�肳��Ă���ꍇ��chunk_size_�Ƃ��Ĉ���.
		 */
		if( mcn->content_length_remain_ ){
			if( mcn->content_length_remain_ >= (size_t)ctx->result_size_ ){
				mcn->content_length_remain_ -= ctx->result_size_;
				ZnkStr_addf( ctx->msgs_, "  NotHttpStart (remain=[%u] result_size=[%d]).\n",
						mcn->content_length_remain_, ctx->result_size_ );
				MoaiLog_printf( "%s", ZnkStr_cstr( ctx->msgs_ ) );
				return HtpScan_e_NotHttpStart;
			} else {
				/***
				 * ���̏ꍇ�Acontent_length_remain_�̒l�̕������͂�M�p�Ȃ�Ȃ�.
				 * ���ʐM�ɂ����ēr���v���I�Ȓ��f���������A���̖��c���c���Ă���󋵂Ǝv����.
				 * ���̏ꍇ��content_length_remain_�̒l����U 0 �ɖ߂��Afirst_line�̎擾��
				 * ���݂�ׂ��ł���.
				 */
				/* for debugger print */
				const size_t extra_size = ctx->result_size_ - mcn->content_length_remain_;
				ZnkStr_addf( ctx->msgs_, "  Invalid content_length_remain : (remain=[%u] result_size=[%d] extra_size=[%u]).\n",
						mcn->content_length_remain_, ctx->result_size_, extra_size );
				MoaiLog_printf( "%s", ZnkStr_cstr( ctx->msgs_ ) );
				mcn->content_length_remain_ = 0;
			}
		}

		/***
		 * buf���ɍs�̏I���p�^�[�����܂܂�邩�ǂ����𒲂ׁA
		 * �ŏ��̍s��S�擾����܂�recv���J��Ԃ�.
		 */
		MoaiLog_printf( "  Try to recv First Line.\n" );
		if( !MoaiIO_recvInPtn( draft_info->stream_, sock, mfds, "\r\n", NULL ) ){
			MoaiLog_printf( "  First Line Recv Error.\n" );
			return HtpScan_e_RecvError;
		}

#define GET_ARG_POS( idx ) \
		arg_pos[ idx ] = ZnkS_lfind_arg( (char*)ZnkBfr_data(draft_info->stream_), 0, ZnkBfr_size(draft_info->stream_), \
				idx, arg_leng+idx, " \t\r\n", 4 ); \

#define COPY_ARG_TKNS( idx ) \
		ZnkS_copy( arg_tkns[ idx ], sizeof(arg_tkns[ idx ]), (char*)ZnkBfr_data(draft_info->stream_)+arg_pos[ idx ], arg_leng[ idx ] )

		GET_ARG_POS( 0 );
		COPY_ARG_TKNS( 0 );
		GET_ARG_POS( 1 );
		COPY_ARG_TKNS( 1 );
		GET_ARG_POS( 2 );
		{
			char* p = (char*)ZnkBfr_data(draft_info->stream_)+arg_pos[ 2 ];
			char* q = strchr( p, '\r' );
			ZnkS_copy( arg_tkns[ 2 ], sizeof(arg_tkns[ 2 ]), p, (size_t)(q-p) );
		}

#undef GET_ARG_POS
#undef COPY_ARG_TKNS

		/***
		 * TODO:
		 * �����ł́A�ŏ��̈���recv �ōŒ�� first line �S�̂̎擾�͐����������Ƃ�
		 * �O��Ƃ��Ă���. ����result_size���ɂ߂ď������A����̎擾�ɂ����s�����ꍇ�A
		 * Http�̊J�n�ł���ɂ��ւ�炸�ȉ��̔�r�Ɏ��s����V�i���I���l������.
		 * �]���āA���̍ŏ��̈���recv�́A���i�ɂ͈�̍s���ǂݍ��܂��܂ŌJ��Ԃ��ׂ��ł���.
		 */
		if( ZnkS_eqCase( arg_tkns[ 2 ], "HTTP/1.1" ) ){
			/* HTTP request */
			http_version = 11;
			htp_type = MoaiHtpType_e_Request;
		} else if( ZnkS_eqCase( arg_tkns[ 2 ], "HTTP/1.0" ) ){
			/* HTTP request */
			http_version = 10;
			htp_type = MoaiHtpType_e_Request;
		} else if( ZnkS_eqCase( arg_tkns[ 0 ], "HTTP/1.1" ) ){
			/* HTTP response */
			http_version = 11;
			htp_type = MoaiHtpType_e_Response;
		} else if( ZnkS_eqCase( arg_tkns[ 0 ], "HTTP/1.0" ) ){
			/* HTTP response */
			http_version = 10;
			htp_type = MoaiHtpType_e_Response;
		} else {
			MoaiIO_addAnalyzeLabel( ctx->msgs_, sock, ctx->result_size_, "scanHttpFirst:NotHttpStart" );
			ZnkStr_addf( ctx->msgs_, "NotHttpStart.\n" );
			MoaiLog_printf( "%s", ZnkStr_cstr( ctx->msgs_ ) );
			htp_type = MoaiHtpType_e_NotHttpStart;
			return HtpScan_e_NotHttpStart;
		}

		/* �Ƃ肠���������_�ł͖��g�p */
		Znk_UNUSED( http_version );

		/***
		 * ���̎��_��Http�̊J�n�炵���p�^�[���͌��o���ꂽ���ƂɂȂ邪�A
		 * ������content_length_remain_�̒l���Q�Ƃ��A���ꂪ��zero�ł���ꍇ�ɂ́A
		 * ���̃f�[�^��Http Body�̓r���Ƃ݂Ȃ��ANotHttpStart�Ƃ��ׂ��ł���.
		 * �Ⴆ�΁AHttp�w�b�_���̂��̂�������Ă���悤�ȃT�C�g���������Ƃ��āA
		 * content body���Ƀw�b�_�̊J�n���̂悤�ȕ����񂪋L�ڂ���Ă���悤�ȃP�[�X���l������.
		 * ������recv�̐؂�ڂ����x���̂悤�ȕ����ɂ�����悤�ȏꏊ�ł��܂��܋�؂�Ă����ꍇ
		 * ���̕�����Http�̊J�n�Ɣ��f����̂͌��ł���.
		 * ����Ȃ��Ƃ��N����m���͋ɂ߂ĒႢ���A�\����������肱��ɑΏ����Ȃ���΂Ȃ�Ȃ�.
		 *
		 * ����ACONNECT�Ȃǂ�Http�ȊO�̒ʐM���s���Ă���ꍇ�́A���͂₻�̓��e�Ɋւ��Ă�
		 * ���f�̎�i���Ȃ�. ���̏ꍇ�Acontent_length_remain_�͏�� 0 �Ƃ�����̂ƒ�߁A
		 * ���̎�̔���͍s��Ȃ�(RecvZero�œK�؂�RAR����؂��邱�Ƃ����҂��邵���Ȃ�).
		 */

		/***
		 * content_length_remain_ == 0�Ȃ��Http�̊J�n�Ƃ��Ċm�肳����.
		 * buf���Ƀw�b�_�̏I���p�^�[�����܂܂�邩�ǂ����𒲂ׁA
		 * �w�b�_����S�擾����܂�recv���J��Ԃ�.
		 */
		if( !MoaiIO_recvInPtn( draft_info->stream_, sock, mfds, "\r\n\r\n", &draft_info->hdr_size_ ) ){
			MoaiLog_printf( "  Hdr Recv Error.\n" );
			return HtpScan_e_RecvError;
		}

		/***
		 * localhost�ւ�loopback�ڑ����Ȃ�Ƃ��Ă����o���A
		 * �ڑ��̖������[�v������˂΂Ȃ�Ȃ�.
		 * �l������V�i���I�͈ȉ��ł���.
		 * moai���J���Ă���port�ԍ���P�Ƃ���.
		 *
		 * Browser���� localhost:P�֖����I��GET�ڑ�
		 * ���̂Ƃ�P��80�łȂ����Browser��proxy���������moai�ɐڑ������.
		 * moai��webserver�Ƃ��Ă�����������Ȃ���΂Ȃ�Ȃ��̂�
		 * �����proxy�Ƃ��Ă���𒆌p����.
		 * ���̂Ƃ����p��͂܂�localhost:P�ƂȂ�.
		 *
		 * Browser���� localhost:P�֖����I��GET�ڑ�
		 * ���̂Ƃ�P��80�ł���΂܂�moai��proxy�Ƃ��Ă���𒆌p���Alocalhost:P�֐ڑ������.
		 * moai��webserver�Ƃ��Ă�����������Ȃ���΂Ȃ�Ȃ��̂�
		 * �����proxy�Ƃ��Ă���𒆌p����.
		 * ���̂Ƃ����p��͂܂�localhost:P�ƂȂ�.
		 */

		ZnkHtpHdrs_clear( &draft_info->hdrs_ );
		MoaiInfo_parseHdr( draft_info, &ctx->body_info_,
				&ctx->as_local_proxy_, st_moai_port, (bool)(htp_type == MoaiHtpType_e_Request),
				ZnkStr_cstr(mcn->hostname_), st_mod_ary, st_server_name );

		switch( htp_type ){
		case MoaiHtpType_e_Response:
		{
			MoaiBodyInfo* body_info      = &ctx->body_info_;
			const char*   req_method_str = "Unknown";
			const char*   req_urp        = "";
			const size_t  response_idx   = MoaiConnection_countResponseIdx( mcn );
			MoaiInfo* info_on_response   = MoaiConnection_getInvokedInfo( mcn, response_idx );
			if( info_on_response ){
				req_method_str = ZnkHtpReqMethod_getCStr( info_on_response->req_method_ );
				req_urp        = ZnkStr_cstr( info_on_response->req_urp_ );
				draft_info->req_method_ = info_on_response->req_method_;
				ZnkStr_set( draft_info->req_urp_, req_urp );
			}
			ZnkStr_addf( ctx->msgs_, "  Http Response [%s][%s](for[%s %s %s]).\n",
					arg_tkns[ 1 ], arg_tkns[ 2 ],
					req_method_str, MoaiConnection_hostname(mcn), req_urp );
			MoaiLog_printf( "%s", ZnkStr_cstr( ctx->msgs_ ) );
			switch( info_on_response->req_method_ ){
			case ZnkHtpReqMethod_e_GET:
			case ZnkHtpReqMethod_e_POST:
			case ZnkHtpReqMethod_e_HEAD:
				if( ZnkS_eq( arg_tkns[ 1 ], "200" ) ){
					MoaiLog_printf( "  body_info : is_chunked=[%s] is_gzip=[%s] txt_type=[%d]\n",
							body_info->is_chunked_ ? "true" : "false",
							body_info->is_gzip_    ? "true" : "false",
							body_info->txt_type_ );
				}
				break;
			default:
				break;
			}
			break;
		}
		case MoaiHtpType_e_Request:
		{
			const char* method = "";
			char hostname_buf[ 4096 ] = "";
			uint16_t port = 0;

			if( ZnkS_isCaseBegin( arg_tkns[ 1 ], "http://" ) ){
				/* Request-URI�ɂ����ăz�X�g�����L�ڂ���Ă���Ƃ݂Ȃ�.
				 * �ŏ���http://�͏������Ă��悩�낤.
				 * ����ɊO����proxy�֓n���ꍇ�A���̂܂܂ł悢���A
				 * �����ł͂Ȃ��ꍇ�Aurp�̌`�ɏC�����Ȃ���΂Ȃ�Ȃ�.
				 */
				ZnkS_copy( hostname_buf, sizeof(hostname_buf), arg_tkns[ 1 ]+7, arg_leng[ 1 ]-7 );
			} else if( ZnkS_isCaseBegin( arg_tkns[ 1 ], "/" ) ){
				check_Host = true;
			} else if( ZnkS_isCaseBegin( arg_tkns[ 1 ], "*" ) ){
				check_Host = true;
			} else {
				/* Request-URI�ɂ����ăz�X�g�����L�ڂ���Ă���Ƃ݂Ȃ� */
				ZnkS_copy( hostname_buf, sizeof(hostname_buf), arg_tkns[ 1 ], arg_leng[ 1 ] );
			}

			/***
			 * �O�̂��߁A��1�Ԗڂ�tkn���m�F.
			 * �ȉ��̂�����ł��Ȃ��ꍇ�̓G���[�Ƃ݂Ȃ�.
			 */
			if( ZnkS_eqCase(        arg_tkns[ 0 ], "GET" ) ){
				ctx->req_method_ =   ZnkHtpReqMethod_e_GET;
			} else if( ZnkS_eqCase( arg_tkns[ 0 ], "POST" ) ){
				ctx->req_method_ =   ZnkHtpReqMethod_e_POST;
			} else if( ZnkS_eqCase( arg_tkns[ 0 ], "HEAD" ) ){
				ctx->req_method_ =   ZnkHtpReqMethod_e_HEAD;
			} else if( ZnkS_eqCase( arg_tkns[ 0 ], "PUT" ) ){
				ctx->req_method_ =   ZnkHtpReqMethod_e_PUT;
			} else if( ZnkS_eqCase( arg_tkns[ 0 ], "DELETE" ) ){
				ctx->req_method_ =   ZnkHtpReqMethod_e_DELETE;
			} else if( ZnkS_eqCase( arg_tkns[ 0 ], "CONNECT" ) ){
				ctx->req_method_ =   ZnkHtpReqMethod_e_CONNECT;
			} else if( ZnkS_eqCase( arg_tkns[ 0 ], "OPTIONS" ) ){
				ctx->req_method_ =   ZnkHtpReqMethod_e_OPTIONS;
			} else if( ZnkS_eqCase( arg_tkns[ 0 ], "TRACE" ) ){
				ctx->req_method_ =   ZnkHtpReqMethod_e_TRACE;
			} else {
				/* not support */
				MoaiIO_addAnalyzeLabel( ctx->msgs_, sock, ctx->result_size_, "scanHttpFirst:Request" );
				ZnkStr_addf( ctx->msgs_, "UnknownMethod.\n" );
				MoaiLog_printf( "%s", ZnkStr_cstr( ctx->msgs_ ) );
				MoaiIO_close_ISock( "  UnknownMethod", sock, mfds );
				return HtpScan_e_UnknownMethod;
			}
			method = arg_tkns[ 0 ];

			ZnkStr_addf( ctx->msgs_, "  Http Request [%s][%s].\n", method, arg_tkns[1] );
			MoaiLog_printf( "%s", ZnkStr_cstr( ctx->msgs_ ) );

			if( check_Host ){
				const uint8_t* hdr_data = ZnkBfr_data( draft_info->stream_ );
				const size_t   hdr_size = draft_info->hdr_size_;
				if( !scanHeaderVar( "Host", hdr_data, hdr_size, hostname_buf, sizeof(hostname_buf) ) ){
					MoaiIO_close_ISock( "  CannotGetHost", sock, mfds );
					return HtpScan_e_CannotGetHost;
				}
			}
			if( !ZnkS_empty(hostname_buf) ){
				char* colon;
				char* slash;
				colon = strchr( hostname_buf, ':' );
				slash = strchr( hostname_buf, '/' );
				if( colon ){
					if( slash && slash < colon ){
						/***
						 * �I�[�\���e�B�������ɂ�URL�G���R�[�f�B���O�Ȃǂɂ��
						 * :���܂܂��\��������. ����colon��slash�����̂Ƃ���
						 * ���̃P�[�X�ł���ƍl�����邽�߁A����colon�͖�������K�v������.
						 */
						port = 80;
					} else {
						/***
						 * slash���Ȃ����܂���colon��slash�����O�ł���ꍇ��
						 * hostname��port�ԍ�����؂� : �ł���.
						 */
						*colon = '\0';
						sscanf( colon+1, "%hd", &port );
					}
				} else {
					port = 80;
				}
				if( slash ){
					*slash = '\0';
				}
			}
			if( !ZnkStr_eq( mcn->hostname_, hostname_buf ) ||  mcn->port_ != port ){
				/***
				 * ���̏ꍇ�Amcn->O_sock_�Ƃ̋�RAS�͊���������A�u���E�U��mcn->I_sock_�̐ڑ���
				 * ���̂܂܈ێ������T�C�N�����āA�V���Ȃ�HTTP�Őڑ����Ă���Ǝv����.
				 * �������A�V���Ȑڑ���z�X�g���ȑO�̂���Ɠ����Ƃ͌���Ȃ�.  
				 * ������`�F�b�N���邽�߁A��̔�r���s���A�z�X�g���܂���port���ȑO�̂���Ƃ�
				 * �قȂ�ꍇ�A��mcn->O_sock_��close���A�V���Ȑڑ�������Ă����V����mcn->O_sock_�Ƃ���.
				 *
				 * ���̂Ƃ��Amcn->I_sock_�̓��T�C�N������Amcn->O_sock_�݂̂����Ȃ�����邱�ƂɂȂ�.
				 * �u���E�U�ɂ���Ă͂��̂悤�Ȃ��Ƃ��L�蓾��.
				 * �Ⴆ��Firefox��www.google.co.jp�ɐڑ����A����ɕʂ̃T�C�g�֐ڑ�����ꍇ�Ȃǂł���.
				 */
				MoaiIO_close_OSock( "  SrcRecycle", mcn->O_sock_, mfds );
			}
			/* �X�V */
			MoaiLog_printf( "  hostname_buf=[%s]\n", hostname_buf );
			ZnkStr_set( mcn->hostname_, hostname_buf );
			mcn->port_ = port;
			break;
		}
		default:
			assert( 0 );
			break;
		}
	}

	/***
	 * content_length_remain_�̏�����.
	 */
	if( ctx->req_method_ == ZnkHtpReqMethod_e_HEAD ){
		mcn->content_length_remain_ = 0;
	} else {
		mcn->content_length_remain_ = ctx->body_info_.content_length_;
		MoaiLog_printf( "  body_info_.content_length_=[%u]\n", ctx->body_info_.content_length_ );
	}

	/***
	 * Connection keep-alive�̗L�����m�F����.
	 */
	{
		ZnkVarp varp = ZnkHtpHdrs_find( draft_info->hdrs_.vars_, "Connection", Znk_NPOS );
		mcn->I_is_keep_alive_ = true;
		if( varp ){
			const char* val  = ZnkHtpHdrs_val_cstr( varp, 0 );
			mcn->I_is_keep_alive_ = ZnkS_eqCase( val, "keep-alive" );
		}
	}

	if( mcn->content_length_remain_ ){
		size_t recved_content_length = ZnkBfr_size( draft_info->stream_ ) - draft_info->hdr_size_;
		if( mcn->content_length_remain_ < recved_content_length ){
			mcn->content_length_remain_ = 0;
		} else {
			mcn->content_length_remain_ -= recved_content_length;
		}
	}


	if( htp_type == MoaiHtpType_e_Response ){
		MoaiConnection mcn      = MoaiConnection_find_byOSock( sock );
		const char*    hostname = MoaiConnection_hostname( mcn );
		MoaiModule     mod      = MoaiModuleAry_find_byHostname( st_mod_ary, hostname );
		MoaiLog_printf( "  Response hostname=[%s]\n", hostname );

		if( mod ){
			ZnkVarp set_cookie = ZnkHtpHdrs_find_literal( draft_info->hdrs_.vars_, "Set-Cookie" );
		
			/***
			 * set_cookie����͂��Amyf��ɂ�����cookie_vars���X�V����.
			 */
			if( set_cookie ){
				const size_t val_size = ZnkHtpHdrs_val_size( set_cookie );
				size_t       val_idx  = 0;
				ZnkMyf       myf      = MoaiModule_ftrSend( mod );
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
			}
		}

		/***
		 * HEAD�̏ꍇ��Connect-Length����[���ł���ɂ��ւ�炸���̃f�[�^�����Ȃ�
		 */
		if( last_req_method == ZnkHtpReqMethod_e_HEAD ){
			if( mod ){
				MoaiModule_invokeOnResponse( mod, draft_info->hdrs_.vars_, NULL, "" );
			}
		} else {
			MoaiBodyInfo* body_info = &ctx->body_info_;
		
			/***
			 * Text�f�[�^�̏ꍇ�͑S�擾�����݂�.
			 */
			if( body_info->txt_type_ != MoaiText_Binary || body_info->is_unlimited_ ){
				processResponse_forText( sock, ctx, mfds, &mcn->content_length_remain_, mod );
			} else if( mod ){
				MoaiModule_invokeOnResponse( mod, draft_info->hdrs_.vars_, NULL, "" );
			}
		} 
		return HtpScan_e_Response;
	}
	{
		const char* hostname     = MoaiConnection_hostname( mcn );
		uint16_t    port         = MoaiConnection_port( mcn );
		ZnkMyf      config       = MoaiMyf_theConfig();
		ZnkStrAry   ignore_hosts = ZnkMyf_find_lines( config, "ignore_hosts" );

		MoaiLog_printf( "  New Destination : [%s:%d]\n", hostname, port );
		if( ZnkStrAry_find_isMatch( ignore_hosts, 0, hostname, Znk_NPOS, ZnkS_isMatchSWC ) != Znk_NPOS ){
			int ret = MoaiIO_sendTxtf( sock, "text/html", "Moai : Ignored Host Blocking[%s] sock=[%d].\n",
					hostname, sock );
			MoaiLog_printf( "  Ignored hostname=[%s]\n", hostname );
			if( ret < 0 || !mcn->I_is_keep_alive_ || ctx->req_method_ == ZnkHtpReqMethod_e_CONNECT ){
				MoaiIO_close_ISock( "  IgnoreBlocking", sock, mfds );
			}
			return HtpScan_e_HostBloking;
		}
	}

	return HtpScan_e_Request;
}


static void
debugInterestURP( const MoaiInfo* info, const MoaiConnection mcn, HtpScanType scan_type, ZnkMyf analysis )
{
	const char*    req_urp         = ZnkStr_cstr(info->req_urp_);
	ZnkStrAry      interest_urp    = ZnkMyf_find_lines( analysis, "interest_urp" );
	if( ZnkStrAry_find_isMatch( interest_urp, 0, req_urp, Znk_NPOS, ZnkS_isMatchSWC ) != Znk_NPOS ){
		if( ZnkStrAry_size(info->hdrs_.hdr1st_) > 1 ){
			const char* req_method_str = ZnkHtpReqMethod_getCStr( info->req_method_ );
			MoaiLog_printf( "  Moai %s %s%s : InterestURP=[%s]\n",
					req_method_str,
					( scan_type == HtpScan_e_Request ) ? "Request" : "Response ",
					( scan_type == HtpScan_e_Request ) ? "" : ZnkStrAry_at_cstr(info->hdrs_.hdr1st_,1),
					req_urp );
		}
		debugHdrVars( info->hdrs_.vars_, "    " );
	}
}

static int
sendToDst( ZnkSocket dst_sock, MoaiInfo* info, MoaiTextType txt_type, ZnkStr text, bool with_header )
{
	int result_size = 0;
	if( with_header ){
		/* HTTP�w�b�_�𑗐M */
		sendHdrs( dst_sock, info->hdrs_.hdr1st_, info->hdrs_.vars_ ); 
		if( txt_type != MoaiText_Binary ){
			result_size = ZnkSocket_send( dst_sock,
					(uint8_t*)ZnkStr_cstr(text), ZnkStr_leng(text) );
		} else {
			/* �f�[�^�𑗐M */
			result_size = ZnkSocket_send( dst_sock,
					ZnkBfr_data(info->stream_)+info->hdr_size_, ZnkBfr_size(info->stream_)-info->hdr_size_ );
		}
	} else {
		/* �f�[�^�𑗐M */
		result_size = ZnkSocket_send( dst_sock, ZnkBfr_data(info->stream_), ZnkBfr_size(info->stream_) );
	}
	return result_size;
}

static MoaiRASResult
requestOnConnectCompleted_CONNECT( ZnkSocket O_sock,
		const char* hostname, MoaiInfoID info_id, MoaiFdSet mfds )
{
	ZnkErr_D( err );
	MoaiConnection mcn            = MoaiConnection_find_byOSock( O_sock );
	ZnkSocket      I_sock         = MoaiConnection_I_sock( mcn );
	MoaiInfo*      info           = MoaiInfo_find( info_id );
	ZnkMyf         config         = MoaiMyf_theConfig();
	const bool     is_proxy_use   = MoaiParentProxy_isAppliedHost( config, hostname, Znk_NPOS );
	bool           connection_established = true;

	if( is_proxy_use ){
		/***
		 * ���̏ꍇ��O��(�O��proxy)�ɂ�CONNECT�w�b�_�𑗂�A
		 * ���̉������e��200�ł��邩�ۂ����m�F����K�v������.
		 */
		ZnkBfr bfr = ZnkBfr_create_null();
		ZnkHtpHdrs_extendToStream( info->hdrs_.hdr1st_, info->hdrs_.vars_, bfr, true );
		ZnkSocket_send( O_sock, ZnkBfr_data(bfr), ZnkBfr_size(bfr) );

		MoaiInfo_clear( info );
		ZnkBfr_clear( bfr );
		MoaiLog_printf( "  Try to recv CONNECT request return from proxy.\n" );
		if( !MoaiIO_recvInPtn( bfr, O_sock, mfds, "\r\n\r\n", &info->hdr_size_ ) ){
			MoaiLog_printf( "  Hdr Recv Error.\n" );
			ZnkBfr_destroy( bfr );
			return MoaiRASResult_e_Ignored;
		}

		/***
		 * �O��proxy����̉������e������.
		 * status��200�Ȃ�CONNECT�Ƃ���.
		 */
		connection_established = false;
		{
			const char* p = (char*)ZnkBfr_data(bfr);
			const char* q = strstr( p, "\r\n" );
			if( q ){
				ZnkHtpHdrs_registHdr1st( info->hdrs_.hdr1st_, p, q - p );
				MoaiLog_printf( "Recving status [%s]\n", ZnkStrAry_at_cstr( info->hdrs_.hdr1st_, 1 ) );
				if( ZnkS_eq( ZnkStrAry_at_cstr( info->hdrs_.hdr1st_, 1 ), "200" ) ){
					connection_established = true;
				}
			}
		}
		ZnkBfr_destroy( bfr );
	}
	if( connection_established ){
		/***
		 * �܂�O���ɉ�������Ȃ�.
		 * I���֒P�� HTTP/1.0 200 connection established ��Ԃ��A���߂�I�������recv��select�ɂ��҂�.
		 * ���̂Ƃ��A(Unknown��)�ʐM�̊J�n�ƂȂ�I_sock�́A�ŏ���CONNECT���\�b�h�𑗂��Ă���I_sock��
		 * �����ł���(�����łȂ���΂Ȃ�Ȃ�).
		 * ����āA���̂悤�ȒʐM�̊J�n��r���ł��邩�ۂ���req_method_�l�ɂ���ĊȒP�Ɏ��ʂł���.
		 * �܂��A���̂悤�ȒʐM�̏I���́A���̂����ꂩ�̑��̐ڑ�������ꂽ�ƌ��o�������ł���.
		 * ���̏ꍇ�A����ꂽ������̎c��̃f�[�^�����ׂđ��M���A���̌㗼���̐ڑ�����A
		 * �c�����f�[�^������Ύ̂Ă�A�Ƃ������������݂Ȃ���΂Ȃ�Ȃ��Ƃ���.
		 *
		 * RFC7231: CONNECT
		 * A tunnel is closed when a tunnel intermediary detects that either side
		 * has closed its connection: the intermediary MUST attempt to send any
		 * outstanding data that came from the closed side to the other side,
		 * close both connections, and then discard any remaining data left undelivered.
		 */
		const char* msg = "HTTP/1.0 200 Connection established. \r\n\r\n";
		int result_size = ZnkSocket_send( I_sock, (uint8_t*)msg, strlen(msg) );
		if( result_size == -1 ){
			ZnkSysErrnoInfo* err_info = ZnkSysErrno_getInfo( ZnkSysErrno_errno() );
			ZnkErr_internf( &err,
					"  ZnkSocket_send : Failure : SysErr=[%s:%s]",
					err_info->sys_errno_key_, err_info->sys_errno_msg_ );
			MoaiConnection_erase( mcn, mfds );
			return MoaiRASResult_e_Ignored;
		}
		MoaiLog_printf( "  CONNECT established I[%d] => O[%d].\n", I_sock, O_sock );
	} else {
		ZnkBfr bfr = ZnkBfr_create_null();
		int result_size;
		ZnkHtpHdrs_extendToStream( info->hdrs_.hdr1st_, info->hdrs_.vars_, bfr, true );
		result_size = ZnkSocket_send( I_sock, ZnkBfr_data(bfr), ZnkBfr_size(bfr) );
		ZnkBfr_destroy( bfr );
		if( result_size == -1 ){
			ZnkSysErrnoInfo* err_info = ZnkSysErrno_getInfo( ZnkSysErrno_errno() );
			ZnkErr_internf( &err,
					"  ZnkSocket_send : Failure : SysErr=[%s:%s]",
					err_info->sys_errno_key_, err_info->sys_errno_msg_ );
			MoaiConnection_erase( mcn, mfds );
			return MoaiRASResult_e_Ignored;
		}
	}

	return MoaiRASResult_e_OK;
}

static MoaiRASResult
requestOnConnectCompleted_GET( ZnkSocket O_sock,
		const char* hostname, MoaiInfoID info_id, MoaiFdSet mfds )
{
	ZnkErr_D( err );
	MoaiConnection mcn  = MoaiConnection_find_byOSock( O_sock );
	MoaiInfo*      info = MoaiInfo_find( info_id );

	/***
	 * �Ƃ肠��������send�͎��s���Ȃ��Ɖ���.
	 */
	int result_size = 0;

	sendHdrs( O_sock, info->hdrs_.hdr1st_, info->hdrs_.vars_ ); 

	/* �f�[�^�𑗐M */
	result_size = ZnkSocket_send( O_sock,
			ZnkBfr_data(info->stream_)+info->hdr_size_, ZnkBfr_size(info->stream_)-info->hdr_size_ );

	if( result_size == -1 ){
		ZnkSysErrnoInfo* err_info = ZnkSysErrno_getInfo( ZnkSysErrno_errno() );
		ZnkErr_internf( &err,
				"  ZnkSocket_send : Failure : SysErr=[%s:%s]",
				err_info->sys_errno_key_, err_info->sys_errno_msg_ );
		MoaiConnection_erase( mcn, mfds );
		return MoaiRASResult_e_Ignored;
	}
	return MoaiRASResult_e_OK;
}

static void
sendOnConnected_CONNECT( MoaiConnection mcn, MoaiFdSet mfds, MoaiInfoID info_id )
{
	ZnkSocket   O_sock   = MoaiConnection_O_sock( mcn );
	const char* hostname = MoaiConnection_hostname( mcn );

	requestOnConnectCompleted_CONNECT( O_sock, hostname, info_id, mfds );
	{
		ZnkFdSet fdst_observe = MoaiFdSet_fdst_observe_r( mfds );
		if( !ZnkFdSet_set( fdst_observe, O_sock ) ){
			MoaiFdSet_reserveConnectSock( mfds, O_sock );
		}
	}
}

static void
sendOnConnected_POST( MoaiConnection mcn, MoaiFdSet mfds, MoaiInfoID info_id )
{
	ZnkSocket   O_sock       = MoaiConnection_O_sock( mcn );
	ZnkFdSet    fdst_observe = MoaiFdSet_fdst_observe_r( mfds );

	MoaiPost_sendRealy( info_id, true, O_sock, mfds );
	if( !ZnkFdSet_set( fdst_observe, O_sock ) ){
		MoaiFdSet_reserveConnectSock( mfds, O_sock );
	}
}

static void
sendOnConnected_GET( MoaiConnection mcn, MoaiFdSet mfds, MoaiInfoID info_id )
{
	ZnkSocket   O_sock   = MoaiConnection_O_sock( mcn );
	const char* hostname = MoaiConnection_hostname( mcn );

	requestOnConnectCompleted_GET( O_sock, hostname, info_id, mfds );
	{
		ZnkFdSet fdst_observe = MoaiFdSet_fdst_observe_r( mfds );
		if( !ZnkFdSet_set( fdst_observe, O_sock ) ){
			MoaiFdSet_reserveConnectSock( mfds, O_sock );
		}
	}
}


static MoaiRASResult
doLocalProxy( MoaiContext ctx, MoaiConnection mcn, MoaiFdSet mfds, MoaiHtpType htp_type, MoaiSockType sock_type )
{
	ZnkErr_D( err );
	MoaiInfo* info = NULL;
	ZnkSocket I_sock = MoaiConnection_I_sock( mcn );
	ZnkSocket O_sock = MoaiConnection_O_sock( mcn );
	const char* hostname = MoaiConnection_hostname( mcn );
	uint16_t    port     = MoaiConnection_port( mcn );

	ZnkMyf config     = MoaiMyf_theConfig();
	ZnkMyf analysis   = MoaiMyf_theAnalysis();

	const bool is_proxy_use = MoaiParentProxy_isAppliedHost( config, hostname, Znk_NPOS );
	MoaiConnectedCallback cb_func = NULL;


	if( htp_type == MoaiHtpType_e_Request ){
		MoaiInfoID info_id = MoaiInfo_regist( ctx->draft_info_ );
		info = MoaiInfo_find( info_id );
		info->req_method_ = ctx->req_method_;

		switch( info->req_method_ ){
		case ZnkHtpReqMethod_e_CONNECT:
			cb_func = sendOnConnected_CONNECT;
			break;
		case ZnkHtpReqMethod_e_POST:
		{
			ZnkStr str = ZnkStr_new( "" );
			const MoaiModuleAry mod_ary  = st_mod_ary;
			const MoaiModule mod = MoaiModuleAry_find_byHostname( mod_ary, hostname );
			MoaiPostMode post_mode = MoaiPost_decidePostMode( config, hostname );
			ZnkVarp varp = NULL;

			{
				/* Cookie filtering */
				if( mod ){
					MoaiModule_filtCookieVars( mod, info->hdrs_.vars_ );
				}

				/* PostVar filtering */
				MoaiPost_parsePostVars3( I_sock, mfds,
						str,
						info->hdr_size_, info->hdrs_.hdr1st_, info->hdrs_.vars_,
						ctx->body_info_.content_length_, info->stream_,
						info->vars_,
						mod );
			}

			varp = ZnkVarpAry_find_byName( info->vars_, "Moai_PostInfoID", Znk_NPOS, false );
			if( varp ){
				MoaiInfoIDStr id_str = {{ 0 }};
				const char* moai_post_info_id = ZnkVar_cstr( varp );
				post_mode = MoaiPostMode_e_SendAfterConfirm;
				Znk_snprintf( id_str.buf_, sizeof(id_str.buf_), "%s", moai_post_info_id );
				info_id = MoaiInfoID_scanIDStr( &id_str );
			}
			{
				MoaiInfoIDStr id_str = {{ 0 }};
				MoaiInfoID_getStr( info_id, &id_str );
				MoaiLog_printf( "doLocalProxy : ZnkHtpReqMethod_e_POST id=[%s] mod=[%p]\n", id_str.buf_, mod );
			}
			if( post_mode == MoaiPostMode_e_Confirm ){
				const char* req_urp = ZnkStr_cstr( info->req_urp_ );
				MoaiInfoIDStr id_str = {{ 0 }};
				MoaiInfoID_getStr( info_id, &id_str );
				ZnkStr_addf( str, "<p><b>Moai : POST to %s%s.</b></p>", hostname, req_urp );
				ZnkStr_addf( str, "<form action=\"http://%s%s\" method=\"POST\" enctype=\"multipart/form-data\">",
						hostname, req_urp );
				ZnkStr_addf( str, "<input type=hidden name=Moai_PostInfoID value=\"%s\">", id_str.buf_ );
				ZnkStr_addf( str, "<input type=submit value=\"Send\"> " );
				ZnkStr_addf( str, "</form>\n" );
				MoaiIO_sendTxtf( I_sock, "text/html", "<pre>%s</pre>", ZnkStr_cstr(str) );
				cb_func = NULL;
			} else {
				cb_func = sendOnConnected_POST;
			}
			ZnkStr_delete( str );
			break;
		}
		case ZnkHtpReqMethod_e_GET:
		default:
			MoaiLog_printf( "  doLocalProxy : Request info->req_method_=[%s]\n", ZnkHtpReqMethod_getCStr(info->req_method_) );
			cb_func = sendOnConnected_GET;
			break;
		}
		if( !MoaiConnection_connectFromISock( hostname, port, I_sock, "doLocalProxy",
				info_id, mfds, is_proxy_use, cb_func ) )
		{
			MoaiLog_printf( "  doLocalProxy : makeConnectionFromISock failure.\n" );
			MoaiIO_sendTxtf( I_sock, "text/html", "<p><b>Moai : doLocalProxy : makeConnectionFromISock Error.</b></p>\n" );
			MoaiIO_close_ISock( "  doLocalProxy", I_sock, mfds );
			return MoaiRASResult_e_Ignored;
		}
		I_sock = MoaiConnection_I_sock( mcn );
		O_sock = MoaiConnection_O_sock( mcn );
	}
	if( htp_type == MoaiHtpType_e_Response ){
		info = ctx->draft_info_;
	}
	if( htp_type == MoaiHtpType_e_NotHttpStart ){
		info = ctx->draft_info_;
	}

	if( ctx->recv_executed_ && O_sock != ZnkSocket_INVALID ){

		switch( htp_type ){
		case MoaiHtpType_e_Request:
		{
			debugInterestURP( info, mcn, htp_type, analysis );

			switch( info->req_method_ ){
			case ZnkHtpReqMethod_e_GET:
			case ZnkHtpReqMethod_e_POST:
			case ZnkHtpReqMethod_e_HEAD:
			{
				break;
			}
			default:
				break;
			}
			break;
		}
		case MoaiHtpType_e_Response:
		default:
		{
			/* Response or NotHttpStart */
			/***
			 * �Ƃ肠��������send�͎��s���Ȃ��Ɖ���.
			 */
			const bool with_header = (bool)( htp_type == MoaiHtpType_e_Response );
			int result_size = 0;

			/***
			 * sock_type�͑��M���ł��邩��A���M��ł���sock�͂��̋t�ƂȂ�.
			 */
			ZnkSocket sock = ( sock_type == MoaiSockType_e_Inner ) ? O_sock 
				: ( sock_type == MoaiSockType_e_Outer ) ? I_sock
				: ZnkSocket_INVALID;

			debugInterestURP( info, mcn, htp_type, analysis );
			if( sock == ZnkSocket_INVALID ){
				MoaiLog_printf( "  sock=[%d] is invalid\n", sock );
				return MoaiRASResult_e_Ignored;
			}
			sendToDst( sock, info, ctx->body_info_.txt_type_, ctx->text_, with_header );
			if( result_size == -1 ){
				ZnkSysErrnoInfo* err_info = ZnkSysErrno_getInfo( ZnkSysErrno_errno() );
				ZnkErr_internf( &err,
						"  ZnkSocket_send : Failure : SysErr=[%s:%s]",
						err_info->sys_errno_key_, err_info->sys_errno_msg_ );
				MoaiConnection_erase( mcn, mfds );
				return MoaiRASResult_e_Ignored;
			}

			break;
		}
		}

	}
	return MoaiRASResult_e_OK;

}

/***
 * LOOPBACK�ȊO���T�|�[�g����ꍇ�A���� localhost�ȊO��LAN��̃}�V������̐ڑ���A
 * 192.168.*.* �Ȃǂ�PrivateIP�̌`����localhost���w�肵���ꍇ���󂯕t����ꍇ�ł���.
 * (WAN����̐ڑ��Ƃ����̂����_��͗L�蓾�邪�A�Z�L�����e�B��̓V�X�e���ɂ�����
 * �����Ă͂Ȃ�Ȃ�)
 *
 * ���̏ꍇ�Aserver_name��LAN�ォ��A�N�Z�X�ł��閼�O�Ƃ��Ȃ���΂Ȃ�Ȃ���
 * server_name����l�ɐݒ肵���ꍇ��PrivateIP�̎����擾�����݁A�����ݒ肷����̂Ƃ���.
 * ���̎����擾�Ɏ��s�����ꍇ�� 127.0.0.1 ���Z�b�g�����.
 * ( ���̂Ƃ��AMoai��Web Configuration�ɂ����� localhost�ȊO����̐ݒ�͕s�ƂȂ� )
 */
static void
autosetServerName( void )
{
	char private_ipstr[ 64 ] = "";
	/***
	 * �ŐV��PrivateIP�������擾����.
	 */
	st_private_ip = 0;
	if( ZnkNetIP_getPrivateIP32( &st_private_ip ) ){
		ZnkNetIP_getIPStr_fromU32( st_private_ip, private_ipstr, sizeof(private_ipstr) );
		MoaiLog_printf( "Moai : AutoGet PrivateIP=[%s].\n", private_ipstr );
	}

	if(  ZnkS_empty( st_acceptable_host )
	  || ZnkS_eq( st_acceptable_host, "127.0.0.1" )
	  || ZnkS_eq( st_acceptable_host, "LOOPBACK" )
	){
		/* LOOPBACK�̏ꍇ */
		ZnkS_copy( st_server_name, sizeof(st_server_name), "127.0.0.1", Znk_NPOS );
		return;
	} else if( ZnkS_empty( st_server_name ) || ZnkS_eq( st_server_name, "127.0.0.1" ) ){
		/***
		 * ANY�̏ꍇ����st_server_name����or127.0.0.1�̂Ƃ��Ast_server_name�̒l��PrivateIP�Œu��������.
		 */
		if( st_private_ip == 0 ){
			/***
			 * PrivateIP�̎����擾�Ɏ��s����Ȃǂ�st_private_ip����̂Ƃ�.
			 * �Ƃ肠����LOOPBACK�p�̒l���w�肵�Ă���.
			 */
			ZnkS_copy( st_server_name, sizeof(st_server_name), "127.0.0.1", Znk_NPOS );
		} else {
			ZnkS_copy( st_server_name, sizeof(st_server_name), private_ipstr, Znk_NPOS );
		}
	}
}

static size_t st_input_ui_idx = 0;

static void
printInputUI_Text( ZnkStr html,
		const char* varname, const char* current_val, const char* new_val, bool is_enable,
		const char* destination )
{
	const char* class_name = ( st_input_ui_idx % 2 ) ? "odd" : "evn";
	const char* readonly_str = is_enable ? "" : "readonly disabled=true";
	ZnkStr_addf( html,
			"<tr class=\"%s\"><td>%s</td><td>%s</td><td><input type=text name=%s value=\"%s\" %s></td><td>%s</td></tr>\n",
			class_name,
			varname, current_val, varname, new_val, readonly_str, destination );
	++st_input_ui_idx;
}
static void
printInputUI_CheckBox( ZnkStr html,
		const char* varname, bool current_val, bool new_val, bool is_enable,
		const char* destination )
{
	const char* class_name = ( st_input_ui_idx % 2 ) ? "odd" : "evn";
	const char* readonly_str = is_enable ? "" : "readonly disabled=true";

	ZnkStr_addf( html,
			"<tr class=\"%s\"><td>%s</td><td>%s</td><td><input type=checkbox name=%s value=\"on\" %s %s></td><td>%s</td></tr>\n",
			class_name,
			varname, current_val ? "on" : "off", varname, new_val ? "checked" : "", readonly_str, destination );
	++st_input_ui_idx;
}
static void
printInputUI_Int( ZnkStr html,
		const char* varname, int current_val, int new_val, bool is_enable,
		const char* destination )
{
	const char* class_name = ( st_input_ui_idx % 2 ) ? "odd" : "evn";
	const char* readonly_str = is_enable ? "" : "readonly disabled=true";
	ZnkStr_addf( html,
			"<tr class=\"%s\"><td>%s</td><td>%d</td><td><input type=text name=%s value=\"%d\" %s></td><td>%s</td></tr>\n",
			class_name,
			varname, current_val, varname, new_val, readonly_str, destination );
	++st_input_ui_idx;
}


static void
printInputUI_SelectBox( ZnkStr html,
		const char* varname, const char* current_val, const char* new_val, bool is_enable,
		ZnkStrAry str_list,
		const char* destination )
{
	const char* class_name = ( st_input_ui_idx % 2 ) ? "odd" : "evn";
	const char* readonly_str = is_enable ? "" : "disabled=true";
	const size_t size = ZnkStrAry_size( str_list );
	size_t idx;
	bool is_selected = false;
	const char* val = "";

	ZnkStr_addf( html,
			"<tr class=\"%s\"><td>%s</td><td>%s</td><td><select name=%s %s>\n",
			class_name,
			varname, current_val, varname, readonly_str );

	for( idx=0; idx<size; ++idx ){
		val = ZnkStrAry_at_cstr( str_list, idx );
		is_selected = ZnkS_eq( val, current_val );
		ZnkStr_addf( html, "<option%s>%s</option>", is_selected ? " selected" : "", val );
	}
	ZnkStr_addf( html,
			"</td><td>%s</td></tr>\n", destination );
	++st_input_ui_idx;
}

static int
printConfig( ZnkSocket sock, ZnkStrAry result_msgs, uint32_t peer_ipaddr )
{
	int ret;
	char proxy[ 1024 ] = "";
	const char* parent_proxy_hostname = NULL;
	bool sys_config_enable = true;
	ZnkStr html = ZnkStr_new( "" );
	ZnkStrAry str_list = ZnkStrAry_create( true );

	ZnkStr_add( html, "<html><body>\n" );
	ZnkStr_add( html, "<p><b><img src=\"moai.png\"> Moai : Web Configuration Version 1.0</b></p>\n" );

	ZnkStr_add( html, "<style type=\"text/css\">\n" );
	ZnkStr_add( html, "#config_filters tbody tr.evn td { background-color: #E0F0D6; }\n" );
	ZnkStr_add( html, "#config_filters tbody tr.odd td { background-color: #FFFFFF; }\n" );
	ZnkStr_add( html, "</style>\n" );

	ZnkStr_add( html, "<style type=\"text/css\">\n" );
	ZnkStr_add( html, "#config_functional tbody tr.evn td { background-color: #D6E0F0; }\n" );
	ZnkStr_add( html, "#config_functional tbody tr.odd td { background-color: #FFFFFF; }\n" );
	ZnkStr_add( html, "</style>\n" );

	ZnkStr_add( html, "<style type=\"text/css\">\n" );
	ZnkStr_add( html, "#config_system tbody tr.evn td { background-color: #F0E0D6; }\n" );
	ZnkStr_add( html, "#config_system tbody tr.odd td { background-color: #FFFFFF; }\n" );
	ZnkStr_add( html, "</style>\n" );


	/* ==== Filters and Plugins Initiation ==== */
	ZnkStr_add( html, "<b>Filters and Plugins</b><br>\n" );
	st_input_ui_idx = 0;
	ZnkStr_addf( html, "<form action=\"http://%s:%d/config\" method=\"POST\" enctype=\"multipart/form-data\">\n",
			st_server_name, st_moai_port );

	ZnkStr_add( html, "<input type=hidden name=Moai_Initiate value=\"initiate\">\n" );
	ZnkStr_add( html, "<input type=submit value=\"Virtual USERS Initiation\">\n" );
	ZnkStr_add( html, "</form>\n" );

	if( result_msgs ){
		size_t idx;
		const size_t size = ZnkStrAry_size( result_msgs );
		for( idx=0; idx<size; ++idx ){
			ZnkStr_addf( html, "%s<br>\n", ZnkStrAry_at_cstr( result_msgs, idx ) );
		}
	}

	ZnkStr_add( html, "<hr>\n" );

	/* ==== Instantly Updatable variables ==== */
	ZnkStr_add( html, "<b>Functional Configuration</b><br>\n" );
	st_input_ui_idx = 0;
	ZnkStr_addf( html, "<form action=\"http://%s:%d/config\" method=\"POST\" enctype=\"multipart/form-data\">\n",
			st_server_name, st_moai_port );
	parent_proxy_hostname = MoaiParentProxy_getHostname();
	if( ZnkS_empty(parent_proxy_hostname) ){
		ZnkS_copy( proxy, sizeof(proxy), "NONE", Znk_NPOS );
	} else {
		Znk_snprintf( proxy, sizeof(proxy), "%s:%d",
				parent_proxy_hostname, MoaiParentProxy_getPort() );
	}

	ZnkStr_add( html, "<table id=config_functional>\n" );
	ZnkStr_add( html, "<thead>\n" );
	ZnkStr_add( html, "<tr><th>variable name</th><th>current value</th><th>new value</th><th>description</th>\n" );
	ZnkStr_add( html, "</thead>\n" );
	ZnkStr_add( html, "<tbody>\n" );

	{
		ZnkStrAry parent_proxys = MoaiParentProxy_getParentProxys();
		printInputUI_SelectBox( html,
				"parent_proxy", proxy, proxy, true,
				parent_proxys,
				"Parent Proxy candidate selection in parent_proxy.txt." );
	}

	printInputUI_CheckBox( html,
			"post_confirm", MoaiPost_isPostConfirm(), MoaiPost_isPostConfirm(), true,
			"Show confirming message at HTTP posting." );

	printInputUI_CheckBox( html,
			"enable_log_file", st_enable_log_file, st_enable_log_file, true,
			"Writing to log file mode(on/off)." );

	printInputUI_CheckBox( html,
			"enable_log_verbose", st_enable_log_verbose, st_enable_log_verbose, true,
			"Verbose logging mode(on/off)." );

	printInputUI_CheckBox( html,
			"blocking_mode", MoaiConnection_isBlockingMode(), MoaiConnection_isBlockingMode(), true,
			"Connection Blocking Mode(on/off)." );

	ZnkStr_add( html, "</tbody>\n" );
	ZnkStr_add( html, "</table>\n" );

	ZnkStr_add( html, "<input type=hidden name=Moai_Update value=\"update\">\n" );
	ZnkStr_add( html, "<input type=submit value=\"Update\">\n" );
	ZnkStr_add( html, "</form>\n" );

	ZnkStr_add( html, "<hr>\n" );

	/* ==== System Configuration( �O���}�V������̍X�V�͕s�Ƃ��� ) ==== */
	if( peer_ipaddr == 0x0100007f || peer_ipaddr == st_private_ip ){
		/* Loopback�ڑ� */
		sys_config_enable = true;
	} else {
		/* ���̃}�V������̐ڑ� */
		sys_config_enable = false;
	}
	ZnkStr_add( html, "<font color=#ff0000><b>System Configuration( You can set this from localhost only )</b><br>\n" );
	st_input_ui_idx = 0;
	ZnkStr_addf( html, "<form action=\"http://127.0.0.1:%d/config\" method=\"POST\" enctype=\"multipart/form-data\">\n",
			st_moai_port );

	ZnkStr_add( html, "<table id=config_system>\n" );
	ZnkStr_add( html, "<thead>\n" );
	ZnkStr_add( html, "<tr><th>variable name</th><th>current value</th><th>new value</th><th>description</th>\n" );
	ZnkStr_add( html, "</thead>\n" );
	ZnkStr_add( html, "<tbody>\n" );

	{
		ZnkStrAry_clear( str_list );
		ZnkStrAry_push_bk_cstr( str_list, "LOOPBACK", Znk_NPOS );
		ZnkStrAry_push_bk_cstr( str_list, "ANY", Znk_NPOS );
		printInputUI_SelectBox( html,
				"acceptable_host", st_acceptable_host, st_acceptable_host, sys_config_enable,
				str_list,
				"Acceptable Host( ANY or LOOPBACK )." );
	}

	printInputUI_Text( html, "server_name", st_server_name, st_server_name, sys_config_enable,
			"SeverName(Use this config POST destination)." );

	printInputUI_Int( html, "moai_port", st_moai_port, st_moai_port, sys_config_enable,
			"The TCP port on which moai listens." );


	ZnkStr_add( html, "</tbody>\n" );
	ZnkStr_add( html, "</table>\n" );

	if( sys_config_enable ){
		ZnkStr_add( html, "<input type=hidden name=Moai_UpdateSys value=\"update_sys\">\n" );
		ZnkStr_add( html, "<input type=submit value=\"Update System\">\n" );
	}
	ZnkStr_add( html, "</form>\n" );


	ZnkStr_add( html, "<hr>\n" );

	/* ==== Restart ==== */
	if( st_you_need_to_restart_moai ){
		ZnkStr_add( html, "<font color=#ff0000><b>You need to restart Moai for adapting to system configuration.</b><br>\n" );
		if( !ZnkS_eq( st_acceptable_host_prev, "ANY" ) ){
			/***
			 * st_acceptable_host�͊��ɐV�����l�ɍX�V����Ă͂��邪�A
			 * �����_�ł܂����ۂ̏�Ԃ�LOOPBACK�ł���Ǝv����. ����Ă܂�post���127.0.0.1�ɂ��Ă����Ȃ���΂Ȃ�Ȃ�.
			 */
			ZnkStr_addf( html, "<form action=\"http://127.0.0.1:%d/config\" method=\"POST\" enctype=\"multipart/form-data\">\n",
					st_moai_port );
		} else {
			ZnkStr_addf( html, "<form action=\"http://%s:%d/config\" method=\"POST\" enctype=\"multipart/form-data\">\n",
					st_server_name, st_moai_port );
		}
	} else {
		if( ZnkS_eq( st_acceptable_host, "ANY" ) ){
			ZnkStr_addf( html, "<form action=\"http://%s:%d/config\" method=\"POST\" enctype=\"multipart/form-data\">\n",
					st_server_name, st_moai_port );
		} else {
			/***
			 * LOOPBACK�̏ꍇ�́Apost���127.0.0.1�ɂ��Ă����Ȃ���΂Ȃ�Ȃ�.
			 * �܂����̏ꍇ�͊O���}�V�������Restart�͔F�߂��Ȃ�.
			 */
			ZnkStr_addf( html, "<form action=\"http://127.0.0.1:%d/config\" method=\"POST\" enctype=\"multipart/form-data\">\n",
					st_moai_port );
		}
	}
	ZnkStr_add( html, "<input type=hidden name=Moai_RestartServer value=\"reboot\">\n" );
	ZnkStr_add( html, "<input type=submit value=\"Restart Moai\">\n" );
	ZnkStr_add( html, "</form>\n" );

	ZnkStr_addf( html, "<a href=\"http://%s:%d\">Back to Top</a>\n", st_server_name, st_moai_port );
	ZnkStr_add( html, "</body></html>\n" );

	ret = MoaiIO_sendTxtf( sock, "text/html", ZnkStr_cstr(html) );
	ZnkStr_delete( html );
	ZnkStrAry_destroy( str_list );
	return ret;
}

static void
initiateFilters( MoaiModuleAry mod_ary, ZnkStrAry result_msgs )
{
	const size_t mod_size = MoaiModuleAry_size( mod_ary );
	size_t       mod_idx;
	MoaiModule   mod;
	const char*  parent_proxy_hostname = MoaiParentProxy_getHostname();
	uint16_t     port                  = MoaiParentProxy_getPort();
	char         parent_proxy[ 4096 ]  = "NONE";
	ZnkStr       result_msg            = ZnkStr_new( "" );
	const char*  target_name           = NULL;
	bool         result                = false;

	if( !ZnkS_empty( parent_proxy_hostname ) && !ZnkS_eq( parent_proxy_hostname, "NONE" ) ){
		Znk_snprintf( parent_proxy, sizeof(parent_proxy), "%s:%hd", parent_proxy_hostname, port );
	}
	for( mod_idx=0; mod_idx<mod_size; ++mod_idx ){
		mod = MoaiModuleAry_at( mod_ary, mod_idx );
		if( mod ){
			MoaiLog_printf( "Moai : invokeInitiate [%s]\n", parent_proxy );
			target_name = MoaiModule_targe_name( mod );
			ZnkStr_set( result_msg, "Cannot call plugin function" );
			result = MoaiModule_invokeInitiate( mod, parent_proxy, result_msg );

			if( result_msgs ){
				ZnkStrAry_push_bk_snprintf( result_msgs, Znk_NPOS,
						"Initiate %s : %s : %s",
						target_name, result ? "Success" : "Failure", ZnkStr_cstr(result_msg) );
			}
			MoaiLog_printf( "Moai : Initiate %s : %s : %s\n",
					target_name, result ? "Success" : "Failure", ZnkStr_cstr(result_msg) );
		}
	}
}

static void
doConfigUpdate( const MoaiInfo* info, bool* is_proxy_updated )
{
	ZnkVarp var;

	var = ZnkVarpAry_find_byName_literal( info->vars_, "parent_proxy", false );
	if( var ){
		if( MoaiParentProxy_set_byAuthority( ZnkVar_cstr(var) ) ){
			*is_proxy_updated = true;
		}
	}
	var = ZnkVarpAry_find_byName_literal( info->vars_, "post_confirm", false );
	MoaiPost_setPostConfirm( var ? true : false );

	var = ZnkVarpAry_find_byName_literal( info->vars_, "enable_log_file", false );
	st_enable_log_file = var ? true : false;

	var = ZnkVarpAry_find_byName_literal( info->vars_, "enable_log_verbose", false );
	st_enable_log_verbose = var ? true : false;

	var = ZnkVarpAry_find_byName_literal( info->vars_, "blocking_mode", false );
	MoaiConnection_setBlockingMode( var ? true : false );

	{
		ZnkMyf config = MoaiMyf_theConfig();
		ZnkVarpAry vars = ZnkMyf_find_vars( config, "config" );
		if( vars ){
			ZnkVarp var = NULL;

			var = ZnkVarpAry_find_byName_literal( vars, "parent_proxy", false );
			if( var ){
				const char*    hostname = MoaiParentProxy_getHostname();
				const uint16_t port     = MoaiParentProxy_getPort();
				char data[ 512 ];
				Znk_snprintf( data, sizeof(data), "%s:%d", hostname, port );
				ZnkVar_set_val_Str( var, data, Znk_strlen(data) );
			}

			var = ZnkVarpAry_find_byName_literal( vars, "post_confirm", false );
			if( var ){
				const char* data = MoaiPost_isPostConfirm() ? "on" : "off";
				ZnkVar_set_val_Str( var, data, Znk_strlen(data) );
			}

			var = ZnkVarpAry_find_byName_literal( vars, "enable_log_file", false );
			if( var ){
				const char* data = st_enable_log_file ? "on" : "off";
				ZnkVar_set_val_Str( var, data, Znk_strlen(data) );
			}

			var = ZnkVarpAry_find_byName_literal( vars, "enable_log_verbose", false );
			if( var ){
				const char* data = st_enable_log_verbose ? "on" : "off";
				ZnkVar_set_val_Str( var, data, Znk_strlen(data) );
			}

			var = ZnkVarpAry_find_byName_literal( vars, "blocking_mode", false );
			if( var ){
				const char* data = MoaiConnection_isBlockingMode() ? "on" : "off";
				ZnkVar_set_val_Str( var, data, Znk_strlen(data) );
			}
			ZnkMyf_save( config, "config.myf" );
		}
	}
}
static void
doConfigUpdateSys( const MoaiInfo* info )
{
	ZnkVarp var;

	var = ZnkVarpAry_find_byName_literal( info->vars_, "acceptable_host", false );
	if( var ){
		ZnkS_copy( st_acceptable_host_prev, sizeof(st_acceptable_host_prev), st_acceptable_host, Znk_NPOS );
		ZnkS_copy( st_acceptable_host, sizeof(st_acceptable_host), ZnkVar_cstr(var), Znk_NPOS );
	}

	var = ZnkVarpAry_find_byName_literal( info->vars_, "server_name", false );
	if( var ){
		ZnkS_copy( st_server_name, sizeof(st_server_name), ZnkVar_cstr(var), Znk_NPOS );
	}
	autosetServerName();

	var = ZnkVarpAry_find_byName_literal( info->vars_, "moai_port", false );
	if( var ){
		ZnkS_getU16U( &st_moai_port, ZnkVar_cstr(var) );
	}
	{
		ZnkMyf config = MoaiMyf_theConfig();
		ZnkVarpAry vars = ZnkMyf_find_vars( config, "config" );
		if( vars ){
			ZnkVarp var = NULL;

			var = ZnkVarpAry_find_byName_literal( vars, "acceptable_host", false );
			if( var ){
				const char* data = st_acceptable_host;
				ZnkVar_set_val_Str( var, data, Znk_strlen(data) );
			}

			var = ZnkVarpAry_find_byName_literal( vars, "moai_port", false );
			if( var ){
				char data[ 256 ] = "0";
				Znk_snprintf( data, sizeof(data), "%hd", st_moai_port );
				ZnkVar_set_val_Str( var, data, Znk_strlen(data) );
			}

			var = ZnkVarpAry_find_byName_literal( vars, "server_name", false );
			if( var ){
				const char* data = st_server_name;
				ZnkVar_set_val_Str( var, data, Znk_strlen(data) );
			}

			ZnkMyf_save( config, "config.myf" );
		}
	}
}

static MoaiRASResult
doWebServer( const MoaiContext ctx, ZnkSocket sock, MoaiConnection mcn, MoaiFdSet mfds )
{
	MoaiRASResult ras_result = MoaiRASResult_e_OK;
	int ret = 0;
	ZnkFdSet fdst_observe_r = MoaiFdSet_fdst_observe_r( mfds );

	/* WebServer�Ƃ��ď��� */
	switch( ctx->req_method_ ){
	case ZnkHtpReqMethod_e_GET:
	{
		MoaiInfo* info = ctx->draft_info_;
		ZnkStr str1 = info->req_urp_;
		ZnkStr str2 = ZnkStr_new( "./doc_root" );
		MoaiLog_printf( "  As WebServer GET\n" );
		/***
		 * �����͖{���Ȃ� str1���� .. �𖳌��ɂ���Ȃǂ̃T�j�^�C�Y���s��Ȃ���΂Ȃ�Ȃ�.
		 */
		ZnkStr_add( str2, ZnkStr_cstr(str1) );
		if( ZnkStr_last(str2) == '/' ){
			ZnkStr_add( str2, "index.html" );
		}
		MoaiLog_printf( "  Requested file is [%s]\n", ZnkStr_cstr(str2) );
		if( ZnkStr_isEnd(str2, "/config" ) ){
			ret = printConfig( sock, NULL, ctx->peer_ipaddr_ );
		} else if( !MoaiIO_sendResponseFile( sock, ZnkStr_cstr(str2) ) ){
			ret = MoaiIO_sendTxtf( sock, "text/html", "<p><b><img src=\"moai.png\"> Moai WebServer : 404 Not found [%s].</b></p>\n",
					ZnkStr_cstr(str2) );
		}
		MoaiLog_printf( "  Requested is done.\n" );
		if( ret < 0 || !mcn->I_is_keep_alive_ ){
			MoaiIO_close_ISock( "  WebServerGET", sock, mfds );
		}
		return MoaiRASResult_e_OK;
	}
	case ZnkHtpReqMethod_e_POST:
	{
		MoaiInfo* info = ctx->draft_info_;
		ZnkStr str1 = info->req_urp_;
		ZnkStr str2 = ZnkStr_new( "" );
		bool is_proxy_updated = false;
		ZnkStrAry result_msgs = ZnkStrAry_create( true );
		const char* hostname = MoaiConnection_hostname( mcn );
		const MoaiModule mod = MoaiModuleAry_find_byHostname( st_mod_ary, hostname );

		ZnkStr_addf( str2, "<p><b><img src=\"moai.png\"> Moai WebServer : POST [%s].</b></p>\n",
				ZnkStr_cstr(str1) );
		ZnkStr_add( str2, "<pre>\n" );

		MoaiPost_parsePostVars3( sock, mfds,
				str2,
				info->hdr_size_, info->hdrs_.hdr1st_, info->hdrs_.vars_,
				ctx->body_info_.content_length_, info->stream_,
				info->vars_,
				mod );

		ZnkStr_add( str2, "</pre>\n" );

		if( info->vars_ ){
			ZnkVarp var = NULL;

			var = ZnkVarpAry_find_byName_literal( info->vars_, "Moai_Update", false );
			if( var ){
				doConfigUpdate( info, &is_proxy_updated );
				ras_result = MoaiRASResult_e_OK;
				MoaiLog_printf( "Moai_Update[%s] done.\n", ZnkVar_cstr(var) );
			}

			var = ZnkVarpAry_find_byName_literal( info->vars_, "Moai_Initiate", false );
			if( var ){
				initiateFilters( st_mod_ary, result_msgs );
				ras_result = MoaiRASResult_e_OK;
			}

			var = ZnkVarpAry_find_byName_literal( info->vars_, "Moai_RestartServer", false );
			if( var ){
				st_you_need_to_restart_moai = false;
				ras_result = MoaiRASResult_e_RestartServer;
			}

			var = ZnkVarpAry_find_byName_literal( info->vars_, "Moai_UpdateSys", false );
			if( var ){
				doConfigUpdateSys( info );
				ras_result = MoaiRASResult_e_OK;
				st_you_need_to_restart_moai = true;
				MoaiLog_printf( "Moai_UpdateSys[%s] done.\n", ZnkVar_cstr(var) );
			}

		}

		if( !st_enable_log_file ){
			MoaiLog_printf( "Moai : config : enable_log_file is off.\n" );
			MoaiLog_printf( "Moai : Stop writing to log file and close it here.\n" );
			MoaiLog_close();
		}


		if( ras_result == MoaiRASResult_e_RestartServer ){
			ZnkStr_clear( str2 );
			ZnkStr_add( str2, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">" );
			ZnkStr_add( str2, "<html><head><META HTTP-EQUIV=\"refresh\" content=\"1;URL=config\"></head>" );
			ZnkStr_add( str2, "<body>Moai Server is Restarted.<br>\n" );
			ZnkStr_add( str2, "<a href=\"./config\">Back to config</a></body></html>\n" );
			ret = MoaiIO_sendTxtf( sock, "text/html", ZnkStr_cstr( str2 ) );
		} else {
			if( ZnkStr_isEnd( str1, "/config" ) ){
				ret = printConfig( sock, result_msgs, ctx->peer_ipaddr_ );
			} else {
				ret = MoaiIO_sendTxtf( sock, "text/html", ZnkStr_cstr( str2 ) );
			}
		}
		ZnkStr_delete( str2 );

		/***
		 * MoaiSever��Restart����ꍇ���A��U�R�l�N�g���N���[�X���������悢.
		 * �����Ȃ��ƃu���E�U���Łu�ڑ������Z�b�g����܂����v���\������邱�Ƃ�����s���D�ł���.
		 */
		if( ret < 0 || !mcn->I_is_keep_alive_ || ras_result == MoaiRASResult_e_RestartServer ){
			MoaiIO_close_ISock( "  WebServerPOST", sock, mfds );
		}
		/***
		 * ����Observe����Ă���Socket�����ׂċ����I��close����.
		 */
		if( is_proxy_updated ){
			MoaiIO_closeSocketAll( "  ProxyUpdate", fdst_observe_r, mfds );
		}
		ZnkStrAry_destroy( result_msgs );
	}
	default:
		break;
	}
	return ras_result;
}

static MoaiRASResult
recvAndSendCore( MoaiContext ctx, ZnkSocket sock, MoaiSockType sock_type, MoaiConnection mcn, MoaiFdSet mfds )
{
	ZnkFdSet fdst_observe_r = MoaiFdSet_fdst_observe_r( mfds );
	MoaiRASResult ras_result = MoaiRASResult_e_OK;

	HtpScanType scan_type = HtpScan_e_None;
	MoaiHtpType htp_type = MoaiHtpType_e_None;

	scan_type = scanHttpFirst( ctx, mcn, sock, sock_type, fdst_observe_r, mfds );
	switch( scan_type ){
	case HtpScan_e_Request:
		htp_type = MoaiHtpType_e_Request;
		break;
	case HtpScan_e_Response:
		htp_type = MoaiHtpType_e_Response;
		break;
	case HtpScan_e_RecvZero:
	case HtpScan_e_RecvError:
		return MoaiRASResult_e_Ignored;
	case HtpScan_e_NotHttpStart:
		htp_type = MoaiHtpType_e_NotHttpStart;
		break;
	case HtpScan_e_UnknownMethod:
	case HtpScan_e_CannotGetHost:
	case HtpScan_e_HostBloking:
		return MoaiRASResult_e_Ignored;
	default:
		return MoaiRASResult_e_Ignored;
	}

	if( ctx->as_local_proxy_ ){
		/* LocalProxy�Ƃ��ď��� */
		ras_result = doLocalProxy( ctx, mcn, mfds, htp_type, sock_type );
	} else {
		/* WebServer�Ƃ��ď��� */
		ras_result = doWebServer( ctx, sock, mcn, mfds );
	}
	return ras_result;
}

static void report_observe( MoaiFdSet mfds, void* arg )
{
	ZnkSocketAry wk_sock_ary    = MoaiFdSet_wk_sock_ary( mfds );
	ZnkFdSet     fdst_observe_r = MoaiFdSet_fdst_observe_r( mfds );
	ZnkFdSet     fdst_observe_w = MoaiFdSet_fdst_observe_w( mfds );
	if( st_enable_log_verbose ){
		MoaiIO_reportFdst( "Moai : ObserveR ", wk_sock_ary, fdst_observe_r, true );
	} else {
		size_t sock_ary_size;
		ZnkSocketAry_clear( wk_sock_ary );
		ZnkFdSet_getSocketAry( fdst_observe_r, wk_sock_ary );
		sock_ary_size = ZnkSocketAry_size( wk_sock_ary );
		MoaiLog_printf( "Moai : ObserveR : %u-th sock events by select.\n", sock_ary_size );
	}
	MoaiIO_reportFdst( "Moai : ObserveW ", wk_sock_ary, fdst_observe_w, true );
}
static void on_accept( MoaiFdSet mfds, ZnkSocket new_accept_sock, void* arg )
{
	MoaiLog_printf( "Moai : new_accept_sock=[%d]\n", new_accept_sock );
	MoaiConnection_regist( "", 0, new_accept_sock, ZnkSocket_INVALID, mfds );
}
static MoaiRASResult recv_and_send( MoaiFdSet mfds, ZnkSocket sock, void* arg )
{
	MoaiConnection mcn = NULL;
	MoaiContext    ctx = (MoaiContext)arg;
	MoaiSockType   sock_type = MoaiSockType_e_None;

	mcn = MoaiConnection_find_byISock( sock );
	if( mcn ){
		sock_type = MoaiSockType_e_Inner;
		return recvAndSendCore( ctx, sock, sock_type, mcn, mfds );
	}

	mcn = MoaiConnection_find_byOSock( sock );
	if( mcn ){
		/***
		 * ����͂��͂�K�v�Ȃ��̂ł�...?
		 */
#if 0
		if( MoaiConnection_isConnectInprogress( mcn ) ){
			MoaiLog_printf( "  recv_and_send : sock=[%d] is ConnectInprogress.\n", sock );
			return MoaiRASResult_e_Ignored;
		}
#endif

		sock_type = MoaiSockType_e_Outer;
		return recvAndSendCore( ctx, sock, sock_type, mcn, mfds );
	}

	{
		ZnkFdSet fdst_observe_r = MoaiFdSet_fdst_observe_r( mfds );
		MoaiLog_printf( "  NotFoundConnection : close sock=[%d].\n", sock );
		ZnkSocket_close( sock );
		ZnkFdSet_clr( fdst_observe_r, sock );
	}
	return MoaiRASResult_e_Ignored;
}

static bool
isAccessAllowIP( ZnkSocket new_accept_sock, uint32_t* ipaddr_ans )
{
	uint32_t ipaddr = 0;
	bool result = false;
	char ipstr[ 64 ] = "";
	
	ZnkSocket_getPeerIPandPort( new_accept_sock, &ipaddr, NULL );
	ZnkNetIP_getIPStr_fromU32( ipaddr, ipstr, sizeof(ipstr) );

	MoaiLog_printf( "Moai : peer ip=[%s]\n", ipstr );

	if( ipaddr == 0x0100007f ){
		/* loopback�ڑ� : ���̏ꍇ�͖������ɋ��� */
		result = true;
	} else {
		bool is_allow = (bool)( ZnkStrAry_find_isMatch( st_access_allow_ips,
					0, ipstr, Znk_NPOS, ZnkS_isMatchSWC ) != Znk_NPOS );
		bool is_deny = (bool)( ZnkStrAry_find_isMatch( st_access_deny_ips,
					0, ipstr, Znk_NPOS, ZnkS_isMatchSWC ) != Znk_NPOS );
		result = is_allow && !is_deny;
#if 0
		if( (ipaddr & 0xffff) == 0x0000a8c0 ){
			/* 192.168.* ����̐ڑ� : ���̏ꍇ�͋��� */
			result = true;
		}
#endif
	}
	if( ipaddr_ans ){
		*ipaddr_ans = ipaddr;
	}
	return result;
}

MoaiRASResult
MoaiServer_main( bool first_initiate, bool enable_parent_proxy )
{
	static bool st_first_initiated = false;
	int sel_ret;
	MoaiFdSet mfds = NULL;

	const char* acceptable_host = "127.0.0.1";
	ZnkServer     srver       = NULL;
	ZnkSocket     listen_sock = ZnkSocket_INVALID;
	MoaiFdSetFuncArg_Report   fnca_report    = { report_observe, NULL };
	MoaiFdSetFuncArg_OnAccept fnca_on_accept = { on_accept, NULL };
	MoaiFdSetFuncArg_RAS      fnca_ras       = { recv_and_send, NULL };
	struct ZnkTimeval waitval;
	bool req_fdst_report = true;
	MoaiContext ctx = MoaiContext_create();
	MoaiRASResult ras_result = MoaiRASResult_e_Ignored;
	ZnkMyf config;
	ZnkMyf target_myf;

	MoaiLog_printf( "\n" );
	MoaiLog_printf( "Moai : MoaiServer_main start.\n" );

	fnca_ras.arg_ = ctx;

	MoaiLog_open( "moai_log.log", false );

	if( !MoaiMyf_loadConfig() ){
		/***
		 * ���̏ꍇ�͒v���I�G���[�Ƃ��ׂ�.
		 */
		MoaiLog_printf( "Moai : Cannot load config.myf.\n" );
		getchar();
		goto FUNC_END;
	}
	MoaiMyf_loadTarget();
	MoaiMyf_loadAnalysis();

	config     = MoaiMyf_theConfig();
	target_myf = MoaiMyf_theTarget();

	Znk_DELETE_PTR( st_mod_ary, MoaiModuleAry_destroy, NULL );
	st_mod_ary = MoaiModuleAry_create( true );
	MoaiModuleAry_loadAllModules( st_mod_ary, target_myf );

	/***
	 * parse config
	 */
	{
		ZnkVarpAry vars = ZnkMyf_find_vars( config, "config" );
		if( vars ){
			ZnkVarp var = NULL;

			if( enable_parent_proxy ){
				var = ZnkVarpAry_find_byName_literal( vars, "parent_proxy", false );
				if( var ){
					MoaiParentProxy_set_byAuthority( ZnkVar_cstr(var) );
				}
			} else {
				MoaiParentProxy_set_byAuthority( "NONE" );
			}

			var = ZnkVarpAry_find_byName_literal( vars, "post_confirm", false );
			if( var ){
				MoaiPost_setPostConfirm( ZnkS_eq( ZnkVar_cstr(var), "on" ) );
			}

			var = ZnkVarpAry_find_byName_literal( vars, "moai_port", false );
			if( var ){
				sscanf( ZnkVar_cstr( var ), "%hu", &st_moai_port );
			}

			var = ZnkVarpAry_find_byName_literal( vars, "proxy_indicating_mode", false );
			if( var ){
				if( ZnkS_eq( ZnkVar_cstr(var), "minus" ) ){
					MoaiParentProxy_setIndicatingMode( -1 );
				} else if( ZnkS_eq( ZnkVar_cstr(var), "plus" ) ){
					MoaiParentProxy_setIndicatingMode( 1 );
				} else {
					MoaiParentProxy_setIndicatingMode( 0 );
				}
			}

			var = ZnkVarpAry_find_byName_literal( vars, "enable_log_file", false );
			if( var ){
				st_enable_log_file = ZnkS_eq( ZnkVar_cstr(var), "on" );
			}

			var = ZnkVarpAry_find_byName_literal( vars, "enable_log_verbose", false );
			if( var ){
				st_enable_log_verbose = ZnkS_eq( ZnkVar_cstr(var), "on" );
			}

			var = ZnkVarpAry_find_byName_literal( vars, "blocking_mode", false );
			if( var ){
				MoaiConnection_setBlockingMode( ZnkS_eq( ZnkVar_cstr(var), "on" ) );
			}

			var = ZnkVarpAry_find_byName_literal( vars, "acceptable_host", false );
			if( var ){
				const char* var_str = ZnkVar_cstr(var);
				ZnkS_copy( st_acceptable_host, sizeof(st_acceptable_host), var_str, Znk_NPOS );
				if( ZnkS_eq( var_str, "ANY" ) ){
					/***
					 * localhost�ȊO����̐ڑ������ׂĎ󂯕t����.
					 * �����Moai�ł͂�����IP�A�h���X��h���C���ɉ�����Forbidden�@�\�Ȃǂ�
					 * ��������Ă��Ȃ��̂ŁA������w�肵���ꍇ��NIC(���[�^�Ȃ�)��OS�̃t�@�C�A
					 * �E�H�[���@�\�ȂǂŐڑ��\��IP��LAN����̂��݂̂̂ɂ���ȂǓK�؂ɐ������邱��.
					 */
					acceptable_host = NULL;
				} else if( ZnkS_eq( var_str, "LOOPBACK" ) || ZnkS_empty( var_str ) ){
					/***
					 * ���̏ꍇ�Alocalhost����̐ڑ�(loopback)�݂̂��󂯕t����.
					 * �����Ƀ��[�J���v���L�V�Ƃ��Ă݂̂̎g�p�Ȃ�΂��̎w�肪��Ԉ��S�ł���.
					 */
					acceptable_host = "127.0.0.1";
				} else {
					/***
					 * ���̏ꍇ�Aacceptable_host�ɂ����Ďw�肵��IP����̐ڑ��݂̂��󂯕t����.
					 * �����p�ł���.
					 */
					acceptable_host = var_str;
				}
			}
			var = ZnkVarpAry_find_byName_literal( vars, "server_name", false );
			if( var ){
				ZnkS_copy( st_server_name, sizeof(st_server_name), ZnkVar_cstr(var), Znk_NPOS );
			}
		}
	}

	MoaiParentProxy_loadCandidate( "parent_proxy.txt" );

	st_access_allow_ips = ZnkMyf_find_lines( config, "access_allow_ips" );
	st_access_deny_ips  = ZnkMyf_find_lines( config, "access_deny_ips" );

	MoaiLog_printf( "Moai : acceptable_host=[%s]\n", st_acceptable_host );
	MoaiLog_printf( "Moai : blocking_mode=[%d]\n", MoaiConnection_isBlockingMode() );

	if( !st_enable_log_file ){
		MoaiLog_printf( "Moai : config : enable_log_file is off.\n" );
		MoaiLog_printf( "Moai : Stop writing to log file and close it here.\n" );
		MoaiLog_close();
	}

	/***
	 * plugin�ɂ����鏉�����֐����Ăяo��.
	 */
	if( first_initiate && !st_first_initiated )
	{
		initiateFilters( st_mod_ary, NULL );
		st_first_initiated = true;
	}

	waitval.tv_sec  = 2;        /* �҂����Ԃ� 2.500 �b���w�� */
	waitval.tv_usec = 500;

	ZnkNetBase_initiate( false );

	//ZnkNetIP_printTest();
	autosetServerName();

	MoaiConnection_initiate();
	MoaiConnection_clearAll( mfds );
	MoaiInfo_initiate();
	                              
	srver = ZnkServer_create( acceptable_host, st_moai_port );
	if( srver == NULL ){
		MoaiLog_printf( "Moai : Cannot create server. May be port %d is already used.\n", st_moai_port );
		goto FUNC_END;
	}
	listen_sock = ZnkServer_getListenSocket( srver );
	MoaiConnection_setListeningSock( listen_sock );
	mfds = MoaiFdSet_create( listen_sock, &waitval );

	MoaiLog_printf( "Moai : Listen port %d...\n", st_moai_port );
	MoaiLog_printf( "\n" );
	
	while( true ){
		sel_ret = MoaiFdSet_select( mfds, &req_fdst_report, &fnca_report );

		/***
		 * fdst_read
		 */
		if( sel_ret < 0 ){
			/* select error */
			uint32_t sys_errno = ZnkSysErrno_errno();
			ZnkSysErrnoInfo* errinfo = ZnkSysErrno_getInfo( sys_errno );
			MoaiLog_printf( "Moai : MoaiFdSet_select Error : sel_ret=[%d] : [%s]\n", sel_ret, errinfo->sys_errno_msg_ );
			MoaiFdSet_printf_fdst_read( mfds );
			ras_result = MoaiRASResult_e_RestartServer;
			goto FUNC_END;
		}
		if( sel_ret == 0 ){
			static size_t count = 0;
			/* select timeout */
			ZnkSocketAry wk_sock_ary = MoaiFdSet_wk_sock_ary( mfds );
			ZnkFdSet      fdst_observe_r = MoaiFdSet_fdst_observe_r( mfds );
			if( count % 10 == 0 ){
				//MoaiLog_printf( "Moai : MoaiFdSet_select timeout : count=[%u]\n", count );
			}
			if( MoaiIO_procExile( wk_sock_ary, fdst_observe_r, mfds ) ){
				req_fdst_report = true;
			}
			MoaiFdSet_procConnectionTimeout( mfds );
			++count;
			continue;
		}
		                            
		ras_result = MoaiFdSet_process( mfds,
				&fnca_on_accept, &fnca_ras, isAccessAllowIP );
		switch( ras_result ){
		case MoaiRASResult_e_CriticalError:
		case MoaiRASResult_e_RestartServer:
			goto FUNC_END;
		default:
			break;
		}

		/***
		 * ����A��ɕ\������悤�ɂ��Ă��邪�A
		 * ���炩�ȕύX�����������ꍇ�̂ݕ\������悤�ɂ��������X�}�[�g�ł���.
		 */
		req_fdst_report = true;

	}
	ras_result = MoaiRASResult_e_OK;

FUNC_END:
	MoaiFdSet_destroy( mfds );
	ZnkSocket_close( listen_sock );
	ZnkNetBase_finalize();
	MoaiContext_destroy( ctx );
	Znk_DELETE_PTR( st_mod_ary, MoaiModuleAry_destroy, NULL );
	MoaiLog_close();

	return ras_result;
}
