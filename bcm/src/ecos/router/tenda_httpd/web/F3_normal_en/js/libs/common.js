//处理ajax 错误时，刷新整个页面
$(document).ajaxError(function () {
	top.location.reload(true);
});

B.setTextDomain("translate");

/******
	网络诊断
	第二个数字表示联网状态 0：已联网  2：错误，未联网 1：尝试联网
********/
var statusMsg = {
	"100": _("You can surf the Internet"),
	"121": _("WAN port unplugged! Please plug the Internet cable into it"),
	"112": _("Checking the user name and password... Please wait. It will take 1~5 minutes"),
	"122": _("IP conflict! Please modify LAN IP"),
	"123": _("Failed! Please confirm your user name and password and try again"),
	"124": _("ERROR: No response from the remote server. Please go through the settings and make sure you have typed everything correctly. If the problem persists, contact your ISP for help"),
	"125": _("Dial-up succeeded, but no Internet access!"),
	"126": _("PPPoE detected. <br>Please select PPPoE and type the correct information manually. Click OK to try to access the Internet"),
	"127": _("Static IP detected. <br>Please select Static IP and configure the IP information manually"),
	"128": _("The router has obtained a valid IP address but cannot access the Internet. Please try the solutions below one by one.") + "<br/>" + "1. <a id='cloneMac' style='text-decoration:underline;' class='text-success' href='javascript:void(0)'>" + _("Clone MAC address") + "</a>." + _("(MAC Clone will work in 30 seconds.)") +
		"<br/>" + _("2. Try another computer and reconfigure the router") + "<br/>" + _("3. Please make sure you have applied a valid Internet service. If not, consult your ISP for help"),
	"129": _("PPPoE detected. <br>Please select PPPoE and type the correct information manually. Click OK to try to access the Internet"),
	"999": _("Disconnected")
};
//显示网络诊断数据
function showWanInternetStatus(statusNumber, textId) {

	var connectStatus = statusNumber.slice(1, 2);
	if (connectStatus == "0") {
		$("#" + textId).attr("class", "text-success");
	} else if (connectStatus == "1") {
		$("#" + textId).attr("class", "text-primary");
	} else {
		$("#" + textId).attr("class", "text-danger");
	}
	$("#" + textId).html(statusMsg[statusNumber]);
}

/********判断是否同网段****************/
function checkIpInSameSegment(ip_lan, mask_lan, ip_wan, mask_wan) {
	if (ip_lan === '' || ip_wan === '') {
		return false;
	}
	var ip1Arr = ip_lan.split("."),
		ip2Arr = ip_wan.split("."),
		maskArr1 = mask_lan.split("."),
		maskArr2 = mask_wan.split("."),
		maskArr = maskArr1,
		i;
	for (i = 0; i < 4; i++) {
		if (maskArr1[i] != maskArr2[i]) {
			if (maskArr1[i] & maskArr2[i] == maskArr1[i]) {
				maskArr = maskArr1;
			} else {
				maskArr = maskArr2;
			}
			break;
		}
	}
	for (i = 0; i < 4; i++) {
		if ((ip1Arr[i] & maskArr[i]) != (ip2Arr[i] & maskArr[i])) {
			return false;
		}
	}
	return true;
}

/***********检查IP 是否为网段或广播IP合法性*/
function checkIsVoildIpMask(ip, mask, str) {
	var ipArry,
		maskArry,
		len,
		maskArry2 = [],
		netIndex = 0,
		netIndex1 = 0,
		broadIndex = 0,
		i = 0;
	str = str || _("IP Address");
	//ip = document.getElementById(ipElem).value;
	//mask = document.getElementById(maskElem).value;

	ipArry = ip.split("."),
		maskArry = mask.split("."),
		len = ipArry.length;

	for (i = 0; i < len; i++) {
		maskArry2[i] = 255 - Number(maskArry[i]);
	}

	for (var k = 0; k < 4; k++) { // ip & mask
		if ((ipArry[k] & maskArry[k]) == 0) {
			netIndex1 += 0;
		} else {
			netIndex1 += 1;
		}
	}
	for (var k = 0; k < 4; k++) { // ip & 255 - mask
		if ((ipArry[k] & maskArry2[k]) == 0) {
			netIndex += 0;
		} else {
			netIndex += 1;
		}
	}

	if (netIndex == 0 || netIndex1 == 0) {
		//document.getElementById(ipElem).focus();
		return _("%s can't be the network segment.", [str]);
	}

	for (var j = 0; j < 4; j++) {
		if ((ipArry[j] | maskArry[j]) == 255) {
			broadIndex += 0;
		} else {
			broadIndex += 1;
		}
	}

	if (broadIndex == 0) {
		//document.getElementById(ipElem).focus();
		return _("%s can't be the broadcast address.", [str]);
	}

	return;
}


/*********对象转换成字符串****************/
function objToString(obj) {
	var str = "",
		prop;
	for (prop in obj) {
		str += prop + "=" + encodeURIComponent(obj[prop]) + "&";
	}
	str = str.replace(/[&]$/, "");
	return str;
}

/**********赋值*************/
function inputValue(obj, callback) {
	var prop,
		tagName;

	for (prop in obj) {
		if (prop && $("#" + prop).length > 0) {
			tagName = document.getElementById(prop).tagName.toLowerCase();
			switch (tagName) {
			case "input":
			case "select":
				if (document.getElementById(prop).type == "checkbox") {
					if (obj[prop] == "true") {
						document.getElementById(prop).checked = true;
					} else {
						document.getElementById(prop).checked = false;
					}
				} else {
					$("#" + prop).val(obj[prop]);
				}
				break;
			default:
				if ($("#" + prop).hasClass("textboxs") || $("#" + prop).hasClass("input-append")) {
					$("#" + prop)[0].val(obj[prop]);
				} else {
					$("#" + prop).text(obj[prop]);
				}
				break;
			}
		} else if (prop && $("[name='" + prop + "']").length > 1) {
			tagName = document.getElementsByName(prop)[0].tagName.toLowerCase();
			if (tagName === "input") {
				if ($("[name='" + prop + "'][value='" + obj[prop] + "']").length > 0) {
					$("[name='" + prop + "'][value='" + obj[prop] + "']")[0].checked = true;
				}
			}
		}
	};
	if (typeof callback == "function") {
		callback.apply();
	}
}

//处理时间变成 天 时分秒
function formatSeconds(value) {
	var theTime = parseInt(value); // 秒 
	var theTime1 = 0; // 分 
	var theTime2 = 0; // 小时
	var theTime3 = 0; // 天
	// alert(theTime); 
	if (theTime > 60) {
		theTime1 = parseInt(theTime / 60);
		theTime = parseInt(theTime % 60);
		// alert(theTime1+"-"+theTime); 
		if (theTime1 > 60) {
			theTime2 = parseInt(theTime1 / 60);
			theTime1 = parseInt(theTime1 % 60);
			if (theTime2 > 24) {
				theTime3 = parseInt(theTime2 / 24);
				theTime2 = parseInt(theTime2 % 24);
			}
		}
	}
	var result = "" + parseInt(theTime) + _("s");
	if (theTime1 > 0) {
		result = "" + parseInt(theTime1) + _("m") + " " + result;
	}
	if (theTime2 > 0) {
		result = "" + parseInt(theTime2) + _("h") + " " + result;
	}
	if (theTime3 > 0) {
		result = "" + parseInt(theTime3) + _("d") + " " + result;
	}
	return result;
}

//base64加密
function Encode() {
	var base64EncodeChars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';

	function utf16to8(str) {
		var out,
			i,
			len,
			c;

		out = "";
		len = str.length;
		for (i = 0; i < len; i++) {
			c = str.charCodeAt(i);
			if ((c >= 0x0001) && (c <= 0x007F)) {
				out += str.charAt(i);
			} else if (c > 0x07FF) {
				out += String.fromCharCode(0xE0 | ((c >> 12) & 0x0F));
				out += String.fromCharCode(0x80 | ((c >> 6) & 0x3F));
				out += String.fromCharCode(0x80 | ((c >> 0) & 0x3F));
			} else {
				out += String.fromCharCode(0xC0 | ((c >> 6) & 0x1F));
				out += String.fromCharCode(0x80 | ((c >> 0) & 0x3F));
			}
		}
		return out;
	}

	function base64encode(str) {
		var out,
			i,
			len;
		var c1,
			c2,
			c3;

		len = str.length;
		i = 0;
		out = "";
		while (i < len) {
			c1 = str.charCodeAt(i++) & 0xff;
			if (i == len) {
				out += base64EncodeChars.charAt(c1 >> 2);
				out += base64EncodeChars.charAt((c1 & 0x3) << 4);
				out += "==";
				break;
			}
			c2 = str.charCodeAt(i++);
			if (i == len) {
				out += base64EncodeChars.charAt(c1 >> 2);
				out += base64EncodeChars.charAt(((c1 & 0x3) << 4) | ((c2 & 0xF0) >> 4));
				out += base64EncodeChars.charAt((c2 & 0xF) << 2);
				out += '=';
				break;
			}
			c3 = str.charCodeAt(i++);
			out += base64EncodeChars.charAt(c1 >> 2);
			out += base64EncodeChars.charAt(((c1 & 0x3) << 4) | ((c2 & 0xF0) >> 4));
			out += base64EncodeChars.charAt(((c2 & 0xF) << 2) | ((c3 & 0xC0) >> 6));
			out += base64EncodeChars.charAt(c3 & 0x3F);
		}
		return out;
	}
	return function (s) {
		return base64encode(utf16to8(s));
	}
}

/********获取随机数**************/
function getRandom() {
	return "?random=" + Math.random();
}

//检查是否超时
function checkIsTimeOut(str) {
	if (str.indexOf("<!DOCTYPE") != -1) {
		return true;
	}
	return false;
}


/*******模块对象********

function ModuleLogic1() {
	this.init = function () {
		this.initEvent();
	};
	this.initEvent = function () {

	};
	this.initValue = function () {

	};
	this.checkData = function () {
		return;
	}
	this.getSubmitData = function () {
		var data = {

		};
		return objToString(data);
	}
}
*****/

//page 逻辑对象
function PageLogic(url, setUrl) {
	var moduleArry;
	var _this = this;
	//初始化入口
	this.init = function () {
		moduleArry = this.modules;
		this.pageRunning = true;
		this.initEvent();
		for (var i = 0; i < moduleArry.length; i++) {
			if (typeof moduleArry[i].init == "function") {
				moduleArry[i].init();
			}

		}
		this.addValidate(); //添加数据验证
		this.getValue(url); //获取数据
	};

	//page 事件绑定
	this.initEvent = function () {

	};

	//取消时重新初始化
	this.reCancel = function () {
		_this.initValue(_this.data);
	}

	//page 获取数据
	this.getValue = function (url) {
		if (!this.pageRunning) {
			return;
		}
		if (typeof url == "string") {
			$.getJSON(url + "?" + Math.random(), _this.initValue);
		}
	}

	//初始化数据
	this.initValue = function (obj) {
		_this.data = obj;
		for (var i = 0; i < moduleArry.length; i++) {
			moduleArry[i].initValue();
		}
	};

	//获取提交数据
	this.getSubmitData = function () {
		var data = "";
		for (var i = 0; i < moduleArry.length; i++) {
			if (typeof moduleArry[i].getSubmitData == "function") {
				data += moduleArry[i].getSubmitData() + "&";
			}
		}

		data = data.replace(/[&]$/, "");
		return data;
	};

	//数据验证初始化
	this.addValidate = function () {
		this.validate = $.validate({
			custom: function () {
				var msg;
				//执行模块内部的数据验证
				for (var i = 0; i < moduleArry.length; i++) {
					if (typeof moduleArry[i].checkData == "function") {
						msg = moduleArry[i].checkData.apply();
					}
					if (msg) {
						return msg;
					}
				}
			},

			success: function () {

				_this.preSubmit();
			},

			error: function (msg) {
				if (msg) {
					_this.showPageMsg(msg);
				}
			}
		});
	};

	//提交数据
	this.preSubmit = function () {
		if (typeof _this.beforeSubmit == "function") {
			if (!_this.beforeSubmit()) {
				return;
			}
		}
		var data = _this.getSubmitData();
		$.ajax({
			url: setUrl,
			type: "POST",
			data: data,
			success: _this.successCallback,
			error: _this.ajaxErrMsg
		});
	};

	//提交成功
	this.successCallback = function (msg) {
		if (checkIsTimeOut(msg)) {
			top.location.reload(true);
			return;
		}

		var num = $.parseJSON(msg).errCode || "-1";
		var ip = _this.rebootIP || "";
		if (num == 0) { //success
			mainLogic.showModuleMsg(_("Saved successfully!"));
			_this.getValue(url);
		} else if (num == "2") {
			mainLogic.showModuleMsg(_("Fail to change password! Old Password is not correct."));
		} else if (num == "100") {
			progressLogic.init("", "reboot", 200, ip);
		} else if (num == "101") {
			window.location = "./login.html";
		}
		$("#submit").removeAttr("disabled");
	}

	//ajax提交数据失败
	this.ajaxErrMsg = function () {
		_this.showPageMsg("Upload data error!");
	}

	//显示用户提示信息
	this.showPageMsg = function (msg) {
		mainLogic.showModuleMsg(msg);
		return;
	}
}