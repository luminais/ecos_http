function MainLogic() {
	var that = this;
	this.init = function () {
		$("body").addClass("index-body");
		this.initHtml();
		this.initEvent();
		that.changeMenu("status");
		this.initModuleHeight();
	};

	this.initHtml = function () {
		var supportLang = B.options.support,
			len = supportLang.length,
			str,
			i = 0;
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
		changeIcon(B.lang);
	};

	function changeLocation() {
		if ($(this).hasClass("weibo")) {
			$(this).attr("href", "http://weibo.com/100tenda");
		} else if ($(this).hasClass("weixin")) {
			$("#weixinWrap").show();
			$(this).attr("href", "javascript:void(0)");
		} else if ($(this).hasClass("facebox")) {
			$(this).attr("href", "https://www.facebook.com/tendatechnology");
		} else if ($(this).hasClass("twitter")) {
			$(this).attr("href", "https://twitter.com/tendasz1999");
		}
	}

	function changeIcon(lang) {
		if (lang == "cn") {
			$("#iconLeft .nav-icon").attr("class", "nav-icon weibo");
			$("#iconRight .nav-icon").attr("class", "nav-icon weixin");
		} else {
			$("#iconLeft .nav-icon").attr("class", "nav-icon facebox");
			$("#iconRight .nav-icon").attr("class", "nav-icon twitter");
		}
	}

	this.initEvent = function () {
		$("#nav-menu").delegate("li", "click", function () {
			var targetMenu = this.children[0].id || "status";
			that.changeMenu(targetMenu);
		});

		$("#selectLang .addLang").on("click", function () {
			if ($.isHidden($("#selectLang .dropdown-menu")[0])) {
				$("#selectLang .dropdown-menu").show();
			} else {
				$("#selectLang .dropdown-menu").hide();
			}
		});

		$("#nav-footer-icon").delegate(".nav-icon", "mouseover", changeLocation);
		$("#nav-footer-icon").delegate(".weixin", "mouseout", function () {
			$("#weixinWrap").hide();
		});

		$("#selectLang .dropdown-menu li").on("click", function () {
			var lang = $(this).attr("data-val");
			$("#selectLang .dropdown-menu").hide();
			B.setLang(lang);
			window.location.reload(true);
			$("#selectLang .lang").html(B.langArr[lang]);
			changeIcon(lang);
		})

		$(window).resize(this.initModuleHeight);

		$('#submit').on('click', function () {
			that.modules.validate.checkAll();
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
				//window.location.reload(true);
			});
		});
		that.getValue();
	};

	this.getValue = function () {
		$.getJSON("goform/getIsHasLoginPwd" + getRandom(), function (obj) {
			if (obj.hasLoginPwd == "true") {
				$("#loginout").show();
			} else {
				$("#loginout").hide();
			}
		});
	};
	//切换菜单栏
	this.changeMenu = function (id) {
		var nextUrl = id,
			mainHtml;
		$("#iframe").load("./" + nextUrl + ".html", function () {
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
				modules.init(); //模块初始化
				if (that.modules && that.modules != modules) { //判断前一个模块是否是当前模块
					//模块切换之后，修改模块运行标志
					that.modules.pageRunning = false;
					$.validate.utils.errorNum = 0; //切换页面时清空数据验证错误
				}
				that.modules = modules; //保留当前运行模块
				that.initModuleHeight();
			});
		});
		$("#nav-menu").removeClass("nav-menu-push");
		$("li>.active").removeClass("active");
		$("#" + id).addClass("active");

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
			$("#nav-menu").css('min-height', minHeight + 70 + "px");
			$("#main-content").css('min-height', minHeight + 70 + "px");
		}

		height = mainHeight;
		if (height >= viewHeight - 110) {
			height = height - 110;
		} else {
			height = viewHeight - 110;
		}

		$("#nav-menu").css('height', height + 40 + "px");
		$("#main-content").css('height', height + 30 + "px");
	};

	this.showModuleMsg = function (text, showTime) {
		var msgBox = $('#form-massage'),
			time;
		msgBox.html(text).fadeIn(300);

		//0 表示不消失
		if (showTime != 0) {
			time = showTime || 2000;
			setTimeout(function () {
				msgBox.fadeOut(700);
			}, time);
		}
	}
}

function ProgressLogic() {
	var that = this;
	var pc = 0;
	this.type = null;
	this.time = null;
	this.upgradeTime = null;
	this.rebootTime = null;
	var ip;
	this.init = function (str, type, rebootTime, hostip) {
		ip = hostip || "";
		$("#progress-dialog").css("display", "block");
		$("#progress-overlay").addClass("in");
		this.type = type;
		this.time = rebootTime || 200;
		var rebootMsg = str || _("Rebooting...Please wait...");
		$("#rebootWrap").find("p:eq(0)").html(rebootMsg);
		if (type != "upgrade") {
			$("#upgradeWrap").addClass("none");
			this.reboot();
		} else {
			this.upgrade();
		}

	};
	this.reboot = function () {
		that.rebootTime = setTimeout(function () {
			that.reboot();
			pc++;
		}, that.time);
		if (pc > 100) {
			clearTimeout(that.upgradeTime);
			if (ip) {
				window.location.href = "http://" + ip;
			} else {
				window.location.reload(true);
			}
			return;
		}
		$("#rebootWrap").find(".progress-bar").css("width", pc + "%");
		$("#rebootWrap").find("span").html(pc + "%");
	};
	this.upgrade = function () {
		that.upgradeTime = setTimeout(function () {
			that.upgrade();
			pc++;
		}, 200);
		if (pc > 100) {
			clearTimeout(that.upgradeTime);
			pc = 0;
			that.reboot();
			return;
		}
		$("#upgradeWrap").find(".progress-bar").css("width", pc + "%");
		$("#upgradeWrap").find("span").html(pc + "%");
	}
}

$(function () {
	var mainLogic = new MainLogic();
	window.mainLogic = mainLogic;
	mainLogic.init();
	var progressLogic = new ProgressLogic();
	window.progressLogic = progressLogic;
})