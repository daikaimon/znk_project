O = .\out_dir
S = .
COMPILER=gcc -Wall -O2
LINKER=gcc
RM=del

BASENAME=zlib
OBJS = \
	$O\adler32.o \
	$O\compress.o \
	$O\crc32.o \
	$O\deflate.o \
	$O\gzio.o \
	$O\infback.o \
	$O\inffast.o \
	$O\inflate.o \
	$O\inftrees.o \
	$O\trees.o \
	$O\uncompr.o \
	$O\zutil.o \

SLIB_FILE=$O\$(BASENAME).a
RES_FILE=$O\$(BASENAME).res

all: $O $(SLIB_FILE)

$O:
	if not exist $O mkdir $O

$(SLIB_FILE): $(OBJS)
	if exist $(SLIB_FILE) $(RM) $(SLIB_FILE)
	ar cru $(SLIB_FILE) $(OBJS)
	ranlib $(SLIB_FILE)

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
	$(COMPILER) -I$S -o $@ -c $<
$O\\%.o: $S\%.cpp
	$(COMPILER) -I$S -o $@ -c $<

$(RES_FILE): $S\$(BASENAME).rc
	windres --input-format=rc --output-format=coff -o$(RES_FILE) $S\$(BASENAME).rc

