(function($) {
var G = {},
	data,
	nets,
	supPwdFormat = "";
	
	function sortDataBySSID(data) {
		data = data || [];
		return data.sort(function (x, y) {
			var ret = false;
			//x = x['s-ssid'].toLowerCase() || 'z';
			//y = y['s-ssid'].toLowerCase() || 'z';
			  x = Math.abs(x['s-signal']) || 100;
			  y = Math.abs(y['s-signal']) || 100;
			try {
				//return x.charCodeAt(0) - y.charCodeAt(0);
				return x - y;
			} catch (ex) {
				return ret;
			}
		});
	}
	
	function refreshTable() {
		$('#refreshTable').hide();
		$('#loading').show();
		$.getJSON('/goform/WirelessRepeatInit?' + Math.random(), function(obj) {
			if(this.responseText.indexOf("!DOCTYPE ") !== -1) {
				window.location.reload();
				return;
			}
			data = obj;
			nets = sortDataBySSID(data['networks']);
			newTable();
			$('#loading').hide();
			$('#refreshTable').show();
		});
	}
	
	function getData() {
		$.getJSON('/goform/WirelessRepeatApInit?' + Math.random(), function(obj) {
			if(this.responseText.indexOf("!DOCTYPE ") !== -1) {
				window.location.reload();
			}
			
			data = obj;
			nets = sortDataBySSID(data['networks']);
			if (data['repeat-mode'] !== '0') {
				$('#refreshTable').hide();
				$('#loading').show();
				setTimeout(refreshTable, 1000);
			}
			
			initHtml();
		});
	}
	
	function newTable() {
		var thead = "<thead><tr> " +
			"<th width='30px'>选择</th>" +
			"<th width='210px'>无线信号名称</th>" +
			"<th width='110px'>MAC 地址</th>" +
			"<th width='30px'>信道</th>" +
			"<th width='120px'>安全</th>" +
			"<th width='74px'>信号强度</th>" +
			"</tr></thead>",
			tr = "",
			tbody = "",
			len = nets.length,
			ssId,
			imgClass,
			nets_enc,
			signalNum,
			signalPercent,
			i;

		for (i = 0; i < len; i++) {
			signalNum = nets[i]['s-signal'];
			ssId = nets[i]['s-ssid'];

			if (signalNum > 0) {
				signalNum = -signalNum;
			}
			signalPercent = translatSignal(signalNum);

			if (signalNum <= -70) {
				imgClass = "icons icons-signal-1";
			} else if (signalNum <= -55) {
				imgClass = "icons icons-signal-2";
			} else if (signalNum <= -45) {
				imgClass = "icons icons-signal-3";
			} else if (signalNum <= -30) {
				imgClass = "icons icons-signal-4";
			} else {
				imgClass = "icons icons-signal-5";
			}

			if (nets[i]['s-encrypt'] == 'NONE') {
				nets_enc = "未加密";
			} else {
				nets_enc = nets[i]['s-encrypt'];
			}
			tr = '<tr><td><input type="radio" name="selectednet" value=' + i + '></td>' +
				'<td class="fixed ssId" title="' + ssId + '">' + ssId + '</td>' +
				'<td>' + nets[i]['s-mac'] + '</td>' +
				'<td>' + nets[i]['s-channel'] + '</td>' +
				'<td>' + nets_enc + '</td>' +
				'<td>' + '<a class="' + imgClass + '" style="float:left;"></a><div style="margin-top:3px;">' + signalPercent + '</div></td>' +
				'</tr>';
			tbody += tr;
		}
		$('#networks').html('<table class="table table-fixed" id="scan-table">' + thead + tbody + "</table>");
		//addTitle();

		$('#scan-table input').on('click', function () {
			var ssidIndex = this.value;
			selectSingal(ssidIndex);
		});
	}

	function translatSignal(percent) {
		var newPer = 0;
		if (percent >= -30) {
			newPer = "100%";
		} else if (percent >= -45) {
			newPer = Math.round(100 - (-30 - percent) / 1.5) + "%";
		} else if (percent >= -55) {
			newPer = 90 - (-45 - percent) * 2 + "%";
		} else if (percent >= -70) {
			newPer = 70 - (-55 - percent) * 2 + "%";
		} else if(percent >= -85) {
			newPer = 40 - (-70 - percent) * 2 + "%";
		} else if(percent >= 95) {
			newPer = 10 - (-85 - percent) + "%"
		} else {
			newPer = "0%"
		}
		return newPer;
	}

	function fillDataById() {
		var id,
			elem;
		
		for(id in data) {
			if(data.hasOwnProperty(id) && (elem = document.getElementById(id))) {
				elem.value = data[id];
			}
		}
	}
	
	function showElem($elem, show) {
		if (show) {
			$elem.show();
		} else {
			$elem.hide();
		}
	}
	
	function initHtml() {
		var id,
			elem,
			$contentdiv = $('#contentdiv'),
			$ssidInfo = $('#ssidInfo'),
			$ssidElem = $('#ssid-text'),
			modeIndex = parseInt(data['repeat-mode'], 10);
		
		document.getElementsByName('repeat-modes')[modeIndex].checked = true;
		
		showElem($contentdiv, modeIndex !== 0);
		showElem($('#pwdiv'), data['super-pwd'] && data['super-dencrypt'] !== 'NONE');
		showElem($ssidInfo, data['super-ssid'] === "");	
		if(data['super-ssid'] !== "") {
			$ssidElem.show().html(data['super-ssid']);
		}
		
		fillDataById(data);
		document.getElementById('super-pwd').value = decodeSSID(data['super-pwd']);
		$('#super-pwd').addPlaceholder('请输入上级设备的无线密码');
		
		if(+data["wifi-power"]) {
			 $(".icons-signal").parent().removeClass("none");	
		} else {
			$(".icons-signal").parent().addClass("none");
		}

		if (data["connect-status"] == "0") {
			$("#connectStatus").html("中继成功");
			$("#connectStatus")[0].className = "text-success";
		} else if (data["connect-status"] == "1") {
			$("#connectStatus").html("未连接");
			$("#connectStatus")[0].className = "text-error";
		} else if (data["connect-status"] == "2") {
			$("#connectStatus").html("密码错误");
			$("#connectStatus")[0].className = "text-error";
		} else {
			$("#connectStatus").html("未连接");
			$("#connectStatus")[0].className =  "text-error";
		}
	}

	function initContentdiv(selected) {
		selected = parseInt(selected, 10);
		
		if(selected == 0) {
			$('#contentdiv').hide();
		} else {
			if (selected == data['repeat-mode']){
				initHtml();
			} else {
				$('#contentdiv').show();
				$('#ssidInfo').show();
				$('#ssid-text').hide().html('');
				$('#pwdiv').hide();
				$('#superpwd').val("");
			}
			refreshTable();
		}
	}
		
	function addTitle() {
		if($('.ssId')) {
			var ssids = $('.ssId'),
				len = ssids.length,
				i;
				
			for(i = 0; i< len ; i++) {
				ssids[i].title = ssids[i].innerHTML;
			}
		}
	}
	
	function selectSingal(index) {
		var selIdx = parseInt(index,10),
			dom_pwdiv = document.getElementById('pwdiv'),
			dom_ssidinfo = document.getElementById('ssidInfo'),
			dom_ssid = document.getElementById('ssid-text'),
			dom_superpwd = document.getElementById('super-pwd'),
			selNet = data['networks'][selIdx],
			name,
			id,
			elem;

		if (Math.abs(selNet["s-signal"]) > 70) {
			alert("当前选择的无线信号较弱，可能导致中继不稳定，建议调整路由器摆放位置");
		}

		for (name in selNet) {
			id = name.replace('s-', 'super-');
			if(elem = document.getElementById(id)) {
				elem.value = selNet[name];
			}
		}
		
		dom_ssidinfo.style.display="none";
		dom_ssid.style.display = "";
		dom_ssid.innerHTML = selNet['s-ssid'];
		
		dom_superpwd.value = "";
		
		window.scroll(0, 0);
		if (document.documentElement) {
			document.documentElement.scrollTop;
		}
		
		
		if (selNet['s-encrypt'] !== "NONE") {
			dom_pwdiv.style.display = "";
			if (selNet['s-encrypt'] =="WEP") {

				supPwdFormat = "WEP";
				//$("#super-pwd").attr("data-options", "");
			} else {
				supPwdFormat = "other";
				//$("#super-pwd").attr("data-options", '{"type":"ssidPasword","args":[8, 63]}');
			}
			dom_superpwd.focus();
		} else {
			dom_pwdiv.style.display = "none";
		}
	}
	
	//submit
	function submit() {
		var repeatMode = getValueByName('repeat-modes'),
			superSsidElem = document.getElementById('ssid-text'),
			superPwd = document.getElementById('super-pwd').value,
			superEncrypt = document.getElementById('super-encrypt').value,
			encryptType = superEncrypt.split('/')[0],
			encrypRule = superEncrypt.split('/')[1] || "",
			superChannel = document.getElementById('super-channel').value,
			superMac = document.getElementById('super-mac').value,
			submitStr = "",
			superSsid = '';
		
		if(repeatMode == 0 && repeatMode === data['repeat-mode']) {
			return;
		}
		if (superSsidElem.innerText) {
			superSsid = superSsidElem.innerText.trim();
		} else if (superSsidElem.textContent) {
			superSsid = superSsidElem.textContent.trim();
		}
		if (superSsid === '' && repeatMode != 0) {
			alert('请选择无线信号！');
			return false;
		}

		if (supPwdFormat == "WEP") {
		//请输入5位、13位ASCII码，或者10位、26位十六进制数；
			var asciiReg = /^[ -~]{5}([ -~]{8})?$/g,
				hexReg = /^[0-9a-fA-F]{10}([0-9a-fA-F]{16})?$/g;

			if (!asciiReg.test(superPwd) && !hexReg.test(superPwd)) {
				alert("请输入正确的上级无线信号密码！");
				document.getElementById("super-pwd").focus();
				return false;
			}
		} else if (supPwdFormat == "other") {
			var asciiReg = /^[ -~]{8,63}$/g;
			if (!asciiReg.test(superPwd)) {
				alert("请输入非中文字符（8-63字符）");
				document.getElementById("super-pwd").focus();
				return false;
			}
			
		}

		if (encryptType === "NONE") {
			superPwd = '';
		}
		
		if(repeatMode == 0) {
			submitStr = "repeat-mode=0";
		} else {
			submitStr = "repeat-mode=" + encodeURIComponent(repeatMode) +
				"&super-ssid=" + encodeURIComponent(superSsid) +
				"&super-pwd=" + encodeURIComponent(superPwd) +
				"&super-encrypt=" + encodeURIComponent(encryptType) +
				"&super-encrule=" + encodeURIComponent(encrypRule) + 
				"&super-channel=" + encodeURIComponent(superChannel) +
				"&super-mac=" + encodeURIComponent(superMac);
		}
		
		if(!window.confirm("路由器将重启使设置生效！")) {
			return false;
		}
		
		G.ajaxMsg = $.ajaxMassage('数据保存中，请稍候...');
		$('#submit')[0].disabled = 'disabled';
		$.post('/goform/WirelessRepeatSave', submitStr, function (req){
			var msg = req.responseText;
			
			if(msg.indexOf("!DOCTYPE ") !== -1) {
				window.location.reload();
				return;
			}
			
			if (msg === "-188") {
				setTimeout(function () {
					getData();
					G.ajaxMsg.text("数据保存成功！");
					$('#submit')[0].disabled = '';
					setTimeout(function () {
						G.ajaxMsg.hide();
					}, 500);
				}, 1200);
			} else {
				G.ajaxMsg.text("数据保存成功！");
				setTimeout(function () {
					$('#submit')[0].disabled = '';
					G.ajaxMsg.hide();
				}, 500);
				progressStrip(location.href.replace('wireless-repeaters.htm', 'index.htm'), 220, '系统重启中，请稍候...');
			}
		});
	}
	
	function initControl() {
		$('#type-select input').on('click',function() {
			initContentdiv(this.value);
		});
		
		G.validate = $.validate({
			success: submit,
			
			error: function (msg) {
				if (msg && typeof msg === 'string') {
					alert(msg);
				}
			}
		});
		
		//refresh
		$('#refreshTable').on('click', function() {
			refreshTable();
		});
		
		//Submit
		$('#submit').on('click', function() {
			G.validate.checkAll();
		});
		
		//Cancel
		$('#cancel').on('click', function() {
			window.location.reload();
		});
	}
	$(function() {
		getData();
		initControl();
	});
}($));