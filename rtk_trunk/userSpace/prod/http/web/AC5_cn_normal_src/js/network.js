define(function (require, exports, module) {
	var pageModule = new PageLogic({
		getUrl: "goform/getWAN",
		modules: "wifiBasicCfg,wifiRelay,lanCfg,wanBasicCfg,wanAdvCfg,internetStatus",
		setUrl: "goform/setWAN"
	});

	pageModule.modules = [];
	pageModule.rebootIP = location.host;
	module.exports = pageModule;
	module.IPConflictAlert = false;
	pageModule.preMode = "";
	pageModule.currentMode = "disabled";

	var do_close_flag = true;

	pageModule.initEvent = function () {

		pageModule.update("internetStatus", 3000, function (obj) {
			obj = obj.internetStatus;

			updateInternetConnectStatus(obj, "wanConnectStatus");
		});

		$("#ok").on("click", function () {
			do_close_flag = false;

			var $this = $(this);
			$this.attr("disabled", true);
			var str = wifiRepeatModule.getSubmitData();
			$.ajax({
				url: "goform/setWifiRelay",
				type: "POST",
				data: str,
				success: function (obj) {

					if (checkIsTimeOut(obj)) {
						top.location.reload(true);
					}
					dialog.close();

					//client+ap或ap模式下，重启后跳转至tendawifi.com
					if (pageModule.currentMode == "client+ap" || pageModule.currentMode == "ap") {
						dynamicProgressLogic.init("reboot", "", 450, "tendawifi.com");
					} else {
						//重启后刷新当前页面
						dynamicProgressLogic.init("reboot", "", 450);
					}
				},
				error: function () {
					$this.removeAttr("disabled");
					mainLogic.showModuleMsg(_("Failed to upload the data."));
				}
			});
		});

		//点击蒙版时，可以关闭弹框
		$("#progress-overlay").on("click",function() {
			if(do_close_flag) {
				dialog.close({
					Id: "changeModeInfo"
				});
			}else{
				return;
			}
			
		});

	}

	pageModule.beforeSubmit = function () {
		var mode = $("input[name=wifiRelayType]:checked")[0].value,
			msg = "",
			rebootFlag;
		if (mode == "wisp" || mode == "client+ap") {
			return;
		}

		if (mode == pageModule.preMode && (pageModule.preMode == "disabled" || pageModule.preMode == "ap")) {
			rebootFlag = false;
		} else {
			rebootFlag = true;
			switch (mode) {
			case "disabled":
				if (pageModule.preMode == "ap") {
					msg = _("When the AP mode is disabled, the router reboots. Do you want to disable this mode?");
				} else if (pageModule.preMode == "wisp") {
					msg = _("When the WISP mode is disabled, the router reboots. Do you want to disable this mode?");
				} else if (pageModule.preMode == "client+ap") {
					msg = _("When the Universal Repeater mode is disabled, the router reboots. Do you want to disable this mode?");
				}
				break;
			case "ap":
				msg = _("When the AP mode is enabled, the router reboots. After the reboot process is complete, please use tendawifi.com to log in to the web UI.");
				break;
			case "client+ap":
				msg = _("When the Universal Repeater mode is enabled, the router reboots. After the reboot process is complete, please use tendawifi.com to log in to the web UI.");
				break;
			case "wisp":
				msg = _("When the WISP mode is enabled, the router reboots. Do you want to enable this mode?");
				break;
			}

		}

		pageModule.currentMode = mode;

		if (rebootFlag) {
			$("#modeMsg").html(msg);
			dialog.open({
				Id: "changeModeInfo",
				height: 300
			});
			return false;
		}
		return true;
	}

	/************wireless repeating************************/
	var wifiRepeatModule = new WifiRepeatModule();
	pageModule.modules.push(wifiRepeatModule);

	function WifiRepeatModule() {
		var wifiRelayTmpObj = {}, 
			ssIDModal = null;

		this.moduleName = "wifiRelay";

		this.init = function () {
			this.initEvent();
		}
		this.initEvent = function () {
			$("#refreshTable").on("click", scanWifi);

			$("#wifiScanTbody").delegate(".selectSSID", "click", changeSSID);
			$("#wifiScanTbody").delegate("td", "click", changeSSID);

			$("input[name='wifiRelayType']").on("click", changeMode);

			$("#wifiRelayPwd").on("keyup", changeConnectBtnStatus);
			$("#connectRelay").on("click", submitWifiRelay);
			$("#sure").on("click", submitWifiRelay);
			$("#cancel-model").on("click",function(){
				ssIDModal && ssIDModal.hide();
			});
			if (!this.addInputEvent) {
				$("#wifiRelayPwd").val("").initPassword(_("Password of the upstream WiFi network"));
				//解决IE下事件绑定不生效的问题;
				if ($("#wifiRelayPwd_").length === 1) {
					$("#wifiRelayPwd_").on("keyup", changeConnectBtnStatus);
				}
				this.addInputEvent = true;
			}
		};
		this.initValue = function (wifiRelayobj) {

			pageModule.preMode = wifiRelayobj.wifiRelayType;
			inputValue(wifiRelayobj);
			$("#wifiRelaySSID").html("<p class='form-control-static'></p>");
			$("#wifiRelaySSID p").text(wifiRelayobj.wifiRelaySSID);

			//未启用无线
			if (!(pageModule.data.wifiBasicCfg.wifiEn == "true" || pageModule.data.wifiBasicCfg.wifiEn_5G == "true")) { 
				$("[name='wifiRelayType']")[0].checked = true;
				$("[name='wifiRelayType']").attr("disabled", true);
			}
			showConnectStatus(wifiRelayobj.wifiRelayConnectStatus);

			wifiRelayTmpObj = { //初始化
				wifiRelaySSID: wifiRelayobj.wifiRelaySSID,
				wifiRelayMAC: wifiRelayobj.wifiRelayMAC,
				wifiRelaySecurityMode: wifiRelayobj.wifiRelaySecurityMode,
				wifiRelayChannel: wifiRelayobj.wifiRelayChannel,
				wifiRelayChkHz: wifiRelayobj.wifiRelayChkHz
			};
			changeMode();
		};

		this.checkData = function () {
			return;
		};

		this.getSubmitData = function () {
			var subObj = {};

			subObj.module1 = this.moduleName;

			subObj.wifiRelayType = $("input[name='wifiRelayType']:checked").val();
			//模式修改，且修改为路由模式后，需要提交WAN口数据
			if (subObj.wifiRelayType == "disabled" && pageModule.preMode != "disabled") {
				return objToString(subObj) + "&" + wanModule.getSubmitData();
			}

			return objToString(subObj);
		};

		/********扫描初始化***********/
		function scanWifi() {
			var list = [];
			$("#refreshTable").addClass("none");
			$("#loading").removeClass("none");

			$("#wifiScan").find("table").addClass("none"); //扫描时隐藏表格
			$.getJSON("goform/getWifiRelay" + getRandom() + "&modules=wifiScan", function (obj) {
				try {
					var test = obj.wifiScan.length;
				} catch (e) {
					obj.wifiScan = [];
				}
				$("#loading").addClass("none");
				$("#refreshTable").removeClass("none");
				list = reSort(obj.wifiScan);
				createTable(list);
				$("#wifiScan").find("table").removeClass("none");
				top.mainLogic.initModuleHeight();
			});

			//扫描时增加2s遮盖层
			addOverLay(1000);
		}

		function showConnectStatus(status) {
			var str = "",
				stObj = {
					"disconnect": _("Disconnected"),
					"bridgeSuccess": _("Bridged successfully."),
					"pwdError": _("Password error.")
				};

			if (status == "disconnect") { //未连接
				str = "text-primary";
			} else if (status == "bridgeSuccess") { //中继成功
				str = "text-success";
			} else { //错误; pwdError
				str = "text-danger";
			}

			$("#wifiRelayStatus").attr("class", str).html(stObj[status]);
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
				signalClass,
				prop;
			if (!pageModule.pageRunning) {
				return;
			}
			if (len == 0) {
				$("#wifiScanTbody").html('<tr><td colspan="3" class="text-danger">' + _("SSID scanning timed out.") + '</td></tr>');
				return;
			}

			$("#wifiScanTbody").html("");
			for (i = 0; i < len; i++) {
				trElem = document.createElement("tr");
				tdElem = document.createElement("td");
				tdElem.innerHTML = "<input type='radio' name='selectSSID' class='selectSSID'>";
				trElem.appendChild(tdElem);
				for (prop in arry[i]) {
					if(prop == "wifiScanChkHz") {
						continue;
					}

					tdElem = document.createElement("td");

					if (prop == "wifiScanMAC" || prop == "wifiScanChannel" ||
						prop == "wifiScanSecurityMode") {
						tdElem.className = "span-fixed hidden-xs";
					} else {
						tdElem.className = "span-fixed";
					}

					if (prop != "wifiScanSignalStrength") {
						var secuModeText = arry[i][prop];
						if (prop == "wifiScanSecurityMode") {
							//未加密时进行语言翻译；
							if (arry[i][prop].toLowerCase() == "none") {
								secuModeText = _("None");
							} else { //有加密时转换成大写显示
								secuModeText = arry[i][prop].toUpperCase();
							}

							$(tdElem).attr("data-title", arry[i][prop]);

						}

						if (typeof tdElem.innerText != "undefined") {
							tdElem.innerText = secuModeText;
						} else { //firefox
							tdElem.textContent = secuModeText;
						}

						if (prop == "wifiScanSSID") {
							$(tdElem).attr("title", arry[i][prop]);

							$(tdElem).attr("data-title", arry[i][prop]);
							$(tdElem).attr("data-hzType", arry[i].wifiScanChkHz);

							$(tdElem).addClass("wifi-ssid-target");
						}

					} else {
						signal = (+arry[i][prop]);
						if (signal > -60) {
							signalClass = "signal_4";
						} else if (signal <= -60 && signal > -70) {
							signalClass = "signal_3";
						} else if (signal <= -70 && signal > -80) {
							signalClass = "signal_2"
						} else {
							signalClass = "signal_1"
						}

						tdElem.innerHTML = "<div class='signal " + signalClass + "'><span style='position:relative;left:21px;top:3px;'>" + translatSignal(signal) + "</span></div>";

					}
					trElem.appendChild(tdElem);

					//增加5G标识
					if (arry[i].wifiScanChkHz == "5G") {
		                // trElem.childNodes[1].innerHTML = arry[i].wifiScanSSID + "<img style='padding-left:20px' src='/img/5g.png'>";
		                trElem.childNodes[1].innerHTML = "<div class='span-fixed' style='display:inline-block;'>" + arry[i].wifiScanSSID + "</div><img style='padding-left:10px;display: inline-block;margin-top: -10px;' src='/img/5g.png'>";
		            }
		            // trElem.childNodes[1].className = "span-fixed";
				}
				if (document.getElementById("wifiScanTbody")) {
					document.getElementById("wifiScanTbody").appendChild(trElem);
				}
			}
		}

		/***********信号转换函数***********/
		function translatSignal(percent) {
			var newPer = 0;
			if (percent >= -30) {
				newPer = "100%";
			} else if (percent >= -45) {
				newPer = Math.round(100 - (-30 - percent) / 1.5) + "%";
			} else if (percent >= -55) {
				newPer = 90 - (-45 - percent) * 2 + "%";
			} else if (percent >= -70) {
				newPer = 70 - (-55 - percent) * 2 + "%";
			} else if (percent >= -85) {
				newPer = 40 - (-70 - percent) * 2 + "%";
			} else if (percent >= -95) {
				newPer = 10 - (-85 - percent) + "%";
			} else {
				newPer = "0%"
			}
			return newPer;
		}


		/******是否显示密码框*******/
		function showWifiPwd(security) {
			$("#hasRelayPwd, #noRelayPwd").addClass("none");
			//未加密时隐藏密码输入框，弹出框标题显示"提示"
			if (security.toLowerCase() == "none") {
				$("#noRelayPwd").removeClass("none");
				// $("#setRelayWrapper .dialog-tips").html(_("Note"));
				ssIDModal && ssIDModal.setTitle(_("Note"));
				//已加密时显示密码输入框，弹出框标题显示”输入密码“,密码输入框聚焦；
			} else {
				// $("#wifiRelayPwd").focus();
				// $("#setRelayWrapper .dialog-tips").html(_("Enter Password"));

				ssIDModal && ssIDModal.setTitle(_("Enter Password"));
				$("#hasRelayPwd").removeClass("none");
			}

			top.mainLogic.initModuleHeight();
		}

		/*******选择ssid******/
		function changeSSID() {
			var $parent = "",
				domClass = $(this).context.className;

			if(domClass == "selectSSID"){
				$parent = $(this).parent().parent();
			}else{
				$parent = $(this).parent();

				//勾选上当前行的radio
				$parent.children().eq(0).children()[0].checked = true;
			}

			var ssid = $parent.children().eq(1).attr("data-title"),
				mac = $parent.children().eq(2).html(),
				channel = $parent.children().eq(3).html(),
				security = $parent.children().eq(4).attr("data-title"),
				signalStrength = $parent.children().eq(5).find('span').html(),
				hzType = $parent.children().eq(1).attr("data-hzType");

			if (+(signalStrength.replace(/[%]/, "")) < 40) {
				top.mainLogic.showModuleMsg(_("The signal strength of the remote wireless network is weak and the wireless bridge may be stopped. Place the router at a location with better signal strength."));
			}


			$("#wifiRelayPwd").val("").removeValidateTipError(true);

			$("#connectRelay").attr("disabled", true).removeClass("btn-primary");

			if (ssid == "") {
				$("#wifiRelaySSID").html('<input type="text" maxlength="32" placeholder="' + _("WiFi name of the upstream router") + '" class="form-control validatebox" data-options="{\'type\': \'ssid\'}" />');
				$("#wifiRelaySSID input").val(ssid);
			} else {

				$("#wifiRelaySSID").html('<p class="form-control-static"></p>');
				$("#wifiRelaySSID p").text(_(_('Enter the password of "%s".'), [ssid])).attr("data-title", ssid);
			}

			wifiRelayTmpObj = { //选择时更新提交数据
				wifiRelaySSID: ssid,
				wifiRelayMAC: mac,
				wifiRelaySecurityMode: security,
				wifiRelayChannel: channel,
				wifiRelayChkHz: hzType
			};
			showWifiPwd(security);

			window.scrollTo(0, 0);


			if(ssIDModal){
				ssIDModal.show();
			}else{
				var title = "";
				if (security.toLowerCase() == "none") {
					title = _("Note");
				} else {
					title = _("Enter Password");
				}
				ssIDModal = $.modalDialog({
					title:title,
					content:$("#setRelayWrapper")
				});
			}

			setTimeout(function () {
				if(security.toLowerCase() !== "none") {
					$("#wifiRelayPwd").focus();
				}
			}, 100);

		}
		/*********改变模式************/
		function changeMode() {
			var securityMode = $("input[name='wifiRelayType']:checked").val() || "disabled";

			$("#routerInfo, #wispInfo, #clientApInfo, #apInfo").addClass("none");
			switch (securityMode) {
			case "disabled":
				$("#wanCfgWrapper").removeClass("none");

				$("#content").hide();
				$("#wifiScan").hide();
				$("#routerInfo").removeClass("none");
				$("#submit").css("cursor", "pointer");

				break;

			case "wisp":
				scanWifi();
				$("#wanCfgWrapper").addClass("none");
				$("#content").show();
				$("#wifiScan").show();
				$("#wispInfo").removeClass("none");
				$("#submit").css("cursor", "not-allowed");
				break;

			case "client+ap":
				scanWifi();
				$("#wanCfgWrapper").addClass("none");
				$("#content").show();
				$("#wifiScan").show();
				$("#clientApInfo").removeClass("none");
				$("#submit").css("cursor", "not-allowed");

				break;

			case "ap":
				$("#wanCfgWrapper").addClass("none");
				$("#content").hide();
				$("#wifiScan").hide();
				$("#apInfo").removeClass("none");
				$("#submit").css("cursor", "pointer");

				break;
			}

			top.mainLogic.initModuleHeight();
		}

		function changeConnectBtnStatus() {
			var _this = this;
			var val = _this.value;
			if (val.length > 7) {
				$("#connectRelay").removeAttr("disabled").addClass("btn-primary");
			} else {
				$("#connectRelay").attr("disabled", true).removeClass("btn-primary");
			}
		}

		/**
		 * @method [提交wisp或xx模式]
		 */
		function submitWifiRelay() {
			do_close_flag = false;

			var $this = $(this);
			$this.attr("disabled", true);

			//如果不加密，则传空的密码过去
			wifiRelayTmpObj.wifiRelayPwd = wifiRelayTmpObj.wifiRelaySecurityMode == "none" ? "" : $("#wifiRelayPwd").val();

			wifiRelayTmpObj.wifiRelayType = $("input[name=wifiRelayType]:checked")[0].value;
			wifiRelayTmpObj.module1 = "wifiRelay";
			$.ajax({
				url: "goform/setWAN",
				type: "POST",
				data: objToString(wifiRelayTmpObj),
				success: function (obj) {
					if (checkIsTimeOut(obj)) {
						top.location.reload(true);
					}
					dialog.close();

					//client+ap或ap模式下，重启后跳转至tendawifi.com
					dynamicProgressLogic.init("reboot", "", 450, "tendawifi.com");
				},
				error: function () {
					$this.removeAttr("disabled");

					mainLogic.showModuleMsg(_("Failed to upload the data."));
				}
			});
		}

	}
	/************END wireless repeating************************/


	/*
	 * @method wanModule [用于显示及配置WAN口基本设置的构造函数]
	 * @return {无}
	 */
	var wanModule = new WanModule();
	pageModule.modules.push(wanModule);

	function WanModule() {
		var _this = this;
		_this.data = {};

		_this.synchroConfigObj = {};

		this.moduleName = "wanBasicCfg";

		this.init = function () {
			this.initEvent();
		};

		this.initEvent = function () {
			this.addInputEvent = false;
			$("#wanIP").toTextboxs("ip-mini");
			$("#wanMask").toTextboxs("ip-mini", "255.255.255.0");
			$("#wanGateway").toTextboxs("ip-mini");
			$("#wanDns1").toTextboxs("ip-mini");
			$("#wanDns2").toTextboxs("ip-mini");

			$("input[name='wanType']").on('click', this.changeWanType);

			$("#wan").delegate(".textboxs", "focus.textboxs blur.textboxs", function () {
				$(this).val(this.val());
			});
			$("#connectInfo").delegate("#cloneMac", "click", this.cloneMAC);
			$(".textboxs").each(function () {
				$(this).val(this.val());
			});

			//解决IE下高度不自动变化问题
			$("#static-set input").on("blur", function () {
				top.mainLogic.initModuleHeight();
			});
		};

		/**
		 * @method [initValue] [初始化wan基本参数配置数据及连接状态，检测WAN口冲突]
		 * @param  {[type]} obj [description]
		 * @return {[type]}     [description]
		 */
		this.initValue = function (obj) {
			$("#wanPPPoEUser, #wanPPPoEPwd, #wanIP, #wanMask, #wanGateway, #wanDns1, #wanDns2").removeValidateTipError(true);

			_this.data = pageModule.data;
			this.rebootIP = _this.data.lanCfg.lanIP;
			inputValue(obj);

			//只有在static时才赋值IP地址
			if (obj.wanType != "static") {
				$("#wanIP")[0].val("");
				$("#wanMask")[0].val("");
				$("#wanGateway")[0].val("");
				$("#wanDns1")[0].val("");
				$("#wanDns2")[0].val("");
			}

			//作用标记使用，让addplaceholder, initPassword只执行一次
			if (!this.addInputEvent) {
				$("#wanPPPoEUser").addPlaceholder(_("User Name from ISP"));
				$("#wanPPPoEPwd").initPassword(_("Password from ISP"));
				this.addInputEvent = true;
			}
			this.changeWanType();

			updateInternetConnectStatus(_this.data.internetStatus, "wanConnectStatus");
		};

		this.changeWanType = function () {
			var wanType = $("input[name='wanType']:checked").val();
			if (wanType == "pppoe") {
				$("#dhcp-set").addClass("none");
				$("#pppoe-set").removeClass("none");
				$("#static-set").addClass('none');
			} else if (wanType == "dhcp") {
				$("#dhcp-set").removeClass("none");
				$("#pppoe-set").addClass("none");
				$("#static-set").addClass('none');
			} else {
				$("#dhcp-set").addClass("none");
				$("#pppoe-set").addClass("none");
				$("#static-set").removeClass('none');
			}

			//当前模式为路由模式且上网方式未切换时，显示联网状态；
			if (_this.data.wifiRelay.wifiRelayType == "disabled" && _this.data.wanBasicCfg.wanType == wanType) {
				$("#connectInfo").removeClass("none");
			} else {
				$("#connectInfo").addClass("none");
			}

			top.mainLogic.initModuleHeight();
		};

		this.cloneMAC = function () {

			var macHost = pageModule.data.wanAdvCfg.macHost,
				obj = {
					"module1": "wanAdvCfg",
					"macClone": "clone",
					"wanMAC": macHost
				},
				submitStr = "";

			submitStr = objToString(obj);
			$.post("goform/setMacClone?" + Math.random(), submitStr, function (str) {
				if (checkIsTimeOut(str)) {
					top.location.reload(true);
					return;
				}
				var num = $.parseJSON(str).errCode || "-1";
				if (num == 0) {
					mainLogic.showModuleMsg(_("The MAC address is cloned successfully."));
				}
			})
		}
		this.checkData = function () {
			//AP模式时不用验证
			var workMode = $("input[name='wifiRelayType']:checked").val();
			if (workMode == "ap") {
				return;
			}

			var wanType = $("input[name='wanType']:checked").val(),
				ip = $("#wanIP")[0].val(),
				mask = $("#wanMask")[0].val(),
				gateway = $("#wanGateway")[0].val(),
				dns1 = $("#wanDns1")[0].val(),
				dns2 = $("#wanDns2")[0].val(),
				msg = "";
			var lanIp = _this.data.lanCfg.lanIP,
				lanMask = _this.data.lanCfg.lanMask;
			if (wanType == "pppoe") {

			} else if (wanType == "static") {
				msg = checkIsVoildIpMask(ip, mask, _("IP Address"));
				if (msg) {
					return msg;
				}
				if (checkIpInSameSegment(ip, mask, lanIp, lanMask)) {
					return _("%s and %s (%s) cannot be in the same network segment.", [_("WAN IP Address"), _("LAN IP Address"), lanIp]);
				}
				if (!checkIpInSameSegment(ip, mask, gateway, mask)) {
					return _("%s and %s must be in the same network segment.", [_("WAN IP Address"), _("Gateway")]);
				}

				if (ip == gateway) {
					return _("WAN IP Address and Default Gateway cannot be the same.");;
				}

				if (dns1 == dns2) {
					return _("Preferred DNS server and Alternate DNS server cannot be the same.");
				}
			}
			return;
		};

		this.getSubmitData = function () { //获取提交数据
			var data = {
				module2: _this.moduleName,
				wanType: $("input[name='wanType']:checked").val(),
				wanPPPoEUser: $("#wanPPPoEUser").val(),
				wanPPPoEPwd: $("#wanPPPoEPwd").val(),
				wanIP: $("#wanIP")[0].val(),
				wanMask: $("#wanMask")[0].val(),
				wanGateway: $("#wanGateway")[0].val(),
				wanDns1: $("#wanDns1")[0].val(),
				wanDns2: $("#wanDns2")[0].val()
			}
			return objToString(data);

		}
	}
	/************END WAN Setting*************/
})