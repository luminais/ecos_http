define(function (require, exports, module) {
	var pageModule = new PageLogic({
		getUrl: "goform/getQos",
		modules: "localhost,onlineList,macFilter",
		setUrl: "goform/setQos"
	});
	pageModule.modules = [];
	module.exports = pageModule;

	/********Attached Devices and Blocked Devices******************/
	var netCrlModule = new AttachedModule();
	pageModule.modules.push(netCrlModule);

	//获取黑名单个数
	function getBlackLength() {
		var index = 0,
			i = 0,
			$listTable = $("#qosList").children(),
			length = $listTable.length,
			$blackTable = $("#qosListAccess").children(),
			blackLength = $blackTable.length,
			$tr;

		//access
		if (length == 1 && $listTable.eq(0).children().length < 2) {

		} else {
			for (i = 0; i < length; i++) {
				$tr = $listTable.eq(i);

				//判断是否关闭 或者上传或下载限制为0
				if ($tr.find(".switch").hasClass("toggle-off-icon")) {
					index++;
				}
			}
		}

		//black
		for (i = 0; i < blackLength; i++) {
			//存在未解除限制的条目
			if ($blackTable.eq(i).find(".deviceName").html()) {
				index++;
			}
		}
		return index;


	}

	pageModule.beforeSubmit = function () {
		if (getBlackLength() > 10) {
			top.mainLogic.showModuleMsg(_("A maximum of %s devices can be added to the blacklist.", [10]));
			return false;
		}
		return true;
	}

	function AttachedModule() {
		var timeFlag,
			refreshDataFlag = true,
			dataChanged;
		var that = this;

		this.data = {};

		this.onlineListData = {};

		this.curFilterMode = "";

		this.moduleName = "qosList";

		this.init = function () {
			dataChanged = false;
			this.initEvent();
		}
		this.initEvent = function () {
			$("#qosList").delegate(".icon-edit", "click", editDeviceName);

			// $("#qosList").delegate(".icon-toggle-on, .icon-toggle-off", "click", clickAccessInternet);
			$("#qosList").delegate(".switch", "click", clickAccessInternet);

			$("#qosList").delegate(".edit-old", "blur", function () {
				$(this).parent().prev().attr("title", $(this).val());
				$(this).parent().prev().text($(this).val());
				$(this).parent().hide();
				$(this).parent().prev().show();
				$(this).parent().next().show();
			});

			$("#qosListAccess").delegate(".del", "click.dd", function (evnet) {
				var e = evnet || window.event;
				$(this).parent().parent().remove();
				dataChanged = true;
				//return;
			});

			//限制输入字节数不超过63
			$("#qosList").delegate(".setDeviceName", "keyup", function () {
				var deviceVal = this.value,
					len = deviceVal.length, //输入总字符数
					totalByte = getStrByteNum(deviceVal); //输入总字节数

				if (totalByte > 63) {
					for (var i = len - 1; i > 0; i--) {
						totalByte = totalByte - getStrByteNum(deviceVal[i]); //每循环一次，总字节数就减去最后一个字符的字节数

						if (totalByte <= 63) { //直到总字节数小于等于63，i值就是边界字符的下标
							this.value = deviceVal.slice(0, i);
							break;
						}
					}
				}

			});

		};
		this.initValue = function () {
			this.data = pageModule.data;

			timeFlag = setTimeout(function () {
				refreshTableList();
			}, 5000);

			$("#qosList").html(""); //
			$("#qosListAccess").html(""); //受限制table


			createOnlineList(this.data);
		};

		/**
		 * @method createOnlineList [创建在线及离线显示列表]
		 * @param  {Object} obj [对象中包含本机IP地址， 在线列表，黑名单列表]
		 */
		function createOnlineList(obj) {
			var i = 0,
				k = 0,
				str = "",
				limitStr = "",
				onlineListlen, blackListLen,
				prop, divElem, tdElem, trElem, upLimit, downLimit,
				localhostIP, hostname, connectType, ipStr, nativeHost; //nativeHost变量表示是否是本机


			localhostIP = obj.localhost.localhost;
			obj.onlineList = reCreateObj(obj.onlineList, "qosListIP", "up");

			//将本机添加到数组首位
			for (var i = 0; i < obj.onlineList.length; i++) {
				if (obj.onlineList[i].qosListIP == localhostIP) {
					var local = obj.onlineList[i];
					obj.onlineList.splice(i, 1);
					obj.onlineList.unshift(local);
					break;
				}
			}

			obj.macFilterList = reCreateObj(obj.macFilter.macFilterList, "mac", "up");
			onlineListlen = obj.onlineList.length;
			macFilterListLen = obj.macFilterList.length;
			//初始化在线列表
			for (i = 0; i < onlineListlen; i++) {
				str = "<tr class='addListTag'>"; //类只为初始化与赋值，用完就删除
				for (prop in obj.onlineList[i]) {
					if (obj.onlineList[i].qosListRemark != "") {
						hostname = obj.onlineList[i].qosListRemark;
					} else {
						hostname = obj.onlineList[i].qosListHostname;
					}

					//如果localhostIP == qosListIP，则代表本机
					if (localhostIP == obj.onlineList[i].qosListIP) {
						nativeHost = true;
					} else {
						nativeHost = false;
					}

					if (localhostIP == obj.onlineList[i].qosListIP) {
						ipStr = obj.onlineList[i].qosListIP + _("Local");
					} else {
						ipStr = obj.onlineList[i].qosListIP;
					}

					var manufacturer = translateManufacturer(obj.onlineList[i].qosListManufacturer);
					if (prop == "qosListHostname") { //主机名
						str += '<td>';
						str += "<div class='col-xs-3 col-sm-3 col-md-2 col-lg-2'>" + manufacturer + "</div>"
						str += '<div class="col-xs-9 col-sm-9 col-md-10 col-lg-9" style="margin-top:10px;"><div class="col-xs-10 span-fixed deviceName" style="height:24px;"></div>';
						str += '<div class="col-xs-10 none">';
						str += ' <input type="text" class="form-control setDeviceName" style="height:24px;padding: 3px 12px;" value="" maxLength="63">';
						str += '</div>';
						str += '<div class="col-xs-2 row"> <span class="ico-small icon-edit"></span> </div>';

						str += '</td>';
					} else if (prop == "qosListMac") {
						//在此处增加width:10%是为了规避由于上、下载数值较小时头部文字出现换行；
						str += '<td class="span-fixed hidden-max-sm" style="width:10%;text-align:center">';
						str += '<span data-target="qosListMac">' + obj.onlineList[i][prop].toUpperCase() + '</span>';

						str += '</td>';
					} else if (prop == "qosListAccess") {
						//白名单模式下没有接入控制方式
						if (obj.macFilter.curFilterMode == "pass") {
							$("#onlineListHead").find(".connectPermit").css("display", "none");
							continue;
						}
						//定时刷新如果是黑名单模式则显示允许连接项
						$("#onlineListHead").find(".connectPermit").css("display", "");

						str += '<td style="text-align:center" class="internet-ctl">';
						if (nativeHost) {
							str += "<div class='nativeHost'>" + _("Local") + "</div>"
						} else {
							// str += "<div class='switch icon-toggle-on'></div>";
							str += "<div class='switch toggle-on-icon'></div>";
						}
						str += '</td>';
					}
				}
				str += '</tr>';
				$("#qosList").append(str);
				$("#qosList .addListTag").find(".deviceName").text(hostname); //主机名赋值
				$("#qosList .addListTag").find(".deviceName").attr("title", hostname);
				$("#qosList .addListTag").find(".setDeviceName").val(hostname);
				$("#qosList .addListTag").find(".setDeviceName").attr("data-mark", obj.onlineList[i].qosListHostname); //绑定主机名
				var upperMac = obj.onlineList[i].qosListMac.toUpperCase();
				$("#qosList .addListTag").find(".setDeviceName").attr("alt", upperMac); //绑定mac
				//add by xc 存储查询返回的在线设备的qosListUpLimit和qosListDownLimit
				that.onlineListData[upperMac] = obj.onlineList[i];

				$("#qosList").find(".addListTag").removeClass("addListTag");
			} //end

			//初始化黑名单列表
			for (k = 0; k < macFilterListLen; k++) {
				if (obj.macFilterList[k].filterMode == "pass") {
					continue;
				}

				str = "<tr class='addListTag'>";
				str += "<td class='deviceName'><div class='col-xs-11 span-fixed'>";
				str += "</div></td>";
				str += "<td class='hidden-max-sm' data-target='mac' style='width:10%;text-align:center'>" + obj.macFilterList[k].mac.toUpperCase() + "</td>";

				str += "<td style='text-align:center'>";
				str += '<input type="button" class="del btn" value="' + _("Unlimit") + '">';
				str += "</td>";
				str += "</tr>";
				$("#qosListAccess").append(str);
				if (obj.macFilterList[k].remark != "") {
					hostname = obj.macFilterList[k].remark;
				} else {
					hostname = (obj.macFilterList[k].hostname  == "Unknown"?_("Unknown"):obj.macFilterList[k].hostname);
				}

				$("#qosListAccess .addListTag").find(".deviceName div").text(hostname);
				$("#qosListAccess .addListTag").find(".deviceName").attr("data-hostname", obj.macFilterList[k].hostname);
				$("#qosListAccess .addListTag").find(".deviceName").attr("data-remark", obj.macFilterList[k].remark);
				$("#qosListAccess").find(".addListTag").removeClass("addListTag");
			} //end
			$("#qosDeviceCount").html("(" + $("#qosList").children().length + ")");
			if ($("#qosList").children().length == 0) {
				str = "<tr><td colspan='2'>" + _("No device") + "</td></tr>";
				$("#qosList").append(str);
			}

			$("#blockedDeviceCount").html("(" + $("#qosListAccess").children().length + ")");
			if ($("#qosListAccess").children().length == 0) {
				str = "<tr><td colspan='2'>" + _("No device") + "</td></tr>";
				$("#qosListAccess").append(str);
			}

			//白名单模式下没有互联网访问控制及黑名单功能。
			if (obj.macFilter.curFilterMode == "pass") {
				$("#blockedDevices").addClass("none");
			} else {
				$("#blockedDevices").removeClass("none");
			}
			that.curFilterMode = obj.macFilter.curFilterMode;
			top.mainLogic.initModuleHeight();
		}

		this.checkData = function () { //数据验证
			var deviceName = "",
				$listTable = $("#qosList").children(),
				length = $listTable.length,
				$td,
				upLimit,
				downLimit,
				i = 0;
			if (length == 1 && $listTable.eq(0).children().length < 2) {
				return;
			}
			for (i = 0; i < length; i++) {
				$td = $listTable.eq(i).children();
				deviceName = $td.find("input[data-mark]").val(); //device name

				if (deviceName.replace(/[ ]/g, "") == "") {
					//$td.find("input[data-mark]").focus();
					//当前元素已隐藏，不能聚焦  IE8会出错
					return _("No space is allowed in a device name.");
				}

			}
			return;
		};

		this.getSubmitData = function () { //获取提交数据
			var listArray = getBlockedList().concat(getAttacheList()),
				onlineListStr = departList("true"),
				offlineListStr = departList("false"),
				onlineObj = {},
				offlineObj = {};


			onlineObj = {
				module1: "onlineList",
				onlineList: onlineListStr
			};

			offlineObj = {
				module2: "macFilter",
				macFilterList: offlineListStr

			};
			if (that.curFilterMode == "pass") {
				return objToString(onlineObj);
			} else {
				return objToString(onlineObj) + "&" + objToString(offlineObj);
			}

			function departList(type) {
				var i = 0,
					tmpList = [],
					tmpStr = "";
				for (i = 0; i < listArray.length; i++) {
					if (listArray[i].access == type) {
						tmpStr += listArray[i].hostname + "\t";
						tmpStr += listArray[i].remark + "\t";
						tmpStr += listArray[i].mac + "\t";
						tmpStr += listArray[i].upLimit + "\t";
						tmpStr += listArray[i].downLimit + "\t";
						tmpStr += listArray[i].access + "\n";
					}
				}
				return tmpStr.replace(/[\n]$/, "");
			}

		};

		function shapingSpeed(value) {
			var val = parseFloat(value);

			if (val > 1024) {
				return (val / 1024).toFixed(2) + "MB/s";
			} else {
				return val.toFixed(0) + "KB/s";
			}
		}

		function refreshTableList() {
			$.get("goform/getQos?" + getRandom() + encodeURIComponent("&modules=localhost,onlineList,macFilter"), updateTable);

			if (!refreshDataFlag || dataChanged) {
				clearTimeout(timeFlag);
				return;
			}
			clearTimeout(timeFlag);
			timeFlag = setTimeout(function () {
				refreshTableList();
			}, 5000);
			if (!pageModule.pageRunning) {
				clearTimeout(timeFlag);
			}
		}

		/*
		 *
		 * @method updateTable 处理获取到的在线列表数据：
		 *    1、如果用户手动修改了当前在线列表数据，则保持不变；如果当前在线列表数据在CGI有更新，则针对该数据进行更新。
		 *    2、如果CGI有新增的在线条目，则append至在线列表里面
		 *    3、如果CGI有删除的在线条目，则将在线列表里面的数据删除
		 * @param {Object} 对象中包含本机，在线列表，黑名单列表数据
		 * @return {Array} 由本机IP、在线列表、黑名单列表组合成的数组
		 *
		 */
		function updateTable(obj) {
			if (checkIsTimeOut(obj)) {
				top.location.reload(true);
			}
			try {
				obj = $.parseJSON(obj);
			} catch (e) {
				obj = {};
			}

			if (isEmptyObject(obj)) {
				top.location.reload(true);
				return;
			}

			if (!pageModule.pageRunning || dataChanged) {
				return;
			}
			var getOnlineList = obj.onlineList;

			var $onlineTbodyList = $("#qosList").children(),
				onlineTbodyLen = $onlineTbodyList.length,
				getOnlineLen = getOnlineList.length,
				j = 0,
				i = 0,
				oldMac, newMac;

			var rowData = new Array(onlineTbodyLen);
			var refreshObj = new Array(getOnlineLen);
			var newDataArray = [];

			for (i = 0; i < getOnlineLen; i++) {
				newMac = getOnlineList[i].qosListMac.toUpperCase();
				that.onlineListData[newMac] = getOnlineList[i];
				refreshObj[i] = {};
				for (j = 0; j < onlineTbodyLen; j++) {
					//TODO : ReasyJS不能对空对象进行操作, 如： 全部被拦截上网的情况
					var $input = $onlineTbodyList.eq(j).find("input[data-mark]");
					if ($input[0]) {
						oldMac = $input.attr("alt").toUpperCase();
					} else {
						oldMac = '';
					}

					if (oldMac == newMac) { //存在
						rowData[j] = {};
						/*if ($onlineTbodyList.eq(j).children().eq(5).children().hasClass("icon-toggle-on")) { //当前为允许时
							$onlineTbodyList.eq(j).children().eq(2).find("span").eq(1).html(shapingSpeed(getOnlineList[i].qosListUpSpeed));

							$onlineTbodyList.eq(j).children().eq(1).find("span").eq(1).html(shapingSpeed(getOnlineList[i].qosListDownSpeed));
						}*/
						rowData[j].refresh = true; //
						refreshObj[i].exist = true;
					}
					if ($onlineTbodyList.eq(i).find("input[data-mark]").hasClass("edit-old")) { //已编辑过的不能删除
						rowData[j] = {};
						rowData[j].refresh = true;
					}
				}
			}

			for (i = 0; i < getOnlineLen; i++) {
				if (!refreshObj[i].exist) {
					newDataArray.push(getOnlineList[i]); //新增
				}
			}

			for (j = 0; j < onlineTbodyLen; j++) {
				if (!rowData[j] || !rowData[j].refresh) {
					//将在线列表中当前已经不在线的设备删除
					$onlineTbodyList.eq(j).remove();
				}
			}

			//清除黑名单列表中显示的数据，重新初始化
			$("#qosListAccess").html("");
			obj.onlineList = newDataArray;
			if (obj.macFilter.curFilterMode == "pass") {
				$("#blockedDevices").addClass("none");
			} else {
				$("#blockedDevices").removeClass("none");
			}
			createOnlineList(obj);

		};

		function editDeviceName() {
			var deviceName = $(this).parent().prev().prev().text(),
				reMarkMaxLength = "";
			$(this).parent().prev().prev().hide(); //隐藏用户名
			$(this).parent().hide(); //隐藏编辑
			$(this).parent().prev().show();
			$(this).parent().prev().find("input").addClass("edit-old"); //编辑时给编辑元素增加类标志
			reMarkMaxLength = $(this).parent().prev().find("input").attr("maxLength");
			$(this).parent().prev().find("input").val(deviceName.substring(0, reMarkMaxLength));
			$(this).parent().prev().find("input").focus();
		}

		function clickAccessInternet() {
			var className = this.className || "switch toggle-on-icon";
			if (className == "switch toggle-on-icon") {

				if (getBlackLength() >= 10) {
					top.mainLogic.showModuleMsg(_("A maximum of %s devices can be added to the blacklist.", [10]));
					return;
				}
				this.className = "switch toggle-off-icon";
			} else {
				this.className = "switch toggle-on-icon";
			}
		}

		//获取允许列表数据
		function getAttacheList() {
			var str = "",
				$listTable = $("#qosList").children(),
				length = $listTable.length,
				$tr,
				hostname,
				internetAccess,
				tmpList = [],
				i = 0;

			//列表为空时
			if (length == 1 && $listTable.eq(0).children().length < 2) {
				return tmpList;
			}
			for (i = 0; i < length; i++) {
				var tmpObj = {},
					upLimit, downLimit;

				$tr = $listTable.eq(i);
				//设备名
				tmpObj.hostname = $tr.find("input[data-mark]").val();
				//备注
				if ($tr.find("input[data-mark]").val() == $tr.find("input[data-mark]").attr("data-mark")) { //主机名与备注一样
					tmpObj.remark = "";
				} else {
					tmpObj.remark = $tr.find("input[data-mark]").val();
				}

				//mac地址
				tmpObj.mac = $tr.find("input[data-mark]").attr("alt");
				//限速
				// tmpObj.upLimit = "38400";
				// tmpObj.downLimit = "38400";
				//edit by xc 上传和下载速度限制用router模式下设置的值，不能重置
				if(tmpObj.mac){
					var upperMac = tmpObj.mac.toUpperCase();
					tmpObj.upLimit = that.onlineListData[upperMac]["qosListUpLimit"];
					tmpObj.downLimit = that.onlineListData[upperMac]["qosListDownLimit"];
				}

				//当允许接入选项为“本机”或者按钮为开启时，传值设为"true"
				internetAccess = $tr.find(".internet-ctl").children().hasClass("toggle-on-icon") || $tr.find(".internet-ctl").children().hasClass("nativeHost");

				if (internetAccess || that.curFilterMode == "pass") {
					tmpObj.access = "true";
				} else {
					tmpObj.access = "false";
				}
				tmpList.push(tmpObj);
			}

			return tmpList;
		}

		//获取禁止列表数据
		function getBlockedList() {
			var $listTable = $("#qosListAccess").children(),
				length = $listTable.length,
				i = 0,
				tmpList = [];
			//暂无设备时
			if (length == 1 && $listTable.eq(0).children().length < 2) {

				return tmpList;
			}
			for (i = 0; i < length; i++) {
				var tmpObj = {};

				tmpObj.hostname = $listTable.eq(i).find("td[data-remark]").attr("data-hostname");
				tmpObj.remark = $listTable.eq(i).find("td[data-remark]").attr("data-remark");

				tmpObj.mac = $listTable.eq(i).find("[data-target='mac']").text(); //mac
				tmpObj.upLimit = "0";
				tmpObj.downLimit = "0";
				tmpObj.access = "false";
				tmpList.push(tmpObj);
			}

			return tmpList;
		}
	}
	/********END Attached Devices and Blocked Devices ************************/
})
