#ifndef INCLUDE_GUARD__Znk_net_ip_h__
#define INCLUDE_GUARD__Znk_net_ip_h__

#include <Znk_base.h>

Znk_EXTERN_C_BEGIN

void ZnkNetIP_printTest( void );
bool
ZnkNetIP_getPrivateIP( char* ipaddr, size_t ipaddr_size );

Znk_EXTERN_C_END

#endif /* INCLUDE_GUARD */
