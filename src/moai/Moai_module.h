#ifndef INCLUDE_GUARD__Moai_module_h__
#define INCLUDE_GUARD__Moai_module_h__

#include <Znk_varp_ary.h>
#include <Znk_myf.h>
#include <Znk_txt_filter.h>

Znk_EXTERN_C_BEGIN

typedef struct MoaiModuleImpl* MoaiModule;

typedef bool (*MoaiInitiateFunc)( ZnkMyf ftr_send, const char* parent_proxy, ZnkStr result_msg );
typedef bool (*MoaiOnPostFunc)( ZnkMyf ftr_send, ZnkVarpAry post_vars );
typedef bool (*MoaiOnResponse)( ZnkMyf ftr_send, ZnkVarpAry hdr_vars, ZnkStr text, const char* req_urp );

MoaiModule
MoaiModule_create( void );

void
MoaiModule_destroy( MoaiModule mod );

bool
MoaiModule_load( MoaiModule mod, const char* target_name );

bool
MoaiModule_saveFilter( const MoaiModule mod );

const char*
MoaiModule_target_name( const MoaiModule mod );

ZnkMyf
MoaiModule_ftrSend( const MoaiModule mod );

ZnkTxtFilterAry
MoaiModule_ftrHtml( const MoaiModule mod );
ZnkTxtFilterAry
MoaiModule_ftrJS( const MoaiModule mod );
ZnkTxtFilterAry
MoaiModule_ftrCSS( const MoaiModule mod );

bool
MoaiModule_invokeInitiate( const MoaiModule mod, const char* parent_proxy, ZnkStr result_msg );
bool
MoaiModule_invokeOnPost( const MoaiModule mod, ZnkVarpAry post_vars );
bool
MoaiModule_invokeOnResponse( const MoaiModule mod, ZnkVarpAry hdr_vars, ZnkStr text, const char* req_urp );

/**
 * @brief
 * �w�b�_�ϐ��̔z�� hdr_vars �� �t�B���^ ftr_send �Ńt�B���^�����O����.
 */
size_t
MoaiModule_filtHtpHeader( const MoaiModule mod, ZnkVarpAry hdr_vars );

/**
 * @brief
 * POST�ϐ��̔z�� post_vars �� �t�B���^ ftr_send �Ńt�B���^�����O����.
 */
size_t
MoaiModule_filtPostVars( const MoaiModule mod, ZnkVarpAry post_vars );

/**
 * @brief
 * �w�b�_�ϐ��̔z�� hdr_vars ����Cookie�t�B�[���h�����݂���ꍇ�́A
 * ����� �t�B���^ ftr_send �Ńt�B���^�����O����.
 */
bool
MoaiModule_filtCookieVars( const MoaiModule mod, ZnkVarpAry hdr_vars );

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
