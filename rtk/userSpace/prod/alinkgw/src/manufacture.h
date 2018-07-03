/* Mac to vendor /device type */

#ifndef __NKGW_MANUFACTURE_H__
#define __NKGW_MANUFACTURE_H__


#define DEV_TYPE_IPHONE 0
#define DEV_TYPE_ANDROID 1

struct Code_to_Str_t macToTypeList[] =
{
	{ DEV_TYPE_IPHONE, "00:03:93",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:05:02",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:0A:27",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:0A:95",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:0D:93",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:10:FA",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:11:24",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:14:51",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:16:CB",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:17:F2",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:19:E3",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:1B:63",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:1C:B3",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:1D:4F",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:1E:52",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:1E:C2",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:1F:5B",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:1F:F3",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:21:E9",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:22:41",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:23:12",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:23:32",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:23:6C",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:23:DF",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:24:36",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:25:00",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:25:4B",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:25:BC",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:26:08",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:26:4A",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:26:B0",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:26:BB",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:30:65",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:3E:E1",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:50:E4",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:61:71",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:88:65",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:A0:40",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:C6:10",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:F4:B9",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "00:F7:6F",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "04:0C:CE",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "04:15:52",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "04:1E:64",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "04:26:65",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "04:48:9A",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "04:54:53",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "04:DB:56",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "04:E5:36",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "04:F1:3E",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "04:F7:E4",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "08:00:07",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "08:70:45",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "0C:15:39",  "苹果"}, //Apple	
	{ DEV_TYPE_IPHONE, "0C:30:21",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "0C:3E:9F",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "0C:4D:E9",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "0C:74:C2",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "0C:77:1A",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "0C:BC:9F",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "10:00:E0",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "10:1C:0C",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "10:40:F3",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "10:93:E9",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "10:9A:DD",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "10:DD:B1",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "14:10:9F",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "14:5A:05",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "14:8F:C6",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "14:99:E2",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "18:20:32",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "18:34:51",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "18:9E:FC",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "18:AF:61",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "18:AF:8F",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "18:E7:F4",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "18:EE:69",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "18:F6:43",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "1C:1A:C0",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "1C:AB:A7",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "1C:E6:2B",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "20:7D:74",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "20:A2:E4",  "苹果"}, //Apple	
	{ DEV_TYPE_IPHONE, "20:C9:D0",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "24:24:0E",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "24:A0:74",  "苹果"}, //Apple	
	{ DEV_TYPE_IPHONE, "24:A2:E1",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "24:AB:81",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "24:E3:14",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "28:0B:5C",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "28:37:37",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "28:5A:EB",  "苹果"}, //Apple	
	{ DEV_TYPE_IPHONE, "28:6A:B8",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "28:6A:BA",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "28:CF:DA",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "28:CF:E9",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "28:E0:2C",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "28:E1:4C",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "28:E7:CF",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "2C:1F:23",  "苹果"}, //Apple	
	{ DEV_TYPE_IPHONE, "2C:B4:3A",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "2C:BE:08",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "2C:F0:EE",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "30:10:E4",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "30:90:AB",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "30:F7:C5",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "34:12:98",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "34:15:9E",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "34:36:3B",  "苹果"}, //Apple	
	{ DEV_TYPE_IPHONE, "34:51:C9",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "34:A3:95",  "苹果"}, //Apple	
	{ DEV_TYPE_IPHONE, "34:C0:59",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "34:E2:FD",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "38:0F:4A",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "38:48:4C",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "3C:07:54",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "3C:15:C2",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "3C:AB:8E",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "3C:D0:F8",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "3C:E0:72",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "40:30:04",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "40:3C:FC",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "40:6C:8F",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "40:A6:D9",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "40:B3:95",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "40:D3:2D",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "44:2A:60",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "44:4C:0C",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "44:D8:84",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "44:FB:42",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "48:43:7C",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "48:60:BC",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "48:74:6E",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "48:D7:05",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "48:E9:F1",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "4C:7C:5F",  "苹果"}, //Apple	
	{ DEV_TYPE_IPHONE, "4C:8D:79",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "4C:B1:99",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "50:EA:D6",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "54:26:96",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "54:72:4F",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "54:9F:13",  "苹果"}, //Apple	
	{ DEV_TYPE_IPHONE, "54:AE:27",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "54:E4:3A",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "54:EA:A8",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "58:1F:AA",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "58:55:CA",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "58:B0:35",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "5C:59:48",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "5C:8D:4E",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "5C:95:AE",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "5C:96:9D",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "5C:97:F3",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "5C:F5:DA",  "苹果"}, //Apple	
	{ DEV_TYPE_IPHONE, "5C:F9:38",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "60:03:08",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "60:33:4B",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "60:69:44",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "60:92:17",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "60:C5:47",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "60:D9:C7",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "60:F8:1D",  "苹果"}, //Apple	
	{ DEV_TYPE_IPHONE, "60:FA:CD",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "60:FB:42",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "60:FE:C5",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "64:20:0C",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "64:76:BA",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "64:9A:BE",  "苹果"}, //Apple	
	{ DEV_TYPE_IPHONE, "64:A3:CB",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "64:B9:E8",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "64:E6:82",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "68:09:27",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "68:5B:35",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "68:64:4B",  "苹果"}, //Apple	
	{ DEV_TYPE_IPHONE, "68:96:7B",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "68:9C:70",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "68:A8:6D",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "68:AE:20",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "68:D9:3C",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "6C:3E:6D",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "6C:40:08",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "6C:70:9F",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "6C:94:F8",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "6C:C2:6B",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "70:11:24",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "70:14:A6",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "70:3E:AC",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "70:56:81",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "70:73:CB",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "70:CD:60",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "70:DE:E2",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "70:E7:2C",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "74:81:14",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "74:E1:B6",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "74:E2:F5",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "78:31:C1",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "78:3A:84",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "78:6C:1C",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "78:7E:61",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "78:9F:70",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "78:A3:E4",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "78:CA:39",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "78:FD:94",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "7C:11:BE",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "7C:6D:62",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "7C:6D:F8",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "7C:C3:A1",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "7C:C5:37",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "7C:D1:C3",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "7C:F0:5F",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "7C:FA:DF",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "80:00:6E",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "80:49:71",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "80:92:9F",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "80:BE:05",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "80:E6:50",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "80:EA:96",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "84:29:99",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "84:38:35",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "84:78:8B",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "84:85:06",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "84:8E:0C",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "84:B1:53",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "84:FC:FE",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "88:1F:A1",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "88:53:95",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "88:63:DF",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "88:C6:63",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "88:CB:87",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "8C:00:6D",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "8C:29:37",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "8C:2D:AA",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "8C:58:77",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "8C:7B:9D",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "8C:7C:92",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "8C:FA:BA",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "90:27:E4",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "90:3C:92",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "90:72:40",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "90:84:0D",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "90:8D:6C",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "90:B2:1F",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "90:B9:31",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "90:FD:61",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "94:94:26",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "94:E9:6A",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "98:03:D8",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "98:5A:EB",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "98:B8:E3",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "98:D6:BB",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "98:E0:D9",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "98:F0:AB",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "98:FE:94",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "9C:04:EB",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "9C:20:7B",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "9C:35:EB",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "9C:F3:87",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "A0:18:28",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "A0:99:9B",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "A0:ED:CD",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "A4:31:35",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "A4:5E:60",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "A4:67:06",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "A4:B1:97",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "A4:C3:61",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "A4:D1:D2",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "A8:20:66",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "A8:5B:78",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "A8:66:7F",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "A8:86:DD",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "A8:88:08",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "A8:8E:24",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "A8:8E:24",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "A8:96:8A",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "A8:BB:CF",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "A8:FA:D8",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "AC:3C:0B",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "AC:7F:3E",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "AC:87:A3",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "AC:CF:5C",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "AC:FD:EC",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "B0:34:95",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "B0:65:BD",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "B0:9F:BA",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "B4:18:D1",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "B4:F0:AB",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "B8:09:8A",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "B8:17:C2",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "B8:78:2E",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "B8:8D:12",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "B8:C7:5D",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "B8:E8:56",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "B8:F6:B1",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "B8:FF:61",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "BC:3B:AF",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "BC:4C:C4",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "BC:52:B7",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "BC:67:78",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "BC:92:6B",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "C0:1A:DA",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "C0:63:94",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "C0:84:7A",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "C0:9F:42",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "C0:CE:CD",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "C0:F2:FB",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "C4:2C:03",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "C8:1E:E7",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "C8:2A:14",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "C8:33:4B",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "C8:6F:1D",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "C8:85:50",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "C8:B5:B7",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "C8:BC:C8",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "C8:E0:EB",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "C8:F6:50",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "CC:08:E0",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "CC:29:F5",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "CC:78:5F",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "D0:03:4B",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "D0:23:DB",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "D0:4F:7E",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "D0:A6:37",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "D0:E1:40",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "D4:9A:20",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "D4:F4:6F",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "D8:00:4D",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "D8:1D:72",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "D8:30:62",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "D8:96:95",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "D8:9E:3F",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "D8:A2:5E",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "D8:BB:2C",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "D8:CF:9C",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "D8:D1:CB",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "DC:2B:61",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "DC:37:14",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "DC:86:D8",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "DC:9B:9C",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "E0:66:78",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "E0:B5:2D",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "E0:B9:BA",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "E0:C9:7A",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "E0:F5:C6",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "E0:F8:47",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "E4:25:E7",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "E4:8B:7F",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "E4:98:D6",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "E4:C6:3D",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "E4:CE:8F",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "E8:04:0B",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "E8:06:88",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "E8:80:2E",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "E8:8D:28",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "EC:35:86",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "EC:85:2F",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "F0:24:75",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "F0:99:BF",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "F0:B4:79",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "F0:C1:F1",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "F0:CB:A1",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "F0:D1:A9",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "F0:DB:E2",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "F0:DB:F8",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "F0:DC:E2",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "F0:F6:1C",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "F4:1B:A1",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "F4:37:B7",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "F4:F1:5A",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "F4:F9:51",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "F8:1E:DF",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "F8:27:93",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "FC:25:3F",  "苹果"}, //Apple
	{ DEV_TYPE_IPHONE, "FC:E9:98",  "苹果"}, //Apple
	
	{ DEV_TYPE_ANDROID, "00:9E:C8", "小米"}, //Xiaomi inc.
	{ DEV_TYPE_ANDROID, "0C:1D:AF", "小米"}, //Xiaomi inc.
	{ DEV_TYPE_ANDROID, "F8:A4:5F", "小米"}, //Xiaomi inc.
	{ DEV_TYPE_ANDROID, "14:F6:5A", "小米"}, //Xiaomi inc.
	{ DEV_TYPE_ANDROID, "18:59:36", "小米"}, //Xiaomi inc.
	{ DEV_TYPE_ANDROID, "28:E3:1F", "小米"}, //Xiaomi inc.
	{ DEV_TYPE_ANDROID, "64:09:80", "小米"}, //Xiaomi inc.
	{ DEV_TYPE_ANDROID, "64:B4:73", "小米"}, //Xiaomi inc.
	{ DEV_TYPE_ANDROID, "68:DF:DD", "小米"}, //Xiaomi inc.
	{ DEV_TYPE_ANDROID, "74:51:BA", "小米"}, //Xiaomi inc.
	{ DEV_TYPE_ANDROID, "7C:1D:D9", "小米"}, //Xiaomi inc.
	{ DEV_TYPE_ANDROID, "8C:BE:BE", "小米"}, //Xiaomi inc.
	{ DEV_TYPE_ANDROID, "98:FA:E3", "小米"}, //Xiaomi inc.
	{ DEV_TYPE_ANDROID, "A0:86:C6", "小米"}, //Xiaomi inc.
	{ DEV_TYPE_ANDROID, "AC:F7:F3", "小米"}, //Xiaomi inc.
	{ DEV_TYPE_ANDROID, "C4:6A:B7", "小米"}, //Xiaomi inc.
	{ DEV_TYPE_ANDROID, "D4:97:0B", "小米"}, //Xiaomi inc.
	{ DEV_TYPE_ANDROID, "F8:A4:5F", "小米"}, //Xiaomi inc.
	
	{ DEV_TYPE_ANDROID, "38:BC:1A", "魅族"},
	
	{DEV_TYPE_ANDROID, "00:18:82", "华为"},
	{DEV_TYPE_ANDROID, "00:1E:10", "华为"},	
	{DEV_TYPE_ANDROID, "00:22:A1", "华为"},
	{DEV_TYPE_ANDROID, "00:25:68", "华为"},
	{DEV_TYPE_ANDROID, "00:25:9E", "华为"},
	{DEV_TYPE_ANDROID, "00:46:4B", "华为"},
	{DEV_TYPE_ANDROID, "00:66:4B", "华为"},
	{DEV_TYPE_ANDROID, "00:E0:FC", "华为"},
	{DEV_TYPE_ANDROID, "04:BD:70", "华为"},
	{DEV_TYPE_ANDROID, "04:C0:6F", "华为"},
	{DEV_TYPE_ANDROID, "04:F9:38", "华为"},
	{DEV_TYPE_ANDROID, "08:19:A6", "华为"},
	{DEV_TYPE_ANDROID, "08:63:61", "华为"},
	{DEV_TYPE_ANDROID, "08:7A:4C", "华为"},
	{DEV_TYPE_ANDROID, "08:E8:4F", "华为"},
	{DEV_TYPE_ANDROID, "0C:37:DC", "华为"},
	{DEV_TYPE_ANDROID, "0C:96:BF", "华为"},
	{DEV_TYPE_ANDROID, "10:1B:54", "华为"},
	{DEV_TYPE_ANDROID, "10:47:80", "华为"},
	{DEV_TYPE_ANDROID, "10:51:72", "华为"},
	{DEV_TYPE_ANDROID, "10:C6:1F", "华为"},
	{DEV_TYPE_ANDROID, "14:B9:68", "华为"},
	{DEV_TYPE_ANDROID, "18:C5:8A", "华为"},
	{DEV_TYPE_ANDROID, "1C:1D:67", "华为"},
	{DEV_TYPE_ANDROID, "1C:8E:5C", "华为"},
	{DEV_TYPE_ANDROID, "20:08:ED", "华为"},
	{DEV_TYPE_ANDROID, "20:0B:C7", "华为"},
	{DEV_TYPE_ANDROID, "20:2B:C1", "华为"},
	{DEV_TYPE_ANDROID, "20:F3:A3", "华为"},
	{DEV_TYPE_ANDROID, "24:09:95", "华为"},
	{DEV_TYPE_ANDROID, "24:69:A5", "华为"},
	{DEV_TYPE_ANDROID, "24:7F:3C", "华为"},
	{DEV_TYPE_ANDROID, "24:9E:AB", "华为"},
	{DEV_TYPE_ANDROID, "24:DB:AC", "华为"},
	{DEV_TYPE_ANDROID, "28:31:52", "华为"},
	{DEV_TYPE_ANDROID, "28:3C:E4", "华为"},
	{DEV_TYPE_ANDROID, "28:5F:DB", "华为"},
	{DEV_TYPE_ANDROID, "28:6E:D4", "华为"},
	{DEV_TYPE_ANDROID, "30:87:30", "华为"},
	{DEV_TYPE_ANDROID, "30:D1:7E", "华为"},
	{DEV_TYPE_ANDROID, "30:F3:35", "华为"},
	{DEV_TYPE_ANDROID, "34:00:A3", "华为"},
	{DEV_TYPE_ANDROID, "34:6B:D3", "华为"},
	{DEV_TYPE_ANDROID, "34:CD:BE", "华为"},
	{DEV_TYPE_ANDROID, "38:F8:89", "华为"},
	{DEV_TYPE_ANDROID, "3C:47:11", "华为"},
	{DEV_TYPE_ANDROID, "3C:DF:BD", "华为"},
	{DEV_TYPE_ANDROID, "3C:F8:08", "华为"},
	{DEV_TYPE_ANDROID, "40:4D:8E", "华为"},
	{DEV_TYPE_ANDROID, "40:CB:A8", "华为"},
	{DEV_TYPE_ANDROID, "44:55:B1", "华为"},
	{DEV_TYPE_ANDROID, "48:46:FB", "华为"},
	{DEV_TYPE_ANDROID, "48:62:76", "华为"},
	{DEV_TYPE_ANDROID, "4C:1F:CC", "华为"},
	{DEV_TYPE_ANDROID, "4C:54:99", "华为"},
	{DEV_TYPE_ANDROID, "4C:8B:EF", "华为"},
	{DEV_TYPE_ANDROID, "4C:B1:6C", "华为"},
	{DEV_TYPE_ANDROID, "50:9F:27", "华为"},
	{DEV_TYPE_ANDROID, "54:39:DF", "华为"},
	{DEV_TYPE_ANDROID, "54:89:98", "华为"},
	{DEV_TYPE_ANDROID, "54:A5:1B", "华为"},
	{DEV_TYPE_ANDROID, "58:1F:28", "华为"},
	{DEV_TYPE_ANDROID, "58:2A:F7", "华为"},
	{DEV_TYPE_ANDROID, "58:7F:66", "华为"},
	{DEV_TYPE_ANDROID, "5C:4C:A9", "华为"},
	{DEV_TYPE_ANDROID, "5C:7D:5E", "华为"},
	{DEV_TYPE_ANDROID, "5C:B4:3E", "华为"},
	{DEV_TYPE_ANDROID, "5C:F9:6A", "华为"},
	{DEV_TYPE_ANDROID, "60:DE:44", "华为"},
	{DEV_TYPE_ANDROID, "60:E7:01", "华为"},
	{DEV_TYPE_ANDROID, "64:16:F0", "华为"},
	{DEV_TYPE_ANDROID, "64:3E:8C", "华为"},
	{DEV_TYPE_ANDROID, "68:8F:84", "华为"},
	{DEV_TYPE_ANDROID, "68:A0:F6", "华为"},
	{DEV_TYPE_ANDROID, "70:54:F5", "华为"},
	{DEV_TYPE_ANDROID, "70:72:3C", "华为"},
	{DEV_TYPE_ANDROID, "70:7B:E8", "华为"},
	{DEV_TYPE_ANDROID, "70:A8:E3", "华为"},
	{DEV_TYPE_ANDROID, "74:88:2A", "华为"},
	{DEV_TYPE_ANDROID, "74:A0:63", "华为"},
	{DEV_TYPE_ANDROID, "78:1D:BA", "华为"},
	{DEV_TYPE_ANDROID, "78:6A:89", "华为"},
	{DEV_TYPE_ANDROID, "78:D7:52", "华为"},
	{DEV_TYPE_ANDROID, "78:F5:FD", "华为"},
	{DEV_TYPE_ANDROID, "7C:60:97", "华为"},
	{DEV_TYPE_ANDROID, "80:38:BC", "华为"},
	{DEV_TYPE_ANDROID, "80:71:7A", "华为"},
	{DEV_TYPE_ANDROID, "80:B6:86", "华为"},
	{DEV_TYPE_ANDROID, "80:D0:9B", "华为"},
	{DEV_TYPE_ANDROID, "80:FB:06", "华为"},
	{DEV_TYPE_ANDROID, "84:A8:E4", "华为"},
	{DEV_TYPE_ANDROID, "84:DB:AC", "华为"},
	{DEV_TYPE_ANDROID, "88:53:D4", "华为"},
	{DEV_TYPE_ANDROID, "88:86:03", "华为"},
	{DEV_TYPE_ANDROID, "88:CE:FA", "华为"},
	{DEV_TYPE_ANDROID, "88:E3:AB", "华为"},
	{DEV_TYPE_ANDROID, "8C:34:FD", "华为"},
	{DEV_TYPE_ANDROID, "90:17:AC", "华为"},
	{DEV_TYPE_ANDROID, "90:4E:2B", "华为"},
	{DEV_TYPE_ANDROID, "90:67:1C", "华为"},
	{DEV_TYPE_ANDROID, "94:04:9C", "华为"},
	{DEV_TYPE_ANDROID, "94:77:2B", "华为"},
	{DEV_TYPE_ANDROID, "9C:28:EF", "华为"},
	{DEV_TYPE_ANDROID, "9C:37:F4", "华为"},
	{DEV_TYPE_ANDROID, "9C:C1:72", "华为"},
	{DEV_TYPE_ANDROID, "A4:99:47", "华为"},
	{DEV_TYPE_ANDROID, "AC:4E:91", "华为"},
	{DEV_TYPE_ANDROID, "AC:85:3D", "华为"},
	{DEV_TYPE_ANDROID, "AC:E2:15", "华为"},
	{DEV_TYPE_ANDROID, "AC:E8:7B", "华为"},
	{DEV_TYPE_ANDROID, "B0:5B:67", "华为"},
	{DEV_TYPE_ANDROID, "B4:15:13", "华为"},
	{DEV_TYPE_ANDROID, "B4:30:52", "华为"},
	{DEV_TYPE_ANDROID, "B8:BC:1B", "华为"},
	{DEV_TYPE_ANDROID, "BC:25:E0", "华为"},
	{DEV_TYPE_ANDROID, "BC:76:70", "华为"},
	{DEV_TYPE_ANDROID, "C0:70:09", "华为"},
	{DEV_TYPE_ANDROID, "C4:05:28", "华为"},
	{DEV_TYPE_ANDROID, "C4:07:2F", "华为"},
	{DEV_TYPE_ANDROID, "C8:D1:5E", "华为"},
	{DEV_TYPE_ANDROID, "CC:53:B5", "华为"},
	{DEV_TYPE_ANDROID, "CC:96:A0", "华为"},
	{DEV_TYPE_ANDROID, "CC:A2:23", "华为"},
	{DEV_TYPE_ANDROID, "CC:CC:81", "华为"},
	{DEV_TYPE_ANDROID, "D0:2D:B3", "华为"},
	{DEV_TYPE_ANDROID, "D0:7A:B5", "华为"},
	{DEV_TYPE_ANDROID, "D4:6A:A8", "华为"},
	{DEV_TYPE_ANDROID, "D4:6E:5C", "华为"},
	{DEV_TYPE_ANDROID, "D4:B1:10", "华为"},
	{DEV_TYPE_ANDROID, "D4:F9:A1", "华为"},
	{DEV_TYPE_ANDROID, "D8:49:0B", "华为"},
	{DEV_TYPE_ANDROID, "DC:D2:FC", "华为"},
	{DEV_TYPE_ANDROID, "E0:24:7F", "华为"},
	{DEV_TYPE_ANDROID, "E0:19:1D", "华为"},
	{DEV_TYPE_ANDROID, "E0:24:7F", "华为"},
	{DEV_TYPE_ANDROID, "E0:97:96", "华为"},
	{DEV_TYPE_ANDROID, "E4:68:A3", "华为"},
	{DEV_TYPE_ANDROID, "E8:08:8B", "华为"},
	{DEV_TYPE_ANDROID, "E8:CD:2D", "华为"},
	{DEV_TYPE_ANDROID, "EC:23:3D", "华为"},
	{DEV_TYPE_ANDROID, "EC:CB:30", "华为"},
	{DEV_TYPE_ANDROID, "F4:55:9C", "华为"},
	{DEV_TYPE_ANDROID, "F4:9F:F3", "华为"},
	{DEV_TYPE_ANDROID, "F4:C7:14", "华为"},
	{DEV_TYPE_ANDROID, "F4:DC:F9", "华为"},
	{DEV_TYPE_ANDROID, "F4:E3:FB", "华为"},
	{DEV_TYPE_ANDROID, "F8:01:13", "华为"},
	{DEV_TYPE_ANDROID, "F8:3D:FF", "华为"},
	{DEV_TYPE_ANDROID, "F8:4A:BF", "华为"},
	{DEV_TYPE_ANDROID, "F8:E8:11", "华为"},
	{DEV_TYPE_ANDROID, "FC:48:EF", "华为"},
	{DEV_TYPE_ANDROID, "FC:E3:3C", "华为"},
	
	{ DEV_TYPE_ANDROID, "00:00:F0", "三星"}, //SamsungE # SAMSUNG ELECTRONICS CO., LTD.
	{ DEV_TYPE_ANDROID, "00:02:78", "三星"}, //SamsungE # Samsung Electro-Mechanics Co., Ltd.
	{ DEV_TYPE_ANDROID, "00:07:AB", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "00:09:18", "三星"}, //SamsungT # SAMSUNG TECHWIN CO.,LTD
	{ DEV_TYPE_ANDROID, "00:0D:AE", "三星"}, //SamsungH # SAMSUNG HEAVY INDUSTRIES CO., LTD.
	{ DEV_TYPE_ANDROID, "00:0D:E5", "三星"}, //SamsungT # Samsung Thales
	{ DEV_TYPE_ANDROID, "00:12:47", "三星"}, //SamsungE # Samsung Electronics Co., Ltd.
	{ DEV_TYPE_ANDROID, "00:12:FB", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "00:13:77", "三星"}, //SamsungE # Samsung Electronics CO., LTD
	{ DEV_TYPE_ANDROID, "00:15:99", "三星"}, //SamsungE # Samsung Electronics Co., LTD
	{ DEV_TYPE_ANDROID, "00:15:B9", "三星"}, //SamsungE # Samsung Electronics Co., Ltd.
	{ DEV_TYPE_ANDROID, "00:16:32", "三星"}, //SamsungE # SAMSUNG ELECTRONICS CO., LTD.
	{ DEV_TYPE_ANDROID, "00:16:6B", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "00:16:6C", "三星"}, //SamsungE # Samsung Electonics Digital Video System Division
	{ DEV_TYPE_ANDROID, "00:16:DB", "三星"}, //SamsungE # Samsung Electronics Co., Ltd.
	{ DEV_TYPE_ANDROID, "00:17:C9", "三星"}, //SamsungE # Samsung Electronics Co., Ltd.
	{ DEV_TYPE_ANDROID, "00:17:D5", "三星"}, //SamsungE # Samsung Electronics Co., Ltd.
	{ DEV_TYPE_ANDROID, "00:18:AF", "三星"}, //SamsungE # Samsung Electronics Co., Ltd.
	{ DEV_TYPE_ANDROID, "00:1A:8A", "三星"}, //SamsungE # Samsung Electronics Co., Ltd.
	{ DEV_TYPE_ANDROID, "00:1B:98", "三星"}, //SamsungE # Samsung Electronics Co., Ltd.
	{ DEV_TYPE_ANDROID, "00:1C:43", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "00:1D:25", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "00:1D:F6", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "00:1E:7D", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "00:1E:E1", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "00:1E:E2", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "00:1F:CC", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "00:1F:CD", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "00:21:19", "三星"}, //SamsungE # Samsung Electro-Mechanics
	{ DEV_TYPE_ANDROID, "00:21:4C", "三星"}, //SamsungE # SAMSUNG ELECTRONICS CO., LTD.
	{ DEV_TYPE_ANDROID, "00:21:D1", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "00:21:D2", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "00:23:39", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "00:23:3A", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "00:23:99", "三星"}, //VdDivisi # VD Division, Samsung Electronics Co.
	{ DEV_TYPE_ANDROID, "00:23:C2", "三星"}, //SamsungE # SAMSUNG Electronics. Co. LTD
	{ DEV_TYPE_ANDROID, "00:23:D6", "三星"}, //SamsungE # Samsung Electronics Co.,LTD
	{ DEV_TYPE_ANDROID, "00:23:D7", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "00:24:54", "三星"}, //SamsungE # Samsung Electronics Co., LTD
	{ DEV_TYPE_ANDROID, "00:24:90", "三星"}, //SamsungE # Samsung Electronics Co.,LTD
	{ DEV_TYPE_ANDROID, "00:24:91", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "00:24:E9", "三星"}, //SamsungE # Samsung Electronics Co., Ltd., Storage System Division
	{ DEV_TYPE_ANDROID, "00:25:38", "三星"}, //SamsungE # Samsung Electronics Co., Ltd., Memory Division
	{ DEV_TYPE_ANDROID, "00:25:66", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "00:25:67", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "00:26:37", "三星"}, //SamsungE # Samsung Electro-Mechanics
	{ DEV_TYPE_ANDROID, "00:26:5D", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "00:26:5F", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "00:50:C2", "三星"}, //SamsungH # SAMSUNG HEAVY INDUSTRIES CO.,LTD.
	{ DEV_TYPE_ANDROID, "00:73:E0", "三星"},
	{ DEV_TYPE_ANDROID, "00:E0:64", "三星"}, //SamsungE # SAMSUNG ELECTRONICS
	{ DEV_TYPE_ANDROID, "00:E3:B2", "三星"},
	{ DEV_TYPE_ANDROID, "00:F4:6F", "三星"},	
	{ DEV_TYPE_ANDROID, "04:18:0F", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "04:1B:BA", "三星"},
	{ DEV_TYPE_ANDROID, "04:FE:31", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "08:08:C2", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "08:37:3D", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "08:3D:88", "三星"},
	{ DEV_TYPE_ANDROID, "08:D4:2B", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "08:EE:8B", "三星"},
	{ DEV_TYPE_ANDROID, "08:FC:88", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "0C:14:20", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "0C:71:5D", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "0C:89:10", "三星"}, 
	{ DEV_TYPE_ANDROID, "0C:B3:19", "三星"},
	{ DEV_TYPE_ANDROID, "0C:DF:A4", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "10:1D:C0", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "10:30:47", "三星"},
	{ DEV_TYPE_ANDROID, "10:3B:59", "三星"},
	{ DEV_TYPE_ANDROID, "10:77:B1", "三星"},
	{ DEV_TYPE_ANDROID, "10:92:66", "三星"},
	{ DEV_TYPE_ANDROID, "10:D3:8A", "三星"},
	{ DEV_TYPE_ANDROID, "10:D5:42", "三星"},
	{ DEV_TYPE_ANDROID, "14:49:E0", "三星"},
	{ DEV_TYPE_ANDROID, "14:89:FD", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "14:A3:64", "三星"}, //SamsungE # Samsung Electronics	
	{ DEV_TYPE_ANDROID, "14:B4:84", "三星"},
	{ DEV_TYPE_ANDROID, "14:F4:2A", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "18:1E:B0", "三星"},
	{ DEV_TYPE_ANDROID, "18:22:7E", "三星"},
	{ DEV_TYPE_ANDROID, "18:26:66", "三星"},
	{ DEV_TYPE_ANDROID, "18:3A:2D", "三星"},
	{ DEV_TYPE_ANDROID, "18:3F:47", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "18:46:17", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "18:67:B0", "三星"}, //SamsungE # Samsung Electronics Co.,LTD
	{ DEV_TYPE_ANDROID, "18:83:31", "三星"},
	{ DEV_TYPE_ANDROID, "18:E2:C2", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "1C:5A:3E", "三星"}, //SamsungE # Samsung Eletronics Co., Ltd (Visual Display Divison)
	{ DEV_TYPE_ANDROID, "1C:62:B8", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "1C:66:AA", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "1C:AF:05", "三星"},
	{ DEV_TYPE_ANDROID, "20:13:E0", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "20:64:32", "三星"}, //SamsungE # SAMSUNG ELECTRO MECHANICS CO.,LTD.
	{ DEV_TYPE_ANDROID, "20:6E:9C", "三星"},
	{ DEV_TYPE_ANDROID, "20:D3:90", "三星"},
	{ DEV_TYPE_ANDROID, "20:D5:BF", "三星"}, //SamsungE # Samsung Eletronics Co., Ltd
	{ DEV_TYPE_ANDROID, "24:4B:03", "三星"},
	{ DEV_TYPE_ANDROID, "24:4B:81", "三星"},
	{ DEV_TYPE_ANDROID, "24:C6:96", "三星"},
	{ DEV_TYPE_ANDROID, "24:DB:ED", "三星"},
	{ DEV_TYPE_ANDROID, "24:F5:AA", "三星"},
	{ DEV_TYPE_ANDROID, "28:98:7B", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "28:BA:B5", "三星"},
	{ DEV_TYPE_ANDROID, "28:CC:01", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "2C:44:01", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "30:19:66", "三星"},
	{ DEV_TYPE_ANDROID, "30:C7:AE", "三星"},
	{ DEV_TYPE_ANDROID, "30:C7:AE", "三星"},
	{ DEV_TYPE_ANDROID, "30:CD:A7", "三星"}, //SamsungE # Samsung Electronics ITS, Printer division
	{ DEV_TYPE_ANDROID, "30:D5:87", "三星"},
	{ DEV_TYPE_ANDROID, "30:D6:C9", "三星"},
	{ DEV_TYPE_ANDROID, "34:23:BA", "三星"},
	{ DEV_TYPE_ANDROID, "34:31:11", "三星"},
	{ DEV_TYPE_ANDROID, "34:AA:8B", "三星"},
	{ DEV_TYPE_ANDROID, "34:BE:00", "三星"},
	{ DEV_TYPE_ANDROID, "34:C3:AC", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "38:01:97", "三星"}, //ToshibaS # Toshiba Samsung Storage Technolgoy Korea Corporation
	{ DEV_TYPE_ANDROID, "38:0A:94", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "38:0B:40", "三星"},
	{ DEV_TYPE_ANDROID, "38:16:D1", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "38:2D:D1", "三星"},
	{ DEV_TYPE_ANDROID, "38:94:96", "三星"},
	{ DEV_TYPE_ANDROID, "38:AA:3C", "三星"}, //SamsungE # SAMSUNG ELECTRO-MECHANICS
	{ DEV_TYPE_ANDROID, "38:EC:E4", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "3C:5A:37", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "3C:62:00", "三星"}, //SamsungE # Samsung electronics CO., LTD
	{ DEV_TYPE_ANDROID, "3C:8B:FE", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "3C:A1:0D", "三星"},
	{ DEV_TYPE_ANDROID, "40:0E:85", "三星"},
	{ DEV_TYPE_ANDROID, "44:4E:1A", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "44:6D:6C", "三星"},
	{ DEV_TYPE_ANDROID, "44:F4:59", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "48:44:F7", "三星"}, //SamsungE # Samsung Electronics Co., LTD
	{ DEV_TYPE_ANDROID, "4C:3C:16", "三星"},
	{ DEV_TYPE_ANDROID, "4C:A5:6D", "三星"},
	{ DEV_TYPE_ANDROID, "4C:BC:A5", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "50:01:BB", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "50:32:75", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "50:56:BF", "三星"},
	{ DEV_TYPE_ANDROID, "50:85:69", "三星"},
	{ DEV_TYPE_ANDROID, "50:A4:C8", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "50:B7:C3", "三星"}, //SamsungE # Samsung Electronics Co.,LTD
	{ DEV_TYPE_ANDROID, "50:CC:F8", "三星"}, //SamsungE # Samsung Electro Mechanics
	{ DEV_TYPE_ANDROID, "50:F5:20", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "50:FC:9F", "三星"},
	{ DEV_TYPE_ANDROID, "54:88:0E", "三星"},
	{ DEV_TYPE_ANDROID, "54:92:BE", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "54:9B:12", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "54:FA:3E", "三星"},
	{ DEV_TYPE_ANDROID, "58:C3:8B", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "5C:0A:5B", "三星"}, //SamsungE # SAMSUNG ELECTRO-MECHANICS CO., LTD.
	{ DEV_TYPE_ANDROID, "5C:2E:59", "三星"},
	{ DEV_TYPE_ANDROID, "5C:3C:27", "三星"},
	{ DEV_TYPE_ANDROID, "5C:A3:9D", "三星"}, //SamsungE # SAMSUNG ELECTRO-MECHANICS CO., LTD.
	{ DEV_TYPE_ANDROID, "5C:E8:EB", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "5C:F6:DC", "三星"},
	{ DEV_TYPE_ANDROID, "60:6B:BD", "三星"}, //SamsungE # Samsung Electronics Co., LTD
	{ DEV_TYPE_ANDROID, "60:77:E2", "三星"},
	{ DEV_TYPE_ANDROID, "60:8F:5C", "三星"},
	{ DEV_TYPE_ANDROID, "60:A1:0A", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "60:AF:6D", "三星"},
	{ DEV_TYPE_ANDROID, "60:D0:A9", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "64:6C:B2", "三星"},
	{ DEV_TYPE_ANDROID, "64:77:91", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "64:B3:10", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "64:B8:53", "三星"},
	{ DEV_TYPE_ANDROID, "68:05:71", "三星"},
	{ DEV_TYPE_ANDROID, "68:48:98", "三星"},
	{ DEV_TYPE_ANDROID, "68:EB:AE", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "6C:2F:2C", "三星"},
	{ DEV_TYPE_ANDROID, "6C:83:36", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "6C:B7:F4", "三星"},
	{ DEV_TYPE_ANDROID, "6C:F3:73", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "70:F9:27", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "74:45:8A", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "74:5F:00", "三星"}, //SamsungS # Samsung Semiconductor Inc.
	{ DEV_TYPE_ANDROID, "78:1F:DB", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "78:25:AD", "三星"}, //SamsungE # SAMSUNG ELECTRONICS CO., LTD.
	{ DEV_TYPE_ANDROID, "78:40:E4", "三星"},
	{ DEV_TYPE_ANDROID, "78:47:1D", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "78:52:1A", "三星"},
	{ DEV_TYPE_ANDROID, "78:59:5E", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "78:9E:D0", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "778:A8:73", "三星"},
	{ DEV_TYPE_ANDROID, "78:AB:BB", "三星"},
	{ DEV_TYPE_ANDROID, "78:D6:F0", "三星"}, //SamsungE # Samsung Electro Mechanics
	{ DEV_TYPE_ANDROID, "78:F7:BE", "三星"},
	{ DEV_TYPE_ANDROID, "7C:F8:54", "三星"},
	{ DEV_TYPE_ANDROID, "80:18:A7", "三星"}, //SamsungE # Samsung Eletronics Co., Ltd
	{ DEV_TYPE_ANDROID, "80:57:19", "三星"},
	{ DEV_TYPE_ANDROID, "84:0B:2D", "三星"}, //SamsungE # SAMSUNG ELECTRO-MECHANICS CO., LTD
	{ DEV_TYPE_ANDROID, "84:25:DB", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "84:38:38", "三星"},
	{ DEV_TYPE_ANDROID, "84:51:81", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "84:55:A5", "三星"},
	{ DEV_TYPE_ANDROID, "84:A4:66", "三星"},
	{ DEV_TYPE_ANDROID, "88:32:9B", "三星"}, //SamsungE # Samsung Electro Mechanics co.,LTD.
	{ DEV_TYPE_ANDROID, "88:9B:39", "三星"},
	{ DEV_TYPE_ANDROID, "8C:71:F8", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "8C:77:12", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "8C:BF:A6", "三星"},
	{ DEV_TYPE_ANDROID, "8C:C8:CD", "三星"}, //SamsungE # Samsung Electronics Co., LTD
	{ DEV_TYPE_ANDROID, "90:00:DB", "三星"},
	{ DEV_TYPE_ANDROID, "90:18:7C", "三星"}, //SamsungE # Samsung Electro Mechanics co., LTD.
	{ DEV_TYPE_ANDROID, "90:F1:AA", "三星"},
	{ DEV_TYPE_ANDROID, "94:01:C2", "三星"},
	{ DEV_TYPE_ANDROID, "94:35:0A", "三星"},
	{ DEV_TYPE_ANDROID, "94:51:03", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "94:63:D1", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "94:D7:71", "三星"},
	{ DEV_TYPE_ANDROID, "98:0C:82", "三星"}, //SamsungE # Samsung Electro Mechanics
	{ DEV_TYPE_ANDROID, "98:1D:FA", "三星"},
	{ DEV_TYPE_ANDROID, "98:52:B1", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "98:83:89", "三星"},
	{ DEV_TYPE_ANDROID, "9C:02:98", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "9C:3A:AF", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "9C:65:B0", "三星"}, 
	{ DEV_TYPE_ANDROID, "9C:D3:5B", "三星"},
	{ DEV_TYPE_ANDROID, "9C:E6:E7", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "A0:07:98", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "A0:0B:BA", "三星"}, //SamsungE # SAMSUNG ELECTRO-MECHANICS
	{ DEV_TYPE_ANDROID, "A0:21:95", "三星"}, //SamsungE # Samsung Electronics Digital Imaging
	{ DEV_TYPE_ANDROID, "A0:75:91", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "A0:82:1F", "三星"},
	{ DEV_TYPE_ANDROID, "A0:B4:A5", "三星"},
	{ DEV_TYPE_ANDROID, "A4:9A:58", "三星"},
	{ DEV_TYPE_ANDROID, "A4:EB:D3", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "A8:06:00", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "A8:7C:01", "三星"},
	{ DEV_TYPE_ANDROID, "A8:9F:BA", "三星"},
	{ DEV_TYPE_ANDROID, "A8:F2:74", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "AC:36:13", "三星"},
	{ DEV_TYPE_ANDROID, "B0:C4:E7", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "B0:C5:59", "三星"},
	{ DEV_TYPE_ANDROID, "B0:D0:9C", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "B0:DF:3A", "三星"},
	{ DEV_TYPE_ANDROID, "B0:EC:71", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "B4:07:F9", "三星"}, //SamsungE # SAMSUNG ELECTRO-MECHANICS
	{ DEV_TYPE_ANDROID, "B4:3A:28", "三星"},
	{ DEV_TYPE_ANDROID, "B4:62:93", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "B4:79:A7", "三星"},
	{ DEV_TYPE_ANDROID, "B4:EF:39", "三星"},
	{ DEV_TYPE_ANDROID, "B8:5A:73", "三星"},
	{ DEV_TYPE_ANDROID, "B8:5E:7B", "三星"},
	{ DEV_TYPE_ANDROID, "B8:6C:E8", "三星"},
	{ DEV_TYPE_ANDROID, "B8:C6:8E", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "B8:D9:CE", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "BC:20:A4", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "BC:44:86", "三星"},
	{ DEV_TYPE_ANDROID, "BC:47:60", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "BC:72:B1", "三星"},
	{ DEV_TYPE_ANDROID, "BC:79:AD", "三星"},
	{ DEV_TYPE_ANDROID, "BC:85:1F", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "BC:8C:CD", "三星"}, 
	{ DEV_TYPE_ANDROID, "BC:B1:F3", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "C0:65:99", "三星"},
	{ DEV_TYPE_ANDROID, "C0:BD:D1", "三星"},
	{ DEV_TYPE_ANDROID, "C4:42:02", "三星"},
	{ DEV_TYPE_ANDROID, "C4:50:06", "三星"},
	{ DEV_TYPE_ANDROID, "C4:57:6E", "三星"},
	{ DEV_TYPE_ANDROID, "C4:62:EA", "三星"},
	{ DEV_TYPE_ANDROID, "C4:73:1E", "三星"}, //SamsungE # Samsung Eletronics Co., Ltd
	{ DEV_TYPE_ANDROID, "C4:88:E5", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "C8:14:79", "三星"},
	{ DEV_TYPE_ANDROID, "C8:19:F7", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "C8:7E:75", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "C8:A8:23", "三星"},
	{ DEV_TYPE_ANDROID, "C8:BA:94", "三星"},
	{ DEV_TYPE_ANDROID, "CC:05:1B", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "CC:07:AB", "三星"},
	{ DEV_TYPE_ANDROID, "CC:3A:61", "三星"}, //SamsungE # SAMSUNG ELECTRO MECHANICS CO., LTD.
	{ DEV_TYPE_ANDROID, "CC:F9:E8", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "CC:FE:3C", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "D0:17:6A", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "D0:22:BE", "三星"},
	{ DEV_TYPE_ANDROID, "D0:59:E4", "三星"},
	{ DEV_TYPE_ANDROID, "D0:66:7B", "三星"}, //SamsungE # Samsung Electronics Co., LTD
	{ DEV_TYPE_ANDROID, "D0:C1:B1", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "D0:DF:C7", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "D4:87:D8", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "D4:88:90", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "D4:E8:B2", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "D8:31:CF", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "D8:57:EF", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "D8:90:E8", "三星"},
	{ DEV_TYPE_ANDROID, "DC:71:44", "三星"}, //SamsungE # Samsung Electro Mechanics
	{ DEV_TYPE_ANDROID, "E0:99:71", "三星"},
	{ DEV_TYPE_ANDROID, "E0:CB:EE", "三星"},
	{ DEV_TYPE_ANDROID, "E4:12:1D", "三星"},
	{ DEV_TYPE_ANDROID, "E4:32:CB", "三星"},
	{ DEV_TYPE_ANDROID, "E4:40:E2", "三星"},
	{ DEV_TYPE_ANDROID, "E4:58:E7", "三星"},
	{ DEV_TYPE_ANDROID, "E4:7C:F9", "三星"}, //SamsungE # Samsung Electronics Co., LTD
	{ DEV_TYPE_ANDROID, "E4:92:FB", "三星"},
	{ DEV_TYPE_ANDROID, "E4:B0:21", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "E4:E0:C5", "三星"}, //SamsungE # Samsung Electronics Co., LTD
	{ DEV_TYPE_ANDROID, "E4:F8:EF", "三星"}, 
	{ DEV_TYPE_ANDROID, "E8:03:9A", "三星"}, //SamsungE # Samsung Electronics Co.,LTD
	{ DEV_TYPE_ANDROID, "E8:11:32", "三星"}, //SamsungE # Samsung Electronics Co.,LTD
	{ DEV_TYPE_ANDROID, "E8:4E:84", "三星"}, 
	{ DEV_TYPE_ANDROID, "E8:50:8B", "三星"},
	{ DEV_TYPE_ANDROID, "E8:E5:D6", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "EC:E0:9B", "三星"}, //SamsungE # Samsung electronics CO., LTD
	{ DEV_TYPE_ANDROID, "F0:08:F1", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "F0:25:B7", "三星"},
	{ DEV_TYPE_ANDROID, "F0:5A:09", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "F0:6B:CA", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "F0:72:8C", "三星"},
	{ DEV_TYPE_ANDROID, "F0:E7:7E", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "F4:09:D8", "三星"}, 
	{ DEV_TYPE_ANDROID, "F4:7B:5E", "三星"}, //SamsungE # Samsung Eletronics Co., Ltd
	{ DEV_TYPE_ANDROID, "F4:9F:54", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "F4:D9:FB", "三星"}, //SamsungE # Samsung Electronics CO., LTD
	{ DEV_TYPE_ANDROID, "F8:04:2E", "三星"}, 
	{ DEV_TYPE_ANDROID, "F8:84:F2", "三星"},
	{ DEV_TYPE_ANDROID, "F8:D0:BD", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	{ DEV_TYPE_ANDROID, "FC:00:12", "三星"}, //ToshibaS # Toshiba Samsung Storage Technolgoy Korea Corporation
	{ DEV_TYPE_ANDROID, "FC:19:10", "三星"},
	{ DEV_TYPE_ANDROID, "FC:1F:19", "三星"}, //SamsungE # SAMSUNG ELECTRO-MECHANICS CO., LTD.
	{ DEV_TYPE_ANDROID, "FC:8F:90", "三星"}, 
	{ DEV_TYPE_ANDROID, "FC:A1:3E", "三星"}, //SamsungE # Samsung Electronics
	{ DEV_TYPE_ANDROID, "FC:C7:34", "三星"}, //SamsungE # Samsung Electronics Co.,Ltd
	
	{ DEV_TYPE_ANDROID, "00:06:1B", "联想"}, //NotebookNotebook
	{ DEV_TYPE_ANDROID, "00:12:FE", "联想"}, //LenovoMo
	{ DEV_TYPE_ANDROID, "00:59:07", "联想"}, //Lenovoe
	{ DEV_TYPE_ANDROID, "14:36:C6", "联想"}, //LenovoMoL
	{ DEV_TYPE_ANDROID, "14:9F:E8", "联想"}, //LenovoMo
	{ DEV_TYPE_ANDROID, "20:76:93", "联想"}, //LenovoBeLenovo
	{ DEV_TYPE_ANDROID, "44:80:EB", "联想"}, 
	{ DEV_TYPE_ANDROID, "50:3C:C4", "联想"}, //LenovoMoLenovo
	{ DEV_TYPE_ANDROID, "60:99:D1", "联想"}, //Vuzix/LeVuzix
	{ DEV_TYPE_ANDROID, "60:D9:A0", "联想"}, //LenovoMoLenovo
	{ DEV_TYPE_ANDROID, "6C:5F:1C", "联想"}, //LenovoMoLenovo
	{ DEV_TYPE_ANDROID, "70:72:0D", "联想"}, //LenovoMoLenovo
	{ DEV_TYPE_ANDROID, "74:04:2B", "联想"},
	{ DEV_TYPE_ANDROID, "80:CF:41", "联想"}, //LenovoMoLenovo
	{ DEV_TYPE_ANDROID, "88:70:8C", "联想"}, //LenovoMoLenovo
	{ DEV_TYPE_ANDROID, "98:FF:D0", "联想"}, //LenovoMoLenovo
	{ DEV_TYPE_ANDROID, "A4:8C:DB", "联想"}, //Lenovo
	{ DEV_TYPE_ANDROID, "AC:38:70", "联想"}, //LenovoMoLenovo
	{ DEV_TYPE_ANDROID, "C8:DD:C9", "联想"}, //LenovoMoLenovo
	{ DEV_TYPE_ANDROID, "CC:07:E4", "联想"}, //LenovoMoLenovo
	{ DEV_TYPE_ANDROID, "D4:22:3F", "联想"}, //LenovoMoLenovo
	{ DEV_TYPE_ANDROID, "D8:71:57", "联想"}, //LenovoMoLenovo
	{ DEV_TYPE_ANDROID, "E4:90:7E", "联想"},
	{ DEV_TYPE_ANDROID, "E8:91:20", "联想"},
	{ DEV_TYPE_ANDROID, "EC:89:F5", "联想"}, //LenovoMoLenovo
	{ DEV_TYPE_ANDROID, "F8:CF:C5", "联想"},
	
	{DEV_TYPE_ANDROID, "00:06:5B", "Dell"}, //DellCompDell
	{DEV_TYPE_ANDROID, "00:08:74", "Dell"}, //DellCompDell
	{DEV_TYPE_ANDROID, "00:0B:DB", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "00:0D:56", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "00:0F:1F", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "00:11:43", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "00:12:3F", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "00:13:72", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "00:14:22", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "00:15:C5", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "00:16:F0", "Dell"}, //Dell
	{DEV_TYPE_ANDROID, "00:18:8B", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "00:19:B9", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "00:1A:A0", "Dell"}, //DellDell	
	{DEV_TYPE_ANDROID, "00:1C:23", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "00:1D:09", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "00:1E:4F", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "00:1E:C9", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "00:21:70", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "00:21:9B", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "00:22:19", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "00:23:AE", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "00:24:E8", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "00:25:64", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "00:26:B9", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "00:B0:D0", "Dell"}, //DellCompDell
	{DEV_TYPE_ANDROID, "00:C0:4F", "Dell"}, //DellCompDELL
	{DEV_TYPE_ANDROID, "10:98:36", "Dell"}, 
	{DEV_TYPE_ANDROID, "14:FE:B5", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "18:03:73", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "18:A9:9B", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "18:FB:7B", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "20:47:47", "Dell"},
	{DEV_TYPE_ANDROID, "24:B6:FD", "Dell"}, //DellDell	
	{DEV_TYPE_ANDROID, "28:C8:25", "Dell"},
	{DEV_TYPE_ANDROID, "34:17:EB", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "34:E6:D7", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "44:A8:42", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "4C:76:25", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "54:9F:35", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "5C:26:0A", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "5C:F9:DD", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "74:86:7A", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "74:E6:E2", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "78:2B:CB", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "78:45:C4", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "84:2B:2B", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "84:8F:69", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "90:B1:1C", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "98:90:96", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "A4:1F:72", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "A4:BA:DB", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "B0:83:FE", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "B8:2A:72", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "B8:AC:6F", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "B8:CA:3A", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "BC:30:5B", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "C8:1F:66", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "D0:67:E5", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "D4:AE:52", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "D4:BE:D9", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "E0:DB:55", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "EC:F4:BB", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "F0:1F:AF", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "F0:4D:A2", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "F8:B1:56", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "F8:BC:12", "Dell"}, //DellDell
	{DEV_TYPE_ANDROID, "F8:DB:88", "Dell"}, //DellDell
	
	{DEV_TYPE_ANDROID, "00:02:55", "IBM"}, //IbmIBM
	{DEV_TYPE_ANDROID, "00:04:AC", "IBM"}, //IbmIBM
	{DEV_TYPE_ANDROID, "00:06:29", "IBM"}, //IbmIBM
	{DEV_TYPE_ANDROID, "00:09:6B", "IBM"}, //IbmIBM
	{DEV_TYPE_ANDROID, "00:0D:60", "IBM"}, //IbmIBM
	{DEV_TYPE_ANDROID, "00:10:D9", "IBM"}, //IbmJapanIBM
	{DEV_TYPE_ANDROID, "00:11:25", "IBM"}, //IbmIBM
	{DEV_TYPE_ANDROID, "00:14:5E", "IBM"}, //IbmIBM
	{DEV_TYPE_ANDROID, "00:17:EF", "IBM"}, //IbmIBM
	{DEV_TYPE_ANDROID, "00:18:B1", "IBM"}, //IbmIBM
	{DEV_TYPE_ANDROID, "00:1A:64", "IBM"}, //IbmIBM
	{DEV_TYPE_ANDROID, "00:20:35", "IBM"}, //IbmIBM
	{DEV_TYPE_ANDROID, "00:21:5E", "IBM"}, //IbmIBM
	{DEV_TYPE_ANDROID, "00:22:00", "IBM"}, //IbmIBM
	{DEV_TYPE_ANDROID, "00:25:03", "IBM"}, //IbmIBM
	{DEV_TYPE_ANDROID, "00:50:76", "IBM"}, //IbmIBM
	{DEV_TYPE_ANDROID, "00:60:94", "IBM"}, //IbmIBM
	{DEV_TYPE_ANDROID, "08:00:5A", "IBM"}, //IbmIBM
	{DEV_TYPE_ANDROID, "08:17:F4", "IBM"}, //IbmIBM
	{DEV_TYPE_ANDROID, "10:00:5A", "IBM"}, //IbmIBM
	{DEV_TYPE_ANDROID, "34:40:B5", "IBM"},
	{DEV_TYPE_ANDROID, "40:F2:E9", "IBM"},
	{DEV_TYPE_ANDROID, "5C:F3:FC", "IBM"}, //IbmIBM
	{DEV_TYPE_ANDROID, "6C:AE:8B", "IBM"}, //IbmIBM
	{DEV_TYPE_ANDROID, "74:99:75", "IBM"}, //IbmIBM
	{DEV_TYPE_ANDROID, "98:BE:94", "IBM"},
	{DEV_TYPE_ANDROID, "A8:97:DC", "IBM"},        //Ibm
	{DEV_TYPE_ANDROID, "E4:1F:13", "IBM"}, //IbmIBM
	{DEV_TYPE_ANDROID, "FC:CF:62", "IBM"}, //IbmIBM

	{DEV_TYPE_ANDROID, "00:00:95", "SONY"}, //SonyTektSONY
	{DEV_TYPE_ANDROID, "00:01:4A", "SONY"}, //SonySony
	{DEV_TYPE_ANDROID, "00:04:1F", "SONY"}, //SonyCompSony
	{DEV_TYPE_ANDROID, "00:0A:D9", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "00:0E:07", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "00:0F:DE", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "00:12:EE", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "00:13:15", "SONY"}, //SonyCompSONY
	{DEV_TYPE_ANDROID, "00:13:A9", "SONY"}, //SonySony
	{DEV_TYPE_ANDROID, "00:15:C1", "SONY"}, //SonyCompSONY
	{DEV_TYPE_ANDROID, "00:16:20", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "00:16:B8", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "00:18:13", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "00:19:63", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "00:19:C5", "SONY"}, //SonyCompSONY
	{DEV_TYPE_ANDROID, "00:1A:75", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "00:1A:80", "SONY"}, //SonySony
	{DEV_TYPE_ANDROID, "00:1B:59", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "00:1C:A4", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "00:1D:0D", "SONY"}, //SonyCompSony
	{DEV_TYPE_ANDROID, "00:1D:28", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "00:1D:BA", "SONY"}, //SonySony
	{DEV_TYPE_ANDROID, "00:1E:45", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "00:1E:DC", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "00:1F:A7", "SONY"}, //SonyCompSony
	{DEV_TYPE_ANDROID, "00:1F:E4", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "00:21:9E", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "00:22:98", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "00:22:A6", "SONY"}, //SonyCompSony
	{DEV_TYPE_ANDROID, "00:23:45", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "00:23:F1", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "00:24:8D", "SONY"}, //SonyCompSony
	{DEV_TYPE_ANDROID, "00:24:BE", "SONY"}, //SonySony
	{DEV_TYPE_ANDROID, "00:24:EF", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "00:25:E7", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "00:D9:D1", "SONY"}, //SonyCompSony
	{DEV_TYPE_ANDROID, "00:EB:2D", "SONY"}, //SonyMobiSony
	{DEV_TYPE_ANDROID, "08:00:46", "SONY"}, //SonySony
	{DEV_TYPE_ANDROID, "18:00:2D", "SONY"}, //SonyMobiSony
	{DEV_TYPE_ANDROID, "1C:7B:21", "SONY"}, //SonyMobiSony
	{DEV_TYPE_ANDROID, "20:54:76", "SONY"}, //SonyMobiSony
	{DEV_TYPE_ANDROID, "24:21:AB", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "28:0D:FC", "SONY"}, //SonyCompSony
	{DEV_TYPE_ANDROID, "30:17:C8", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "30:39:26", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "30:75:12", "SONY"}, //SonyMobiSony
	{DEV_TYPE_ANDROID, "30:A8:DB", "SONY"}, //SonyMobiSony
	{DEV_TYPE_ANDROID, "30:F9:ED", "SONY"}, //SonySony
	{DEV_TYPE_ANDROID, "3C:07:71", "SONY"}, //SonySony
	{DEV_TYPE_ANDROID, "40:2B:A1", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "40:B8:37", "SONY"},
	{DEV_TYPE_ANDROID, "44:74:6C", "SONY"}, //SonyMobiSony
	{DEV_TYPE_ANDROID, "44:D4:E0", "SONY"}, //SonyMobiSony
	{DEV_TYPE_ANDROID, "4C:21:D0", "SONY"}, //SonyMobiSony
	{DEV_TYPE_ANDROID, "54:42:49", "SONY"}, //SonySony
	{DEV_TYPE_ANDROID, "54:53:ED", "SONY"}, //SonySony
	{DEV_TYPE_ANDROID, "58:17:0C", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "5C:B5:24", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "68:76:4F", "SONY"}, //SonyMobiSony
	{DEV_TYPE_ANDROID, "6C:0E:0D", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "6C:23:B9", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "70:9E:29", "SONY"}, //SonyCompSony
	{DEV_TYPE_ANDROID, "78:84:3C", "SONY"}, //SonySony
	{DEV_TYPE_ANDROID, "84:00:D2", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "84:8E:DF", "SONY"}, //SonyMobiSony
	{DEV_TYPE_ANDROID, "8C:64:22", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "90:C1:15", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "94:CE:2C", "SONY"}, //SonyMobiSony
	{DEV_TYPE_ANDROID, "A0:E4:53", "SONY"}, //SonyMobiSony
	{DEV_TYPE_ANDROID, "A8:E3:EE", "SONY"}, //SonyCompSony
	{DEV_TYPE_ANDROID, "AC:9B:0A", "SONY"},
	{DEV_TYPE_ANDROID, "B4:52:7D", "SONY"}, //SonyMobiSony
	{DEV_TYPE_ANDROID, "B4:52:7E", "SONY"}, //SonyMobiSony
	{DEV_TYPE_ANDROID, "B8:F9:34", "SONY"}, //SonyEricSony
	{DEV_TYPE_ANDROID, "BC:6E:64", "SONY"}, //SonyMobiSony
	{DEV_TYPE_ANDROID, "C4:3A:BE", "SONY"},
	{DEV_TYPE_ANDROID, "D0:51:62", "SONY"}, //SonyMobiSony
	{DEV_TYPE_ANDROID, "D8:D4:3C", "SONY"}, //SonySony
	{DEV_TYPE_ANDROID, "E0:63:E5", "SONY"}, //SonyMobiSony
	{DEV_TYPE_ANDROID, "F0:BF:97", "SONY"}, //SonySony
	{DEV_TYPE_ANDROID, "F8:D0:AC", "SONY"}, //SonyCompSony
	{DEV_TYPE_ANDROID, "FC:0F:E6", "SONY"}, //SonyCompSony
	{DEV_TYPE_ANDROID, "FC:F1:52", "SONY"}, //SonySony
	
	{DEV_TYPE_ANDROID, "00:15:EB", "中兴"},
	{DEV_TYPE_ANDROID, "00:19:C6", "中兴"},
	{DEV_TYPE_ANDROID, "00:1E:73", "中兴"},
	{DEV_TYPE_ANDROID, "00:22:93", "中兴"},
	{DEV_TYPE_ANDROID, "00:25:12", "中兴"},
	{DEV_TYPE_ANDROID, "00:26:ED", "中兴"},
	{DEV_TYPE_ANDROID, "08:18:1A", "中兴"},
	{DEV_TYPE_ANDROID, "0C:12:62", "中兴"},
	{DEV_TYPE_ANDROID, "14:60:80", "中兴"},
	{DEV_TYPE_ANDROID, "18:44:E6", "中兴"},
	{DEV_TYPE_ANDROID, "20:89:86", "中兴"},
	{DEV_TYPE_ANDROID, "2C:26:C5", "中兴"},
	{DEV_TYPE_ANDROID, "2C:95:7F", "中兴"},
	{DEV_TYPE_ANDROID, "30:F3:1D", "中兴"},
	{DEV_TYPE_ANDROID, "34:4B:50", "中兴"},
	{DEV_TYPE_ANDROID, "34:4D:EA", "中兴"},
	{DEV_TYPE_ANDROID, "34:DE:34", "中兴"},
	{DEV_TYPE_ANDROID, "34:E0:CF", "中兴"},
	{DEV_TYPE_ANDROID, "38:46:08", "中兴"},
	{DEV_TYPE_ANDROID, "48:28:2F", "中兴"},
	{DEV_TYPE_ANDROID, "4C:09:B4", "中兴"},
	{DEV_TYPE_ANDROID, "4C:16:F1", "中兴"},
	{DEV_TYPE_ANDROID, "4C:AC:0A", "中兴"},
	{DEV_TYPE_ANDROID, "4C:CB:F5", "中兴"},
	{DEV_TYPE_ANDROID, "54:22:F8", "中兴"},
	{DEV_TYPE_ANDROID, "68:1A:B2", "中兴"},
	{DEV_TYPE_ANDROID, "6C:8B:2F", "中兴"},
	{DEV_TYPE_ANDROID, "6C:A7:5F", "中兴"},
	{DEV_TYPE_ANDROID, "70:9F:2D", "中兴"},
	{DEV_TYPE_ANDROID, "78:31:2B", "中兴"},	
	{DEV_TYPE_ANDROID, "78:E8:B6", "中兴"},
	{DEV_TYPE_ANDROID, "84:74:2A", "中兴"},
	{DEV_TYPE_ANDROID, "8C:79:67", "中兴"},
	{DEV_TYPE_ANDROID, "8C:E0:81", "中兴"},
	{DEV_TYPE_ANDROID, "90:1D:27", "中兴"},
	{DEV_TYPE_ANDROID, "94:A7:B7", "中兴"},
	{DEV_TYPE_ANDROID, "98:6C:F5", "中兴"},
	{DEV_TYPE_ANDROID, "98:F5:37", "中兴"},
	{DEV_TYPE_ANDROID, "9C:A9:E4", "中兴"},
	{DEV_TYPE_ANDROID, "9C:D2:4B", "中兴"},
	{DEV_TYPE_ANDROID, "A0:EC:80", "中兴"},
	{DEV_TYPE_ANDROID, "A4:7E:39", "中兴"},
	{DEV_TYPE_ANDROID, "A8:A6:68", "中兴"},
	{DEV_TYPE_ANDROID, "B0:75:D5", "中兴"},
	{DEV_TYPE_ANDROID, "B4:98:42", "中兴"},
	{DEV_TYPE_ANDROID, "B4:B3:62", "中兴"},
	{DEV_TYPE_ANDROID, "C8:64:C7", "中兴"},
	{DEV_TYPE_ANDROID, "C8:7B:5B", "中兴"},
	{DEV_TYPE_ANDROID, "CC:1A:FA", "中兴"},
	{DEV_TYPE_ANDROID, "CC:7B:35", "中兴"},
	{DEV_TYPE_ANDROID, "D0:15:4A", "中兴"},
	{DEV_TYPE_ANDROID, "D0:5B:A8", "中兴"},
	{DEV_TYPE_ANDROID, "D4:37:D7", "中兴"},
	{DEV_TYPE_ANDROID, "D8:55:A3", "中兴"},
	{DEV_TYPE_ANDROID, "D8:74:95", "中兴"},
	{DEV_TYPE_ANDROID, "DC:02:8E", "中兴"},
	{DEV_TYPE_ANDROID, "E0:C3:F3", "中兴"},
	{DEV_TYPE_ANDROID, "E4:77:23", "中兴"},
	{DEV_TYPE_ANDROID, "EC:1D:7F", "中兴"},
	{DEV_TYPE_ANDROID, "EC:8A:4C", "中兴"},
	{DEV_TYPE_ANDROID, "F0:84:C9", "中兴"},
	{DEV_TYPE_ANDROID, "F4:6D:E2", "中兴"},
	{DEV_TYPE_ANDROID, "F8:DF:A8", "中兴"},
	{DEV_TYPE_ANDROID, "FC:C8:97", "中兴"},

	{-1,	NULL, NULL}
};
#endif
