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
unsigned int path_offset[] = {0x12141416,0x0E101012,0x1C161600,0x12121416, 
                                    0x0C0E1010,0x12121416,0x0C0E1010,0x201C1C20};

																		

/* OFDM 6,9,12,18,24,36,48,54 */								  
unsigned char offset_OFDM[]={20,20,18,18,
                                    16,16,14,12};
/* HT MCS0-7 MCS8-15 */    
unsigned char offset_MCS[]={20,18,18,16,
								   14,14,12,10,
								   18,18,16,16,
								   14,14,12,10};
/*VHT NSS1 MCS0-9 NSS2 MCS0-9*/ 								   
unsigned char offset_VHT[]={20,18,18,16,
								   14,12,10,2,
								   2,2,20,18,		
								   18,16,14,12,
								   10,2,2,2};
