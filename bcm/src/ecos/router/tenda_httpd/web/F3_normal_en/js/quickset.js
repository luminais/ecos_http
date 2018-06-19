function ModuleLogic(view) {
	var that = this;
	this.init = function () {
		this.currenPageId = "detect";
		this.nextPageId = null;
		this.initEvent();
		setTimeout(function () {
			that.getValue();
		}, 3000);


	};
	this.timer = null; //定时器
	this.outWanType = null;
	this.initEvent = function () { //初始化事件
		$("#skipLineoff").on("click", function () { //未插网线时的跳过
			clearTimeout(that.timer);
			view.changePage("error", "quickSetWiFiWrap");
		});
		that.addInputEvent = false;

		/**pppoe 模块事件*/
		$("#pppoeNext").on("click", function () {
			view.checkData("quickSetPPPoEWrap", "quickSetWiFiWrap");
			that.pppoeSkip = false;
		});
		$("#pppoeSkip").on("click", function () {
			view.changePage("quickSetPPPoEWrap", "quickSetWiFiWrap");
			that.pppoeSkip = true;
		});
		/**static 模块事件*/
		$("#staticNext").on("click", function () {
			view.checkData("quickSetStaticWrap", "quickSetWiFiWrap");
			that.staticSkip = false;
		})
		$("#staticSkip").on("click", function () {
			view.changePage("quickSetStaticWrap", "quickSetWiFiWrap");
			that.staticSkip = true;
		});

		/**dhcp 模块事件*/
		$("#dhcpNext").on("click", function () {
			view.checkData("quickSetDHCPWrap", "quickSetWiFiWrap");
		});

		/**wifi 模块事件*/
		$("#wifiNext").on("click", function () {
			view.checkData("quickSetWiFiWrap", "DoneWrap");
		});

		$("#wifiBack").on("click", function () {
			if (view.currenPageId == "error") {
				clearTimeout(that.timer);
				that.timer = setTimeout(function () {
					that.getValue();
				}, 2000);
			}
			view.changePage(view.nextPageId, view.currenPageId);
		})

		$("#gohome").on("click", function () {
			location.href = "./index.html";
		});

		$("#wifiWarningBack, .dialog-close").on("click", function () {
			$("#wifiWarning").hide();
			$("#progress-overlay").removeClass("in");
		});

		//无线未加密弹出框
		$("#wifiWarningNext").on("click", function () {
			$("#wifiWarning").hide();
			$("#progress-overlay").removeClass("in");
			view.preSubmit("quickSetWiFiWrap", "DoneWrap");
		});
	}
	this.getValue = function () {
		$.getJSON("goform/getWizard" + getRandom(), that.initValue);
	}

	//初始化数据
	this.initValue = function (obj) {
		this.outWanType = obj.wizardWANDetection;
		view.lanIP = obj.lanIP;
		view.lanMask = obj.lanMask;
		$("#wizardSSID").val(obj.wizardSSID);
		if (!that.addInputEvent) {

			$("#wizardWanIP").addPlaceholder(_("IP"));
			$("#wizardWanMask").addPlaceholder(_("Subnet Mask"));
			$("#wizardWanGateway").addPlaceholder(_("Default Gateway"));
			$("#wizardWanDns1").addPlaceholder(_("Preferred DNS Server"));
			$("#wizardWanDns2").addPlaceholder(_("Alternative DNS Server"));

			$("#wizardPPPoEUser").addPlaceholder(_("User Name from ISP"));
			$("#wizardPPPoEPwd").initPassword(_("Password from ISP"));

			$("#wizardSSID").addPlaceholder(_("WiFi Name"));
			$("#wizardSSIDPwd").initPassword(_("WiFi Password"));
			that.addInputEvent = true;
		}
		that.timer = setTimeout(function () {
			that.getValue();
		}, 2000);

		if (obj.wizardWANDetection != "detecting") {
			switch (obj.wizardWANDetection) {
			case "disabled":
				that.nextPageId = "error";
				view.changePage(that.currenPageId, that.nextPageId);
				that.currenPageId = that.nextPageId;
				return;
			case "pppoe":
				that.nextPageId = "quickSetPPPoEWrap";
				break;
			case "dhcp":
				that.nextPageId = "quickSetDHCPWrap";
				break;
			case "static":
				that.nextPageId = "quickSetStaticWrap";
				break;
			}
			view.changePage(that.currenPageId, that.nextPageId);
			that.currenPageId = that.nextPageId;
			clearTimeout(that.timer);
		}
	};
};


function ModuleView() {
	var that = this;
	this.currenPageId = null; //保存当前页面ID
	this.nextPageId = null; //保存下一个显示ID
	this.lanIP = null;
	this.lanMask = null;
	//最后设置
	function endSetup() {
		var num = 8,
			timer = null;
		timer = setInterval(function () {
			num--;
			$("#countWrap").html(num);
			if (num == 0) {
				clearInterval(timer);
				$("#savingWrap").hide();
				$("#savedWrap").show();
				$.getJSON("goform/getWizard" + getRandom(), function (obj) {
					if (obj.connectInternet == "true") { //可以上网
						$("#successinternetWrap, #moreTips").show();
						$("#savedWrap img").attr("src", "img/conn-ok.png"); //不能上网时3秒后自动刷新页面
					} else {
						$("#savedWrap img").attr("src", "img/ok.png"); //不能上网时3秒后自动刷新页面
						$("#successinternetWrap, #moreTips").hide();
						setTimeout(function () {
							window.location = "./index.html";
						}, 3000)
					}
				});


			}
		}, 1000);


	}

	//切换页面，不需要验证数据
	this.changePage = function (currenPage, nextPage) {
		$("#" + currenPage).hide();
		$("#" + nextPage).show();
		if (currenPage != "quickSetWiFiWrap" && currenPage != "quickSetLoginWrap") {
			this.currenPageId = currenPage;
			this.nextPageId = nextPage;
			$(".breadcrumb .active").removeClass("active");
			if (nextPage == "quickSetWiFiWrap") {
				$(".breadcrumb").find("li").eq(1).addClass("active");
			} else if (nextPage == "DoneWrap") {
				$(".breadcrumb").find("li").eq(2).addClass("active");
			} else {
				$(".breadcrumb").find("li").eq(0).addClass("active");
			}
		}
		if (nextPage == "DoneWrap") {
			$("#ssid-msg").text($("#wizardSSID").val());
			endSetup();
		}
	};
	this._currenId = null; //临时存放数据验证需要参数
	this._nextId = null;
	this.checkData = function (currenPage, nextPage) { //检查数据合法性
		this._currenId = currenPage;
		this._nextId = nextPage;
		this.validate.checkAll();

	}

	function checkPPPoEData() {
		var pppoeUser = $("#wizardPPPoEUser").val(),
			pppoePwd = $("#wizardPPPoEPwd").val();

	}

	//检查静态IP地址合法性
	function checkStaticData() {
		var ip = $("#wizardWanIP").val(),
			mask = $("#wizardWanMask").val(),
			gateway = $("#wizardWanGateway").val(),
			dns1 = $("#wizardWanDns1").val(),
			dns2 = $("#wizardWanDns2").val();
		var lanIp = that.lanIP,
			lanMask = that.lanMask;
		var msg = checkIsVoildIpMask(ip, mask, _("IP Address"));
		if (msg) {
			$("#wizardWanIP").focus();
			return msg;
		}
		if (checkIpInSameSegment(ip, mask, lanIp, lanMask)) {
			$("#wizardWanIP").focus();
			return _("%s and %s (%s) should not be in the same network segment.", [_("WAN IP"), _("LAN IP"), lanIp]);
		}
		if (!checkIpInSameSegment(ip, mask, gateway, mask)) {
			$("#wizardWanGateway").focus();
			return _("%s and %s must be in the same network segment.", [_("WAN IP"), _("Gateway")]);
		}

		if(ip == gateway) {
			return _("WAN IP and Default Gateway can't be the same.");
		}
		if (dns1 == dns2) {
			return _("Preferred DNS server and Alternative DNS server can't be the same.");
		}
	}

	this.validate = $.validate({
		custom: function () {},

		success: function () {
			var currenPage = that._currenId,
				nextPage = that._nextId,
				msg = "";
			switch (currenPage) {
			case "quickSetPPPoEWrap":
				msg = checkPPPoEData();
				break;
			case "quickSetStaticWrap":
				msg = checkStaticData();
				break;
			case "quickSetWiFiWrap":
				if ($("#wizardSSIDPwd").val() == "") {
					$("#wifiWarning").show();
					$("#progress-overlay").addClass("in");
					return;
				}
				break;
			case "quickSetDHCPWrap":
				break;
			}
			if (msg) {
				alert(msg);
				return;
			}
			that.preSubmit(currenPage, nextPage);
			//that.changePage(currenPage, nextPage);
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
	this.preSubmit = function (currenPage, nextPage) {
		var data, subStr;
		if (currenPage == "quickSetWiFiWrap") {
			data = {
				mode: "wifi",
				wizardSSID: $("#wizardSSID").val(),
				wizardSSIDPwd: $("#wizardSSIDPwd").val(),
				timeZone: getTimeZone()
			};

		} else if (currenPage == "quickSetPPPoEWrap") {
			data = {
				mode: "pppoe",
				wizardPPPoEUser: $("#wizardPPPoEUser").val(),
				wizardPPPoEPwd: $("#wizardPPPoEPwd").val()
			}
		} else if (currenPage == "quickSetStaticWrap") {
			data = {
				mode: "static",
				wizardWanIP: $("#wizardWanIP").val(),
				wizardWanMask: $("#wizardWanMask").val(),
				wizardWanGateway: $("#wizardWanGateway").val(),
				wizardWanDns1: $("#wizardWanDns1").val(),
				wizardWanDns2: $("#wizardWanDns2").val()
			}
		} else if (currenPage == "quickSetDHCPWrap") {
			data = {
				mode: "dhcp"
			}
		}

		subStr = objToString(data);
		$.post("goform/setWizard", subStr, function (str) {
			var num = $.parseJSON(str).errCode;
			if (num == "0") {
				that.changePage(currenPage, nextPage);
			}
		})
	}
}



$(function () {
	var moduleView = new ModuleView();
	var moduleLogic = new ModuleLogic(moduleView);
	moduleLogic.init();
})