var getWizardTimer = null;

function ModuleLogic(view) {
	var that = this;
	var dialog = new Dialog();

	that.firstInitData = true; //当前是否是第一次请求; 
	that.firstInitTransferData = true; //当前是否第一次对迁移到的用户名密码进行赋值；
	that.firstInitWanDetectResult = true;

	this.init = function () {

		//默认为pppoe拨号
		$("input[name=wanType][value=pppoe]")[0].checked = true;
		this.currenPageId = "quickSetWrap";
		this.nextPageId = null;
		this.initEvent();
		this.getValue();
	};

	this.initEvent = function () { //初始化事件

		that.addInputEvent = false;
		/**wifi 模块事件*/
		$("#save").on("click", function () {
			view.checkData("quickSetWrap", "quicksetDoneWrap");
		});

		$("input[name=wanType]").on("click", function () {
			that.changeWanType();
		});

		$("#inportBtn").on("click", function () {
			dialog.open({
				Id: "inportWanPPPOE",
				height: "550px"
			});
		});


		$("#sure").on("click", function () {
			view.checkData("quickWireless", "quicksetDoneWrap", "normalSet", "quickSetWrap");
			//$("#main_content .container").removeClass("none");
		});

		//解决IE高度不自动变化问题

		$("#quickSetStaticWrap input,#quickSetPPPoEWrap input").on("blur", function () {
			var oHeight = $(".network-config-wrapper").height() + 34;

			$(".network-wrapper").css("height", oHeight);
		});

		//点击蒙版时，可以关闭弹框
		$("#progress-overlay").on("click",function() {
			dialog.close({
				Id: "inportWanPPPOE"
			});
		});
	};

	this.changeWanType = function () {
		var wanType = $("input[name=wanType]:checked").val(),
			wrapperHeight;
		$("#quickSetPPPoEWrap, #quickSetStaticWrap, #quickSetDHCPWrap").addClass("none");
		switch (wanType) {
		case "pppoe":
			$("#quickSetPPPoEWrap").removeClass("none");
			break;
		case "static":
			$("#quickSetStaticWrap").removeClass("none");
			break;
		default: //dhcp
			$("#quickSetDHCPWrap").removeClass("none");
		}

		//切换wanType初始化高度
		wrapperHeight = $(".network-config-wrapper").height();
		$(".network-wrapper").css("height", wrapperHeight);

	};

	this.getValue = function () {
		var modules = "lanCfg,wanDetection,wanBasicCfg,isWifiClients,wifiBasicCfg,synchroStatus",
			data = "?random=" + Math.random() + "&modules=" + encodeURIComponent(modules);

		$.get("goform/getWizard" + data, function (str) {
			var obj = {};

			try {
				obj = $.parseJSON(str);
			} catch (e) {
				obj = {};
			}

			that.wanBasicCfg = obj.wanBasicCfg;
			that.wifiBasicCfg = obj.wifiBasicCfg;

			//发现导入成功,提示用户并刷新当前页面;
			if (obj.synchroStatus.synchroStatus == "true") {
				if (getWizardTimer) {
					clearInterval(getWizardTimer);
				}
				alert(_("Synchronization success. The current page is refreshed when you click OK."));
				window.location.reload();
				//返回，以免由于后台返回慢，定时器还在继续请求，提示用户
				return;

				//迁移成功 && 未导入;
			} else if (obj.wanBasicCfg.wanPPPoEUser != "") {
				that.PPPOEstatus = "transferSuc";

				//未迁移成功；
			} else {
				that.PPPOEstatus = "transferFail";
			}

			//仅在第一次数据初始化时执行：根据迁移是否成功来选择需要用户配置的页面
			if (that.firstInitData) {
				$("#main_content").children().addClass("none");

				//用户名和密码迁移成功 && 产测未写入密码。则提示用户配置无线密码
				if (that.PPPOEstatus == "transferSuc" && obj.wifiBasicCfg.wifiPwd == "") {
					$("#main_content .wireless-container").removeClass("none");

					//(用户密码迁移成功 && 产测已经写入密码) || 用户名密码未迁移成功, 则显示上网设置及无线设置项
				} else {
					$("#main_content .container-notinport").removeClass("none");
				}
				that.firstInitData = false;
			}

			that.initValue(obj);
		});
	};

	//初始化数据
	this.initValue = function (obj) {

		view.lanIP = obj.lanCfg.lanIP;
		view.lanMask = obj.lanCfg.lanMask;
		view.srcSSID = obj.wifiBasicCfg.wifiSSID;
		view.srcWrlPwd = obj.wifiBasicCfg.wifiPwd;
		view.connectTypes = obj.isWifiClients.isWifiClients || "false";

		//第一次初始化时
		if (!that.addInputEvent) {
			//无线参数赋值，且只需要赋值一次，后续不需要更新值
			$("#wifiSSID").val(obj.wifiBasicCfg.wifiSSID);
			$("#wifiPwd").val(obj.wifiBasicCfg.wifiPwd);
			$("#quickSSID").val(obj.wifiBasicCfg.wifiSSID);
			$("#quickPwd").val(obj.wifiBasicCfg.wifiPwd);
			$("#wanPPPoEPwd").val("");

			//样式初始化
			$("#wanPPPoEUser").addPlaceholder(_("User Name from ISP"));
			$("#wanPPPoEPwd").initPassword(_("Password from ISP"));
			$("#wifiSSID").addPlaceholder(_("WiFi Name"));
			$("#wifiPwd").initPassword(_("8 or more characters"));
			$("#quickPwd").initPassword(_("8 or more characters"));
			$("#wanIP").addPlaceholder(_("IP Address"));
			$("#wanMask").addPlaceholder(_("Subnet Mask"));
			$("#wanGateway").addPlaceholder(_("Default Gateway"));
			$("#wanDns1").addPlaceholder(_("Preferred DNS"));
			$("#wanDns2").addPlaceholder(_("Alternate DNS"));

			//设立定时刷新机制，2s获取一次值；检测上网方式、迁移状态、导入状态
			getWizardTimer = setInterval(function () {
				that.getValue();
			}, 2000);

			that.addInputEvent = true;
		}

		switch (that.PPPOEstatus) {
			//pppoe迁移成功,但未导入，则继续检测导入状态
		case "transferSuc":
			//迁移只赋值一次,后续刷新过程中不再改变值；
			if (that.firstInitTransferData) {
				dialog.close();
				$(".net-status").addClass("none");
				$("input[name=wanType][value=pppoe]")[0].checked = true;
				$("#wanPPPoEUser").val(that.wanBasicCfg.wanPPPoEUser);
				//wanPPPoEPwd_用于规避IE8问题；
				$("#wanPPPoEPwd").val(that.wanBasicCfg.wanPPPoEPwd);
				that.changeWanType();
				that.firstInitTransferData = false;
			}
			break;

			//未迁移成功，继续检测状态；
		case "transferFail":
			var wanType = $("input[name=wanType]:checked").val(),
				adslStr = "<span class='text-warning'>" + _("PPPoE") + "</span>",
				dhcpStr = "<span class='text-warning'>" + _("Dynamic IP Address") + "</span>",
				staticStr = "<span class='text-warning'>" + _("Static IP Address") + "</span>",
				noWireStr = '<span class="text-danger">' + _("Tips: WAN port disconnected. Please connect an Ethernet cable with Internet connectivity to the port.") + '</span>',
				sysCheckStr = _("As detected, your connection type is:");

			//第一次检测到联网结果 && 未迁移成功。则切换联网方式，后续不再对检测结果进行更新；同时继续检测导入、迁移状态；
			//若检测到联网结果前已经迁移成功了，则不对检测结果进行处理
			if (obj.wanDetection.wanDetection != "detecting" && this.firstInitWanDetectResult == true && that.firstInitTransferData == true) {

				this.firstInitWanDetectResult = false;
				switch (obj.wanDetection.wanDetection) {
				case "disabled":
					//如果得到的结果是未插网线，则继续检测，至到得到明确的上网方式，因此把标记值置为true；
					this.firstInitWanDetectResult = true;
					$(".net-status").html(noWireStr);
					break;
				case "pppoe":
					$(".net-status").html(sysCheckStr + adslStr);
					break;
				case "dhcp":
					$(".net-status").html(sysCheckStr + dhcpStr);
					$("input[name=wanType][value=dhcp]")[0].checked = true;
					that.changeWanType();

					break;
				case "static":
					$(".net-status").html(sysCheckStr + staticStr);
					$("input[name=wanType][value=static]")[0].checked = true;
					that.changeWanType();
					break;
				}
			};

			break;
		}
	};
}

function ModuleView() {
	var that = this;
	this.currenPageId = null; //保存当前页面ID
	this.nextPageId = null; //保存下一个显示ID
	this.lanIP = null;
	this.lanMask = null;
	this.connectTypes = "false"; //无线连接(true), 有线（false）
	this.srcSSID = "";

	//切换页面，不需要验证数据
	this.changePage = function (currenPage, nextPage, specialPage, prevPage) {
		var leaftSec = 2,
			distSSID = $("#wifiSSID")[0].value,
			distPwd = $("#wifiPwd")[0].value,
			quickSSID = $("#quickSSID")[0].value,
			quickPwd = $("#quickPwd")[0].value,
			timer = null,
			hasChangeWifi = false;

		$("#wireSuccess").html(_("You will be redirected to the user interface after %s seconds.", [3]));
		$("#" + currenPage).hide();
		$("#" + nextPage).show();

		if (specialPage) {
			$("#" + prevPage).hide();
			$("#" + specialPage).show()
		}

		$("#save")[0].disabled = false;

		if (specialPage) { //有pppoe密码时
			if ((this.srcSSID == quickSSID && quickPwd == this.srcWrlPwd)) {
				hasChangeWifi = false;
			} else {
				hasChangeWifi = true;
			}
		} else {
			if ((this.srcSSID == distSSID && distPwd == this.srcWrlPwd)) {
				hasChangeWifi = false;
			} else {
				hasChangeWifi = true;
			}
		}

		if (this.connectTypes == "false" || !hasChangeWifi) { //有线连接或无线默认值未修改时跳到保存成功3s跳转页面;

			$("#wireSuccess").removeClass("none");

			timer = setInterval(function () {
				$("#wireSuccess").html(_("You will be redirected to the user interface after %s seconds.", [leaftSec]));
				leaftSec = leaftSec - 1;
				if (leaftSec < 1) {
					clearInterval(timer);
					window.location = "/index.html";
				}
			}, 1000);

		} else { //无线连接且修改了无线默认值时提示用户重新连接
			$("#wirelessSuccess").removeClass("none");
			if (specialPage) {
				$("#newSSID").html(quickSSID);
			} else {
				$("#newSSID").html(distSSID);
			}
		}

	};

	this._currenId = null; //临时存放数据验证需要参数
	this._nextId = null;
	this._specialId = null;
	this._prevId = null;
	this.checkData = function (currenPage, nextPage, specialPage, prevPage) { //检查数据合法性
		this._currenId = currenPage;
		this._nextId = nextPage;
		this._specialId = specialPage;
		this._prevId = prevPage;
		this.validate.checkAll();
	}


	//检查静态IP地址合法性
	function checkStaticData() {
		var ip = $("#wanIP").val(),
			mask = $("#wanMask").val(),
			gateway = $("#wanGateway").val(),
			dns1 = $("#wanDns1").val(),
			dns2 = $("#wanDns2").val();
		var lanIp = that.lanIP,
			lanMask = that.lanMask;
		var msg = checkIsVoildIpMask(ip, mask, _("IP Address"));
		if (msg) {
			$("#wanIP").focus();
			return msg;
		}
		if (checkIpInSameSegment(ip, mask, lanIp, lanMask)) {
			$("#wanIP").focus();
			return _("%s and %s (%s) cannot be in the same network segment.", [_("WAN IP Address"), _("LAN IP Address"), lanIp]);
		}
		if (!checkIpInSameSegment(ip, mask, gateway, mask)) {
			$("#wanGateway").focus();
			return _("%s and %s must be in the same network segment.", [_("WAN IP Address"), _("Gateway")]);
		}

		if (ip == gateway) {
			return _("WAN IP Address and Default Gateway cannot be the same.");
		}
		if (dns1 == dns2) {
			return _("Preferred DNS server and Alternate DNS server cannot be the same.");
		}
	}


	this.validate = $.validate({
		custom: function () {},

		success: function () {
			var currenPage = that._currenId,
				nextPage = that._nextId,
				specialPage = that._specialId,
				prevPage = that._prevId,
				msg = "",
				wanType = $("[name=wanType]:checked")[0].value;

			if (wanType == "static") {
				msg = checkStaticData();
			}

			if (msg) {
				alert(msg);
				return;
			}
			that.preSubmit(currenPage, nextPage, specialPage, prevPage);
		},

		error: function (msg) {}
	});

	function getTimeZone() {
		var a = [],
			b = new Date().getTime(),
			zone = new Date().getTimezoneOffset() / -60;

		if (a = displayDstSwitchDates()) {
			if (a[0] < a[1]) {
				if (b > a[0] && b < a[1]) {
					zone--;
				}
			} else {
				if (b > a[0] || b < a[1]) {
					zone--;
				}
			}
		}
		return zone;
	}

	//提交数据
	this.preSubmit = function (currenPage, nextPage, specialPage, prevPage) {
		var data = {},
			subStr;
		$("#quickSetWrap").find("input").each(function () {
			var _this = this,
				name = this.name,
				type = _this.type;
			switch (type) {
			case "radio":
				data.wanType = $("[name=wanType]:checked")[0].value;
				break;
			case "button":
				break;
			default:
				data[name] = this.value;
			}
		});
		//data.sysTimeZone = getTimeZone();
		data.module1 = "wifiBasicCfg";
		data.module2 = "wanBasicCfg";
		data.module3 = "synSysTime"; //edit by pjl@与系统设置的时间模块区分开;
		if (data.wifiPwd == "") {
			data.wifiNoPwd = "true";
		}

		//不需设置pppoe时
		if (that._currenId == "quickWireless") {
			data = {};
			data.wifiPwd = $("#quickPwd").val();
			data.wifiSSID = $("#quickSSID").val();

			data.module1 = "wifiBasicCfg";
			data.module2 = "synSysTime";
		}

		data.sysTimeZone = getTimeZone();

		subStr = objToString(data);
		$("#save")[0].disabled = true;
		$.post("goform/setWizard", subStr, function (str) {
			clearInterval(getWizardTimer);

			var num = $.parseJSON(str).errCode;
			if (num == "0") {
				that.changePage(currenPage, nextPage, specialPage, prevPage);
			}
		});
	}
}


$(function () {
	var moduleView = new ModuleView();
	var moduleLogic = new ModuleLogic(moduleView);
	moduleLogic.init();
});