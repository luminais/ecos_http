define(function (require, exports, module) {
	var pageModule = new PageLogic({
		getUrl: "goform/getWifi",
		modules: "wifiBasicCfg,wifiAdvCfg,wifiPower,wifiWPS,wifiGuest,wifiBeamforming",
		setUrl: "goform/setWifi"
	});

	pageModule.modules = [];
	pageModule.rebootIP = location.host;
	module.exports = pageModule;

	pageModule.initEvent = function () {
		pageModule.update("wifiAdvCfg", 2000, update);
	}

	function update(obj) {

		if (obj.wifiAdvCfg.wifiChannelCurrent == "0") {
			$("#wifiBandwidthCurrent, #wifiChannelCurrent").addClass("none");
		} else {
			$("#wifiBandwidthCurrent, #wifiChannelCurrent").removeClass("none");
		}

		$("#wifiChannelCurrent").text(obj.wifiAdvCfg.wifiChannelCurrent);
		$("#wifiBandwidthCurrent").text(obj.wifiAdvCfg.wifiBandwidthCurrent == "" ? "" : (obj.wifiAdvCfg.wifiBandwidthCurrent + "MHz"));

		if (obj.wifiAdvCfg.wifiChannelCurrent_5G == "0") {
			$("#wifiBandwidthCurrent_5G, #wifiChannelCurrent_5G").addClass("none");
		} else {
			$("#wifiBandwidthCurrent_5G, #wifiChannelCurrent_5G").removeClass("none");
		}

		$("#wifiChannelCurrent_5G").text(obj.wifiAdvCfg.wifiChannelCurrent_5G);
		$("#wifiBandwidthCurrent_5G").text(obj.wifiAdvCfg.wifiBandwidthCurrent_5G == "" ? "" : (obj.wifiAdvCfg.wifiBandwidthCurrent_5G + "MHz"));

		// var type = obj.wifiTime.wifiRelayType;
		var type = obj.wifiAdvCfg.wifiRelayType;
		changeWifiRelayType(type);
	}

	function changeWifiRelayType(type) {
		$("#wifiParamWrap, #wpsWrap, #wifiGuestWrap").addClass("none");

		switch (type) {
		case "disabled":
			if(top.G_data.wpsModule.hasWPSModule == "true") {
				$("#wifiParamWrap,#wpsWrap,#wifiGuestWrap").removeClass("none");
			}else{
				$("#wifiParamWrap,#wifiGuestWrap").removeClass("none");
			}
			break;
		case "wisp":
			//edit by xc wisp模式下不支持wps功能，故隐藏该模块  
			// $("#wpsWrap").removeClass("none");
			break;
		case "client+ap":
			break;
		default: //ap
			//edit by xc 按照规格文档router模式和AP模式支持WPS功能
			if(top.G_data.wpsModule.hasWPSModule == "true") {
				$("#wifiParamWrap,#wpsWrap,#wifiGuestWrap").removeClass("none");
			}else{
				$("#wifiParamWrap,#wifiGuestWrap").removeClass("none");
			}
		}

		//AP模式下不支持访客网络 WPS
		if(top.G_data.wifiRelay.wifiRelayType == "ap"){
			$("#wifiGuestWrap").addClass("none");
			$("#wpsWrap").addClass("none");
		}
	

		top.mainLogic.initModuleHeight();
	}

	pageModule.beforeSubmit = function () {
		var wifiEn = $("[name=wifiEn]:checked").val(),
			wifiPwd = $("#wifiPwd").val(),
			pwd = pageModule.data.wifiBasicCfg.wifiPwd,
			wifiSecurityMode = $("#wifiSecurityMode").val(),

			wifiEn_5G = $("[name=wifiEn_5G]:checked").val(),
			wifiPwd_5G = $("#wifiPwd_5G").val(),
			pwd_5G = pageModule.data.wifiBasicCfg.wifiPwd_5G,
			wifiSecurityMode_5G = $("#wifiSecurityMode_5G").val(),

			remindFlag = false;

		//当无线开启时，修改无线名称或密码，需要提示用户"无线连接将断开，请重新连接"
		//2.4G
		if (wifiEn == "true" && (pageModule.data.wifiBasicCfg.wifiSSID != $("#wifiSSID").val() || (wifiSecurityMode != "none" && wifiPwd !== pwd))) {
			remindFlag = true;
		}
		
		//5G
		if (wifiEn_5G == "true" && (pageModule.data.wifiBasicCfg.wifiSSID_5G != $("#wifiSSID_5G").val() || (wifiSecurityMode_5G != "none" && wifiPwd_5G !== pwd_5G))) {
			remindFlag = true;
		}

		if(remindFlag) {
			if (!confirm(_("The wireless connection will be released. Please connect again."))) {
				return false;
			}
		}
		
		return true;
	}

	/*************Page Control*********/
	var pageModuleInit = new PageModuleInit();
	pageModule.modules.push(pageModuleInit);

	function PageModuleInit() {

		this.initValue = function () {
			// var type = pageModule.data.wifiTime.wifiRelayType;
			var type = pageModule.data.wifiAdvCfg.wifiRelayType;
			changeWifiRelayType(type);

		}
	}
	/*************END Page Control***********/

	/*
	 * 显示无线基本配置
	 * @method wifiBasicCfgModule
	 * @return {无}
	 */
	var wifiBasicCfgModule = new WifiBasicCfgModule();
	pageModule.modules.push(wifiBasicCfgModule);

	function WifiBasicCfgModule() {

		var _this = this;
		this.moduleName = "wifiBasicCfg";

		var G_Wifi_Flag_24 = true,
			G_Wifi_Flag_5 = true;

		this.init = function () {
			$("#allWifiContentWrap").hide();
			this.initEvent();
		};
		this.initEvent = function () {
			this.addInputEvent = false;
			//2.4G
			$("#wifiSecurityMode").on("change", _this.changeSecurityMode_24);
			$("#helpTips").on("mouseover", function () {
				$("#hideSSIDTips").show();
			});
			$("#helpTips").on("mouseout", function () {
				$("#hideSSIDTips").hide();
			});
			$("input[name='wifiEn']").on("click", _this.changeWifiEn_24);

			//5G
			$("#wifiSecurityMode_5G").on("change", _this.changeSecurityMode_5);
			$("#helpTips5").on("mouseover", function () {
				$("#hideSSIDTips5").show();
			});
			$("#helpTips5").on("mouseout", function () {
				$("#hideSSIDTips5").hide();
			});
			$("input[name='wifiEn_5G']").on("click", _this.changeWifiEn_5);

			//双频优选
			$("input[name='doubleBandUnityEnable']").on("click", _this.changeDoubleBand);

			//双频优选开启后的 无线开关
			$("input[name='wifiTotalEn']").on("click", _this.changeWifiTotalEn);
		};

		/**
		 * [initValue 用于初始化显示的数据]
		 * @param  {object} obj [wifiBasicCfg]
		 */
		this.initValue = function (obj) {
			//重置（点击取消时将容错提示语去掉）
			$("#wifiSSID").removeValidateTipError(true);
			$("#wifiSSID_5G").removeValidateTipError(true);

			inputValue(obj);

			if (!this.addInputEvent) {
				$("#wifiSSID").addPlaceholder(_("WiFi Name"));
				$("#wifiPwd").initPassword(_("8 or more characters"));

				$("#wifiSSID_5G").addPlaceholder(_("WiFi Name"));
				$("#wifiPwd_5G").initPassword(_("8 or more characters"));
				this.addInputEvent = true;
			}

			_this.changeWifiEn_24();
			_this.changeSecurityMode_24();

			_this.changeWifiEn_5();
			_this.changeSecurityMode_5();

			//是否支持双频优选
			if(obj.HasDoubleBandUnity == "true") {
				$("#doubleBandWrap").removeClass("none");

				if(obj.doubleBandUnityEnable == "true"){

					//双频优选开启时，显示隐藏以双频优选为最后显示
					_this.changeDoubleBand();

					//无线开关赋值
					if(obj.wifiTotalEn == "true") {
						$("input[name='wifiTotalEn'][value='true']")[0].checked = true;
					} else {
						$("input[name='wifiTotalEn'][value='false']")[0].checked = true;
					}

					_this.changeWifiTotalEn();

					$(".band-double").addClass("none");
					$(".band-double-show").removeClass("none");

					//当开启双频优选时，ssid的规格有变化，长度限制由32变成29（因为2.4G/5G公用一个ssid，而5G有一个固定后缀_5G）
					$("#wifiSSID").attr("data-options", '{"type":"ssid_quickset"}');
					$("#wifiSSID").attr("maxLength", 29);
				}else{
					$(".band-double").removeClass("none");
					$(".band-double-show").addClass("none");
					//关闭时，需判断5G开关状态
					_this.changeWifiEn_5();
					$("#wifiSSID").attr("data-options", '{"type":"ssid"}');
					$("#wifiSSID").attr("maxLength", 32);
				}
			}
		};

		this.getSubmitData = function () {
			var data = {
				module1: this.moduleName,
				doubleBandUnityEnable: $("[name='doubleBandUnityEnable']:checked").val(),
				wifiTotalEn: $("input[name='wifiTotalEn']:checked").val(),
				wifiEn: $("input[name='wifiEn']:checked").val(),
				wifiSSID: $("#wifiSSID").val(),
				wifiSecurityMode: $("#wifiSecurityMode").val(),
				wifiPwd: $("#wifiPwd").val(),
				wifiHideSSID: $("#wifiHideSSID:checked").val() || "false",

				wifiEn_5G: $("input[name='wifiEn_5G']:checked").val(),
				wifiSSID_5G: $("#wifiSSID_5G").val(),
				wifiSecurityMode_5G: $("#wifiSecurityMode_5G").val(),
				wifiPwd_5G: $("#wifiPwd_5G").val(),
				wifiHideSSID_5G: $("#wifiHideSSID_5G:checked").val() || "false"
			};

			if(pageModule.data.wifiBasicCfg.HasDoubleBandUnity == "true") {

				if($("[name='doubleBandUnityEnable']:checked").val() == "true") {

					//双频优选启用时，2.4G和5G wifi 开关同步总开关
					data.wifiEn_5G = data.wifiEn = data.wifiTotalEn; 

					//data.wifiEn_5G = data.wifiEn;
					data.wifiSSID_5G = data.wifiSSID;
					data.wifiSecurityMode_5G = data.wifiSecurityMode;
					data.wifiPwd_5G = data.wifiPwd;
					data.wifiHideSSID_5G = data.wifiHideSSID;
				}
			}else{
				delete data.doubleBandUnityEnable;
				delete data.wifiTotalEn;
			}

			return objToString(data);
		};

		/**
			无线开关	
		*/
		this.changeWifiTotalEn = function() {
			
			var wifiTotalEn = $("input[name='wifiTotalEn']:checked").val();

			if(wifiTotalEn == "true") {
				
				$(".wifiWrap_24").removeClass("none");
				$(".wifiWrap_5").removeClass("none");
				$(".band-double").addClass("none");

				G_Wifi_Flag_24 = true;
				G_Wifi_Flag_5 = true;
				_this.changeGlobalWrap(); 

				_this.handleWifiSignShow();
			} else {
				$("input[name='wifiEn'][value='"+wifiTotalEn+"']")[0].checked = true;
				$("input[name='wifiEn_5G'][value='"+wifiTotalEn+"']")[0].checked = true;
				_this.changeWifiEn_24();
				_this.changeWifiEn_5();
			}

		};

		/***********改变双频优选******/
		this.changeDoubleBand = function() {
			var doubleBandEn = $("input[name='doubleBandUnityEnable']:checked").val();

			$("input[name='wifiTotalEn']")[0].checked = true;

			$("input[name='wifiEn']")[0].checked = true;
			$(".wifiWrap_24").removeClass("none");

			$("input[name='wifiEn_5G']")[0].checked = true;
			$(".wifiWrap_5").removeClass("none");

			if(doubleBandEn == "true"){
				$(".band-double").addClass("none");
				$(".band-double-show").removeClass("none");

				//当开启双频优选时，ssid的规格有变化，长度限制由32变成29（因为2.4G/5G公用一个ssid，而5G有一个固定后缀_5G）
				$("#wifiSSID").attr("data-options", '{"type":"ssid_quickset"}');
				$("#wifiSSID").attr("maxLength", 29);
			}else{
				$(".band-double").removeClass("none");
				$(".band-double-show").addClass("none");

				$("#wifiSSID").attr("data-options", '{"type":"ssid"}');
				$("#wifiSSID").attr("maxLength", 32);

				//5G的ssid同步2.4G的,且要加上后缀_5G
				$("#wifiSSID_5G").val($("#wifiSSID").val() + "_5G");
			}

			G_Wifi_Flag_24 = true;
			G_Wifi_Flag_5 = true;
			_this.changeGlobalWrap(); 

			_this.handleWifiSignShow();
		};

		/***********改变加密模式******/
		this.changeSecurityMode_24 = function () {
			var securityMode = $("#wifiSecurityMode").val();
			if (securityMode != "NONE") {
				$("#wifiPwd").parent().parent().removeClass("none");
			} else {
				$("#wifiPwd").parent().parent().addClass("none");
			}

			if (securityMode != "WPA/AES") {
				$("#wps").show();
			} else {
				$("#wps").hide();
			}

			top.mainLogic.initModuleHeight();
		};

		this.changeSecurityMode_5 = function () {
			var securityMode = $("#wifiSecurityMode_5G").val();
			if (securityMode != "NONE") {
				$("#wifiPwd_5G").parent().parent().removeClass("none");
			} else {
				$("#wifiPwd_5G").parent().parent().addClass("none");
			}

			if (securityMode != "WPA/AES") {
				$("#wps5").show();
			} else {
				$("#wps5").hide();
			}

			top.mainLogic.initModuleHeight();
		};

		//处理信号强度模块是否显示隐藏
		this.handleWifiSignShow = function() {
			$("#wifiSignalWrap").removeClass("none");
			//2.4G和5G都不支持时 
			if($.isHidden($("#wifiSignal")[0]) && $.isHidden($("#wifiSignal5")[0])) {
				$("#wifiSignalWrap").addClass("none");
			} else { //其他情况则显示
				$("#wifiSignalWrap").removeClass("none");
			}
		};

		/***********切换无线开关******/
		this.changeWifiEn_24 = function() {
			var wifiEn = $("input[name='wifiEn']:checked").val();
			if (wifiEn == "true") {
                $(".wifiWrap_24").removeClass("none");

                G_Wifi_Flag_24 = true;
            } else {
                $(".wifiWrap_24").addClass("none");

                G_Wifi_Flag_24 = false;
            }   

            
            _this.changeGlobalWrap();        
			_this.handleWifiSignShow();
		};

		this.changeWifiEn_5 = function() {
			var wifiEn = $("input[name='wifiEn_5G']:checked").val();
			if (wifiEn == "true") {
                $(".wifiWrap_5").removeClass("none");

                G_Wifi_Flag_5 = true;
            } else {
                $(".wifiWrap_5").addClass("none");

                G_Wifi_Flag_5 = false;
            }
            
            _this.changeGlobalWrap();
			_this.handleWifiSignShow();
		};

		this.changeGlobalWrap = function() {
			if(!G_Wifi_Flag_24 && !G_Wifi_Flag_5) {
            	$("#allWifiContentWrap").hide();
            }else{
            	$("#allWifiContentWrap").show();
            }

            top.mainLogic.initModuleHeight();
		}
	}
	/*************END WiFi Name and Password***************/


	/*
	 * 显示访客网络配置
	 * @method wifiGuestModule
	 * @return {无}
	 */
	var wifiGuestModule = new WifiGuestModule();
	pageModule.modules.push(wifiGuestModule);

	function WifiGuestModule() {

		var _this = this;
		this.moduleName = "wifiGuest";

		this.setIptValue = function() {
			var val = this.value.replace(/[^\d\.]/g, "");

			//含有小数位时取整数
            if (this.value.indexOf(".") !== -1) {
                val = this.value.split(".")[0];
            }

	        val = (val == "" ? 0 : val);
	        val = parseFloat(val > 1000 ? 1000 : parseFloat(val).toFixed(2));
	        $(this).parent(".input-append").find("[type=hidden]").val(val);

	        if (parseFloat(val, 10) === 0) {
	            this.value = _("Unlimited");
	        } else {
	            this.value = val;
	        }
		};

		this.init = function () {
			this.initHtml();
			this.initEvent();
		};
		this.initHtml = function() {
			var selectObj = {
                "initVal": _("Unlimited"),
                "editable": "1",
                "seeAsTrans": true,
                "size": "",
                "options": [{
                    "0": _("Unlimited"),
                    "2": "2",
                    "4": "4",
                    "8": "8",
                    ".divider": ".divider",
                    ".hand-set": _("Manual")
                }]
            };
			$("#shareSpeed").toSelect(selectObj);
			$("#shareSpeed .input-box").val(_("Unlimited"));
			$("#shareSpeed input[type=hidden]").val(0);

            $("#shareSpeed input[type='text']").attr("maxLength", 4).on("focus", function () {
            	if(this.value == _("Unlimited")) {
            		return;
            	}
                this.value = this.value.replace(/[^\d\.]/g, "");
            }).on("blur", function () {
                _this.setIptValue.call(this);
            }).each(function () {
                _this.setIptValue.call(this);
            });;
		};

		this.initEvent = function () {
			this.addInputEvent = false;

			$("input[name='guestEn']").on("click", _this.changeGuestEn);

			/*******添加事件，选择共享网速时*******/
			$("#shareSpeed.input-append ul li").on("click", function (e) {
				$("#shareSpeed input[type=hidden]")[0].value = $(this).attr("data-val") == ".hand-set" ? $("#shareSpeed input[type=hidden]").val() : $(this).attr("data-val");
			});
		};

		/**
		 * [initValue 用于初始化显示的数据]
		 * @param  {object} obj [wifiBasicCfg]
		 */
		this.initValue = function (obj) {
			//重置（点击取消时将容错提示语去掉）
			$("#guestSSID,#guestSSID_5G,#guestPwd").removeValidateTipError(true);

			inputValue(obj);

			var shareSpeed = (obj.guestShareSpeed / 128) + "";
        	/*$("#shareSpeed").val(shareSpeed);
        	$("#shareSpeed")[0].val(shareSpeed);*/
        	$("#shareSpeed input[type=hidden]").val(shareSpeed);
        	$("#shareSpeed .input-box").val((shareSpeed == "0" ? _("Unlimited") : shareSpeed));

			if (!this.addInputEvent) {
				$("#guestPwd").initPassword(_("Blank means no password"));
				this.addInputEvent = true;
			}

			_this.changeGuestEn();
		};

		this.getSubmitData = function () {
			var data = {
				module2: this.moduleName,
				guestEn: $("input[name='guestEn']:checked").val(),
				guestEn_5G: $("input[name='guestEn']:checked").val(),
				guestSSID: $("#guestSSID").val(),
				guestSSID_5G: $("#guestSSID_5G").val(),
				guestPwd: $("#guestPwd").val(),
				guestPwd_5G: $("#guestPwd").val(),
				guestValidTime: $("#guestValidTime").val(),
				// guestShareSpeed:  $("#shareSpeed")[0].val() * 128
				guestShareSpeed:  $("#shareSpeed input[type=hidden]").val() * 128
			};

			return objToString(data);
		};

		/***********切换无线开关******/
		this.changeGuestEn = function() {
			var guestEn = $("input[name='guestEn']:checked").val();
			if (guestEn == "true") {
                $("#wifiGuestContent").show();
            } else {
                $("#wifiGuestContent").hide();
            }
            top.mainLogic.initModuleHeight();
		};
	}
	/*************END guest network***************/

	/*
	 * 显示无线信号调节设置
	 * @method wifiPowerModule
	 * @return {无}
	 */
	var wifiPowerModule = new WifiPowerModule();
	pageModule.modules.push(wifiPowerModule);

	function WifiPowerModule() {

		this.moduleName = "wifiPower";
		/**
		 * [initValue 用于初始化显示的数据]
		 * @param  {object} obj [wifiPower]
		 */
		this.initValue = function (obj) {
			inputValue(obj);

			createPowerView24(obj.wifiPowerGear);
			createPowerView5(obj.wifiPowerGear_5G);

			wifiBasicCfgModule && wifiBasicCfgModule.handleWifiSignShow();
			
		};

		this.getSubmitData = function () {
			var data = {
				module3: this.moduleName,
				wifiPower: $("input[name=wifiPower]:checked")[0].value,
				wifiPower_5G: $("input[name=wifiPower_5G]:checked")[0].value
			}
			return objToString(data);
		};

		function createPowerView24(gear) {
			if (gear == "hide_power") {//不支持此频段的信号强度调节
				$("#wifiSignal").hide();
			} else if (gear == "hide_low") {//无低档
				$("#wifiPowerLowerWrap").addClass("none");

				$("#wifiPowerNormalWrap").removeClass("none");
				$("#wifiPowerHighWrap").removeClass("none");
			} else if (gear == "hide_normal") {//无中档
				$("#wifiPowerLowerWrap").removeClass("none");

				$("#wifiPowerNormalWrap").addClass("none");

				$("#wifiPowerHighWrap").removeClass("none");
			} else if (gear == "hide_high") {//无高档
				$("#wifiPowerLowerWrap").removeClass("none");
				$("#wifiPowerNormalWrap").removeClass("none");

				$("#wifiPowerHighWrap").addClass("none");
			} else if(gear == "no_hide") {//有三档
				$("#wifiPowerLowerWrap").removeClass("none");
				$("#wifiPowerNormalWrap").removeClass("none");
				$("#wifiPowerHighWrap").removeClass("none");
			}
		}

		function createPowerView5(gear) {
			if (gear == "hide_power") {//不支持此频段的信号强度调节
				$("#wifiSignal5").hide();
			} else if (gear == "hide_low") {//无低档
				$("#wifiPowerLowerWrap5").addClass("none");

				$("#wifiPowerNormalWrap5").removeClass("none");
				$("#wifiPowerHighWrap5").removeClass("none");
			} else if (gear == "hide_normal") {//无中档
				$("#wifiPowerLowerWrap5").removeClass("none");

				$("#wifiPowerNormalWrap5").addClass("none");

				$("#wifiPowerHighWrap5").removeClass("none");
			} else if (gear == "hide_high") {//无高档
				$("#wifiPowerLowerWrap5").removeClass("none");
				$("#wifiPowerNormalWrap5").removeClass("none");

				$("#wifiPowerHighWrap5").addClass("none");
			} else if(gear == "no_hide") {//有三档
				$("#wifiPowerLowerWrap5").removeClass("none");
				$("#wifiPowerNormalWrap5").removeClass("none");
				$("#wifiPowerHighWrap5").removeClass("none");
			}
		}
	}
	/**************END Wireless Power********/


	/*
	 * 显示无线定时开关设置
	 * @method wifiTimeModule
	 * @return {无}
	 */
	// var wifiTimeModule = new WifiTimeModule();
	// pageModule.modules.push(wifiTimeModule);

	// function WifiTimeModule() {
	// 	var oldDate; /******保存初始化日期******/
	// 	this.moduleName = "wifiTime";

	// 	this.init = function () {
	// 		this.initHtml();
	// 		this.initEvent();
	// 	}
	// 	this.initHtml = function () {
	// 		var hourStr = "",
	// 			minStr = "",
	// 			i = 0;
	// 		for (i = 0; i < 24; i++) {
	// 			hourStr += "<option value='" + ("100" + i).slice(-2) + "'>" + ("100" + i).slice(-2) + "</option>";
	// 		}

	// 		$("#startHour, #endHour").html(hourStr);

	// 		for (i = 0; i < 60; i++) {
	// 			if (i % 5 === 0) {
	// 				minStr += "<option value='" + ("100" + i).slice(-2) + "'>" + ("100" + i).slice(-2) + "</option>";
	// 			}
	// 		}
	// 		$("#startMin, #endMin").html(minStr);
	// 	};
	// 	this.initEvent = function () {
	// 		$("input[name='wifiTimeEn']").on("click", changeWifiTimeEn);
	// 		$("[id^=day]").on("click", clickTimeDay);
	// 	};

	// 	/**
	// 	 * [initValue 用于初始化显示的数据]
	// 	 * @param  {object} obj [wifiTime]
	// 	 */
	// 	this.initValue = function (obj) {
	// 		inputValue(obj);
	// 		translateDate(obj.wifiTimeDate);
	// 		oldDate = obj.wifiTimeDate;
	// 		var time = obj.wifiTimeClose.split("-");
	// 		$("#startHour").val(time[0].split(":")[0]);
	// 		$("#startMin").val(time[0].split(":")[1]);
	// 		$("#endHour").val(time[1].split(":")[0]);
	// 		$("#endMin").val(time[1].split(":")[1]);
	// 		changeWifiTimeEn();
	// 	};
	// 	this.checkData = function () {
	// 		//这两种模式下不需要传该模块值;
	// 		if (pageModule.data.wifiAdvCfg.wifiRelayType == "wisp" || pageModule.data.wifiAdvCfg.wifiRelayType == "client+ap") {
	// 			return;
	// 		}
	// 		if ($("[name='wifiTimeEn']")[0].checked) {
	// 			var date = getScheduleDate();
	// 			if (date == "00000000") {
	// 				return _("Select at least one day.");
	// 			}

	// 			if ($("#startHour").val() + ":" + $("#startMin").val() == $("#endHour").val() + ":" + $("#endMin").val()) {
	// 				return _("The end time and start time cannot be the same.");
	// 			}
	// 		}
	// 		return;
	// 	};
	// 	this.getSubmitData = function () {
	// 		//这两种模式下不需要传该模块值;
	// 		if (pageModule.data.wifiAdvCfg.wifiRelayType == "wisp" || pageModule.data.wifiAdvCfg.wifiRelayType == "client+ap") {
	// 			return "";
	// 		}
	// 		var time = $("#startHour").val() + ":" + $("#startMin").val() + "-" +
	// 			$("#endHour").val() + ":" + $("#endMin").val();
	// 		var data = {
	// 			module4: this.moduleName,
	// 			wifiTimeEn: $("input[name='wifiTimeEn']:checked").val() || "false",
	// 			wifiTimeClose: time,
	// 			wifiTimeDate: getScheduleDate()
	// 		};
	// 		return objToString(data);
	// 	};

	// 	/*******启用或禁用定时开关********/
	// 	function changeWifiTimeEn() {
	// 		if ($("input[name='wifiTimeEn']")[0].checked) {
	// 			$("#wifiScheduleCfg").show();
	// 		} else {
	// 			$("#wifiScheduleCfg").hide();
	// 		}
	// 		top.mainLogic.initModuleHeight();
	// 	}
	// 	/*********获取定时重启日期字符串***********/
	// 	function getScheduleDate() {
	// 		var i = 0,
	// 			len = 8,
	// 			str = "";
	// 		for (i = 0; i < len; i++) {
	// 			if ($("#day" + i)[0].checked) {
	// 				str += "1";
	// 			} else {
	// 				str += "0";
	// 			}
	// 		}
	// 		return str;
	// 	}

	// 	/**********点击everyday**********/
	// 	function clickTimeDay() {
	// 		var dataStr = getScheduleDate();
	// 		if (this.id == "day0") { //点击everyday
	// 			if (this.checked) {
	// 				translateDate("11111111");
	// 			} else {
	// 				translateDate("00000000");
	// 			}
	// 		} else {
	// 			if (dataStr.slice(1) == "1111111") {
	// 				translateDate("11111111");
	// 			} else {
	// 				translateDate("0" + dataStr.slice(1));
	// 			}
	// 		}
	// 	}

	// 	/*******根据字符串改变日期的选择*******/
	// 	function translateDate(str) {
	// 		var dayArry = str.split(""),
	// 			len = dayArry.length,
	// 			i = 0;
	// 		for (i = 0; i < len; i++) {

	// 			$("#day" + i)[0].checked = dayArry[i] == 1;
	// 		}
	// 	}

	// }
	/*************END WiFi Schedule *****************************/

	/*
	 * 显示无线信道与频宽设置
	 * @method wifiAdvCfgModule
	 * @return {无}
	 */
	var wifiAdvCfgModule = new WifiAdvCfgModule();
	pageModule.modules.push(wifiAdvCfgModule);

	function WifiAdvCfgModule() {
		this.moduleName = "wifiAdvCfg";
		this.channelArr_5G = [];

		this.init = function () {
			this.initEvent();
		};

		this.initEvent = function () {
			$("#wifiMode").on("change", function () {
				changeWifiMode_24();
			}); 

			//切换5G频宽，信道随之改变
			$("#wifiBandwidth_5G").on("change", function () {
				wifiAdvCfgModule.changeWifiBand_5();
			});
		};

		/**
		 * [initValue 用于初始化显示的数据]
		 * @param  {object} obj [wifiAdvCfg]
		 */
		this.initValue = function (obj) {
			this.channelArr_5G = obj.wifiChannelList_5G;

			obj.wifiBandwidthCurrent = obj.wifiBandwidthCurrent + "MHz";
			obj.wifiBandwidthCurrent_5G = obj.wifiBandwidthCurrent_5G + "MHz";

			createChannel_24(obj.wifiChannelList);
			changeWifiMode_24(obj.wifiMode);

			this.createBand_5(obj.wifiBandwidth_5G);

			inputValue(obj);

			if (obj.wifiChannelCurrent == "0") {
				$("#wifiBandwidthCurrent, #wifiChannelCurrent").addClass("none");
			} else {
				$("#wifiBandwidthCurrent, #wifiChannelCurrent").removeClass("none");
			}

			if (obj.wifiChannelCurrent_5G == "0") {
				$("#wifiBandwidthCurrent_5G, #wifiChannelCurrent_5G").addClass("none");
			} else {
				$("#wifiBandwidthCurrent_5G, #wifiChannelCurrent_5G").removeClass("none");
			}

			//hasWifiAntijam为false时，没有无线抗干扰功能，隐藏；
			if (obj.hasWifiAntijam == "true") {
				$("#wifiAntijamWrap").removeClass("none");
			} else {
				$("#wifiAntijamWrap").addClass("none");
			}

		};

		this.getSubmitData = function () {
			//这两种模式下不需要传该模块值;
			if (pageModule.data.wifiAdvCfg.wifiRelayType == "wisp" || pageModule.data.wifiAdvCfg.wifiRelayType == "client+ap") {
				return "";
			}

			var data = {
				module5: this.moduleName,
				wifiMode: $("#wifiMode").val(),
				wifiChannel: $("#wifiChannel").val(),
				wifiBandwidth: $("#wifiBandwidth").val(),

				wifiMode_5G: $("#wifiMode_5G").val(),
				wifiChannel_5G: $("#wifiChannel_5G").val(),
				wifiBandwidth_5G: $("#wifiBandwidth_5G").val(),

				wifiAntijamEn: $("#wifiAntijamEn:checked").val() || "false"

			}
			return objToString(data);
		};

		function createChannel_24(channelArr) {
			var str = '<option value="auto">' + _("Auto") + '</option>',
				i = 0;

			for (; i < channelArr.length; i++) {
				str += '<option value="' + channelArr[i] + '">' + _("Channel") + channelArr[i] + "</option>";
			}

			$("#wifiChannel").html(str);
		}

		this.createChannel_5 = function(channelArr) {
			var str = '<option value="auto">' + _("Auto") + '</option>',
				i = 0;

			for (; i < channelArr.length; i++) {
				str += '<option value="' + channelArr[i] + '">' + _("Channel") + channelArr[i] + "</option>";
			}

			$("#wifiChannel_5G").html(str);
		};

		this.createBand_5 = function(_band) {
			if (this.channelArr_5G["80"].length !== 0) {
				$("#wifiBandwidth_5G").html('<option value="20">20</option><option value="40">40</option><option value="80">80</option><option value="auto">20/40/80</option>');
			} else if (this.channelArr_5G["40"].length !== 0) {
				$("#wifiBandwidth_5G").html('<option value="20">20</option><option value="40">40</option><option value="auto">20/40</option>');
			} else {
				$("#wifiBandwidth_5G").html('<option value="20">20</option>').val("20");
			}

			// _band = (_band == "auto"?"80":_band);
			if(_band == "auto") {
				_band = (this.channelArr_5G["80"].length !== 0 ? "80" : "40");	//自动信道时，先取80，如果80没有信道，则取40
			}

			this.createChannel_5(this.channelArr_5G[_band]);
		};

		function changeWifiMode_24(mod) {
			var mode = mod || $("#wifiMode").val();

			if (mode === "bgn") {
				$("#wifiBandwidth").html('<option value="20">20</option><option value="40">40</option><option value="auto">20/40</option>');
			} else {
				$("#wifiBandwidth").html('<option value="20">20</option>');
			}
		}

		this.changeWifiBand_5 = function() {
			var band = $("#wifiBandwidth_5G").val(),
				adv_channel_5g = $("#wifiChannel_5G").val();

			// band = (band == "auto"?"80":band);
			if(band == "auto") {
				band = (this.channelArr_5G["80"].length !== 0 ? "80" : "40");	//自动信道时，先取80，如果80没有信道，则取40
			}

			this.createChannel_5(this.channelArr_5G[band]);

			if ($("#wifiChannel_5G option[value='" + adv_channel_5g + "']").length == 0) {
				$("#wifiChannel_5G").val("auto");
			} else {
				$("#wifiChannel_5G").val(adv_channel_5g);
			}
		};
	}
	/**************END Wireless Parameters********/


	/*
	 * 显示无线wifiBeamforming设置
	 * @method wifiBeamformingModule
	 * @return {无}
	 */
	var wifiBeamformingModule = new WifiBeamformingModule();
	pageModule.modules.push(wifiBeamformingModule);

	function WifiBeamformingModule() {
		this.moduleName = "wifiBeamforming";

		this.init = function () {
			this.initEvent();
		};
		this.initEvent = function () {
		};

		this.initValue = function (obj) {
			inputValue(obj);

			if(obj.hasWifiBeaforming == "false") {
				$("#wifiBeaformingWrap").hide();
			}
		}
		this.getSubmitData = function () {
			var data = {
				module6: this.moduleName,
				wifiBeaformingEn: $("input[name='wifiBeaformingEn']:checked").val() || "false"
			};
			return objToString(data);
		}
	}
	/*************END Beamforming *******************************/

	/*
	 * 显示无线WPS设置
	 * @method wifiWPSModule
	 * @return {无}
	 */
	// if(CONFIG_HASWPS === true){
	if(top.G_data.wpsModule.hasWPSModule == "true") {
		var wifiWPSModule = new WifiWPSModule();
		pageModule.modules.push(wifiWPSModule);
	// }

		function WifiWPSModule() {
			this.moduleName = "wifiWPS";

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
						mainLogic.showModuleMsg(_("PBC is configured successfully."));
						$("#wpsPBC").removeAttr("disabled");
					});
				});
			};

			/**
			 * [initValue 用于初始化显示的数据]
			 * @param  {object} obj [wifiWPS]
			 */
			this.initValue = function (obj) {
				inputValue(obj);
				changeWpsEn();
			}
			this.getSubmitData = function () {
				//这两种模式下不需要传该模块值;
				if (pageModule.data.wifiAdvCfg.wifiRelayType == "ap" || pageModule.data.wifiAdvCfg.wifiRelayType == "client+ap") {
					return "";
				}

				var data = {
					module7: this.moduleName,
					wpsEn: $("input[name='wpsEn']:checked").val() || "false"
				};
				return objToString(data);
			}

			/*******启用或禁用WPS********/
			function changeWpsEn() {
				if ($("input[name='wpsEn']").length > 0 && $("input[name='wpsEn']")[0].checked) {
					$("#wpsCfg").removeClass("none");
				} else {
					$("#wpsCfg").addClass("none");
				}
				top.mainLogic.initModuleHeight();
			}
		}
	}
	/*************END WPS *******************************/
})
