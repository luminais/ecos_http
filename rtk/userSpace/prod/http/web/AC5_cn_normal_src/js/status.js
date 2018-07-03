define(function (require, exports, module) {

	/***************页面信息初始化**************/
	var pageModule = new PageLogic({
		getUrl: "goform/getStatus",
		modules: "internetStatus,deviceStatistics,systemInfo,wanAdvCfg,wifiRelay,wifiBasicCfg,sysTime"
	});

	pageModule.modules = [];

	var connectStatusCode = {
		//0代表未联网, 
		//1代表已联网; 
		//2代表连接失败；
		//3代表连接中; 
		//4代表连接成功

		//STATIC
		"0101": 0, //值为0时 表示未联网
		"0102": 0,
		"0103": 0,
		"0104": 0,
		"0105": 0,
		"0106": 1, //值为1时表示已经联网

		//DHCP
		"0201": 0,
		"0202": 0,
		"0203": 0,
		"0204": 0,
		"0205": 0,
		"0206": 1,
		"0207": 0,
		"0208": 0,

		//PPPoE
		"0301": 0,
		"0302": 0,
		"0303": 0,
		"0304": 0,
		"0305": 0,
		"0306": 1, //已联网
		"0307": 0,
		"0308": 0,

		/************WISP**************/
		//STATIC 
		"1102": 2, //值为3时表示连接失败
		"1103": 3, //值为4时表示连接中
		"1104": 4, //值为5时表示连接成功
		"1105": 4,
		"1106": 4,
		"1107": 2, //无线密码错误
		//end

		//DHCP 
		"1202": 2,
		"1203": 3, //正确的应该是连接中
		"1204": 4,
		"1205": 4,
		"1206": 4,
		"1207": 4,
		"1208": 4,
		"1209": 2, //无线密码错误
		//end

		//PPPoE 
		"1302": 2,
		"1303": 4,
		"1304": 4,
		"1305": 4,
		"1306": 4,
		"1307": 4,
		"1308": 4,
		"1309": 2, //无线密码错误

		/************APClinet**************/
		"2102": 2,
		"2103": 3, //正确的应该是连接中
		"2104": 4,
		"2202": 2,
		"2203": 3, //正确的应该是连接中
		"2204": 4,
		"2302": 2,
		"2303": 3, //正确的应该是连接中
		"2304": 4,
		"2107": 2, //无线密码错误
		"2209": 2, //无线密码错误
		"2309": 2, //无线密码错误
		/************AP**************/
		"4102": 2,
		"4104": 4,
		"4202": 2,
		"4204": 4,
		"4302": 2,
		"4304": 4
	};
	var statusTimer = null;
	pageModule.initEvent = function () {

		pageModule.pageRunning = true;
		clearInterval(statusTimer);
		statusTimer = setInterval(function () {
			pageModule.getValue("goform/getStatus", "internetStatus,deviceStatistics,systemInfo,wanAdvCfg,wifiRelay,wifiBasicCfg,sysTime");
		}, 3000);

		if (!pageModule.pageRunning) {
			clearInterval(statusTimer);
			return;
		}
	}
	module.exports = pageModule;

	/*
	 *
	 * @method statusSysModule [显示联网状态模块的数据]
	 * @return {无}
	 */
	var statusSysModule = new StatusSysModule();
	pageModule.modules.push(statusSysModule);

	function StatusSysModule() {

		this.moduleName = "internetStatus";

		this.init = function () {
			this.initEvent();
		};
		this.initEvent = function () {
			$("#statusInternet").delegate("#cloneMac", "click", cloneMAC);
		};
		this.initValue = function (obj) {

			updateInternetConnectStatus(pageModule.data.internetStatus, "wan-connect-status");

			var wifiRelayObj = pageModule.data.wifiRelay;
			var internetStatuObj = pageModule.data.internetStatus,
				conStr, disconStr;

			$("#wifiRelaySSID, #devices, #wifiSSID").html("");

			switch (wifiRelayObj.wifiRelayType) {
			case "disabled":
				conPic = "connect-ui bottom-line";

				$("#internetPicWrapper #internetPic").attr("class", "picture pic-net");
				$("#superDeviceDesc").html(_("Internet"));
				$("#statistic").removeClass("none");
				break;

			case "wisp":
				conPic = "connect-ui picture pic-wifiExtend col-sm-offset-0 col-md-offset-1 col-lg-offset-2";
				$("#internetPicWrapper #internetPic").attr("class", "picture pic-wisp");
				$("#superDeviceDesc").html(_("WiFi"));
				$("#wifiRelaySSID").attr("title", pageModule.data.wifiRelay.wifiRelaySSID).html(_truncate(pageModule.data.wifiRelay.wifiRelaySSID));
				$("#wifiSSID").attr("title", pageModule.data.wifiBasicCfg.wifiSSID).html(_truncate(pageModule.data.wifiBasicCfg.wifiSSID));
				$("#devices").html(pageModule.data.deviceStastics.statusOnlineNumber + "<span class='hide-foreign'>" +
					_("台") + "</span>");
				$("#statistic").removeClass("none");
				break;

			case "client+ap":
				conPic = "connect-ui picture pic-wifiExtend col-sm-offset-0 col-md-offset-1 col-lg-offset-2";

				$("#internetPicWrapper #internetPic").attr("class", "picture pic-preRouter");
				$("#superDeviceDesc").html(_("Upstream Router"));
				$("#wifiRelaySSID").attr("title", pageModule.data.wifiRelay.wifiRelaySSID).html(_truncate(pageModule.data.wifiRelay.wifiRelaySSID));
				$("#wifiSSID").attr("title", pageModule.data.wifiBasicCfg.wifiSSID).html(_(pageModule.data.wifiBasicCfg.wifiSSID));
				$("#devices").html(pageModule.data.deviceStastics.statusOnlineNumber + "<span class='hide-foreign'>" +
					_("台") + "</span>");
				$("#statistic").addClass("none");

				break;
			case "ap":
				conPic = "connect-ui bottom-line";
				$("#internetPicWrapper #internetPic").attr("class", "picture pic-preRouter");
				$("#superDeviceDesc").html(_("Upstream Router"));
				$("#statistic").addClass("none");

				break;
			}

			//根据联网状态显示状态图片及信息
			var connectStatus = connectStatusCode[internetStatuObj.wanConnectStatus.slice(3, 7)];
			var tmpStr;
			switch (connectStatus) {

			case 0: //未联网
				$("#connectStatus").html(_("Disconnected")).removeClass("text-success").addClass("text-warning");
				$(".connect-ui").attr("class", "connect-ui").html("<img src='img/pic-error.png' style='width:100%;'/>");
				break;

			case 1: //已联网;
			case 3: //连接中的状态时，隐藏文字提示信息
			case 4: //连接成功

				tmpStr = _("Connection success");
				if (connectStatus == 1) {
					tmpStr = _("Connected");
				} else if (connectStatus == 3) { //连接中的状态不显示
					tmpStr = "&nbsp;";
				}

				$("#connectStatus").html(tmpStr).addClass("text-success").removeClass("text-warning");
				$(".connect-ui").attr("class", conPic).html("");
				break;

			case 2: //连接失败；
				$("#connectStatus").html(_("Connection failure")).removeClass("text-success").addClass("text-warning");

				//AP模式下，失败的图片为横线；
				if (wifiRelayObj.wifiRelayType == "ap") {
					$(".connect-ui").attr("class", "connect-ui").html("<img src='img/pic-error.png' style='width:100%;'/>");
				} else {
					$(".connect-ui").attr("class", "connect-ui").html("<img src='img/connect-error.png' class='connect-error' style='heigth:30px;'/>");
				}
				break;
			};
		}

		/**
		 * @method [字符过长时截短显示]
		 * @return {String} [截短后的字符]
		 */
		function _truncate(str) {
			var len = str.length;
			if (len > 12) {
				return str.slice(0, 12) + "...";
			}
			return str;
		}

		function cloneMAC() {
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
	}
	/***************END Internet Connection Status*******************/


	/*
	 * 显示带宽，用户数等信息
	 * @method deviceStastics
	 * @param {Object} deviceStastics 从后台获取的关于联网状态的数据
	 * @return {无}
	 */
	var deviceStastics = new DeviceStastics();
	pageModule.modules.push(deviceStastics);

	function DeviceStastics() {
		this.moduleName = "deviceStastics";
		this.init = function () {
			this.initEvent();
		};
		this.initEvent = function () {
			$("#statistic fieldset").on("click", function () {
				mainLogic.changeMenu("net-control"); 
			});
		};
		this.initValue = function (obj) {

			$("#statusOnlineNumber").html(obj.statusOnlineNumber);
			shapingSpeed("statusDownSpeed", obj.statusDownSpeed);
			shapingSpeed("statusUpSpeed", obj.statusUpSpeed);

			//路由及wisp模式下，如果获取到IP地址，则显示在线设备及实时网速
			//client+ap 及ap模式下，隐藏在线设备及实时网速
			$("#statistic").addClass("none");
			var hasIP = pageModule.data.internetStatus.wanConnectStatus.slice(2, 3) == "1" ? true : false;
			switch (pageModule.data.wifiRelay.wifiRelayType) {
			case "disabled":
			case "wisp":
				if (hasIP) {
					$("#statistic").removeClass("none");
				}
				break;
			}
		};

		function shapingSpeed(id, value) {
			var val = parseFloat(value);

			if (val > 1024) {
				$("#" + id).html((val / 1024).toFixed(1));
				$("#" + id + "~small").html("MB/s");
			} else {
				$("#" + id).html(val.toFixed(1));
				$("#" + id + "~small").html("KB/s");
			}
		}
	}

	/***************END Attached Devices and Real-time Statistics************/


	/*
	 *
	 * @method systemInfo [显示系统的信息]
	 * @return {无}
	 */
	var systemInfo = new SystemInfo();
	pageModule.modules.push(systemInfo);

	function SystemInfo() {
		this.moduleName = "systemInfo";

		this.initValue = function (systemInfoObj) {
			var obj = $.extend({}, systemInfoObj, pageModule.data.sysTime);
			switch (obj.wanType) {
			case "dhcp":
				obj.wanType = _("Dynamic IP Address");
				break;
			case "pppoe":
				obj.wanType = _("PPPoE");
				break;
			default:
				obj.wanType = _("Static IP Address");
			}

			//第三位数为1表示获取到了IP地址；
			var hasIP = pageModule.data.internetStatus.wanConnectStatus.slice(2, 3) == "1" ? true : false;
			$("#statusWAN, #wanBasic, #conType, #wanConnectTime-wrapper, #sysTimecurrentTime-wrapper").addClass("none");
			switch (pageModule.data.wifiRelay.wifiRelayType) {
				//路由模式及wisp模式下，如果获取到IP地址,则显示系统信息；
			case "disabled":
			case "wisp":
				$("#wanBasic, #conType, #wanConnectTime-wrapper").removeClass("none");
				if (hasIP) {
					$("#statusWAN").removeClass("none");
				}
				break;

				//client+ap及ap模式下，如果获取到IP地址&连接成功，则显示系统信息
			case "client+ap":
			case "ap":
				$("#sysTimecurrentTime-wrapper").removeClass("none");
				if (hasIP && connectStatusCode[pageModule.data.internetStatus.wanConnectStatus.slice(3, 7)] == 4) {
					$("#statusWAN").removeClass("none");
				}
				break;
			}
			//end

			for (var prop in obj) {
				if (obj[prop] == "") {
					obj[prop] = "-";
				}
			}
			inputValue(obj);

			$("#wanConnectTime").html(formatSeconds(obj.wanConnectTime));
		};
	}
	/***************END System Info***************/

});