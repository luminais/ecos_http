/*last modifier dx; date 9.30*//*last modifier dx; date 2014.4.15*/
(function () {
var pattern = /^\d*\.?\d+/,
	data,
	list,
	blacklist,
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
			blacklist = data['blacklist'];
			list.sort(function (x, y) {
				x = parseFloat(x['downloadspd']);
				y = parseFloat(y['downloadspd']);
				  return y - x;
			});
			timer = setTimeout(refreshTable, 5000);
			createTable();
			createBlackTable();
			initHtml();
			
		});
	}
	
	function addUnit(o,unit) {
		var temp = o.value;
		if(o.value == "无限制" || o.value == "禁止上网") return;
		if(temp !== null && pattern.exec(temp) !== null) {
			temp = pattern.exec(temp);
			if(parseFloat(temp) > 300) {
				temp = 300;
			}
			temp = temp + unit;
			o.value = temp;
		} else {
			o.value = "无限制";
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

			if(selectedval.trim() == "手动设定（Mbps）") {
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
				$('#access-ctrl').html('一键解除');
				G.netAccess = 0;
			} else {
				$('.switch').each(function(){
					$(this).removeClass('off');
				});
				$('.cover').addClass('none');
				$('#access-ctrl').html('一键限制');
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

		//黑名单按钮
		$(document).on("click", function(e) {
			var target = e.target || e.srcElement;

			if (target.className.indexOf("blacklist-handle") == -1) return;
			if (target.getAttribute("data-limit") == "allow") {
				target.setAttribute("data-limit", "limit");
				target.innerHTML = "移出列表";
			} else {
				target.setAttribute("data-limit", "allow");
				target.innerHTML = "撤销移除";				
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
		if(+data["wifi-power"]) {
			 $(".icons-signal").parent().removeClass("none");	
		} else {
			 $(".icons-signal").parent().addClass("none");
		}	
		$('.switch').each(function(){
			if(!$(this).hasClass('off')){
					G.netAccess = 1;
				}
			});
			if(G.netAccess == 1){
				$('#access-ctrl').html('一键限制');
			} else {
				$('#access-ctrl').html('一键解除');
			}
		show(data['enablecontrol']);
	}

	//创建黑名单列表
	function createBlackTable() {
		var len = blacklist.length;

		var thead = '<thead><th width="510px">MAC地址</th><th class="txt-center">操作&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</th></thead>',
			tbody = '',
			btnStr = '<a tindex="&" data-limit="limit" class="blacklist-handle btn">移出列表</a>';

		if (len == 0) {
			$("#blacklistWrap").addClass("none");
		} else {
			$("#blacklistWrap").removeClass("none");
		}

		for (var i = 0; i < blacklist.length; i++) {
			tbody += '<tr><td>' + blacklist[i].macval + '</td><td class="txt-center">' + btnStr.replace("&", i+"") + '</td></tr>';
		}

		$("#blacklist")[0].innerHTML = "<table class='table table-fixed'>" +thead + "<tbody>" + tbody +"</tbody></table>";
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
			
		var thead = "<thead><tr><th width='186px'>主机名称</th><th width='72px'>备注</th><th width='60px'>连接方式</th><th width='63px'>下载速度</th><th width='130px'>网速限制</th><th width='70px'>互联网访问</th></tr></thead>",
			tbody = "";
		if(navigator.vendor && navigator.vendor.indexOf('Apple') !== -1) {
			thead = "<thead><tr><th width='186px'>主机名称</th><th width='72px'>备注</th><th width='60px'>连接方式</th><th width='63px'>下载速度</th><th width='130px'>网速限制</th><th width='90px'>互联网访问</th></tr></thead>";
		}
		if(len === 0) {
			noRecord = "<tr height='37px'><td colspan='6'>当前无活动客户端</td></tr>";
		} else {
			noRecord = "";
		}
		tbody += noRecord;
		for(i = 0; i < len; i++) {
			record = list[i];
			dropDownStr = "<li><a>禁止上网</a></li>";
			textColor = "";
			if(record['ipval'] == data['localip']) {
				hostName = "(本机)" + record['hostname'];
				switchStr = '本机';
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
					coverStr = '<a class="cover" title="开启互联网访问启用此项"></a>';
				} else {
					switchStr = '<a class="switch"></a>';
					coverStr = '<a class="cover none" title="开启互联网访问启用此项"></a>';
				}
			}
			
			if(record['spdlimit'] == "301.00") {
				spd = "无限制";
				record['downloadspd'] += "Mbps";
			} else if(record['spdlimit'] == "0.00") {
				spd = "禁止上网";
				record['downloadspd'] = "--";
			} else {
				spd = record['spdlimit'] + "Mbps";
				record['downloadspd'] += "Mbps";
			}
			
			tr='<tr><td class="' + textColor + '">' +
				'<input type="hidden" name="macval" value="' + record['macval'] + '" >' +
				'<ul><li class="host-name fixed">' + hostName + '</li><li>' +
				'<span class="host-info">IP 地址: ' + record['ipval'] + '</span></li><li>' +
				'<span class="host-info">MAC 地址: ' + record['macval'] + '</span></li></td>' +
				'<td><input type="text" class="input-mini remarkText validatebox" maxlength="20" data-options=\'{"type":"remarkTxt", "args":[",;\\""]}\' value="' + record['remark'] + '"></td>'+
				'<td><div class="text-success">' + record['linktype'] +  '</div></td>' +
				'<td class="fixed">' + record['downloadspd'] + '</td>' +
				'<td><div style="position: relative;">' + coverStr + '</div><div class="input-append"><input class="input-small input-box" type="text" style="width:70px;" value="' + spd + '">' +
				'<div class="btn-group"><button class="toggle btn btn-small"><span class="caret"></span></button></div>' +
				'<div class="input-select">' +
				'<ul class="dropdown-menu"><li><a>无限制</a></li><li><a>0.5Mbps(可正常浏览网页)</a></li><li><a>1.0Mbps(可观看标清视频)</a></li><li><a>2.0Mbps(可观看高清视频)</a></li><li class="divider"></li><li><a class="hand-set">手动设定（Mbps）</a></li></ul>' +
				'</div></div></td>' +
				'<td>' + switchStr + '</td>' +
				'</tr>';
			tbody+=tr;
		}
		$('#netCtrl')[0].innerHTML = "<table class='table table-fixed'>" +thead + "<tbody>" + tbody +"</tbody></table>";
		var $greentds = $('.text-success'),
			tlen = $greentds.length;
		for(i = 0; i < tlen; i++) {
			if($greentds[i].innerHTML == 1) {
				$greentds[i].innerHTML = "无线";
				$greentds.eq(i).removeClass('text-success');
			} else if($greentds[i].innerHTML == 0){
				$greentds[i].innerHTML = "有线";
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
			len = (list.length == 0 ? 0: $trs.length),
			i = 1,
			submitStrArr = [],
			blacklistArr = [],//黑名单列表
			moveBlackListArr = [],//本次从黑名单移除的列表
			submitStr,
			enCtrl = 1;//getValueByName('switcher');
		//if(enCtrl != 0){
		for(;i < len; i++) {
			var $tr = $trs.eq(i),
				mark = $tr.find('td').eq(1).find('input')[0].value,
				mac = $tr.find('td').eq(0).find('input')[0].value,
				speed = $tr.find('td').eq(4).find('input')[0].value,
				accessButton = $tr.find('td').eq(5).find('a');
			
			if(speed === "无限制" || speed === "") {
				speed = "301";
			} else if(speed === "禁止上网") {
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

			//如果为禁止上网，则加到黑名单
			if (speed == 0) {
				blacklistArr.push(mac);
			}
			submitStrArr.push(mac + "," + speed + "," + mark);
		}

		//收集移除黑名单的列表
		$(".blacklist-handle").each(function() {
			var blackIndex = parseInt(this.getAttribute("tindex"), 10),
				allow = this.getAttribute("data-limit") == "allow" ? 1 : 0;

			if (allow == 0) {
				blacklistArr.push(blacklist[blackIndex].macval);
			} else {
				submitStrArr.push(blacklist[blackIndex].macval + ",301,");//取消黑名单，设成无限制加入到普通设备列表
				moveBlackListArr.push(blacklist[blackIndex].macval);
			}
		});


		submitStr = "enablecontrol=" + enCtrl + "&list=" + encodeURIComponent(submitStrArr.join(';'));
		submitStr += "&blacklist=" + encodeURIComponent(blacklistArr.join(' '));
		submitStr += "&moveBlackList=" + encodeURIComponent(moveBlackListArr.join(' '));
		/*} else  {
			submitStr = "enablecontrol=" + enCtrl;
		}*/
		
		//add confirm before apply speedCtrl at 2014.1.26 
		/*if(!confirm("网络将断开几秒使配置生效，是否继续？")) {
			location.href = location.href;
			G.reqOK = 1;
			return;
		}*/
		$('#submitval')[0].disabled = 'disbaled';
		clearTimeout(timer);
		G.ajaxMsg = $.ajaxMassage('数据保存中，请稍候...');
		$.post('/goform/SpeedControlSave', submitStr, function (req){
			G.reqOK = 1;
			if(req.responseText.indexOf('mac_overflow') !== -1) {
				alert("禁止上网主机已满20条，自动将初始条目清除！");
			} else if(req.responseText.indexOf('tc_overflow') !== -1) {
				alert("限速主机已满20条，自动将初始条目清除！");
			} else if(req.responseText.indexOf('mark_overflow') !== -1) {
				alert("备注已满20条，自动将初始条目清除！");
			}
			setTimeout(function () {
				G.ajaxMsg.text("数据保存成功！");
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