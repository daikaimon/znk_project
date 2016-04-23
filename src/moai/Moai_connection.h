#ifndef INCLUDE_GUARD__Moai_connection_h__
#define INCLUDE_GUARD__Moai_connection_h__

#include <Znk_socket.h>
#include <Znk_htp_hdrs.h>

Znk_EXTERN_C_BEGIN

/* DNS�֘A��RFC�ł���RFC1035�ɂ��΁A�h���C�����͍ő�ł�255�I�N�e�b�g�ȉ��Ƃ���.
 * �]���āA�����ł�hostname�̃o�b�t�@�T�C�Y�Ƃ���256����Ώ\�� */
#define MOAI_HOSTNAME_MAX 256  
/* IPV6���܂߂��64������Ώ\�� */
#define MOAI_IPADDR_MAX 64  

typedef enum {
	 MoaiSockType_e_None=0 /* ���ڑ���ԂȂ� */
	,MoaiSockType_e_Listen /* ���X�j���O�\�P�b�g */
	,MoaiSockType_e_Outer  /* �O���T�C�g�ւ̐ڑ��\�P�b�g */
	,MoaiSockType_e_Inner  /* ���[�J���z�X�g���̃u���E�U�����Accept�\�P�b�g */
} MoaiSockType;

typedef struct MoaiConnection_tag* MoaiConnection;
struct MoaiConnection_tag {
	char            hostname_[ MOAI_HOSTNAME_MAX ]; /* O���z�X�g�� */
	//char            ipaddr_[ MOAI_IPADDR_MAX ];     /* O��IP �A�h���X */
	uint16_t        port_;                          /* O���Ƃ̒ʐM�ɗp����port */
	ZnkSocket       src_sock_;      /* key�ƂȂ�sock */
	MoaiSockType    sock_type_;     /* src_sock�̃^�C�v */
	bool            is_keep_alive_; /* src_sock��keep-alive�ł��邩�ۂ� */
	ZnkSocket       dst_sock_;      /* tunneling���[�h�ł���ꍇ��dst_sock */
	size_t          content_length_remain_;
	bool            as_local_proxy_;
	//ZnkStr          req_uri_;
	ZnkStr          req_urp_; /* Request URL Path : URL�ɂ�����I�[�\���e�B�̏I���ȍ~��/�Ŏn�܂镔�������� */
	ZnkHtpReqMethod req_method_;
	uint64_t        exile_time_;
	uint64_t        waiting_;
	/* TODO:timeout����Ǝ����I��close���邽�߂̏����ǉ������� */
};

void
MoaiConnection_initiate( void );

void
MoaiConnection_clear( MoaiConnection htcn );

MoaiConnection
MoaiConnection_find( const ZnkSocket query_sock );
MoaiConnection
MoaiConnection_find_dst_sock( const ZnkSocket query_sock );

MoaiConnection
MoaiConnection_intern( ZnkSocket sock, MoaiSockType sock_type );

void
MoaiConnection_clear_bySock( ZnkSocket sock );

void
MoaiConnection_clearAll( void );

Znk_EXTERN_C_END

#endif /* INCLUDE_GUARD */
