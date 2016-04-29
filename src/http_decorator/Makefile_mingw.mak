O = .\out_dir
S = .
COMPILER=gcc -Wall -O2
LINKER=gcc

MY_LIBS_ROOT=..
INCLUDE_FLAG+=  \
	-I$(MY_LIBS_ROOT)\libZnk \

BASENAME=http_decorator
OBJS = \
	$O\main.o \

EXE_FILE=$O\$(BASENAME).exe
RES_FILE=$O\$(BASENAME).res

all: $O $(EXE_FILE)

$O:
	if not exist $O mkdir $O

$(EXE_FILE): $(OBJS) $(RES_FILE)
	$(LINKER) -o $(EXE_FILE) $(RES_FILE) $(OBJS) $(MY_LIBS_ROOT)\libZnk\out_dir\Znk-0.9.dll.a -Wl,-dn,-lstdc++

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

