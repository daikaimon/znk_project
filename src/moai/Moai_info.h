#ifndef INCLUDE_GUARD__Moai_info_h__
#define INCLUDE_GUARD__Moai_info_h__

#include <Znk_str.h>
#include <Znk_str_dary.h>
#include <Znk_var.h>
#include <Znk_htp_hdrs.h>
#include "Moai_module_ary.h"
#include <assert.h>

Znk_EXTERN_C_BEGIN

typedef enum {
	 MoaiText_e_Binary=0
	,MoaiText_e_HTML
	,MoaiText_e_JS
	,MoaiText_e_CSS
} MoaiTextType;

typedef struct {
	struct ZnkHtpHdrs_tag hdrs_;
	ZnkVarpDAry           vars_;   /* for POST */
	ZnkBfr                stream_; /* ����M�̍ۂɎ��ۂɎg����X�g���[�� */
	size_t                hdr_size_;
	const char*           proxy_hostname_;
	uint16_t              proxy_port_;
} MoaiInfo;

Znk_INLINE void
MoaiInfo_clear( MoaiInfo* info )
{
	ZnkHtpHdrs_clear( &info->hdrs_ );
	ZnkVarpDAry_clear( info->vars_ );
	ZnkBfr_clear( info->stream_ );
	info->hdr_size_ = 0;
}

typedef struct {
	bool    is_chunked_;
	bool    is_gzip_;
	bool    is_unlimited_;
	size_t  content_length_;
	MoaiTextType txt_type_;
} MoaiBodyInfo;

Znk_INLINE void
MoaiBodyInfo_clear( MoaiBodyInfo* body_info )
{
	MoaiBodyInfo zero = { 0 };
	*body_info = zero;
}

/**
 * @param is_proxy:
 *   ���̃v���O�����̒��p�悪����Ƃ͕ʂ�proxy�T�[�o�ł��邩�ۂ����w��.
 *   true�ł���ꍇ�A���̃v���O�����͒��p������ɕʂ�proxy�Ɉ����p��.
 *   false�ł���ꍇ�A���̃v���O�����͍ŏI�I�Ȓ��p��Ƀ_�C���N�g�ɒ��p����.
 *
 * @param as_local_proxy:
 *   ���̊֐��́A���̃v���O������LocalProxy�Ƃ��Ďg�p����Ă��邩�ۂ���
 *   �������肷��. ���̔��茋�ʂ��i�[�����.
 *
 * @param my_port:
 *   ���̃v���O�������g���Ă���port�ԍ����w�肷��.
 */
void
MoaiInfo_parseHdr( MoaiInfo* info, MoaiBodyInfo* body_info,
		ZnkMyf config, bool* as_local_proxy, uint16_t my_port, bool is_request,
		const char* response_hostname,
		MoaiModuleAry mod_ary, ZnkMyf mtgt, ZnkStr req_urp );

Znk_EXTERN_C_END

#endif /* INCLUDE_GUARD */
