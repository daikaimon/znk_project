# znk_project
===================================

This is Japanese overview. English version is [here][7].

## znk_project�Ƃ�?
-----------------------------------

"znk_project" �͉�X**���{HTTP������(�ʏ�NHK)**�ɂ��t���[���I�[�v���\�[�X�ȃA�v���P�[�V����
����у��C�u�������J��/�񋟂��郊�|�W�g���ł�.
�����͂��ׂ�libZnk�Ƃ�����{���C�u�����ɂ����Windows �܂���Linux�����ɊJ�����ꂽ���̂ł���܂�.

���̃y�[�W�̉E��ɂ���܂��uClone or download�v�Ƃ����{�^�����N���b�N�����
����ɁuDownload ZIP�v�Ə����ꂽ�����������̂ŁA������N���b�N�����
�v���W�F�N�g�S�̂��A�[�J�C�u�����ŐV��zip�t�@�C�����_�E�����[�h�ł��܂�.

���邢�́A�ȉ��̃����N���������𒼐ڃ_�E�����[�h�\�ł�.

  https://github.com/mr-moai-2016/znk_project/archive/master.zip


���݂��̃��|�W�g���Œ񋟂��Ă���܂����͈̂ȉ��̒ʂ�:

* libZnk : ��{�I��C/C++�p���C�u����  
* Moai: �܂�����܂ꂽ(�΂����)���[�J���v���L�V�G���W��  
* Moai plugins: �f���p��Moai�v���O�C��.
* http_decorator : ~~�}�X�R�b�g~~�V���v����HTTP�N���C�A���g.  

�ȉ��͂����Ɋւ���h�L�������g�ւ̃����N�ꗗ�ƂȂ�܂�: 

* [Moai�ɂ���: ��{����][2]
* [Moai/Virtual USERS�@�\: Moai���g���Čf���̃��[�U�������z��������@�ɂ���][4]  
* [Moai Reference Manual: �S�@�\�̉��][3]
* [Moai FAQ][8]
* [�\�[�X�R�[�h����̃R���p�C�����@][6]
* [�\�[�X��������ы@�\�g���̂��߂̃K�C�h���C��: �v���O���}����][9]
* [libZnk�ɂ���][1]
* [�����[�X�m�[�g][5]


## What's new?
-----------------------------------
* 2016/07/01 Version 1.1.2 �����[�X.  
  �ӂ��΃v���O�C�����C��.
  ����A�ӂ��΂ɂ�HTML�R�����g�A�E�g���ɁA����������Ő���ȕ\����j�Q����p�^�[�����܂܂�Ă��܂�.
  �������������T�j�^�C�Y�@�\�𓱓����܂���.

  �����[�X�m�[�g��[������][5]�ł�.

* 2016/07/03  �y�j���[�X�z  
  **�ӂ��΂̎d�l�ύX�ɂ��A�Â�filter�t�@�C���ł͕\����������肪�������܂�**.  
  **filters/futaba_recv.myf**���ŐV�̂��̂ɒu�������Ă�������. 
  ���邢�͓��t�@�C�����̈ȉ��̕������̂悤�ɏC�����܂�.  

~~~html
  �C���O:
  replace ['</iframe>'] ['</iframe -->']

  �C����:
  replace ['</iframe'] ['</iframe --><noscript></noscript']
~~~

  ����Ɉȉ��̈ꕶ��ǉ����܂�.
~~~html
  replace ['display:none'] ['display:inline']
~~~

  �������A�ӂ��΂̎d�l�͌��ݕp�ɂɕύX����Ă��邽�߁A
  �܂������ɖ�肪��������\��������܂�.
  �Â��o�[�W�������g���Ă�����͍ŐV�̃����[�X(Ver1.1.2)�Ƀo�[�W�����A�b�v���邱�Ƃ������߂��܂�.
  ( �ŐV�̃o�[�W�����ł͂��̖��̏C�����s���Ă��܂� ).

* 2016/07/01 Version 1.1.1 �����[�X.  
  �v���O�C���ɂ�����on_post�֐��̎d�l��ύX.
  �ӂ��΂̐V�����d�l�ɑΉ�.

  �����[�X�m�[�g��[������][5]�ł�.

* 2016/06/30 Version 1.1 �����[�X.  
  �v���Ԃ�̃��W���[�o�[�W�����ԍ������グ�����[�X�ƂȂ�܂�.

  �v���O�C���ɂ�����on_post�֐��̎d�l��ύX.
  �ӂ��΂̐V�����d�l�ɑΉ�.
  libZnk�̃o�[�W������1.1�ɕύX.

* 2016/06/22 Version 1.0.7 �����[�X.  
  �ӂ��΂̐V�����d�l�ɑΉ�.
  POST�ϐ�ptua��navigator.userAgent�̒l��ݒ肷��悤�ӂ��΂̎d�l���ύX����Ă��܂�.
  ����ɑΉ�����C���ƂȂ�܂�.

* 2016/06/21 Version 1.0.6 �����[�X.  
  **Android���[�U�̊F�l�I���܂������܂���**(iPhone�ȕ��̓T�[�Z����)�I
  **���� Moai for Android (�����I�o�[�W����)�̓o��ł�.**
  �����X�J���`�[����Android�����ɃR���p�C���������̂�bin_for_android�Ƃ��Ă��񋟂��܂�.
  �����Ă܂��A���Ȃ���Android�����ɃR���p�C�����邽�߂̎w�j��HowToCompile�ɂĉ�����Ă���܂�.

* 2016/06/13 : Version 1.0.5 �����[�X.  

  Cygwin�ɂ����ăR���p�C�����o���Ȃ��Ȃ��Ă��������C��.
  �܂�Cygwin������Makefile����уR���p�C��/�C���X�g�[���X�N���v�g�Ȃǂ�ǉ�.
  Cygwin���Moai���R���p�C�����Ďg�����������܂������Ȃ������Ƃ������A
  ���Ȃ�ȒP�ɂȂ����Ǝv���܂��̂ŁA���̋@��ɐ���Ƃ���������������.

* 2016/06/01 : Version 1.0.4 �����[�X.  
  ���������Moai�̑傫�ȍ\�z�Ƃ��āu��҂̎�𗣂ꂽ�c�[���v��ڎw���܂�.
  ����̎�ȏC���_�Ƃ��ẮA���t�@�����X�}�j���A���ɂ��鑗�Mfilter�Ɋւ���h�L�������g�̏ڍ׉���
  �V����Cookie�p�t�B���^�w��ł��� cookie_force �̓���������܂�.

* 2016/05/23 : IE7�ȑO�ŉ摜���e���APOST���Ɋ܂܂��filename���t���p�X�̂܂܂ɂȂ��Ă���C���[���o�J�[��&hearts;�ȕs����C��.
  (mIpN0ZXK0����AkQKv21nt0���񂲎w�E���肪�Ƃ��������܂�).

* 2016/05/21 : �����o�[�W����1.0�������[�X����܂���.  

## <a name="license">���C�Z���X
-----------------------------------

  Copyright (c) Nippon HTTP Kenkyujo(NHK)

  ���̃��|�W�g���Œ񋟂����c�[������у��C�u�����͊�{�I�ɂ͂��ׂ�
  NYSL(�ς�Ȃ�Ă��Ȃ�D���ɂ���)�Ƃ������C�Z���X�Œ񋟂���܂�.
  ( NYSL�ɂ��Ă� http://www.kmonos.net/nysl ���Q��. )  

  NYSL�̓p�u���b�N�h���C���Ƃقړ��`�ŁA�Ⴆ�Έȉ��̂��Ƃ����R�ɍs���܂�.
  �܂��������s���ɂ������ĉ�X�ɋ����Ƃ�K�v�͑S���Ȃ����A��X��҂̖��O��
  ���̃��|�W�g���R���ł��邱�Ƃ𖾋L����K�v������܂���.(���L���Ă��悢����ǂ�)
  <ul>
  <li>�Ĕz�z�Ȃǂ����R�ɍs����.</li>
  <li>���p/�񏤗p�Ɋւ�炸���Ȃ��̊J��/�g���c�[���ȂǂŎ��R�ɗ��p�ł���.
      (�ܘ_�A�^G��U���C�Z���X�̂悤�ȃ\�[�X�R�[�h���J�̋`���Ȃ�Ă��̂͂Ȃ�).</li>
  <li>�����Ȃǂ��Ă��Ȃ��̖��O�Ō��J�ł���.
      �ق�̂�����Ƃ̉����ł����Ă��S�R�\��Ȃ�.
	  ����ǂ��납�P�Ɋ֐��̃v���t�B�b�N�X�⃉�C�u��������ς��������̂��̂ł����Ă�OK.
	  �ނ��덬��������邽�߁A���̂悤�ɂ��Ă��ꂽ�����L�. </li>

  </ul>

  �܂��v����Ɉꌾ�Ō����΁u**�Ȃ�ł�OK��!���܂���̍D���ɂ���**�v�Ƃ������Ƃł�.

  �������AlibZnk�ɂ����Ă�zlib�Ƃ������C�u���������̂܂܊ۂ��Ɗ܂܂�Ă���A
  ���̕����ɂ����Ă̂�zlib�̃��C�Z���X(���̃��C�Z���X���قƂ�ǐ������S���Ȃ��悤�Ȃ��̂ł���)
  �ɏ]�����̂Ƃ��܂�.


## <a name="as_is">�Ɛӎ���
-----------------------------------

  �{�\�t�g�E�F�A�́u����̂܂܁v�ŁA�����ł��邩�Öقł��邩���킸�A
  ����̕ۏ؂��Ȃ��񋟂���܂�. �{�\�t�g�E�F�A�̎g�p�ɂ���Đ����邢���Ȃ鑹�Q�ɂ��Ă��A
  ��҂͈�؂̐ӔC�𕉂�Ȃ����̂Ƃ��܂�.

  This software is provided 'as-is', without any express or implied warranty.
  In no event will the authors be held liable for any damages arising
  from the use of this software.



[1]: https://github.com/mr-moai-2016/znk_project/blob/master/src/libZnk/README.md
[2]: https://github.com/mr-moai-2016/znk_project/blob/master/src/moai/README.md
[3]: https://github.com/mr-moai-2016/znk_project/blob/master/src/moai/Reference.md
[4]: https://github.com/mr-moai-2016/znk_project/blob/master/src/virtual_users/VirtualUSERS.md
[5]: https://github.com/mr-moai-2016/znk_project/blob/master/src/ReleaseNote.md
[6]: https://github.com/mr-moai-2016/znk_project/blob/master/src/HowToCompile.md
[7]: https://github.com/mr-moai-2016/znk_project/blob/master/README_en.md
[8]: https://github.com/mr-moai-2016/znk_project/blob/master/src/moai/FAQ.md
[9]: https://github.com/mr-moai-2016/znk_project/blob/master/src/Hacking.md
