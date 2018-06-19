(function (window, document, $) {
"use strict";
	var G = {
			ALT: 18,
			BACKSPACE: 8,
			CAPS_LOCK: 20,
			COMMA: 188,
			COMMAND: 91,
			COMMAND_LEFT: 91, // COMMAND
			COMMAND_RIGHT: 93,
			CONTROL: 17,
			DELETE: 46,
			DOWN: 40,
			END: 35,
			ENTER: 13,
			ESCAPE: 27,
			HOME: 36,
			INSERT: 45,
			LEFT: 37,
			MENU: 93, // COMMAND_RIGHT
			NUMPAD_ADD: 107,
			NUMPAD_DECIMAL: 110,
			NUMPAD_DIVIDE: 111,
			NUMPAD_ENTER: 108,
			NUMPAD_MULTIPLY: 106,
			NUMPAD_SUBTRACT: 109,
			PAGE_DOWN: 34,
			PAGE_UP: 33,
			PERIOD: 190,
			RIGHT: 39,
			SHIFT: 16,
			SPACE: 32,
			TAB: 9,
			UP: 38,
			WINDOWS: 91 // COMMAND
		};
	
	function toFixedLengthNum(str, len) {
		var fixedLen = parseInt(len, 10) || 2,
			myStr, fixedNum;

		fixedNum = fixedLen > 0 ? Math.pow(10, fixedLen) : 100;
		
		if(parseInt(str) > fixedNum) {
			return str;
		}
		myStr = (parseInt(str, 10) + fixedNum).toString();
		
		return myStr.substr(1, 1 + fixedLen);
	}
	
	function secondToTimeStr(str) {
		var second = parseInt(str, 10) > 86400 ? 86400 : parseInt(str, 10),
			hour = parseInt(second / 3600, 10),
			minute = parseInt((second % 3600 ) / 60);
		
		return toFixedLengthNum(hour) + ":" + toFixedLengthNum(minute);
	}
	
	function timeStrToSecond(str) {
		// str = "04:00,14:20;16:00,20:12"
		var timeArr = str.split(":");
		
		return timeArr[0] * 3600 + timeArr[1] * 60;
	}
	
	function rolloverTimeStr(str){
		var timeGroup = (str || "0,0").split(";"),
			len = timeGroup.length,
			rolloverFun = secondToTimeStr,
			secondArr, sLen, i, j;
		
		if (timeGroup[1]) {
			timeGroup = [timeGroup[0].split(',')[0] + "," + timeGroup[1].split(',')[1]];
			len = timeGroup.length;
		}
		if (str.indexOf(":") !== -1) {
			rolloverFun = timeStrToSecond;
		}
		
		for (i = 0; i < len; i++) {
			secondArr = timeGroup[i].split(",");
			sLen = secondArr.length;
			
			for (j = 0; j < sLen; j++) {
				secondArr[j] = rolloverFun(secondArr[j] || 0);
			}
			
			timeGroup[i] = secondArr.join(',');
		}
		
		return timeGroup.join(";");
	}
	
	function transformData() {
		G.data["time-interval"] = rolloverTimeStr(G.data["time-interval"]);
	}
	
	function preSubmit() {
		var startTime = $("#stime-1")[0].value + ":" + $("#stime-2")[0].value,
			endTime = $("#etime-1")[0].value + ":" + $("#etime-2")[0].value,
			submitStr = "enable=" + getValueByName("switcher");

		startTime = rolloverTimeStr(startTime);
		endTime = rolloverTimeStr(endTime);
		
		// 固定为每天，为后续扩展留下接口
		
		if($("#stime-1")[0].value == "" || $("#stime-2")[0].value == "" || $("#etime-1")[0].value == "" || $("#etime-2")[0].value == "") {
			alert("请输入正确的时间！");
			return;
		}
		
		submitStr += "&time-round=" + getCheckboxVal("week");
		submitStr += "&time-str=" + getCheckboxValStr("week");
		if (parseInt(startTime, 10) > parseInt(endTime)) {
			submitStr += "&time-interval=" + startTime + ",86400;0," + endTime;
		} else {
			submitStr += "&time-interval=" + startTime + "," + endTime;
		}
		
		G.ajaxMsg = $.ajaxMassage('数据保存中，请稍候...');
		$.post('/goform/SetWifiControl?' + Math.random(), submitStr, function (req){

			if(req.responseText.indexOf('<DOCTYPE') !== -1) {
				window.location.reload();
			}
			setTimeout(function () {
				getData(refreshData)
				G.ajaxMsg.text("数据保存成功！");
				setTimeout(function () {
					G.ajaxMsg.hide();
				}, 500);
			}, 1000);
		});
	}
	
	function getCheckboxVal(name) {
		var checkboxDomArr = $("input[name="+name+"]"),
			checkboxValArr = [],
			checkboxValStr = ""
		for (var i = 0; i < checkboxDomArr.length; i++) {
			if (checkboxDomArr[i].checked) {
				checkboxValStr += "1";
			} else {
				checkboxValStr += "0";
			}
		}
		return checkboxValStr;
	}
	function getCheckboxValStr(name) {
		var flag = 0;
		var checkboxDomArr = $("input[name="+name+"]"),
			checkboxValArr = [],
			checkboxValStr = ""
		for (var i = 0; i < checkboxDomArr.length; i++) {
			if (checkboxDomArr[i].checked) {
				if(flag == 0){
					flag = 1;
					checkboxValStr += checkboxDomArr[i].value;
				}
				else
				{
					checkboxValStr += ","
					checkboxValStr += checkboxDomArr[i].value;
				}
			} 
		}
		return checkboxValStr;
	}
	function initControler() {
		$("#enableCtrl").on('click', function () {
			$("#wifi-control").removeClass("none");
		});
		$("#disableCtrl").on('click', function () {
			$("#wifi-control").addClass("none");
		});
		
		$('.time-hour').on('blur', function() {
			
			if(this.value > 23) {
				this.value = 23;
			} else {
				this.value = /2[0-3]?|[0-1]?[0-9]?/.exec(this.value);
			}
		});
		$('.time-minute').on('blur', function(){
			if(this.value > 59) {
				this.value = 59;
			} else {
				this.value = /[0-5]?[0-9]?/.exec(this.value);
			}

		});
		 $(".timebox").on('keyup', function(){
			if(this.maxLength == this.value.length && 
				this.nextElementSibling){
				this.nextElementSibling.nextElementSibling.focus()
			}
		}); 
 
		$('.timebox').on('keypress', function() {
			allowChars(this, event, 1);
		});
		
		$("input[name=repeat-time]").on("click", function() {
			changeRepeatTime(this.value);
		});
		// 添加数据验证控制器
		G.validate = $.validate({
			custom: function () {},
			success: preSubmit,
			error: function (msg) {
				if (msg && typeof msg === 'string') {
					alert(msg);
				}
			}
		});
		$('#submit').on('click', function () {
			G.validate.checkAll();
		});
		$('#cancel').on('click',function() {
			window.location.reload();
		});
	}
	
	function changeRepeatTime(val) {
		switch (val) {
			case "all":
				$("input[name=week]").each(function(i){
					this.checked = true;
					this.disabled = true;
				});
				break;
			case "appoint":
				$("input[name=week]").each(function(i){
					this.disabled = false;
				});
				break;
		}
	}
	
	
	function allowChars(oTextbox, oEvent, bBlockPaste) {
		var my_regexp,
			target,
			sValidChars = (oTextbox.getAttribute("validchars")).replace(/^\s+|\s+$/g,"");//trim
		oEvent = oEvent || window.event;
		target = oEvent.target || oEvent.srcElement;
		oEvent.keyCode = oEvent.charCode || oEvent.keyCode || oEvent.which;

		if (sValidChars.charAt(0) !== '/') {
			my_regexp = eval('/['+sValidChars+']/g')
		} else {
			my_regexp = eval(sValidChars);
		}
		var sChar = String.fromCharCode(oEvent.keyCode);
		//alert(oEvent.keyCode);

		 //Opera浏览器不不支持charCode，
		if (navigator.userAgent.indexOf("Opera") > -1) {
			sChar = String.fromCharCode(oEvent.keyCode); 
		} 

		var bIsValidChar = my_regexp.test(sChar);

		//火狐中oEvent.keyCode == 0为功能键,在Opera浏览器功能键有键值
		if (oEvent.keyCode == 0 || oEvent.keyCode == G.BACKSPACE || oEvent.keyCode == G.LEFT 
			|| oEvent.keyCode == G.RIGHT || oEvent.keyCode == G.DOWN) { 
			bIsValidChar = true;
		} 
		
		/*if (bBlockPaste) {*/
		if(bBlockPaste && !(bIsValidChar && !(oEvent.ctrlKey && sChar == "v")) || !bBlockPaste && !(bIsValidChar || oEvent.ctrlKey)) {
			if(oEvent.preventDefault) {
				oEvent.preventDefault();
				return false;
			} else {
				oEvent.returnValue = false;
				return false;
			}
		}
		/*} else {
			if(!(bIsValidChar || oEvent.ctrlKey)) {
				if(oEvent.preventDefault) {
					oEvent.preventDefault();
				} else {
					oEvent.returnValue = false;
				}
			}
		}*/
		/*************add 数字溢出限制输入处理***********/
		if(target.selectionEnd) {
			var maxVal,
				presentVal = parseInt(target.value, 10),
				charVal = parseInt(sChar, 10),
				endVal;
			if(target.id == "stime-1" || target.id == "etime-1") {
				maxVal = 23;
			} else if(target.id == "stime-2" || target.id == "etime-2" ) {
				maxVal = 59;
			}
			switch(target.selectionEnd - target.selectionStart) {
				case 0:
					if(target.selectionStart == 0) {
						endVal = charVal * 10 + presentVal;
					}else {
						endVal = charVal + presentVal * 10;
					}
					break;
				case 1:
					if(target.value.length == 1) {
						endVal = charVal;
					} else {
						if(target.selectionEnd == 1) {
							endVal = charVal * 10 + presentVal % 10;
						} else {
							endVal = charVal + presentVal - presentVal % 10;
						}
					}
					break;
				default:
					endVal = charVal;
					break;
			}
			if(endVal > maxVal) {
				if(oEvent.preventDefault) {
					oEvent.preventDefault();
					return false;
				} else {
					oEvent.returnValue = false;
					return false;
				}
			}
		}
	}
	
	function initTimeInterval () {
		var timeGroup = G.data["time-interval"],
			startTime = timeGroup.split(',')[0].split(':'),
			endTime = timeGroup.split(',')[1].split(':');
		$("#stime-1").val(startTime[0]);
		$("#stime-2").val(startTime[1]);
		$("#etime-1").val(endTime[0]);
		$("#etime-2").val(endTime[1]);
	}
	
	function initView(data) {
		setValueByName('switcher', G.data.enable);
		var enableCtrlElem = document.getElementById('enableCtrl'),
			enableCtrlLabel = document.getElementById('enableCtrlLabel'),
			val;
	/*	if(G.data.wanMode !== 'ap') {
			enableCtrlElem.disabled = true;
			setValueByName('switcher', 0);
			enableCtrlLabel.title = "如果要打开定时开关，请关闭无线桥接";
		} else {
			enableCtrlElem.disabled = false;
			enableCtrlLabel.title = '';
		}*/
		if ($("#enableCtrl")[0].checked) {
			$("#wifi-control").removeClass("none");
		}
		
		if(+G.data["wifi-power"]) {
			 $(".icons-signal").parent().removeClass("none");	
		} else {
			 $(".icons-signal").parent().addClass("none");
		}	
		
		initTimeInterval(G.data['time-interval']);
		if (G.data["time-round"] == "1111111") {
			val = "all";
			$("#everyday")[0].checked = true;
			$("input[name=week]").each(function(i) {
				
				this.checked = true;
				this.disabled = true;
			});
			
		} else {
			$("#appointday")[0].checked = true;
			val = "appointday";
			var timeRoundArr = G.data["time-round"].split("");
			$("input[name=week]").each(function(i) {
				if (timeRoundArr[i] == "1") {
					this.checked = true;
				} else {
					this.checked = false;
				}
				this.disabled = false;
			});
		}
	}
	
	// 初始化页面
	function init(data){
		G.data = data;
		transformData();
		initView();
		initControler();
	}
	
	// 只刷新页面显示数据
	function refreshData(data) {
		G.data = data;
		transformData();
		initView();
	}
	
	function getData(callBack) {
		
		$.getJSON("/goform/GetWifiControl?" + Math.random(), callBack);
	}
	
	// 文档加载完，图片未加载完时运行
	$(function() {
		getData(init);
	});
}(window, document, $));