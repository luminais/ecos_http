$(function () {
	function cloneMAC() {
		var macHost = moduleView.data.statusWAN.macHost;
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
	var pageModule = new PageLogic("goform/getStatus");
	pageModule.initEvent = function () {
		$("#gohome").on("click", function () {
			window.location = "./index.html";
		});
		$("#wan-connect-status").delegate("#cloneMac", "click", cloneMAC);
	};

	pageModule.initValue = function (obj) { //初始化数据
		pageModule.data = obj;
		showWanInternetStatus(obj.statusInternet, "wan-connect-status");
		if (obj.statusInternet.slice(1, 2) == "0") {
			$(".pic-wan").removeClass("pic-wan-error");
		} else {
			$(".pic-wan").addClass("pic-wan-error");
		}
	}

	pageModule.init();
})