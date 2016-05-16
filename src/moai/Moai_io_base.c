#include "Moai_io_base.h"
#include "Moai_connection.h"
#include "Moai_log.h"
#include <Znk_net_base.h>
#include <Znk_socket.h>
#include <Znk_missing_libc.h>
#include <Znk_str.h>
#include <Znk_s_base.h>
#include <time.h>


static void
formatTimeString( char* buf, size_t buf_size, time_t clck )
{
	struct 	tm *timePtr;
	if( clck == 0 )
		clck = time(NULL);
	timePtr = gmtime( &clck );
	//strftime( buf, buf_size, "%a, %d %b %Y %T GMT", timePtr );
	strftime( buf, buf_size, "%a, %d %b %Y %H:%M:%S GMT", timePtr );
}
static void
makeResponseHdr_200( ZnkStr str, const char* content_type, int contentLength, int modTime )
{
	char timeBuf[ 128 ];
	formatTimeString( timeBuf, sizeof(timeBuf), 0 );

	ZnkStr_addf( str,
			"HTTP/1.0 200 OK\r\n"
			"Date: %s\r\n"
			"Content-Type: %s\r\n",
			timeBuf, content_type
	);

	if( contentLength > 0 ){
		ZnkStr_addf( str,
				"Content-Length: %d\r\n",
				contentLength
		);
	}
	ZnkStr_add( str, "\r\n" );
}

int
MoaiIO_sendTxtf( ZnkSocket sock, const char* content_type, const char* fmt, ... )
{
	int ret = 0;
	va_list ap;
	ZnkStr ans = ZnkStr_new("");
	ZnkStr msg = ZnkStr_new("");

	va_start(ap, fmt);

	ZnkStr_vsnprintf( msg, Znk_NPOS, Znk_NPOS, fmt, ap );
	makeResponseHdr_200( ans, content_type, ZnkStr_leng(msg), 0 );
	ZnkStr_add( ans, ZnkStr_cstr(msg) );
	ret = ZnkSocket_send_cstr( sock, ZnkStr_cstr(ans) );

	va_end(ap);

	ZnkStr_delete( msg );
	ZnkStr_delete( ans );
	return ret;
}
bool
MoaiIO_sendResponseFile( ZnkSocket sock, const char* filename )
{
	bool result = false;
	ZnkFile fp = ZnkF_fopen( filename, "rb" );
	if( fp ){
		size_t read_size;
		uint8_t buf[ 4096 ];
		ZnkBfr bfr = ZnkBfr_create_null();
		ZnkStr str = ZnkStr_new( "" );
		const char* content_type = "text/html";
		const char* ext = ZnkS_get_extension( filename, '.' );

		if( ZnkS_eqCase( ext, "jpg" ) || ZnkS_eqCase( ext, "jpeg" ) ){
			content_type = "image/jpeg";
		} else if( ZnkS_eqCase( ext, "png" ) ){
			content_type = "image/png";
		} else if( ZnkS_eqCase( ext, "gif" ) ){
			content_type = "image/gif";
		} else if( ZnkS_eqCase( ext, "htm" ) || ZnkS_eqCase( ext, "html" ) ){
			content_type = "text/html";
		} else if( ZnkS_eqCase( ext, "css" ) ){
			content_type = "text/css";
		} else if( ZnkS_eqCase( ext, "txt" ) ){
			content_type = "text/plain";
		} else if( ZnkS_eqCase( ext, "xml" ) ){
			content_type = "text/xml";
		} else if( ZnkS_eqCase( ext, "js" ) ){
			content_type = "application/javascript";
		} else {
			content_type = "application/octet-stream";
		}

		while( true ){
			read_size = ZnkF_fread( buf, 1, sizeof(buf), fp );
			if( read_size == 0 ){
				break; /* eof */
			}
			ZnkBfr_append_dfr( bfr, buf, read_size );
		}
		makeResponseHdr_200( str, content_type, ZnkBfr_size(bfr), 0 );
		ZnkBfr_insert( bfr, 0, (uint8_t*)ZnkStr_cstr(str), ZnkStr_leng(str) );
		if( ZnkSocket_send( sock, ZnkBfr_data(bfr), ZnkBfr_size(bfr) ) < 0 ){
			result = false;
		}
		ZnkBfr_destroy( bfr );
		ZnkF_fclose( fp );
		result = true;
	}
	return result;
}


Znk_INLINE double
getCurrentSec( void ){ return ( 1.0 / CLOCKS_PER_SEC ) * clock() + 1.0; }

static char
getSockTypeCh( MoaiSockType sock_type )
{
	switch( sock_type ){
	case MoaiSockType_e_Listen: return 'L';
	case MoaiSockType_e_Outer:  return 'O';
	case MoaiSockType_e_Inner:  return 'I';
	default: break;
	}
	return 'N';
}

const char*
MoaiIO_makeSockStr( char* buf, size_t buf_size, ZnkSocket sock, bool detail )
{
	MoaiConnection mcn = NULL;
	MoaiSockType sock_type = MoaiSockType_e_None;

	if( MoaiConnection_isListeningSock( sock ) ){
		sock_type = MoaiSockType_e_Listen;
	} else if( (mcn = MoaiConnection_find_byISock( sock )) != NULL ){
		sock_type = MoaiSockType_e_Inner;
	} else if( (mcn = MoaiConnection_find_byOSock( sock )) != NULL ){
		sock_type = MoaiSockType_e_Outer;
	}

	if( mcn ){
		const char* hostname = MoaiConnection_hostname( mcn );
		const uint16_t port  = MoaiConnection_port( mcn );
		if( detail
		  && sock_type != MoaiSockType_e_Listen
		  && sock_type != MoaiSockType_e_None
		  && !ZnkS_empty(hostname) )
		{
			ZnkSocket I_sock = MoaiConnection_I_sock( mcn );
			ZnkSocket O_sock = MoaiConnection_O_sock( mcn );
			const char* arrow = sock_type == MoaiSockType_e_Inner ? "=>" :
				sock_type == MoaiSockType_e_Outer ? "<=" : "<=>";

			if( port == 80 ){
				Znk_snprintf( buf, buf_size, "%s(%d%s%d)",
						hostname, I_sock, arrow, O_sock );
			} else {
				Znk_snprintf( buf, buf_size, "%s:%d(%d%s%d)",
						hostname, port, I_sock, arrow, O_sock );
			}
		} else {
			Znk_snprintf( buf, buf_size, "%d(%c)", sock, getSockTypeCh( sock_type ) );
		}
	} else {
		Znk_snprintf( buf, buf_size, "%d(%c)", sock, getSockTypeCh( sock_type ) );
	}
	return buf;
}
void
MoaiIO_addAnalyzeLabel( ZnkStr msgs, ZnkSocket sock, int result_size, const char* label )
{
	char scbuf[ 1024 ];
	ZnkStr_addf( msgs, "  %s : sock=[%s] result_size=[%d] : ",
			label, MoaiIO_makeSockStr( scbuf, sizeof(scbuf), sock, true ),result_size );
}

static bool
isReported( ZnkObjAry repoted, MoaiConnection query_mcn )
{
	const size_t size = ZnkObjAry_size( repoted );
	size_t idx;
	MoaiConnection mcn = NULL;
	for( idx=0; idx<size; ++idx ){
		mcn = (MoaiConnection)ZnkObjAry_at( repoted, idx );
		if( query_mcn == mcn ){
			return true;
		}
	}
	return false;
}
static MoaiConnection
findReportingConnection( ZnkObjAry repoted, ZnkSocket sock )
{
	MoaiConnection mcn = NULL;
	mcn  = MoaiConnection_find_byISock( sock );
	if( mcn ){
		return isReported( repoted, mcn ) ? NULL : mcn;
	}
	mcn  = MoaiConnection_find_byOSock( sock );
	if( mcn ){
		return isReported( repoted, mcn ) ? NULL : mcn;
	}
	return NULL;
}

void
MoaiIO_reportFdst( const char* label, ZnkSocketAry sock_ary, ZnkFdSet fdst_observe, bool detail )
{
	size_t sock_ary_size;

	ZnkSocketAry_clear( sock_ary );
	ZnkFdSet_getSocketAry( fdst_observe, sock_ary );
	sock_ary_size = ZnkSocketAry_size( sock_ary );

	if( sock_ary_size ){
		bool   first_printed = false;
		MoaiConnection mcn = NULL;
		size_t line_leng = 0;
		ZnkSocket sock;
		size_t    i;
		char      buf[ 1024 ] = "";
		ZnkObjAry repoted = ZnkObjAry_create( NULL );
		MoaiLog_printf("%s : sock=[", label );
		line_leng = Znk_strlen( label ) + 9;
	
		for( i=0; i<sock_ary_size; ++i ){
			sock = ZnkSocketAry_at( sock_ary, i );
			if( MoaiConnection_isListeningSock( sock ) ){
				mcn = NULL;
			} else {
				mcn = findReportingConnection( repoted, sock );
				if( mcn == NULL ){
					continue;
				}
				ZnkObjAry_push_bk( repoted, (ZnkObj)mcn );
			}
	
			if( first_printed ){
				MoaiLog_printf(", ");
				line_leng += 2;
			}
			MoaiIO_makeSockStr( buf, sizeof(buf), sock, detail );
			if( line_leng + Znk_strlen( buf ) > 80 ){
				MoaiLog_printf( "\n    " );
				line_leng = 4;
			}
			MoaiLog_printf( "%s", buf );
			line_leng += Znk_strlen( buf );
			first_printed = true;
		}
		MoaiLog_printf("]\n" );
		ZnkObjAry_destroy( repoted );
	}

}

bool
MoaiIO_procExile( ZnkSocketAry sock_ary, ZnkFdSet fdst, MoaiFdSet mfds )
{
	size_t    i;
	size_t    sock_ary_size;
	ZnkSocket sock;
	MoaiConnection mcn = NULL;
	bool deletion_done = false;

	ZnkSocketAry_clear( sock_ary );
	ZnkFdSet_getSocketAry( fdst, sock_ary );
	sock_ary_size = ZnkSocketAry_size( sock_ary );

	for( i=0; i<sock_ary_size; ++i ){
		sock  = ZnkSocketAry_at( sock_ary, i );
		mcn = MoaiConnection_find_byISock( sock );
		if( mcn ){
			ZnkSocket O_sock = MoaiConnection_O_sock( mcn );
			if( O_sock != ZnkSocket_INVALID ){
				continue;
			}
			/* This is Exile socket */
			if( (uint64_t)getCurrentSec() - mcn->exile_time_ > 15 ){
				MoaiLog_printf( "Moai : Exile[%d] deletion by timeout.\n", sock );
				ZnkSocket_close( sock );
				MoaiConnection_erase( mcn, mfds );
				ZnkFdSet_clr( fdst, sock );
				return deletion_done = true;
			}
		}
	}
	return deletion_done;
}

void
MoaiIO_close_ISock( const char* label, ZnkSocket sock, MoaiFdSet mfds )
{
	char buf[ 1024 ];
	MoaiLog_printf( "  MoaiIO_close_ISock : %s sock=[%s]\n",
			label, MoaiIO_makeSockStr(buf,sizeof(buf),sock,true) );

	if( !MoaiConnection_isListeningSock( sock ) ){
		MoaiConnection mcn = NULL;
		if( (mcn = MoaiConnection_find_byISock( sock )) != NULL ){
			ZnkFdSet fdst_observe_r = MoaiFdSet_fdst_observe_r( mfds );
			ZnkFdSet_clr( fdst_observe_r,sock );
			ZnkSocket_close( sock );
			mcn->I_sock_ = ZnkSocket_INVALID;
		}
	}
}
void
MoaiIO_close_OSock( const char* label, ZnkSocket sock, MoaiFdSet mfds )
{
	char buf[ 1024 ];
	MoaiLog_printf( "  MoaiIO_close_OSock : %s sock=[%s]\n",
			label, MoaiIO_makeSockStr(buf,sizeof(buf),sock,true) );

	if( !MoaiConnection_isListeningSock( sock ) ){
		MoaiConnection mcn = NULL;
		if( (mcn = MoaiConnection_find_byOSock( sock )) != NULL ){
			ZnkFdSet fdst_observe_r = MoaiFdSet_fdst_observe_r( mfds );
			MoaiFdSet_removeConnectingSock( mfds, sock );
			ZnkFdSet_clr( fdst_observe_r, sock );
			ZnkSocket_close( sock );
			mcn->O_sock_ = ZnkSocket_INVALID;
		}
	}
}
#if 0
void
MoaiIO_closeSocket( const char* label, ZnkSocket sock, ZnkFdSet fdst_observe, MoaiFdSet mfds )
{
	/***
	 * �����Ńp�[�g�i�[�ƂȂ�socket���ǂ����邩�Ƃ�����肪����.
	 * �ȉ��ϐ����Ƃ̍���������邽�߁Asock�̒l����̓I��1900�Ƃ��čl����.
	 *
	 * ���1.
	 * ���̂Ƃ��A���Ȃ��Ƃ�cinfo->dst_sock_��1900�ɓ������悤��cinfo��
	 * ����cinfo->dst_sock_�̒l��ZnkSocket_INVALID�ɖ߂��Ă����ׂ��ł���.
	 * �����Ȃ��Ǝ��̃V�i���I������������.
	 * 1. �܂�x(I)<=>1900(O)�Ƃ����ڑ������݂����Ƃ���.
	 * 2. 1900(O)��RecvZero�����m���A1900��close 
	 * 3. ���̂Ƃ�x��cinfo->dst_sock_��1900�̒l�������ɂ��̂܂܎c���Ă����Ƃ���.
	 * 4. I������̐V�����ڑ�y(I)������. ZnkSocket_open�ɂ��\�P�b�g���V�K�쐬���ꂽ�Ƃ���.
	 *    ���̐V�K�쐬�\�P�b�g�̒l�Ƃ��Ē��O��close����1900���ė��p�����󋵂͊��ƕp�ɂɔ���������.
	 * 5. x(I)��cinfo->dst_sock_���z�肷��ڑ���ƁAy(I)��cinfo->dst_sock_���z�肷��ڑ����
	 *    ��ʂɈقȂ�. �ɂ��ւ�炸������dst_sock_�͓���1900�Ƃ����l�ƂȂ��Ă���.
	 *    �܂���I���\�P�b�g�ɑ΂��A���O���\�P�b�g���Ή�����Ƃ����ُ��ԂƂȂ�.
	 * 6. x(I) ��O���Ƃ̐ڑ���������Exile(���Q)Socket�ł��邪�A�܂�I(Browser)����close���Ă��Ȃ�.
	 *    Browser�͂��̃\�P�b�g��V�����ڑ��Ƃ��čė��p���邱�Ƃ�����.
	 *    ���̂Ƃ��Ax(I)��1900�ƒ��p����邪�A�قړ�����y(I)����̐ڑ������������ꍇ:
	 *    6-1. x(I)����̐ڑ���Host��y(I)����̐ڑ���Host�ƈقȂ�ꍇ�A�Е��̒ʐM�悪
	 *         ��������̂ƂȂ��Ă��܂����炩�ɂ܂���.
	 *    6-2. x(I)����̐ڑ���Host��y(I)����̐ڑ���Host�Ɠ����ł���ꍇ�A�ꌩ���Ȃ���������
	 *         ����1900�Ƃ���socket�ɗႦ�Γ��HTTP GET���N�G�X�g�Ȃǂ��قړ����ɑ�����.
	 *         ���������1900����recv����郌�X�|���X�������̃��N�G�X�g��A���������̂Ƃ���
	 *         ����Ԃ���Ă��܂�. �܂����recv�ɂ���Ď擾�����f�[�^观̒���HTTP���X�|���X
	 *         �̊J�n�ʒu����܂܂��Ƃ������悤�Ȉُ�`������M���꓾��.
	 *         ����̓��X�|���X�̏I���̌��m�̉�͏����ɍ����������炷.
	 *
	 * ���2.
	 * �ł́Acinfo->dst_sock_��1900�ɓ������悤��cinfo�ɑΉ�����sock(�܂葊��ƂȂ�sock)��
	 * close�����Ă����ׂ����H
	 * ����͂������肪I��sock�ł���΁Aclose���Ă��悢�������̂��߁A�ڑ����ێ����Ă��悢. 
	 * �������肪O��sock�ł���΂���O��sock��close���ׂ��ł���. �����O��sock���P�Ƀl�b�g���[�N��
	 * �g���t�B�b�N��̗��R�Ȃǂɂ��A��M�Ɏ��Ԃ��������Ēx��Ă����ꍇ�A���̊ԂɐV����I�������
	 * �ڑ����������A���ꂪ�������̂܂��ڑ��������Ă���O��sock��(�ڑ���Host������������Ƃ������Ƃ�)
	 * close�����ɂ��̂܂܍ė��p�����Ƃ���ƁA���̐V�����ڑ��̃��X�|���X�̎�M�ɂ�����
	 * ���O��(���͂≽�̈Ӗ����Ȃ�)�ڑ��̃��X�|���X�̍ŏI�������O���ɘA�����ꂽ��ԂŎ�M�����
	 * �V�i���I���l�����邩��ł���. ����̓��X�|���X�̏I���̌��m�̉�͏����ɍ����������炷.
	 *
	 * ����O���ւ�connectToServer�����͎��Ԃ������邱�Ƃ�����(Blocking Socket�̏ꍇ�͓���)
	 * O���ł����炩�ɑS��M�������������Ƃ��ۏ�ł���󋵂Ɍ���A�ڑ����ێ����Ă��悢�̂ł͂Ȃ���?
	 * �܂肻�̂悤�ȏꍇ�ɂ����Exile Socket�Ƃ��āA����connectToServer�̋@��ɂ����
	 * �ė��p����`�ɂ��č�������}�肽��.
	 */
	char buf[ 1024 ];
	MoaiLog_printf( "%s : close sock=[%s]\n", label, MoaiIO_makeSockStr(buf,sizeof(buf),sock,true) );
	/***
	 * ������sock�ɑ΂���mcn��find���AO_sock���擾�������close����.
	 * I_sock�̐ڑ��ɂ��Ă͘M����Ȃ�.
	 */
#if 0
	if( !MoaiConnection_isListeningSock( sock ) ){
		MoaiConnection mcn = NULL;
		if( (mcn = MoaiConnection_find_byOSock( sock )) != NULL ){
			ZnkSocket I_sock = mcn->I_sock_;
			ZnkSocket O_sock = sock;
			ZnkSocket_close( O_sock );
			MoaiConnection_clear( mcn, mfds );
			mcn->I_sock_ = I_sock;
			ZnkFdSet_clr( fdst_observe, O_sock );
		} else if( (mcn = MoaiConnection_find_byISock( sock )) != NULL ){
			ZnkSocket I_sock = sock;
			ZnkSocket O_sock = MoaiConnection_O_sock( mcn );
			ZnkSocket_close( I_sock );
			ZnkSocket_close( O_sock );
			MoaiConnection_erase( mcn, mfds );
			ZnkFdSet_clr( fdst_observe, I_sock );
			ZnkFdSet_clr( fdst_observe, O_sock );
		} else {
			/* I_sock�ł�O_sock�ł��Ȃ� */
			/* �Ƃ肠�������̏ꍇ�͒P����close����ƂƂ���fdst_observe������m���ɏ������Ă��� */
			ZnkSocket_close( sock );
			ZnkFdSet_clr( fdst_observe, sock );
		}
	}
#endif
}
#endif

/***
 * listening sock�������Afdst_observe���̑Ssock���N���[�Y.
 */
void
MoaiIO_closeSocketAll( const char* label, ZnkFdSet fdst_observe, MoaiFdSet mfds )
{
	size_t    i;
	size_t    sock_ary_size;
	ZnkSocket sock;
	ZnkSocketAry sock_ary = ZnkSocketAry_create();

	ZnkFdSet_getSocketAry( fdst_observe, sock_ary );
	sock_ary_size = ZnkSocketAry_size( sock_ary );

	for( i=0; i<sock_ary_size; ++i ){
		sock = ZnkSocketAry_at( sock_ary, i );
		if( !MoaiConnection_isListeningSock( sock ) ){
			MoaiIO_close_OSock( label, sock, mfds );
			MoaiIO_close_ISock( label, sock, mfds );
		}
	}
	ZnkSocketAry_destroy( sock_ary );
}


bool
MoaiIO_recvInPtn( ZnkBfr stream, ZnkSocket sock, MoaiFdSet mfds, const char* ptn, size_t* result_size )
{
	//ZnkFdSet fdst_observe_r = MoaiFdSet_fdst_observe_r( mfds );
	const char* p = NULL;
	size_t  ptn_leng = Znk_strlen( ptn );
	uint8_t buf[ 4096 ];
	int  recv_size;
	if( result_size ){ *result_size = 0; }
	while( true ){
		p = Znk_memmem( ZnkBfr_data(stream), ZnkBfr_size(stream), ptn, ptn_leng );
		if( p ){
			/* found */
			if( result_size ){
				uint8_t* begin = ZnkBfr_data( stream );
				uint8_t* end   = (uint8_t*)p + ptn_leng;
				*result_size = end - begin;
			}
			break;
		}
		recv_size = ZnkSocket_recv( sock, buf, sizeof(buf) );
		if( recv_size <= 0 ){
			char errmsg_buf[ 4096 ];
			int  err_code = ZnkNetBase_getLastErrCode();
			if( recv_size != 0 ){
				ZnkStr msgs = ZnkStr_new( "" );
				ZnkNetBase_getErrMsg( errmsg_buf, sizeof(errmsg_buf), err_code );
				MoaiIO_addAnalyzeLabel( msgs, sock, recv_size, "recvInPtn" );
				ZnkStr_addf( msgs, "RecvError.\n" );
				ZnkStr_addf( msgs, "  errmsg=[%s]\n", errmsg_buf );
				MoaiLog_printf( "%s", ZnkStr_cstr( msgs ) );
				MoaiIO_close_ISock( "  RecvError", sock, mfds );
				MoaiIO_close_OSock( "  RecvError", sock, mfds );
				ZnkStr_delete( msgs );
			} else {
				MoaiLog_printf( "  recv_size=[%d]\n", recv_size );
				MoaiIO_close_ISock( "  UnexpectedRecvZero(recvInPtn)", sock, mfds );
				MoaiIO_close_OSock( "  UnexpectedRecvZero(recvInPtn)", sock, mfds );
			}
			return false;
		}
		ZnkBfr_append_dfr( stream, buf, recv_size );
	}
	return true;
}
bool
MoaiIO_recvByPtn( ZnkBfr stream, ZnkSocket sock, MoaiFdSet mfds, const char* ptn, size_t* result_size )
{
	//ZnkFdSet fdst_observe_r = MoaiFdSet_fdst_observe_r( mfds );
	const size_t old_bfr_size = ZnkBfr_size(stream);
	const char* p = NULL;
	size_t ptn_leng = Znk_strlen( ptn );
	uint8_t buf[ 4096 ];
	int  recv_size;
	if( result_size ){ *result_size = 0; }
	while( true ){
		p = Znk_memmem( ZnkBfr_data(stream)+old_bfr_size, ZnkBfr_size(stream)-old_bfr_size, ptn, ptn_leng );
		if( p ){
			/* found */
			if( result_size ){
				uint8_t* begin = ZnkBfr_data( stream )+old_bfr_size;
				uint8_t* end   = (uint8_t*)p + ptn_leng;
				*result_size = end - begin;
			}
			break;
		}
		recv_size = ZnkSocket_recv( sock, buf, sizeof(buf) );
		if( recv_size <= 0 ){
			char errmsg_buf[ 4096 ];
			int  err_code = ZnkNetBase_getLastErrCode();
			if( err_code != 0 ){
				ZnkStr msgs = ZnkStr_new( "" );
				ZnkNetBase_getErrMsg( errmsg_buf, sizeof(errmsg_buf), err_code );
				MoaiIO_addAnalyzeLabel( msgs, sock, recv_size, "recvByPtn" );
				ZnkStr_addf( msgs, "RecvError.\n" );
				ZnkStr_addf( msgs, "  errmsg=[%s]\n", errmsg_buf );
				MoaiLog_printf( "%s", ZnkStr_cstr( msgs ) );
				MoaiIO_close_ISock( "  RecvError", sock, mfds );
				MoaiIO_close_OSock( "  RecvError", sock, mfds );
				ZnkStr_delete( msgs );
			} else {
				MoaiIO_close_ISock( "  analyzeRecv : UnexpectedRecvZero(recvByPtn)", sock, mfds );
				MoaiIO_close_OSock( "  analyzeRecv : UnexpectedRecvZero(recvByPtn)", sock, mfds );
			}
			return false;
		}
		ZnkBfr_append_dfr( stream, buf, recv_size );
	}
	return true;
}
bool
MoaiIO_recvBySize( ZnkBfr stream, ZnkSocket sock, MoaiFdSet mfds, size_t size, size_t* result_size )
{
	//ZnkFdSet fdst_observe_r = MoaiFdSet_fdst_observe_r( mfds );
	const size_t old_bfr_size = ZnkBfr_size(stream);
	uint8_t buf[ 4096 ];
	int  recv_size;
	if( result_size ){ *result_size = 0; }
	while( true ){
		if( ZnkBfr_size(stream) - old_bfr_size >= size ){
			break;
		}
		recv_size = ZnkSocket_recv( sock, buf, sizeof(buf) );
		if( recv_size <= 0 ){
			char errmsg_buf[ 4096 ];
			int  err_code = ZnkNetBase_getLastErrCode();
			if( err_code != 0 ){
				ZnkStr msgs = ZnkStr_new( "" );
				ZnkNetBase_getErrMsg( errmsg_buf, sizeof(errmsg_buf), err_code );
				MoaiIO_addAnalyzeLabel( msgs, sock, recv_size, "recvBySize" );
				ZnkStr_addf( msgs, "RecvError.\n" );
				ZnkStr_addf( msgs, "  errmsg=[%s]\n", errmsg_buf );
				MoaiLog_printf( "%s", ZnkStr_cstr( msgs ) );
				MoaiIO_close_ISock( "  RecvError", sock, mfds );
				MoaiIO_close_OSock( "  RecvError", sock, mfds );
				ZnkStr_delete( msgs );
			} else {
				MoaiIO_close_ISock( "  analyzeRecv : UnexpectedRecvZero(recvBySize)", sock, mfds );
				MoaiIO_close_OSock( "  analyzeRecv : UnexpectedRecvZero(recvBySize)", sock, mfds );
			}
			return false;
		}
		ZnkBfr_append_dfr( stream, buf, recv_size );
		if( result_size ){ *result_size += recv_size; }
	}
	return true;
}

bool
MoaiIO_recvByZero( ZnkBfr stream, ZnkSocket sock, MoaiFdSet mfds, size_t* result_size )
{
	//ZnkFdSet fdst_observe_r = MoaiFdSet_fdst_observe_r( mfds );
	uint8_t buf[ 4096 ];
	int  recv_size;
	if( result_size ){ *result_size = 0; }
	while( true ){
		recv_size = ZnkSocket_recv( sock, buf, sizeof(buf) );
		if( recv_size <= 0 ){
			char errmsg_buf[ 4096 ];
			int  err_code = ZnkNetBase_getLastErrCode();
			if( err_code != 0 ){
				ZnkStr msgs = ZnkStr_new( "" );
				ZnkNetBase_getErrMsg( errmsg_buf, sizeof(errmsg_buf), err_code );
				MoaiIO_addAnalyzeLabel( msgs, sock, recv_size, "recvByZero" );
				ZnkStr_addf( msgs, "RecvError.\n" );
				ZnkStr_addf( msgs, "  errmsg=[%s]\n", errmsg_buf );
				MoaiLog_printf( "%s", ZnkStr_cstr( msgs ) );
				MoaiIO_close_ISock( "  RecvError", sock, mfds );
				MoaiIO_close_OSock( "  RecvError", sock, mfds );
				ZnkStr_delete( msgs );
				return false;
			}
			MoaiLog_printf( "MoaiIO_recvByZero : RecvZero. OK.\n" );
			break; /* OK. */
		}
		ZnkBfr_append_dfr( stream, buf, recv_size );
		if( result_size ){ *result_size += recv_size; }
	}
	return true;
}

