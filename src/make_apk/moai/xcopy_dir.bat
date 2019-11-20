@echo off

REM
REM xcopy /E ���u���S�Ɂv�g�����@.
REM
REM   �K���R�s�[�����f�B���N�g���ł��邩�ǂ������܂����肷��.
REM   (�R�s�[���Ƃ��ăt�@�C�����w�肵�Ă��܂����ꍇ�� xcopy�R�}���h�̎��s���m���ɖh��).
REM   ����R�s�[���Ƃ��ăt�@�C�����w�肵�Ă��܂����ꍇ�Axcopy�R�}���h�͎׈��ȋ���������̂Œ��ӂ���.
REM
REM �f�B���N�g��/�t�@�C������@
REM
REM   �܂��Ō���� \ ��t���� exist���肷��.
REM   ����Ńf�B���N�g���̏ꍇ�𒊏o.
REM   ���� \ ��t���� exist���肷��.
REM   ����Ŏc�����t�@�C���̏ꍇ�𒊏o.
REM   �Ō�ɂ��ׂĂɍ��v���Ȃ��ꍇ�͎w�肵���p�X�͑��݂��Ȃ��Ɣ��f����΂悢.
REM

if exist %1\ goto SrcIsDirectory
if exist %1  goto SrcIsFile
goto SrcNotFound

:SrcIsDirectory
if exist %2\ goto DstIsDirectory
if exist %2  goto DstIsFile
if not exist %2\ mkdir %2

:DstIsDirectory
xcopy /H /C /Y /E %1 %2
goto End

:SrcNotFound
echo xcopy_dir Error : Src %1 is not found.
goto End

:SrcIsFile
echo xcopy_dir Error : Src %1 is not directory.
goto End

:DstIsFile
echo xcopy_dir Error : Dst %2 is not directory.
goto End

:End
