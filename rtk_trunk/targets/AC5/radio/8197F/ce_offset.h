/*
path_offset describe
power offset in 0.5db step 

RATE 18_06, 54_24 CCK1 MCS0-3, MCS7_4, MCS11_8,MCS15_12 A11_2B11
(a) means pathA, (b) means pathB, (ab) means pathAB
18   (ab)¡¢ 12    (ab)¡¢ 9     (ab)¡¢  6    (ab)£»
54   (ab)¡¢ 48    (ab)¡¢ 36    (ab)¡¢ 24    (ab)£»
5.5  (b) ¡¢ 2     (b) ¡¢  1    (ab)¡¢ MCS32(ab)£»
MCS3 (ab)¡¢ MCS2 (ab)¡¢ MCS1 (ab)¡¢ MCS0 (ab)£»
MCS7 (ab)¡¢ MCS6 (ab)¡¢ MCS5 (ab)¡¢ MCS4 (ab)£»
MCS11(ab)¡¢ MCS10(ab)¡¢ MCS9 (ab)¡¢ MCS8 (ab)£»
MCS15(ab)¡¢ MCS14(ab)¡¢ MCS13(ab)¡¢ MCS12(ab)£»
11   (a) ¡¢ 5.5   (a) ¡¢  2    (a)¡¢   11   (b)
*/
unsigned int path_offset[] = {0x11131313, 0x0D0E0F10, 0x12121200,0x0F101112, 
                                    0x0C0D0D0E,0x0D0E0F10,0x0A0B0B0C,0x12121212};
	
/* OFDM 6,9,12,18,24,36,48,54 */		
unsigned char offset_OFDM[]={19,19,18,17,
                                   16,14,12,9};
    
/* HT MCS0-7 MCS8-15 */    
unsigned char offset_MCS[]={18,17,17,15,
                                  14, 12, 10, 8,
                                  17,16,16,14,
                                  13, 11, 9, 8};
/*VHT NSS1 MCS0-9 NSS2 MCS0-9*/
unsigned char offset_VHT[]={18,17,17,15,
                                  14,12,10, 8,
                                  6 ,2, 17,16,
                                  16,14,13,11,
                                  9,7, 5, 1};
