define(function (require, exports, module) {

	var pageModule = new PageLogic("goform/getWifi", "goform/setWifi");
	pageModule.modules = [];
	pageModule.rebootIP = location.host;
	module.exports = pageModule;

	/*************WiFi Name and Password*********/
	var wifiSsidModule = new WifiSsidModule();
	pageModule.modules.push(wifiSsidModule);

	function WifiSsidModule() {
		this.init = function () {
			this.initEvent();
		}
		this.initEvent = function () {
			this.addInputEvent = false;
			$("#wifiEn").on("click", changeWifiEn);
			$("#wifiSecurityMode").on("click", changeSecurityMode);
			$("#helpTips").on("mouseover", function () {
				$("#hideSSIDTips").show();
			})
			$("#helpTips").on("mouseout", function () {
				$("#hideSSIDTips").hide();
			})
		}
		this.initValue = function () {
			var obj = pageModule.data;
			inputValue(obj);
			if (!this.addInputEvent) {
				$("#wifiSSID").addPlaceholder(_("WiFi Name"));
				$("#wifiPwd").initPassword(_("WiFi Password"));
				this.addInputEvent = true;
			}
			$("#wifiEn").html("");
			if (obj.wifiEn == "true") {
				$("#wifiWrap").show();
				$("#wifiEn").removeClass("icon-toggle-off").addClass("icon-toggle-on");
			} else {
				$("#wifiWrap").hide();
				$("#wifiEn").removeClass("icon-toggle-on").addClass("icon-toggle-off");
			}
			changeSecurityMode();
		}

		this.getSubmitData = function () {
			var data = {
				wifiEn: $("#wifiEn").hasClass("icon-toggle-on") || "false",
				wifiSSID: $("#wifiSSID").val(),
				wifiSecurityMode: $("#wifiSecurityMode").val(),
				wifiPwd: $("#wifiPwd").val(),
				wifiHideSSID: $("#wifiHideSSID:checked").val() || "false"
			};
			return objToString(data);
		};

		function changeWifiEn() {
			if ($("#wifiEn").hasClass("icon-toggle-off")) {
				$("#wifiWrap").show();
				$("#wifiEn").removeClass("icon-toggle-off").addClass("icon-toggle-on");
			} else {
				$("#wifiWrap").hide();
				$("#wifiEn").removeClass("icon-toggle-on").addClass("icon-toggle-off");
			}
			top.mainLogic.initModuleHeight();
		}

		/***********改变加密模式******/
		function changeSecurityMode() {
			var securityMode = $("#wifiSecurityMode").val();
			if (securityMode != "none") {
				$("#wifiPwd").parent().parent().removeClass("none");
			} else {
				$("#wifiPwd").parent().parent().addClass("none");
			}
			top.mainLogic.initModuleHeight();
		}
	}
	/*************END WiFi Name and Password***************/


	/*************WiFi Signal Strength*********************/
	var wifiPowerModule = new WifiPowerModule();
	pageModule.modules.push(wifiPowerModule);

	function WifiPowerModule() {
		this.initValue = function () {
			$("[name='wifiPower'][value='" + pageModule.data.wifiPower + "']")[0].checked = true;
		};
		this.getSubmitData = function () {
			var data = {
				wifiPower: $("input[name='wifiPower']:checked").val() || "low"
			}
			return objToString(data);
		}
	}
	/*************END WiFi Signal Strength*************************/

	/*************WiFi Schedule*******************/
	var wifiScheduleModule = new WifiScheduleModule();
	pageModule.modules.push(wifiScheduleModule);

	function WifiScheduleModule() {
		var oldDate; /******保存初始化日期******/
		this.init = function () {
			this.initHtml();
			this.initEvent();
		}
		this.initHtml = function () {
			var hourStr = "",
				minStr = "",
				i = 0;
			for (i = 0; i < 24; i++) {
				hourStr += "<option value='" + ("100" + i).slice(-2) + "'>" + ("100" + i).slice(-2) + "</option>";
			}

			$("#startHour, #endHour").html(hourStr);

			for (i = 0; i < 60; i++) {
				if (i % 5 === 0) {
					minStr += "<option value='" + ("100" + i).slice(-2) + "'>" + ("100" + i).slice(-2) + "</option>";
				}
			}
			$("#startMin, #endMin").html(minStr);
		}
		this.initEvent = function () {
			$("input[name='wifiTimeEn']").on("click", changeWifiTimeEn);
			$("[id^=day]").on("click", clickTimeDay);
		}
		this.initValue = function () {
			var obj = pageModule.data;
			inputValue(obj.wifiTime);
			translateDate(obj.wifiTime.wifiTimeDate);
			oldDate = obj.wifiTime.wifiTimeDate;
			var time = obj.wifiTime.wifiTimeClose.split("-");
			$("#startHour").val(time[0].split(":")[0]);
			$("#startMin").val(time[0].split(":")[1]);
			$("#endHour").val(time[1].split(":")[0]);
			$("#endMin").val(time[1].split(":")[1]);
			changeWifiTimeEn();
		}
		this.checkData = function () {
			if ($("[name='wifiTimeEn']")[0].checked) {
				var date = getScheduleDate();
				if (date == "00000000") {
					return _("Select one day at least.");
				}
			}
			return;
		}
		this.getSubmitData = function () {
			var time = $("#startHour").val() + ":" + $("#startMin").val() + "-" +
				$("#endHour").val() + ":" + $("#endMin").val();
			var data = {
				wifiTimeEn: $("input[name='wifiTimeEn']:checked").val() || "false",
				wifiTimeClose: time,
				wifiTimeDate: getScheduleDate()
			};
			return objToString(data);
		}

		/*******启用或禁用定时开关********/
		function changeWifiTimeEn() {
			if ($("input[name='wifiTimeEn']")[0].checked) {
				$("#wifiScheduleWrap").show();
			} else {
				$("#wifiScheduleWrap").hide();
			}
			top.mainLogic.initModuleHeight();
		}
		/*********获取定时重启日期字符串***********/
		function getScheduleDate() {
			var i = 0,
				len = 8,
				str = "";
			for (i = 0; i < len; i++) {
				if ($("#day" + i)[0].checked) {
					str += "1";
				} else {
					str += "0";
				}
			}
			return str;
		}

		/**********点击everyday**********/
		function clickTimeDay() {
			var dataStr = getScheduleDate();
			if (this.id == "day0") { //点击everyday
				if (this.checked) {
					translateDate("11111111");
				} else {
					translateDate("00000000");
				}
			} else {
				if (dataStr.slice(1) == "1111111") {
					translateDate("11111111");
				} else {
					translateDate("0" + dataStr.slice(1));
				}
			}
		}

		/*******根据字符串改变日期的选择*******/
		function translateDate(str) {
			var dayArry = str.split(""),
				len = dayArry.length,
				i = 0;
			for (i = 0; i < len; i++) {
				$("#day" + i)[0].checked = true ? dayArry[i] == 1 : 0;
			}
		}

	}
	/*************END WiFi Schedule *****************************/

	/*************WPS*************************************/
	var wifiWpsModule = new WifiWpsModule();
	pageModule.modules.push(wifiWpsModule);

	function WifiWpsModule() {
		this.init = function () {
			this.initEvent();
		}
		this.initEvent = function () {
			$("input[name='wpsEn']").on("click", changeWpsEn);


			$("#wpsPBC").on("click", function () {
				$("#wpsPBC").attr("disabled", true);
				$.post("goform/setWifiWps", "action=pbc", function (msg) {
					if (checkIsTimeOut(msg)) {
						top.location.reload(true);
						return;
					}
					mainLogic.showModuleMsg(_("PBC configured successfully!"));
					$("#wpsPBC").removeAttr("disabled");
				});
			});
		}
		this.initValue = function () {
			var obj = pageModule.data;
			inputValue(obj.wifiWPS);
			changeWpsEn();
		}
		this.getSubmitData = function () {
			var data = {
				wpsEn: $("input[name='wpsEn']:checked").val() || "false"
			};
			return objToString(data);
		}

		/*******启用或禁用WPS********/
		function changeWpsEn() {
			if ($("input[name='wpsEn']")[0].checked) {
				$("#wpsWrap").show();
				$("#wifiSecurityMode").attr("disabled", true);
				$("#wifiPwd").attr("disabled", true);
			} else {
				$("#wifiSecurityMode").removeAttr("disabled");
				$("#wifiPwd").removeAttr("disabled");
				$("#wpsWrap").hide();
			}
			top.mainLogic.initModuleHeight();
		}
	}
	/*************END WPS *******************************/

	/*************Wireless Parameters********************************/
	var wifiParameterModule = new WifiParameterModule();
	pageModule.modules.push(wifiParameterModule);

	function WifiParameterModule() {
		this.initValue = function () {
			var obj = pageModule.data;
			inputValue(obj.wifi);
		}
		this.getSubmitData = function () {
			var data = {
				wifiMode: $("#wifiMode").val(),
				wifiChannel: $("#wifiChannel").val(),
				wifiBandwidth: $("#wifiBandwidth").val()
			}
			return objToString(data);
		}
	}
	/**************END Wireless Parameters********************************************/

})