O = ./out_dir
S = .
COMPILER=gcc -Wall -O2
LINKER=gcc

MY_LIBS_ROOT=..
INCLUDE_FLAG+=  \
	-I$(MY_LIBS_ROOT)/libZnk \

BASENAME=2ch
OBJS = \
	$O/main.o \
	$O/init.o \

DLIB_FILE=$O/cyg$(BASENAME).dll
DEF_FILE=.\$(BASENAME).def

CLEAN_LIST= \
	$O/__sentinel_clean__ \
	./$O/*.a \
	./$O/*.dll \
	./$O/*.o \


all: $O $(DLIB_FILE)

$O:
	mkdir -p $O

$(DLIB_FILE): $(OBJS)
	if test -e $(DLIB_FILE); then rm $(DLIB_FILE); fi
	gcc -shared -o $(DLIB_FILE) $(OBJS) $(MY_LIBS_ROOT)\libZnk\out_dir\cygZnk.dll $(SUB_LIBS)

##
# Sub-Directory�Ɋi�[���ꂽ�t�@�C���Ȃǂɂ��_��ɑΉ����邽�߁A
# �����ł̓T�t�B�b�N�X���[���ł͂Ȃ��p�^�[�����[����p����.
#
$O/%.o: $S/%.c
	$(COMPILER) -I$S $(INCLUDE_FLAG) -o $@ -c $<
$O/%.o: $S/%.cpp
	$(COMPILER) -I$S $(INCLUDE_FLAG) -o $@ -c $<


clean:
	@for tgt in $(CLEAN_LIST) ; do  if test -e "$$tgt" ; then echo "rm $$tgt" ; rm "$$tgt" ; fi ;  done
