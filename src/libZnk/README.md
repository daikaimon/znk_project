# libZnk
===================================

## Overview
-----------------------------------
  libZnk��C/C++��Ŏg���ėp�̊�{�������W�߂��悭����^�C�v�̃��C�u�����ł���.  
  ���̃o�[�W�����ɂ����Ă͋�̓I�ɂ͈ȉ��̂悤�ȏ������T�|�[�g����.  

*   �����n�Ԃ̈Ⴂ���z������w�b�_(Znk_base.h)
*   �ėp�̃o�C�i���z�񂨂�ѕ����񏈗�
*   �t�@�C��IO����уf�B���N�g������
*   �G���[����
*   �\�P�b�g�ʐM�����HTTP�Ɋւ��鏈��
*   zip�����gzip�̉𓀏���(zlib���g�p)
*   ���I���[�h�⏕�@�\
*   ���ϐ�����
*   �ėp�ϐ��A�v���~�e�B�u�^�A����т��̓��I�z��
*   �ėp�̐ݒ�t�@�C��(myf)�p�[�T
*   �~���[�e�b�N�X����уX���b�h
*   ���w�����MD5�v�Z�Ȃ�  

  ���̃��C�u�����͊����Ĕ�r�I���K�͂ɉ������Ă���.
  �Ƃ肠����C�Ńv���O���~���O����ꍇ�A�Œ�����̂��炢�͂Ȃ��Ƙb�ɂȂ�Ȃ��Ƃ�����
  ���x���̂��̂ɗ��߂Ă���(C�p�̃��C�u���������ꉞC++����ł��g���A����p�̃��[�e�B���e�B��
  �኱�����񋟂���. C++��p�̃w�b�_�ɂ��Ă͊g���q��hpp�Ƃ��Ă���).
  �܂����̃��C�u�������g���̂ɑ��̃��C�u�������C���X�g�[������K�v�͊�{�I�ɂȂ�.
  ������zlib���g�p���Ă��邪�A���̃f�B���N�g���z���ɓ������Ă��邽�߁AWindows�ɂ�����
  �킴�킴������C���X�g�[������K�v���Ȃ�.
  Linux�ɂ����Ă͂����libpthread, libdl, libstdc++���g�p���Ă��邪�A�����͂��Ƃ�
  ���Ȃ����ŏ��\����Linux���C���X�g�[�����Ă����Ƃ��Ă��A�܂��m���ɃV�X�e���ɍŏ�����
  �����Ă�����̂΂���ł���.
  ���AMacOS�ł̓���m�F�͂��Ă��Ȃ����A�T�|�[�g���Ă��鏈���͊�{�I��BSD�n�ł����Ȃ�
  ���̂Ȃ̂őΉ�������̂͂���قǓ���͂Ȃ��Ƃ͎v��.


## License
-----------------------------------

  ���̃��C�u������ zlib �ȊO�̕����ɂ��Ă�NYSL(�ς�Ȃ�Ă��Ȃ�D���ɂ���)�ł���.  
  zlib�̕����ɂ��Ă͈ȉ���zlib���C�Z���X�ɏ]��.

~~~
  zlib 1.2.3 : Copyright (C) 1995-2005 Jean-loup Gailly and Mark Adler

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
~~~

  �܂�zlib���C�u�������g���\�t�g�E�F�A�ɏ�L�̒��쌠�\�����L������K�v�͂Ȃ����A
  �����L������Ȃ�A�������L�����Ă����Ȃ���΂Ȃ�Ȃ��Ƃ��������x�̈Ӗ��ł���.
  �����Ă������̃��C�u�����̃\�[�X�R�[�h�����ς��Ďg�p���Ă���ꍇ�́A
  �I���W�i���Ƃ̍���������邽�߈ꉞ���̂��Ƃɂ��Ĉꌾ�����Ă����Ƃ������Ƃł���.

  �悭�킩��Ȃ���΂����ɂ��Ă͂��Ȃ��̃\�t�g�E�F�A�ɂ͉����L��������
  ���C�u�����ɂ��Ă͂��̂܂܂��g�p����΂悢.


## �t��zlib�ɂ���
-----------------------------------

  libZnk��zlib��ÓI�Ɋ܂߂�`�Ŏg�p����.  
  �������Ă���zlib�́A�\�[�X�R�[�h���̂̓I���W�i�������؉��ς��Ă��Ȃ����A
  �t����example��h�L�������g�t�@�C���ނ͏璷�Ȃ̂ŁA���C�Z���X����(README)�ȊO��
  �폜���Ă���.

  libZnk use zlib as statically sub-object.
  zlib in here, its source code itself is not at all modified from its original version,
  but accessory of example and document files are deleted except for the license terms
  (README) to avoid redundancy.


## �Ɛӎ���
-----------------------------------

  �{�\�t�g�E�F�A�́u����̂܂܁v�ŁA�����ł��邩�Öقł��邩���킸�A
  ����̕ۏ؂��Ȃ��񋟂���܂�. �{�\�t�g�E�F�A�̎g�p�ɂ���Đ����邢���Ȃ鑹�Q�ɂ��Ă��A
  ��҂͈�؂̐ӔC�𕉂�Ȃ����̂Ƃ��܂�.

  This software is provided 'as-is', without any express or implied warranty.
  In no event will the authors be held liable for any damages arising
  from the use of this software.

