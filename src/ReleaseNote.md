Release Note �ŏI�X�V: 2016/04/29
-----------------------------------

## 2016/04/21 Version 0.8 �����[�X.  
  ��-�e�X�g�o�[�W����.  
  ���񃊃��[�X.

## 2016/04/29 Version 0.9 �����[�X.  
  ��-�e�X�g�o�[�W����.  
  Version 0.8����̏C���_�͈ȉ��̒ʂ�.  
~~~
    * �f�t�H���g���������I�Ɂu���A!!���A!!���A�[�C�v�ɂȂ��Ă��܂��s����C��(���h��.
    * �f�t�H���g�|�[�g��8123����8124�֕ύX(��͂�Polipo�Ɣ��ƕs�ւȂ̂�)
    * config.myf�ɂ�����proxy_next�Ƃ������ڂ͍폜���A�����parent_proxy_list�Ƃ������O�ɕύX.
	* �R�}���h���C����������O��proxy��IP�A�h���X�ƃ|�[�g�ԍ����w�肷�������p�~.
      (�ւ���config.myf��parent_proxy_list�w����g������)
    * config.myf�ɂ�����master_confirm�Ƃ������ڂ��폜���A�����post_confirm�Ƃ������O�ɕύX.
    * analysis.myf�ɂ�����confirm_hosts�Ƃ������ڂ��폜���A�����config.myf���Ɉړ�.
	  ����� post_confirm_hosts �Ƃ������O�ɕύX.
    * ��M�t�B���^�ɂ�����CSS, JS�t�B���^�����T�|�[�g�ł������̂��C��. �@�\����悤�ɂ���.
    * �O���v���L�V�Ɋւ���ݒ�ƒ��p�����܂��@�\���Ă��Ȃ������s����C��.
    * config.myf�ɂ�����proxy_indicating_mode, proxy_minus, proxy_plus�w���ǉ�. 
    * config.myf�ɂ�����enable_log_file��ǉ�.
    * config.myf�ɂ�����acceptable_host, server_name��ǉ�.

    * libZnk�̃o�[�W������0.9�Ƃ���.
      ���̃����[�X�ł̂��ׂẴv���W�F�N�g�͂��̃o�[�W����0.9�x�[�X�œ��삷��.
      ���o�[�W����0.8��dll��so�Ƃ͌݊������Ȃ��̂Œ���.

    * HowToCompile�ɂ����� MinGW���g�p����ꍇ�ɂ����������L�q���C��.
      �����ł��܂��������R���p�C���𓊂�����������ꂽ��\����Ȃ�.
      ZNK_MINGW_DIR ��ݒ肷��ׂ�bat�t�@�C���͐�������bat_utils/setenv_mingw.bat�ƂȂ�܂�.

    * LAN��ɂ����鑼�̃}�V������̐ڑ����T�|�[�g.
      ���o�[�W����0.8�ł͎��}�V���ȊO�͖ⓚ���p�ŎՒf���Ă������A���̐�����r�������`�ƂȂ�.
      ����͂��ƒ�ɂ���Q��ڂR��ڂ̃}�V��������AVisual USERS�̋N�����Ă���P��ڂ̃}�V���֐ڑ�����
      Virtual USERS�̒񋟂���@�\�𗘗p����Ƃ������悤�ȗp�r��z�肵�Ă���.
      ���̐����́Aconfig.myf����acceptable_host�ɂ����Đ؂�ւ��邱�Ƃ��ł���.
      ���̒l��ANY���w�肷�邱�Ƃɂ�葼�̃}�V������̐ڑ����\�ƂȂ�A
      LOOPBACK���w�肷�邱�Ƃɂ��A���o�[�W����0.8�Ɠ��l���}�V���ȊO�͖ⓚ���p�ŎՒf����悤�ɂȂ�.
      �f�t�H���g�ł͈��S�̂��߁A�ꉞLOOPBACK�Ƃ��Ă���.
      ���̒l��ANY�Ƃ���ꍇ�́A���[�^��OS�Ȃǂɂ���t�@�C�A�E�H�[���@�\��
      WAN(�O���C���^�[�l�b�g)����̃|�[�g8124�ւ̐ڑ����ł��Ȃ��悤������Ɩh�䂷�邱��.
      (�ʏ�A�f�t�H���g�ł͖h�䂳��Ă���͂��ł��邯��ǂ�)

    * localhost:8124/config ���\�������Config��ʂ��@�\����.
    * �e��h�L�������g�̐����ƍ��V.
~~~
