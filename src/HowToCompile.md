# �\�[�X�R�[�h�̃R���p�C���菇
-----------------------------------

## <a name="index">�ڎ�
-----------------------------------
* [Windows�ȕ��ցA�͂��߂ẴR�}���h�v�����v�g](#windows_first)
* [Windows�ȕ��ցA�͂��߂Ă�MinGW](#windows_next)
* [MinGW�̓�����@](#get_mingw)
* [Windows��MinGW���g���ꍇ](#use_mingw_on_windows)
* [Windows��VC���g���ꍇ](#use_vc_on_windows)
* [Linux�̏ꍇ](#compile_on_linux)
* [Cygwin�̏ꍇ](#compile_on_cygwin)
* [MSYS1.0��MinGW���g�p����ꍇ](#compile_on_msys10)
* [Windows��BCC5.5(Borland C++ Compiler 5.5)�܂���DMC(Digital Mars C/C++)���g���ꍇ(���܂�)](#use_others_on_windows)
* [Android�ɂ���](#about_android)
* [MacOS�ɂ���](#about_macos)

## <a name="windows_first">Windows�ȕ��ցA�͂��߂ẴR�}���h�v�����v�g
-----------------------------------

  �܂��X�^�[�g���j���[����R�}���h�v�����v�g���J�����@����ł���.
  (**�������炩��b�I**���ĕ��͂��̍��̓X���[����[����](#windows_next)�i�ނ���)

  �u**�X�^�[�g���j���[�H�Ȃ�ł�������́H**�v�ȕ��͉�ʈ�ԍ����ɂ���Windows�̃��S��
  �������{�^��������ł���.  �u����Ȃ��̂͌�������Ȃ��c�v�ȕ��́c��ނ𓾂܂�.
  �u�R�}���h�v�����v�g Windows8�v���L�[���[�h�ɂ��ăO�O���ė~����.

  �X�^�[�g���j���[���J������u�v���O�����v=>�u�A�N�Z�T���v=>�u�R�}���h�v�����v�g�v�̏��őI��.  
  ����Ɛ^�����ȃE�B���h�E���\�������͂��ł���.  ���ꂪ�R�}���h�v�����v�g�ł���I

  Windows7�̏ꍇ�͂��ꂪ�u���ׂẴv���O�����v=>�u�A�N�Z�T���v=>�u�R�}���h�v�����v�g�v�̏��ƂȂ�.  
  Windows10�̏ꍇ�́u���ׂẴA�v���v=>�uWindows �V�X�e���c�[���v=>�u�R�}���h�v�����v�g�v�̏��ƂȂ�.  
  Windows8/8.1�̏ꍇ�́A�X�^�[�g���j���[�ɊY������{�^���Ȃ��̂����A�E�N���b�N��������
  ��ʈ�ԉE���Ɍ����u���ׂẴA�v���v�{�^���������ė~����. ����Ɖ�ʏ�Ɂu�R�}���h�v�����v�g�v
  �̃A�C�R���������̂ł����ɂ킩��Ǝv��. �ǂ����Ă��킩��Ȃ��ꍇ�͂����ł��ꂱ�ꌾ�����
  ��͂�O�O���Ă�����������������낤.  

  �R�}���h�v�����v�g�����ɂ̓L�[�{�[�h���� exit �Ɠ��͂���Enter�L�[��������
  ���ʂɃE�B���h�E�E��ɂ��� x �{�^��������.

  �ȏオ�R�}���h�v�����v�g���J�����@���I
  �S���̏��S�҂̕���**10�񂭂炢�J���ĕ��Ă��J��Ԃ�**�A�m���Ƀ}�X�^�[���Ă������ƃH�I

  <a href="#user-content-index">�ڎ��֖߂�</a>


## <a name="windows_next">Windows�ȕ��ցA�͂��߂Ă�MinGW
-----------------------------------

  �R�}���h�v�����v�g���J����悤�ɂȂ����ł��낤���H
  �܂��ȕ��͂���**10�񂭂炢**���K���Ċm���Ƀ}�X�^�[���Ă������ƃH�I

  �ł͎��ɃR���p�C�����C���X�g�[�����悤.
  **MinGW**�ƌĂ΂�Ă���ł����W���[�Ȃ��̂̈���g��.

  �u[MinGW�̓�����@](#get_mingw)�v�̍��ւƐi����.
  (���ɃC���X�g�[�����Ă�����͂��̍��̓X���[���邱��)

  ���AMinGW�ł͂Ȃ�VC���g�������Ƃ������́u[Windows��VC���g���ꍇ](#use_vc_on_windows)�v���Q�Ƃ��Ă�����������.

  <a href="#user-content-index">�ڎ��֖߂�</a>


## <a name="get_mingw">MinGW�̓�����@:
-----------------------------------
  ���W���[�Ȃ����ɃO�O��΂�����ł����肨��уC���X�g�[�����@��������ꂽ�T�C�g�͂���̂���
  �����ł��ȒP�ɐ������Ă���.

  �܂��_�E�����[�h�T�C�g�ł��� https://sourceforge.net/projects/mingw �փA�N�Z�X.
  ( MinGW�̌����T�C�g�� http://www.mingw.org �Ȃ̂ł��邪�_�E�����[�h����t�@�C���Q��
   ���ׂ�sourceforge�̕��֒u����Ă��� )
  �uDownload mingw-get-setup.exe�v�Ə����ꂽ�΂̃{�^��������̂ł�����N���b�N.
  mingw-get-setup.exe ���_�E�����[�h����. ���Ƃ���C:\MinGW�Ƃ����t�H���_�������
  ���̒��ɕۑ����Ă����Ƃ悢���낤(����̃t�@�C���T�C�Y��85KB�Ƌɂ߂ď�����).

  ����mingw-get-setup.exe �����s����.
  ���̃v���O������MinGW�̃C���X�g�[���������Ń_�E�����[�h����Ƃ��������
  **�C���X�g�[�����C���X�g�[������C���X�g�[��**�ł���(�킩��ɂ��������񂵂���).
  ���̎菇�ōs��.

####
  1. �����ŏ���**�A�z��**Windows���u�Z�L�����e�B�̌x���v��\�����Ă��邩������Ȃ���
     **MinGW�ɂ��̎�̖�肪���낤�͂����Ȃ�**�̂Łu���s�v.

  2. �uMinGW Installation Manager Setup Tool�v�E�B���h�E���\�������̂ŁuInstall�v������.

  3. ���̉�ʂŃC���X�g�[������w��ł���. �f�t�H���g�ł� C:\MinGW�ƂȂ��Ă��邪�uChange�v��
    �����ς��邱�Ƃ��ł���(���̏ꍇ�͈ȉ��̐�����C:\MinGW�ƂȂ��Ă��镔�������̃t�H���_��
    �ǂݑւ��ė~����). �ŏI�I��180MB���x�K�v�ł���̂ŁA����]�T���Ȃ���Α��̃h���C�u�ɕς���
    �Ȃǂ��邱��. ���̑����ɘM��K�v�͂Ȃ��̂ł����ݒ肵�I������uContinue�v������.

  4. �����C:\MinGW�z���փt�@�C���Q�������Ń_�E�����[�h�����. �����Ń_�E�����[�h�����̂�
    MinGW�{�̂ł͂Ȃ����̃C���X�g�[���ł���A���v�T�C�Y�ɂ��Ă��悻2MB�قǂł���. 100%�ƕ\�����ꂽ��A
   �uContinue�v������. �����ŊԈ���āuQuit�v�������ƃE�B���h�E�����Ă��܂����c

    **���S���Ă��������I
    C:\MinGW\bin\mingw-get.exe�����s����Α������\������܂���.**

    ���� C:\MinGW\bin\mingw-get.exe ������MinGW�̖{���̃C���X�g�[���ł���AInstallation Manager
    �ƌĂ΂����̂ł���.

  5. �uMinGW Installation Manager�v�Ə����ꂽ�E�B���h�E���\�������̂ł�������C���X�g�[��������
    ���ڂ�I��. �S���I��ł�����Ƃ�ł��Ȃ����ʂɂȂ邵�A���̕K�v���S���Ȃ��̂ł����ł�
    mingw32-base �� mingw32-g++������I��(���ꂾ���ł����v180MB���x�ƂȂ�A���̂Ƃ��댋�\��
    ���ʂɂ͂Ȃ�.
    �܂������̃y�C���ɂ����āuBasic Setup�v��I�񂾏�Ԃɂ��A�E���̃y�C����mingw32-base�Ƃ���
    �s�̎l�p�`�̂Ƃ�����N���b�N����(���邢�͉E�N���b�N�ł��悢).
    ���̂Ƃ��v���_�E�����j���[���\�������̂ŁuMark for Installation�v��I��.
    �l�p�`�̂Ƃ���ɃI�����W�F�̖��}�[�N���\�������ΑI�����ꂽ��ԂɂȂ������Ƃ��Ӗ�����.
    mingw32-g++�ɂ��Ă����l�ɂ��đI����Ԃɂ��Ă�����.

    �J��Ԃ��������ȊO�͂���Ȃ�. �K�v�ŏ���������X�̃��b�g�[�ł���.
    �����������ꂻ�̑����K�v�ɂȂ邱�Ƃ����邩������Ȃ����c���ꂾ���Ŗ{���ɑ��v�Ȃ̂��H

    **���S���Ă��������I
    ��ł�����xC:\MinGW\bin\mingw-get.exe�����s����΂��ł��ǉ��C���X�g�[���ł��܂���.**

	���̂�����Ɋւ��ĐS�z�ȃv���O���}�̕��͉��L��[�⑫����1](#note1)���Q�Ƃ��ꂽ��.

  6. ���āA�����͐������̂ł��悢����ۂ̃p�b�P�[�W�{�̂��_�E�����[�h����уC���X�g�[������.
    ��ԍ���ɂ���uInstallation�v���j���[���J���A�uApply Changes�v��I��.
    �uSchedule of Pending Actions�v�Ƃ����E�B���h�E�������̂ŁuApply�v�{�^���������Ύ��ۂ�
    �p�b�P�[�W�Q���_�E�����[�h����уC���X�g�[�������.

    �p�b�P�[�W�͖ܘ_���k����Ă���̂Ń_�E�����[�h�͑債�Ď��Ԃ͊|����Ȃ�.
    �l�b�g���[�N�����ʍ��G���Ă��Ȃ��Ȃ�A�M�҂̕n��ȉ���ł��R���قǂŏI�镪�ʂł���.
    �܂��C���X�g�[���Ƃ����Ă�C:\MinGW�z���ɕK�v�Ȉ��k�t�@�C�����𓀂���邾���̂��Ƃł���.
    ��ł��q�ׂ��ʂ�Amingw32-base�����̏ꍇ�A�W�J��͍��v150MB�قǂɂȂ�.

  7. �uApplying Scaduled Changes�v�Ƃ����E�B���h�E���o�āuAll changes were applied successfully;�v
    �ȂǂƏ����Ă���ΐ����ł���. �uClose�v�{�^����������.
    �uMinGW Installation Manager�v�E�B���h�E���܂��\������Ă���Ǝv����������E���x�{�^���ŕ��Ă悢.

##
  ����ɂĈꌏ�����C���X�g�[�������ł���.
  �u[Windows��MinGW���g���ꍇ](#use_mingw_on_windows)�v�̍��ւƐi����.


<a name="note1">**�y�⑫����1�z�v���O���}�ȕ���**
~~~
    mingw32-base�ɂ��C���X�g�[�������͈̂ȉ��ł���.

    * �R���p�C��gcc(C�̂�)
    * �����Jld
    * ���C�u�����A��ar�����ranlib�A
    * make�R�}���h(MinGW�ł�mingw32-make.exe�Ƃ������O�ł���)
    * �f�o�b�Kgdb
    * �W��C���C�u����(���̂Ƃ����I�����N�����̂̓f�t�H���g�ł�msvcrt.dll�ł���)
    * MinGW�p��WindowAPI SDK
    * ���\�[�X�R���p�C��windres(icon�Ȃǂ̐ݒ�ɗ��p)
    * �v���t�@�C��gprof
    * ���̑��኱��GNU���[�e�B���e�B

    ����Amingw32-g++�ɂ��C���X�g�[�������͈̂ȉ��ł���.

    * �R���p�C��gcc(C++�p�̓����R�}���hg++)
    * �W��C++���C�u����(libstdc++)
 
    ����Ă���C++�͑S�����킸�A�܂�{����C���ꂾ����������Ȃ��̂ł���΁A
    ���̂Ƃ���mingw32-base�����ŏ\���ł���A�܂�mingw32-g++�͕s�v�ł���.
    (mingw32-base�����̏ꍇ�A30MB�قǐߖ�ł��đS���ō��v150MB���x�ɂȂ�). 
    �Ƃ͂����A���ۂɂ� C++ �̃R�[�h�� C �ƈꏏ�����ɂȂ��čJ�ɑ�����ꂩ�����Ă���A
    ���ꂪ�R���p�C���ł��Ȃ��悤�ł͍���̂ō���͂�����K�{���ڂƂ���.

    ���Agcc�͗l�X�Ȍ���̃t�����g�G���h�ɂȂ��Ă���AC++���R���p�C������ۂ�
    �R�}���h�Ƃ��Ă�gcc�R�}���h���g���΂悢(gcc�͊g���q�ɂ����C��C++���������I�ɔ��f����).
    ����mingw32-g++���C���X�g�[�����Ă��Ȃ���Ԃ�C++���R���p�C�����悤�Ƃ����
    gcc�͈ȉ��̔��ɂ킩��ɂ����G���[���b�Z�[�W���o���̂Œ��ӂ��K�v��. 

    gcc: error: CreateProcess: No such file or directory

    ���������ꂪ�\�����ꂽ�Ȃ� mingw32-g++���C���X�g�[�����Y��Ă��邩�A
    ���s���Ă��邩���Ă���̂ł�����xmingw-get.exe���N�����Ă�����ăC���X�g�[�����悤.
~~~


<a name="note2">**�y�⑫����2�zMinGW�̃A���C���X�g�[���A�ړ��A�o�b�N�A�b�v�ɂ���**
~~~
    mingw32-base ����� mingw32-g++ �̓N���[�����n���f�B�ł���.
    ����̃C���X�g�[���ɂ����Ă̓��W�X�g���Ȃǂ̏������݂Ȃǂ͍s���Ȃ�.
    �]���ăA���C���X�g�[���������ꍇ�͒P��C:\MinGW�����̃t�H���_���폜����΂悢.
    
    �܂��C���X�g�[��������A���̃t�H���_���ۂ��ƕʂ̏ꏊ�ֈړ����Ă����܂�Ȃ�.
    ���邢�͂��Ȃ��̂Q��ڂ̃}�V���ɁA���̃t�H���_���R�s�[���邾���ł��̃}�V���ł�
    �g�����Ƃ��ł���. �܂�Q��ڂ̃}�V����ōĂя�L�̎菇�ŃC���X�g�[������K�v��
    �Ȃ��Ƃ������Ƃł���(���Ă��悢����ǂ�).
    
    �����ăo�b�N�A�b�v�ɂ��Ă����l�ɍs����. �Ⴆ��MinGW���p�ς݂ɂȂ��Ă��΂炭�g��
    �\�肪�Ȃ��̂ō���PC����폜���������A���x�܂��g�����Ƃ����邩������Ȃ��Ǝv����Ȃ�A
    MinGW�t�H���_�S�̂��ꖇ��CD�ɏĂ��Ȃǂ��ĂƂ��Ă����Ƃ悢. ����K�v�ɂȂ�����
    ���̃t�H���_�����̂܂܊ۂ���PC�փR�s�[����Α����ɕ����ł��邩��ł���.
    
    �_�E�����[�h���ꂽ�A�[�J�C�u�́AC:\MinGW\var\cache\mingw-get\packages���Ɉꎞ�ۑ�
    ����Ă���̂ŁA�C�ɂȂ�Ȃ炻������m�F�����Ƃ悢. �C���X�g�[�������S�ɏI������ł���΁A
    ���̃t�H���_���̃A�[�J�C�u�t�@�C��(�g���q��xz�Alzma�ƂȂ��Ă������)�͍폜���Ă��悢.
    �܂�����A�ʐM�̉ߒ��ł����̃A�[�J�C�u�����Ă��邩�_�E�����[�h���s���S�ł������ꍇ�́A
    �ă_�E�����[�h���K�v�ƂȂ邪�A���̏ꍇ�����̃t�H���_���̊Y���A�[�J�C�u���蓮�ŏ����K�v������.
    ����ɂ͂��̏ꍇ�̓f�t�H���g�ł��g���̃u���E�U�̃L���b�V�����������Ă����K�v������
    ( mingw-get.exe�̓_�E�����[�h���A���g���̃u���E�U�̋@�\������ŗ��p���Ă��邽�߂ł��� ).
~~~

  <a href="#user-content-index">�ڎ��֖߂�</a>


## <a name="use_mingw_on_windows">Windows��MinGW���g���ꍇ
-----------------------------------

  src/bat_utils �t�H���_��setenv_mingw.bat �Ƃ����t�@�C��������.
  ������e�L�X�g�G�f�B�^(�������Ȃ�)�ŊJ���A�ȉ��̍s��K�v�ɉ����ēK�؂ɏC������.

~~~
    set ZNK_MINGW_DIR=C:\MinGW
~~~

  C:\MinGW�̕��������Ȃ����C���X�g�[������MinGW�̃t�H���_�̃p�X�ɏC�����邾���ł���.
  (C:\MinGW�ɃC���X�g�[������Ă���ꍇ�͏C���̕K�v�͂Ȃ�)
  �C�����I�����炱����㏑���ۑ����Asrc�t�H���_�ɂ��� compile_by_mingw.bat��
  �N���b�N���Ď��s���悤.

  MinGW���K�؂ɃC���X�g�[������Ă���Ȃ�΁A���ׂẴR���p�C���������ōs���A
  ���ꂼ��̃v���W�F�N�g�̃t�H���_����out_dir�Ƃ����t�H���_�������I�ɐ�������A
  ���̒��ɖړI��exe�t�@�C����dll����������Ă���͂��ł���.

  ��͂�����src�t�H���_�̈��ɂ���bin_for_win32�t�H���_�ւƃR�s�[���邾���Ȃ̂���
  �ʓ|�Ȃ� src�t�H���_�ɂ��� install_bin_for_win32.bat �����s����΂��̃R�s�[�܂Ŏ����I��
  �s���Ă����̂Ŋ��p����Ƃ悢.

  **�n�C�I���S�҂̕��͂����܂Ńb�I**
  ����ȍ~�͒����҂̕������̓��e�ƂȂ��Ă���܂�.


  **�y�Q�l�zcompile_by_mingw.bat �������ōs���Ă��邱��**

  �����ł�compile_by_mingw.bat ���g�킸�ɐݒ肷����@���q�ׂ�.
  ���������compile_by_mingw.bat �������ōs���Ă��邱�Ƃ̉���ł�����.

  ��{�I�ɂ̓R�}���h�v�����v�g���J���Amingw32-make.exe �����s�ł���悤�Ɋ��ϐ��𐮂�����ŁA
  �ȉ������s����ƃr���h�\�ł���.
~~~
    mingw32-make -f Makefile_mingw.mak
~~~

  �����Ȃ���΂Ȃ�Ȃ����ϐ��́APATH�ł���. 

  ���ϐ�PATH�Ɋւ��ẮA�ȉ��̃R�}���h�����݂���t�H���_�̃p�X�� ; ��؂�Œǉ�����.
~~~
    gcc.exe  ld.exe  ar.exe  ranlib.exe  mingw32-make.exe
~~~
  �Ⴆ�� ����炪 C:\MinGW\bin���ɂ���Ȃ�΁A�R�}���h�v�����v�g��ňȉ��̂悤�ɓ��͂���
  Enter�L�[������.

~~~
    set PATH=C:\MinGW\bin;%PATH%
~~~

  ����ŏ�ŋ����� gcc.exe �Ȃǂ��t���p�X���w�肷�邱�ƂȂ��P��gcc.exe(gcc�ł��悢)��
  ���͂��邾���Ŏ��s�\�ƂȂ�. ���Ɏ��ۂ�gcc�Ɠ��͂���Enter�L�[�������Ă݂悤.
  �ugcc: fatal error: no input files�v�Əo���琬���ł���.
  ����A�ugcc�͓����R�}���h�܂��͊O���R�}���h�`�Ȃ񂽂炩�񂽂�`�Ƃ��ĔF������Ă��܂���v
  �Əo���玸�s�ł���. ��L��PATH�̐ݒ�ɂ����ĉ������ԈႦ�Ă���(�Ⴆ�Αł��ԈႦ�Ȃ�).

  <a href="#user-content-index">�ڎ��֖߂�</a>


## <a name="use_vc_on_windows">Windows��VC���g���ꍇ
-----------------------------------
  ������g���Ă���悤�ȕ��͊��ɂ�����x�킩���Ă���������Ƃ͎v����...

  src�t�H���_�ɂ��� compile_by_vc.bat���N���b�N���Ď��s���悤.
  VC���K�؂ɃC���X�g�[������Ă���Ȃ�΁A���ׂẴR���p�C���������ōs���A
  out_dir���ɖړI��exe�t�@�C����dll�����������͂��ł���.


  **�y�Q�l�zcompile_by_vc.bat �������ōs���Ă��邱��**

  �����ł�compile_by_vc.bat ���g�킸�ɐݒ肷����@���q�ׂ�.
  ���������compile_by_vc.bat �������ōs���Ă��邱�Ƃ̉���ł�����.

  ��{�I�ɂ̓R�}���h�v�����v�g���J���A���ϐ��𐮂�����ŁA�ȉ������s�����
  �r���h�\�ł���.

~~~
    nmake -f Makefile_vc.mak
~~~

  �����Ȃ���΂Ȃ�Ȃ����ϐ��́APATH, INCLUDE, LIB�ł���.  
  �����Ɋւ���ڍׂ̓O�O�b������������������Ȃ����A�ꉞ��������.  

  ���ϐ�PATH�Ɋւ��ẮA�ȉ��̃R�}���h�����݂���t�H���_�̃p�X�� ; ��؂�Œǉ�����.
~~~
    cl.exe  link.exe  lib.exe  nmake.exe
~~~
  
  ���������ꂾ���ł͎��͕s�\���ł���ɂ����̃R�}���h���ˑ�����dll���܂܂��t�H���_��
  ���l�ɒǉ����Ȃ���΂Ȃ�Ȃ�. ���Ȃ��Ƃɂ��̃t�H���_�͂����̃R�}���h�̒u����
  ����t�H���_�Ƃ͕�(����Common7�z���̂ǂ���)�ł������肷��.

  �Ⴆ�� ��L�̃R�}���h�� C:\Program Files\Microsoft Visual Studio 8\VC\bin ���ɂ���Ȃ��
  �R�}���h�v�����v�g��ňȉ��̂悤�ɓ��͂���(�s�̍Ō㖈��Enter�L�[����������).

~~~
    set PATH=C:\Program Files\Microsoft Visual Studio 8\VC\bin;%PATH%
    set PATH=C:\Program Files\Microsoft Visual Studio 8\Common7\IDE;%PATH%
    set PATH=C:\Program Files\Microsoft Visual Studio 8\Common7\Tools%PATH%
    set PATH=C:\Program Files\Microsoft Visual Studio 8\Common7\Tools\Bin;%PATH%
~~~

  ����ŏ�ŋ����� cl.exe �Ȃǂ��t���p�X���w�肷�邱�ƂȂ��P��cl.exe(cl�ł��悢)��
  ���͂��邾���Ŏ��s�\�ƂȂ�. ���ۂ�cl�Ɠ��͂���Enter�L�[�������Ă݂悤.
  �uMicrosoft.. Optimize Compiler...���񂽂炩�񂽂�v�Əo���琬���ł���.

  �������A��L�͂��Ȃ蒷���c  
  ����Ȃ��̖�����͂��Ă͂����Ȃ��̂ŁA�����ň�e�N�j�b�N���Љ�悤.
  �e�L�X�g�G�f�B�^(�������Ȃ�)���J���A��L�̃R�}���h���R�s�y���āA�Ⴆ��my_set_path.bat
  �Ƃ������O�ł����ۑ�����.
  ���Ƃ̓R�}���h�v�����v�g�ォ��ȉ�����͂���΁A��L�̃R�}���h�Q���ꊇ���Ď��s�ł���.
~~~
    call my_set_path.bat
~~~

  ���ϐ�INCLUDE�Ɋւ��ẮA�w�b�_�̑��݂���t�H���_�̃p�X�����ϐ�PATH�Ɠ��l�̌`���Őݒ肷��.
  �����������ĕK�v�Ȃ͕̂W��C���C�u�����̃w�b�_(stdio.h�Ȃ�)�̑��݂���t�H���_��
  WindowsAPI(����͈�ʂ�PlatformSDK�ƌĂ΂��)�̃w�b�_(windows.h�Ȃ�)�̑��݂���t�H���_
  �̂Q�ł���. �Ⴆ�Έȉ��̂悤�ɂȂ�.

~~~
    set INCLUDE=C:\Program Files\Microsoft Visual Studio 8\VC\include;%INCLUDE%
    set INCLUDE=C:\Program Files\Microsoft Visual Studio 8\VC\PlatformSDK\include;%INCLUDE%
~~~

  ���ϐ�LIB�Ɋւ��ẮA���C�u����(�g���qlib)�̑��݂���t�H���_�̃p�X�����ϐ�PATH�Ɠ��l�̌`���Őݒ肷��.
  �����������ĕK�v�Ȃ͕̂W��C���C�u����(libcmt.lib�Ȃ�)�̑��݂���t�H���_��
  WindowsAPI(����͈�ʂ�PlatformSDK�ƌĂ΂��)�̃��C�u����(Kernel32.lib�Ȃ�)�̑��݂���t�H���_
  �̂Q�ł���. �Ⴆ�Έȉ��̂悤�ɂȂ�.

~~~
    set LIB=C:\Program Files\Microsoft Visual Studio 8\VC\lib;%LIB%
    set LIB=C:\Program Files\Microsoft Visual Studio 8\VC\PlatformSDK\lib;%LIB%
~~~

  �Ƃ���Œʏ�͂�������X���蓮�őS���ݒ肷��K�v�͂Ȃ��AVC�̃C���X�g�[���f�B���N�g���z����
  �����������ݒ肷�邽�߂�bat�t�@�C�������݂���̂ł�����g��. 
  �����VC�̃o�[�W�����ɂ���Ē��g���قȂ邪VC6.0�������Avsvars32.bat �Ƃ������O�ɂȂ��Ă���.
  ���AVC7.0�ȍ~�AVC���C���X�g�[�����ꂽ�ꍇ�� VS??COMNTOOLS (??�ɂ̓o�[�W�����ԍ�������)�Ƃ���
  �`���̊��ϐ����ݒ肳���. ����𗘗p����vsvars32.bat�̏ꏊ����肷�邱�Ƃ��ł���.
  �������Ă���X�N���v�g setenv_vc.bat�͂��������������̂ł���.
  ������compile_by_vc.bat�͓����ł�����Ăяo��.

  <a href="#user-content-index">�ڎ��֖߂�</a>


## <a name="compile_on_linux">Linux�̏ꍇ:
-----------------------------------
  ������g���Ă���悤�ȕ��͊��ɂ�����x�킩���Ă���������Ƃ͎v����...

  gcc, g++, make, ld, ar, ranlib �R�}���h�����s�ł���悤�ɕK�v�ɉ����Ă������C���X�g�[������.
  �����͂ǂ�� C/C++�ł̊J���ɂ����čŒ���K�v�ƂȂ��{�R�}���h�΂���ł���.
  �ʂɃC���X�g�[�����Ă��悢���ADevelopment�n�̃J�e�S���ɂ����{�p�b�P�[�W���C���X�g�[�������
  �܂����ׂĎ����ŃC���X�g�[�������Ǝv���i�Ⴆ��Arch Linux�Ȃ� pacman �� base-devel ���C���X�g�[������). 
  ��̓I�ȃC���X�g�[�����@�͂��ꂼ���Linux�f�B�X�g���r���[�V�����ɂ���Đ獷���ʂȂ̂ł����ł͋L�ڂ��Ȃ�. 
  �f�B�X�g���r���[�V�����ɂ���Ă͍ŏ�����C���X�g�[������Ă���\��������.

  ���̏�Ń^�[�~�i��(xterm, urxvt�ȂǂȂ�ł��悢��)���J���A�ȉ��̃V�F���X�N���v�g�����s����.

    ./compile_on_linux.sh

  ���̃V�F���X�N���v�g�ɂ��AMakefile_linux.mak �̑��݂���S�f�B���N�g���֎����I�Ɉړ�����
  make -f Makefile_linux.mak �����s�����. ���̎��_�ŃR���p�C����̃o�C�i���́A�e�f�B���N�g������
  out_dir�Ɋi�[����Ă���.
  (Permission denied�̂悤�ȃG���[���b�Z�[�W���\������Ă��̃V�F���X�N���v�g�����s�ł��Ȃ��ꍇ�́A
  �܂� chmod 755 compile_on_linux.sh �Łu���s�����v��^���A�ēx�����Ă݂悤�I)

  ���ɁA��ŃR���p�C�������S�o�C�i����ݒ�t�@�C�����C���X�g�[������ɂ́A�ȉ������s����΂悢.

	./install_bin_for_linux.sh

  ����ɂ��A���̃f�B���N�g���̈��ɂ��� ../bin_for_linux �֕K�v�Ȃ��ׂẴt�@�C�����C���X�g�[�������.
  (Permission denied�̂悤�ȃG���[���b�Z�[�W���\������Ă��̃V�F���X�N���v�g�����s�ł��Ȃ��ꍇ�́A
  ��Ɠ��l�� chmod 755 install_bin_for_linux.sh �Łu���s�����v��^���A�ēx�����Ă݂悤�I)

  �ȏ�ŃR���p�C������уC���X�g�[���͊����ł���.

  ../bin_for_linux�ֈړ����Amoai���N�����悤.
  �ȉ��̂悤�ȃ��b�Z�[�W���\��������moai�͖����N�����Ă���.

~~~
  Moai : config load OK.
  Moai : target load OK.
  Moai : analysis load OK.
  Moai : Filter Loading [filters/�c.myf]
  Moai : Plugin Loading [plugins/�c.so]
  �c
  Moai : acceptable_host=[LOOPBACK]
  Moai : blocking_mode=[0]
  Moai : AutoGet PrivateIP=[192.168.�c].
  Moai : Listen port 8124...

  Moai : ObserveR : 1-th sock events by select.
~~~

  ���̏�ԂŃu���E�U�ɂ�����v���L�V�ݒ�Őڑ����127.0.0.1:8124�ɐݒ肷��΁A
  moai�����[�J���v���L�V�Ƃ��ċ@�\�����邱�Ƃ��ł���͂��ł���.

  <a href="#user-content-index">�ڎ��֖߂�</a>


## <a name="compile_on_cygwin">Cygwin�̏ꍇ:
-----------------------------------
  ����Cygwin�����p����Ă���AMinGW�Ȃǂ̋����J�������킴�킴�C���X�g�[���������Ȃ�����������Ǝv��.
  ���̍��ڂ͂��̂悤�ȕ������ł���.

> Cygwin�ł͂�₱�������ƂɁA���̏�œ��삷��MinGW���C���X�g�[�����邱�Ƃ��ł��A���̏ꍇgcc�I�v�V�����Ƃ���
>  -mno-cygwin���g�����Ƃ�Cygwin��gcc�ł͂Ȃ�MinGW��gcc���Ăяo�����Ƃ��ł���.
> �����ł͐ݒ�t�@�C���Ȃǃ��_�ɂ�₱�����������Ȃ����߁A���̂悤�Ȏg�����ɂ��Ă͑Ή����Ȃ�.
> �܂菃����Windows�v���O���������p�r�ł͂Ȃ��AUNIX�G�~�����[�^�Ƃ��ăv���O���������Ƃ���Cygwin�{����
> ��|�Ɍ��肷����̂Ƃ���. MinGW���g�������ꍇ�́A��L�ł̐����̂悤�ɃR�}���h�v�����v�g��Ŏg�p���邩�A
> ���邢��MSYS1.0���g���ė~����. ( �ǂ����Ă�Cygwin���MinGW���g����������Makefile_cygwin.mak���e���C������K�v������.
> ���������dll���̔�������R�}���h���C���I�v�V���� -mno-cygwin ��ǉ�������x�̕ύX�ōςނƂ͎v�� ).

  gcc-core, gcc-g++, make, binutils(ld, ar, ranlib, as�R�}���h�Ȃ�)���C���X�g�[������.
  �����͂ǂ�� C/C++�ł̊J���ɂ����čŒ���K�v�ƂȂ��{�p�b�P�[�W�ł���.
  setup-x86.exe �ɂ����Ă������ʂɃC���X�g�[�����Ă��悢���ADevel�J�e�S���ɂ����͑S�Ċ܂܂�Ă���̂�
  �ʓ|�Ȃ炱�̃J�e�S�����ƈꊇ�ŃC���X�g�[�����Ă��悢.

  ���̏��Cygwin�^�[�~�i�����J���A�ȉ��̃V�F���X�N���v�g�����s����.

    ./compile_on_cygwin.sh

  ���̃V�F���X�N���v�g�ɂ��AMakefile_cygwin.mak �̑��݂���S�f�B���N�g���֎����I�Ɉړ�����
  make -f Makefile_cygwin.mak �����s�����. ���̎��_�ŃR���p�C����̃o�C�i���́A�e�f�B���N�g������
  out_dir�Ɋi�[����Ă���.
  ���ACygwin�̏ꍇ�A�o���オ��dll�̖��O�ɂ�cyg�v���t�B�b�N�X�����Ă���.
  ����ΕʂɕK�{�Ƃ����킯�ł͂Ȃ��̂����ACygwin�p��dll��VC��MinGW�ɂ�萶�����镁�ʂ�dll�Ƃ͈ꕔ�قȂ邽�߁A
  ��ʂ��邽�߂ɂ��̂悤�ɂ��Ă���. 

  ���ɁA��ŃR���p�C�������S�o�C�i����ݒ�t�@�C�����C���X�g�[������ɂ́A�ȉ������s����΂悢.

	./install_bin_for_cygwin.sh

  ����ɂ��A���̃f�B���N�g���̈��ɂ��� ../bin_for_cygwin �֕K�v�Ȃ��ׂẴt�@�C�����C���X�g�[�������.

  �ȏ�ŃR���p�C������уC���X�g�[���͊����ł���.

  ../bin_for_cygwin�ֈړ����Amoai���N�����悤.
  �ȍ~��Linux�łƓ����ł���.

  <a href="#user-content-index">�ڎ��֖߂�</a>


## <a name="compile_on_msys10">MSYS1.0�̏ꍇ:
-----------------------------------

  �쐬��.

  <a href="#user-content-index">�ڎ��֖߂�</a>


## <a name="use_others_on_windows">Windows��BCC5.5(Borland C++ Compiler 5.5)�܂���DMC(Digital Mars C/C++)���g���ꍇ(���܂�)
-----------------------------------
  ���Ȃ���MinGW�ł�VC�ł��Ȃ������Ă�����I�ԂƂ����Ȃ�A���͂��X�͉��������܂�.

  ����p��makefile�́A�����΂�̂œ������Ă��Ȃ����A���̂Ƃ���\�[�X�R�[�h���̂�
  �����ł��r���h�ł���悤�ɈڐA���Ɉꉞ�z�����Ă���.
  �q���g�������Ă����̂ŋ������������ȕ��͊e��makefile���쐬���ė~����.

  **BCC5.5**  
  obj�����ꍇ�̎w����@:
~~~
    bcc32 -w -RT -c -oYourFile.obj YouFile.c
~~~

  exe�����ꍇ�̎w����@(OBJS��+�L���Ŋeobj�t�@�C����A���������̂��w�肷��Ƃ����ϑԎd�l�ł���):
~~~
    ilink32 -w -Gn -C -Tpe -ap c0x32.obj $(OBJS) $(LFLAGS),YourApplication.exe,,import32.lib cw32mt.lib,,YourResource.res
~~~

  �ÓI���C�u����(lib)�����ꍇ�̎w����@:
~~~
    tlib YourStaticLib.lib /P4096 $(OBJS)
~~~

  dll����т���ɑΉ�����C���|�[�g���C�u���������ꍇ�̎w����@(���̕��@��__stdcall�ɂ͑Ή����Ȃ�):
~~~
    ilink32 -w -Gn -C -Tpd c0d32x.obj $(OBJS) $(LFLAGS),YourDLL.dll,,import32.lib cw32mt.lib,YourDLL.def
    implib -f -a YourDLL.lib YourDLL.dll
    �܂���
    implib -f -a YourDLL.lib YourDLL.def
~~~

  **DMC(�ŐV�o�[�W�����̂�)**  
  obj�����ꍇ�̎w����@:
~~~
    dmc -HP99 -Bj -j0 -Ab -Ae -Ar -w6 -c -oYourFile.obj YouFile.c
~~~

  exe�����ꍇ�̎w����@:
~~~
    LINK -EXET:NT $(OBJS),YourApplication.exe,,ws2_32.lib user32.lib kernel32.lib,,YourResource.res
~~~

  �ÓI���C�u����(lib)�����ꍇ�̎w����@:
~~~
    lib -c -p4096 YourStaticLib.lib $(OBJS)
~~~

  dll����т���ɑΉ�����C���|�[�g���C�u���������ꍇ�̎w����@:
~~~
    LINK -EXET:NT $(OBJS),YourDLL.dll,,ws2_32.lib user32.lib kernel32.lib,YourDLL.def
    implib /system YourDLL.lib YourDLL.dll
~~~

  <a href="#user-content-index">�ڎ��֖߂�</a>


## <a name="about_android">Android�ɂ���:
-----------------------------------
  Android�͊�{�I��Linux�ł��邪�APC Linux�Ƃ͑傫���قȂ镔��������.
  �����Ēʏ�AAndroid�ɂ�����A�v����Java VM���x�[�X�Ƃ���(�G���g���|�C���g�Ƃ���)�񋟂����.
  Java VM�x�[�X�ȃA�v���ł́A�O���t�B�J���ȃA�C�R�����z�[����ʂɕ\������A
  ������^�b�v���邱�ƂŋN������.

  �������Ȃ���A����r���h���� Moai for Android ��Java VM�x�[�X�ł͂Ȃ�Android��Linux���
  ���ڑ���l�C�e�B�u�A�v���Ƃ���C����ŊJ������. ���̏ꍇ�z�[����ʂ�Moai�p�̓��ʂȃA�C�R�����\������邱�Ƃ͂Ȃ�.
  Moai���N������ɂ́A�܂��[���G�~�����[�^���J���Acd�R�}���h�ɂ�� moai���s�o�C�i�������݂���f�B���N�g��
  (bin_for_android)�ւƈړ����Ă���moai���s�o�C�i���𒼐ڋN������`�ɂȂ�.
  �v����ɖ{����Linux����Windows�̃R�}���h�v�����v�g�Ȃǂ���v���O�������N������v�̂ōs��.

  ���āA�O�u���������Ȃ������A�l�C�e�B�u�A�v���Ƃ���C����̃\�[�X�R�[�h���R���p�C������ɂ�Android NDK���g��.
  Android NDK�� https://developer.android.com/ndk/downloads/index.html ���_�E�����[�h�ł���.
  �܂��J����Android��ł͍s���Ȃ����߁APC���K�v�ł���. �܂�Android NDK��PC�փ_�E�����[�h���APC��ɂĊJ�����s��.

  ������ Android NDK�̃t�@�C���T�C�Y�͂��Ȃ�傫���̂Œ��ӂ��Ăق���. �_�E�����[�h����zip�t�@�C���͖�750MB���x������A
  ����ɂ�����𓀓W�J����ƁA�Ȃ��3GB�قǂɂ��Ȃ�I �܂肱����P���ɍ��v���������ł�
  �f�B�X�N�̋󂫂�4GB���x�K�v�ƂȂ�. �����Windows��ŃR���p�C������ꍇ�A�����Cygwin���g�����Ƃ�z�肷��.
  (Cygwin���g�킸�ɃR���p�C�����邱�Ƃ������炭�\�����A����Ɋւ��Ă͂܂�����/�������ł���)  
  ���AAndroid Studio ����� android-sdk �͍���͑S���g��Ȃ�. �C���X�g�[���������s�v�ł���.
  ���l��JDK(Java Development Kit)��JRE�Ȃǂ��K�v�Ȃ�.

  Android NDK���_�E�����[�h������A������f�B�X�N�։𓀓W�J����.
  �����𓀂ł������낤���H�Ȃɂ��t�@�C���T�C�Y���n���f�J�C���߁A�𓀃c�[���ŃG���[�Ȃǂ��\������Ă��Ȃ����Ƃ�O�̂��ߊm�F���悤.

  ���� android_setting_ndk.sh ���e�L�X�g�G�f�B�^�ŊJ���AZNK_NDK_HOME�̒l��Android NDK���𓀓W�J���ꂽ�f�B���N�g��
  �̃p�X�ɏ���������. �Ⴆ��Windows��C:\android-ndk-r12�t�H���_�ɉ𓀓W�J����Ă���Ȃ�ACygwin��ł̂���������p�X��
  /cygdrive/c/android-ndk-r12�ƂȂ�A�ȉ��̂悤�ɏ���������`�ƂȂ�.
  

~~~
   export ZNK_NDK_HOME=/cygdrive/c/android-ndk-r12
~~~

  ��͈ȉ��̃R�}���h�����ԂɎ��s���Ă����΂悢.
  (Windows�̏ꍇ��Cygwin���N�����Ă�������s���悤.)
~~~
  chmod 755 *.sh
  source ./android_setting_ndk.sh
  ./android_compile.sh
  ./android_install_bin.sh
~~~

  src/libZnk, src/moai �Ȃǂ�android�f�B���N�g�������݂��邪�A���̓����ɂ���jni�f�B���N�g����
  Android NDK�ŃR���p�C�����邽�߂�Makefile���i�[����Ă���.
  android_compile.sh�����s����ƁA�܂�����android �f�B���N�g���ֈړ�����ndk-build�R�}���h���Ăяo�����.
  ���̂Ƃ��A���f�B���N�g���� libs, obj �f�B���N�g������������A�r���h�����o�C�i���⃉�C�u�����Ȃǂ�
  �����ɏo�͂����.
  ./android_install_bin.sh �����s���邱�ƂŁA�����̃o�C�i���� ../bin_for_android�ւƃC���X�g�[�������`�ƂȂ�.


  �Ō�ɏo���オ���� bin_for_android �����@�ɓ]��������ł�������s����.
  ����ɂ͐F�X���@�����邪�A�Ƃ肠������bin_for_android����Uzip�t�@�C���Ɍł߂�Web��ɃA�b�v��
  ���@���炱����_�E�����[�h����Ƃ����N�\��肭�ǂ����@��~~�����Ő�������ɂ�~~��ԊȒP��.
  �܂��ǂ�ȕ��@�ł��悢���Ƃ肠�����Ȃ�Ƃ���������@�ւƓ]�����ė~����.

  ���s����ɂ������Ă͎��̓_�ɒ��ӂ��Ă�����������.

  <ul>
  <li>�z�[���f�B���N�g���ȊO�ł͏������݌�������ю��s�����̊֌W�Ŏ��s�ł��Ȃ�.<br>
      ����Ă܂��u�[���G�~�����[�^�v���J���Acd �R�}���h���z�[���f�B���N�g���ֈړ�����.
      (�P��cd�Ɠ��͂���Enter�������΂悢).<br>
      ������bin_for_android.zip���_�E�����[�h�Ȃǂ����ꍇ�́ADownload�f�B���N�g������z�[���f�B���N�g����
      ������R�s�[���Ȃ���΂Ȃ�Ȃ�.
      Download�f�B���N�g���͒ʏ�A/storage/sdcard0/Download �Ȃǂ̃p�X�ɑ��݂��A���̔z���Ƀ_�E�����[�h����
      �t�@�C�����u����Ă���.
      �ȉ��̂悤�Ɏ��s���邱�Ƃł�����z�[���f�B���N�g���փR�s�[���悤.
~~~
      cp /storage/sdcard0/Download/bin_for_android.zip ~/
~~~
  </li>
  <li>bin_for_android.zip ���z�[���f�B���N�g���փR�s�[������Aunzip bin_for_android.zip �ł�����𓀓W�J���悤.
      ���� cd bin_for_android/armeabi �Ŏ��s�o�C�i���̂���f�B���N�g�����ւƈړ����A
      moai ����� http_decorator �Ɏ��s������^����. 
      �ȉ������s���Ă�����.
~~~
      chmod 755 moai http_decorator
~~~
  </li>
  <li>moai�͋N������libZnk.so�𓮓I���[�h����.
      ����𐬌������邽�߂�LD_LIBRARY_PATH��K�؂Ɏw�肵�Ă����Ȃ���΂Ȃ�Ȃ�.
      �ȉ������s���Ă�����.
~~~
      export LD_LIBRARY_PATH=. 
~~~
  </li>
  </ul>

  ����ł悤�₭������������.
  ./moai �Ɠ��͂��A��L�uLinux�̏ꍇ�v�Ő��������̂Ɠ��l�̃��b�Z�[�W���\�������ΐ����ł���.


  <a href="#user-content-index">�ڎ��֖߂�</a>


## <a name="about_macos">MacOS�ɂ���:
-----------------------------------
  ����ł͖��T�|�[�g�ł���.
  ���ɃR���p�C�������݂�Ȃ�(�茳�ɂȂ��̂Ŗ��m�F�ł��邪)Darwin���g�����ƂɂȂ�Ǝv��.
  �����gcc, make�R�}���h�Ȃ�Linux�Ɠ��l�̃c�[�����܂܂�Ă���A����Makefile_linux.mak��
  �����蒼������Γ��l�̕��@�Ńr���h�ł���͂��ł���.
  (�_�C�i�~�b�N�����N���C�u�����̊g���q��so�ł͂Ȃ�dylib��tbd�Ƃ��������̂ɂȂ�悤�ł���.
  ���̑������Jld�̃I�v�V�����������炭MacOS�Ǝ��̂��̂��w�肵�Ȃ���΂Ȃ�Ȃ����낤)

  <a href="#user-content-index">�ڎ��֖߂�</a>


