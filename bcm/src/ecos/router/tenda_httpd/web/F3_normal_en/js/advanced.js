define(function (require, exports, module) {

	var pageModule = new PageLogic("goform/getNAT", "goform/setNAT");
	pageModule.modules = [];
	module.exports = pageModule;
	/**********Port Forwarding *******************/
	var portMap = new PortMapModule();
	pageModule.modules.push(portMap);

	function PortMapModule() {
		this.init = function () {
			this.initHtml();
			this.initEvent();
		};
		this.initHtml = function () {
			var selectObj = {
				"initVal": "21",
				"editable": "1",
				"size": "small",
				"options": [{
					"21": "21 (FTP)",
					"23": "23 (TELNET)",
					"25": "25 (SMTP)",
					"53": "53 (DNS)",
					"80": "80 (HTTP)",
					"3389": _("3389 (Remote Desktop)"),
					"9000": "9000",
					".divider": ".divider",
					".hand-set": _("Manual")
				}]
			};
			$("#internalPort").toSelect(selectObj);
			var selectObj1 = {
				"initVal": "All",
				"editable": "1",
				"size": "small",
				"options": [{
					"All": "All",
					".divider": ".divider",
					".hand-set": _("Manual")
				}]
			};
			$("#externalIP").toSelect(selectObj1);
		}
		this.initEvent = function () {
			$("#portHead").delegate(".icon-plus-circled", "click", addPortList);
			$("#portTbody").delegate(".icon-minus-circled", "click", delPortList);

			/******添加事件，只能输入数字******/
			$("#internalPort .input-box, #externalPort").on("keyup blur", function () {
				this.value = (parseInt(this.value, 10) || "")
			});

			/*******添加事件，选择下拉框时，将外网端口与内网端口相同*********/
			$("#internalPort.input-append ul").on("click", function (e) {
				$("#externalPort")[0].value = ($(this).parents(".input-append").find("input")[0].value || "");
			});
		};
		this.initValue = function () {
			//inputValue(obj);
			createPortTable(pageModule.data.portList);
		};
		this.checkData = function () {
			return;
		};

		this.getSubmitData = function () {
			var data = {
				portList: getPortListValue()
			};
			return objToString(data);
		}

		function createPortTable(arry) {
			var listArry = arry,
				len = listArry.length,
				i = 0,
				str = "",
				outIp;
			$("#portTbody").html("");
			for (i = 0; i < len; i++) {
				outIp = listArry[i].portListExtranetIP;
				if (outIp == "0.0.0.0") {
					outIp = _("All");
				}
				str += "<tr>";
				str += "<td>" + listArry[i].portListIntranetIP + "</td>";
				str += "<td>" + listArry[i].portListIntranetPort + "</td>";
				str += "<td>" + outIp + "</td>";
				str += "<td>" + listArry[i].portListExtranetPort + "</td>";
				str += "<td>" + $("#protocol [value='" + listArry[i].portListProtocol + "']").html() + "</td>";
				str += "<td><div class='ico icon-minus-circled text-primary'></div></td>";
				str += "</tr>";
			}
			$("#portTbody").append(str);
		}

		/*******检查添加时的数据合法性*******/
		function checkAddListValidate() {
			var inIp = $("#internalIP").val(),
				inPort = $("#internalPort")[0].val(),
				outIp = $("#externalIP")[0].val(),
				outPort = $("#externalPort").val(),
				lanIP = pageModule.data.lanIP,
				lanMask = pageModule.data.lanMask;
			var i = 0,
				portArry = $("#portTbody").children(),
				length = portArry.length;

			//判断IP地址合法性
			if (!(/^([1-9]|[1-9]\d|1\d\d|2[0-1]\d|22[0-3])\.(([0-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.){2}([0-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])$/).test(inIp)) {
				$("#internalIP").focus();
				return _("Please input a valid IP address.");
			}

			if (!checkIpInSameSegment(inIp, lanMask, lanIP, lanMask)) {
				$("#internalIP").focus();
				return _("%s and %s must be in the same network segment.", [_("Internal IP"), _("LAN IP")]);
			}

			var msg = checkIsVoildIpMask(inIp, lanMask, _("Internal IP"));
			if (msg) {
				$("#internalIP").focus();
				return msg;
			}

			if (inIp == lanIP) {
				return (_("Internal IP should not be the same with the login IP(%s)", [lanIP]));
			}

			if (inPort == "" || parseInt(inPort, 10) > 65535 || parseInt(inPort, 10) < 1) {
				$("#internalPort").find(".input-box").focus();
				return (_("Internal Port Range: 1-65535"));

			}

			//判断IP地址合法性
			if (outIp == "All") {

			} else {
				if (!(/^([1-9]|[1-9]\d|1\d\d|2[0-1]\d|22[0-3])\.(([0-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.){2}([0-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])$/).test(outIp)) {
					$("#externalIP").find(".input-box").focus();
					return _("Please input a valid IP address.");
				}
			}

			if (outPort == "" || parseInt(outPort, 10) > 65535 || parseInt(outPort, 10) < 1) {
				$("#externalPort").focus();
				return (_("External Port Range: 1-65535"));
			}

			for (i = 0; i < length; i++) {
				if (portArry.eq(i).children().eq(2).html() == outPort) {
					return _("External port repeat! One external port can only be used for one mapping.")
				}
			}

			if ($("#portTbody").children().length >= 16) {
				return (_("Up to %s entries can be added.", [16]));

			}

			return;
		}

		function addPortList() {
			var str = "";
			var inIp = $("#internalIP").val(),
				inPort = $("#internalPort")[0].val(),
				outIp = $("#externalIP")[0].val(),
				outPort = $("#externalPort").val(),
				protocol = $("#protocol").val();
			var msg = checkAddListValidate();
			if (msg) {
				mainLogic.showModuleMsg(msg);
				return;
			}
			str += "<tr>";
			str += "<td>" + inIp + "</td>";
			str += "<td>" + inPort + "</td>";
			str += "<td>" + outIp + "</td>";
			str += "<td>" + outPort + "</td>";
			str += "<td>" + $("#protocol option:selected").html() + "</td>";
			str += "<td><div class='ico icon-minus-circled text-primary'></div></td>";
			str += "</tr>";
			$("#portTbody").append(str);
			top.mainLogic.initModuleHeight();
		}

		/******获取table表格提交的字符串*********/
		function getPortListValue() {
			var str = "",
				i = 0,
				$portArry = $("#portTbody").children(),
				length = $portArry.length;

			for (i = 0; i < length; i++) {
				str += $portArry.eq(i).children().eq(0).html() + ";";
				str += $portArry.eq(i).children().eq(1).html() + ";";
				str += $portArry.eq(i).children().eq(2).html() + ";";
				str += $portArry.eq(i).children().eq(3).html() + ";";
				str += $portArry.eq(i).children().eq(4).html();
				str += "~";
			}
			str = str.replace(/[~]$/, "");
			return str;
		}

		function delPortList() {
			$(this).parent().parent().remove();
			top.mainLogic.initModuleHeight();
		}
	}

	/********END  Port Forwarding******************/

	/**********static IP**************/
	var staticModule = new StaticModule();
	pageModule.modules.push(staticModule);

	function StaticModule() {
		this.init = function () {
			this.initEvent();
		};
		this.initEvent = function () {
			$("#staticHead").delegate(".icon-plus-circled", "click", addStaticList);
			$("#staticTbody").delegate(".icon-minus-circled", "click", delStaticList)
		};
		this.initValue = function () {
			createStaticIpList(pageModule.data.staticList);
		}
		this.checkData = function () {
			return;
		};
		this.getSubmitData = function () {
			var data = {
				staticList: getStaticValue()
			}
			return objToString(data);
		};

		function createStaticIpList(arry) {
			var i = 0,
				len = arry.length;
			$("#staticTbody").html("");
			for (i = 0; i < len; i++) {
				listStr = "";
				listStr += "<tr>";
				listStr += '<td class="span-fixed">' + arry[i].staticIP + "</td>";
				listStr += '<td class="span-fixed">' + arry[i].staticMac + "</td>";
				listStr += '<td class="span-fixed remark"></td>';
				listStr += "<td><div class='ico icon-minus-circled text-primary'></div></td>";
				listStr += "</tr>";
				$("#staticTbody").append(listStr);
				$("#staticTbody").find(".remark").text(arry[i].staticRemark);
				$("#staticTbody").find(".remark").removeClass("remark");
			}
		}

		function checkAddStaticValidate() {
			var staticIp = $("#staticIp").val(),
				mac = $("#staticMac").val(),
				startIP = pageModule.data.lanStartIP,
				endIP = pageModule.data.lanEndIP,
				lanIP = pageModule.data.lanIP,
				lanMask = pageModule.data.lanMask;

			if (!(/^([1-9]|[1-9]\d|1\d\d|2[0-1]\d|22[0-3])\.(([0-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.){2}([0-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])$/).test(staticIp)) {
				$("#staticIp").focus();
				return _("Please input a valid IP address.");
			}
			if (!checkIpInSameSegment(staticIp, lanMask, lanIP, lanMask)) {
				$("#staticIp").focus();
				return _("%s and %s must be in the same network segment.", [_("Static IP"), _("LAN IP")]);
			}

			var ipArry = staticIp.split("."),
				sipArry = startIP.split("."),
				eipArry = endIP.split("."),
				ipNumber,
				sipNumber,
				eipNumber;
			ipNumber = parseInt(ipArry[0], 10) * 256 * 256 * 256 + parseInt(ipArry[1], 10) * 256 * 256 + parseInt(ipArry[2], 10) * 256 + parseInt(ipArry[3], 10);
			sipNumber = parseInt(sipArry[0], 10) * 256 * 256 * 256 + parseInt(sipArry[1], 10) * 256 * 256 + parseInt(sipArry[2], 10) * 256 + parseInt(sipArry[3], 10);
			eipNumber = parseInt(eipArry[0], 10) * 256 * 256 * 256 + parseInt(eipArry[1], 10) * 256 * 256 + parseInt(eipArry[2], 10) * 256 + parseInt(eipArry[3], 10);
			if (ipNumber < sipNumber || ipNumber > eipNumber) {
				$("#staticIp").focus();
				return _("IP address must be included in the address pool of DHCP");
			}

			if (!(/^([0-9a-fA-F]{2}:){5}[0-9a-fA-F]{2}$/).test(mac)) {
				$("#staticMac").focus();
				return _("Please input a valid MAC address.");
			}

			var subMac1 = mac.split(':')[0];

			if (subMac1.charAt(1) && parseInt(subMac1.charAt(1), 16) % 2 !== 0) {
				$("#staticMac").focus();
				return _('The second character must be even number.');
			}
			if (mac === "00:00:00:00:00:00") {
				$("#staticMac").focus();
				return _('MAC can not be 00:00:00:00:00:00.');
			}

			var $listArry = $("#staticTbody").children(),
				len = $listArry.length,
				i = 0,
				listMac,
				listIp;
			for (i = 0; i < len; i++) {
				listIp = $listArry.eq(i).children().eq(0).html();
				listMac = $listArry.eq(i).children().eq(1).html();
				if (staticIp == listIp) {
					$("#staticIp").focus();
					return _("This IP address is used. Please try another.");
				}
				if (listMac == mac) {
					$("#staticMac").focus();
					return _("This MAC address is used. Please try another.");
				}
			}
		}

		function delStaticList() {
			$(this).parent().parent().remove();
			top.mainLogic.initModuleHeight();
		}

		function addStaticList() {
			var msg = checkAddStaticValidate(),
				str;
			if (msg) {
				mainLogic.showModuleMsg(msg);
				return;
			}
			str = "<tr>";
			str += '<td class="span-fixed">' + $("#staticIp").val() + "</td>";
			str += '<td class="span-fixed">' + $("#staticMac").val() + "</td>";
			str += '<td class="span-fixed">' + $("#staticRemark").val() + "</td>";
			str += "<td><div class='ico icon-minus-circled text-primary'></div></td>";
			str += "</tr>";
			$("#staticTbody").append(str);
		}

		function getStaticValue() {
			var str = "",
				i = 0,
				$staticArry = $("#staticTbody").children(),
				length = $staticArry.length;

			for (i = 0; i < length; i++) {
				str += $staticArry.eq(i).children().eq(0).html() + "\t";
				str += $staticArry.eq(i).children().eq(1).html() + "\t";
				str += $staticArry.eq(i).children().eq(2).text() + "\t";
				str += "\n";
			}
			str = str.replace(/[\n]$/, "");
			return str;
		}
	}

	/**********END static IP***********/

	/*********dmz*********/
	var dmzModule = new DmzModule();
	pageModule.modules.push(dmzModule);

	function DmzModule() {
		this.init = function () {
			this.initEvent();
		};
		this.initEvent = function () {
			$("input[name='dmzEn']").on("click", changeDmzEn);
		};
		this.initValue = function () {
			$("input[name='dmzEn'][value='" + pageModule.data.dmz.dmzEn + "']")[0].checked = true;
			$("#dmzHostIP").val(pageModule.data.dmz.dmzHostIP || "");

			changeDmzEn();
		};
		this.checkData = function () {
			var dmzIP = $("#dmzHostIP").val(),
				lanIP = pageModule.data.lanIP,
				lanMask = pageModule.data.lanMask;
			if ($("input[name='dmzEn']")[0].checked) {
				//判断IP地址合法性
				if (!checkIpInSameSegment(dmzIP, lanMask, lanIP, lanMask)) {
					$("#dmzHostIP").focus();
					return _("%s and %s must be in the same network segment.", [_("Host IP"), _("LAN IP")]);
				}

				var msg = checkIsVoildIpMask(dmzIP, lanMask, _("Host IP"));
				if (msg) {
					$("#dmzHostIP").focus();
					return msg;
				}

				if (dmzIP == lanIP) {
					return _("DMZ host IP should not be the same with the login IP(%s)", [lanIP]);
				}
			}
			return;
		};
		this.getSubmitData = function () {
			var data = {
				dmzEn: $("input[name='dmzEn']:checked").val(),
				dmzHostIP: $("#dmzHostIP").val()
			}
			return objToString(data);
		};

		function changeDmzEn() {
			var dmzEn = $("input[name='dmzEn']:checked").val();
			if (dmzEn == "true") {
				$("#dmzWrap").removeClass("none");
			} else {
				$("#dmzWrap").addClass("none");
			}
			top.mainLogic.initModuleHeight();
		}
		var selectObj1 = {
			"initVal": "All",
			"editable": "1",
			"size": "small",
			"options": [{
				"All": "All",
				".divider": ".divider",
				".hand-set": _("Manual")
			}]
		};
		$("#externalIP").toSelect(selectObj1);

	}

	/*********END dmz*****/

	/******ddns *****/
	var ddnsModule = new DdnsModule();
	pageModule.modules.push(ddnsModule);

	function DdnsModule() {
		this.init = function () {
			this.initEvent();
		};
		this.initEvent = function () {
			$("input[name='ddnsEn']").on("click", changeDdnsEn);
			$("#register").on("click", function () {
				var url = $("#ddnsServiceName").val();
				window.open("http://" + url);
			});
		};
		this.initValue = function () {
			inputValue(pageModule.data.ddns);
			changeDdnsEn();
		};
		this.checkData = function () {
			return;
		};
		this.getSubmitData = function () {
			var data = {
				ddnsEn: $("input[name='ddnsEn']:checked").val(),
				ddnsServiceName: $("#ddnsServiceName").val(),
				ddnsServer: $("#ddnsServer").val(),
				ddnsUser: $("#ddnsUser").val(),
				ddnsPwd: $("#ddnsPwd").val()
			}
			return objToString(data);
		};

		function changeDdnsEn() {
			var ddnsEn = $("input[name='ddnsEn']:checked").val();
			if (ddnsEn == "true") {
				$("#ddnsWrap").removeClass("none");
			} else {
				$("#ddnsWrap").addClass("none");
			}
			top.mainLogic.initModuleHeight();
		}
	}

	/******END ddns*******/

	/*********upnp**********/
	var upnpModule = new UpnpModule();
	pageModule.modules.push(upnpModule);

	function UpnpModule() {
		this.init = function () {
			this.initEvent();
		};
		this.initEvent = function () {

		};
		this.initValue = function () {
			inputValue(pageModule.data);
		};
		this.checkData = function () {
			return;
		}
		this.getSubmitData = function () {
			var data = {
				upnpEn: $("input[name='upnpEn']:checked").val()
			};
			return objToString(data);
		}
	}

	/*********END upnp*******/
})