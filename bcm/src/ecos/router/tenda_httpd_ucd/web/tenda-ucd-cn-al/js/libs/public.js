function setCheckbox(arry) {
	var i = 0,
		len = arry.length,
		val;
	for(i = 0; i< len; i++) {
		val = $("#"+arry[i]).val();
		if(val == 1) {
			$("#"+arry[i]).attr("checked",true);	
		} else {
			$("#"+arry[i]).attr("checked",false);		
		}
	}
}

function getCheckbox(arry) {
	var i = 0,
		length = arry.length;
	for(i = 0; i < length; i++) {
		if($("#"+arry[i])[0].checked == true) {
			$("#"+arry[i]).val(1);	
		} else {
			$("#"+arry[i]).val(0);	
		}
	}
}

function inputValue(obj) {
	var prop;
	for(prop in obj) {
		$("#"+prop).val(obj[prop]);	
	}	
}

function formatSeconds(value) { 
	var theTime = parseInt(value);// 秒 
	var theTime1 = 0;// 分 
	var theTime2 = 0;// 小时
	var theTime3 = 0;// 天
	// alert(theTime); 
	if(theTime > 60) { 
		theTime1 = parseInt(theTime/60); 
		theTime = parseInt(theTime%60); 
		// alert(theTime1+"-"+theTime); 
		if(theTime1 > 60) { 
			theTime2 = parseInt(theTime1/60); 
			theTime1 = parseInt(theTime1%60); 
			if(theTime2 > 24) {
				theTime3 = parseInt(theTime2/24); 
				theTime2 = parseInt(theTime2%24); 	
			}
		} 
	} 
	var result = ""+parseInt(theTime)+("秒"); 
	if(theTime1 > 0) { 
		result = ""+parseInt(theTime1)+("分钟")+" "+result; 
	} 
	if(theTime2 > 0) { 
		result = ""+parseInt(theTime2)+("小时")+" "+result; 
	} 
	if(theTime3 > 0) { 
		result = ""+parseInt(theTime3)+("天")+" "+result; 
	}
	return result; 
}

function checkIpInSameSegment(eip,emask,ip,mask)
{
	var index=0;
	var eipp="";
	var emaskk="";
	if(typeof(eip)=="object")
		eipp=eip.split(".");
	else
		eipp=eip.split(".");

	if(typeof(emask)=="object")
		emaskk=emask.split(".");
	else
		emaskk=emask.split(".");
	if(ip == '' && mask == '')
		return false;
	var ipp=ip.split(".");
	var maskk=mask.split(".");
	var msk = maskk;
	for(var i=0;i<4;i++)
	{
		if(emaskk[i]==maskk[i]) {
			continue;	
		} else if(emaskk[i]>maskk[i]) {
			msk = maskk;
			break;
		} else {
			msk = emaskk;
			break;	
		}
	}
	for(var i=0;i<4;i++) {	
		if((eipp[i]&msk[i])!=(ipp[i]&msk[i])) {
			return false;
		}
	}
	return true;
}

function objTostring(obj) {
	var prop,
		str = "";
	for(prop in obj) {
		str += prop + "=" + encodeURIComponent(obj[prop]) + "&";
	}
	str = str.replace(/[&]$/,"");
	return str;
}

function showErrMsg(id,str) {
	$("#"+id).html(str);
	setTimeout(function() {
		$("#"+id).html("&nbsp;");
	},2000)
}

function controlIframe(width,height) {
	var v_width = $.viewportWidth(),
		v_height = $.viewportHeight();
	$(".main-dailog").css("width",width+"px");
	$(".dailog-iframe").css("width",width+"px");	
	$(".dailog-iframe").css("height",height+"px");
	$(".main-dailog").css("left",(v_width-width)/2+"px");
	$(".main-dailog").css("top",(v_height-height-50)/2+"px");
}

function showSaveMsg(num,str,flag) {
	var str = str || "设置保存中...";
	flag = flag || "-1";
	$(".save-msg").removeClass("none");
	$("#gbx_overlay").remove();
	$("<div id='gbx_overlay'></div>").appendTo("body");	
	if(num == 0) {
		if(!$(".main-dailog").hasClass("none")) {
			$(".main-dailog").addClass("none");
		}
		$("#page-message").html(str);
		if(flag == "-1") {
			setTimeout(function() {
				$(".save-msg").addClass("none");
				$("#gbx_overlay").remove();
			},2000);
		} else if(flag == "1") {
			$(".save-loading").addClass("hidden");
			setTimeout(function() {
				$(".save-msg").addClass("none");
				$("#gbx_overlay").remove();
				$(".save-loading").removeClass("hidden");
			},1000);	
		}
	} else if(num == "999") { 
		$("#page-message").html("保存失败");
		$("#page-message").addClass("none");
		top.location.reload(true);	
	} else{
		if(!$(".main-dailog").hasClass("none")) {
			$(".main-dailog").addClass("none");
		}
		$("#page-message").html("保存失败");
		setTimeout(function() {
			$(".save-msg").addClass("none");
			$("#gbx_overlay").remove();
		},1000);
	}	
}

/***********************
	*重启、升级、恢复出厂设置、还原等操作
	*str: 操作的动作 
	
**********************/
var pc = 0;
var upgradeTime = 0,
	rebootTime = 0;
(function($){
	$.progress = {
		showPro: function(str, str2,ip) {
			str2 = str2 || ("设备正在重启中，请稍后...");
			ipaddress = "";
			ipaddress = ip || "";
			$("<div id='gbx_overlay'></div>").appendTo("body");
			var html = '<div id="loading_div" >' +
							'<div id="up_contain">'+
							'<span class="upgrading"><span class="upgrade_pc"></span></span><br />'+("设备升级中，请不要关闭电源")+'<span id="upgrade_text"></span>'+
							'</div>' +
							'<span id="load_pc" class="up-loadding load-reboot"></span><br />'+str2+'<span id="load_text"></span>' +
						'</div>';
			$(html).appendTo("body");
			$this_obj = $("#loading_div")
			$("#loading_div").css("left", ($.viewportWidth()-$this_obj.width())/2);
			$("#loading_div").css("top", ($.viewportHeight()-$this_obj.height())/2);
			$this_obj.css("z-index",3000);
			$this_obj.css("position","fixed");
			switch(str) {
				case "upgrade":
				case "reboot":
				case "restore":
				case "restoreDefault":
					$("#up_contain").addClass("none");
					rebooting(700);
					break;
				case "uploadFile":
					$("#up_contain").addClass("none");
					rebooting(50);
					break;
			}
		}
	}
})(jQuery);

function rebooting(time) {
	time = 900 || 200;
	if(pc <= 100) {
		clearTimeout(rebootTime);
		rebootTime = setTimeout('rebooting('+time+')', time);
		//$(".load_pc").css("width", pc+'%');
		$("#load_pc").attr("class","up-loadding load-reboot step"+(pc%5));
		$("#load_text").html(pc+'%');
		pc++;
	} else {
		clearTimeout(rebootTime);
		pc = 1;
		$("#gbx_overlay").remove();
		$("#loading_div").remove();	
		if(ipaddress != "" && (/^([1-9]|[1-9]\d|1\d\d|2[0-1]\d|22[0-3])\.(([0-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.){2}([0-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])$/).test(ipaddress) == true) {
			top.location = "http://"+ipaddress;	
		} else {
			//window.location = location;
			top.location.reload(true);
		}
	}
}
