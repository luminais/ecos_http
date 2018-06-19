(function (window, document, $) {
	"use strict";
	var G = {};
	
	G.firstInit = true;
	G.ajaxOn = false;
	G.changeView = false;
	var processView = {//to,hide, show, updateCurView
		detect: {
			to_notlinked: {// remove like-parent
				hide: 'quickset-detect'
				,show: 'quickset-wan-notlinked'
			}
			,to_setAdsl: {//未插网线->插网线，画面会从未连接到pppoe、static、dhcp,需额外隐藏掉notlinked画面
				hide: 'quickset-detect,quickset-wan-notlinked'
				,show: 'quickset-adsl,process-adsl-active,process-wlpwd-inactive'
			}
			,to_setStatic: {
				hide: 'quickset-detect,quickset-wan-notlinked'
				,show: 'quickset-static,process-adsl-active,process-wlpwd-inactive'
			}
			,to_setDhcp: {
				hide: 'quickset-detect,quickset-wan-notlinked'
				,show: 'quickset-dhcp,process-adsl-active,process-wlpwd-inactive'
			}
			,to_setWifi: {
				hide: 'quickset-detect,quickset-wan-notlinked'
				,show: 'quickset-wifi,process-adsl-ok,process-wlpwd-active'
			}
		}
		,notlinked: {
			to_detect: {
				hide: 'quickset-wan-notlinked'
				,show: 'quickset-detect'
			}
			,to_setWifi: {
				hide: 'quickset-wan-notlinked'
				,show: 'quickset-wifi,process-adsl-ok,process-wlpwd-active'
			}
		}//process-adsl-active process-wlpwd-inactive  process-adsl-ok   process-wlpwd-active
		,setAdsl: {
			to_setWifi: {
				hide: 'quickset-adsl,process-adsl-active,process-wlpwd-inactive'
				,show: 'quickset-wifi,process-adsl-ok,process-wlpwd-active'
			}
		}
		,setStatic:  {
			to_setWifi: {
				hide: 'quickset-static,process-adsl-active,process-wlpwd-inactive'
				,show: 'quickset-wifi,process-adsl-ok,process-wlpwd-active'
			}
		}
		,setDhcp:  {
			to_setWifi: {
				hide: 'quickset-dhcp,process-adsl-active,process-wlpwd-inactive'
				,show: 'quickset-wifi,process-adsl-ok,process-wlpwd-active'
			}
		}
		,setWifi:  {
			to_saving: {
				hide: 'quickset-wifi,process-adsl-ok,process-wlpwd-active'
				,show: 'quickset-save'
			}
			,to_confirmNoPwd: {
				hide: ''
				,show: 'confirm-no-pwd,gbx_overlay'
			}
		}
		,saving: {
			to_result: {
				hide: 'quickset-save'
				,show: 'quickset-save-result'
			}
		}
		,confirmNoPwd:  {
			to_saving: {
				hide: 'quickset-wifi,confirm-no-pwd,gbx_overlay,process-adsl-ok,process-wlpwd-active'
				,show: 'quickset-save'
			}
			,to_setWifi: {
				hide: 'confirm-no-pwd,gbx_overlay'
				,show: ''
			}
		}
	}
	function setScene(setter) {
		var toHide,
			toShow,
			len;
		toHide = setter.hide.split(',');
		toShow = setter.show.split(',');
		for(len = toHide.length; len-- > 0;) {
			$('#' + toHide[len]).addClass('none');
		}
		for(len = toShow.length; len-- > 0;) {
			$('#' + toShow[len]).removeClass('none');
		}
	}
	function initControler() {//bind event
		$('#notlinked-next').on('click', function() {
			if(G.data.line === 'ok') {
				setScene(processView.notlinked.to_detect);
			} else {
				rock(document.getElementById('notlinked-rock'));
			}
			
		});
		$('#notlinked-skip').on('click', function() {
			clearTimeout(G.updateData);
			G.detectSkip = true;
			setScene(processView.notlinked.to_setWifi);
		});
		$('#adsl-next').on('click', function() {
			if($('#adsl-user')[0].value === '') {
				alert(_("Username can't be leave blank"));
			} else if($('#adsl-pwd')[0].value === '') {
				alert(_("password can't be leave blank"));
			} else {
				setScene(processView.setAdsl.to_setWifi);
			}
		});
		$('#adsl-skip').on('click', function() {
			setScene(processView.setAdsl.to_setWifi);
			G.data.wanType = 'dhcp';
		});
		$('#dhcp-next').on('click', function() {
			setScene(processView.setDhcp.to_setWifi);
		});
		$('#dhcp-skip').on('click', function() {
			setScene(processView.setDhcp.to_setWifi);
		});
		$('#static-next').on('click', function() {
			setScene(processView.setStatic.to_setWifi);
		});
		$('#static-skip').on('click', function() {
			setScene(processView.setStatic.to_setWifi);
		});
		$('#wifi-next').on('click', function() {
			 if($('#ssid')[0].value === '') {
				alert(_("WiFi Name can't be leave blank"));
			}else if($.getUtf8Length($('#ssid')[0].value) > 32) {
				alert(_('WiFi Name length exceed 32 Bytes'));
			}else if($('#ssid-pwd')[0].value.length < 8) {
				alert(_('The password cannot be less than 8 characters.'));
			} else {
				setScene(processView.setWifi.to_saving);
				$('#main-field').addClass('like-parent');
				saveData();
			}
		});
		$('#wifi-skip').on('click', function() {
			if(G.confirmed) {
				
				//跳过设置将清空还原填写的ssid以及ssid-pwd
				document.getElementById('ssid').value = G.data.ssid;
				document.getElementById('ssid-pwd').value = '';
				setScene(processView.setWifi.to_saving);
				$('#main-field').addClass('like-parent');
				saveData();
			} else {
				G.confirmed = true;
				setScene(processView.setWifi.to_confirmNoPwd);
			}
			

		});
		$('#setpwd').on('click', function() {

			setScene(processView.confirmNoPwd.to_setWifi);
			$('#ssid-pwd')[0].focus();
		});
		$('#closewindow').on('click', function() {
			document.getElementById('setpwd').click();
		});
		$('#notsetpwd').on('click', function() {
			
			//跳过设置将清空还原填写的ssid以及ssid-pwd
			document.getElementById('ssid').value = G.data.ssid;
			document.getElementById('ssid-pwd').value = '';
			setScene(processView.confirmNoPwd.to_saving);
			$('#main-field').addClass('like-parent');
			saveData();
		});
		$('#surfIndex').on('click', function() {
			window.location = '/index.htm';
		});
	}
	
	function getTimeZone(){
		var a  = [],
        	b = new Date().getTime(),
        	zone = new Date().getTimezoneOffset() / -60;
        
	        if( a = displayDstSwitchDates() ){     
	            if(a[0] < a[1]){          
	                if(b > a[0] && b < a[1]) {
	                    zone--;
	                }    
	            }else{  
	                if(b > a[0] || b < a[1]){
	                    zone--;
	                }
	            }
	        }
		return zone;
	}
	
	function saveData() {
		var timeCount = document.getElementById('countDown'),
			f = document.quicksetForm,
			i = 10;
		G.timer = setInterval(function () {
			if(i-- > 0) {
				timeCount.innerHTML = i;
			} else {
				clearTimeout(G.timer)
			}
		}, 1000);

		f.wanType.value = G.data.wanType;
		f.timeZone.value = getTimeZone();
		$.post('./goform/saveQuickSetData', $.serialize(document.quicksetForm), function(req) {
			var responseText = req.responseText;
			if(responseText.toLowerCase().indexOf('<!doctype html')!== -1) {
				window.location.reload(1);
			}
			setScene(processView.saving.to_result);
			if(responseText == 'internetok') {
				$('#saved-success').html(_('Congratulations! You can browse the Internet now!'));
			} else {
				$('#saved-success').html(_('Saved Successfully!'));
				$('#saved-tip').addClass('none');
				$('#jump-to-index').addClass('none');
				setTimeout(function() {
					window.location = '/index.htm';
				}, 2000);
			}
			$('#saved-ssid').html(_('Please connect your WiFi: ') + $('#ssid')[0].value);
		})

		//handle 无线连接将无法接收到回应
		var curSsid = document.getElementById('ssid').value,
			curSsidPwd = document.getElementById('ssid-pwd').value;
		if(G.data.transmitter == 'wireless' && (G.data.ssid !== curSsid || curSsidPwd !== '')) {
			setTimeout(function() {
				setScene(processView.saving.to_result);
				$('#saved-success').html(_('Saved Successfully!'));
				$('#saved-ssid').addClass('none');
				$('#saved-tip').html(_('As you had changed wireless configuration, the connection will be linked down, please reconnect: %s', [curSsid]));
				$('#jump-to-index').addClass('none');
			}, 1000);
		}
	}
	function updateStat() {
		$.getJSON('./goform/getWanConnectStatus?'+ Math.random(), function(jsonData) {
			if(!G.detectSkip) {
				G.data = jsonData;
				initScene();
			}
		});
	}
	
	function initScene() {
		if(G.data['error-info'] == '11') {
			wispSameNetWithSuper();
			//G.updateData = setTimeout(updateStat, 1000);
			return;
		}
		if(G.data.net === 'wait' || G.data.line === 'no') {
			G.updateData = setTimeout(updateStat, 1000);
		}
		
		//detect complete
		if(G.data.line === 'ok' && G.data.net === 'wait') {
			setScene(processView.notlinked.to_detect);
			$('#main-field').addClass('like-parent');
			return;
		}
		$('#main-field').removeClass('like-parent');
		if(G.data.line === 'no') {
			setScene(processView.detect.to_notlinked);
		} else {
			switch(G.data.wanType) {
				case 'pppoe':
					setScene(processView.detect.to_setAdsl);
					break;
				case 'dhcp':
					setScene(processView.detect.to_setDhcp);
					break;
				default:
					setScene(processView.detect.to_setStatic);
			}
		}
	}
	
	function initView() {
		$('#ssid').val(G.data.ssid);
		$('#ssid-pwd').addPlaceholder(_('8~63 characters'));
	}
	function init(jsonData) {
		G.data = jsonData;
		/*data = { 
		"net": "wait", //wait:表示未检测完毕   ok:表示检测完毕
		"line": "no", //no:表示未连接网线   ok:表示已连接网线
		"wanType":"dhcp",  // pppoe dhcp两种值
		"ssid":"tenda-to_abcde"//ssid,
		"transmitter": "wire"//wire or wireless
		}*/
		initView();
		initScene();
	}
	function rolling(obj, points) {
		var i = parseInt(points, 10) || 7,
			j = i,
			contentEle = ['.........','........','.......','......','.....','....','...','..','.'];
		setInterval(function() {
			obj.innerHTML = contentEle[--j];
			if(j === 0) {
				j = i;
			}
		}, 400);
	}
	function rock(obj, time, amplitude_x) {
		var rockTime = parseInt(time, 10) * 1000 || 1200,
			amplitude_x = parseInt(amplitude_x, 10) || 30,
			oPadding = parseInt(obj.style.paddingLeft, 10) || 0,
			step = 3,
			toggle = amplitude_x / step,
			i = 0,
			swing;
		swing = setInterval(function() {
			if(i++ < toggle) {
				obj.style.paddingLeft = ((parseInt(obj.style.paddingLeft, 10) || 0) + step) + 'px';
			} else {
				if (i >= 2 * toggle) {
					i =0;
				}
				obj.style.paddingLeft = ((parseInt(obj.style.paddingLeft, 10) || 0) - step) + 'px';
			}
		}, 15);
		setTimeout(function() {
			obj.style.paddingLeft = oPadding + 'px';
			clearTimeout(swing);
		}, rockTime);
	}

	function wispSameNetWithSuper() {
		var ipnum = G.data['lan-ip'].split("."),
			ipval = parseInt(ipnum[2]),
			m,
			newIp,
			conMassage,
			loc;

		m = (parseInt(ipnum[2]) + 1) % 256;
		newIp = ipnum[0] + "." + ipnum[1] + "." + m + "." + ipnum[3];

		conMassage = _('Auto detection: Your current connetion type may be DHCP.But IP conflicts occur.The login IP will be changed to %s .Please log in again using %s.',[newIp, newIp]);

		if (!G.dialog) {
			G.dialog = $.dialog({
				title: _('Message from webpage'),
				showNoprompt: false,
				content: '<p>' + conMassage + '</p>',
				apply: function () {
					if(this.noprompt === "true") {
						saveDialogStatus(this.noprompt);
					}
					G.ajaxMsg = $.ajaxMassage(_('Saving…please wait…'));
					loc = "./goform/AdvSetLanip?GO=index.htm" + "&LANIP=" + newIp + "&LANMASK=255.255.255.0";
					window.location = loc;
				},
				cancel: function () {
					if(this.noprompt === "true") {
						saveDialogStatus(this.noprompt);
					}
					setScene(processView.detect.to_setWifi);
					$('#main-field').removeClass('like-parent');
				}
			});
		} else {
			G.dialog.open();
		}
	}
	function getData() {
		$.getJSON('./goform/getWanConnectStatus?'+ Math.random(), init);
	}

	// 页面DOM元素加载完成，图片未加载时运行
	$(function () {
		initControler();
		rolling(document.getElementById('rolling1'), 9);
		rolling(document.getElementById('rolling2'), 9);
		getData();
	});
	window.onload = function() {
		$('#adsl-user, #adsl-pwd, #ssid-pwd').val("");
	}
}(window, document, $));