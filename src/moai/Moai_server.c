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

#include <Znk_server.h>
#include <Znk_socket_dary.h>
#include <Znk_fdset.h>
#include <Znk_err.h>
#include <Znk_net_base.h>
#include <Znk_socket.h>
#include <Znk_sys_errno.h>
#include <Znk_s_base.h>
#include <Znk_missing_libc.h>
#include <Znk_str_dary.h>
#include <Znk_str_fio.h>
#include <Znk_zlib.h>
#include <Znk_def_util.h>
#include <Znk_htp_hdrs.h>
#include <Znk_dlink.h>
#include <Znk_cookie.h>
#include <Znk_txt_filter.h>

#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

static bool        st_master_confirm = true;
static uint16_t    st_moai_port = 8123;
static bool        st_enable_log_file = true;


Znk_INLINE double
getCurrentSec( void ){ return ( 1.0 / CLOCKS_PER_SEC ) * clock(); }


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

typedef enum {
	 HtpType_e_None=0
	,HtpType_e_Request
	,HtpType_e_Response
	,HtpType_e_NotHttpStart
} HtpType;


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
processResponse( ZnkSocket O_sock, MoaiContext ctx, ZnkFdSet fdst_observe,
		size_t* content_length_remain )
{
	bool is_direct_download = false;
	MoaiInfo*     info      = &ctx->hi_;
	MoaiBodyInfo* body_info = &ctx->body_info_;

	/***
	 * HTML�̏ꍇ�͑S�擾�����݂�.
	 */
	if( body_info->txt_type_ != MoaiText_e_Binary || body_info->is_unlimited_ ){
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
					MoaiIO_recvByPtn( ctx->hi_.stream_, O_sock, fdst_observe, "\r\n", &result_size );
					MoaiLog_printf( "MoaiIO_recvByPtn : result_size=[%u]\n", result_size );
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
					MoaiLog_printf( "chunk_size=[%u]\n", chunk_size );
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
					if( !MoaiIO_recvBySize( ctx->hi_.stream_, O_sock, fdst_observe, chunk_remain, &result_size ) ){
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
			MoaiLog_printf( "MoaiIO_recvByZero Mode Begin\n" );
			MoaiIO_recvByZero( ctx->hi_.stream_, O_sock, fdst_observe, &result_size );
			MoaiLog_printf( "MoaiIO_recvByZero Mode End\n" );
		} else {
			while( *content_length_remain ){
				MoaiIO_recvBySize( ctx->hi_.stream_, O_sock, fdst_observe, *content_length_remain, &result_size );
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

			{
				MoaiConnection htcn = MoaiConnection_find( O_sock );
				const char* hostname = htcn->hostname_;
				MoaiModule mod = MoaiModuleAry_find_byHostname( ctx->mod_ary_, ctx->target_myf_, hostname );
				if( mod ){
					ZnkTxtFilterAry txt_ftr = NULL;
					switch( body_info->txt_type_ ){
					case MoaiText_e_HTML:
						txt_ftr = MoaiModule_ftrHtml( mod );
						break;
					case MoaiText_e_JS:
						txt_ftr = MoaiModule_ftrJS( mod );
						break;
					case MoaiText_e_CSS:
						txt_ftr = MoaiModule_ftrCSS( mod );
						break;
					default:
						break;
					}
					if( txt_ftr ){
						ZnkTxtFilterAry_exec( txt_ftr, ctx->text_ );
					}
				}
				htcn->waiting_ = 0;
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
			case MoaiText_e_HTML:
				fp = ZnkF_fopen( "result.html", "wb" );
				break;
			case MoaiText_e_JS:
				fp = ZnkF_fopen( "result.js", "wb" );
				break;
			case MoaiText_e_CSS:
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
			ZnkHtpHdrs hdrs = &ctx->hi_.hdrs_;
			const char* key;
			bool exist_content_length = false;
			size_t size;
			size_t idx;
			ZnkVarp varp;
	
			ZnkHtpHdrs_erase( hdrs->vars_, "Transfer-Encoding" );

			size = ZnkVarpDAry_size( hdrs->vars_ );
			for( idx=0; idx<size; ++idx ){
				varp = ZnkVarpDAry_at( hdrs->vars_, idx );
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
}
static void
sendHdrs( ZnkSocket sock, ZnkStrDAry hdr1st, const ZnkVarpDAry vars )
{
	ZnkBfr bfr = ZnkBfr_create_null();
	ZnkHtpHdrs_extendToStream( hdr1st, vars, bfr, true );
	ZnkSocket_send( sock, ZnkBfr_data(bfr), ZnkBfr_size(bfr) );
}

static void
debugHdrVars( const ZnkVarpDAry vars, const char* prefix_tab )
{
	const size_t size = ZnkVarpDAry_size( vars );
	size_t  idx;
	size_t  val_idx;
	size_t  val_size;
	ZnkVarp varp = NULL;
	for( idx=0; idx<size; ++idx ){
		varp = ZnkVarpDAry_at( vars, idx );
		val_size = ZnkHtpHdrs_val_size( varp );
		MoaiLog_printf( "%s", prefix_tab );
		MoaiLog_printf( "[%s]: ", ZnkHtpHdrs_key_cstr(varp) );
		for( val_idx=0; val_idx<val_size; ++val_idx ){
			MoaiLog_printf( "[%s]",  ZnkHtpHdrs_val_cstr(varp,val_idx) );
		}
		MoaiLog_printf( "\n" );
	}
}



/***
 * I�������request���AO���֑���AO�������response���AI���֑���Ƃ���
 * request/response�̊�{�P�ʂ������ł�RAR(Request And Response)�ƌď̂��邱�Ƃɂ���.
 *
 * ���RAR�̐���Ƃ��Ă͈ȉ��ƂȂ�.
 * socket I �� O ���������Ƃ��āA�܂� I ���� O �ւ�forward ���A
 * ���ꂪ�I������ O ���� I �ւ�forward���s��.
 *
 * select I�̓ǂݍ��݉����o.
 * forward : recv(from I) => send(to O)
 * select I�̓ǂݍ��݉����o.
 * forward : recv(from I) => send(to O)
 * select I�̓ǂݍ��݉����o.
 * forward : recv(from I) => send(to O)
 *
 * ������c�����recv���I��. �����������炭c�͂܂��ڑ����ێ���close�����Ȃ�.
 * �܂�recv(from I)��0���ۂ��ł��̃^�C�~���O�𔻒肷�邱�Ƃ͂ł��Ȃ����A
 * ����Ȃ��Ƃ�����΁Arecv(from I)���琧�䂪�i���ɋA���Ă��Ȃ��Ȃ�.
 * ���̂��߁A�����ł� select �֖߂��Ă��̔�����s��˂΂Ȃ�Ȃ�.
 *
 * select O�̓ǂݍ��݉����o.
 * forward : recv(from O) => send(to I)
 * select O�̓ǂݍ��݉����o.
 * forward : recv(from O) => send(to I)
 * select O�̓ǂݍ��݉����o.
 * forward : recv(from O) => send(to I)
 *
 * recv(from O) �� 0 ��Ԃ����Ƃ� O����close�������Ƃ����o.
 * ���邢��recv(from I) �� 0 ��Ԃ����Ƃ� I����close�������Ƃ����o.
 * ���RAR���I���ƂȂ�.
 */
static HtpScanType
scanHttpFirst( MoaiContext ctx, MoaiConnection htcn, ZnkSocket sock, ZnkFdSet fdst_observe )
{
	/***
	 * �����ł͎������o���ӂ܂��������forward���s��.
	 * ���̏ꍇ�ɂ����������o���s���Ȃ��ꂽ�ꍇ�͌㑱��forward2�����s���Ă͂Ȃ�Ȃ�.
	 * (���ł�ZnkSocket_recv�����Ăяo����Ă���Aselect�ɖ߂��Ȃ��Ă͂Ȃ�Ȃ�)
	 */
	HtpType htp_type = HtpType_e_None;
	MoaiInfo* info = &ctx->hi_;
	char sock_str[ 4096 ];

	ctx->hostname_ = "";
	ctx->port_ = 80;
	ctx->hostname_buf_[0] = '\0';
	ctx->buf_[0] = '\0';
	ctx->result_size_ = 0;
	ctx->recv_executed_ = false;
	ctx->req_method_ = ZnkHtpReqMethod_e_Unknown;
	MoaiBodyInfo_clear( &ctx->body_info_  );
	ZnkStr_clear( ctx->msgs_ );
	ZnkStr_clear( ctx->text_ );
	ZnkStr_clear( ctx->req_urp_ );
	MoaiInfo_clear( info );

	MoaiLog_printf( "== Begin scanHttpFirst sock=%d[%s] ==\n",
			sock,
			MoaiIO_makeSockStr( sock_str, sizeof(sock_str), sock, true ) );
	{
		size_t arg_pos[ 3 ];
		size_t arg_leng[ 3 ];
		char   arg_tkns[ 3 ][ 4096 ];
		int http_version = 0;
		bool check_Host = false;

		ctx->result_size_ = ZnkSocket_recv( sock, ctx->buf_, sizeof(ctx->buf_) );
		if( ctx->result_size_ <= 0 ){
			char errmsg_buf[ 4096 ];
			int  err_code = ZnkNetBase_getLastErrCode();
			if( err_code != 0 ){
				ZnkNetBase_getErrMsg( errmsg_buf, sizeof(errmsg_buf), err_code );
				MoaiIO_addAnalyzeLabel( ctx->msgs_, sock, ctx->result_size_ );
				ZnkStr_addf( ctx->msgs_, "RecvError.\n" );
				ZnkStr_addf( ctx->msgs_, "  errmsg=[%s]\n", errmsg_buf );
				MoaiLog_printf( "%s", ZnkStr_cstr( ctx->msgs_ ) );
				MoaiIO_closeSocket( "  RecvError", sock, fdst_observe );
			} else {
				if( htcn->req_method_ == ZnkHtpReqMethod_e_CONNECT ){
					/* ���̏ꍇ�ARecvZero�����o�����̂�I���ł���AO���ł���A
					 * CONNECT�ʐM�̏I�����Ӗ����A���̑��葤�����Ȃ���΂Ȃ�Ȃ�. */
					ZnkSocket dst_sock = htcn->dst_sock_;
					MoaiIO_closeSocket( "  RecvZero(EndOf CONNECT)", sock, fdst_observe );
					MoaiIO_closeSocket( "  RecvZero(EndOf CONNECT)", dst_sock, fdst_observe );
				} else {
					MoaiIO_closeSocket( "  RecvZero", sock, fdst_observe );
#if 0
					/* �����I. w3m�ł͂��ꂪ�Ȃ��Ƃ��܂������Ȃ��̂ł�? */
					{
						ZnkSocket dst_sock = htcn->dst_sock_;
						HtpIO_closeSocket( "  RecvZero", dst_sock, fdst_observe );
					}
#endif
				}
			}
			htcn->waiting_ = 0;
			return err_code == 0 ? HtpScan_e_RecvZero : HtpScan_e_RecvError;
		}
		ZnkBfr_append_dfr( info->stream_, ctx->buf_, ctx->result_size_ );
		ctx->recv_executed_ = true;

		if( htcn->req_method_ == ZnkHtpReqMethod_e_CONNECT ){
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
		 * �������܂Ƃ߂ď������邽�߁AContent-Length���w�肳��Ă���ꍇ��chunk_size_�Ƃ���
		 * ����.
		 */
		if( htcn->content_length_remain_ ){
			if( htcn->content_length_remain_ >= (size_t)ctx->result_size_ ){
				htcn->content_length_remain_ -= ctx->result_size_;
				ZnkStr_addf( ctx->msgs_, "  NotHttpStart (remain=[%u] result_size=[%d]).\n",
						htcn->content_length_remain_, ctx->result_size_ );
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
				const size_t extra_size = ctx->result_size_ - htcn->content_length_remain_;
				ZnkStr_addf( ctx->msgs_, "  Invalid content_length_remain : (remain=[%u] result_size=[%d] extra_size=[%u]).\n",
						htcn->content_length_remain_, ctx->result_size_, extra_size );
				MoaiLog_printf( "%s", ZnkStr_cstr( ctx->msgs_ ) );
				htcn->content_length_remain_ = 0;
			}
		}

		/***
		 * buf���ɍs�̏I���p�^�[�����܂܂�邩�ǂ����𒲂ׁA
		 * �ŏ��̍s��S�擾����܂�recv���J��Ԃ�.
		 */
		if( !MoaiIO_recvInPtn( info->stream_, sock, fdst_observe, "\r\n", NULL ) ){
			MoaiLog_printf( "  First Line Recv Error.\n" );
			return HtpScan_e_RecvError;
		}

#define GET_ARG_POS( idx ) \
		arg_pos[ idx ] = ZnkS_lfind_arg( (char*)ZnkBfr_data(info->stream_), 0, ZnkBfr_size(info->stream_), \
				idx, arg_leng+idx, " \t\r\n", 4 ); \

#define COPY_ARG_TKNS( idx ) \
		ZnkS_copy( arg_tkns[ idx ], sizeof(arg_tkns[ idx ]), (char*)ZnkBfr_data(info->stream_)+arg_pos[ idx ], arg_leng[ idx ] )

		GET_ARG_POS( 0 );
		COPY_ARG_TKNS( 0 );
		GET_ARG_POS( 1 );
		COPY_ARG_TKNS( 1 );
		GET_ARG_POS( 2 );
		{
			char* p = (char*)ZnkBfr_data(info->stream_)+arg_pos[ 2 ];
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
			htp_type = HtpType_e_Request;
		} else if( ZnkS_eqCase( arg_tkns[ 2 ], "HTTP/1.0" ) ){
			/* HTTP request */
			http_version = 10;
			htp_type = HtpType_e_Request;
		} else if( ZnkS_eqCase( arg_tkns[ 0 ], "HTTP/1.1" ) ){
			/* HTTP response */
			http_version = 11;
			htp_type = HtpType_e_Response;
		} else if( ZnkS_eqCase( arg_tkns[ 0 ], "HTTP/1.0" ) ){
			/* HTTP response */
			http_version = 10;
			htp_type = HtpType_e_Response;
		} else {
			MoaiIO_addAnalyzeLabel( ctx->msgs_, sock, ctx->result_size_ );
			ZnkStr_addf( ctx->msgs_, "NotHttpStart.\n" );
			MoaiLog_printf( "%s", ZnkStr_cstr( ctx->msgs_ ) );
			htp_type = HtpType_e_NotHttpStart;
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
		if( !MoaiIO_recvInPtn( info->stream_, sock, fdst_observe, "\r\n\r\n", &info->hdr_size_ ) ){
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

		ZnkHtpHdrs_clear( &info->hdrs_ );
		MoaiInfo_parseHdr( info, &ctx->body_info_,
				ctx->config_, &htcn->as_local_proxy_, st_moai_port,
				(bool)(htp_type == HtpType_e_Request), htcn->hostname_,
				ctx->mod_ary_, ctx->target_myf_, ctx->req_urp_ );
		MoaiLog_printf( "ctx->req_urp_=[%s]\n", ZnkStr_cstr( ctx->req_urp_ ) );

		switch( htp_type ){
		case HtpType_e_Response:
		{
			MoaiBodyInfo* body_info = &ctx->body_info_;
			const char* req_method = ZnkHtpReqMethod_getCStr( htcn->req_method_ );
			//MoaiIO_addAnalyzeLabel( ctx->msgs_, sock, ctx->result_size_ );
			ZnkStr_addf( ctx->msgs_, "  Http Response [%s][%s](from[%s] for[%s]).\n",
					arg_tkns[ 1 ], arg_tkns[ 2 ], htcn->hostname_, req_method );
			MoaiLog_printf( "%s", ZnkStr_cstr( ctx->msgs_ ) );
			switch( htcn->req_method_ ){
			case ZnkHtpReqMethod_e_GET:
			case ZnkHtpReqMethod_e_POST:
			case ZnkHtpReqMethod_e_HEAD:
				if( ZnkS_eq( arg_tkns[ 1 ], "200" ) ){
					MoaiLog_printf( "  body_info : is_chunked=[%d] is_gzip=[%d] txt_type=[%d]\n",
							body_info->is_chunked_, body_info->is_gzip_, body_info->txt_type_ );
				}
				break;
			default:
				break;
			}
			break;
		}
		case HtpType_e_Request:
		{
			const char* method = "";

			if( ZnkS_isCaseBegin( arg_tkns[ 1 ], "http://" ) ){
				/* Request-URI�ɂ����ăz�X�g�����L�ڂ���Ă���Ƃ݂Ȃ�.
				 * �ŏ���http://�͏������Ă��悩�낤.
				 * ����ɊO����proxy�֓n���ꍇ�A���̂܂܂ł悢���A
				 * �����ł͂Ȃ��ꍇ�Aurp�̌`�ɏC�����Ȃ���΂Ȃ�Ȃ�.
				 */
				ZnkS_copy( ctx->hostname_buf_, sizeof(ctx->hostname_buf_), arg_tkns[ 1 ]+7, arg_leng[ 1 ]-7 );
			} else if( ZnkS_isCaseBegin( arg_tkns[ 1 ], "/" ) ){
				check_Host = true;
			} else if( ZnkS_isCaseBegin( arg_tkns[ 1 ], "*" ) ){
				check_Host = true;
			} else {
				/* Request-URI�ɂ����ăz�X�g�����L�ڂ���Ă���Ƃ݂Ȃ� */
				ZnkS_copy( ctx->hostname_buf_, sizeof(ctx->hostname_buf_), arg_tkns[ 1 ], arg_leng[ 1 ] );
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
				MoaiIO_addAnalyzeLabel( ctx->msgs_, sock, ctx->result_size_ );
				ZnkStr_addf( ctx->msgs_, "UnknownMethod.\n" );
				MoaiLog_printf( "%s", ZnkStr_cstr( ctx->msgs_ ) );
				MoaiIO_closeSocket( "  UnknownMethod", sock, fdst_observe );
				return HtpScan_e_UnknownMethod;
			}
			method = arg_tkns[ 0 ];

			//MoaiIO_addAnalyzeLabel( ctx->msgs_, sock, ctx->result_size_ );
			ZnkStr_addf( ctx->msgs_, "  Http Request [%s][%s].\n", method, arg_tkns[1] );
			MoaiLog_printf( "%s", ZnkStr_cstr( ctx->msgs_ ) );

			if( check_Host ){
				const uint8_t* hdr_data = ZnkBfr_data( info->stream_ );
				const size_t   hdr_size = info->hdr_size_;
				if( !scanHeaderVar( "Host", hdr_data, hdr_size, ctx->hostname_buf_, sizeof(ctx->hostname_buf_) ) ){
					MoaiIO_closeSocket( "  CannotGetHost", sock, fdst_observe );
					return HtpScan_e_CannotGetHost;
				}
			}
			if( !ZnkS_empty(ctx->hostname_buf_) ){
				char* colon;
				char* slash;
				colon = strchr( ctx->hostname_buf_, ':' );
				if( colon ){
					*colon = '\0';
					sscanf( colon+1, "%hd", &ctx->port_ );
				} else {
					ctx->port_ = 80;
				}
				slash = strchr( ctx->hostname_buf_, '/' );
				if( slash ){
					*slash = '\0';
				}
				ctx->hostname_ = ctx->hostname_buf_;
			}
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
		htcn->content_length_remain_ = 0;
	} else {
		htcn->content_length_remain_ = ctx->body_info_.content_length_;
		MoaiLog_printf( "  body_info_.content_length_=[%u]\n", ctx->body_info_.content_length_ );
	}

	/***
	 * Connection keep-alive�̗L�����m�F����.
	 */
	{
		ZnkVarp varp = ZnkHtpHdrs_find( info->hdrs_.vars_, "Connection", Znk_NPOS );
		htcn->is_keep_alive_ = true;
		if( varp ){
			const char* val  = ZnkHtpHdrs_val_cstr( varp, 0 );
			htcn->is_keep_alive_ = ZnkS_eqCase( val, "keep-alive" );
		}
	}

	if( htcn->content_length_remain_ ){
		size_t recved_content_length = ZnkBfr_size( info->stream_ ) - info->hdr_size_;
		if( htcn->content_length_remain_ < recved_content_length ){
			htcn->content_length_remain_ = 0;
		} else {
			htcn->content_length_remain_ -= recved_content_length;
		}
	}


	//MoaiLog_printf( "  content_length_remain=[%u]\n", htcn->content_length_remain_ );
	//MoaiLog_printf( "  hdr_size=[%u]\n", info->hdr_size_ );
	if( htp_type == HtpType_e_Response ){

		MoaiConnection htcn = MoaiConnection_find( sock );
		const char* hostname = htcn->hostname_;
		MoaiModule mod = MoaiModuleAry_find_byHostname( ctx->mod_ary_, ctx->target_myf_, hostname );
		if( mod ){
			MoaiModule_invokeOnResponseHdr( mod, info->hdrs_.vars_ );
		}

		/***
		 * HEAD�̏ꍇ��Connect-Length����[���ł���ɂ��ւ�炸���̃f�[�^�����Ȃ�
		 */
		if( htcn->req_method_ != ZnkHtpReqMethod_e_HEAD ){
			processResponse( sock, ctx, fdst_observe, &htcn->content_length_remain_ );
		}
		return HtpScan_e_Response;
	}
	MoaiLog_printf( "  New Destination : hostname=[%s] port=[%d]\n", ctx->hostname_, ctx->port_ );
	if( !ZnkS_empty( htcn->hostname_ ) ){
		MoaiLog_printf( "  Old Destination : hostname=[%s] port=[%d]\n", htcn->hostname_, htcn->port_ );
	}

	{
		ZnkStrDAry ignore_hosts = ZnkMyf_find_lines( ctx->config_, "ignore_hosts" );
		if( ignore_hosts && ZnkStrDAry_find( ignore_hosts, 0, ctx->hostname_, Znk_NPOS ) != Znk_NPOS ){
			int ret = MoaiIO_sendTxtf( sock, "text/html", "Moai : Ignored Host Blocking[%s] sock=[%d].\n",
					ctx->hostname_, sock );
			MoaiLog_printf( "  Ignored hostname=[%s]\n", ctx->hostname_ );
			if( ret < 0 || !htcn->is_keep_alive_ || ctx->req_method_ == ZnkHtpReqMethod_e_CONNECT ){
				MoaiIO_closeSocket( "  IgnoreBlocking", sock, fdst_observe );
			}
			return HtpScan_e_HostBloking;
		}
	}

	return HtpScan_e_Request;
}


static void
debugInterestURP( const MoaiInfo* info, const MoaiConnection htcn, HtpScanType scan_type, ZnkMyf analysis )
{
	const char* req_urp    = ZnkStr_cstr( htcn->req_urp_ );
	const char* req_method = ZnkHtpReqMethod_getCStr( htcn->req_method_ );
	ZnkStrDAry interest_urp = ZnkMyf_find_lines( analysis, "interest_urp" );
	if( interest_urp && ZnkStrDAry_find( interest_urp, 0, req_urp, Znk_NPOS ) != Znk_NPOS ){
		if( ZnkStrDAry_size(info->hdrs_.hdr1st_) > 1 ){
			MoaiLog_printf( "  Moai %s %s%s : InterestURP=[%s]\n",
					req_method,
					( scan_type == HtpScan_e_Request ) ? "Request" : "Response ",
					( scan_type == HtpScan_e_Request ) ? "" : ZnkStrDAry_at_cstr(info->hdrs_.hdr1st_,1),
					req_urp );
		}
		debugHdrVars( info->hdrs_.vars_, "    " );
	}
}

static int
sendToDst( ZnkSocket dst_sock, MoaiInfo* info, MoaiBodyInfo* body_info, ZnkStr htm1, HtpScanType scan_type )
{
	int result_size = 0;
	switch( scan_type ){
	case HtpScan_e_Request:
	case HtpScan_e_Response:
		/* HTTP�w�b�_�𑗐M */
		sendHdrs( dst_sock, info->hdrs_.hdr1st_, info->hdrs_.vars_ ); 
		if( body_info->txt_type_ != MoaiText_e_Binary ){
			result_size = ZnkSocket_send( dst_sock,
					(uint8_t*)ZnkStr_cstr(htm1), ZnkStr_leng(htm1) );
		} else {
			/* �f�[�^�𑗐M */
			result_size = ZnkSocket_send( dst_sock,
					ZnkBfr_data(info->stream_)+info->hdr_size_, ZnkBfr_size(info->stream_)-info->hdr_size_ );
		}
		break;
	default:
		/* �f�[�^�𑗐M */
		result_size = ZnkSocket_send( dst_sock, ZnkBfr_data(info->stream_), ZnkBfr_size(info->stream_) );
		break;
	}
	return result_size;
}

static bool
doLocalProxy( MoaiContext ctx, ZnkSocket I_sock, MoaiConnection htcn,
		MoaiFdSet mfds,
		MoaiInfo* info, HtpScanType scan_type )
{
	ZnkErr_D( err );
	ZnkFdSet fdst_observe = MoaiFdSet_fdst_observe( mfds );

	if( htcn->dst_sock_ == ZnkSocket_INVALID ){
		ZnkSocket dst_sock = ZnkSocket_open();
		{
			MoaiConnection htcn_r = MoaiConnection_intern( dst_sock, MoaiSockType_e_Outer );
			if( htcn_r ){
				htcn_r->dst_sock_     = I_sock;
				htcn_r->port_         = ctx->port_;
				ZnkS_copy( htcn_r->hostname_, sizeof(htcn_r->hostname_), ctx->hostname_, Znk_NPOS );
			} else {
				MoaiLog_printf( "  doLocalProxy : MoaiConnection_intern failure.\n" );
				MoaiIO_sendTxtf( I_sock, "text/html", "<p><b>Moai : Tunnel : MoaiConnection_intern Error.</b></p>\n" );
				MoaiIO_closeSocket( "  MoaiConnection_intern", I_sock, fdst_observe );
				MoaiIO_closeSocket( "  MoaiConnection_intern", dst_sock, fdst_observe );
				return false;
			}
		}
		htcn->dst_sock_     = dst_sock;
		htcn->port_         = ctx->port_;
		ZnkS_copy( htcn->hostname_, sizeof(htcn->hostname_), ctx->hostname_, Znk_NPOS );
		MoaiLog_printf( "  Connecting... dst_sock=[%d]\n", dst_sock );

		{
			const char* cnct_hostname = ctx->hostname_;
			uint16_t    cnct_port     = ctx->port_;

			if( MoaiParentProxy_isUse( ctx->config_, cnct_hostname, Znk_NPOS ) ){
				/***
				 * ���̏ꍇ�����I�ɐڑ�������parent_proxy�ł���.
				 */
				cnct_hostname = MoaiParentProxy_getHostname();
				cnct_port     = MoaiParentProxy_getPort();
			}
			if( !ZnkSocket_connectToServer( dst_sock, cnct_hostname, cnct_port, &err ) ){
				int ret = MoaiIO_sendTxtf( I_sock, "text/html",
						"<p><b>Moai : Tunnel : connectToServer Error.</b></p>I_sock=[%d] hostname=[%s] port=[%d]<br>%s\n",
						dst_sock, cnct_hostname, cnct_port, ZnkErr_cstr(err) );
				if( ret < 0 || !htcn->is_keep_alive_ ){
					MoaiIO_closeSocket( "  ConnectToServer", I_sock, fdst_observe );
					MoaiIO_closeSocket( "  ConnectToServer", dst_sock, fdst_observe );
				}
				return false;
			}
			htcn->exile_time_ = 0;
		}

		MoaiLog_printf( "  Connected : port=[%d] sock=[I:%d <=> O:%d]\n", ctx->port_, I_sock, dst_sock );
		if( !ZnkFdSet_set( fdst_observe, dst_sock ) ){
			MoaiLog_printf( "  ZnkFdSet_set failure. sock(by connect)=[%d]\n", dst_sock );
			MoaiFdSet_reserveConnectSock( mfds, dst_sock );
		}
	}

	if( htcn->dst_sock_ != ZnkSocket_INVALID ){
		ZnkSocket dst_sock = htcn->dst_sock_;
		if( !ctx->recv_executed_ ){
			MoaiLog_printf( "  recv_executed is false\n" );
		} else {
			if( scan_type == HtpScan_e_Request ){
				MoaiConnection htcn_r = MoaiConnection_find( dst_sock );
				htcn->req_method_   = ctx->req_method_;
				htcn_r->req_method_ = ctx->req_method_;
				htcn_r->exile_time_ = 0;
			}
			if( ctx->req_method_ == ZnkHtpReqMethod_e_CONNECT ){
				bool connection_established = true;
				if( MoaiParentProxy_isUse( ctx->config_, htcn->hostname_, Znk_NPOS ) ){
					/***
					 * ���̏ꍇ��O���ɂ�CONNECT�w�b�_�𑗂�A
					 * ���̉������e��200�ł��邩�ۂ����m�F����K�v������.
					 */
					ZnkBfr bfr = ZnkBfr_create_null();
					ZnkHtpHdrs_extendToStream( info->hdrs_.hdr1st_, info->hdrs_.vars_, bfr, true );
					ZnkSocket_send( dst_sock, ZnkBfr_data(bfr), ZnkBfr_size(bfr) );

					MoaiInfo_clear( info );
					if( !MoaiIO_recvInPtn( info->stream_, dst_sock, fdst_observe, "\r\n\r\n", &info->hdr_size_ ) ){
						MoaiLog_printf( "  Hdr Recv Error.\n" );
						return false;
					}

					ZnkBfr_destroy( bfr );
					connection_established = false;
					{
						const char* q = strstr( (char*)ctx->buf_, "\r\n" );
						if( q ){
							ZnkHtpHdrs_registHdr1st( info->hdrs_.hdr1st_, (char*)ctx->buf_, (uint8_t*)q - ctx->buf_ );
							MoaiLog_printf( "Recving status [%s]\n", ZnkStrDAry_at_cstr( info->hdrs_.hdr1st_, 1 ) );
							if( ZnkS_eq( ZnkStrDAry_at_cstr( info->hdrs_.hdr1st_, 1 ), "200" ) ){
								connection_established = true;
							}
						}
					}
				}
				if( connection_established ){
					/***
					 * ���̏ꍇ�͂܂�O���ɉ�������Ȃ�.
					 * I���֒P�� HTTP/1.0 200 connection established ��Ԃ��A
					 * ���߂�I�������recv��select�ɂ��҂�.
					 * ���̂Ƃ��A(Unknown��)�ʐM�̊J�n�ƂȂ�I_sock�́A�ŏ���CONNECT���\�b�h��
					 * �����Ă���I_sock�Ɠ����ł���(�����łȂ���΂Ȃ�Ȃ�).
					 * ����āA���̂悤�ȒʐM�̊J�n��r���ł��邩�ۂ���req_method_�l�ɂ����
					 * �ȒP�Ɏ��ʂł���.
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
					ctx->result_size_ = ZnkSocket_send( I_sock, (uint8_t*)msg, strlen(msg) );
					if( ctx->result_size_ == -1 ){
						ZnkSysErrnoInfo* err_info = ZnkSysErrno_getInfo( ZnkSysErrno_errno() );
						ZnkErr_internf( &err,
								"  ZnkSocket_send : Failure : SysErr=[%s:%s]",
								err_info->sys_errno_key_, err_info->sys_errno_msg_ );
						MoaiIO_closeSocket( "  CONNECT", I_sock, fdst_observe );
						MoaiIO_closeSocket( "  CONNECT", dst_sock, fdst_observe );
						return false;
					}
					MoaiLog_printf( "  CONNECT established I=[%d] O=[%d].\n", I_sock, htcn->dst_sock_ );
				} else {
					ZnkBfr bfr = ZnkBfr_create_null();
					ZnkHtpHdrs_extendToStream( info->hdrs_.hdr1st_, info->hdrs_.vars_, bfr, true );
					ctx->result_size_ = ZnkSocket_send( I_sock, ZnkBfr_data(bfr), ZnkBfr_size(bfr) );
					ZnkBfr_destroy( bfr );
					if( ctx->result_size_ == -1 ){
						ZnkSysErrnoInfo* err_info = ZnkSysErrno_getInfo( ZnkSysErrno_errno() );
						ZnkErr_internf( &err,
								"  ZnkSocket_send : Failure : SysErr=[%s:%s]",
								err_info->sys_errno_key_, err_info->sys_errno_msg_ );
						MoaiIO_closeSocket( "  CONNECT", I_sock, fdst_observe );
						MoaiIO_closeSocket( "  CONNECT", dst_sock, fdst_observe );
						return false;
					}
				}

				htcn->as_local_proxy_ = true;
				{
					MoaiConnection htcn_r = MoaiConnection_find( dst_sock );
					htcn_r->as_local_proxy_ = true;
				}
			} else {

				/* Set req_urp */
				if( scan_type == HtpScan_e_Request ){
					switch( htcn->req_method_ ){
					case ZnkHtpReqMethod_e_GET:
					case ZnkHtpReqMethod_e_POST:
					case ZnkHtpReqMethod_e_HEAD:
					{
						MoaiConnection htcn_r = MoaiConnection_find( dst_sock );
						ZnkStr_set( htcn_r->req_urp_, ZnkStr_cstr( ctx->req_urp_ ) );
						ZnkStr_set( htcn->req_urp_,   ZnkStr_cstr( ctx->req_urp_ ) );
						break;
					}
					default:
						break;
					}
				}

				/* Cookie filtering */
				if( scan_type == HtpScan_e_Request ){
					switch( htcn->req_method_ ){
					case ZnkHtpReqMethod_e_GET:
					case ZnkHtpReqMethod_e_POST:
					case ZnkHtpReqMethod_e_HEAD:
					{
						const MoaiModule mod = MoaiModuleAry_find_byHostname(
								ctx->mod_ary_, ctx->target_myf_, ctx->hostname_ );
						if( mod ){
							MoaiModule_filtCookieVars( mod, info->hdrs_.vars_ );
						}
						break;
					}
					default:
						break;
					}
				}

				debugInterestURP( info, htcn, scan_type, ctx->analysis_ );

				if( htcn->req_method_ == ZnkHtpReqMethod_e_POST && scan_type == HtpScan_e_Request ){
					info->proxy_hostname_ = MoaiParentProxy_getHostname();
					info->proxy_port_     = MoaiParentProxy_getPort();
					MoaiPost_proc( I_sock, fdst_observe,
							ctx->hostname_, info, &ctx->body_info_,
							htcn->as_local_proxy_, st_master_confirm,
							ctx->mod_ary_, ctx->target_myf_, ctx->analysis_, ZnkStr_cstr(ctx->req_urp_) );
				} else {

					/***
					 * �Ƃ肠��������send�͎��s���Ȃ��Ɖ���.
					 */
					ctx->result_size_ = sendToDst( dst_sock, info, &ctx->body_info_, ctx->text_, scan_type );

					if( ctx->result_size_ == -1 ){
						ZnkSysErrnoInfo* err_info = ZnkSysErrno_getInfo( ZnkSysErrno_errno() );
						ZnkErr_internf( &err,
								"  ZnkSocket_send : Failure : SysErr=[%s:%s]",
								err_info->sys_errno_key_, err_info->sys_errno_msg_ );
						MoaiIO_closeSocket( "  SendToOuter", I_sock, fdst_observe );
						MoaiIO_closeSocket( "  SendToOuter", dst_sock, fdst_observe );
						return false;
					}
					{
						MoaiConnection htcn_r = MoaiConnection_find( dst_sock );
						htcn_r->waiting_ = (uint64_t)getCurrentSec();
					}
				}
			}
			return true;
		}
	}
	return true;
}

static int
printConfig( ZnkSocket sock )
{
	int ret;
	char proxy[ 1024 ] = "";
	const char* parent_proxy_hostname = NULL;
	ZnkStr html = ZnkStr_new( "" );
	ZnkStr_add( html, "<p><b><img src=\"moai.png\"> Moai WebServer : Config</b></p>\n" );
	ZnkStr_addf( html, "<form action=\"http://127.0.0.1:%d/config\" method=\"POST\" enctype=\"multipart/form-data\">\n", st_moai_port );
	parent_proxy_hostname = MoaiParentProxy_getHostname();
	if( parent_proxy_hostname ){
		Znk_snprintf( proxy, sizeof(proxy), "%s:%d",
				parent_proxy_hostname, MoaiParentProxy_getPort() );
	}
	ZnkStr_addf( html, "Parent Proxy : <input type=text name=proxy size=\"28\" value=\"%s\" readonly disabled=true><br>\n", proxy );
	ZnkStr_addf( html, "<input type=checkbox name=master_confirm value=\"on\" %s>Master Confirm ",
			st_master_confirm ? "checked" : "" );
	ZnkStr_add( html, " : This is enable/disable switch of Confirm Mode at HTTP posting.\n<br>" );
	ZnkStr_add( html, "<input type=submit value=\"�X�V\">\n" );
	ZnkStr_add( html, "</form>\n" );
	ret = MoaiIO_sendTxtf( sock, "text/html", ZnkStr_cstr(html) );
	ZnkStr_delete( html );
	return ret;
}

static bool
readAndReply( MoaiContext ctx, ZnkSocket sock, MoaiConnection htcn, MoaiFdSet mfds )
{
	MoaiInfo* info = &ctx->hi_;
	ZnkFdSet fdst_observe = MoaiFdSet_fdst_observe( mfds );

	HtpScanType scan_type = HtpScan_e_None;
	if( htcn == NULL ){
		MoaiIO_closeSocket( "  SendToOuter", sock, fdst_observe );
		return false;
	}

	scan_type = scanHttpFirst( ctx, htcn, sock, fdst_observe );
	switch( scan_type ){
	case HtpScan_e_Request:
		if( !ZnkS_eq( htcn->hostname_, ctx->hostname_ ) ||  htcn->port_ != ctx->port_ ){
			/***
			 * ���̏ꍇ�Ahtcn->dst_sock_�Ƃ̋�RAR�͊���������A�u���E�U��
			 * htcn->src_sock_�̐ڑ������̂܂܈ێ������T�C�N�����āA�V���Ȃ�HTTP��
			 * �ڑ����Ă���Ǝv����. �������A�V���Ȑڑ���z�X�g���ȑO�̂���Ɠ����Ƃ͌���Ȃ�.  
			 * ������`�F�b�N���邽�߁A��̔�r���s���A�z�X�g���܂���port���ȑO�̂���Ƃ�
			 * �قȂ�ꍇ�A��htcn->dst_sock_��close���A�V���Ȑڑ�������Ă����V����
			 * htcn->dst_sock_�Ƃ���.
			 *
			 * ���̂Ƃ��Ahtcn->src_sock_�̓��T�C�N������Ahtcn->dst_sock_�݂̂�
			 * ���Ȃ�����邱�ƂɂȂ�. �u���E�U�ɂ���Ă͂��̂悤�Ȃ��Ƃ��L�蓾��.
			 * �Ⴆ��Firefox��www.google.co.jp�ɐڑ����A����ɕʂ̃T�C�g�֐ڑ�����ꍇ�Ȃǂł���.
			 */
			MoaiIO_closeSocket( "  SrcRecycle", htcn->dst_sock_, fdst_observe );
			htcn->dst_sock_ = ZnkSocket_INVALID;
		}
		break;
	case HtpScan_e_Response:
		break;
	case HtpScan_e_RecvZero:
	case HtpScan_e_RecvError:
		return false;
	case HtpScan_e_NotHttpStart:
		break;
	case HtpScan_e_UnknownMethod:
	case HtpScan_e_CannotGetHost:
	case HtpScan_e_HostBloking:
		return false;
	default:
		return false;
	}

	if( !htcn->as_local_proxy_ ){
		int ret = 0;

		/* WebServer�Ƃ��ď��� */
		switch( ctx->req_method_ ){
		case ZnkHtpReqMethod_e_GET:
		{
			ZnkStr str1 = ctx->req_urp_;
			ZnkStr str2 = ZnkStr_new( "./doc_root" );
			MoaiLog_printf( "  As WebServer GET\n" );
			/***
			 * �����͖{���Ȃ� str1���� .. �𖳌��ɂ���Ȃǂ̃T�j�^�C�Y���s��Ȃ���΂Ȃ�Ȃ�.
			 */
			ZnkStr_add( str2, ZnkStr_cstr(str1) );
			if( ZnkStr_last(str2) == '/' ){
				ZnkStr_add( str2, "index.html" );
			}
			if( ZnkStr_isEnd(str2, "/config" ) ){
				ret = printConfig( sock );
			} else if( !MoaiIO_sendResponseFile( sock, ZnkStr_cstr(str2) ) ){
				ret = MoaiIO_sendTxtf( sock, "text/html", "<p><b><img src=\"moai.png\"> Moai WebServer : 404 Not found [%s].</b></p>\n",
						ZnkStr_cstr(str2) );
			}
			if( ret < 0 || !htcn->is_keep_alive_ ){
				MoaiIO_closeSocket( "  WebServerGET", sock, fdst_observe );
			}
			return true;
		}
		case ZnkHtpReqMethod_e_POST:
		{
			ZnkStr str1 = ctx->req_urp_;
			ZnkStr str2 = ZnkStr_new( "" );
			MoaiInfo* new_info;
			ZnkStr_addf( str2, "<p><b><img src=\"moai.png\"> Moai WebServer : POST [%s].</b></p>\n",
					ZnkStr_cstr(str1) );
			ZnkStr_add( str2, "<pre>\n" );
			new_info = MoaiPost_parsePostVars( sock, fdst_observe,
					str2, ctx->hostname_, info, &ctx->body_info_,
					ctx->mod_ary_, ctx->target_myf_ );
			ZnkStr_add( str2, "</pre>\n" );

			if( new_info->vars_ ){
				ZnkVarp var = NULL;
				var = ZnkVarpDAry_find_byName_literal( new_info->vars_, "master_confirm", false );
				st_master_confirm = var ? true : false;
			}

			if( ZnkStr_isEnd(str1, "/config" ) ){
				ret = printConfig( sock );
			} else {
				ret = MoaiIO_sendTxtf( sock, "text/html", ZnkStr_cstr( str2 ) );
			}
			ZnkStr_delete( str2 );

			if( ret < 0 || !htcn->is_keep_alive_ ){
				MoaiIO_closeSocket( "  WebServerPOST", sock, fdst_observe );
			}
			return true;
		}
		default:
			break;
		}
	}
	/* LocalProxy�Ƃ��ď��� */
	return doLocalProxy( ctx, sock, htcn, mfds, info, scan_type );
}

static void report_observe( MoaiFdSet mfds, void* arg )
{
	ZnkSocketDAry wk_sock_dary = MoaiFdSet_wk_sock_dary( mfds );
	ZnkFdSet      fdst_observe = MoaiFdSet_fdst_observe( mfds );
	MoaiIO_reportFdst( "Moai : Observe ", wk_sock_dary, fdst_observe, true );
}
static void on_accept( MoaiFdSet mfds, ZnkSocket new_accept_sock, void* arg )
{
	MoaiConnection_intern( new_accept_sock, MoaiSockType_e_Inner );
	MoaiLog_printf( "Moai : new_accept_sock=[%d]\n", new_accept_sock );
}
static bool recv_and_send( MoaiFdSet mfds, ZnkSocket sock, void* arg )
{
	MoaiConnection htcn = MoaiConnection_find( sock );
	MoaiContext    ctx  = (MoaiContext)arg;
	return readAndReply( ctx, sock, htcn, mfds );
}

int
MoaiServer_main( int argc, char **argv )
{
	int result = EXIT_FAILURE;
	int sel_ret;
	MoaiFdSet mfds = NULL;

	const char* host = "127.0.0.1";
	ZnkServer     srver       = NULL;
	ZnkSocket     listen_sock = ZnkSocket_INVALID;
	MoaiFdSetFuncArg_Report   fnca_report    = { report_observe, NULL };
	MoaiFdSetFuncArg_OnAccept fnca_on_accept = { on_accept, NULL };
	MoaiFdSetFuncArg_RAS      fnca_ras       = { recv_and_send, NULL };
	struct ZnkTimeval waitval;
	bool req_fdst_report = true;
	MoaiContext ctx = MoaiContext_create();

	fnca_ras.arg_ = ctx;

	MoaiLog_open( "moai_log.log", false );

	if( ZnkMyf_load( ctx->target_myf_, "target.myf" ) ){
		MoaiLog_printf("Moai : target load OK.\n" );
	} else {
		MoaiLog_printf("Moai : target load NG.\n" );
	}
	if( ZnkMyf_load( ctx->config_, "config.myf" ) ){
		MoaiLog_printf("Moai : config load OK.\n" );
	} else {
		MoaiLog_printf("Moai : config load NG.\n" );
	}
	if( ZnkMyf_load( ctx->analysis_, "analysis.myf" ) ){
		MoaiLog_printf("Moai : analysis load OK.\n" );
	} else {
		MoaiLog_printf("Moai : analysis load NG.\n" );
	}

	MoaiModuleAry_loadAllModules( ctx->mod_ary_, ctx->target_myf_ );

	/***
	 * parse config
	 */
	{
		ZnkVarpDAry vars = ZnkMyf_find_vars( ctx->config_, "config" );
		if( vars ){
			ZnkVarp var = NULL;
			var = ZnkVarpDAry_find_byName_literal( vars, "master_confirm", false );
			if( var ){
				st_master_confirm = ZnkS_eq( ZnkVar_cstr(var), "on" );
			}
			var = ZnkVarpDAry_find_byName_literal( vars, "moai_port", false );
			if( var ){
				sscanf( ZnkVar_cstr( var ), "%hu", &st_moai_port );
			}
			var = ZnkVarpDAry_find_byName_literal( vars, "proxy_indicating_mode", false );
			if( var ){
				if( ZnkS_eq( ZnkVar_cstr(var), "minus" ) ){
					MoaiParentProxy_setIndicatingMode( -1 );
				} else if( ZnkS_eq( ZnkVar_cstr(var), "plus" ) ){
					MoaiParentProxy_setIndicatingMode( 1 );
				} else {
					MoaiParentProxy_setIndicatingMode( 0 );
				}
			}
			var = ZnkVarpDAry_find_byName_literal( vars, "enable_log_file", false );
			if( var ){
				st_enable_log_file = ZnkS_eq( ZnkVar_cstr(var), "on" );
			}
		}
	}

	if( !st_enable_log_file ){
		MoaiLog_printf( "Moai : config : enable_log_file is off.\n" );
		MoaiLog_printf( "Moai : Stop writing to log file and close it here.\n" );
		MoaiLog_close();
	}

	waitval.tv_sec  = 2;        /* �҂����Ԃ� 2.500 �b���w�� */
	waitval.tv_usec = 500;

	ZnkNetBase_initiate( false );

	if( argc > 1 ){
		const char* parent_proxy_hostname = argv[ 1 ];
		uint16_t    parent_proxy_port = 0;
		if( argc > 2 ){
			sscanf( argv[2], "%hd", &parent_proxy_port );
		}
		MoaiParentProxy_set( parent_proxy_hostname, parent_proxy_port );
	}

	MoaiConnection_initiate();
	MoaiConnection_clearAll();
	MoaiPost_initiate();
	                              
	srver = ZnkServer_create( host, st_moai_port );
	if( srver == NULL ){
		MoaiLog_printf( "Moai : Cannot create server. May be port %d is already used.\n", st_moai_port );
		goto FUNC_END;
	}
	listen_sock = ZnkServer_getListenSocket( srver );
	MoaiConnection_intern( listen_sock, MoaiSockType_e_Listen );
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
			continue;
		}
		if( sel_ret == 0 ){
			/* select timeout */
			ZnkSocketDAry wk_sock_dary = MoaiFdSet_wk_sock_dary( mfds );
			ZnkFdSet      fdst_observe = MoaiFdSet_fdst_observe( mfds );
			if( MoaiIO_procExile( wk_sock_dary, fdst_observe ) ){
				req_fdst_report = true;
			}
			continue;
		}
		                            
		MoaiFdSet_process( mfds, &fnca_on_accept, &fnca_ras );

		/***
		 * ����A��ɕ\������悤�ɂ��Ă��邪�A
		 * ���炩�ȕύX�����������ꍇ�̂ݕ\������悤�ɂ��������X�}�[�g�ł���.
		 */
		req_fdst_report = true;

	}
	result = EXIT_SUCCESS;

FUNC_END:
	MoaiFdSet_destroy( mfds );
	ZnkSocket_close( listen_sock );
	ZnkNetBase_finalize();
	MoaiContext_destroy( ctx );
	MoaiLog_close();
	return result;
}
