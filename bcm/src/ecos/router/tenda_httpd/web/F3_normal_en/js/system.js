define(function (require, exports, module) {

	var pageModule = new PageLogic("goform/getSysTools", "goform/setSystem");
	pageModule.modules = [];
	pageModule.beforeSubmit = function () {
		if (pageModule.data.lan.lanIP != $("#lanIP").val()) {
			if (!confirm(_('The login IP will be changed into %s.', [$("#lanIP").val()]))) {
				return false;
			}
		}
		return true;
	}
	module.exports = pageModule;

	/***********Login Password****************/
	var loginPwdModule = new LoginPwdModule();
	pageModule.modules.push(loginPwdModule);

	function LoginPwdModule() {

		this.initValue = function () {
			var obj = pageModule.data;
			if (obj.firmware.hasPwd == "true") {
				$("#oldPwdWrap").show();
			} else {
				$("#oldPwdWrap").hide();
			}
			$("#oldPwd").val("");
		}
		this.checkData = function () {
			if ($("#newPwd").val() != $("#cfmPwd").val()) {
				$("#cfmPwd").focus();
				return _("Password mismatch!");
			}
			return;
		};
		this.getSubmitData = function () {
			var encode = new Encode();
			var data = {
				oldPwd: encode($("#oldPwd").val()),
				newPwd: encode($("#newPwd").val())
			};
			return objToString(data);
		}
	}
	/***********END Login Password******************/

	/***********WAN Parameters*************************/
	var wanParamModule = new WanParamModule();
	pageModule.modules.push(wanParamModule);

	function WanParamModule() {
		this.init = function () {
			this.initEvent();
		}
		this.initEvent = function () {
			$("#macClone").on("change", changeMacCloneType);
		};
		this.initValue = function () {
			var obj = pageModule.data;
			this.initHtml(obj.wan);
			setWanValue(obj.wan);
			routerMac = obj.mac.macRouter;
			hostMac = obj.mac.macHost;

			inputValue(obj.mac);
			changeMacCloneType();
		}

		this.getSubmitData = function () {
			var wanMac = "";
			if ($("#macClone").val() == "clone") {
				wanMac = hostMac;
			} else if ($("#macClone").val() == "default") {
				wanMac = routerMac;
			} else {
				wanMac = $("#macCurrentWan").val();
			}
			var data = {
				wanServerName: $("#wanServerName")[0].val(),
				wanServiceName: $("#wanServiceName")[0].val(),
				wanMTU: $("#wanMTU")[0].val(),
				macClone: $("#macClone").val(),
				wanMAC: wanMac,
				wanSpeed: $("#wanSpeed").val()
			};
			return objToString(data);
		};
		this.initHtml = function (obj) {
			var serverNameObj = {
				"initVal": obj.wanServerName,
				"editable": "1",
				"size": "small",
				"options": [{
					"default": _("Default"),
					".divider": ".divider",
					".hand-set": _("Manual")
				}]
			};
			if (obj.wanType == "pppoe") {
				$("#serverNameWrap").show();
				$("#wanMTU").attr("data-options", '{"type":"num", "args":[576, 1492]}');
			} else {
				$("#serverNameWrap").hide();
				$("#wanMTU").attr("data-options", '{"type":"num", "args":[576, 1500]}');
			}
			$("#wanServerName").toSelect(serverNameObj);
			serverNameObj.initVal = obj.wanServiceName;
			$("#wanServiceName").toSelect(serverNameObj);

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

		function changeMacCloneType() {
			var macCloneType = $("#macClone").val();
			if (macCloneType == "clone") {
				$("#macCurrenWrap").html(_("Local Host's MAC: %s", [hostMac]));
				$("#macCurrentWan").hide();
				$("#macCurrenWrap").show();
			} else if (macCloneType == "default") {
				$("#macCurrenWrap").html(_("Factory MAC: %s", [routerMac]));
				$("#macCurrentWan").hide();
				$("#macCurrenWrap").show();
			} else {
				$("#macCurrentWan").show();
				$("#macCurrenWrap").hide();
			}
			top.mainLogic.initModuleHeight();
		}

		var hostMac,
			routerMac;

		function setWanValue(obj) {
			$("#wanSpeed").val(obj.wanSpeed);
			$("#wanMTUCurrent").html(obj.wanMTUCurrent);
			$("#wanSpeedCurrent").html($("#wanSpeed").find("option[value='" + obj.wanSpeedCurrent + "']").html());
		}
	}
	/***********END WAN Parameters***************************/

	/***********LAN Parameters*********************/
	var lanModule = new LanModule();
	pageModule.modules.push(lanModule);

	function LanModule() {
		this.init = function () {
			this.initEvent();
		};
		this.initEvent = function () {
			$("#dhcpEn").on("click", changeDhcpEn);
		}
		this.initValue = function () {
			var obj = pageModule.data;
			inputValue(obj.lan);
			changeDhcpEn();
		}
		this.checkData = function () {
			var lanIP = $("#lanIP").val(),
				lanMask = $("#lanMask").val(),
				startIP = $("#lanDhcpStartIP").val(),
				endIP = $("#lanDhcpEndIP").val();
			var msg = checkIsVoildIpMask(lanIP, lanMask, _("LAN IP"));
			if (msg) {
				$("#lanIP").focus();
				return msg;
			}
			if ($("#dhcpEn")[0].checked) {
				if (!checkIpInSameSegment(startIP, lanMask, lanIP, lanMask)) {
					$("#lanDhcpStartIP").focus();
					return _("%s and %s must be in the same network segment.", [_("Start IP"), _("LAN IP")]);
				}

				if (!checkIpInSameSegment(endIP, lanMask, lanIP, lanMask)) {
					$("#lanDhcpEndIP").focus();
					return _("%s and %s must be in the same network segment.", [_("End IP"), _("LAN IP")]);
				}
				var sipArry = startIP.split("."),
					eipArry = endIP.split("."),
					sipNumber,
					eipNumber;
				sipNumber = parseInt(sipArry[0], 10) * 256 * 256 * 256 + parseInt(sipArry[1], 10) * 256 * 256 + parseInt(sipArry[2], 10) * 256 + parseInt(sipArry[3], 10);
				eipNumber = parseInt(eipArry[0], 10) * 256 * 256 * 256 + parseInt(eipArry[1], 10) * 256 * 256 + parseInt(eipArry[2], 10) * 256 + parseInt(eipArry[3], 10);
				if (sipNumber > eipNumber) {
					$("#lanDhcpEndIP").focus();
					return _("The start IP can't be greater than the end IP.");
				}
				if ($("#lanDns1").val() == $("#lanDns2").val()) {
					return _("Preferred DNS server and Alternative DNS server can't be the same.");
				}
			}
			return;
		}
		this.getSubmitData = function () {
			var data = {
				lanIP: $("#lanIP").val(),
				lanMask: $("#lanMask").val(),
				dhcpEn: $("#dhcpEn")[0].checked == true ? "true" : "false",
				lanDhcpStartIP: $("#lanDhcpStartIP").val(),
				lanDhcpEndIP: $("#lanDhcpEndIP").val(),
				lanDhcpLeaseTime: $("#lanDhcpLeaseTime").val(),
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
		this.init = function () {
			this.initEvent();
		};
		this.initEvent = function () {
			$("#remoteWebEn").on("click", changeRemoteEn);
			$("#remoteWebType").on("change", changeRemoteWebType);
		}
		this.initValue = function () {
			var obj = pageModule.data;
			inputValue(obj.remoteWeb);
			changeRemoteEn();
			changeRemoteWebType();
		}
		this.getSubmitData = function () {
			var data = {
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
	var timeModule = new TimeModule();
	pageModule.modules.push(timeModule);

	function TimeModule() {
		this.initValue = function () {
			var obj = pageModule.data;
			inputValue(obj.sysTime);
			if (obj.sysTime.internetStatus == "true") {
				$("#internetTips").show();
			} else {
				$("#internetTips").hide();
			}
		}
		this.getSubmitData = function () {
			var data = {
				sysTimeZone: $("#sysTimeZone").val()
			};
			return objToString(data);
		}
	}
	/***********END Date & Time**************/

	/***********Device Management**********/
	var manageModule = new ManageModule();
	pageModule.modules.push(manageModule);

	function ManageModule() {
		this.init = function () {
			this.initEvent();

		}
		this.initEvent = function () {

			$("#reboot").on("click", function () {
				if (confirm(_("Reboot the device?"))) {
					$.post("goform/sysReboot", "action=reboot", function (str) {
						if (checkIsTimeOut(str)) {
							top.location.reload(true);
							return;
						}
						var num = $.parseJSON(str).errCode;
						if (num == 100) {
							progressLogic.init("", "reboot", 200);
						}
					})
				}
			});

			$("#restore").on("click", function () {
				if (confirm(_("Resetting to factory default will clear all settings of the router."))) {
					$.post("goform/sysRestore", "action=restore", function (str) {
						if (checkIsTimeOut(str)) {
							top.location.reload(true);
							return;
						}
						var num = $.parseJSON(str).errCode;
						if (num == 100) {
							progressLogic.init(_("Resetting...Please wait..."), "restore", 200, "192.168.0.1");
						}
					})
				}
			});

			$("#export").on("click", function () {
				window.location = "/cgi-bin/DownloadSyslog/RouterSystem.log?" + Math.random();
			});

			$("#upgradeFile").on("change", goUpgrade);
		}
		this.initValue = function () {

		}
		this.checkData = function () {
			return;
		}
		this.getSubmitData = function () {
			pageModule.rebootIP = $("#lanIP").val();
			var data = {
				firmwareAutoMaintenanceEn: $("#firmwareAutoMaintenanceEn")[0].checked == true ? "true" : "false"
			};
			return objToString(data);
		};
		var uploadUnit = (function () {
			var uploadFlag = false;
			return {
				uploadFile: function (id, url, callback) {
					url = url || './cgi-bin/upgrade';
					if (uploadFlag) {
						return;
					}
					uploadFlag = true;
					$.ajaxFileUpload({
						url: url,
						secureuri: false,
						fileElementId: id,
						dataType: 'text',
						success: function () {
							uploadFlag = false;
							if (typeof callback === "function") {
								callback.apply(this, arguments);
							}
						}
					});
				}
			};
		}());

		function goUpgrade() {

			var upgradefile = document.getElementById('upgradeFile').value,
				upform = document.upgradefrm;

			if (upgradefile == null || upgradefile == "") {
				return;

				// 判断文件类型
			} //else {//if (!isFileType(upgradefile, ['bin', 'trx'])) {
			// alert('请选择文件名以 “trx” 或 “bin”结尾的文件');
			//  upform.reset();
			//   return;
			//  }

			if (confirm(_('Upgrade the device?'))) {
				uploadUnit.uploadFile("upgradeFile", "", function (msg) {
					if (checkIsTimeOut(msg)) {
						top.location.reload(true);
						return;
					}

					var num = $.parseJSON(msg).errCode;
					if (num == "100") {
						parent.progressLogic.init("", "upgrade");
					} else if (num == "201") {
						mainLogic.showModuleMsg(_("Firmware error!"));
					} else if (num == "201") {
						mainLogic.showModuleMsg(_("Upgrade failed!"));
					} else if (num == "202") {
						mainLogic.showModuleMsg(_("Firmware size is too large!"));
					}
				})

			} else {

			}
		}
	}
	/***********END Device Management**********/
})