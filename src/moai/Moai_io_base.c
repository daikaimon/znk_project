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
	strftime( buf, buf_size, "%a, %d %b %Y %T GMT", timePtr );
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
getCurrentSec( void ){ return ( 1.0 / CLOCKS_PER_SEC ) * clock(); }

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
	MoaiConnection cinfo  = MoaiConnection_find( sock );
	bool broken_link = false;
	if( cinfo ){
		if( detail
		  && cinfo->sock_type_ != MoaiSockType_e_Listen
		  && cinfo->sock_type_ != MoaiSockType_e_None
		  && !ZnkS_empty(cinfo->hostname_) )
		{
			MoaiConnection I_cnct = NULL;
			MoaiConnection O_cnct = NULL;
			ZnkSocket I_sock = ZnkSocket_INVALID;
			ZnkSocket O_sock = ZnkSocket_INVALID;
			char I_waiting = '?';
			char O_waiting = '?';
			switch( cinfo->sock_type_ ){
			case MoaiSockType_e_Inner:
			{
				I_cnct    = cinfo;
				I_sock    = I_cnct->src_sock_;
				I_waiting = I_cnct->waiting_ ? 'w' : ' ';
				O_cnct    = MoaiConnection_find( I_cnct->dst_sock_ );
				if( O_cnct ){
					broken_link = (bool)( O_cnct->src_sock_ != I_cnct->dst_sock_ );
					O_sock    = O_cnct->src_sock_;
					O_waiting = O_cnct->waiting_ ? 'w' : ' ';
				}
				break;
			}
			case MoaiSockType_e_Outer:
			{
				O_cnct    = cinfo;
				O_sock    = O_cnct->src_sock_;
				O_waiting = O_cnct->waiting_ ? 'w' : ' ';
				I_cnct    = MoaiConnection_find( O_cnct->dst_sock_ );
				if( I_cnct ){
					broken_link = (bool)( O_cnct->src_sock_ != I_cnct->dst_sock_ );
					I_sock    = I_cnct->src_sock_;
					I_waiting = I_cnct->waiting_ ? 'w' : ' ';
				}
				break;
			}
			default:
				assert( 0 );
				break;
			}

			if( cinfo->port_ == 80 ){
				Znk_snprintf( buf, buf_size, "%s(%s%d%c=>%d%c)", cinfo->hostname_,
						broken_link ? "Broken:" : "",
						I_sock, I_waiting, O_sock, O_waiting );
			} else {
				Znk_snprintf( buf, buf_size, "%s:%d(%s%d%c=>%d%c)", cinfo->hostname_, cinfo->port_,
						broken_link ? "Broken:" : "",
						I_sock, I_waiting, O_sock, O_waiting );
			}
		} else {
			Znk_snprintf( buf, buf_size, "%d(%c)", sock, getSockTypeCh( cinfo->sock_type_ ) );
		}
	} else {
		Znk_snprintf( buf, buf_size, "%d(N)", sock );
	}
	return buf;
}
void
MoaiIO_addAnalyzeLabel( ZnkStr msgs, ZnkSocket sock, int result_size )
{
	char scbuf[ 1024 ];
	ZnkStr_addf( msgs, "  AnalyzeRecv : sock=[%s] result_size=[%d] : ",
			MoaiIO_makeSockStr( scbuf, sizeof(scbuf), sock, true ),result_size );
}

void
MoaiIO_reportFdst( const char* label, ZnkSocketDAry sock_dary, ZnkFdSet fdst, bool detail )
{
	size_t    i;
	size_t    sock_dary_size;
	ZnkSocket sock;
	char      buf[ 1024 ] = "";
	ZnkSocketDAry repoted = ZnkSocketDAry_create();
	MoaiConnection cinfo = NULL;
	size_t line_leng = 0;
	bool first_printed = false;

	ZnkSocketDAry_clear( sock_dary );
	ZnkFdSet_getSocketDAry( fdst, sock_dary );
	sock_dary_size = ZnkSocketDAry_size( sock_dary );

	MoaiLog_printf("%s : sock=[", label );
	line_leng = Znk_strlen( label ) + 9;

	for( i=0; i<sock_dary_size; ++i ){
		sock  = ZnkSocketDAry_at( sock_dary, i );
		if( ZnkSocketDAry_find( repoted, sock ) != Znk_NPOS ){
			continue;
		}
		ZnkSocketDAry_push_bk( repoted, sock );

		cinfo = MoaiConnection_find( sock );
		if( cinfo ){
			if( ZnkSocketDAry_find( repoted, cinfo->dst_sock_ ) == Znk_NPOS ){
				if( ZnkSocketDAry_find( sock_dary, cinfo->dst_sock_ ) != Znk_NPOS ){
					/* sock_dary�ɂ��m���ɑ��݂��� */
					ZnkSocketDAry_push_bk( repoted, cinfo->dst_sock_ );
				} else {
					/***
					 * cinfo->src_sock_��Observe����Ă���ɂ��ւ�炸
					 * cinfo->dst_sock_��Observe����Ă��Ȃ�.
					 * ���̏ꍇ��cinfo->dst_sock_�͗\�񂳂�Ă���\��������.
					 */
				}
			} else {
				/***
				 * �ُ�:
				 * cinfo->dst_sock_��reported�ς݂Ȃ�A�{��cinfo->src_sock_��reported����Ă���A
				 * ���̏����͂����I�����Ă���͂�.
				 * �]���āA�����ɗ����ꍇ�Adst_sock_��src_sock_�̑��ݎQ�Ƃ����Ă���.
				 * MoaiIO_makeSockStr�͂��̏ꍇ���ʂȕ\��������悤�ɂȂ��Ă���̂ŁA
				 * �Ƃ肠�����\���͂���ɔC����.
				 */
			}
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

	ZnkSocketDAry_destroy( repoted );
}

bool
MoaiIO_procExile( ZnkSocketDAry sock_dary, ZnkFdSet fdst )
{
	size_t    i;
	size_t    sock_dary_size;
	ZnkSocket sock;
	MoaiConnection cinfo = NULL;
	bool deletion_done = false;

	ZnkSocketDAry_clear( sock_dary );
	ZnkFdSet_getSocketDAry( fdst, sock_dary );
	sock_dary_size = ZnkSocketDAry_size( sock_dary );

	for( i=0; i<sock_dary_size; ++i ){
		sock  = ZnkSocketDAry_at( sock_dary, i );
		cinfo = MoaiConnection_find( sock );
		if( cinfo ){
			/***
			 * TODO:
			 * Timeout�����Ƃ��Ĉȉ��̂悤�Ȉ��ՂȎ����ł͂܂���.
			 * ���ꂾ�Ɠ���T�C�g�ȂǂŒ����Ԃ̃X�g���[�~���O�Đ������鎞��
			 * �r����Timeout�������A�K�v�ȃR�l�N�V������close����Ă��܂�.
			 * �����ƃ}�V�ȕ��@����������K�v������.
			 */
#if 0
			if( cinfo->waiting_ ){
				static const int st_wait_sec_max = 480;
				if( (uint64_t)getCurrentSec() - cinfo->waiting_ > st_wait_sec_max ){
					/***
					 * �����ꂩ����ő҂����Ԃ̌��E�����������ꍇ
					 * �o���Ƃ��ɕ��A�����S�N���A����.
					 */
					MoaiConnection dst_cnct = MoaiConnection_find( cinfo->dst_sock_ );
					ZnkSocket_close( cinfo->src_sock_ );
					ZnkSocket_close( dst_cnct->src_sock_ );
					ZnkFdSet_clr( fdst, cinfo->src_sock_ );
					ZnkFdSet_clr( fdst, dst_cnct->src_sock_ );
					MoaiConnection_clear( cinfo );
					MoaiConnection_clear( dst_cnct );
					MoaiLog_printf( "Moai : Waiting[%d] deletion by timeout.\n", sock );
					return deletion_done = true;
				}
			}
#endif
			if( cinfo->sock_type_ != MoaiSockType_e_Inner ){
				continue;
			}
			if( cinfo->dst_sock_ != ZnkSocket_INVALID ){
				continue;
			}
			/* This is Exile socket */
			if( (uint64_t)getCurrentSec() - cinfo->exile_time_ > 15 ){
				MoaiLog_printf( "Moai : Exile[%d] deletion by timeout.\n", sock );
				ZnkSocket_close( sock );
				MoaiConnection_clear( cinfo );
				ZnkFdSet_clr( fdst, sock );
				return deletion_done = true;
			}
		}
	}
	return deletion_done;
}

void
MoaiIO_closeSocket( const char* label, ZnkSocket sock, ZnkFdSet fdst_observe )
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
	ZnkSocket_close( sock );
	/***
	 * �����ł܂�sock�ɑ΂���cinfo��find���Acinfo->dst_sock_���擾���A
	 * cinfo->dst_sock_��(O)���ł���΂����close����.
	 * cinfo->dst_sock_��(I)���ł���΂���̐ڑ��͈ێ������܂܂ɂ��邪�A
	 * cinfo->dst_sock_�ɑ΂���rinfo��find���Ă����rinfo->dst_sock_�̒l��ZnkSocket_INVALID�ɂ��Ă����ׂ�.
	 */
	{
		MoaiConnection cinfo = MoaiConnection_find( sock );
		if( cinfo ){
			ZnkSocket dst_sock = cinfo->dst_sock_;
			if( dst_sock != ZnkSocket_INVALID ){
				MoaiConnection rinfo = MoaiConnection_find( dst_sock );
				if( rinfo->sock_type_ == MoaiSockType_e_Outer || rinfo->sock_type_ == MoaiSockType_e_None ){
					/* close������Ɋ��S���� */
					ZnkSocket_close( dst_sock );
					MoaiConnection_clear( rinfo );
					ZnkFdSet_clr( fdst_observe, dst_sock );
				} else if( rinfo->sock_type_ == MoaiSockType_e_Inner ){
					/* dst_sock��Exile������ */
					rinfo->dst_sock_ = ZnkSocket_INVALID;
					//rinfo->is_tunneling_ = false;
				}
			}
			/***
			 * �����cinfo->dst_sock_�̒l��sock�Ɠ������悤��cinfo��S�������āA
			 * ������dst_sock_�̒l��ZnkSocket_INVALID�ɏC������悤�ȏ�����
			 * �t�������������悢�̂�...?
			 */
		}
	}
	MoaiConnection_clear_bySock( sock );
	ZnkFdSet_clr( fdst_observe, sock );
}

/***
 * listening sock�������Afdst_observe���̑Ssock���N���[�Y.
 */
void
MoaiIO_closeSocketAll( const char* label, ZnkFdSet fdst_observe )
{
	size_t    i;
	size_t    sock_dary_size;
	ZnkSocket sock;
	ZnkSocketDAry sock_dary = ZnkSocketDAry_create();
	MoaiConnection cinfo;

	ZnkFdSet_getSocketDAry( fdst_observe, sock_dary );
	sock_dary_size = ZnkSocketDAry_size( sock_dary );

	for( i=0; i<sock_dary_size; ++i ){
		sock = ZnkSocketDAry_at( sock_dary, i );
		cinfo = MoaiConnection_find( sock );
		if( cinfo && cinfo->sock_type_ != MoaiSockType_e_Listen ){
			MoaiIO_closeSocket( label, sock, fdst_observe );
		}
	}
	ZnkSocketDAry_destroy( sock_dary );
}


bool
MoaiIO_recvInPtn( ZnkBfr stream, ZnkSocket sock, ZnkFdSet fdst_observe, const char* ptn, size_t* result_size )
{
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
			if( err_code != 0 ){
				ZnkStr msgs = ZnkStr_new( "" );
				ZnkNetBase_getErrMsg( errmsg_buf, sizeof(errmsg_buf), err_code );
				MoaiIO_addAnalyzeLabel( msgs, sock, recv_size );
				ZnkStr_addf( msgs, "RecvError.\n" );
				ZnkStr_addf( msgs, "  errmsg=[%s]\n", errmsg_buf );
				MoaiLog_printf( "%s", ZnkStr_cstr( msgs ) );
				MoaiIO_closeSocket( "  RecvError", sock, fdst_observe );
				ZnkStr_delete( msgs );
			} else {
				MoaiLog_printf( "  recv_size=[%d]\n", recv_size );
				MoaiIO_closeSocket( "  UnexpectedRecvZero(recvInPtn)", sock, fdst_observe );
			}
			return false;
		}
		ZnkBfr_append_dfr( stream, buf, recv_size );
	}
	return true;
}
bool
MoaiIO_recvByPtn( ZnkBfr stream, ZnkSocket sock, ZnkFdSet fdst_observe, const char* ptn, size_t* result_size )
{
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
				MoaiIO_addAnalyzeLabel( msgs, sock, recv_size );
				ZnkStr_addf( msgs, "RecvError.\n" );
				ZnkStr_addf( msgs, "  errmsg=[%s]\n", errmsg_buf );
				MoaiLog_printf( "%s", ZnkStr_cstr( msgs ) );
				MoaiIO_closeSocket( "  RecvError", sock, fdst_observe );
				ZnkStr_delete( msgs );
			} else {
				MoaiIO_closeSocket( "  analyzeRecv : UnexpectedRecvZero(recvByPtn)", sock, fdst_observe );
			}
			return false;
		}
		ZnkBfr_append_dfr( stream, buf, recv_size );
	}
	return true;
}
bool
MoaiIO_recvBySize( ZnkBfr stream, ZnkSocket sock, ZnkFdSet fdst_observe, size_t size, size_t* result_size )
{
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
				MoaiIO_addAnalyzeLabel( msgs, sock, recv_size );
				ZnkStr_addf( msgs, "RecvError.\n" );
				ZnkStr_addf( msgs, "  errmsg=[%s]\n", errmsg_buf );
				MoaiLog_printf( "%s", ZnkStr_cstr( msgs ) );
				MoaiIO_closeSocket( "  RecvError", sock, fdst_observe );
				ZnkStr_delete( msgs );
			} else {
				MoaiIO_closeSocket( "  analyzeRecv : UnexpectedRecvZero(recvBySize)", sock, fdst_observe );
			}
			return false;
		}
		ZnkBfr_append_dfr( stream, buf, recv_size );
		if( result_size ){ *result_size += recv_size; }
	}
	return true;
}

bool
MoaiIO_recvByZero( ZnkBfr stream, ZnkSocket sock, ZnkFdSet fdst_observe, size_t* result_size )
{
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
				MoaiIO_addAnalyzeLabel( msgs, sock, recv_size );
				ZnkStr_addf( msgs, "RecvError.\n" );
				ZnkStr_addf( msgs, "  errmsg=[%s]\n", errmsg_buf );
				MoaiLog_printf( "%s", ZnkStr_cstr( msgs ) );
				MoaiIO_closeSocket( "  RecvError", sock, fdst_observe );
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

