var random = Math.random(),
	updateModal = null;
seajs.config({
	map: [
		//防止js文件夹下的文件被缓存
		[/(.*js\/[^\/\.]*\.(?:js))(?:.*)/, '$1?' + "t=" + random]
	]
});

var G_data = {};

function MainLogic() {
	var that = this;
	this.nextPage = "";

	this.init = function () {
		$("body").addClass("index-body");
		this.initHtml();
		this.initEvent();
		that.getValue();
		this.initModuleHeight();
	};

	this.initHtml = function () {
		var supportLang = B.options.support,
			len = supportLang.length,
			str,
			i = 0;
			//eidt by xc 若只支持一种语言，则不显示语言切换按钮
		if(len == 1){
			$("#selectLang").hide();
		}else{
			str = "<div class='dropdown'>";
			str += "<div class='addLang'>";
			str += "<span class='lang'>" + B.langArr[B.lang] + "</span>";
			str += "<span class='caret'></span>";
			str += "</div>";
			str += "<ul class='dropdown-menu'>";
			for (i = 0; i < len; i++) {
				lang = supportLang[i];
				str += "<li data-val='" + lang + "'><a>" + B.langArr[lang] + "</a></li>";
			}
			str += "</ul>";

			str += "</div>";
			$("#selectLang").html(str);
		}

		changeIcon(B.lang);
	};

	function changeLocation() {
		if ($(this).hasClass("icon-weibo")) {
			$(this).attr("href", "http://weibo.com/100tenda");
		} else if ($(this).hasClass("icon-wechat")) {
			$("#weixinWrap").show();
			$(this).attr("href", "javascript:void(0)");
		} else if ($(this).hasClass("icon-facebook")) {
			$(this).attr("href", "https://www.facebook.com/tendatechnology");
		} else if ($(this).hasClass("icon-twitter")) {
			$(this).attr("href", "https://twitter.com/tendasz1999");
		}
	}

	function changeIcon(lang) {
		$("#nav-footer-icon-cn, #nav-footer-icon-multi").addClass("none");
		if (lang == "cn") {
			$("#nav-footer-icon-cn").removeClass("none");

			$('.brand').attr('href', 'http://tenda.com.cn');
		} else {
			$("#nav-footer-icon-multi").removeClass("none");
			$('.brand').attr('href', 'http://tendacn.com');
		}
	}

	this.initEvent = function () {
		var clickTag = "click";
		$("#nav-menu").delegate("li", "click", function () {
			var targetMenu = this.children[0].id || "status";
			that.changeMenu(targetMenu);
		});

		if (window.ontouchstart) { //当某些手机浏览器不支持click事件
			clickTag = "touch";
		}

		$(document).delegate("*", "click", function (e) {
			var target = e.target || e.srcElement,
				clickSetLang;
			if ($(target.parentNode).hasClass('addLang') || $(target.parentNode).attr('id') === "navbar-button") {
				target = target.parentNode;
			}
			if ($(target).attr('id') != "navbar-button") {
				if ($(target).attr('id') != "nav-menu") {
					if (!$(".navbar-toggle").hasClass("none") && $("#nav-menu").hasClass("nav-menu-push")) {
						$("#nav-menu").removeClass("nav-menu-push");
					}
				}
			}


			if ($(target).hasClass("addLang")) {
				clickSetLang = true;
			}

			if (clickSetLang) {
				//$("#selectLang .dropdown-menu").show();
			} else {
				$("#selectLang .dropdown-menu").hide();
			}

		});

		$("#selectLang .addLang").on("click", function () {
			if ($.isHidden($("#selectLang .dropdown-menu")[0])) {
				$("#selectLang .dropdown-menu").show();
			} else {
				$("#selectLang .dropdown-menu").hide();
			}
		});

		//for foreign version
		//$("#nav-footer-icon").delegate(".nav-icon", "mouseover", changeLocation);
		//$("#nav-footer-icon").delegate(".weixin", "mouseout", function () {
		//$("#weixinWrap").hide();
		//});

		$("#selectLang .dropdown-menu li").on("click", function () {
			var lang = $(this).attr("data-val");
			$("#selectLang .dropdown-menu").hide();
			B.setLang(lang);
			window.location.reload(true);
			$("#selectLang .lang").html(B.langArr[lang]);
			changeIcon(lang);
		});

		$(window).resize(this.initModuleHeight);

		$('#submit').on('click', function () {
			that.modules.validate.checkAll();
			$('#submit')[0].blur();
		});

		$("#navbar-button").on("click", function () {
			if (!$("#nav-menu").hasClass("nav-menu-push")) {
				$("#nav-menu").addClass("nav-menu-push");
			} else {
				$("#nav-menu").removeClass("nav-menu-push");
			}
		});

		//Cancel
		$('#cancel').on('click', function () {
			that.modules.reCancel();
		});

		$("#loginout").on("click", function () {
			$.post("goform/loginOut", "action=loginout", function () {
				window.location.href = "./login.html";
			});
		});

		var appManageModal = null;
		$("#appManage").on("click", function () {
			if(appManageModal){
				appManageModal.show();
			}else{
				appManageModal = $.modalDialog({
					title:$("#appDianoseWrapperTitle").text(),
					content:$("#appDianoseWrapper")
				});
			}
			// dialog.open({
			// 	Id: "appDianoseWrapper",
			// 	height: "300px"
			// });

		});
	};

	this.getValue = function () {
		$.getData("goform/getHomePageInfo", "loginAuth,wifiRelay,wpsModule,hasNewSoftVersion", function (obj) {
			G_data = obj;

			if (obj.loginAuth.hasLoginPwd == "true") {
				$(".loginout").show();
			} else {
				$(".loginout").hide();
			}

			var wifiRelayObj = obj.wifiRelay,
				fistMenu = "status";

			switch (wifiRelayObj.wifiRelayType) {
			case "disabled"://路由
			case "wisp":
				$("#userManageWrap").remove();
				break;
			case "client+ap":
			case "ap":
				$("#netCtrNavWrap").remove();
				$("#parentalCtrNavWrap").remove();
				break;
			}
			that.changeMenu(fistMenu);
			onineUpgradeLogic.init("homePage", obj);
		});
	};
	//切换菜单栏
	this.changeMenu = function (id) {
		var nextUrl = id,
			mainHtml, name,
			$iframe = $(iframe);

		//edit by xc 菜单切换时，若两次点击的是同一个按钮这不做任何操作；
		/*if($iframe.attr("data-active") == id) {
			loadCallBack();
			return;
		};*/

		$iframe.addClass("none").attr("data-active",id);
		$iframe.load("./" + nextUrl + ".html?" + random, function () {
			loadCallBack();
		});
		$("#nav-menu").removeClass("nav-menu-push");
		$("li>.active").removeClass("active");
		$("li span").removeClass("active");

		$("#" + id + " span").addClass("active");
		$("#" + id).addClass("active");


		function loadCallBack(){
			if ($("#iframe").find("meta").length > 0) {
				top.location.reload(true);
				return;
			}
			if (id == "status") {
				$("#submit").addClass("none");
				$("#cancel").addClass("none");
			} else {
				$("#submit").removeClass("none");
				$("#cancel").removeClass("none");
			}

			seajs.use("./js/" + nextUrl, function (modules) { //加载模块所需函数
				//翻译
				B.translatePage();
				$("#iframe").removeClass("none");
				modules.init(); //模块初始化
				$("#submit").css("cursor", "pointer");
				if (that.modules && that.modules != modules) { //判断前一个模块是否是当前模块
					if (typeof that.modules.upgradeLoad == "object" || typeof that.modules.inport == "object") { //解决ajaxupload 影响高度问题
						//if (that.modules.upgradeLoad._input == "object") {
						$("[name='upgradeFile']").parent().remove();
						$("[name='inportFile']").parent().remove();
						//}
					}

					//模块切换之后，修改模块运行标志
					that.modules.pageRunning = false;
					$.validate.utils.errorNum = 0; //切换页面时清空数据验证错误
				}
				that.modules = modules; //保留当前运行模块
				that.initModuleHeight();
			});
		}

	};
	this.initModuleHeight = function () {
		var viewHeight = $.viewportHeight(),
			menuHeight = $("#sub-menu").height(),
			mainHeight = $("#iframe").height(),
			height,
			minHeight;
		minHeight = Math.max(menuHeight, mainHeight);
		if (minHeight < (viewHeight - 110)) {
			$("#nav-menu").css('min-height', minHeight + "px");
			$("#main-content").css('min-height', minHeight + "px");
		} else {
			$("#nav-menu").css('min-height', minHeight + 40 + "px");
			$("#main-content").css('min-height', minHeight + 40 + "px");
		}

		height = mainHeight;
		if (height >= viewHeight - 110) {
			height = height - 110;
		} else {
			height = viewHeight - 110;
		}

		if (minHeight > height) {
			height = minHeight;
		}

		$("#nav-menu").css('height', height + 40 + "px");
		$("#main-content").css('height', height + 30 + "px");
	};

	this.showMsgTimer = null;

	this.showModuleMsg = function (text, showTime) {
		var msgBox = $('#form-massage'),
			time;
		msgBox.html(text).fadeIn(10);

		clearTimeout(that.showMsgTimer);
		//0 表示不消失
		if (showTime != 0) {
			time = showTime || 2000;
			that.showMsgTimer = setTimeout(function () {
				msgBox.fadeOut(700);
			}, time);
		}
	};
	this.hideModuleMsg = function () {
		$("#form-massage").fadeOut(10);
	};
}

function OnineUpgradeLogic() {
	var upgradeErrorArr = [_("Unknown"), _("Unknown error"), _("JSON is too long."), _("Malloc failure. No memory is available."), _("Connecting to UCloud failed."), _("Socket error."), _("Socket selection timed out."), _("Common error, which is usually reported when executing a command in an internal process fails."), _("Incorrect module or command."), _("The command is not registered with UCloud."), _("Parsing data failed."), _("Detecting data failed."), _("Parsing, creating, or detecting data packets failed."), _("Internal UCloud connection process failure."), _("Connecting to the server failed."), _("The command for connecting to the UCloud server is incorrect."), _("Server authentication failure."), _("UCloud-based management is disabled."), _("Connecting to the server failed."), _("UCloud is being upgraded or a speed test is in process."), _("Connecting to the server..."), _("No more server can be bounded to this account.")];

	var that = this;
	that.upgradeStatusTimer = null;
	that.hasBindEvent = false;

	this.init = function (page, data) {
		//值的重置
		that.menuPage = page;
		that.timeFlag = 0; //用来记录向CGI定时请求的次数。
		that.nextPage = "latestNewVersion";
		that.hasNewSoftVersion = {};
		that.onlineUpgradeReady = {};
		clearInterval(that.upgradeStatusTimer);
		that.clickedUpgradeBtnImmediatelyBtn = false;
		clearInterval(that.getNewSoftVersionTimer);
		//每隔5s向后台发一次请求，检查是否有软件更新
		that.getNewSoftVersionTimer = setInterval(function () {
			$.getData("goform/getHomePageInfo?" + Math.random(), "hasNewSoftVersion", function (obj) {
				that.timeFlag++;
				hasNewSoftVersionInit(obj);

				//当向后台请求的次数大于或等于2次时，停止向后台发请求；设置这个请求的原因是，在快速设置页面设置PPPOE拨号后，进行主配置页面时，有可能5~10s内才能获取到软件更新的信息。
				if (that.timeFlag >= 2) {
					clearInterval(that.getNewSoftVersionTimer);
				}
			});
		}, 5000);

		hasNewSoftVersionInit(data);
		/**
		 * @method [根据从后台获取到的值判断是否有更新软件]
		 * @param  {Object} [从后台获取到的hasNewSoftVersion模块参数]
		 */
		function hasNewSoftVersionInit(obj) {

			//模块参数先保存下来
			that.hasNewSoftVersion = obj.hasNewSoftVersion;

			//首页如果没有新的升级软件则不提示更新；
			if (page == "homePage" && that.hasNewSoftVersion.hasNewSoftVersion == "false") {
				return;
			}
			//定时刷新时，如果获取到有最新软件则不再发页面请求。
			clearInterval(that.getNewSoftVersionTimer);

			//打开弹出框
			// dialog.open({
			// 	Id: "upgradeDianoseWrapper",
			// 	height: "540px"
			// });

			 if(updateModal){
			 	updateModal.show();
			 }else{
				updateModal = $.modalDialog({
					title:$("#upgradeDianoseWrapperTitle").text(),
					content:$("#upgradeDianoseWrapper"),
					width:600
				});
			 }

			// var dialogHeight = $.viewportHeight() * 0.85 - 60;
			// $("#upgradeDianoseWrapper .dialog-container").css("height", dialogHeight + "px");
			//有版本更新
			if (that.hasNewSoftVersion.hasNewSoftVersion == "true") {
				that.nextPage = "hasNewVersionRemind";
				var str = '<ul id="newVersionOptimize" style="height:120px;overflow-y:auto;">';
				newVersionOptimizeArr = that.hasNewSoftVersion.newVersionOptimize;
				for (var i = 0; i < newVersionOptimizeArr.length; i++) {
					str += "<li>" + newVersionOptimizeArr[i] + "</li>";
				}
				str += "</ul>";

				//首页时显示不再提示功能
				if (page == "homePage") {
					str += '<label for="noPrompt" style="margin-left:25px;"><input type="checkbox" name="noPrompt" id="noPrompt"/>' + _('Never remind me of this version.') + '</label>';
				}
				$("#newVersionContent").html(str);
			}

			that.changeDialogPage();


		}

		//防止重复绑定事件；
		if (!that.hasBindEvent) {
			that.initEvent();
			that.hasBindEvent = true;
		}
	};

	this.initEvent = function () {
		$(".notUpgradeBtn").on("click", function () {

			//在主页有“不再提示选项”，则要获取该值并传送给后台
			if (that.menuPage == "homePage") {
				var noPrompt = $("#newVersionContent").find("input[name=noPrompt]")[0].checked ? "on" : "off";
				$.post("goform/setHomePageInfo?" + Math.random(), "module1=noUpgradePrompt&noPrompt=" + noPrompt, function () {
					// dialog.close();
					updateModal.hide();
				});
			} else {
				// dialog.close();
				updateModal.hide();
			}
		});

		$(".upgradeBtnImmediatelyBtn").on("click", that.upgradeImmediately);
	};

	this.changeDialogPage = function () {
		//页面显示内容重置
		$("#curNewVersionMsg, #upgradeConfig, #newVersionContent,#waitDownload,#progress, #upgradeBtnGroup, #upgradeWarn").addClass("none");
		$("#upgradeRemind, #progress-msg, #upgradeErrorMsg").html("");

		switch (that.nextPage) {
			//当前为最新版本
		case "latestNewVersion":
			$("#curNewVersionMsg").removeClass("none");
			break;

			//当前有更新版本提示,并列出新增功能点及修改点
		case "hasNewVersionRemind":
			$("#upgradeConfig, #newVersionContent, #upgradeBtnGroup").removeClass("none");

			$("#upgradeRemind").html('<p class="text-primary text-center" style="font-size:18px; color:#FF6600">' + _('A later version is available.') + '</p><p class="help-block text-center"><span>' + _('Latest Version:') + '</span><span id="updateVersion"></span><span style="margin-left:10px;">' + _('Release Date:') + '</span><span id="updateDate"></span></p><hr />');

			$("#upgradeRemind #updateVersion").html(that.hasNewSoftVersion.updateVersion);
			$("#upgradeRemind #updateDate").html(that.hasNewSoftVersion.updateDate);

			//若点击立即升级，返回失败的错误代码，则显示出来
			if (that.onlineUpgradeReady.upgradeReady == "fail" && that.clickedUpgradeBtnImmediatelyBtn) {
				$("#upgradeErrorMsg").html(upgradeErrorArr[+that.onlineUpgradeReady.upgradeErrorCode]);
			}

			break;

			//等待下载中
		case "newVersionWaitingDownload":
			$("#upgradeConfig, #waitDownload").removeClass("none");

			$("#upgradeRemind").html("<p class='text-center'>" + _("Firmware Upgrade") + "</p>");
			break;

			//进入下载，下载完成后升级并重启。
		default:
			$("#upgradeDianoseWrapper").find(".progress-bar").css("width", "0%");
			$("#upgradeConfig, #progress, #upgradeWarn").removeClass("none");
			$("#upgradeRemind").html("<p class='text-center'>" + _("Firmware Upgrade") + "</p>");
			$("#progress-msg").html("<p class='text-left'></p>");
			that.download();

			break;
		}
	};

	this.upgradeImmediately = function () {

		$("#upgradeBtnImmediatelyBtn").parents(".md-con-body").addClass("no_close_flag");

		clearInterval(that.upgradeStatusTimer);
		that.clickedUpgradeBtnImmediatelyBtn = true;
		that.upgradeStatusTimer = setInterval(function () {
			$.getData("goform/getHomePageInfo?upgrade=immediately", "onlineUpgradeReady", function (obj) {

				that.onlineUpgradeReady = obj.onlineUpgradeReady;
				switch (obj.onlineUpgradeReady.upgradeReady) {
				case "fail": //有新版本，但无法在线升级，返回错误,并停止发请求
					that.nextPage = "hasNewVersionRemind";
					clearInterval(that.upgradeStatusTimer);
					break;
				case "wait": //有新版本，当前需要等待下载
					that.nextPage = "newVersionWaitingDownload";
					break;
				case "true": //有新版本，当前可以进入下载
					that.nextPage = "newVersionDownload";
					break;
				}
				that.changeDialogPage();
			});
		}, 1000);

	};

	this.download = function () {
		var pc = parseInt(that.onlineUpgradeReady.downloadSize / that.onlineUpgradeReady.totalSize * 100, 10);
		$("#upgradeDianoseWrapper").find("#progress-msg").html("<span style='float:left;'>" + _("Downloading... %s%", [pc]) + "</span><span style='float:right'>" + _("Remaining: %s", [formatSeconds(that.onlineUpgradeReady.restTime)]) + "</span>");

		$("#upgradeDianoseWrapper").find(".progress-bar").css("width", pc + "%");
		if (that.onlineUpgradeReady.writing == "true") {
			clearInterval(that.upgradeStatusTimer);
			onlineProgress.init(450);
		}
	};

}

/*动态速度进度条*/
function DynamicProgressLogic() {
	var addr,
		that = this,
		pc = 0,
		criticalVal = 0, //临界值：
		criticalSpeed = 0, //临界速度：get到数据后，定时器将以该速度5s内走完。
		flag = 0;
	this.type = null;
	this.speed = null;
	this.upgradeTime = null;
	this.rebootTime = null;
	this.requestTime = null;

	//str代表提示信息，speed代表自定义进度条速度，type代表进度条类型，address代表数据接口
	this.init = function (type, str, speed, address) {
		addr = address || "";
		//$("#progress-dialog").css("display", "block");
		dialog.open({
			Id: "progressDianoseWrapper",
			height: "300px"
		});

		$("#progress-overlay").addClass("in");
		this.type = type;
		this.speed = speed || 500;
		var rebootMsg = str || _("Restarting... Please wait.");
		this.getCriticalValue();
		//$("#rebootWrap").find("p:eq(0)").html(rebootMsg);
		if (type != "upgrade") {
			$("#upgradeWrap").addClass("none");
			this.reboot();
		} else {
			this.upgrade();
		}

		//在调用进度条后，需要停掉请求软件更新信息的接口，
		//否则在后台起来后，由于此接口还在请求，后台会重定向到易安装页面，就会出现进度条还没跑完，页面就进行了跳转的问题
		if(window.onineUpgradeLogic && onineUpgradeLogic.getNewSoftVersionTimer){
			clearInterval(onineUpgradeLogic.getNewSoftVersionTimer);
			onineUpgradeLogic.getNewSoftVersionTimer = null;
		}
	}

	this.getCriticalValue = function () { //获取临界值
		criticalVal = (1 - 5 / (100 * (that.speed / 1000))) * 100;
	}

	this.reboot = function () {
		that.rebootTime = setTimeout(function () {
			that.reboot();
			pc++;
		}, that.speed);
		if (pc > 100) {
			flag = 0;
			clearTimeout(that.upgradeTime);
			clearTimeout(that.rebootTime);
			if (addr) {
				window.location.href = "http://" + addr;
			} else {
				window.location.reload();
			}
			return;
		}

		//8s之后，发送请求，确保后台已经关闭
		that.requestTime = setTimeout(function () {
			$.get("./login.html", function (data) {

				//此时后台可能已经起来了
				if (typeof data == "string" && pc < criticalVal && flag < 10 && data.indexOf("<!DOCTYPE html>").toString() !== "-1") {

					//连续获取到数据，确保后台已经起来
					if (flag > 5) {

						//计算剩余进度在5s内走完的定时器速度
						criticalSpeed = (5 / (100 - pc + 1)) * 1000;
						that.speed = criticalSpeed;
					}
					flag++;
				}
			});
		}, 10000);

		if (flag == 10) {
			clearTimeout(that.requestTime);
		}


		$("#progressDianoseWrapper").find(".progress-bar").css("width", pc + "%");
		//$("#rebootWrap").find("span").html(pc + "%");
		if (that.type == "restore") {
			str = _("Resetting... %s%", [pc]);
			dialogTitle = _("Restore Factory Settings");

		} else {
			str = _("Rebooting... %s%", [pc]);
			dialogTitle = _("Reboot");
		}
		$("#progressDianoseWrapper").find(".progress-tip").html(str);
		$("#progressDianoseWrapper").find(".dialog-title").html(dialogTitle);

	}

	this.upgrade = function () {
		that.upgradeTime = setTimeout(function () {
			that.upgrade();
			pc++;
		}, 450);
		if (pc > 100) {
			clearTimeout(that.upgradeTime);
			pc = 0;
			that.reboot();
			return;
		}
		$("#progressDianoseWrapper").find(".progress-bar").css("width", pc + "%");
		$("#progressDianoseWrapper").find(".progress-tip").html(_("Upgrading... %s%", [pc]));
		$("#progressDianoseWrapper").find(".dialog-title").html(_("Firmware Upgrade"));
	}
}

function OnlineProgress() {
	var that = this;

	var pc = 0;
	var criticalVal = 0; //临界值
	var criticalSpeed = 0; //临界速度：get到数据后，定时器将以该速度5s内走完。

	var ip;
	var flag = 0;

	this.time = null;

	this.upgradeTime = null;
	this.rebootTime = null;
	this.requestTime = null;

	this.init = function (progressTime, hostip) {
		pc = 0;
		ip = hostip || "";
		this.time = progressTime || 200;
		this.upgrade();
	};
	this.reboot = function () {
		that.rebootTime = setTimeout(function () {
			that.reboot();
			pc++;
		}, that.time);

		if (pc > 100) {
			flag = 0;
			clearTimeout(that.rebootTime);
			if (ip) {
				window.location.href = "http://" + ip;
			} else {
				window.location.reload(true);
			}
			return;
		}

		//10s之后，发送请求，确保后台已经关闭
		that.requestTime = setTimeout(function () {
			$.get("./login.html", function (data) {

				//此时后台可能已经起来了
				if (typeof data == "string" && pc < criticalVal && flag < 10 && data.indexOf("<!DOCTYPE html>").toString() !== "-1") {

					//连续获取到数据，确保后台已经起来
					if (flag > 5) {

						//计算剩余进度在5s内走完的定时器速度
						criticalSpeed = (5 / (100 - pc + 1)) * 1000;
						that.time = criticalSpeed;
					}
					flag++;
				}
			});
		}, 10000);

		if (flag == 10) {
			clearTimeout(that.requestTime)
		}

		$("#upgradeRemind p").text(_("Reboot"));
		$("#upgradeDianoseWrapper").find(".progress-bar").css("width", pc + "%");
		$("#upgradeDianoseWrapper").find("#progress-msg").html(_("Rebooting... %s", [pc]) + "%");
	};

	this.getCriticalValue = function () { //获取临界值
		criticalVal = (1 - 5 / (100 * (that.time / 1000))) * 100;
	}



	this.upgrade = function () {
		that.upgradeTime = setTimeout(function () {
			that.upgrade();
			pc++;
		}, 450);
		if (pc > 100) {
			clearTimeout(that.upgradeTime);
			pc = 0;
			this.getCriticalValue();

			that.reboot();
			return;
		}
		$("#upgradeDianoseWrapper").find(".progress-bar").css("width", pc + "%");
		$("#upgradeDianoseWrapper").find("#progress-msg").html(_("Upgrading... %s", [pc]) + "%");
	}
}

$(function () {
	$("#main_content").show();

	var dynamicProgressLogic = new DynamicProgressLogic();
	window.dynamicProgressLogic = dynamicProgressLogic;

	var onlineProgress = new OnlineProgress();
	window.onlineProgress = onlineProgress;

	var dialog = new Dialog();
	window.dialog = dialog;

	var onineUpgradeLogic = new OnineUpgradeLogic();
	window.onineUpgradeLogic = onineUpgradeLogic;

	var mainLogic = new MainLogic();
	window.mainLogic = mainLogic;
	mainLogic.init();

})
