define(function (require, exports, module) {
	var pageModule = new PageLogic("goform/getWifiRelay", "goform/setWifiRelay");
	pageModule.modules = [];
	pageModule.rebootIP = location.host;
	module.exports = pageModule;
	/************wireless repeating************************/
	var wifiRepeatModule = new WifiRepeatModule();
	pageModule.modules.push(wifiRepeatModule);

	function WifiRepeatModule() {
		var subObj = {};
		this.init = function () {
			this.initEvent();
		}
		this.initEvent = function () {
			$("#refreshTable").on("click", scanWifi);
			$("#wifiScan").delegate(".selectSSID", "click", changeSSID);
			$("input[name='wifiRelayType']").on("click", changeMode);
		};
		this.initValue = function () {
			var obj = pageModule.data;
			inputValue(obj);
			if (obj.wifiEn != "true") { //未启用无线
				$("[name='wifiRelayType']")[0].checked = true;
				$("[name='wifiRelayType']").attr("disabled", true);
			}

			subObj = { //初始化
				wifiRelaySSID: obj.wifiRelaySSID,
				wifiRelayMAC: obj.wifiRelayMAC,
				wifiRelaySecurityMode: obj.wifiRelaySecurityMode,
				wifiRelayChannel: obj.wifiRelayChannel
			}

			changeMode();
			showWifiPwd(obj.wifiRelaySecurityMode);
			//scanWifi();
		}
		this.checkData = function () {
			var workMode = $("input[name='wifiRelayType']:checked").val() || "ap",
				wifiPwd = $("#wifiRelayPwd").val();
			if (workMode != "ap") {
				if (wifiPwd != "") {
					if ((/^[0-9a-fA-F]+$/).test(wifiPwd)) {
						if (wifiPwd.length < 8) {
							$("#wifiRelayPwd").focus();
							return _("The password of base station router must contain 8~63 ASCII, or 64 HEX.");

						}
					} else {
						$("#wifiRelayPwd").focus();
						return _("The password of base station router must contain 8~63 ASCII, or 64 HEX.");
					}
				} else {
					$("#wifiRelayPwd").focus();
					return _("The password of base station router must contain 8~63 ASCII, or 64 HEX.");
				}
			}
			return;
		};

		this.getSubmitData = function () {
			subObj.wifiRelayPwd = $("#wifiRelayPwd").val();
			subObj.wifiRelayType = $("input[name='wifiRelayType']:checked").val();
			return objToString(subObj);
		};

		/********扫描初始化***********/
		function scanWifi() {
			var list = [];
			$("#refreshTable").addClass("none");
			$("#loading").removeClass("none");
			$("#wifiScanTbody").html("");
			$.getJSON("goform/getWifiScan" + getRandom(), function (obj) {
				$("#loading").addClass("none");
				$("#refreshTable").removeClass("none");
				list = reSort(obj.wifiScan);
				createTable(list);
			});
		}

		function sortNumber(a, b) {
			return a - b;
		}

		function reSort(arry) {
			var listArry = arry,
				len = listArry.length,
				i = 0,
				strength,
				propArry = [], //临时存放信号强度数组
				sortArry = [], //最后返回的数组
				newArry = []; //存放排序后的数组

			for (i = 0; i < len; i++) {
				if ((+listArry[i].wifiScanSignalStrength) > 0) {
					listArry[i].wifiScanSignalStrength = 0 - (+listArry[i].wifiScanSignalStrength);
				}
				propArry.push(listArry[i].wifiScanSignalStrength);
			}

			newArry = propArry.sort(sortNumber);
			for (i = 0; i < len; i++) {
				for (var j = 0; j < listArry.length; j++) {
					if (newArry[i] == listArry[j].wifiScanSignalStrength) { // 排序好的元素中寻找原obj元素
						sortArry.push(listArry[j]); //重新排序后的obj
						listArry.splice(j, 1); //  去掉已经找到的；
						break;
					}
				}
			}
			return sortArry.reverse();
		}

		/**********初始化ssid表格**************/
		function createTable(arry) {
			var trElem, tdElem,
				i = 0,
				len = arry.length,
				signal,
				prop;
			$("#wifiScan").show();
			for (i = 0; i < len; i++) {
				trElem = document.createElement("tr");
				tdElem = document.createElement("td");
				tdElem.innerHTML = "<input type='radio' name='selectSSID' class='selectSSID'>";
				trElem.appendChild(tdElem);
				for (prop in arry[i]) {
					tdElem = document.createElement("td");
					tdElem.className = "span-fixed";
					if (prop != "wifiScanSignalStrength") {
						if (typeof tdElem.innerText != "undefined") {
							tdElem.innerText = arry[i][prop];
						} else { //firefox
							tdElem.textContent = arry[i][prop];
						}
					} else {
						signal = (+arry[i][prop]);
						if (signal > -60) {
							signal = "signal_4";
						} else if (signal <= -60 && signal > -70) {
							signal = "signal_3";
						} else if (signal <= -70 && signal > -80) {
							signal = "signal_2"
						} else {
							signal = "signal_1"
						}

						tdElem.innerHTML = "<div class='signal " + signal + "'></div>";

					}
					trElem.appendChild(tdElem);
				}
				document.getElementById("wifiScanTbody").appendChild(trElem);
			}
			top.mainLogic.initModuleHeight();
		}

		/******是否显示密码框*******/
		function showWifiPwd(security) {
			if (security.toLowerCase() != "none") {
				$("#wifiRelayPwdWrap").removeClass("none");
			} else {
				$("#wifiRelayPwdWrap").addClass("none");
			}
			top.mainLogic.initModuleHeight();
		}



		/*******选择ssid******/
		function changeSSID() {
			var $parent = $(this).parent().parent();
			var ssid = $parent.children().eq(1).text(),
				mac = $parent.children().eq(2).html(),
				channel = $parent.children().eq(3).html(),
				security = $parent.children().eq(4).html();
			$("#wifiRelayPwd").val("");
			$("#wifiRelayPwd").focus();
			$("#wifiRelaySSID").text(ssid);
			subObj = { //选择时更新提交数据
				wifiRelaySSID: ssid,
				wifiRelayMAC: mac,
				wifiRelaySecurityMode: security,
				wifiRelayChannel: channel
			}
			showWifiPwd(security);
			$("body").scrollTop(0);
		}

		/*********改变模式************/
		function changeMode() {
			var securityMode = $("input[name='wifiRelayType']:checked").val() || "ap";
			if (securityMode === "ap") {
				$("#content").hide();
				$("#wifiScan").hide();
			} else {
				$("#content").show();
			}
			top.mainLogic.initModuleHeight();
		}
	}
	/************END wireless repeating************************/

})