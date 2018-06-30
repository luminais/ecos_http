		REALTEK 11nRouter SDK(based on ecos-3.0)-eCos_v1.4
		---------------------------------------------------------

Package List
============
  1. rtl819x_eCos.tar.gz - containing the source code of ecos-3.0 v1.4 sdk
  2. rtl819x-bootcode-eCos-v1.4-8881A.tar.gz 
  3. rtl819x-bootcode-eCos-v1.4-97dl-96e-96eu.tar.gz 
  4. rtl819x-bootcode-eCos-v1.4-97d-8367r.tar.gz
  5. README.txt - this file
  6. INSTALL.txt - how to build code 
  7. Document.tar.gz - containing the documents for this SDK
  8. image.tar.gz	-	containning the images of each kind of combination.
  		        -	The images is specially builded for release.
	image/fw-96e-88e-gw.bin			- gateway image for 8196E+88ER
	image/fw-96e-92e-gw.bin			- gateway image for 8196E+92E
	image/fw-96e-92e-ipv6-gw.bin		- gateway image for 8196E+92E+IPV6
	image/fw-96eu-88e-gw.bin		- gateway image for 8196EU+88ER
	image/fw-8881a-92e.bin			- gateway image for 8881A+92E
	image/fw-8881a-92e-ipv6-gw.bin		- gateway image for 8881A+92E+IPV6
	image/fw-8881a-88e.bin                  - gateway image for 8881A+88E
	image/fw-8881a-CMJ.bin			- gateway image for 8881A_CMJ
	image/fw-8881a-CMJ_rtk_ui.bin		- gateway image for 8881A_CMJ_RTK_UI
	image/fw-8881a-XDG.bin			- gateway image for 8881A_XDG
	image/fw-8881ab-8367rb-92e-gw.bin	- gateway image for 8881A_8367R_92E
	image/fw-8881a-92e-ipv6-middleast.bin   - gateway image for 8881A+92E+IPV6+TR069+middleast
	image/fw-97dl-92e-8812.bin		- gateway image for 97DL+92E+8812
	image/fw-97d-8367r-92e-8812-gw.bin	- gateway image for 97D+8367R+92E+8812

	image/96e_88er_nfjrom			- MP image for 8196E+88ER
	image/96e_92e_nfjrom			- MP image for 8196E+92E
	image/96eu_88e_nfjrom			- MP image for 8196EU+88ER
	image/8881A_92e_nfjrom                  - MP image for 8881A+92E
        image/8881A_88e_nfjrom                  - MP image for 8881A+88E
	image/8881A_cmj_nfjrom			- MP image for 8881A_CMJ
	image/8881a-XDG-mp-nfjrom		- MP image for 8881A_XDG
	image/8881ab-8367rb-92e-mp-nfjrom	- MP image for 8881A_8367R_92E
	image/97dl_92e_8812_nfjrom		- MP image for 97DL+92E+8812
	image/97d-8367r-92e-8812-mp-nfjrom	- MP image for 97D+8367R+92E+8812

	image/boot_96e.bin			- bootloader for 8196E+88ER/8196E+92E/8196E+92E+IPV6
	image/boot_8881AQ.bin			- bootloader for 8881AQ+88E/92E
	image/boot_8881AM.bin			- bootloader for 8881A_CMJ/8881A_XDG
	image/boot_8881ab_8367r.bin		- bootloader for 8881AB+8367R+92E
	image/boot_97dl.bin			- bootloader for 97DL+92E+8812
	image/boot_97d_8367r.bin		- bootloader for 97D+8367R+92E+8812

Environment
===========
  Fedora 9, Ubuntu 8.10/9.10 are recommended


Install the eCos-3.0 sdk package
=============================
  1. Copy 'rtl819x_eCos.tar.gz' to a file directory on a Linux PC
  2. Type 'tar xvzf rtl819x_eCos.tar.gz' to extract the package
 
 Install the bootcode package
============================= 
  Type 'tar xvzf rtl819x-bootcode-eCos-v1.4-97dl-96e-96eu.tar.gz' to extract the package 

build the ecos kernel/bootcode
=============================
  follow the INSTALL.txt file
