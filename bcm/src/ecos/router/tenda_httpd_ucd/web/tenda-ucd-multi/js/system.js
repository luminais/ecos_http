function isFileType(name, types) {
	var fileTypeArr = name.split('.'),
		fileType = fileTypeArr[fileTypeArr.length - 1],
		len = types.length,
		i;

	for (i = 0; i < len; i++) {
		if (fileType === types[i]) {
			return true;
		}
	}

	return false;
}

(function (window, document) {

	/*local variable define*/
	var G = {},
		pattern = /^\d*\.?\d+/;//匹配正数
	/*end of variable define*/

	G.reqOK = 1;
	G.mtuval = 1500;
	G.firstInit = true;

	function disableMacbox(macbox, macCheck, srcSelect, dontfocus) {
		if(macCheck.checked && srcSelect.options[0].selected || !macCheck.checked) {
			macbox.disable(true);
			G.validate.check(macbox);
		} else {
			macbox.disable(false);
			if(dontfocus == undefined) {
				macbox.toFocus();
			}
		}

	}
	function setMac() {
		var $macbox =  $('#macbox'),
			macbox = $macbox [0],
			macCheck = $('#macClone')[0],
			srcSelect = $('#macSrcSelect')[0];
			//wireless access handling

			if(G.data.transmitter == 1) {
				srcSelect.disabled = true;
				srcSelect.options[1].selected = true;
			}

		if(G.data['mac-def'] !== G.data['mac-wan']) {
			macCheck.checked = true;
			if(G.data['mac-wan'] !== G.data['mac-pc']) {
				srcSelect.options[1].selected = true;
			}
		}
		macbox.val(G.data['mac-wan']);
		disableMacbox(macbox, macCheck, srcSelect, 1);

		//add EventListener for checkbox
		macCheck.onclick = function() {
			if(this.checked) {
				if(srcSelect.options[1].selected){
					if(G.data['mac-wan'] !== G.data['mac-def'] && G.data['mac-wan'] !== G.data['mac-pc']) {
						macbox.val(G.data['mac-wan']);
					} else {
						macbox.val("");
					}
				} else {
					macbox.val(G.data['mac-pc']);
				}
			} else {
				macbox.val(G.data['mac-def']);
			}


			disableMacbox(macbox,macCheck,srcSelect);
		};

		//add EventListener for srcSelect
		srcSelect.onchange = function() {
			if(macCheck.checked) {
				if(this.options[0].selected){
					macbox.val(G.data['mac-pc']);
					} else {
						if(G.data['mac-wan'] !== G.data['mac-def'] && G.data['mac-wan'] !== G.data['mac-pc']) {
							macbox.val(G.data['mac-wan']);
						} else {
							macbox.val("");
						}
					}
			} else {
				macbox.val(G.data['mac-def']);
			}

			disableMacbox(macbox,macCheck,srcSelect);
		};
	}

	function toinput(tohide, tofocus,flag) {
		document.getElementById(tohide).style.display="none";
		if(flag) {
			document.getElementById(tofocus).value="";
		}
		document.getElementById(tofocus).focus();
	}

	//父元素事件监听冒泡
	function selectvalue(root, inputbox, unit) {
		var selectedval;

		root = typeof root == "string" ? document.getElementById(root) : root;

		$(root).on('mouseover', function(e) {
		var target = e.target||e.srcElement;
			if(target.tagName.toLowerCase() !== "a") return;
			selectedval = target.innerText || target.textContent;
		});

		$(root).on('click', function(e) {
			var target = e.target||e.srcElement,
				dropdownBox = document.getElementById(inputbox);

			if(target.tagName.toLowerCase()  !== "a") {
				return;
			}
			if(selectedval.trim() == _('Manual Setup')) {
				dropdownBox.focus();
				dropdownBox.select();
			} else {
				if((unit === 1)&&(pattern.exec(selectedval) !== null)) {
					dropdownBox.value = pattern.exec(selectedval);
				}else if ((unit !== 0)&&(pattern.exec(selectedval) !== null)) {
					dropdownBox.value = pattern.exec(selectedval) + unit;
				}else {
					dropdownBox.value = selectedval;
				}

				G.validate.check(dropdownBox);
			}

			root.style.display="none";
		});
	}
	/*end of dropdown menu handling*/
	function setSoftVer() {
		$('.softVer')[0].innerHTML = _('Current version: ') + G.data.softVer;
	}

	function disOldpwd() {
		if(G.data.hasoldpwd == 1) {
			document.getElementById('group_oldpwd').style.display = "";
		} else {
			document.getElementById('group_oldpwd').style.display = "none";
		}
	}

	function setMtu() {
		var ctype = G.data['con-type'],
			mtuElem = document.getElementById('mtubox')/*mtu--,
			autoMtuElem = document.getElementById('autoSetMtu'),
			autoMtuLabel = document.getElementById('label-setmtu'),
			toggleButton = document.getElementById('dropbtnmtu')*/;
		if(ctype === "0") {
			mtuElem.value = G.data['mtu-static'];
		} else if(ctype === "1") {
			mtuElem.value = G.data['mtu-dhcp'];
		} else if(ctype === "2") {
			mtuElem.value = G.data['mtu-adsl'];
			/*mtu--autoMtuLabel.style.display = 'block';
			if(G.data['autoSetMtu'] == '1') {
				autoMtuElem.checked = true;
				mtuElem.disabled = true;
				toggleButton.disabled = true;
			}
			$('#autoSetMtu').on('click', function() {
				if(this.checked) {
					mtuElem.disabled = true;
					toggleButton.disabled = true;
				} else {
					mtuElem.disabled = false;
					toggleButton.disabled = false;
				}
			});*/
		}
		G.mtuval = mtuElem.value;
	}

	function setWps() {
		var ewps = document.getElementById('enwps');
		if(G.data.enwps === "1") {
			ewps.checked = true;
		} else {
			ewps.checked = false;
		}
	}

	function setLanIp() {
		var elem = document.getElementById("LANIP");
		if(G.data.lanip && (elem.value == null||elem.value == "") ){
		elem.value = G.data.lanip;
		}
	}

	function setWanSpeed() {
		var wanSpeedElem = document.getElementById('wanSpeed');
		if(G.data.wanspeed) {
			wanSpeedElem.value = G.data.wanspeed;
		}
	}

	function setBroadcastSsid() {
		var ssidBroadElem = document.getElementById('broadcastssid');
		if(G.data.broadcastssid === "1") {
			ssidBroadElem.checked = true;
		} else {
			ssidBroadElem.checked = false;
		}
	}

	function setChannel() {
		var channelElem = document.getElementById('wireless-channel');
		if(G.data.channel) {
			channelElem.value = G.data.channel;
		}
	}

	function setDhcpSever() {
		var dhcpSever = document.getElementById('endhcp');
		if(G.data.dhcpEn == 1) {
			dhcpSever.checked = true;
		} else {
			dhcpSever.checked = false;
		}
	}
	function setWlBandwidth() {
		var wlBandwidthEl = document.getElementById('wlBandwidth');
		if(G.data.channel) {
			wlBandwidthEl.value = G.data.wlBandwidth;
		}
	}
	function initHtml() {
		var id,
			elem,
			conState = $('#con-state')[0],
			conType = $('#con-type')[0],
			conTypeIdx = [_('Static IP'), _('DHCP'), _('PPPoE')],
			conStateIdx = [_('Unconnected'), _('Connecting…'), _('Connected with Server')],
			currenTime,
			curTimeTip = document.getElementById('curTimeTip');
			//wpsElem = document.getElementById('enwps');

		disOldpwd();

		if(G.firstInit) {
			$('#newpwd').addPlaceholder(_('Number, letter, or underscore.'));
			$('#pwd2').addPlaceholder(_('Confirm your new password.'));
			G.firstInit = false;
		}
		if(G.data.wanMode !== 'ap') {
			document.getElementById('wireless-channel').disabled = true;
		} else {
			document.getElementById('wireless-channel').disabled = false;
		}
		for (id in G.data) {
			elem = document.getElementById(id);

			if (elem) {
				if(typeof elem.value !== 'string') {
					elem.innerHTML = G.data[id];
				}
			}
		}

		//hack: 后台无法处理PPPOE子网掩码传值。。
		if(parseInt(G.data['con-type'],10) === 2 && G.data.submask !== "") {
			document.getElementById('submask').innerHTML = '255.255.255.255';
		};

		conType.innerHTML = conTypeIdx[parseInt(G.data['con-type'],10)];
		conState.innerHTML = conStateIdx[parseInt(G.data['con-state'],10)];
		

		if(G.data['internet-state'] == '1' && G.data.time_str) {
			//currenTime = new Date(parseInt(G.data.curTime, 10) * 1000);
			curTimeTip.innerHTML = _('Time has been synchronized with Internet successfully');
			document.getElementById('curTime').innerHTML = G.data.time_str;
		} else {
			currenTime = new Date();
			curTimeTip.innerHTML = _('Time will be automantically synchronized with Internet');
			document.getElementById('curTime').innerHTML = currenTime.getFullYear() + '-' +
			(currenTime.getMonth()+101).toString().slice(-2) + '-' +
			(currenTime.getDate() + 100).toString().slice(-2) + ' ' +
			(currenTime.getHours() + 100).toString().slice(-2) + ':' +
			(currenTime.getMinutes() + 100).toString().slice(-2) + ':' +
			(currenTime.getSeconds() + 100).toString().slice(-2);
		}
		
		if (G.data['con-state'] == "2") {
			conState.className = "text-success info-block ";
		} else if (G.data['con-state']  == "1") {
			conState.className = "text-title info-block ";
		} else {
			conState.className = "text-error info-block ";
		}
	}

	function refreshTable() {
		$.getJSON('/goform/SystemManageInit?' + Math.random(), function(obj) {
			if(this.responseText.indexOf('!DOCTYPE ') !== -1) {
				window.location.reload();
				return;
			}
			G.data = obj;
			initHtml();
			G.stimer = setTimeout(refreshTable, 5000);
		});
	}

	function setValue() {
		setMac();
		setMtu();
		setLanIp();
		setSoftVer();
		//setWps();
		setWanSpeed();
		setBroadcastSsid();
		setChannel();
		setDhcpSever();
		setWlBandwidth();
		
		//add 处理时区
		document.getElementById('timeZone').value = G.data.timeZone;
	}

	function getData() {
		$.getJSON('/goform/SystemManageInit?' + Math.random(), function(obj) {
			if(this.responseText.indexOf('!DOCTYPE ') !== -1) {
				window.location.reload();
				return;
			}
			G.data = obj;

			if(parseInt(G.data['con-type'],10) === 2) {
				var mtuBox = document.getElementById('mtubox'),
					mtuUl = document.getElementById('MTUVAL1'),
					mtuUli = mtuUl.getElementsByTagName('li')[0],
					mtuUliText = mtuUli.innerText || mtuUli.textContent;

				mtuUliText = mtuUliText.trim();
				// 对adsl对输入框和下拉列表做特殊处理
				mtuBox.setAttribute('data-options','{"type": "num", "args":[256,1492]}');
				if(mtuUliText == "1500") {
					mtuUl.removeChild(mtuUli);
				}
			}

			initHtml();
			setValue();
			G.stimer = setTimeout(refreshTable, 5000);
		});
	}

	function pwdsubmithandle(msg) {
		if(msg !== '1') {
			alert(_('The old password is incorrect. The settings will not be saved.'));
			document.getElementById('oldpwd').focus();
		} else {
			document.getElementById('oldpwd').value = "";
			document.getElementById('newpwd').value = "";
			document.getElementById('pwd2').value = "";
		}
	}

	function setDefValue(id ,defvalue) {
		var elem = document.getElementById(id);
		if(elem.value == "") {
			elem.value = defvalue;
		} else if(id == 'mtubox') {
			if(isNaN(elem.value)) {
				elem.value = "";
			} else {
				elem.value = parseInt(elem.value, 10);
			}
		}
	}
	function initControler() {
		//create macbox
		$('#macbox').toTextboxs('mac');

		/*add eventListener*/
		$('#LANIP').on('click',function() {
			toinput('LANIPS','LANIP');
		});

		$('#LANIP').on('blur',function() {
			setDefValue('LANIP',G.data.lanip);
		});

		$('#mtubox').on('click',function() {
			toinput('MTUVAL1','mtubox');
		});

		$('#mtubox').on('blur', function() {
			setDefValue('mtubox',G.mtuval);
		});

		$('#oldpwd').on('focus', function() {
			$('#oldpwd').removeValidateTip(true);
		});
		//select option bind title
		$('select').on('mouseover', function() {
			var $this = $(this);
			if(this.getAttribute('data-bind-title')) {
				return;
			}
		 	this.setAttribute('data-bind-title', true);
			$this.find('option').each(function() {
				var $option = $(this);
				var _text = $.trim($option.context.text);
				if(_text.length > 0 && !this.getAttribute('title')) {
					this.setAttribute('title', _text);
				}
			});
		});
		$(document).on('click', function(e) {
			var target = e.target || e.srcElement,
				ulList,
				targetDis;
			/*mtu--if(document.getElementById('autoSetMtu').checked && (target.id == 'dropbtnmtu' || target.parentNode.id == 'dropbtnmtu')) {
				return;
			}*/

			if( $(target.parentNode).hasClass('toggle') ) {
				target = target.parentNode;
			}
			if($(target).hasClass('toggle')) {
				ulList =target.parentNode.parentNode.getElementsByTagName('ul')[0],
				targetDis = ulList.style.display;
			}
			$('.toggle').each(function() {
				this.parentNode.parentNode.getElementsByTagName('ul')[0].style.display = 'none';
			});

			if($(target).hasClass('toggle') ) {
				ulList.style.display = (targetDis == 'none' || targetDis == '') ? 'block':'none';
			}
		});

		$('#reboot').on('click', function() {
			var url = "/goform/SysToolReboot";

			if(window.confirm(_('Are you sure to reboot the router?'))) {
				$.get(url, function(){ });
				progressStrip(location.href.replace('system.htm', 'index.htm'), 120, _('Rebooting…'));
			}
		});

		$('#restore').on('click', function() {
			if(confirm(_('The login IP address will reset to 192.168.0.1 after restoring the router to factory default. Are you sure to restore?'))){
				document.restorefrm.submit() ;
			}
		});

		$('#downloadlog').on('click',function() {
			if(confirm(_('Export the system logs and specify a local file to save the system logs.'))) {
				window.location = "/cgi-bin/DownloadSyslog/RouterSystem.log?" + Math.random();
			}
		});

		G.validate = $.validate({
			custom: function () {
				var pwd2 = document.getElementById('pwd2'),
					oldpwd = document.getElementById('oldpwd'),
					group_oldpwd = document.getElementById('group_oldpwd'),
					newpwd = document.getElementById('newpwd'),
					lanip = document.getElementById('LANIP').value,
					gateway = G.data.gateway,
					wanip = G.data.wanip,
					lanMask = '255.255.255.0',
					wanMask = G.data.submask;

				//pwd
				if(newpwd.value !== pwd2.value) {
					G.reqOK = 1;
					pwd2.focus();
					setTimeout(function () {
						$('#pwd2').removeValidateTip(true)
						.addValidateTip(_('The passwords do not match.'), true)
						.showValidateTip();
					}, 40);
					return true;
				}
				if(group_oldpwd.style.display !== 'none' && oldpwd.value == '') {
					if(newpwd.value != '') {
						$('#oldpwd').removeValidateTip(true)
						.addValidateTip(_('Please enter the old password.'), true)
						.showValidateTip();
						return true;
					}
				}

				if(isSameNet(lanip, wanip, lanMask, wanMask) || isSameNet(lanip, gateway, lanMask, wanMask)) {
						document.getElementById('LANIP').focus();
						G.reqOK = 1;
						return _('IP conflict! The login IP address cannot be the same as default gateway or DNS server. Please make a change.');
				}
			},

			success: function () {
				if(!G.reqOK) {
					return;
				} else {
					G.reqOK = 0;
				}
				var reboot = 0,
					submitStr = "",
					confirmStr = _('Commit changes?'),
					oldpwd = document.getElementById('oldpwd').value,
					newpwd = document.getElementById('newpwd').value,
					lanipElem = document.getElementById('LANIP'),
					lanip = lanipElem.value,
					endhcp = document.getElementById('endhcp').checked ? 1 : 0,
					channel = document.getElementById('wireless-channel').value,
					broadcastSsid = document.getElementById('broadcastssid').checked ? "1" : "0",
					wanSpeed = document.getElementById('wanSpeed').value,
					timeZone = document.getElementById('timeZone').value,
					channelBandwidth = document.getElementById('wlBandwidth').value;

				submitStr = $.serializeByClass('validatebox') + '&dhcpEn=' + endhcp + '&channel=' + channel +
					'&broadcastssid=' + broadcastSsid + '&wanspeed=' + wanSpeed;
				if (G.data.hasoldpwd == 1) {
					submitStr += "&oldpwd=" + encodeURIComponent(str_encode(oldpwd));
				}
				/*mtu--if(document.getElementById('autoSetMtu').checked) {
					submitStr += "&autoSetMtu=1";
				} else {
					submitStr += "&autoSetMtu=0";
				}*/
				//add timeZone handle
				submitStr += "&timeZone=" + timeZone;
				submitStr += "&curTime=" + (new Date()).getTime() / 1000;

				submitStr += "&wlBandwidth=" + channelBandwidth;
				submitStr += "&Enewpwd=" + str_encode(newpwd);
				if(G.data.lanip !== lanip) {
					if(lanip.split('.')[3] === '255'){
						alert(_('There is a mismatch between IP address and Subnet Mask.Please change the last input 255 into a value from 1 to 254.'));
						G.reqOK = 1;
						return;
					}
					if(confirm(_('The login IP address will be changed into %s',[lanip]))) {
						submitStr += "&lanip=" + lanip;
						reboot = 1;
					} else {
						G.reqOK = 1;
						return;
					}
				} else if(!confirm(confirmStr)) {
					G.reqOK = 1;
					return;
				}
				$('#submitval')[0].disabled = 'disabled';
				clearTimeout(G.stimer);
				G.ajaxMsg = $.ajaxMassage(_('Saving…please wait…'));
				$.post('/goform/SystemManageSave', submitStr, function (req) {
					var pwdError = req.responseText.indexOf('pwdError') !== -1,
						pwdOk = req.responseText.indexOf('pwdOk') !== -1;
					if(req.responseText.indexOf('ip_conflict') !== -1) {
						G.ajaxMsg.hide();
						alert(_('The login IP address cannot be in  the same net segment as the WAN IP address.'));
						lanipElem.value = G.data.lanip;
						G.reqOK = 1;
						return;
					}
					if(pwdError) {
						G.ajaxMsg.hide();
						pwdsubmithandle('0');
						G.reqOK = 1;
						$('#submitval')[0].disabled = '';
						return;
					} else if(pwdOk) {
						pwdsubmithandle('1');
					}
					setTimeout(function () {
						G.ajaxMsg.text(_('Saved successfully!'));
						G.reqOK = 1;
						setTimeout(function () {
							G.ajaxMsg.hide();
							$('#submitval')[0].disabled = '';
						}, 500);
						if(reboot) {
							setTimeout(function() {
								progressStrip("http://"+lanip, 200, _('Rebooting…'));
							},900);
						} else {
							setTimeout(getData,600);
						}
					}, 1500);
				});
				//alert(submitData);
			},

			error: function (msg) {
				if(msg && typeof msg === "string" ) {
					alert(msg);
				}
			}
		});

		G.validate.addElems([document.getElementById('pwd2'), document.getElementById('newpwd')]);
		$('#submitval').on('click',function() {
			G.validate.checkAll();
		});

		$('#cancel').on('click',function() {
			window.location.reload();
		});
	}

	$(function() {
		//dropdown-menu
		selectvalue('MTUVAL1','mtubox',1);
		selectvalue('LANIPS','LANIP',0);
		initControler();
		getData();
	});
	window.onload = function(){
		document.getElementById('oldpwd').value = ''; //解决火狐浏览器自动填充密码引发的显示问题
	};

	function showUpgrading() {
		var url = location.href.replace('system.htm', '');

		progressStrip(url, 130, _('Rebooting…please wait…'), 240, _('Upgrading... Do not remove the power supply.'));
		$("#fileFrame").addClass('none');
		$("#upgradefrm").removeClass('none');
		clearTimeout(G.stimer);
	}

	window.showUpgrading = showUpgrading;

}(window, document));