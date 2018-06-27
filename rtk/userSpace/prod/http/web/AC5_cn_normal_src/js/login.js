(function () {
	var LoginView = function (encode) {
		var that = this;
		this.getVal = function (elem) {
			if (elem && $("#" + elem).length > 0) {
				return $("#" + elem).val();
			} else {
				return;
			}
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
			$("#country").html(str);
			$("#country").attr("data-val", B.lang);
		};
		this.addEvent = function () {
			$("#login-password").initPassword(_("Login Password"), true);

			document.onkeydown = function (e) {
				that.enterDown(e);
			};

			$("#save").on("click", function (e) {
				loginLogic.validate.checkAll();
			});

			$("#country .addLang").on("click", function () {
				if ($.isHidden($("#country .dropdown-menu")[0])) {
					$("#country .dropdown-menu").show();
				} else {
					$("#country .dropdown-menu").hide();
				}
			});

			$("#country .dropdown-menu li").on("click", function () {
				var lang = $(this).attr("data-val");
				$("#country").attr("data-val", lang);
				$("#country .dropdown-menu").hide();
				$("#country .lang").html(B.langArr[lang]);
				B.setLang(lang);
				window.location.reload(true);
			});

			$(document).on("click", function (e) {
				var target = e.target || e.srcElement;
				if (target != $("#country .addLang")[0]) {
					$("#country .dropdown-menu").hide();
				}
			});
		};
		this.enterDown = function (events) {

			//解决火狐浏览器不支持event事件的问题。
			var e = events || window.event,
				char_code = e.charCode || e.keyCode;

			if (char_code == 13) { //判断是 “Enter”键
				if (e.preventDefault) { //阻止浏览器默认事件
					e.preventDefault();
				} else {
					e.returnValue = false;
				}
				//LoginLogic.validate.checkAll();
				//that.addSubmitEvent(function () {
				loginLogic.validate.checkAll();
				//});
			}
			return false;
		};
		this.getSubmitData = function () {

			$("#password").val(encode(this.getVal("login-password")));

		};
		this.showInvalidError = function (str) {
			$("#errMsg").html("&nbsp;");
			setTimeout(function () {
				if ($("#login-password_") && $("#login-password_").length > 0) {
					if (!$.isHidden($("#login-password_")[0])) {
						$("#login-password_").focus();
					} else {
						$("#login-password").focus();
					}
				} else {
					$("#login-password").focus();
				}
			}, 400);

			setTimeout(function () {
				$("#errMsg").html(str);
			}, 200);
		}
		this.successCallback = function (str) {
			var num = $.parseJSON(str).errCode || "-1";
			if (num == 0) {
				window.location.href = "./index.html";
			} else {
				that.showInvalidError(_("Password error!"));
			}
		};
		var clickTipsFlag = true;
		this.addSubmitEvent = function (callBack) {

			$("#forgotCaret").on("click", function () {
				if (!clickTipsFlag) {
					return;
				}
				clickTipsFlag = false;
				if ($("#login-caret").hasClass("active")) {
					$(".forget-info").fadeIn(300, function () {
						clickTipsFlag = true;
					});
					$("#login-caret").removeClass("active");
				} else {
					$(".forget-info").fadeOut(300, function () {
						clickTipsFlag = true;
					});
					$("#login-caret").addClass("active");
				}
			});
		};

	};

	var LoginSubmit = function () {
		this.login = function (obj) {
			$.ajax({
				url: obj.url,
				type: "POST",
				data: obj.subData,
				success: obj.successCallback,
				error: obj.errorCallback
			});
		}
	}

	function LoginLogic(view, submit) {
		var that = this;
		this.init = function () {
			var errCode = location.href.split("?")[1] || "0";
			switch (errCode) {
			case "1":
				$("#loginPwdWrap").addClass("shake");
				if ($("#loginLangWrap").length > 0) {
					$("#loginLangWrap").addClass("shake");
				}
				view.showInvalidError(_("Password error!"));
				break;
			case "2": //有登录密码&&登录用户已满4个
				view.showInvalidError(_("Up to 4 administrators have logged in."));
				$("#loginWrap").addClass("fly-in");
				break;
			case "3": //无登陆密码登陆&&登录用户已满4个
				$("#loginWrap").addClass("none");
				$("#overUserWrap").removeClass("none").addClass("fly-in");
				break;
			default:
				$("#loginWrap").addClass("fly-in");
			}
			view.initHtml();
			view.addEvent();
			this.addValidate();
			view.addSubmitEvent(function () {
				that.validate.checkAll();
			});
			var lang = B.getLang();
			$("#country").val(lang);
			setTimeout(function () {
				if ($("#login-password_") && $("#login-password_").length > 0) {
					if (!$.isHidden($("#login-password_")[0])) {
						$("#login-password_").focus();
					} else {
						$("#login-password").focus();
					}
				} else {
					$("#login-password").focus();
				}
			}, 400)


		}
		this.addValidate = function () {
			this.validate = $.validate({
				custom: function () {
					var password = view.getVal('login-password');

					function checksValid(password) {
						return password !== '';
					}

					if (!checksValid(password)) {
						return _("Please specify a login password.");
					}
				},

				success: function () {
					view.getSubmitData();
					B.setLang($("#country").attr("data-val"));

					document.forms[0].submit();
				},

				error: function (msg) {
					view.showInvalidError(msg);
				}
			});
		}
	}
	$("#main_content").show();
	var encode = new Encode(),
		loginView = new LoginView(encode),
		loginSubmit = new LoginSubmit(),
		loginLogic = new LoginLogic(loginView, loginSubmit);
	window.loginLogic = loginLogic;
	loginLogic.init();
})();