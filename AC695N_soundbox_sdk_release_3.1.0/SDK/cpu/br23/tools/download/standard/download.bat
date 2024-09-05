@echo off

cd %~dp0

copy ..\..\script.ver .
copy ..\..\uboot.boot .
copy ..\..\tone.cfg .
copy ..\..\cfg_tool.bin .
copy ..\..\app.bin .
copy ..\..\br23loader.bin .
copy ..\..\eq_cfg_hw.bin .
copy ..\..\ota_all.bin .
copy ..\..\ota_nor.bin .


..\..\isd_download.exe -tonorflash -dev br23 -boot 0x12000 -div8 -wait 300 -uboot uboot.boot -app app.bin -res tone.cfg cfg_tool.bin eq_cfg_hw.bin -format all %1
:: -key 193-AC690X-B718.key 
:: -format all
::-reboot 2500

  

@rem ɾ����ʱ�ļ�-format all
if exist *.mp3 del *.mp3 
if exist *.PIX del *.PIX
if exist *.TAB del *.TAB
if exist *.res del *.res
if exist *.sty del *.sty



@rem ���ɹ̼������ļ�
copy ota_all.bin ota.bin
..\..\fw_add.exe -noenc -fw jl_isd.fw  -add ota.bin -type 100 -out jl_isd_all.fw
copy ota_nor.bin ota.bin
..\..\fw_add.exe -noenc -fw jl_isd.fw  -add ota.bin -type 100 -out jl_isd_nor.fw

@rem �������ýű��İ汾��Ϣ�� FW �ļ���
..\..\fw_add.exe -noenc -fw jl_isd_all.fw -add script.ver -out jl_isd_all.fw
..\..\fw_add.exe -noenc -fw jl_isd_nor.fw -add script.ver -out jl_isd_nor.fw



..\..\ufw_maker.exe -fw_to_ufw jl_isd_all.fw
..\..\ufw_maker.exe -fw_to_ufw jl_isd_nor.fw
copy jl_isd_all.ufw update.ufw
copy jl_isd_nor.ufw nor_update.ufw
copy jl_isd_all.fw jl_isd.fw
del jl_isd_all.ufw jl_isd_nor.ufw jl_isd_all.fw jl_isd_nor.fw


@REM ���������ļ������ļ�
::ufw_maker.exe -chip AC800X %ADD_KEY% -output config.ufw -res bt_cfg.cfg

::IF EXIST jl_696x.bin del jl_696x.bin 


@rem ��������˵��
@rem -format vm        //����VM ����
@rem -format cfg       //����BT CFG ����
@rem -format 0x3f0-2   //��ʾ�ӵ� 0x3f0 �� sector ��ʼ�������� 2 �� sector(��һ������Ϊ16���ƻ�10���ƶ��ɣ��ڶ�������������10����)

ping /n 2 127.1>null
IF EXIST null del null

