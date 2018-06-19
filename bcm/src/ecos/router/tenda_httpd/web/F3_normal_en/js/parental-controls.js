define(function (require, exports, module) {

	var pageModule = new PageLogic("goform/getParentControl", "goform/setParentControl");
	pageModule.modules = [];
	module.exports = pageModule;

	/*************Attached Devices******************/
	var attachedModule = new AttachedModule();
	pageModule.modules.push(attachedModule);

	function AttachedModule() {
		this.init = function () {
			this.initEvent();
		}
		this.initEvent = function () {
			$("#onlineList").delegate(".icon-toggle-off, .icon-toggle-on", "click", changeDeviceManage);

			$("#onlineList").delegate(".icon-edit", "click", editDeviceName);

			$("#onlineList").delegate(".form-control", "blur", function () {
				$(this).next().attr("title", $(this).val());
				$(this).next().text($(this).val());
				$(this).next().show(); //显示设备名称
				$(this).next().next().show(); //显示编辑按钮
				$(this).hide(); //隐藏自身
			});
		};
		this.initValue = function () {
			createOnlineList(pageModule.data.onlineList); //生成在线列表
		};


		this.checkData = function () {
			var deviceName = "",
				$listTable = $("#onlineList").children(),
				length = $listTable.length,
				$td,
				i = 0;
			for (i = 0; i < length; i++) {
				$td = $listTable.eq(i).children();
				deviceName = $td.find("input").eq(0).val(); //device name
				if (deviceName.replace(/[ ]/g, "") == "") {
					$td.find("input").eq(0).focus();
					return _("Space is not supported in a password!");
				}
			}
			return;
		}
		this.getSubmitData = function () {
			var data = {
				onlineList: getOnlineListData()
			}
			return objToString(data);
		};

		function createOnlineList(arry) {
			var len = arry.length,
				i = 0,
				str = "",
				prop,
				hostname,
				divElem,
				divElem1,
				trElem,
				tdElem;
			$("#onlineList").html("");
			for (i = 0; i < len; i++) {
				trElem = document.createElement("tr");
				for (prop in arry[i]) {
					if (prop != "onlineListMAC" && prop != "onlineListRemark") {
						tdElem = document.createElement("td");
						if (prop == "onlineListIP") {
							tdElem.innerHTML = arry[i][prop];
						} else if (prop == "onlineListHostname") { //handle for device name
							if (arry[i].onlineListRemark != "") {
								hostname = arry[i].onlineListRemark;
							} else {
								hostname = arry[i].onlineListHostname;
							}
							tdElem.innerHTML = '<input type="text" class="form-control none" style="width:66%;" value="">';
							tdElem.getElementsByTagName("input")[0].value = hostname;
							divElem = document.createElement('div');
							divElem1 = document.createElement('div');
							divElem.className = "col-xs-8 span-fixed";

							divElem.setAttribute('title', hostname);
							divElem.setAttribute('alt', arry[i].onlineListMAC);
							divElem.setAttribute('data-mark', arry[i].onlineListHostname);

							if (typeof divElem.innerText != "undefined") {
								divElem.innerText = hostname;
							} else { //firefox
								divElem.textContent = hostname;
							}

							tdElem.appendChild(divElem);
							divElem1.className = "col-xs-2 ";
							divElem1.innerHTML = '<span class="ico-small icon-edit" title="' + _("Edit") + '">&nbsp;</span>';
							tdElem.appendChild(divElem1);

						} else if (prop == "onlineListLimitEn") {
							if (arry[i][prop] == "true") {
								tdElem.innerHTML = "<div class='switch icon-toggle-on'></div>";
							} else {
								tdElem.innerHTML = "<div class='switch icon-toggle-off'></div>";
							}
						} else if (prop == "onlineListConnectTime") {
							tdElem.innerHTML = formatSeconds(arry[i][prop]);

						}
						trElem.appendChild(tdElem);
					}
				}
				document.getElementById("onlineList").appendChild(trElem);
			}
			top.mainLogic.initModuleHeight();
		}

		function getOnlineListData() {
			var str = "",
				i = 0,
				listArry = $("#onlineList").children(),
				len = listArry.length,
				hostname;
			for (i = 0; i < len; i++) {
				hostname = $(listArry).eq(i).children().find("div").eq(0).attr("data-mark");
				str += hostname + "\t"; //主机名
				if (hostname == $(listArry).eq(i).children().find("input").val()) {
					str += "\t";
				} else {
					str += $(listArry).eq(i).children().find("input").val() + "\t"; //备注
				}
				str += $(listArry).eq(i).children().find("div").eq(0).attr("alt") + "\t"; //mac
				str += $(listArry).eq(i).children().eq(1).html() + "\t"; //ip
				str += $(listArry).eq(i).children().eq(3).find("div").hasClass("icon-toggle-on") + "\n";
			}
			str = str.replace(/[\n]$/, "");
			return str;
		}

		function editDeviceName() {
			var deviceName = $(this).parent().prev("div").text();
			$(this).parent().parent().find("div").hide();
			$(this).parent().parent().find("input").show();
			$(this).parent().parent().find("input").val(deviceName);
			$(this).parent().parent().find("input").focus();
		}

		function changeDeviceManage() {
			var className = this.className || "icon-toggle-on";
			if (className == "switch icon-toggle-on") {
				this.className = "switch icon-toggle-off";
			} else {
				this.className = "switch icon-toggle-on";
			}
		}

	}
	/*************END Attached Devices************************/

	/*************Access Restrictions*******************/

	var restrictionModule = new RestrictionModule();
	pageModule.modules.push(restrictionModule);

	function RestrictionModule() {
		this.init = function () {
			this.initHtml();
			this.initEvent();
		}

		this.initHtml = function () {
			var hourStr = "",
				minStr = "",
				i = 0;
			for (i = 0; i < 24; i++) {
				hourStr += "<option value='" + ("100" + i).slice(-2) + "'>" + ("100" + i).slice(-2) + "</option>";
			}

			$("#startHour, #endHour").html(hourStr);

			for (i = 0; i < 60; i++) {
				if (i % 5 === 0) {
					minStr += "<option value='" + ("100" + i).slice(-2) + "'>" + ("100" + i).slice(-2) + "</option>";
				}
			}
			$("#startMin, #endMin").html(minStr);
		};

		this.initEvent = function () {
			$("[id^=day]").on("click", clickTimeDay);
			$("#addUrl").on("click", addUrlList);
			$("#urlList").delegate(".ico", "click", deUrlList);
			$("#parentCtrlURLFilterMode").on("change", changeUrlMode);
		}
		this.initValue = function () {
			var obj = pageModule.data;
			translateDate(obj.parentCtrl.parentCtrlOnlineDate);

			oldDate = obj.parentCtrl.parentCtrlOnlineDate;
			var time = obj.parentCtrl.parentCtrlOnlineTime.split("-");
			$("#startHour").val(time[0].split(":")[0]);
			$("#startMin").val(time[0].split(":")[1]);
			$("#endHour").val(time[1].split(":")[0]);
			$("#endMin").val(time[1].split(":")[1]);

			$("#parentCtrlURLFilterMode").val(obj.parentCtrl.parentCtrlURLFilterMode);
			createUrlList(obj.urlFilter.urlFilterList); //生成URLlist
			changeUrlMode();
		}
		this.checkData = function () {
			var date = getScheduleDate();
			if (date == "00000000") {
				return _("Select one day at least.");
			}
			return;
		};
		this.getSubmitData = function () {
			var time = time = $("#startHour").val() + ":" + $("#startMin").val() + "-" +
				$("#endHour").val() + ":" + $("#endMin").val();
			var data = {
				parentCtrlOnlineTime: time,
				parentCtrlOnlineDate: getScheduleDate(),
				parentCtrlURLFilterMode: $("#parentCtrlURLFilterMode").val(),
				urlList: getUrlListData()
			}

			return objToString(data);
		}

		/*********获取定时重启日期字符串***********/
		function getScheduleDate() {
			var i = 0,
				len = 8,
				str = "";
			for (i = 0; i < len; i++) {
				if ($("#day" + i)[0].checked) {
					str += "1";
				} else {
					str += "0";
				}
			}
			return str;
		}

		var oldDate; /******保存初始化日期******/

		/**********点击everyday**********/
		function clickTimeDay() {
			var dataStr = getScheduleDate();
			if (this.id == "day0") { //点击everyday
				if (this.checked) {
					translateDate("11111111");
				} else {
					translateDate("00000000");
				}
			} else {
				if (dataStr.slice(1) == "1111111") {
					translateDate("11111111");
				} else {
					translateDate("0" + dataStr.slice(1));
				}
			}
		}

		/*******根据字符串改变日期的选择*******/
		function translateDate(str) {
			var dayArry = str.split(""),
				len = dayArry.length,
				i = 0;
			for (i = 0; i < len; i++) {
				$("#day" + i)[0].checked = true ? dayArry[i] == 1 : 0;
			}
		}


		function changeUrlMode() {
			var urlMode = $("#parentCtrlURLFilterMode").val();
			if (urlMode != "disable") {
				$("#urlFilterWrap").show();
			} else {
				$("#urlFilterWrap").hide();
			}

			mainLogic.initModuleHeight();
		}



		/*******handle URL*****/
		function CheckUrlVolidate(str_url) {
			/*var strRegex = "^((https|http|ftp|rtsp|mms)?://)" //
				+ "?(([0-9a-z_!~*'().= $%-] : )?[0-9a-z_!~*'().= $%-] @)?" //ftp的user@
				+ "(([0-9]{1,3}\.){3}[0-9]{1,3}" // IP形式的URL- 199.194.52.184
				+ "|" // 允许IP和DOMAIN（域名）
				+ "([0-9a-z_!~*'()-] \.)*" // 域名- www.
				+ "([0-9a-z][0-9a-z-]{0,61})?[0-9a-z]\." // 二级域名
				+ "[a-z]{2,6})" // first level domain- .com or .museum
				+ "(:[0-9]{1,4})?" // 端口- :80
				+ "((/?)|" // a slash isn't required if there is no file name
				+ "(/[0-9a-z_!~*'().;?:@= $,%#-] ) /?)$";
			var re = new RegExp(strRegex);*/
			var re = /^([A-Za-z]+:\/\/)?[A-Za-z0-9-_]{2,}(\.[A-Za-z0-9-_%&?\/.=])?/;
			//(/^([A-Za-z]+:\/\/)?[A-Za-z0-9-_]+(\.[A-Za-z0-9-_%&?\/.=])?/).test("https://wwwcom:8080")
			//re.test()
			if (re.test(str_url)) {
				return (true);
			} else {
				return (false);
			}
		}

		function addUrlList() {
			var url = $('#urlFilterAllow').val(),
				len = $("#urlList").children().length,
				i = 0;
			if (url == "") {
				$('#urlFilterAllow').focus();
				mainLogic.showModuleMsg(_("Please input a key word of domain name!"));
				return;
			}
			if (!CheckUrlVolidate(url)) {
				$('#urlFilterAllow').focus();
				mainLogic.showModuleMsg(_("Please input a key word of domain name!"));
				return;
			}
			var trList = $("#urlList").children();
			for (i = 0; i < len; i++) {
				if (url == trList.eq(i).children().eq(1).text()) {
					$('#urlFilterAllow').focus();
					mainLogic.showModuleMsg(_("This is used. Try another."));
					return;
				}
			}

			if (len >= 16) {
				mainLogic.showModuleMsg(_("Up to %s entries can be added.", [16]));
				return;
			}

			var str;
			str += "<tr>";
			str += "<td align='center'>" + (len + 1) + "</td>";
			str += "<td>" + url + "</td>";
			str += '<td align="center"><div class="ico icon-minus-circled text-primary"></div></td>';
			$("#urlList").append(str);
			top.mainLogic.initModuleHeight();

		}

		function deUrlList() { //删除URLlist
			$(this).parent().parent().remove();
			top.mainLogic.initModuleHeight();
		}

		function getUrlListData() { //获取URL提交数据
			var str = "",
				i = 0,
				listArry = $("#urlList").children(),
				len = listArry.length;
			for (i = 0; i < len; i++) {
				str += $(listArry).eq(i).children().eq(1).text() + "\n";
			}
			str = str.replace(/[\n]$/, "");
			return str;
		}

		function createUrlList(arry) { //生成URL 列表
			var i = 0,
				len = arry.length,
				str = "";
			for (i = 0; i < len; i++) {
				str += "<tr>";
				str += "<td align='center'>" + (i + 1) + "</td>";
				str += "<td>" + arry[i] + "</td>";
				str += '<td align="center"><div class="ico icon-minus-circled text-primary"></div></td>';
			}
			$("#urlList").html(str);
		}
	}
	/**************EDN Access Restrictions***************************/
})