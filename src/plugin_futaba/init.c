#include <Moai_plugin_dev.h>
#include <Znk_stdc.h>
#include <Znk_def_util.h>
#include <Znk_zlib.h>
#include <Znk_missing_libc.h>
#include <Znk_net_base.h>
#include <Znk_s_base.h>
#include <Znk_str.h>
#include <Znk_str_fio.h>
#include <Znk_str_ptn.h>
#include <Znk_cookie.h>
#include <Znk_myf.h>
#include <Znk_dir.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>
#include <limits.h>
#include <time.h>


typedef enum {
	 FtbSrvType_e_Unknown=0
	,FtbSrvType_e_may
	,FtbSrvType_e_jun
	,FtbSrvType_e_dec
	,FtbSrvType_e_img
	,FtbSrvType_e_cgi
} SrvType;

static bool st_is_dos_path = false;

static unsigned int
getRandomUInt( void )
{
	return (unsigned int)( rand() * UINT_MAX );
}

static bool
decorateHeaderGET( const char* hostname, const char* request_uri,
		const char* ua, const char* referer, ZnkVarpAry cookie )
{
	const char* tmpdir = st_is_dos_path ? ".\\tmp\\" : "./tmp/";
	ZnkFile fp = NULL;
	char http_hdr_file[ 256 ] = "";
	Znk_snprintf( http_hdr_file, sizeof(http_hdr_file), "%shttp_hdr.dat", tmpdir );
	fp = ZnkF_fopen( http_hdr_file, "wb" );
	if( fp ){
		ZnkF_fprintf( fp, "GET %s HTTP/1.1\r\n", request_uri );
		ZnkF_fprintf( fp, "Host: %s\r\n", hostname );
		ZnkF_fprintf( fp, "User-Agent: %s\r\n", ua );
		//ZnkF_fprintf( fp, "Accept: */*\r\n" );
		ZnkF_fprintf( fp, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n" );
		ZnkF_fprintf( fp, "Accept-Language: ja,en-US;q=0.7,en;q=0.3\r\n" );
		ZnkF_fprintf( fp, "Accept-Encoding: gzip, deflate\r\n" );

		/* Referer */
		if( !ZnkS_empty(referer) ){
			ZnkF_fprintf( fp, "Referer: %s\r\n", referer );
		}
		/* Cookie */
		{
			const size_t size = ZnkVarpAry_size( cookie );
			if( size ){
				size_t idx;
				ZnkVarp varp;
				ZnkF_fprintf( fp, "Cookie: " );
				for( idx=0; idx<size; ++idx ){
					varp = ZnkVarpAry_at( cookie, idx );
					ZnkF_fprintf( fp, "%s=%s", ZnkStr_cstr(varp->name_), ZnkVar_cstr(varp) );
					if( idx < size-1 ){
						ZnkF_fprintf( fp, "; " );
					}
				}
				ZnkF_fprintf( fp, "\r\n" );
			}
		}

		ZnkF_fprintf( fp, "Connection: keep-alive\r\n" );
		ZnkF_fprintf( fp, "\r\n" );
		ZnkF_fclose( fp );
		return true;
	}
	return false;
}

static bool
getCatalog( const char* srv_name, const char* board_id,
		ZnkStr posttime, const char* ua, ZnkVarpAry cookie, ZnkStr thre_uri,
		const char* parent_proxy )
{
	bool result = false;
	char result_file[ 256 ] = "";
	char cookie_file[ 256 ] = "";
	const char* exedir = st_is_dos_path ? ".\\"      : "./";
	const char* tmpdir = st_is_dos_path ? ".\\tmp\\" : "./tmp/";
	{
		char hostname[ 256 ];
		char req_uri[ 1024 ];
		char cmd[ 4096 ];
		char parent_cnct[ 4096 ] = "";
		int ret = EXIT_FAILURE;

		Znk_snprintf( hostname, sizeof(hostname), "%s.2chan.net", srv_name );
		if( ZnkS_empty( parent_proxy ) || ZnkS_eq( parent_proxy, "NONE" ) ){
			Znk_snprintf( parent_cnct, sizeof(parent_cnct), "%s", hostname );
			Znk_snprintf( req_uri, sizeof(req_uri), "/%s/futaba.php?mode=cat", board_id );
		} else {
			/* by proxy */
			Znk_snprintf( parent_cnct, sizeof(parent_cnct), "%s", parent_proxy );
			/* �����ł�http://�͕K�{�ł���(�Ȃ��Ƃ��܂������Ȃ�proxy������) */
			Znk_snprintf( req_uri, sizeof(req_uri), "http://%s/%s/futaba.php?mode=cat", hostname, board_id );
		}
		decorateHeaderGET( hostname, req_uri, ua, "", cookie );
		Znk_snprintf( result_file, sizeof(result_file), "%scatalog.htm", tmpdir );
		Znk_snprintf( cmd, sizeof(cmd), "%shttp_decorator %s %s %scookie.txt %shttp_hdr.dat",
				exedir, parent_cnct, result_file, tmpdir, tmpdir );
		ret = system( cmd );
		if( ret == EXIT_FAILURE ){
			return false;
		}
	}
	{
		ZnkFile fp = ZnkF_fopen( result_file, "rb" );
		if( fp ){
			ZnkStr line = ZnkStr_new( "" );
			while( true ){
				if( !ZnkStrFIO_fgets( line, 0, 4096, fp ) ){
					/* eof */
					break;
				}
				if( strstr( ZnkStr_cstr(line), "<a href='res/" ) ){
					ZnkF_printf_e( "*** line=[%s]\n", ZnkStr_cstr(line) );
					if( ZnkStrPtn_getBetween( thre_uri, (uint8_t*)ZnkStr_cstr(line), ZnkStr_leng(line), "<a href='", "' " ) ){
						/* line�͂����s�v�Ȃ̂ł�����ꎞ�o�b�t�@�Ƃ��ėp���� */
						ZnkStr_sprintf( line, 0, "/%s/%s", board_id, ZnkStr_cstr(thre_uri) );
						ZnkStr_swap( line, thre_uri );
						break;
					}
				}
			}
			ZnkStr_delete( line );
			ZnkF_fclose( fp );
		} else {
			return false;
		}
	}
	{
		Znk_snprintf( cookie_file, sizeof(cookie_file), "%scookie.txt", tmpdir );
		if( ZnkCookie_load( cookie, cookie_file ) ){
			ZnkVarp varp = ZnkVarpAry_find_byName( cookie, "posttime", Znk_strlen_literal( "posttime" ), false );
			ZnkStr_set( posttime, ZnkVar_cstr(varp) );
			result = true;
		}
	}
	return result;

}

static bool
getCaco( const char* srv_name, ZnkStr caco, const char* ua, ZnkVarpAry cookie, ZnkStr thre_uri,
		const char* parent_proxy )
{
	bool result = false;
	char result_file[ 256 ] = "";
	{
		int ret = EXIT_FAILURE;
		char hostname[ 256 ];
		char req_uri[ 1024 ];
		char cmd[ 4096 ];
		char referer[ 4096 ];
		const char* exedir = st_is_dos_path ? ".\\"      : "./";
		const char* tmpdir = st_is_dos_path ? ".\\tmp\\" : "./tmp/";
		char parent_cnct[ 4096 ] = "";

		Znk_snprintf( hostname, sizeof(hostname), "%s.2chan.net", srv_name );
		Znk_snprintf( referer, sizeof(referer), "http://%s%s", hostname, ZnkStr_cstr(thre_uri) );
		if( ZnkS_empty( parent_proxy ) || ZnkS_eq( parent_proxy, "NONE" ) ){
			Znk_snprintf( parent_cnct, sizeof(parent_cnct), "%s", hostname );
			Znk_snprintf( req_uri, sizeof(req_uri), "/bin/cachemt7.php" );
		} else {
			/* by proxy */
			Znk_snprintf( parent_cnct, sizeof(parent_cnct), "%s", parent_proxy );
			/* �����ł�http://�͕K�{�ł���(�Ȃ��Ƃ��܂������Ȃ�proxy������) */
			Znk_snprintf( req_uri, sizeof(req_uri), "http://%s/bin/cachemt7.php", hostname );
		}
		decorateHeaderGET( hostname, req_uri, ua, referer, cookie );
		Znk_snprintf( result_file, sizeof(result_file), "%scaco.txt", tmpdir );
		Znk_snprintf( cmd, sizeof(cmd), "%shttp_decorator %s %s %scookie.txt %shttp_hdr.dat",
				exedir, parent_cnct, result_file, tmpdir, tmpdir );

		ret = system( cmd );
		if( ret == EXIT_FAILURE ){
			return false;
		}
	}

	{
		ZnkFile fp = ZnkF_fopen( result_file, "rb" );
		if( fp ){
			ZnkStr line = ZnkStr_new( "" );
			ZnkStrFIO_fgets( line, 0, 4096, fp );
			ZnkStrPtn_getBetween( caco, (uint8_t*)ZnkStr_cstr(line), ZnkStr_leng(line), "\"", "\"" );
			result = true;
			ZnkStr_delete( line );
			ZnkF_fclose( fp );
		}
	}
	return result;

}

static bool
loadLineAryData( ZnkStrAry line_ary, const char* filename )
{
	ZnkFile fp = ZnkF_fopen( filename, "rb" );
	if( fp ){
		ZnkStr line = ZnkStr_new( "" );
		ZnkStrAry_clear( line_ary );

		while( true ){
			if( !ZnkStrFIO_fgets( line, 0, 4096, fp ) ){
				break; /* eof : �ǂݍ��݊��� */
			}

			ZnkStr_chompNL( line ); /* ���s������ */
			if( ZnkStr_first( line ) == '#' ){
				continue; /* �s����#�Ŏn�܂�ꍇ��skip */
			}
			/* �O���ɂ���white-space������ */
			while( ZnkStr_first( line ) == ' ' || ZnkStr_first( line ) == '\t' ){
				ZnkStr_cut_front_c( line );
			}
			if( ZnkStr_empty( line ) ){
				continue; /* ��s��skip */
			}

			/* ���̍s����̒l�Ƃ��ĕ�����z��ւƒǉ� */
			ZnkStrAry_push_bk_cstr( line_ary, ZnkStr_cstr(line), ZnkStr_leng(line) );
		}
		ZnkStr_delete( line );
		ZnkF_fclose( fp );
		return true;
	}
	return false;
}

static uint32_t
murmurhash3_32_gc( const char* key, size_t key_length, uint32_t seed )
{
	static const uint32_t c1 = 0xcc9e2d51;
	static const uint32_t c2 = 0x1b873593;
	const size_t remainder = key_length & 3; /* key_length % 4 */
	const size_t bytes = key_length - remainder;

	uint32_t h1 = seed;
	uint32_t i = 0;
	uint32_t h1b;
	uint32_t k1;
	
	while( i < bytes ){
		/***
		 * ISO/IEC 9899/1990 6.3 �ɂ��΁AC�ɂ����ĕ���p�����_(Sequence point)�Ƃ�
		 * ���ׂĂ̕���p�������������Ƃ��ۏ؂���Ă���_�ł���. ����͎��̖{���̏I���A
		 * �������̕]��������̊֐��̌ďo�����A&&�A||�A�O�����Z�q?:�A�J���}���Z�q�ɂ�����
		 * ���݂���. �����ȊO�ł̉��Z�q�ŋ�؂�ꂽ�͈͂ł́A���͈͓̔��ŕϐ�x��
		 * ������X�V�����ꍇ�́A���͈͓̔��ŕϐ�x�𕡐���X�V�����ꍇ��x�̕]��������
		 * ����`�ł���. �܂����Ƃ����̂悤�Ȕ͈͂ɂ�����x�̍X�V�����݂̂ł����Ă��A
		 * x�X�V�O�̒l���Ax�X�V��̒l�����肷�邽�߂����ɃA�N�Z�X����Ă���ꍇ(x = x+1�Ȃ�)�������A
		 * x�̕ύX�ƎQ�Ƃ������ɍs���Ă���ꍇ(a[x]=x++�Ȃ�)�̓���͖���`�ƂȂ�.
		 * 
		 * �]���ăI���W�i����Javascript�ɂ���悤�� ++i ����̎����ŕ����g���L�q��C�ł�NG�ł���
		 * ( | ���Z�q�ŋ敪����Ă͂��邪�A����͏�L�̏����𖞂������Z�q�ł͂Ȃ����Ƃɒ��� )�A
		 * �Ⴆ�Έȉ��̂悤�Ɍ�ł܂Ƃ߂ăC���N�������g����K�v������.
		 */
		k1 =
			( (key[ i   ] & 0xff)      ) |
			( (key[ i+1 ] & 0xff) << 8 ) |
			( (key[ i+2 ] & 0xff) << 16) |
			( (key[ i+3 ] & 0xff) << 24);
		i += 4;
		k1  = ((((k1 & 0xffff) * c1) + ((((k1 >> 16) * c1) & 0xffff) << 16))) & 0xffffffff;
		k1  = (k1 << 15) | (k1 >> 17);
		k1  = ((((k1 & 0xffff) * c2) + ((((k1 >> 16) * c2) & 0xffff) << 16))) & 0xffffffff;
		h1 ^= k1;
		h1  = (h1 << 13) | (h1 >> 19);
		h1b = ((((h1 & 0xffff) * 5) + ((((h1 >> 16) * 5) & 0xffff) << 16))) & 0xffffffff;
		h1  = (((h1b & 0xffff) + 0x6b64) + ((((h1b >> 16) + 0xe654) & 0xffff) << 16));
	}

	k1 = 0;
	
	switch( remainder ){
	case 3: k1 ^= (key[ i + 2 ] & 0xff) << 16; /* fall-through */
	case 2: k1 ^= (key[ i + 1 ] & 0xff) << 8;  /* fall-through */
	case 1: k1 ^= (key[ i     ] & 0xff);
		k1  = (((k1 & 0xffff) * c1) + ((((k1 >> 16) * c1) & 0xffff) << 16)) & 0xffffffff;
		k1  = (k1 << 15) | (k1 >> 17);
		k1  = (((k1 & 0xffff) * c2) + ((((k1 >> 16) * c2) & 0xffff) << 16)) & 0xffffffff;
		h1 ^= k1;
		break;
	default:
		break;
	}
	
	h1 ^= key_length;
	
	h1 ^= h1 >> 16;
	h1  = (((h1 & 0xffff) * 0x85ebca6b) + ((((h1 >> 16) * 0x85ebca6b) & 0xffff) << 16)) & 0xffffffff;
	h1 ^= h1 >> 13;
	h1  = ((((h1 & 0xffff) * 0xc2b2ae35) + ((((h1 >> 16) * 0xc2b2ae35) & 0xffff) << 16))) & 0xffffffff;
	h1 ^= h1 >> 16;
	
	return h1 >> 0;
}
static void
setMurmurhash32ToVar( ZnkVarp varp, const char* src_key )
{
	uint32_t val;
	char buf[ 256 ];

	val = murmurhash3_32_gc( src_key, Znk_strlen(src_key), 31 );
	Znk_snprintf( buf, sizeof(buf), "%u", val );
	ZnkVar_set_val_Str( varp, buf, Znk_strlen(buf) );
}

static ZnkVarp
refPostVar( ZnkMyf myf, const char* var_name )
{
	return ZnkMyf_refVar( myf, "post_vars", var_name );
}
static ZnkVarp
refCookieVar( ZnkMyf myf, const char* var_name )
{
	return ZnkMyf_refVar( myf, "cookie_vars", var_name );
}
static ZnkVarp
refUserAgent( ZnkMyf myf )
{
	return ZnkMyf_refVar( myf, "header_vars", "User-Agent" );
}

static bool
shuffleMyfFilter( ZnkMyf myf, const char* srv_name, const char* board_id,
		const char* parent_proxy, char* result_msg, size_t result_msg_size )
{
	bool result = false;

	ZnkStr       caco     = ZnkStr_new( "" );
	ZnkStr       posttime = ZnkStr_new( "" );
	ZnkStr       thre_uri = ZnkStr_new( "" );
	ZnkStrAry   line_ary = ZnkStrAry_create( true );

	ZnkVarp      varp = NULL;
	ZnkVarpAry  cookie = NULL;
	/***
	 * user_agent.txt�̓ǂݍ��݂Ɏ��s�����ꍇ�͈ȉ��̒l���g����.
	 */
	const char*  ua = "futaba";
	unsigned int random_uval1;
	unsigned int random_uval2;

	srand((unsigned) time(NULL));

	cookie = ZnkVarpAry_create( true );

	ZnkF_printf_e( "shuffleMyfFilter : parent_proxy=[%s]", parent_proxy );
	/***
	 * user_agent.txt�ɗ񋓂���Ă���User-Agent���烉���_���Ɉ�I�сA
	 * �����Http�w�b�_�ɃZ�b�g����.
	 * user_agent.txt�ɂ͉�X�����R�ɒl���L�q���邱�Ƃ��ł��邪
	 * ���ۂɗL�蓾��l���L�q���Ă������Ƃ������߂���.
	 *
	 * �f�t�H���g�ŕt�����Ă���user_agent.txt�́A�l�b�g��ɓ]�����Ă���l���E���W��
	 * �����K���ɂł����グ�����̂ł���.
	 * ������C�����邱�ƂȂ����̂܂܎g���Ă��悢���A�V�����u���E�U���o�ꂷ�邽�т�
	 * �i�X�Ƃ��̏��͌Â��Ȃ邵�A�ł���΂��Ȃ��Ǝ��̃v���C�x�[�g�� user_agent.txt��
	 * �������Ă����ق����Ȃɂ��Ɗm���ł͂���.
	 * �蓮�ōD�݂̃��X�g���쐬���Ă����Ă��悢���A �J�ɂ���User-Agent�W�F�l���[�^��
	 * �g���̂���̎�ł���.
	 * (�ꉞ�AUSERS�̎����o�[�W�����ł͂��̂悤�ȃW�F�l���[�^�̒ǉ����\�肵�Ă͂���).
	 *
	 * caco�R�[�h�擾�̂��߂�cachemt7.php��GET����̂ł��邪�A���̍ۂ�User-Agent��
	 * ����������. ���̒l��cachemt7.php���`�F�b�N���Ă���\�����ے�͂ł��Ȃ�.
	 * ����āA������O��User-Agent��Randomize���������Ēu���K�v�����邱�Ƃ�
	 * ���ӂ���.
	 */
	varp = refUserAgent( myf );
	if( varp ){
		size_t size;
		size_t idx;
		loadLineAryData( line_ary, "user_agent.txt" );
		size = ZnkStrAry_size( line_ary );
		if( size ){
			random_uval1 = getRandomUInt();
			idx = random_uval1 % size;
			ua = ZnkStrAry_at_cstr( line_ary, idx );
		}
		ZnkVar_set_val_Str( varp, ua, Znk_strlen(ua) );
	}

	/***
	 * ���݂ł�catalog��GET����ƁA���̉�������Set-Cookie�ɂ����
	 * Cookie��posttime�l���ݒ肳���悤�ł���.
	 * (���邢�́Acatalog�Ɉ����A�N�Z�X���Ă��Ȃ���Cookie���������ɂ����
	 * posttime�l���ݒ肳��Ă��Ȃ���Ԃŏ����POST���s���ƁA
	 * �uCookie��L���ɂ��Ă��������v���b�Z�[�W���Ԃ���邪�A���̉�������
	 * Set-Cookie�ɂ���Ă����̒l���ݒ肳���.
	 *
	 * �Ƃ肠�����A�����ł�Cookie posttime�l�̎擾��ړ��ĂɈ�xcatalog�ɃA�N�Z�X����.
	 * �����炭�����̃��[�U���ŏ��ɃA�N�Z�X����̂�catalog�ł��낤���炱��͑Ó��ȑI���Ǝv����.
	 * ���Aposttime�̒l�͑S�T�[�o�ŋ��ʂł���. ��������΃T�[�o���ɒl��ێ�����K�v�͂Ȃ�.
	 */
	if( !getCatalog( srv_name, board_id, posttime, ua, cookie, thre_uri, parent_proxy ) ){
		Znk_snprintf( result_msg, result_msg_size, "Cannot get catalog" );
		goto FUNC_END;
	}
	
	/***
	 * �����ň�xHTTP�ʐM(http_decorator��p����)���s���Acachemt7.php�̓��e�𓾂�.
	 *
	 * ���Ȃ��̎g���Ă���u���E�U��cachemt7.php�ɒ��ڃA�N�Z�X�����Ƃ�
	 * �������̓��e������ς��Ȃ��̂ł���΁A����͂Q��ڈȍ~�̒ʐM�ł�
	 * �L���b�V�����Q�Ƃ��Ă��邽�߂ł���.
	 *
	 * ����Ahttp_decorator�ɂ��HTTP�ʐM�̓L���b�V���O����Ȃ�.
	 * �܂薈���������e�ƂȂ�.
	 */
	if( !getCaco( srv_name, caco, ua, cookie, thre_uri, parent_proxy ) ){
		Znk_snprintf( result_msg, result_msg_size, "Cannot get caco code" );
		goto FUNC_END;
	}

	/***
	 * ���̕ϐ��͌��݂ł�caco�R�[�h�̒l�����̂܂܊i�[�����悤�ł���.
	 */
	varp = refPostVar( myf, "pthc" );
	if( varp ){
		ZnkVar_set_val_Str( varp, ZnkStr_cstr(caco), ZnkStr_leng(caco) );
	}

	/***
	 * ���̕ϐ��͌���ł́A�Â�caco�R�[�h�̒l���o���邾���c�����Ƃ�_���Ƃ���
	 * ������̂Ǝv����. ���ہACookie��L���b�V�����N���A�����Ƃ��Ă�
	 * localStorage(futabapt)�̒l����������Ă��Ȃ��ꍇ�́A�Â�caco�l��������
	 * �i�[�����d�g�݂ƂȂ��Ă���.
	 * �����炭�Apthc�̒l���ς��pthb�ɂ͈ȑO��caco�R�[�h�l�����̂܂܎c���Ă����ꍇ��
	 * futaba.php���ł̃��[�U����A���S���Y����pthc���pthb�̒l��D�悷����̂Ǝv����.
	 *
	 * �]���āApthb�։�X���ݒ肷�ׂ��l�͋�l�����݂�pthc�̒l�Ƃ������ƂɂȂ邪�A
	 * �Ƃ肠��������shuffle�����ɂ����Ă͋�l��ݒ肵�Ă����΂悢.
	 */
	varp = refPostVar( myf, "pthb" );
	if( varp ){
		ZnkVar_set_val_Str( varp, "", 0 );
	}

	/***
	 * ���̕ϐ��͎��ۂɂ�POST�ϐ��Q�̒��ɑ��݂����AlocalStorage�ɂ��̖��O�ő��݂���.
	 * ����͏����POST����caco�R�[�h�l�ɂ���ď����������.
	 * �����ĂQ��ڂ�POST�ɂ����ĎQ�Ƃ���Apthb�l�Ƃ��Ďg����.
	 * �����ł́Aon_post_before �̌Ăяo���ɂ���Ă��̋���������������̂Ƃ���.
	 *
	 * �Ƃ肠��������shuffle�����ɂ����Ă�localStorage�̊��S�������V�~�����[�g����΂悢.
	 * �܂��l��ݒ肵�Ă����΂悢.
	 */
	varp = refPostVar( myf, "USERS_futabapt" );
	if( varp ){
		ZnkVar_set_val_Str( varp, "", 0 );
	}

	/***
	 * ��ʂ�hash�l���猳������𕜌�����͕̂s�\�ł���.
	 * �]���āA�ȉ���setMurmurhash32ToVar�̑�Q�����Ƃ��ė^���錳������́A
	 * ��X�����R�Ɍ��߂Ă����܂�Ȃ�.
	 *
	 * �����ł͂��̌�������Ƃ��ĂƂ肠���������_���Ȑ������g���Ă���.
	 * ���������������ł�futaba.php������ɑ΍􂷂�V�i���I���l�����Ȃ��͂Ȃ�.
	 * �����Ŗ��S���������߂ɁA�����_���Ȓl���p����.
	 * ���̃����_���ȂQ�l�������āA��X�̃}�V�����̃_�~�[�l�Ƃ���.
	 */
	random_uval1 = getRandomUInt();
	random_uval2 = getRandomUInt();
	varp = refPostVar( myf, "flrv" );
	if( varp ){
		char src_key[ 4096 ];
		Znk_snprintf( src_key, sizeof(src_key), "%s###%u###%u", ua, random_uval1, random_uval2 );
		setMurmurhash32ToVar( varp, src_key );
	}

	/***
	 * �ŋߓ������ꂽUser-Agent�����܂܂Ȃ�fingerprint�l�ł���.
	 * User-Agent�̒l��ς����ꍇ�Aflrv�l�͕ς��Aflvv�l�͕ς��Ȃ�.
	 * ���̂��Ƃ���User-Agent�U����������x���o���邱�Ƃ��_���Ǝv����.
	 *
	 * flrv�Ɠ��l�Ƀ����_��������΂悢���A�قȂ�l�ƂȂ�悤���ӂ���.
	 * (���̗��҂������l�ƂȂ�m���̓[���ł͂Ȃ��ɂ��Ă��V���w�I�ɒႭ�A
	 * �܂莖���㖾�炩�Ɉُ�ł���A���̂��Ƃ�futaba.php���ł��u����
	 * ����\�ł���)
	 * �ȉ��ł͎d�l�ɒ�����key�������User-Agent���݂̂��܂߂Ă��Ȃ�.
	 */
	varp = refPostVar( myf, "flvv" );
	if( varp ){
		char src_key[ 4096 ];
		Znk_snprintf( src_key, sizeof(src_key), "%u###%u", random_uval1, random_uval2 );
		setMurmurhash32ToVar( varp, src_key );
	}

	/***
	 * screen_size.txt�ɗ񋓂���Ă���T�C�Y���烉���_���Ɉ�I�сA
	 * �����depth�������_���ɑI���������scsz���쐬����.
	 * screen_size.txt�ɂ͉�X�����R�ɒl���L�q���邱�Ƃ��ł��邪
	 * ���ۂɗL�蓾��l���L�q���Ă������Ƃ������߂���.
	 *
	 * �f�t�H���g�ŕt�����Ă���screen_size.txt���C�����邱�ƂȂ����̂܂܎g���Ă�
	 * �Ȃ����Ȃ��낤. �命���̃��[�U�͂����ɋL�ڂ���Ă���𑜓x�̂����ꂩ��
	 * �g�p���Ă���͂��ł���(����ł������ߕs����������Ȃ玩�R�ɍ폜/�ǉ�����΂悢).
	 */
	varp = refPostVar( myf, "scsz" );
	if( varp ){
		char scsz[ 256 ];
		const char* wh = "640x480";
		size_t size;
		/***
		 * 32bit�ł���ꍇ�������炭���v�I�Ɉ�ԍ����ł��낤����A
		 * �ȉ��ł͊m���������Ȃ�悤���߂ɐݒ肵�Ă���.
		 */
		static const int depth_tbl[] = { 16, 24, 24, 32, 32, 32, 32 };
		int depth;

		/* get wh */
		loadLineAryData( line_ary, "screen_size.txt" );
		size = ZnkStrAry_size( line_ary );
		if( size ){
			size_t idx;
			random_uval1 = getRandomUInt();
			idx = random_uval1 % size;
			wh = ZnkStrAry_at_cstr( line_ary, idx );
		}

		/* get depth */
		random_uval2 = getRandomUInt();
		depth = depth_tbl[ random_uval2 % Znk_NARY(depth_tbl) ]; 

		/* set scsz */
		Znk_snprintf( scsz, sizeof(scsz), "%sx%d", wh, depth );
		ZnkVar_set_val_Str( varp, scsz, Znk_strlen(scsz) );
	}

	varp = refCookieVar( myf, "posttime" );
	if( varp ){
		ZnkVar_set_val_Str( varp, ZnkStr_cstr(posttime), ZnkStr_leng(posttime) );
	}

	result = true;
FUNC_END:
	ZnkStr_delete( caco );
	ZnkStr_delete( posttime );
	ZnkStr_delete( thre_uri );
	ZnkStrAry_destroy( line_ary );

	return result;
}

bool
initiate( ZnkMyf flr_send, const char* parent_proxy, char* result_msg, size_t result_msg_size )
{
	/***
	 * �ŏ��ɎQ�Ƃ���T�[�o���Ɣ�ID�����w��.
	 * �Ⴆ�� may �� b �̑g�ݍ��킹�Ȃ�񎟌���May���Ӗ�����.
	 */
	const char* srv_name = "may";
	const char* board_id = "b";

#if defined(Znk_TARGET_WINDOWS)
	st_is_dos_path = true;
#else
	st_is_dos_path = false;
#endif

	ZnkDir_mkdirPath( "./tmp", Znk_NPOS, '/' );
	if( !shuffleMyfFilter( flr_send, srv_name, board_id, parent_proxy, result_msg, result_msg_size ) ){
		return false;
	}
	if( !ZnkMyf_save( flr_send, "filters/futaba_send.myf" ) ){
		Znk_snprintf( result_msg, result_msg_size, "Cannot save futaba_send.myf" );
		return false;
	}
	Znk_snprintf( result_msg, result_msg_size, "Virtual USERS done. Your futaba_send.myf is randomized successfully." );
	return true;
}

