define(function (require, exports, module) {

	var pageModule = new PageLogic({
		getUrl: "goform/getSysTools",
		modules: "loginAuth,wanAdvCfg,lanCfg,softWare,wifiRelay,sysTime,remoteWeb,isWifiClients,systemInfo,hasNewSoftVersion",
		setUrl: "goform/setSysTools"
	});

	pageModule.modules = [];
	pageModule.moduleName = "wifiRelay";

	pageModule.initEvent = function () {
		pageModule.update("sysTime", 5000, updateTime);
	}

	pageModule.beforeSubmit = function () {
		if (pageModule.data.lanCfg.lanIP != $("#lanIP").val()) {
			if (!confirm(_('The login IP address will be changed into %s.', [$("#lanIP").val()]))) {
				return false;
			}
		}
		return true;
	};

	function updateTime(obj) {
		$("#sysTimecurrentTime").text(obj.sysTime.sysTimecurrentTime);
	}
	module.exports = pageModule;

	/*page control*/
	var pageModuleInit = new PageModuleInit();
	pageModule.modules.push(pageModuleInit);

	function PageModuleInit() {

		this.initValue = function () {
			var wifiRelayObj = pageModule.data.wifiRelay;
			if (wifiRelayObj.wifiRelayType == "ap" || wifiRelayObj.wifiRelayType == "client+ap") {
				$("#lanParame, #remoteWeb, #wanParam").addClass("none");
				$("#reminder").text(_("If this function is enabled, the router reboots at 03:00 a.m. every day."));
			} else {
				$("#reminder").text(_("If this function is enabled, the router reboots during 02:00 a.m. to 05:30 a.m. every day when the traffic is less than 3 KB/s."));
			}
		}
	}
	/*************END Page Control***********/

	/*
	 *
	 * @method loginPwdModule [显示及设置登录密码模块相关的数据]
	 * @return {无}
	 */
	var loginPwdModule = new LoginPwdModule();
	pageModule.modules.push(loginPwdModule);

	function LoginPwdModule() {

		this.moduleName = "loginAuth";
		this.data = {};

		this.init = function () {
			this.addInputEvent = false;
		}
		this.initValue = function (loginObj) {

			//点击取消时重置
			$("#newPwd, #cfmPwd, #oldPwd").removeValidateTipError(true);

			$("#newPwd, #cfmPwd,#oldPwd").val("");
			this.data = loginObj;
			if (loginObj.hasLoginPwd == "true") {
				$("#oldPwdWrap").show();
			} else {
				$("#oldPwdWrap").hide();
			}

			if (!this.addInputEvent) {
				$("#oldPwd").initPassword(_("Digits and letters only"), true);
				$("#newPwd").initPassword(_("Digits and letters only"), true);
				$("#cfmPwd").initPassword(_("Confirm Password"), true);
				this.addInputEvent = true;
			}
		}
		this.checkData = function () {
			if ($("#newPwd").val() != $("#cfmPwd").val()) {
				if ($("#cfmPwd_") && $("#cfmPwd_").length > 0) {
					if (!$.isHidden($("#cfmPwd_")[0])) {
						$("#cfmPwd_").focus();
					} else {
						$("#cfmPwd").focus();
					}
				} else {
					$("#cfmPwd").focus();
				}
				return _("Password mismatch.");
			}
			return;
		};
		this.getSubmitData = function () {
			var encode = new Encode();
			var data = {
				module1: this.moduleName,
				//oldPwd: encode($("#oldPwd").val()),
				newPwd: encode($("#newPwd").val())
			};
			if (this.data.hasLoginPwd == "true") {
				data.oldPwd = encode($("#oldPwd").val());
			}
			return objToString(data);
		}
	}
	/***********END Login Password******************/



	/*
	 * @method wanParamModule [显示及设置登录WAN口高级设置模块的数据]
	 * @return {无}
	 */
	var wanParamModule = new WanParamModule();
	pageModule.modules.push(wanParamModule);

	function WanParamModule() {
		var hostMac, routerMac;

		this.moduleName = "wanAdvCfg";

		this.data = {};

		this.init = function () {
			this.initEvent();
		}
		this.initEvent = function () {
			$("#macClone").on("change", changeMacCloneType);
			$("#wanServer").on("change", changeWanServerType);
			$("#wanService").on("change", changeWanServiceType);

		};
		this.initValue = function (wanAdvObj) {
			$("#wanServerName, #wanServiceName, #wanMTU, #macCurrentWan").removeValidateTipError(true);
			routerMac = wanAdvObj.macRouter;
			hostMac = wanAdvObj.macHost;
			this.initHtml(wanAdvObj);

			wanAdvObj.wanServer = wanAdvObj.wanServerName == "" ? "default" : "manual";
			wanAdvObj.wanService = wanAdvObj.wanServiceName == "" ? "default" : "manual";
			initMacOption(wanAdvObj);
			inputValue(wanAdvObj);
			$("#wanSpeedCurrent").html($("#wanSpeed").find("option[value='" + wanAdvObj.wanSpeedCurrent + "']").html());

			changeMacCloneType();
			changeWanServerType();
			changeWanServiceType();
		}

		this.getSubmitData = function () {

			//ap模式下与client+ap模式下不需要传wan值;
			if (pageModule.data.wifiRelay.wifiRelayType == "ap" || pageModule.data.wifiRelay.wifiRelayType == "client+ap") {
				return "";
			}

			var wanMac = "",
				macClone = $("#macClone").val();

			if (macClone == "clone") {
				wanMac = hostMac;
			} else if (macClone == "default") {
				wanMac = routerMac;
			} else {
				wanMac = $("#macCurrentWan").val().replace(/[-]/g, ":");
			}
			var data = {
				module2: this.moduleName,
				wanServerName: $("#wanServer").val() == "default" ? "" : $("#wanServerName").val(),
				wanServiceName: $("#wanService").val() == "default" ? "" : $("#wanServiceName").val(),
				wanMTU: $("#wanMTU")[0].val(),
				macClone: $("#macClone").val(),
				wanMAC: wanMac.toUpperCase(),
				wanSpeed: $("#wanSpeed").val()
			};
			return objToString(data);
		};
		this.initHtml = function (obj) {

			if (obj.wanType == "pppoe") {
				$("#serverNameWrap").show();
				$("#wanMTU").attr("data-options", '{"type":"num", "args":[576, 1492]}');
			} else {
				$("#serverNameWrap").hide();
				$("#wanMTU").attr("data-options", '{"type":"num", "args":[576, 1500]}');
			}

			$("#wanMTU").toSelect({
				"initVal": obj.wanMTU,
				"editable": "1",
				"size": "small",
				"options": [{
					"1492": "1492",
					"1480": "1480",
					"1450": "1450",
					"1400": "1400",
					".divider": ".divider",
					".hand-set": _("Manual")
				}]
			});
		}

		function initMacOption(obj) {
			var wanMac = obj.macCurrentWan;

			if (pageModule.data.isWifiClients.isWifiClients == "true") {
				$("#macClone option[value='clone']").remove();
				//obj.macClone = "manual";
			}

			if (obj.macClone == "clone" && wanMac != hostMac) {
				obj.macClone = "manual";
			}
		}

		function changeMacCloneType() {
			$("#macCurrentWan").removeValidateTipError(true);

			var macCloneType = $("#macClone").val();
			if (macCloneType == "clone") {
				$("#macCurrenWrap").html(_("Local Host MAC Address: %s", [hostMac]));
				$("#macCurrentWan").hide();
				$("#macCurrenWrap").show();
			} else if (macCloneType == "default") {
				$("#macCurrenWrap").html(_("Default MAC Address: %s", [routerMac]));
				$("#macCurrentWan").hide();
				$("#macCurrenWrap").show();
			} else {
				$("#macCurrentWan").show();
				$("#macCurrenWrap").hide();
			}
			top.mainLogic.initModuleHeight();
		}

		function changeWanServerType() {
			$("#wanServerName").removeValidateTipError(true);

			var wanServerType = $("#wanServer").val();
			$("#wanServerInfoWrap, #wanServerName").addClass("none");

			if (wanServerType == "default") {
				$("#wanServerInfoWrap").removeClass("none")
			} else {
				$("#wanServerName").removeClass("none");
			}
		}

		function changeWanServiceType() {
			$("#wanServiceName").removeValidateTipError(true);

			var wanServiceType = $("#wanService").val();
			$("#wanServiceInfoWrap, #wanServiceName").addClass("none");

			if (wanServiceType == "default") {
				$("#wanServiceInfoWrap").removeClass("none")
			} else {
				$("#wanServiceName").removeClass("none");
			}
		}
	}
	/***********END WAN Parameters***************************/

	/*
	 * @method lanModule [显示及设置登录LAN口模块的数据]
	 * @return {无}
	 */
	var lanModule = new LanModule();
	pageModule.modules.push(lanModule);

	function LanModule() {
		var _this = this;
		this.moduleName = "lanCfg";

		this.data = {};

		this.init = function () {
			this.initEvent();
		};

		this.changeIpNet = function () {
			var ipCheck = false,
				lanIP = $("#lanIP").val();
			if ((/^([1-9]|[1-9]\d|1\d\d|2[0-1]\d|22[0-3])\.(([0-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.){2}([0-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])$/).test(lanIP)) {
				ipCheck = true;
			}

			if ($("#lanMask").parent().hasClass("has-error") || $("#lanIP").parent().hasClass("has-error") || !ipCheck) {
				return;
			}

			var ipNet = "",
				ipArry = $("#lanIP").val().split(".");

			ipNet = ipArry[0] + "." + ipArry[1] + "." + ipArry[2] + ".";
			$(".ipNet").html(ipNet);

			//判断初始化时LAN IP == LAN DNS1，则在修改LAN IP时同时改变LAN DNS1
			if (_this.data.lanIP == _this.data.lanDns1) {
				$("#lanDns1").val($("#lanIP").val());
			}

		}

		this.initEvent = function () {
			$("#dhcpEn").on("click", changeDhcpEn);
			$("#lanIP").on("blur", _this.changeIpNet);
			$.validate.valid.lanMaskExt = {
				all: function (str) {
					var msg = $.validate.valid.mask.all(str);
					if (msg) {
						return msg;
					}
					if (str !== "255.255.255.0" && str !== "255.255.0.0" && str !== "255.0.0.0") {
						return _("Variable-Length Subnet Mask is not available.");
					}
				}
			}

		}
		this.initValue = function (lanCfgObj) {
			$("#lanIP, #lanMask, #lanDhcpStartIP,#lanDhcpEndIP, #lanDns1, #lanDns2").removeValidateTipError(true);

			this.data = lanCfgObj;
			inputValue(this.data);

			var ipNet = "",
				ipArry = this.data.lanDhcpStartIP.split(".");
			ipNet = ipArry[0] + "." + ipArry[1] + "." + ipArry[2] + ".";
			$(".ipNet").html(ipNet);
			$("#lanDhcpStartIP").val(this.data.lanDhcpStartIP.split(".")[3]);
			$("#lanDhcpEndIP").val(this.data.lanDhcpEndIP.split(".")[3]);
			changeDhcpEn();
		}
		this.checkData = function () {
			//client+ap 及 ap模式下没有该模块功能，不需要进行验证；
			if (pageModule.data.wifiRelay.wifiRelayType == "client+ap" || pageModule.data.wifiRelay.wifiRelayType == "ap") {
				return;
			}
			var lanIP = $("#lanIP").val(),
				lanMask = $("#lanMask").val(),
				startIP = $(".ipNet").eq(0).html() + $("#lanDhcpStartIP").val(),
				endIP = $(".ipNet").eq(0).html() + $("#lanDhcpEndIP").val(),
				wanIP = pageModule.data.systemInfo.statusWanIP,
				wanMask = pageModule.data.systemInfo.statusWanMask;

			var msg = checkIsVoildIpMask(lanIP, lanMask, _("LAN IP Address"));
			if (msg) {
				$("#lanIP").focus();
				return msg;
			}

			if ((/^([1-9]|[1-9]\d|1\d\d|2[0-1]\d|22[0-3])\.(([0-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.){2}([0-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])$/).test(wanIP)) {

				if (checkIpInSameSegment(lanIP, lanMask, wanIP, wanMask)) {
					$("#lanIP").focus();
					return _("%s and %s (%s) cannot be in the same network segment.", [_("LAN IP Address"), _("WAN IP Address"), wanIP]);
				}
			}

			if (lanMask !== "255.255.255.0" && lanMask !== "255.255.0.0" && lanMask !== "255.0.0.0") {
				return _("Subnet mask error. Variable-Length Subnet Mask is not available.");
			}



			if ($("#dhcpEn")[0].checked) {
				if (!checkIpInSameSegment(startIP, lanMask, lanIP, lanMask)) {
					$("#lanDhcpStartIP").focus();
					return _("%s and %s must be in the same network segment.", [_("Start IP Address"), _("LAN IP Address")]);
				}

				msg = checkIsVoildIpMask(startIP, lanMask, _("Start IP Address"));
				if (msg) {
					$("#lanDhcpStartIP").focus();
					return msg;
				}

				if (!checkIpInSameSegment(endIP, lanMask, lanIP, lanMask)) {
					$("#lanDhcpEndIP").focus();
					return _("%s and %s must be in the same network segment.", [_("End IP Address"), _("LAN IP Address")]);
				}

				msg = checkIsVoildIpMask(endIP, lanMask, _("End IP Address"));
				if (msg) {
					$("#lanDhcpEndIP").focus();
					return msg;
				}

				var sipArry = startIP.split("."),
					eipArry = endIP.split("."),
					sipNumber,
					eipNumber;
				sipNumber = parseInt(sipArry[0], 10) * 256 * 256 * 256 + parseInt(sipArry[1], 10) * 256 * 256 + parseInt(sipArry[2], 10) * 256 + parseInt(sipArry[3], 10);
				eipNumber = parseInt(eipArry[0], 10) * 256 * 256 * 256 + parseInt(eipArry[1], 10) * 256 * 256 + parseInt(eipArry[2], 10) * 256 + parseInt(eipArry[3], 10);
				if (sipNumber > eipNumber) {
					$("#lanDhcpEndIP").focus();
					return _("The start IP address cannot be greater than the end IP address.");
				}
				if ($("#lanDns1").val() == $("#lanDns2").val()) {
					return _("Preferred DNS server and Alternate DNS server cannot be the same.");
				}
			}
			return;
		}
		this.getSubmitData = function () {
			//ap模式下与client+ap模式下不需要传wan值;
			if (pageModule.data.wifiRelay.wifiRelayType == "ap" || pageModule.data.wifiRelay.wifiRelayType == "client+ap") {
				return "";
			}

			var data = {
				module3: this.moduleName,
				lanIP: $("#lanIP").val(),
				lanMask: $("#lanMask").val(),
				dhcpEn: $("#dhcpEn")[0].checked == true ? "true" : "false",
				lanDhcpStartIP: $(".ipNet").eq(0).html() + $("#lanDhcpStartIP").val(),
				lanDhcpEndIP: $(".ipNet").eq(0).html() + $("#lanDhcpEndIP").val(),
				//lanDhcpLeaseTime: $("#lanDhcpLeaseTime").val(),
				lanDns1: $("#lanDns1").val(),
				lanDns2: $("#lanDns2").val()
			};
			return objToString(data);
		}

		function changeDhcpEn() {
			if ($("#dhcpEn")[0].checked) {
				$("#dnsWrap").show();
			} else {
				$("#dnsWrap").hide();
			}
			top.mainLogic.initModuleHeight();
		}
	}
	/***********EDN LAN Parameters*************/



	/***********Remote Web Management*******************/
	var remoteModule = new RemoteModule();
	pageModule.modules.push(remoteModule);

	function RemoteModule() {
		this.moduleName = "remoteWeb";
		this.init = function () {
			this.initEvent();
		};
		this.initEvent = function () {
			$("#remoteWebEn").on("click", changeRemoteEn);
			$("#remoteWebType").on("change", changeRemoteWebType);
		}
		this.initValue = function (obj) {
			inputValue(obj);
			changeRemoteEn();
			changeRemoteWebType();
		}
		this.checkData = function () {
			//client+ap 及 ap模式下没有该模块功能，不需要进行验证；
			if (pageModule.data.wifiRelay.wifiRelayType == "client+ap" || pageModule.data.wifiRelay.wifiRelayType == "ap") {
				return;
			}

			var lanIP = $("#lanIP").val(),
				lanMask = $("#lanMask").val(),
				remoteWebIP = $("#remoteWebIP").val();

			if ($("#remoteWebEn")[0].checked && $("#remoteWebType").val() == "specified") {
				var msg = checkIsVoildIpMask(remoteWebIP, "255.255.255.0", _("Remote IP Address"));
				if (msg) {
					$("#remoteWebIP").focus();
					return msg;
				}

				if (remoteWebIP == lanIP) {
					$("#remoteWebIP").focus();
					return _("%s cannot be the same as the %s (%s).", [_("Remote IP Address"), _("LAN IP Address"), lanIP]);
				}

				if (checkIpInSameSegment(remoteWebIP, lanMask, lanIP, lanMask)) {
					$("#remoteWebIP").focus();
					return _("%s and %s (%s) cannot be in the same network segment.", [_("Remote IP Address"), _("LAN IP Address"), lanIP]);
				}
			}
		}
		this.getSubmitData = function () {
			//ap模式下与client+ap模式下不需要传wan值;
			if (pageModule.data.wifiRelay.wifiRelayType == "ap" || pageModule.data.wifiRelay.wifiRelayType == "client+ap") {
				return "";
			}

			var data = {
				module4: this.moduleName,
				remoteWebEn: $("#remoteWebEn")[0].checked == true ? "true" : "false",
				remoteWebType: $("#remoteWebType").val(),
				remoteWebIP: $("#remoteWebIP").val(),
				remoteWebPort: $("#remoteWebPort").val()
			};
			return objToString(data);
		}

		function changeRemoteEn() {
			if ($("#remoteWebEn")[0].checked) {
				$("#remoteWrap").show();
			} else {
				$("#remoteWrap").hide();
			}
			top.mainLogic.initModuleHeight();
		}

		function changeRemoteWebType() {
			if ($("#remoteWebType").val() == "any") {
				$("#remoteWebIP").parent().hide();
			} else {
				$("#remoteWebIP").parent().show();
			}
			top.mainLogic.initModuleHeight();
		}
	}
	/***********END Remote Web Management***************/

	/***********Date & Time***************/
	//if(CONFIG_HASSYSTIME === true){
		var timeModule = new TimeModule();
		pageModule.modules.push(timeModule);

		function TimeModule() {

			this.moduleName = "sysTime";
			this.initValue = function (obj) {
				inputValue(obj);
				if (obj.internetState == "true") {
					$("#internetTips").show();
				} else {
					$("#internetTips").hide();
				}
			}
			this.getSubmitData = function () {
				var data = {
					module5: this.moduleName,
					sysTimeZone: $("#sysTimeZone").val()
				};
				return objToString(data);
			}
		}
		
	//}
	/***********END Date & Time**************/

	 

	/*
	 * 显示及设置本设置的系统信息
	 * @method manageModule
	 * @param {Object} softWare 从后台获取的关于设置的信息
	 * @return {无}
	 */
	var manageModule = new ManageModule();
	pageModule.modules.push(manageModule);

	function ManageModule() {

		this.moduleName = "softWare";

		this.init = function () {
			this.initEvent();
			goUpgrade();
			goInport();
		};
		this.initEvent = function () {
			$("#reboot").on("click", function () {
				var $this = $(this);
				$this.attr("disabled", true);

				if (confirm(_("Reboot the device?"))) {
					$(this).blur();

					$.post("goform/sysReboot", "module1=sysOperate&action=reboot", function (str) {
						if (checkIsTimeOut(str)) {
							top.location.reload(true);
							return;
						}
						var num = $.parseJSON(str).errCode;
						if (num == 100) {
							dynamicProgressLogic.init("reboot", "", 450);
							clearTimeout(pageModule.updateTimer);
						}
					})
				}
				$this.removeAttr("disabled");

			});

			$("#restore").on("click", function () {
				var $this = $(this);
				$this.attr("disabled", true);
				if (confirm(_("Restoring the factory settings clears all current settings of the router."))) {
					$(this).blur();
					$.post("goform/sysRestore", "module1=sysOperate&action=restore", function (str) {
						if (checkIsTimeOut(str)) {
							top.location.reload(true);
							return;
						}
						var num = $.parseJSON(str).errCode;
						if (num == 100) {
							var jumpIp = (window.location.href.indexOf('tendawifi') == -1 ? '192.168.0.1' : '');
							dynamicProgressLogic.init("restore", _("Resetting... Please wait."), 450, jumpIp);
							clearTimeout(pageModule.updateTimer);
						}
					});
				}
				$this.removeAttr("disabled");
			});

			$("#export").on("click", function () {
				window.location = "/cgi-bin/DownloadSyslog/RouterSystem.log?" + Math.random();
			});

			$("#exportConfig").on("click", function () {
				window.location = "/cgi-bin/DownloadCfg/RouterCfm.cfg?" + Math.random();
			});

			$("#onlineUpgradeBtn").on("click", function () {

				$.getData("goform/getHomePageInfo?" + Math.random(), "hasNewSoftVersion", function (obj) {
					onineUpgradeLogic.init("system", obj);
				});
			});

		};
		this.initValue = function (softObj) {
			$("#autoMaintenanceEn")[0].checked = (softObj.autoMaintenanceEn == "true");
			$("#firmwareVision").html(softObj.softVersion);

		};
		this.checkData = function () {
			return;
		};
		this.getSubmitData = function () {
			pageModule.rebootIP = $("#lanIP").val();
			var data = {
				module6: this.moduleName,
				autoMaintenanceEn: $("#autoMaintenanceEn")[0].checked == true ? "true" : "false"
			};
			return objToString(data);
		};

		function goUpgrade() {
			pageModule.upgradeLoad = new AjaxUpload("upgrade", {
				action: './cgi-bin/upgrade',
				name: 'upgradeFile',
				responseType: 'json',

				onSubmit: function (file, ext) {
					if (confirm(_("Upgrade the device?"))) {
						if (!ext) {
							return false;
						}
	
					} else {
						document.upgradefrm.reset();
						return false;
					}
				},
				onComplete: function (file, msg) {
					if (typeof msg == 'string' && checkIsTimeOut(msg)) {
						top.location.reload(true);
						return;
					}
					var num = msg.errCode;
					if (num == "100") {
						dynamicProgressLogic.init("upgrade", "", 450);
					} else if (num == "201") {
						mainLogic.showModuleMsg(_("Firmware error.") + " " + _("The router will reboot."));
						setTimeout(function () {
							dynamicProgressLogic.init("reboot", "", 450);
						}, 2000);
						clearTimeout(pageModule.updateTimer);
					} else if (num == "202") {
						mainLogic.showModuleMsg(_("Upgrade failed."));
					} else if (num == "203") {
						mainLogic.showModuleMsg(_("The firmware size is too large.") + " " + _("The router will reboot."));
						setTimeout(function () {
							dynamicProgressLogic.init("reboot", "", 450);
						}, 2000)
						clearTimeout(pageModule.updateTimer);
					}
				}
			});
		}

		function goInport() {
			pageModule.inport = new AjaxUpload("inport", {
				action: './cgi-bin/UploadCfg',
				name: 'inportFile',
				responseType: 'json',

				onSubmit: function (file, ext) {
					if (confirm(_('Restore now?'))) {
						if (!ext) {
							return false;
						}

					} else {
						document.inportfrm.reset();
						return false;
					}
				},
				onComplete: function (file, msg) {
					if (typeof msg == 'string' && checkIsTimeOut(msg)) {
						top.location.reload(true);
						return;
					}
					var num = msg.errCode;
					if (num == "100") {
						dynamicProgressLogic.init("reboot", "", 450);
					} else if (num == "201") {
						mainLogic.showModuleMsg(_("Firmware error.") + " " + _("The router will reboot."));
						setTimeout(function () {
							dynamicProgressLogic.init("reboot", "", 450);
						}, 2000);
					} else if (num == "202") {
						mainLogic.showModuleMsg(_("Failed to import the configurations."));
						setTimeout(function () {
							dynamicProgressLogic.init("reboot", "", 450);
						}, 2000);
					}
					clearTimeout(pageModule.updateTimer);
				}
			});
		}

	}
	/***********END Device Management**********/

})
