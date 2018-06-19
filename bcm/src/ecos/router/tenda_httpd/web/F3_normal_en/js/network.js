define(function (require, exports, module) {
	var pageModule = new PageLogic("goform/getWAN", "goform/setWAN");
	pageModule.modules = [];
	module.exports = pageModule;

	/************WAN Setting*************/
	var wanModule = new WanModule();
	pageModule.modules.push(wanModule);

	function WanModule() {
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
		};

		this.initValue = function () {
			var obj = pageModule.data;
			this.rebootIP = obj.lanIP;
			inputValue(obj);
			if (!this.addInputEvent) {
				$("#wanPPPoEUser").addPlaceholder(_("User Name from ISP"));
				$("#wanPPPoEPwd").initPassword(_("Password from ISP"));
				this.addInputEvent = true;
			}
			this.changeWanType();

			showWanInternetStatus(obj.wanConnectStatus, "wanConnectStatus");
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
			top.mainLogic.initModuleHeight();
		};

		this.cloneMAC = function () {
			var macHost = pageModule.data.macHost;
			$.post("goform/setMacClone", "wanMac=" + macHost, function (str) {
				if (checkIsTimeOut(str)) {
					top.location.reload(true);
					return;
				}
				var num = $.parseJSON(str).errCode || "-1";
				if (num == 0) {
					mainLogic.showModuleMsg(_("Clone MAC successfully!"));
				}
			})
		}
		this.checkData = function () {
			var wanType = $("input[name='wanType']:checked").val(),
				//pppoeUser = $("#pppoeUser").val(),
				//pppoePwd = $("#pppoePwd").val(),
				ip = $("#wanIP")[0].val(),
				mask = $("#wanMask")[0].val(),
				gateway = $("#wanGateway")[0].val(),
				dns1 = $("#wanDns1")[0].val(),
				dns2 = $("#wanDns2")[0].val(),
				msg = "";
			var lanIp = pageModule.data.lanIP,
				lanMask = pageModule.data.lanMask;
			if (wanType == "pppoe") {

			} else if (wanType == "static") {
				msg = checkIsVoildIpMask(ip, mask, _("IP Address"));
				if (msg) {
					return msg;
				}
				if (checkIpInSameSegment(ip, mask, lanIp, lanMask)) {
					return _("%s and %s (%s) should not be in the same network segment.", [_("WAN IP"), _("LAN IP"), lanIp]);
				}
				if (!checkIpInSameSegment(ip, mask, gateway, mask)) {
					return _("%s and %s must be in the same network segment.", [_("WAN IP"), _("Gateway")]);
				}

				if (ip == gateway) {
					return _("WAN IP and Default Gateway can't be the same.");;
				}

				if (dns1 == dns2) {
					return _("Preferred DNS server and Alternative DNS server can't be the same.");
				}
			}
			return;
		};

		this.getSubmitData = function () { //获取提交数据
			var data = {
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