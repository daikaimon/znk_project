O = .\out_dir
S = .
COMPILER=gcc -Wall -O2
LINKER=gcc

MY_LIBS_ROOT=..
INCLUDE_FLAG+=  \
	-I$(MY_LIBS_ROOT)\libZnk \

include Makefile_version.mak

BASENAME=2ch
OBJS = \
	$O\dll_main.o \
	$O\main.o \
	$O\init.o \

DLIB_FILE=$O\$(BASENAME).dll
ILIB_FILE=$O\$(BASENAME).dll.a
RES_FILE=$O\$(BASENAME).res
DEF_FILE=.\$(BASENAME).def

all: $O $(DLIB_FILE)

$O:
	if not exist $O mkdir $O

$(DLIB_FILE): $(OBJS)
	gcc -g -Wl,--disable-stdcall-fixup,--kill-at -shared -o $(DLIB_FILE) $(OBJS) $(MY_LIBS_ROOT)\libZnk\out_dir\Znk-$(DL_VER).dll.a $(SUB_LIBS) -lws2_32 $(DEF_FILE)
	dlltool --kill-at --dllname $(BASENAME).dll -d $(DEF_FILE) -l $(ILIB_FILE)

##
# Sub-Directory�Ɋi�[���ꂽ�t�@�C���Ȃǂɂ��_��ɑΉ����邽�߁A
# �����ł̓T�t�B�b�N�X���[���ł͂Ȃ��p�^�[�����[����p����.
# ���̂Ƃ� : ��荶���Ɋւ��Ă� \ �̓G�X�P�[�v�V�[�P���X�Ƃ��ē����A
# ������E���ɂ��Ă̓G�X�P�[�v�V�[�P���X�Ƃ��ē����Ȃ��Ƃ���
# ���ɂ�₱�����d�l�����݂���.
# ����āA�Ⴆ�� $O\\%.o: $S\%.c �̂悤�� : ��荶���Ɋւ��Ă� \\ ��
# ��d�ˁA������E���ɂ��Ă͔��΂� \ ��������ɂ��Ȃ���΂Ȃ�Ȃ�.
#
$O\\%.o: $S\%.c
	$(COMPILER) -I$S $(INCLUDE_FLAG) -o $@ -c $<
$O\\%.o: $S\%.cpp
	$(COMPILER) -I$S $(INCLUDE_FLAG) -o $@ -c $<

$(RES_FILE): $S\$(BASENAME).rc
	windres --input-format=rc --output-format=coff -o$(RES_FILE) $S\$(BASENAME).rc

