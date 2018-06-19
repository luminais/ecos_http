/*last modifier dx; date 9.30*//*last modifier dx; date 2014.4.15*/
(function () {
var pattern = /^\d*\.?\d+/,
	data,
	list,
	timer,
	dataChange = 0,
	G = {};

	G.reqOK = 1;
	G.netAccess = 0;//只要有一个非本机客户端可以上网，就置为1
	function refreshTable() {
		$.getJSON('/goform/SpeedControlInit?' + Math.random(), function(obj) {
			if(this.responseText.indexOf('!DOCTYPE ') !== -1) {
				window.location.reload();
				return;
			}
			data = obj;
			list = data['controllist'];
			list.sort(function (x, y) {
				x = parseFloat(x['downloadspd']);
				y = parseFloat(y['downloadspd']);
				  return y-x;
			});
			createTable();
			timer = setTimeout(refreshTable, 5000);
		});
	}

	function getData() {
		$.getJSON('/goform/SpeedControlInit?' + Math.random(), function(obj) {
			data = obj;
			list = data['controllist'];
			list.sort(function (x, y) {
				x = parseFloat(x['downloadspd']);
				y = parseFloat(y['downloadspd']);
				  return y - x;
			});
			timer = setTimeout(refreshTable, 5000);
			createTable();
			initHtml();

		});
	}

	function addUnit(o,unit) {
		var temp = o.value;
		if(o.value == _("addUnit") ||
			o.value == _("Forbidden")) return;
		if(temp !== null && pattern.exec(temp) !== null) {
			temp = pattern.exec(temp);
			if(parseFloat(temp) > 300) {
				temp = 300;
			}
			temp = temp + unit;
			o.value = temp;
		} else {
			o.value = _("Unlimited");
		}
	}


	function toInput(o,flag) {
		if(o.parentNode.className.indexOf( 'input-append') !== -1) {
			tofocus = o,
			tohide = o.parentNode.getElementsByTagName('ul')[0];
		}else {
			tofocus = o.parentNode.parentNode.parentNode.parentNode.getElementsByTagName('input')[0],
			tohide = o.parentNode.parentNode;
		}
		tohide.style.display="none";
		if(flag) {
		tofocus.value="";
		}
		tofocus.focus();
	}

	var selectedval;
	//父元素事件监听冒泡
	function selectValue(o,unit) {
		var root = typeof o == "object" ? o.getElementsByTagName('ul')[0] : getElementsByClassName('input-append')[0].getElementsByTagName('ul')[0];
		var items=root.getElementsByTagName("li");

		$(root).on('mouseover', function(e) {
		var target=e.target||e.srcElement;
			if(target.tagName.toLowerCase() !=="a") return;
			clearTimeout(timer);
			selectedval=target.innerText || target.textContent;
		});

		$(root).on('click', function(e) {
			var target=e.target||e.srcElement,
				dropdownBox = o.getElementsByTagName('input')[0],
				prevalue = dropdownBox.value;
			if(target.tagName.toLowerCase() !=="a") {
				return false;
			}

			if(selectedval.trim() == _('Manual Setup (Mbps)')) {
				dropdownBox.focus();
				dropdownBox.select();
			} else {
				if((unit===1)&&(pattern.exec(selectedval)!==null)) {
					dropdownBox.value = pattern.exec(selectedval);
				} else if ((unit!==0)&&(pattern.exec(selectedval)!==null)) {
					dropdownBox.value = pattern.exec(selectedval) + unit;
				} else {
					dropdownBox.value = selectedval;
				}
			}

			/*if(dropdownBox.value !== prevalue) {*/
				dataChange = 1;
				clearTimeout(timer);
			/*}*/
			root.style.display = "none";
			return false;
		});

	}

	function initEvent() {
		//event click radio
		$('#type-select input').on('click',function() {
			var showdiv = parseInt(this.value,10);
			show(showdiv);
		});
		//submit
		$('#submitval').on('click',function() {

			G.validate.checkAll();
		});
		//cancel
		$('#cancel').on('click',function() {
			window.location.reload();
		});

		//new! at 4.15  add EventListener for access-ctrl
		$('#access-ctrl').on('click', function() {
			clearTimeout(timer);
			$('.switch').each(function(){
				if(!$(this).hasClass('off')){
					G.netAccess = 1;
				}
			});
			if(G.netAccess == 1) {
				$('.switch').each(function(){
					$(this).addClass('off');
				});
				$('.cover').removeClass('none');
				$('#access-ctrl').html(_('Onekey Limitation'));
				G.netAccess = 0;
			} else {
				$('.switch').each(function(){
					$(this).removeClass('off');
				});
				$('.cover').addClass('none');
				$('#access-ctrl').html(_('Onekey Permission'));
			}
		});
		// add EventListener for dropdown-toggle in & document
		$(document).on('click', function(e) {
			var target = e.target || e.srcElement;
			if( $(target.parentNode).hasClass('toggle') ) {
				target = target.parentNode;
			}
			if($(target).hasClass('toggle')) {
				var ulList =target.parentNode.parentNode.getElementsByTagName('ul')[0],
					targetDis = ulList.style.display;
			}
			$('.toggle').each(function() {
				this.parentNode.parentNode.getElementsByTagName('ul')[0].style.display = 'none';
			});
			if($(target).hasClass('toggle') ) {
				ulList.style.display = (targetDis == 'none' || targetDis == '') ? 'block':'none';
				if(ulList.style.display !== 'none' || dataChange === 1) {
					clearTimeout(timer);
				} else {
					timer = setTimeout(refreshTable,5000);
				}
			}
		});
	}

	function show(showdiv) {
		if(showdiv == 1) {
			$('#netCtrl').show();
			} else {
			$('#netCtrl').hide();
		}
	}

	function initHtml() {
		if(data['enablecontrol'] === '1'){
			document.getElementById('enableCtrl').checked = true;
		} else {
			document.getElementById('disableCtrl').checked = true;
		}
		$('.switch').each(function(){
			if(!$(this).hasClass('off')){
					G.netAccess = 1;
				}
			});
			if(G.netAccess == 1){
				$('#access-ctrl').html(_('Onekey Limitation'));
			} else {
				$('#access-ctrl').html(_('Onekey Permission'));
			}
		show(data['enablecontrol']);
	}

	//new! at 4.15
	function createTable() {
		var len = list.length,
			i,
			record,
			dropDownStr,
			noRecord,
			textColor,
			hostName,
			switchStr,
			coverStr="",
			sysType,
			//conTime,
			spd;

		var thead = "<thead><tr><th>" + _('Device Info') +
			"</th><th width='74px'>" + _('Note') + "</th><th width='66px'>" + _('Connection') +
			"</th><th width='63px'>" + _('Speed') + "</th><th width='140px'>" + _('Bandwidth Control') +
			"</th><th width='78px'>" + _('Access') + "</th></tr></thead>",
			tbody = "";
		if(navigator.vendor && navigator.vendor.indexOf('Apple') !== -1) {
			thead = "<thead><tr><th>" + _('Device Info') +
			"</th><th width='86px'>" + _('Note') + "</th><th width='78px'>" + _('Connection') +
			"</th><th width='75px'>" + _('Speed') + "</th><th width='152px'>" + _('Bandwidth Control') + "</th><th width='90px'>" + _('Access') + "</th></tr></thead>";
		}
		if(len === 0) {
			noRecord = "<tr height='37px'><td colspan='6'>" + _('There are no devices connected to the router.') + "</td></tr>";
		} else {
			noRecord = "";
		}
		tbody += noRecord;
		for(i = 0; i < len; i++) {
			record = list[i];
			dropDownStr = "<li><a>" + _('Forbidden') +"</a></li>";
			textColor = "";
			if(record['ipval'] == data['localip']) {
				hostName = _('(Native Device)') + record['hostname'];
				switchStr = _('Native');
				coverStr="",
				dropDownStr = "";
				textColor = "text-info";
			} else{
				hostName = record['hostname'];
				if(hostName == "") {
					hostName = " ";
				}
				if(record['spdlimit'] == "0.00") {
					switchStr = '<a class="switch off"></a>';
					coverStr = '<a class="cover" title="' + _('Turn on/off to enable/disable the device to access the Internet.') +'"></a>';
				} else {
					switchStr = '<a class="switch"></a>';
					coverStr = '<a class="cover none" title="' + _('Turn on/off to enable/disable the device to access the Internet.') + '"></a>';
				}
			}

			if(record['spdlimit'] == "301.00") {
				spd = _('Unlimited');
				record['downloadspd'] += "Mbps";
			} else if(record['spdlimit'] == "0.00") {
				spd = _('Forbidden');
				record['downloadspd'] = "--";
			} else {
				spd = record['spdlimit'] + "Mbps";
				record['downloadspd'] += "Mbps";
			}

			tr='<tr><td class="' + textColor + '">' +
				'<input type="hidden" name="macval" value="' + record['macval'] + '" >' +
				'<ul><li class="host-name fixed">' + hostName + '</li><li>' +
				'<span class="host-info">' + _('IP:') + record['ipval'] + '</span></li><li>' +
				'<span class="host-info">' + _('MAC:') + record['macval'] + '</span></li></td>' +
				'<td><input type="text" class="input-mini remarkText validatebox" maxlength="20" data-options=\'{"type":"remarkTxt", "args":[",;\\""]}\' value="' + record['remark'] + '"></td>'+
				'<td><div class="text-success">' + record['linktype'] +  '</div></td>' +
				'<td class="fixed">' + record['downloadspd'] + '</td>' +
				'<td><div style="position: relative;">' + coverStr + '</div><div class="input-append"><input class="input-small input-box" type="text" value="' + spd + '">' +
				'<div class="btn-group"><button class="toggle btn btn-small"><span class="caret"></span></button></div>' +
				'<div class="input-select">' +
				'<ul class="dropdown-menu"><li><a>' + _('Unlimited') +'</a></li><li><a>' + _('0.5Mbps(Browsing  webpage smoothly)') +'</a></li><li><a>' + _('1.0Mbps(Watching standard definition video smoothly)') + '</a></li><li><a>' + _('2.0Mbps(Watching  high-definition video smoothly)') + '</a></li><li class="divider"></li><li><a class="hand-set">' + _('Manual Setup (Mbps)') + '</a></li></ul>' +
				'</div></div></td>' +
				'<td>' + switchStr + '</td>' +
				'</tr>';
			tbody+=tr;
		}
		$('#netCtrl')[0].innerHTML = "<table class='table table-fixed'>" + thead + "<tbody>" + tbody +"</tbody></table>";
		var $greentds = $('.text-success'),
			tlen = $greentds.length;
		for(i = 0; i < tlen; i++) {
			if($greentds[i].innerHTML == 1) {
				$greentds[i].innerHTML = _('Wireless');
				$greentds.eq(i).removeClass('text-success');
			} else if($greentds[i].innerHTML == 0){
				$greentds[i].innerHTML = _('Wired');
			} else {
				$greentds.eq(i).removeClass('text-success');
			}
		}

		/*init tableEvent*/
		//add title for host & remark
		$('.fixed').each(function() {
			this.title = this.innerHTML;
		});

		$('.remarkText').each(function() {
			var predata = this.value;
			this.title = this.value;
			this.onkeyup = function(e) {
				this.title = this.value;
			}

			this.onfocus = function(e) {
				clearTimeout(timer);
			}

			this.onblur = function() {
				if(predata !== this.value ) {
					dataChange = 1;
				}
				if(dataChange ===0) {
					timer = setTimeout(refreshTable,5000);
				}
			}

		});

		// add EventListener for input-box
		$('.input-box').each(function() {
			var prevalue = this.value;
			this.onclick = function(e) {
				toInput(this);
			}
			this.onfocus = function(e) {
				clearTimeout(timer);
			}
			this.onblur = function(e) {
				if(this.value !== prevalue) {
					dataChange =1;
				}
				addUnit(this,'Mbps');
				if(dataChange === 0) {
					timer = setTimeout(refreshTable,5000);
				}
			}
		});

		// add EventListener for dropdown-menu
		$('.input-append').each(function (i) {
			selectValue(this, 'Mbps');
		});

		// add EventListener for dropdown-menu's hand-set
		$('.hand-set').each(function() {
			this.onclick = function(e) {
				toInput(this);
			}
		});

		//new! at 4.15  add EventListener for netCtrl switcher
		var switcher = $('.switch');
			if(switcher.length !== 0) {
				switcher.on('click',function() {
					clearTimeout(timer);
					var tar = $(this);
					if(tar.hasClass('off')) {
						tar.removeClass('off');
						tar.parent().parent().find('.cover').addClass('none');
					} else {
						tar.addClass('off');
						tar.parent().parent().find('.cover').removeClass('none');
					}
				});
				$('#access-ctrl').show();
				$('#access-ctrl')[0].style.display = 'block';
			} else {
				$('#access-ctrl').hide();
			}

		G.validate = $.validate({
			custom: function () {},
			success: function () {
				submit();
			},
			error: function () {
				G.reqOK = 1;
			}
		});
		/*end of init tableEvent*/
	}

	function submit() {
		if(!G.reqOK) {
			return;
		} else {
			G.reqOK = 0;
		}
		var trs = document.getElementById('netCtrl').getElementsByTagName('tr'),
			$trs = $(trs),
			len = $trs.length,
			i = 1,
			submitStrArr = [],
			submitStr,
			enCtrl = 1;//getValueByName('switcher');
		//if(enCtrl != 0){
		for(;i < len; i++) {
			var $tr = $trs.eq(i),
				mark = $tr.find('td').eq(1).find('input')[0].value,
				mac = $tr.find('td').eq(0).find('input')[0].value,
				speed = $tr.find('td').eq(4).find('input')[0].value,
				accessButton = $tr.find('td').eq(5).find('a');

			if(speed === _('Unlimited') || speed === "") {
				speed = "301";
			} else if(speed === _('Forbidden')) {
				speed = "0";
			}
			if(accessButton.length && accessButton.hasClass('off')) {
				speed = "0";
			} else if(accessButton.length && !accessButton.hasClass('off')){
				if(speed == "0") {
					speed = "301";
				}
			}
			if(speed != "301"  && speed != "0") {
				speed = parseFloat(speed);
			}
			submitStrArr.push(mac + "," + speed + "," + mark);
		}
		submitStr = "enablecontrol=" + enCtrl + "&list=" + encodeURIComponent(submitStrArr.join(';'));

		$('#submitval')[0].disabled = 'disbaled';
		clearTimeout(timer);
		G.ajaxMsg = $.ajaxMassage(_('Saving…please wait…'));
		$.post('/goform/SpeedControlSave', submitStr, function (req){
			G.reqOK = 1;
			if(req.responseText.indexOf('mac_overflow') !== -1) {
				alert(_('You have forbidden 20 devices. If added one more, the initial entry will be cleared.'));
			} else if(req.responseText.indexOf('tc_overflow') !== -1) {
				alert(_('You have limited 20 devices.  If added one more, the initial entry will be cleared.'));
			} else if(req.responseText.indexOf('mark_overflow') !== -1) {
				alert(_('You have noted 20 devices. If added one more, the initial entry will be cleared.'));
			}
			setTimeout(function () {
				G.ajaxMsg.text(_('Saved successfully!'));
				setTimeout(function () {
					G.ajaxMsg.hide();
					$('#submitval')[0].disabled = '';
					getData();
				}, 500);
			}, 1500);
		});

	}

	//DOM Ready
	$(function() {
		initEvent();
		getData();
	});
}());