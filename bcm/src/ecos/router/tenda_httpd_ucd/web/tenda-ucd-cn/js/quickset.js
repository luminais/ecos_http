function ModuleLogic(view) {
	var that = this;
	this.init = function () {
		$("input[name=wanType][value=pppoe]")[0].checked = true;

		this.currenPageId = "quickSetWrap";
		this.nextPageId = null;
		this.initEvent();
		this.getValue();
	};

	this.timer = null; //定时器
	this.initEvent = function () { //初始化事件

		that.addInputEvent = false;

		/**wifi 模块事件*/
		$("#save").on("click", function () {
			view.checkData("quickSetWrap", "quicksetDoneWrap");
		});

		$("input[name=wanType]").on("click", function () {
			that.changeWanType();
		});

	};

	this.changeWanType = function () {
		var wanType = $("input[name=wanType]:checked")[0].value;
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
	};

	this.getValue = function () {
		$.getJSON("goform/getWizard" + getRandom(), that.initValue);
	}

	//初始化数据
	this.initValue = function (obj) {
		this.outWanType = obj.wizardWANDetection;
		view.lanIP = obj.lanIP;
		view.lanMask = obj.lanMask;
		view.connectTypes = obj.wifiMobileDevice || "false";

		if (!that.addInputEvent) {
			$("#wizardSSID").val(obj.wizardSSID);
			$("#wizardPPPoEUser").addPlaceholder("请输入宽带运营商提供的帐号");
			$("#wizardPPPoEPwd").initPassword("请输入宽带运营商提供的密码");
			$("#wizardSSID").addPlaceholder("无线名称");
			$("#wizardSSIDPwd").initPassword("请输入8-64位的无线密码");
			that.addInputEvent = true;
		}

		that.timer = setTimeout(function () {
			that.getValue();
		}, 2000);


		var wanType = $("input[name=wanType]:checked").val(),
			adslStr = "<span class='text-warn'>宽带拨号</span>",
			dhcpStr = "<span class='text-warn'>动态IP</span>",
			staticStr = "<span class='text-warn'>静态IP</span>",
			noWireStr = '<span class="text-danger">提醒：路由器WAN口未插网线，请检查并连接好WAN口网线！</span>'

		if (obj.wizardWANDetection != "detecting") {
			switch (obj.wizardWANDetection) {

			case "disabled":
				$(".net-status").html(noWireStr);
				break;
			case "pppoe":
				$(".net-status").html("系统检测到您的联网方式为：" + adslStr);
				break;
			case "dhcp":
				$(".net-status").html("系统检测到您的联网方式为：" + staticStr);
				break;
			case "static":
				$(".net-status").html("系统检测到您的联网方式为：" + dhcpStr);
				break;
			case "connected":
				$(".net-status").html("联网成功！");
			}
			clearTimeout(that.timer);
		};
	};
}


function ModuleView() {
	var that = this;
	this.currenPageId = null; //保存当前页面ID
	this.nextPageId = null; //保存下一个显示ID
	this.lanIP = null;
	this.lanMask = null;
	this.connectTypes = "false"; //是否为无线连接(true), 有线（false）

	//切换页面，不需要验证数据
	this.changePage = function (currenPage, nextPage) {
		var leaftSec = 4;
		$("#" + currenPage).hide();
		$("#" + nextPage).show();
		if (this.connectTypes == "false") { //有线连接

			$("#wireSuccess").removeClass("none");
			setInterval(function () {
				$("#dynamic-sec").html(leaftSec);
				leaftSec = leaftSec - 1;
				if (leaftSec < 1) {
					window.location = "/index.html";
				}
			}, 1000);

		} else {
			$("#wirelessSuccess").removeClass("none");
		}

	};

	this._currenId = null; //临时存放数据验证需要参数
	this._nextId = null;
	this.checkData = function (currenPage, nextPage) { //检查数据合法性
		this._currenId = currenPage;
		this._nextId = nextPage;
		this.validate.checkAll();

	}


	//检查静态IP地址合法性
	function checkStaticData() {
		var ip = $("#wizardWanIP")[0].value,
			mask = $("#wizardWanMask")[0].value,
			gateway = $("#wizardWanGateway")[0].value,
			dns1 = $("#wizardWanDns1")[0].value,
			dns2 = $("#wizardWanDns2")[0].value;
			
		var lanIp = that.lanIP,
			lanMask = that.lanMask;
		var msg = checkIsVoildIpMask(ip, mask, _("IP地址"));
		if (msg) {
			$("#wizardWanIP").focus();
			return msg;
		}
		if (checkIpInSameSegment(ip, mask, lanIp, lanMask)) {
			$("#wizardWanIP").focus();
			return "WAN口IP和LAN口IP(" +lanIP+ ")不应该在同网段.";
		}
		if (!checkIpInSameSegment(ip, mask, gateway, mask)) {
			$("#wizardWanGateway").focus();
			return "WAN口IP和网关必须在同一网段.";
		}

		if (ip == gateway) {
			return "WAN口IP和默认网关不能相同.";
		}
		if (dns1 == dns2) {
			return "首选DNS服务器和备选DNS服务器不能相同.";
		}
	}


	this.validate = $.validate({
		custom: function () {},

		success: function () {
			var currenPage = that._currenId,
				nextPage = that._nextId,
				msg = "",
				wanType = $("[name=wanType]:checked")[0].value;

			if (wanType == "static") {
				msg = checkStaticData();
			}

			if (msg) {
				alert(msg);
				return;
			}
			that.preSubmit(currenPage, nextPage);
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
			case "checkbox":
				// data.readUserProtocal = $("[name=readUserProtocal]")[0].checked ? "1" : "0";
				break;
			case "button":
				break;
			default:
				data[name] = this.value;
			}
		});
		data.timeZone = getTimeZone();

		subStr = objToString(data);
		$.post("goform/setWizard", subStr, function (req) {
			// var str = req.responseText || "{}";
			// var num = $.parseJSON(str).errCode;
			top.location.reload(true);

			/*if (num == "0") {
				that.changePage(currenPage, nextPage);
			}*/
		})
	}
}



$(function () {
	var moduleView = new ModuleView();
	var moduleLogic = new ModuleLogic(moduleView);
	moduleLogic.init();
});