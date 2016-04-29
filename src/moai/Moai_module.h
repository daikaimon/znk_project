#ifndef INCLUDE_GUARD__Moai_module_h__
#define INCLUDE_GUARD__Moai_module_h__

#include <Znk_varp_dary.h>
#include <Znk_myf.h>
#include <Znk_txt_filter.h>

Znk_EXTERN_C_BEGIN

typedef struct MoaiModuleImpl* MoaiModule;

typedef bool (*MoaiOnInitFunc)( ZnkMyf myf, const char* proxy_hostname, const char* proxy_port );
typedef bool (*MoaiOnPostFunc)( ZnkMyf ftr_send );
typedef bool (*MoaiOnResponseHdr)( ZnkMyf ftr_send, ZnkVarpDAry hdr_vars );

MoaiModule
MoaiModule_create( void );

void
MoaiModule_destroy( MoaiModule mod );

bool
MoaiModule_load( MoaiModule mod, const char* target_name );

bool
MoaiModule_saveFilter( const MoaiModule mod );

ZnkMyf
MoaiModule_ftrSend( const MoaiModule mod );

ZnkTxtFilterAry
MoaiModule_ftrHtml( const MoaiModule mod );
ZnkTxtFilterAry
MoaiModule_ftrJS( const MoaiModule mod );
ZnkTxtFilterAry
MoaiModule_ftrCSS( const MoaiModule mod );

bool
MoaiModule_invokeOnInit( const MoaiModule mod, const char* parent_proxy_hostname, const char* parent_proxy_port );
bool
MoaiModule_invokeOnPostBefore( const MoaiModule mod );
bool
MoaiModule_invokeOnResponseHdr( const MoaiModule mod, ZnkVarpDAry hdr_vars );

/**
 * @brief
 * �w�b�_�ϐ��̔z�� hdr_vars �� �t�B���^ ftr_send �Ńt�B���^�����O����.
 */
size_t
MoaiModule_filtHtpHeader( const MoaiModule mod, ZnkVarpDAry hdr_vars );

/**
 * @brief
 * POST�ϐ��̔z�� post_vars �� �t�B���^ ftr_send �Ńt�B���^�����O����.
 */
size_t
MoaiModule_filtPostVars( const MoaiModule mod, ZnkVarpDAry post_vars );

/**
 * @brief
 * �w�b�_�ϐ��̔z�� hdr_vars ����Cookie�t�B�[���h�����݂���ꍇ�́A
 * ����� �t�B���^ ftr_send �Ńt�B���^�����O����.
 */
bool
MoaiModule_filtCookieVars( const MoaiModule mod, ZnkVarpDAry hdr_vars );

/**
 * @brief
 * �w�肵�� hostname ������MoaiModule�̏����Ώۂł��邩�ۂ���Ԃ�.
 */
bool
MoaiModule_isTargetHost( const MoaiModule mod, const ZnkMyf mtgt, const char* hostname );


const char*
MoaiTarget_findTargetName( const ZnkMyf mtgt, const char* hostname );

Znk_EXTERN_C_END

#endif /* INCLUDE_GUARD */
