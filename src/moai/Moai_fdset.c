#include "Moai_fdset.h"
#include "Moai_log.h"
#include <Znk_fdset.h>
#include <Znk_socket_dary.h>
#include <assert.h>

struct MoaiFdSetImpl {
	ZnkSocket         listen_sock_;
	ZnkFdSet          fdst_read_;
	ZnkFdSet          fdst_observe_;
	ZnkSocketDAry     reserve_accept_socks_;
	ZnkSocketDAry     reserve_connect_socks_;
	ZnkSocketDAry     wk_sock_dary_;
	struct ZnkTimeval waitval_;
};

MoaiFdSet
MoaiFdSet_create( ZnkSocket listen_sock, struct ZnkTimeval* waitval )
{
	MoaiFdSet mfds = Znk_malloc( sizeof( struct MoaiFdSetImpl ) );
	mfds->listen_sock_  = listen_sock;
	mfds->fdst_read_    = ZnkFdSet_create();

	mfds->fdst_observe_ = ZnkFdSet_create();
	ZnkFdSet_zero( mfds->fdst_observe_ );
	/* ���X�j���O�\�P�b�g���Ď��Ώۂɒǉ� */
	ZnkFdSet_set( mfds->fdst_observe_, listen_sock );

	mfds->reserve_accept_socks_  = ZnkSocketDAry_create();
	mfds->reserve_connect_socks_ = ZnkSocketDAry_create();
	mfds->wk_sock_dary_          = ZnkSocketDAry_create();

	mfds->waitval_ = *waitval;
	return mfds;
}
void
MoaiFdSet_destroy( MoaiFdSet mfds )
{
	if( mfds ){
		ZnkFdSet_destroy( mfds->fdst_read_ );
		ZnkFdSet_destroy( mfds->fdst_observe_ );
		ZnkSocketDAry_destroy( mfds->reserve_accept_socks_ );
		ZnkSocketDAry_destroy( mfds->reserve_connect_socks_ );
		ZnkSocketDAry_destroy( mfds->wk_sock_dary_ );
		Znk_free( mfds );
	}
}

ZnkFdSet
MoaiFdSet_fdst_observe( MoaiFdSet mfds )
{
	return mfds->fdst_observe_;
}
ZnkSocketDAry
MoaiFdSet_wk_sock_dary( MoaiFdSet mfds )
{
	return mfds->wk_sock_dary_;
}
void
MoaiFdSet_reserveConnectSock( MoaiFdSet mfds, ZnkSocket sock )
{
	size_t idx = ZnkSocketDAry_find( mfds->reserve_connect_socks_, sock );
	if( idx == Znk_NPOS ){
		ZnkSocketDAry_push_bk( mfds->reserve_connect_socks_, sock );
	}
}

int
MoaiFdSet_select( MoaiFdSet mfds, bool* req_before_report, MoaiFdSetFuncArg_Report* fnca_report )
{
	ZnkFdSet fdst_read    = mfds->fdst_read_;
	ZnkFdSet fdst_observe = mfds->fdst_observe_;
	ZnkSocket maxfd;
	int sel_ret;

	/* fds_observe �� fds_read �ɃR�s�[ */
	ZnkFdSet_copy( fdst_read, fdst_observe );
	
	maxfd = ZnkFdSet_getMaxOfSocket( fdst_read );

	/***
	 * ��1����:
	 *   Unix�ł�select�̑�1������fdst_observe�Ɏw�肵�Ă���Ssock�̂�����
	 *   �ő�l+1�ȏ�̒l���w�肷��. ������sock�l��FD_SETSIZE�𒴂��Ȃ����Ƃ��ۏႳ��Ă���̂�
	 *   �蔲�����������ꍇ�͂�����FD_SETSIZE���w�肵�Ă��悢.
	 *
	 *   Windows�ł�select�̑�1�����͎g���Ă��Ȃ����߁A���̂Ƃ���Ȃ�ł��悢.
	 *
	 * ��2����:
	 *   fdst_read�͂����œ��͂Əo�̗͂��������˔����Ă���.
	 *   �Ď��ΏۂƂ��������ׂĂ�socket�̏W����fdst_read�ɃZ�b�g���Ă���.
	 *   ���ꂪ���͂ƂȂ�.
	 *   select���琧�䂩�A���ė����Ƃ��Afdst_read�ɂ͕ω��̔�������socket�̏W���݂̂�
	 *   �i�[����Ă���(����͍ŏ��ɗ^����fdst_read�̕����W���ł���).
	 *   ���ꂪ�o�͂ƂȂ�.
	 *   ��������΁A�ω��̔������Ă��Ȃ�socket�Q�͌��̏W�����獷��������Ă���.
	 *   �ȉ��ł͂���fdst_read�̌��ʂ𒲍�����΂悢.
	 *
	 */
	if( *req_before_report ){
		//ZnkSocketDAry_clear( sock_dary );
		//ZnkFdSet_getSocketDAry( fdst_read, sock_dary );
		*req_before_report = false;

		//MoaiIO_reportFdst( "Moai : Observe ", mfds->wk_sock_dary_, mfds->fdst_observe_, true );
		if( fnca_report ){
			fnca_report->func_( mfds, fnca_report->arg_ );
		}
	}
	sel_ret = ZnkFdSet_select( maxfd+1, fdst_read, NULL, NULL, &mfds->waitval_ );
	/***
	 * ����������after_report�n�̊֐����T�|�[�g���邩������Ȃ�.
	 */
	return sel_ret;
}

static void
adoptReserveSocks( ZnkFdSet fdst_observe, ZnkSocketDAry reserve_socks, ZnkSocketDAry wk_sock_dary )
{
	size_t idx;
	size_t size;
	ZnkSocket sock;
	bool updated = false;
	size = ZnkSocketDAry_size( reserve_socks );
	for( idx=0; idx<size; ++idx ){ 
		sock = ZnkSocketDAry_at( reserve_socks, idx );
		if( ZnkFdSet_set( fdst_observe, sock ) ){
			/* marking erase */
			updated = true;
			MoaiLog_printf( "  Moai : adoptReserveSocks [%d] under fdst_observe.\n", sock );
			ZnkSocketDAry_set( reserve_socks, idx, ZnkSocket_INVALID );
		}
	}
	/* compaction */
	if( updated ){
		ZnkSocketDAry_clear( wk_sock_dary );
		for( idx=0; idx<size; ++idx ){ 
			sock = ZnkSocketDAry_at( reserve_socks, idx );
			if( sock != ZnkSocket_INVALID ){
				ZnkSocketDAry_push_bk( wk_sock_dary, sock );
			}
		}
		ZnkSocketDAry_swap( wk_sock_dary, reserve_socks );
	}
}

/***
 * ZnkSocketDAry���g���o�[�W����.
 * Windows/Unix�̑o���Ŏg�p�\�ł���.
 */
void
MoaiFdSet_process( MoaiFdSet mfds,
		MoaiFdSetFuncArg_OnAccept* fnca_on_accept, MoaiFdSetFuncArg_RAS* fnca_ras )
{
	ZnkFdSet fdst_read    = mfds->fdst_read_;
	ZnkFdSet fdst_observe = mfds->fdst_observe_;
	ZnkSocketDAry wk_sock_dary = mfds->wk_sock_dary_;

	if( ZnkFdSet_isset( fdst_read, mfds->listen_sock_ ) ){
		/***
		 * fdst_read��ListenSocket���c���Ă����ꍇ�́A
		 * ListenSocket�ɐV�K�ڑ��v���������������Ƃ�����.
		 * ���̏ꍇ�A�܂������D��I�ɏ�������.
		 */
		ZnkSocket new_accept_sock = ZnkSocket_accept( mfds->listen_sock_ );

		if( fnca_on_accept ){
			fnca_on_accept->func_( mfds, new_accept_sock, fnca_on_accept->arg_ );
		}

		/***
		 * �Ď��\�P�b�g�W���֒ǉ�.
		 */
		if( !ZnkFdSet_set( fdst_observe, new_accept_sock ) ){
			/***
			 * ���s�����ꍇ��reserve_accept_socks_�փX�g�b�N���Ă���.
			 */
			MoaiLog_printf( "  ZnkFdSet_set failure. sock(by accept)=[%d]\n", new_accept_sock );
			ZnkSocketDAry_push_bk( mfds->reserve_accept_socks_, new_accept_sock ); 
		}
		/***
		 * listen_sock�Ɋւ��鏈���͏I�����̂ŁA
		 * fdst_read���炱��͏���.
		 */
		ZnkFdSet_clr( fdst_read, mfds->listen_sock_ );
	}

	{
		/***
		 * ���̎��_��fdst_read�Ɋi�[����Ă���̂́A���ɃN���C�A���g�Ƃ�
		 * �ڑ��ς�Socket�݂̂ł���͂��ł���.
		 * �����ł͂�������o���Arecv/send�����Ȃǂ��s��.
		 */
		size_t i;
		size_t sock_dary_size;
		const size_t fd_setsize = ZnkFdSet_FD_SETSIZE();

		ZnkSocketDAry_clear( wk_sock_dary );
		ZnkFdSet_getSocketDAry( fdst_read, wk_sock_dary );
		sock_dary_size = ZnkSocketDAry_size( wk_sock_dary );

		Znk_UNUSED( fd_setsize );
		assert( sock_dary_size <= fd_setsize );

		for( i=0; i<sock_dary_size; ++i ){
			ZnkSocket  sock = ZnkSocketDAry_at( wk_sock_dary, i );
			if( fnca_ras ){
				fnca_ras->func_( mfds, sock, fnca_ras->arg_ );
			}
		}
	}

	/***
	 * �Ō�� reserve_accept_socks_ ����� reserve_connect_socks_ �������ł��Ȃ���������.
	 * (fnca_ras->func_�̌Ăяo����fdst_observe�ɋ󂫂��������\��������)
	 */
	adoptReserveSocks( mfds->fdst_observe_, mfds->reserve_accept_socks_, mfds->wk_sock_dary_ );
	adoptReserveSocks( mfds->fdst_observe_, mfds->reserve_connect_socks_, mfds->wk_sock_dary_ );
}
