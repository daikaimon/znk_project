#ifndef INCLUDE_GUARD__Znk_dir_h__
#define INCLUDE_GUARD__Znk_dir_h__

#include <Znk_str.h>

Znk_EXTERN_C_BEGIN

typedef struct ZnkDirInfoImpl* ZnkDirId;


/**
 * @brief
 * �J�����g�f�B���N�g���̕ύX
 */
bool
ZnkDir_changeCurrentDir( const char* dir );

/**
 * @brief
 *   �J�����g�f�B���N�g���̃t���p�X���擾
 *
 * @return
 *   ans �ɂ̓J�����g�f�B���N�g�����ݒ肳��邪�A
 *   ���̓��� C������ւ̃|�C���^��Ԃ�.
 *   �{������͕K�v�Ȃ����A�G���[���b�Z�[�W�̍\�z���̗��֐��̂��߂̎d�l�ł���.
 */
const char*
ZnkDir_getCurrentDir( ZnkStr ans );


/**
 * @brief
 * file �������t�@�C���̃T�C�Y���擾����.
 * �����Ȃ��true, �����Ȃ����false��Ԃ�.
 * (�w�肳�ꂽ�p�X���f�B���N�g���ł������ꍇ���Ɏ��s����)
 */
bool
ZnkDir_getFileByteSize( const char* file, int64_t* file_size );


/**
 * @brief:
 *   �����ŗ^�����t�@�C��src_file �� dst_file �փR�s�[����.
 *   �����̓f�B���N�g���ł����Ă͂Ȃ�Ȃ�(���̏ꍇ���������G���[�l��Ԃ�).
 */
bool
ZnkDir_copyFile_byForce( const char* src_file, const char* dst_file );

/**
 * @brief:
 *   �����ŗ^�����t�@�C�� ���폜����.
 *   �����̓f�B���N�g���ł����Ă͂Ȃ�Ȃ�(���̏ꍇ���������G���[�l��Ԃ�).
 *
 * @return:
 *   �폜�ɐ��������ꍇtrue��Ԃ�.
 *   �����Ȃ���false��Ԃ�.
 */
bool
ZnkDir_deleteFile_byForce( const char* file );

/**
 * @brief:
 *   �����ŗ^�����f�B���N�g���̒��g����ł������ꍇ�A
 *   ���̃f�B���N�g�����폜����B
 *
 * @return:
 *   �폜�ɐ��������ꍇtrue��Ԃ�.
 *   �w�肵���f�B���N�g������łȂ�������A�����Ɏw�肵�����̂��t�@�C����
 *   �������ꍇ�A���̑��Ȃ�炩�̗��R�ō폜�Ɏ��s�����ꍇ�͂Ȃɂ�����false��Ԃ�.
 */
bool
ZnkDir_rmdir( const char* dir );


/**
 * @brief
 *   �t�@�C��/�f�B���N�g������ύX(�K�v�ɉ����Ĉړ�)����B
 *   �������A�f�B���N�g���Ɋւ��Ă͓����h���C�u���̈ړ��Ɍ�����.
 *   UNIX�Ɋւ��ẮA�t�@�C��/�f�B���N�g���Ƃ��ɓ����}�E���g���̈ړ��Ɍ�����.
 *
 * @return:
 *   �ړ�(���l�[��)�ɐ��������ꍇtrue��Ԃ�.
 *   �����Ȃ���false��Ԃ�.
 */
bool
ZnkDir_rename( const char* src_path, const char* dst_path );



/**
 * @brief:
 *   �����ŗ^�����f�B���N�g�����쐬����.
 *   path�ɂ͑��΃p�X��A/B/C�Ƃ����悤�ȕ����̊K�w���w�肵�Ă������ꋓ��
 *   �쐬���邱�Ƃ��ł���.
 *
 * @param path:
 *   �V�K�쐬�������f�B���N�g�����w�肷��.
 *   A/B/C�Ƃ����悤�ȕ����̊K�w�������̂��\�ŁAA��B�����݂��Ȃ��ꍇ�ł�
 *   �G���[�Ƃ͂Ȃ炸�����I�ɍ쐬�����.
 *
 * @param path_leng:
 *   path�̕����񒷂��w�肷��.
 *   path �� NULL�I�[���Ă���ꍇ�Ɍ���A������Znk_NPOS���w�肷�邱�Ƃ��ł��A
 *   ���̏ꍇ�� strlen(path)���w�肳�ꂽ�̂Ɠ����Ƃ݂Ȃ����.
 *
 * @param sep
 *   �f�B���N�g������؂�Z�p���[�^����(�ȉ������SEP�Ə���)�Ƃ��āA
 *   / �݂̂��g�������ꍇ�� '/' ���w�肷��.
 *   ���̏ꍇ�Apath�ɓ��{����܂߂邱�Ƃ��o����.
 *
 *   ����ȊO�̕������w�肵���ꍇ�� / �� \ �̑o���ɑΉ�����.
 *   ���̏ꍇ�A����炪��������path���������Ƃ��\�ł��邪�A
 *   path�ɓ��{����܂߂Ă͂Ȃ�Ȃ�.
 *
 * @return:
 *   �쐬�ɐ��������ꍇtrue��Ԃ�.
 *   ���邢�͎w�肵��path�����ɑ��݂���ꍇ��true��Ԃ�.
 *   �Ȃ�炩�̗��R�ō쐬�Ɏ��s�����ꍇ�͂Ȃɂ�����false��Ԃ�.
 */
bool
ZnkDir_mkdirPath( const char* path, size_t path_leng, char sep );



/**
 * @brief
 *   dir_name �ɂ��w�肵�� directory �� Open ����.
 *   ���s�����NULL��Ԃ�.
 *   ����ɂ��Adir_name ������ directory�����ɂ��� file��directory
 *   (�̎�ɖ��O)���C�e���[�g�ɂ���ď����擾���邱�Ƃ��ł���.
 *   �ʏ�͕K���AZnkDir_openDir, ZnkDir_readDir, ZnkDir_closeDir���Z�b�g�ɂ��Ďg��.
 *   �ڂ����g�p���@�� exam���Q��.
 *
 * @param dir_name:
 *   ���g���C�e���[�g�������f�B���N�g���p�X���w�肷��.
 *   ���΃p�X�A��΃p�X�̂�����ł��悢.
 *
 * @param dir_name_leng:
 *   ZnkDir_openDirEx �̂ݑ��݂���.
 *   dir_name �̕����񒷂��w�肷��.
 *   ����dir_name��NULL�I�[���Ă���ꍇ�Ɍ���A������Znk_NPOS���w�肷�邱�Ƃ��ł��A
 *   ���̏ꍇ�� strlen(dir_name) ���w�肳�ꂽ���̂Ƃ݂Ȃ����.
 *
 * @param is_skip_dot_and_dot_dot:
 *   ZnkDir_openDirEx �̂ݑ��݂���.
 *   ZnkDir_readDir�ɂ����� file/directory���̎擾�ɂ����āA. �� .. ���擾���ꂽ�ꍇ�A
 *   �����ł���������I�ɃX�L�b�v���A���̖��O���擾����. ���ʂȎ���Ȃ�����A�ʏ��
 *   true �ɂ��Ă����΂悢���낤.
 *
 * @exam
 *   ���Ƃ��΁A����f�B���N�g��dir���ɂ���t�@�C���ƃf�B���N�g�������ׂ�
 *   �W���o�͂ɕ\������R�[�h�͈ȉ��̂悤�ɂȂ�.
 *
 *   test1()
 *   {
 *     const char* name;
 *     ZnkDirId id = ZnkDir_openDir( dir );
 *     if( id == NULL ){
 *       return false; // Error
 *     }
 *     while( true ){
 *       name = ZnkDir_readDir( id );
 *       if( name == NULL ){
 *         break; // ���ׂăC�e���[�g��������.
 *       }
 *       // name �� . �܂��� .. �ɓ��������ۂ��̃`�F�b�N�͕s�v�ł���.
 *       // �����̓f�t�H���g�ł� ZnkDir_readDir���Ŏ����I�ɃX�L�b�v�����.
 *       printf( "[%s]\n", name );
 *     }
 *     ZnkDir_closeDir( id );
 *   }
 *
 *   ���ɁAZnkDir_openDirEx ���g�����������.
 *   ���x�̗�ł́A. �܂��� .. �������ăX�L�b�v����悤�ɂ͂��Ă��Ȃ����߁A
 *   ���[�U�͂��̎�̔����K�v�Ƃ���(�������ʏ�͂��̂悤�Ȃ��Ƃ͕s�v�ł���).
 *
 *   test2()
 *   {
 *     const char* name;
 *     ZnkDirId id = ZnkDir_openDirEx( dir, Znk_NPOS, false );
 *     if( id == NULL ){
 *       return false; // Error
 *     }
 *     while( true ){
 *       name = ZnkDir_readDir( id );
 *       if( name == NULL ){
 *         break; // ���ׂăC�e���[�g��������.
 *       }
 *       if( strcmp( name, "." ) == 0 || strcmp( name, ".." ) == 0 ){
 *         continue;
 *       }
 *       printf( "[%s]\n", name );
 *     }
 *     ZnkDir_closeDir( id );
 *   }
 */
ZnkDirId
ZnkDir_openDirEx( const char *dir_name, size_t dir_name_leng, bool is_skip_dot_and_dot_dot );
Znk_INLINE ZnkDirId
ZnkDir_openDir( const char *dir_name ){
	return ZnkDir_openDirEx( dir_name, Znk_NPOS, true );
}

/**
 * @brief
 *   ZnkDirId �� read ���A�t�@�C�����ւ̃|�C���^��Ԃ�.
 *   ���̃t�@�C������ id���w�������f�[�^�ɓ���镶����o�b�t�@�ւ̃|�C���^�ł���A
 *   ���ς�free���Ă͂Ȃ�Ȃ�. �܂���x���̊֐����Ă΂ꂽ��́A���̓����o�b�t�@��
 *   ���̃t�@�C��/�f�B���N�g�����w���悤�ɓ����Ŏ����I�ɍX�V�����. �]���Ă�����
 *   �f�B���N�g���̏I�[�ɒB���A���̂Ƃ�NULL��Ԃ�.
 *   �����������łȂ�炩��error�����������ꍇ��NULL��Ԃ�.
 *   �ڂ����g�p���@�ɂ��ẮAZnkDir_openDir���Q��.
 */
const char*
ZnkDir_readDir( ZnkDirId id );

/**
 * @brief
 * ZnkDirId ���N���[�Y����.
 * �ڂ����g�p���@�ɂ��ẮAZnkDir_openDir���Q��.
 */
void ZnkDir_closeDir( ZnkDirId id );


/**
 * @brief
 *   dir_path �ɂ��w�肵���f�B���N�g�������ɂ���file/directory��
 *   �����擾����(���̏ꍇ�A. �� .. �͏����ăJ�E���g�����).
 *   �����܂�dir_path�̒����ɂ�����̂����ł���A���̉���sub-directory
 *   �܂ł��ċA�I�ɐ����������ł͂Ȃ����Ƃɒ��ӂ���.
 *
 * @return
 *   �f�B���N�g�������ɂ���file/directory�̌���Ԃ�.
 *   �������Adir_path�Ŏw�肵���f�B���N�g�����I�[�v���ł��Ȃ��ȂǁA
 *   �Ȃ�炩�̃G���[�����������ꍇ�AZnk_NPOS ��Ԃ�.
 *   ���̒l�� 0 �̂Ƃ��Adir_path�������f�B���N�g���͋�ł��邱�Ƃ�����.
 */
Znk_INLINE size_t
ZnkDir_getNumOfDirList( const char* dir_path )
{
	size_t count = 0;
	ZnkDirId dir_id = ZnkDir_openDir(dir_path);
	if( dir_id == NULL ){
		return Znk_NPOS; /* Error */
	} else {
		while( ZnkDir_readDir( dir_id ) ){ ++count; }
		ZnkDir_closeDir( dir_id );
	}
	return count;
}


Znk_EXTERN_C_END

#endif /* INCLUDE_GUARD */
