#include "Doc_html.h"
#include "Doc_util.h"

#include <Rano_dir_util.h>

#include <Znk_myf.h>
#include <Znk_s_base.h>
#include <Znk_stdc.h>
#include <Znk_mbc_jp.h>
#include <Znk_bfr_bif.h>
#include <Znk_bif.h>
#include <Znk_str_ex.h>
#include <Znk_dir.h>
#include <Znk_missing_libc.h>

#include <stdio.h>

struct Info {
	char* href_;
	char* name_;
	char* category_;
};

static const char*
getImgPath( const char* author )
{
	if( ZnkS_eq( author, "Mr.Moai" ) ){
		return "/imgs/moai.png";
	}
	if( ZnkS_eq( author, "Zenkaku" ) ){
		return "/imgs/zenkaku.png";
	}
	if( ZnkS_eq( author, "K.Yakisoba.H" ) ){
		return "/imgs/yakisoba.png";
	}
	return NULL;
}

static void
drawConfig( ZnkStr ans, ZnkMyf in_myf, ZnkMyf menu_myf )
{
	ZnkVarpAry  global_config = ZnkMyf_find_vars( menu_myf, "config" );
	ZnkVarp     title_author_varp = ZnkVarpAry_findObj_byName_literal( global_config, "title_author", false );
	const char* title_author      = title_author_varp ? ZnkVar_cstr( title_author_varp ) : "Author";
	ZnkVarp     title_update_varp = ZnkVarpAry_findObj_byName_literal( global_config, "title_update", false );
	const char* title_update      = title_update_varp ? ZnkVar_cstr( title_update_varp ) : "LastUpdate";
	ZnkVarpAry  config = ZnkMyf_find_vars( in_myf, "config" );
	ZnkVarp     author_varp  = ZnkVarpAry_findObj_byName_literal( config, "author", false );
	ZnkStrAry   author_list  = ( author_varp && ZnkVar_prim_type( author_varp ) == ZnkPrim_e_StrAry ) ?
		ZnkVar_str_ary( author_varp ) : NULL;
	ZnkVarp     update_varp  = ZnkVarpAry_findObj_byName_literal( config, "update", false );
	const char* update       = update_varp ? ZnkVar_cstr( update_varp ) : NULL;


#if 0
	if( author || update ){
		ZnkStr_addf( ans,
				"<div class=MstyComment>\n"
				"<u><b>%s</b></u> : %s<br>\n"
				"<br>\n"
				"<u><b>%s</b></u> : %s<br>\n"
				"<br>\n"
				"</div>\n"
				"<br>\n",
				title_author, author ? author : "",
				title_update, update ? update : "" );
	}
#else
	if( author_list ){
		size_t author_list_size = ZnkStrAry_size( author_list );
		size_t idx;

		ZnkStr_addf( ans,
				"<div class=\"card\">\n"
				"	<u><b>%s</b></u><br>\n"
				"	<div class=\"card-content\">\n", title_author );

		for( idx=0; idx<author_list_size; ++idx ){
			const char* author = ZnkStrAry_at_cstr( author_list, idx );
			const char* imgpath = getImgPath( author );
			if( imgpath ){
				ZnkStr_addf( ans,
						"		<div class=\"media\">\n"
						"			<div class=\"media-left\">\n"
						"				<figure class=\"image is-48x48\">\n"
						"				<img src=\"%s\" alt=\"none image\">\n"
						"				</figure>\n"
						"			</div>\n"
						"			<div class=\"media-content\">\n"
						"				<p class=\"title is-4\">%s</p>\n"
						"				<p class=\"subtitle is-6\">@znk project</p>\n"
						"			</div>\n"
						"		</div>\n", imgpath, author );
			} else {
				ZnkStr_addf( ans,
						"		<div class=\"media\">\n"
						"			<div class=\"media-content\">\n"
						"				<p class=\"title is-4\">%s</p>\n"
						"				<p class=\"subtitle is-6\">@znk project</p>\n"
						"			</div>\n"
						"		</div>\n", author );
			}
		}

		if( update ){
			/* 23:09 PM - 20 Jun 2019 */
			ZnkStr_addf( ans,
					"		<div class=\"content\">\n"
					"		  <time datetime=\"2016-1-1\">%s</time>\n"
					"		</div>\n", update );
		}

		ZnkStr_addf( ans,
				"	</div>\n"
				"</div>\n" );

	}
#endif
	ZnkStr_add( ans, "\n\n" );
}

static bool
findInStrs( const char* begin, const char* end, ZnkStrAry strs )
{
	const size_t size = ZnkStrAry_size( strs );
	size_t       idx;
	if( begin > end ){
		return false;
	}
	for( idx=0; idx<size; ++idx ){
		const size_t str_leng = ZnkStrAry_at_leng( strs, idx );
		const char*  str      = ZnkStrAry_at_cstr( strs, idx );
		if( (size_t)(end-begin) == str_leng && ZnkS_eqCaseEx( begin, str, str_leng ) ){
			/* found */
			return true;
		}
	}
	return false;
}

static bool
isInTags( const char* tag_begin, ZnkStrAry nb_tags )
{
	const char* p       = tag_begin+1;
	const char* q       = Znk_strchr( tag_begin+1, ' ' );
	const char* tag_end = Znk_strchr( tag_begin+1, '>' );
	if( tag_end == NULL ){
		/* this is broken tag */
		return false;
	}
	if( *p == '/' ){ ++p; }
	if( q == NULL || q > tag_end ){
		q = tag_end;
		if( *(q-1) == '/' ){
			/* <xxx/>�Ƃ����^�C�v�̃^�O.
			 * ���A�����ł̏����ł�tag_begin����q�܂ł̊Ԃɂ̓X�y�[�X�����݂��Ȃ�����
			 * �Ō�� / �� (href�Ȃǂł�)�p�X��dsp�ƂȂ��Ă���\���͏��O�ł���. */
			--q;
		}
		if( p > q ){
			/* ������ </> �Ƃ����p�^�[���������ꍇ�͂��̂悤�ȏ󋵂��N������ */
			return false;
		}
	}
	return findInStrs( p, q, nb_tags );
}
static void
drawLine( ZnkStr ans, ZnkStrAry lines, size_t idx, ZnkStrAry nb_tags )
{
	bool with_br = true;
	ZnkStr line = ZnkStrAry_at( lines, idx );
	if( ZnkStr_last( line ) == '>' ){
		if( ZnkStr_isEnd( line, "<\\>" ) ){
			ZnkStr_cut_back( line, 3 );
			with_br = false;
		} else if( nb_tags ){
			char* tag_begin = ZnkStrEx_strrchr( line, '<' );
			if( tag_begin ){
				if( isInTags( tag_begin, nb_tags ) ){
					with_br = false;
				}
			}
		}
	}
	if( with_br ){
		ZnkStr_addf( ans, "%s<br>\n", ZnkStr_cstr( line ) );
	} else {
		ZnkStr_addf( ans, "%s\n", ZnkStr_cstr( line ) );
	}
}

static void
drawAtFirst( ZnkStr ans, ZnkMyf in_myf, ZnkMyf menu_myf )
{
	ZnkStrAry atfirst = ZnkMyf_find_lines( in_myf, "AtFirst" );
	if( atfirst ){
		ZnkVarpAry   global_config  = ZnkMyf_find_vars( menu_myf, "config" );
		ZnkVarp      sec_title_varp = ZnkVarpAry_findObj_byName_literal( global_config, "title_atfirst", false );
		const char*  sec_title      = sec_title_varp ? ZnkVar_cstr( sec_title_varp ) : "At First";
		ZnkVarpAry   config       = ZnkMyf_find_vars( in_myf, "config" );
		ZnkVarp      nb_tags_varp = ZnkVarpAry_findObj_byName_literal( config, "nb_tags", false );
		ZnkStrAry    nb_tags      = nb_tags_varp ? ZnkVar_str_ary( nb_tags_varp ) : NULL;
		const size_t size         = ZnkStrAry_size( atfirst );
		size_t       idx;

		ZnkStr_add( ans, "<a name=AtFirst></a>\n" );
		ZnkStr_addf( ans, "<h1 class=\"title\">%s</h1>\n", sec_title );
		ZnkStr_add( ans, "<p>\n" );
		for( idx=0; idx<size; ++idx ){
			drawLine( ans, atfirst, idx, nb_tags );
		}
		ZnkStr_add( ans, "</p>\n" );
		ZnkStr_add( ans, "<br>\n" );
		ZnkStr_add( ans, "\n\n" );
	}
}
static void
drawIndex( ZnkStr ans, ZnkMyf in_myf, ZnkMyf menu_myf )
{
	ZnkPrimpAry index = ZnkMyf_find_prims( in_myf, "Index" );
	if( index ){
		ZnkVarpAry   global_config  = ZnkMyf_find_vars( menu_myf, "config" );
		ZnkVarp      sec_title_varp = ZnkVarpAry_findObj_byName_literal( global_config, "title_index", false );
		const char*  sec_title      = sec_title_varp ? ZnkVar_cstr( sec_title_varp ) : "Index";
		const size_t size = ZnkPrimpAry_size( index );
		size_t idx;
		ZnkStr_add( ans, "<a name=Index></a>\n" );
		ZnkStr_addf( ans, "<h1 class=\"title\">%s</h1>\n", sec_title );
		ZnkStr_add( ans, "<br>\n" );
		ZnkStr_add( ans, "<ul>\n" );
		for( idx=0; idx<size; ++idx ){
			ZnkPrimp  prim = ZnkPrimpAry_at( index, idx );
			ZnkStrAry sary = ZnkPrim_strAry( prim );
			if( ZnkStrAry_size(sary) >= 2 ){
				const char* href  = ZnkStrAry_at_cstr( sary, 0 );
				const char* title = ZnkStrAry_at_cstr( sary, 1 );
				ZnkStr_addf( ans, "\t<li><b><a class=MstyElemLink href=%s>%s</a></b></li>\n", href, title );
			}
		}
		ZnkStr_add( ans, "</ul><br>\n" );
		ZnkStr_add( ans, "\n\n" );
	}
}

static bool
drawSection( ZnkStr ans, ZnkMyf in_myf, ZnkMyf menu_myf, const char* sec_name, const char* sec_title, const char* title_category, bool index_hidden )
{
	ZnkStrAry sec = ZnkMyf_find_lines( in_myf, sec_name );
	if( sec ){
		ZnkVarpAry   global_config  = ZnkMyf_find_vars( menu_myf, "config" );
		ZnkVarp      msg_backto_index_varp = ZnkVarpAry_findObj_byName_literal( global_config, "msg_backto_index", false );
		const char*  msg_backto_index      = msg_backto_index_varp ? ZnkVar_cstr( msg_backto_index_varp ) : "Back to Index";
		ZnkVarpAry   config       = ZnkMyf_find_vars( in_myf, "config" );
		ZnkVarp      nb_tags_varp = ZnkVarpAry_findObj_byName_literal( config, "nb_tags", false );
		ZnkStrAry    nb_tags      = nb_tags_varp ? ZnkVar_str_ary( nb_tags_varp ) : NULL;
		const size_t size         = ZnkStrAry_size( sec );
		size_t       idx;

		ZnkStr_addf( ans, "<a name=%s></a>\n", sec_name );
		ZnkStr_addf( ans, "<h1 class=\"title\">%s</h1>\n", sec_title );
		if( title_category ){
			ZnkVarp     title_attr_fmt_varp = ZnkVarpAry_findObj_byName_literal( config, "title_attr_fmt", false );
			const char* title_attr_fmt      = title_attr_fmt_varp ? ZnkVar_cstr( title_attr_fmt_varp ) : NULL;
			if( title_attr_fmt ){
				ZnkStr_addf( ans, title_attr_fmt, title_category );
			}
		}
		//ZnkStr_add( ans, "<p>\n" );
		for( idx=0; idx<size; ++idx ){
			drawLine( ans, sec, idx, nb_tags );
		}
		//ZnkStr_add( ans, "</p>\n" );

		if( !index_hidden ){
			ZnkStr_add( ans, "\t<br>\n" );
			ZnkStr_addf( ans, "\t<b><a class=MstyElemLink href=#Index>%s</a></b><br>\n", msg_backto_index );
		}
		ZnkStr_add( ans, "<br>\n" );
		ZnkStr_add( ans, "\n\n" );
		return true;
	}
	return false;
}

static void
drawLink( ZnkStr ans, ZnkMyf in_myf, ZnkMyf menu_myf )
{
	ZnkPrimpAry link = ZnkMyf_find_prims( in_myf, "Link" );
	if( link ){
		ZnkVarpAry   global_config  = ZnkMyf_find_vars( menu_myf, "config" );
		ZnkVarp      sec_title_varp = ZnkVarpAry_findObj_byName_literal( global_config, "title_link", false );
		const char*  sec_title      = sec_title_varp ? ZnkVar_cstr( sec_title_varp ) : NULL;
		const size_t size = ZnkPrimpAry_size( link );
		size_t idx;
		ZnkStr_add( ans, "<a name=Link></a>\n" );
		if( sec_title ){
			ZnkStr_addf( ans, "<h1 class=\"title\">%s</h1>\n", sec_title );
		}
		ZnkStr_add( ans, "<hr>\n" );
		ZnkStr_add( ans, "<ul>\n" );
		for( idx=0; idx<size; ++idx ){
			ZnkPrimp  prim = ZnkPrimpAry_at( link, idx );
			ZnkStrAry sary = ZnkPrim_strAry( prim );
			if( ZnkStrAry_size(sary) >= 2 ){
				const char* href  = ZnkStrAry_at_cstr( sary, 0 );
				const char* title = ZnkStrAry_at_cstr( sary, 1 );
				ZnkStr_addf( ans, "\t<li><a class=MstyElemLink href=%s>%s</a></li>\n", href, title );
			}
		}
		ZnkStr_add( ans, "</ul><br>\n" );
		ZnkStr_add( ans, "\n\n" );
	}
}


static bool st_pause_on_exit = false;

static void
makeOne( ZnkStr ans, ZnkMyf in_myf, ZnkMyf menu_myf, const char* in_file_path, const char* out_file_path )
{
	bool index_hidden = false;

	ZnkStr_clear( ans );
	ZnkMyf_clear( in_myf );
	
	ZnkMyf_load( in_myf, in_file_path );

	{
		ZnkVarpAry  config = ZnkMyf_find_vars( in_myf, "config" );
		ZnkVarp     category_path_varp = ZnkVarpAry_findObj_byName_literal( config, "category_path", false );
		ZnkStrAry   category_path      = category_path_varp ? ZnkVar_str_ary( category_path_varp ) : NULL;
		ZnkVarp     urp_varp           = ZnkVarpAry_findObj_byName_literal( config, "urp", false );
		const char* urp                = urp_varp           ? ZnkVar_cstr( urp_varp ) : NULL;
		ZnkVarp     doc_title_varp     = ZnkVarpAry_findObj_byName_literal( config, "doc_title", false );
		const char* doc_title          = doc_title_varp     ? ZnkVar_cstr( doc_title_varp ) : NULL;
		ZnkVarp     index_hidden_varp  = ZnkVarpAry_findObj_byName_literal( config, "index_hidden", false );
		index_hidden = index_hidden_varp ? ZnkS_eq( ZnkVar_cstr( index_hidden_varp ), "true" ) : false;
		DocUtil_drawHeader( ans, category_path, urp, menu_myf, doc_title );
	}

	//drawConfig( ans, in_myf, menu_myf );
	drawAtFirst( ans, in_myf, menu_myf );

	if( !index_hidden ){
		drawIndex( ans, in_myf, menu_myf );
	}

	{
		ZnkPrimpAry index = ZnkMyf_find_prims( in_myf, "Index" );
		if( index ){
			const size_t size = ZnkPrimpAry_size( index );
			size_t idx;
			for( idx=0; idx<size; ++idx ){
				ZnkPrimp  prim = ZnkPrimpAry_at( index, idx );
				ZnkStrAry sary = ZnkPrim_strAry( prim );
				if( ZnkStrAry_size(sary) >= 2 ){
					const char* href  = ZnkStrAry_at_cstr( sary, 0 );
					const char* title = ZnkStrAry_at_cstr( sary, 1 );
					const char* title_category = NULL;
					if( ZnkStrAry_size(sary) >= 3 ){
						title_category = ZnkStrAry_at_cstr( sary, 2 );
					}
					if( href[ 0 ] == '#' ){
						const char* sec_name = href + 1;
						if( !drawSection( ans, in_myf, menu_myf, sec_name, title, title_category, index_hidden ) ){
							Znk_printf_e( "docgen Error : Cannot find section[%s]. in [%s]\n", sec_name, in_file_path );
							st_pause_on_exit = true;
						}
					}
				}
			}
		}
	}

	drawConfig( ans, in_myf, menu_myf );
	drawLink( ans, in_myf, menu_myf );

	DocUtil_drawEnd( ans );

	//convertSJIStoUTF8( ans );
	{
		ZnkFile fp = Znk_fopen( out_file_path, "wb" );
		if( fp ){
			Znk_fputs( ZnkStr_cstr(ans), fp );
			Znk_fclose( fp );
		}
	}
}



struct FilterInfo {
	ZnkStr ans_;
	ZnkMyf in_myf_;
	ZnkMyf menu_myf_;
	const char* src_topdir_;
	const char* dst_topdir_;
	ZnkStrAry   file_list_;
	ZnkStrAry   ignore_list_;
};

static bool
filterFile( const char* src_file_path, const char* dst_file_path, void* arg )
{
	if( ZnkS_eq( ZnkS_get_extension( dst_file_path, '.' ), "in" ) ){
		/* .in ������ */
		char out_file_path[ 256 ] = "";
		struct FilterInfo* info = Znk_force_ptr_cast( struct FilterInfo*, arg );
		ZnkStr ans      = info->ans_;
		ZnkMyf in_myf   = info->in_myf_;
		ZnkMyf menu_myf = info->menu_myf_;
		ZnkS_copy( out_file_path, sizeof(out_file_path), dst_file_path, Znk_strlen( dst_file_path ) - 3 );
		makeOne( ans, in_myf, menu_myf, src_file_path, out_file_path );
		return true;
	}
	return false;
}

static bool
isIgnoreDir( ZnkDirRecursive recur, const char* top_dir, void* arg, size_t local_err_num )
{
	struct FilterInfo* info = Znk_force_ptr_cast( struct FilterInfo*, arg );
	ZnkStrAry ignore_list = info->ignore_list_;
	size_t src_topdir_leng = Znk_strlen( info->src_topdir_ );
	const char* under_path = top_dir + src_topdir_leng;

	if( under_path[ 0 ] == '/' ){ ++under_path; }
	if( ignore_list && ZnkStrAry_find_isMatch( ignore_list, 0, under_path, Znk_NPOS, ZnkS_isMatchSWC ) != Znk_NPOS ){
		/* skip */
		return true;
	}
	return false;
}

static ZnkStrAry
makeIgnoreList( void )
{
	ZnkStrAry list = ZnkStrAry_create( true );
	ZnkStrAry_push_bk_cstr( list, "*/mkf_android", Znk_NPOS );
	ZnkStrAry_push_bk_cstr( list, "*/mkf_triggers", Znk_NPOS );
	ZnkStrAry_push_bk_cstr( list, "*/out_dir", Znk_NPOS );
	ZnkStrAry_push_bk_cstr( list, "*/source_doc", Znk_NPOS );
	ZnkStrAry_push_bk_cstr( list, "*/doc_in", Znk_NPOS );
	return list;
}

bool
DocHtml_make( const char* in_dir, const char* out_dir, ZnkStr ermsg )
{
	bool result = false;
	ZnkMyf in_myf = ZnkMyf_create();
	ZnkMyf menu_myf = ZnkMyf_create();
	ZnkStr ans = ZnkStr_new( "" );
	struct FilterInfo info = { 0 };

	info.ans_      = ans;
	info.in_myf_   = in_myf;
	info.menu_myf_ = menu_myf;
	info.src_topdir_  = in_dir;
	info.dst_topdir_  = out_dir;
	info.file_list_   = ZnkStrAry_create( true );
	info.ignore_list_ = makeIgnoreList();

	ZnkDir_mkdirPath( out_dir, Znk_NPOS, '/', NULL );

	{
		char menu_myf_path[ 256 ];
		Znk_snprintf( menu_myf_path, sizeof(menu_myf_path), "%s/menu.myf", in_dir );
		if( !ZnkMyf_load( menu_myf, menu_myf_path ) ){
			if( ermsg ){
				ZnkStr_addf( ermsg, "docgen Error : Cannot load [%s].\n", menu_myf_path );
			}
			goto FUNC_END;
		}
	}

	RanoDirUtil_filterDir( in_dir, out_dir,
			"docgen", ermsg,
			NULL, NULL,
			filterFile, &info,
			isIgnoreDir, &info );

	result = true;
FUNC_END:
	ZnkStr_delete( ans );
	ZnkMyf_destroy( menu_myf );
	ZnkMyf_destroy( in_myf );

	ZnkStrAry_destroy( info.file_list_ );
	ZnkStrAry_destroy( info.ignore_list_ );

	return result;
}

bool
DocHtml_isPauseOnExit( void )
{
	return st_pause_on_exit;
}
