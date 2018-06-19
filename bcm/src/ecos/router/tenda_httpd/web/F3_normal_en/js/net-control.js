define(function (require, exports, module) {
	var pageModule = new PageLogic("goform/getQos", "goform/setQos");
	pageModule.modules = [];
	module.exports = pageModule;

	/********Attached Devices and Blocked Devices******************/
	var netCrlModule = new AttachedModule();
	pageModule.modules.push(netCrlModule);

	function AttachedModule() {
		var timeFlag,
			refreshDataFlag = true,
			dataChanged = false;

		this.init = function () {
			this.initEvent();
		}
		this.initEvent = function () {
			$("#qosList").delegate(".icon-edit", "click", editDeviceName);

			$("#qosList").delegate(".icon-toggle-on, .icon-toggle-off", "click", clickAccessInternet);

			$("#qosList").delegate(".edit-old", "blur", function () {
				$(this).parent().prev().attr("title", $(this).val());
				$(this).parent().prev().text($(this).val());
				$(this).parent().hide();
				$(this).parent().prev().show();
				$(this).parent().next().show();
			});

			$("#qosListAccess").delegate(".del", "click.dd", function (evnet) {
				var e = evnet || window.event;
				$(this).parent().parent().css("display", "none");
				dataChanged = true;
				//return;
			});

			//解决在其他语言下选择无限制时 出现No Limit 的问题
			$("#qosList").delegate(".dropdown li", "click", function () {
				if (this.getAttribute("data-val") == "No Limit") {
					$(this).parent().parent().parent().find("input[type='text']").val(_("No Limit"));
					//解决赋值问题
					$(this).parent().parent().parent().find("input[type='hidden']").val(_("No Limit"));
				}
			});

			$("#qosList").delegate(".dropdown input[type='text']", "blur.refresh", function () {
				refreshDataFlag = true;
				if (this.value.replace(/[ ]+$/, "") == _("No Limit") || this.value.replace(/[ ]+$/, "") == "No Limit") {
					this.value = _("No Limit");
					return;
				}
				if (isNaN(parseFloat(this.value))) {
					this.value = "";
				} else {
					if (parseFloat(this.value).toFixed(2) > 300) { //大于300则表示无限制
						this.value = _("No Limit");
					} else {
						if (parseFloat(this.value).toFixed(2) == parseInt(this.value, 10)) {
							this.value = parseInt(this.value, 10) + "Mbps";
						} else {
							this.value = parseFloat(this.value).toFixed(2) + "Mbps";
						}
					}

				}

				timeFlag = setTimeout(function () {
					refreshTableList();
				}, 5000);
			});

			$("#qosList").delegate(".dropdown .input-box", "focus.refresh change.refresh", function () {
				this.value = this.value.replace(/[Mbps]+$/, "");
				refreshDataFlag = false;
			});
		};
		this.initValue = function () {
			timeFlag = setTimeout(function () {
				refreshTableList();
			}, 5000);
			obj = pageModule.data;
			$("#qosList").html(""); //
			$("#qosListAccess").html(""); //受限制table
			createOnlineList(obj.qosList);
		};

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
				deviceName = $td.find("input").eq(0).val(); //device name
				upLimit = ($td.eq(4).find(".input-append")[0].val()); //up limit
				downLimit = ($td.eq(3).find(".input-append")[0].val()); //down limit
				if (deviceName.replace(/[ ]/g, "") == "") {
					$td.find("input").eq(0).focus();
					return _("Space is not supported in a password!");
				}
				if (isNaN(parseFloat(upLimit)) && upLimit != _("No Limit")) {
					$td.eq(4).find(".dropdown .input-box").focus();
					return _("Please input a valid number.")
				}

				if (isNaN(parseFloat(downLimit)) && downLimit != _("No Limit")) {
					$td.eq(3).find(".dropdown .input-box").focus();
					return _("Please input a valid number.")
				}
			}
			return;
		};

		this.getSubmitData = function () { //获取提交数据
			var data = {},
				list,
				blockedData = getBlockedList(),
				attachedData = getAttacheList();
			list = attachedData;
			if (blockedData.permit.length > 0) {
				list += "\n" + blockedData.permit;
			}
			if (blockedData.forbid.length > 0) {
				list += "\n" + blockedData.forbid;
			}

			data = {
				qosList: list
			}
			return objToString(data);
		};

		//创建在线和禁用表格列表
		function createOnlineList(arry) {
			var i = 0,
				len = arry.length,
				str = "",
				limitStr = "",
				prop,
				divElem,
				tdElem,
				trElem;
			var selectDownObj = {
					"initVal": "21",
					"editable": "1",
					"size": "small",
					"options": [{
						"No Limit": _("No Limit")
					}, {
						"1": _("1.0Mbps(Web Browsing)")
					}, {
						"2": _("2.0Mbps(SD Videos)")
					}, {
						"3": _("3.0Mbps(HD Videos)")
					}, {
						".divider": ".divider"
					}, {
						".hand-set": _("Manual(unit: Mbps)")
					}]
				},
				selectUpObj = {
					"initVal": "21",
					"editable": "1",
					"size": "small",
					"options": [{
						"No Limit": _("No Limit")
					}, {
						"0.25": _("0.25Mbps")
					}, {
						"0.5": _("0.50Mbps")
					}, {
						"1": _("1.0Mbps")
					}, {
						".divider": ".divider"
					}, {
						".hand-set": _("Manual(unit: Mbps)")
					}]
				};

			var upLimit,
				downLimit;
			var hostname,
				connectType;

			for (i = 0; i < len; i++) {
				if (arry[i].qosListAccess && arry[i].qosListAccess == "true") { //可以上网
					str = "<tr class='addListTag'>"; //类只为初始化与赋值，用完就删除
					for (prop in arry[i]) {
						if (arry[i].qosListRemark != "") {
							hostname = arry[i].qosListRemark;
						} else {
							hostname = arry[i].qosListHostname;
						}
						if (arry[i].qosListConnectType == "wifi") {
							connectType = "icon-wireless";
						} else {
							connectType = "icon-wired";
						}
						if (prop == "qosListHostname") { //主机名
							str += '<td>';
							str += '<div class="col-xs-10 span-fixed deviceName"></div>';
							str += '<div class="col-xs-10 none">';
							str += ' <input type="text" class="form-control setDeviceName" value="">';
							str += '</div>';
							str += '<div class="col-xs-2 row"> <span class="ico-small icon-edit"></span> </div>';
							str += '<div class="col-xs-12 help-inline"> <span class="ico-small ' + connectType + '"></span> <span>' + arry[i].qosListIP + '</span> </div>';
							str += '</td>';
						} else if (prop == "qosListUpSpeed" || prop == "qosListDownSpeed") {
							str += '<td class="span-fixed">';
							if (prop == "qosListUpSpeed") {
								str += '<span class="text-warning">&uarr;</span> <span>' + arry[i][prop] + 'Mbps</span>';
							} else {
								str += '<span class="text-success">&darr;</span> <span>' + arry[i][prop] + 'Mbps</span>';
							}
							str += '</td>';
						} else if (prop == "qosListUpLimit" || prop == "qosListDownLimit") {
							str += '<td>';
							str += '<span class="dropdown ' + prop + ' input-medium validatebox" required="required" maxLength="5"></span>';
							str += '</td>';
							if (prop == "qosListUpLimit") {
								upLimit = arry[i][prop] + "Mbps";
								if (+arry[i][prop] == 301) {
									upLimit = _("No Limit");
								}
							} else {
								downLimit = arry[i][prop] + "Mbps";
								if (+arry[i][prop] == 301) {
									downLimit = _("No Limit");
								}
							}
						} else if (prop == "qosListAccess") {
							str += '<td>';
							str += "<div class='switch icon-toggle-on'></div>";
							str += '</td>';

						}
					}
					str += '</tr>';
					$("#qosList").append(str);
					$("#qosList .addListTag").find(".deviceName").text(hostname); //主机名赋值
					$("#qosList .addListTag").find(".deviceName").attr("title", hostname);
					$("#qosList .addListTag").find(".setDeviceName").val(hostname);
					$("#qosList .addListTag").find(".setDeviceName").attr("data-mark", arry[i].qosListHostname); //绑定主机名
					$("#qosList .addListTag").find(".setDeviceName").attr("alt", arry[i].qosListMac); //绑定mac
					selectUpObj.initVal = upLimit;
					$("#qosList .addListTag").find(".qosListUpLimit").toSelect(selectUpObj); //初始化下拉框
					selectDownObj.initVal = downLimit;
					$("#qosList .addListTag").find(".qosListDownLimit").toSelect(selectDownObj); //初始化下拉框
					$("#qosList").find(".addListTag").removeClass("addListTag");
				} else { //受限制
					str = "<tr class='addListTag'>";
					str += "<td class='deviceName'>";
					tdElem = document.createElement("td");
					if (typeof tdElem.innerText != "undefined") {
						tdElem.innerText = arry[i].qosListHostname;
					} else { //firefox
						tdElem.textContent = arry[i].qosListHostname;
					}
					str += "</td>";
					str += "<td>" + arry[i].qosListMac + "</td>";

					str += "<td>";
					str += '<input type="button" class="del btn" value="' + _("Remove") + '">';
					str += "</td>";
					str += "</tr>";
					$("#qosListAccess").append(str);
					if (arry[i].qosListRemark != "") {
						hostname = arry[i].qosListRemark;
					} else {
						hostname = arry[i].qosListHostname;
					}
					$("#qosListAccess .addListTag").find(".deviceName").text(hostname);
					$("#qosListAccess .addListTag").find(".deviceName").attr("data-mark", arry[i].qosListHostname);
					$("#qosListAccess").find(".addListTag").removeClass("addListTag");
				}
			}
			$("#qosDeviceCount").html("(" + $("#qosList").children().length + ")");
			if ($("#qosList").children().length == 0) {
				str = "<tr><td>" + _("No device") + "</td></tr>";
				$("#qosList").append(str);
			}

			$("#blockedDeviceCount").html("(" + $("#qosListAccess").children().length + ")");
			if ($("#qosListAccess").children().length == 0) {
				str = "<tr><td>" + _("No device") + "</td></tr>";
				$("#qosListAccess").append(str);
			}

			top.mainLogic.initModuleHeight();
		}

		function refreshTableList() {
			$.getJSON("goform/getQos" + getRandom(), updateTable);

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

		function updateTable(obj) {

			var listData = obj.qosList,
				newLength = listData.length,
				newMac,
				i = 0;
			if (!pageModule.pageRunning || dataChanged) {
				return;
			}
			var $tbodyList = $("#qosList").children(),
				oldLength = $tbodyList.length,
				oldMac,
				j = 0;
			var rowData = new Array(oldLength);
			var refreshObj = new Array(newLength);
			var newData = [];

			for (i = 0; i < newLength; i++) {
				newMac = listData[i].qosListMac;
				refreshObj[i] = {};
				for (j = 0; j < oldLength; j++) {
					oldMac = $tbodyList.eq(j).children().eq(0).find("input").eq(0).attr("alt");
					if (oldMac == newMac) { //存在
						rowData[j] = {};
						if ($tbodyList.eq(j).children().eq(5).children().hasClass("icon-toggle-on")) { //当前为允许时
							$tbodyList.eq(j).children().eq(2).find("span").eq(1).html(listData[i].qosListUpSpeed + "Mbps");

							$tbodyList.eq(j).children().eq(1).find("span").eq(1).html(listData[i].qosListDownSpeed + "Mbps");
						}
						rowData[j].refresh = true; //
						refreshObj[i].exist = true;
					}
					if ($tbodyList.eq(i).children().eq(0).find("input").eq(0).hasClass("edit-old")) { //已编辑过的不能删除
						rowData[j] = {};
						rowData[j].refresh = true;
					}
				}
			}

			for (i = 0; i < newLength; i++) {
				if (!refreshObj[i].exist) {
					newData.push(listData[i]); //新增
				}
			}

			for (j = 0; j < oldLength; j++) {
				if (!rowData[j] || !rowData[j].refresh) { //不需要保留的数据全部删除
					$tbodyList.eq(j).remove();
				}
			}
			$("#qosListAccess").html("");
			createOnlineList(newData);
		};

		function editDeviceName() {
			var deviceName = $(this).parent().prev().prev().text();
			$(this).parent().prev().prev().hide(); //隐藏用户名
			$(this).parent().hide(); //隐藏编辑
			$(this).parent().prev().show();
			$(this).parent().prev().find("input").addClass("edit-old"); //编辑时给编辑元素增加类标志
			$(this).parent().prev().find("input").val(deviceName);
			$(this).parent().prev().find("input").focus();
		}

		function clickAccessInternet() {
			var className = this.className;
			if (className == "switch icon-toggle-on") {
				this.className = "switch icon-toggle-off";
			} else {
				this.className = "switch icon-toggle-on";
			}
		}

		//获取允许列表数据
		function getAttacheList() {
			var str = "",
				$listTable = $("#qosList").children(),
				length = $listTable.length,
				$td,
				hostname,
				i = 0;
			if (length == 1 && $listTable.eq(0).children().length < 2) {
				return "";
			}
			for (i = 0; i < length; i++) {
				$td = $listTable.eq(i).children();
				str += $td.find("input").eq(0).attr("data-mark") + "\t"; //主机名
				if ($td.find("input").eq(0).val() == $td.find("input").eq(0).attr("data-mark")) { //主机名与备注一样
					str += "" + "\t";
				} else {
					str += $td.find("input").eq(0).val() + "\t"; //备注名
				}
				str += $td.find("input").eq(0).attr("alt") + "\t"; //mac	
				str += ($td.eq(4).find(".input-append")[0].val() == _("No Limit") ? "301" : parseFloat($td.eq(4).find(".input-append")[0].val())) + "\t"; //up limit
				str += ($td.eq(3).find(".input-append")[0].val() == _("No Limit") ? "301" : parseFloat($td.eq(3).find(".input-append")[0].val())) + "\t"; //down limit
				str += $td.eq(5).children().hasClass("icon-toggle-on") + "\n";
			}
			str = str.replace(/[\n]$/, "");
			return str
		}

		//获取禁止列表数据 
		function getBlockedList() {
			var permitStr = "",
				forbidStr = "",
				hostname,
				data = {},
				$listTable = $("#qosListAccess").children(),
				length = $listTable.length,
				i = 0;
			//暂无设备时
			if (length == 1 && $listTable.eq(0).children().length < 2) {
				data = {
					permit: "",
					forbid: ""
				}
				return data;
			}
			for (i = 0; i < length; i++) {
				if ($listTable.eq(i).css("display") == "none") { //允许访问
					hostname = $listTable.eq(i).children().eq(0).attr("data-mark");
					permitStr += hostname + "\t";
					if (hostname == $listTable.eq(i).children().eq(0).text()) {
						permitStr += "\t";
					} else {
						permitStr += $listTable.eq(i).children().eq(0).text() + "\t"; //device name
					}
					permitStr += $listTable.eq(i).children().eq(1).text() + "\t"; //mac
					permitStr += "301" + "\t";
					permitStr += "301" + "\t";
					permitStr += "true" + "\n";
				} else { //拒绝访问
					hostname = $listTable.eq(i).children().eq(0).attr("data-mark");

					forbidStr += hostname + "\t"; //device name
					if (hostname == $listTable.eq(i).children().eq(0).text()) {
						forbidStr += "\t";
					} else {
						forbidStr += $listTable.eq(i).children().eq(0).text() + "\t"; //device name
					}
					forbidStr += $listTable.eq(i).children().eq(1).text() + "\t"; //mac
					forbidStr += "0" + "\t";
					forbidStr += "0" + "\t";
					forbidStr += "false" + "\n";
				}
			}
			data = {
				permit: permitStr.replace(/[\n]$/, ""),
				forbid: forbidStr.replace(/[\n]$/, "")
			}
			return data;
		}
	}
	/********END Attached Devices and Blocked Devices ************************/
})