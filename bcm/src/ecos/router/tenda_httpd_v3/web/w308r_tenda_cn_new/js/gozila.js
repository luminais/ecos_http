function macsCheck(I,s){
    var m =/([0-9a-fA-F]{2}\:){5}[0-9a-fA-F]{2}/;
    if(I.length!=17 || !m.test(I)){
   	    alert(s+" 无效 !!!");
   	    I.value = I.defaultValue;
   	    return false
    }
    return true
}

function toNextMac(f, obj, nextIndex){
	obj.value = obj.value.replace(/[^0-9a-fA-F]/g, '');
	if(obj.value.length === 2 && nextIndex) {
		nextIndex = parseInt(nextIndex, 10);
		f.elements[obj.id][nextIndex].focus();
	}
}
function ill_check(val, ill_array, str) {
	var i = 0;
	for(; i< ill_array.length; i++) {
		if (val.indexOf(ill_array[i]) != -1) {
			alert(str + " 有非法字符: "+ill_array[i]);
			return false;
		}
	}
	return true;
}

function isNumber(val, label) {
	var t = /^[0-9]{1,}$/;
	if (!t.test(val)) {
		alert(label + " 必须是数字!!!");
		return false;
	}
	return true ;
}

function rangeCheck(elem, min, max, label) {
	var val = elem.value;
	
	if (!isNumber(val, label)) {
		return false;
	}
	if ((val < min) || (val > max)) {	
		alert(label + " 超出范围 !!!") ;
		elem.value = v.defaultValue ;
		return false;
	} else {
		return true;
	}
}

function chkStrLen(s,m,M,msg){
	var str = s.value;
	if (str.length < m || str.length > M ){
		alert(msg+" 长度不在范围内!!!");
		return false;
	}
    return true;
}
//add by stanley
function ipCheck(srcip,dstip,mask) {
  	var sip = srcip.split('.'),
		dip = dstip.split('.'),
		lanmsk = mask.split('.');
	if(sip[0] == dip[0]){
		if(sip[0]<128 && lanmsk[1] == "0")
			return true;	
	}else{
		return false;
	}
	
	if(sip[1] == dip[1]){
		if(sip[0]<192 && lanmsk[2] == "0")
			return true;
	}else{
		  return false;
	}

	if(sip[2] == dip[2]){
		if(sip[0]<224 && lanmsk[3] == "0")
			return true;
	}else{
		  return false;
	}
}
//end


function refresh(destination) {
   window.location = destination ;		
}

function decomList(str,len,idx,dot) {  
	var t = str.split(dot);
	return t[idx];
}


function verifyIP0(ipa,msg) {
	var ip=combinIP2(ipa);
	if (ip=='' || ip=='0.0.0.0'){
		return true;
	}
	return verifyIP2(ipa,msg);
}
function verifyIP2(ipa, msg, subnet) {
	var //tip = /^[0-9]{1,}$/,
		ip = [],
		i,
		tip = /^(25[0-5]|2[0-4]\d|1\d\d|[1-9]\d?)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}$/;
		
	if (ipa.length==4){
		if (ipa[0].value) {
			for (var i=0; i<4; i++) {
				ip[i] = ipa[i].value;
			}
		} else {
			ip = ipa;
		}
	} else if (ipa.value) {
		ip = ipa.value.split(".");
	} else if (typeof ipa === 'string') {
		ip = ipa.split('.');
	}

	if(ip.length != 4){
		alert(msg+"无效 !!!");
		return false;
	}
	/*
	for(i = 0; i < 4; i++){
		if(!tip.test(ip[i])){
			alert(msg+"包含非法字符!!!") ;
			return 0;
		}
	}
	*/
	if(ip[0] == 0 || ip[0] == 127){
		alert(msg+"无效 !!!");
		return false;	
	}
	if(ip[1] == "" || ip[2] == ""){
		alert(msg+"无效 !!!");
		return false;	
	}
	
	if(ip[1] == " " || ip[2] == " "){
		alert(msg+"无效 !!!");
		return false;	
	}

	if(ip[0] >=224) {
		alert(msg+"无效 !!!");
		return false;
	}
 /*
    for (var i = 0; i < 4; i++){
        d = ip[i];
        if (d < 256 && d >= 0) {
			if (i != 3 || subnet == 1){
				continue;
			} else {
 				if (d != 255 && d !=0 )
				continue;
			}
						
        }	
        alert(msg+"无效 !!!");
        return false;
    }*/
	if(!tip.test(ipa.value))
	{
		alert(msg + "无效！");
		return false;
	}
	//只用于验证标准ip分类地址
	/*
	if (ip[0] < 128) {
		if (ip[1] == 0 && ip[2] == 0 && ip[3] == 0) {
			alert(msg + "无效，主机部分为全0！");
			return false;
		} else if (ip[1] == 255 && ip[2] == 255 && ip[3] == 255) {
			alert(msg + "无效，主机部分为全1！");
			return false;
		}
	} else if (ip[0] < 192) {
		if (ip[2] == 0 && ip[3] == 0) {
			alert(msg + "无效，主机部分为全0！");
			return false;	
		} else if (ip[2] == 255 && ip[3] == 255) {
			alert(msg + "无效，主机部分为全1！");
			return false;
		}
	} else if (ip[0] < 224) {
		if (ip[3] == 0) {
			alert(msg + "无效，主机部分为全0！");
			return false;
		} else if (ip[3] == 255) {
			alert(msg + "无效，主机部分为全1！");
			return false;
		}	
	}
	*/
    return true;
}

function clearInvalidIpstr(ipa) {
	var ip = [],
		i;
	if(ipa == ""){
		return ipa;	
	}
	ip=ipa.split(".");
	if(ip.length<4){
		return parseInt(ipa,10).toString();	
	}
	for(i=0;i<4;i++) {
		var tmp = parseInt(ip[i],10);
		ip[i] = tmp.toString();
	}
	ipa=ip[0]+'.'+ip[1]+'.'+ip[2]+'.'+ip[3];
	return ipa;	
}


function decomMAC2(ma,macs,nodef) {
    var re = /^([0-9a-fA-F]{2}:){5}[0-9a-fA-F]{2}$/,
		d;
    if (re.test(macs)||macs=='') {
		if (ma.length!=6) {
			ma.value=macs;
			return true;
		}
		if (macs!='') {
			d=macs.split(":");
		} else {
			d=['','','','','',''];
		}
        for (i = 0; i < 6; i++) {
            ma[i].value = d[i];
			if (!nodef) {
				ma[i].defaultValue = d[i];
			}
		}
        return true;
    }
    return false;
}

function decomIP2(ipa,ips,nodef) {
    var re = /^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$/,
		d;
    if (re.test(ips)) {
        d =  ips.split(".");
        for (i = 0; i < 4; i++) {
            ipa[i].value=d[i];
			if (!nodef) ipa[i].defaultValue=d[i];
		}
        return true;
    }
    return false;
}

function combinIP2(d) {
	var ip;
	if (d.length!=4) {
		return d.value;
	}
    ip=d[0].value+"."+d[1].value+"."+d[2].value+"."+d[3].value;
    if (ip == "...") {
        ip="";
	}
    return ip;
}
function combinMAC2(m) {
    var mac = m[0].value.toUpperCase()+":"+m[1].value.toUpperCase()+":"+ 
			m[2].value.toUpperCase()+":"+m[3].value.toUpperCase()+":"+
			m[4].value.toUpperCase()+":"+m[5].value.toUpperCase();
    if (mac==":::::"){
        mac="";
	}
    return mac;
}

function ipMskChk(ipa, str){
	var t = /^[0-9]{1,}$/,
		m = [],
		f = 0,
		v;
		
	if (ipa.length==4){
		if (ipa[0].value) {
			for (var i=0; i<4; i++) {
				m[i] = ipa[i].value;
			}
		} else {
			m = ipa;
		}
	} else if (ipa.value) {
		m = ipa.value.split(".");
	} else if (typeof ipa === 'string') {
		m = ipa.split('.');
	}
	
	if (m.length!=4) {
		alert(str+" 无效 !!!");
		return 0;
	}
    for(i=0;i<4;i++){
		if(!t.test(m[i])){
			alert(str+"包含非法字符 !!!") ; return 0;
		}
	}
	v = (m[0]<<24)|(m[1]<<16)|(m[2]<<8)|(m[3]);
  
   	for (k=0;k<32;k++) {
		if ((v>>k)&1) {
			f = 1;
		} else if (f==1){
			alert(str+" 无效 !!!") ;
			return 0 ;
		}
	}
	if (f==0) {
		alert(str+" 无效 !!!");
		return 0;
	}
	return 1 ;
}

function Cfg(i,n,v){
	this.i = i;
    this.n = n;
    this.v = this.o = v;
}

var CA = [];

function addCfg(n,i,v) {
	CA.length++;
    CA[CA.length-1]= new Cfg(i,n,v);
}

function idxOfCfg(kk) {
    if (typeof kk == "undefined") {
		alert("没有定义");
		return -1;
	}
    for (var i=0; i< CA.length;i++) {
        if (typeof CA[i].n != "undefined" && CA[i].n == kk )
            return i;
    }
    return -1;
}

function getCfg(n) {
	var idx = idxOfCfg(n)
	if ( idx >=0) {
		return CA[idx].v;
	} else {
		return "";
	}
}

function setCfg(n,v) {
	var idx = idxOfCfg(n)
	if ( idx >=0) {
		CA[idx].v = v ;
	}
}

//从配置解析到表单
function cfg2Form(f) {
	var e;
    for (var i=0;i < CA.length;i++){
        e = f[CA[i].n];
        if (e) {
			if (typeof e.name == "undefined" &&  typeof e[0].name == "undefined"){
				continue;
			}
			
			if (e.length && e[0].type == "text") {
				if (e.length==4) {
					decomIP2(e,CA[i].v);
				} else if (e.length==6) {
					decomMAC2(e,CA[i].v);
				}
			} else if (e.length && e[0].type == "radio") {
				for (var j=0;j < e.length;j++) {
					e[j].checked = e[j].defaultChecked = (e[j].value == CA[i].v);
				}
			} else if (e.type=="checkbox") {
				e.checked=e.defaultChecked=Number(CA[i].v);
			} else if (e.type=="select-one") {
				for (var j=0;j<e.options.length;j++) {
					e.options[j].selected = e.options[j].defaultSelected =
							(e.options[j].value==CA[i].v);
				}
			} else {
				e.value=getCfg(e.name);
			}
			if (e.defaultValue != "undefined") {
				e.defaultValue = e.value;
			}
		}
    }
}

//从表单提交到配置
var frmExtraElm='';
function form2Cfg(f){
	var e;
    for (var i=0;i<CA.length;i++){
        e = f[CA[i].n];
		if (e) {
			if (e.disabled) continue;
			if ( e.length && e[0].type=="text" ) {
				if (e.length==4) CA[i].v=combinIP2(e);
				else if (e.length==6) CA[i].v=combinMAC2(e);
			} else if ( e.length && e[0].type=="radio") {
				for (var j=0;j<e.length;j++) {
					if (e[j].checked) {
						CA[i].v=e[j].value;
						break;
					}
				}
			}
			else
			if (e.type=="checkbox")
				setCfg(e.name, Number(e.checked) );
			else
				setCfg(e.name, e.value);
		}
    }
}

function fit2(n) {
	var s = String(n+100).substr(1,2);
	return s;
}

function timeStr(t){
	if (t < 0) {
		str = '00:00:00';
		return str;
	}
	var s=t%60,
		m=parseInt(t/60)%60,
		h=parseInt(t/3600)%24,
		d=parseInt(t/86400),
		str='';
	
	if (d > 999) {
		return '永久';
	}
	if (d) {
		str +=d+'天 ';
	}
	str += fit2(h) + ':';
	str += fit2(m) + ':';
	str += fit2(s);
	return str;
}

function rmEntry(a,i){
	if (a.splice) {
		a.splice(i,1);
	} else {
		if (i>=a.length) {
			return;
		}
		for (var k=i+1;k<=a.length;k++){
			a[k-1]=a[k];
		}
		a.length--;
	}
}

//1
function checkText(str) {
	var re = /^[A-Za-z0-9_]+$/;
	if (str === "") {
		return true;
	}
	return re.test(str);
}

//1 mac_clone
function CheckMAC(mac) {	
	var re = /^([0-9a-fA-F]{2}:){5}[0-9a-fA-F]{2}$/;
    if (!re.test(mac)) {
        return false;
    }
    return true;
}


//1mac_tc
//mac ip url 过滤 初始化
function init2(f) {
	var n = getCfg("curNum");
	if(n == "") {
		n = 1;
	}
		
	if(getCfg("check_en") == 1) {
		document.getElementById("check").checked = true;
	} else {
		document.getElementById("check").checked = false;
	}
	onCheck();

	f.curNum.selectedIndex = n-1;
	onChangeNum(f);
}

//异步传输生成对象
function GetReqObj(){
	var req = null;	
	try{
		req = new XMLHttpRequest();
		if(req.overrideMimeType) {
			req.overrideMimeType("text/xml");
		}
	} catch (trymicrosoft){
		try {
			req = new ActiveXObject("MSXML2.XMLHTTP");
		} catch (othermicrosoft) {
			try {
				req = new ActiveXObject("Microsoft.XMLHTTP");
			} catch (failed) {
				req = false;
			}
		}
	 }	
	if(!req) {
		alert("Error initializing HttpRequest!");
		return false;
	}	
	return req;
}

function IsReboot() {
	if(confirm("重启路由器使设置生效，是否立即重启")) {	
		return 1;
	} else {
		return 0;
	}
}

function ckRemark(s) {
	var re = /^\w{0,}$/;
	if(!re.test(remark)) {
		alert("注释只能由数字、字母、下划线组成");
		return false;
	}	
	return true;
}

function ckMacReserve(mac){
	var m = mac.split(":");	
	for(var i=0;i<6;i++) {
		m[i] = m[i].toUpperCase();
	}
	if(m =="00:00:00:00:00:00" || m == "FF:FF:FF:FF:FF:FF") {
		alert("MAC地址为保留地址,请重新输入。")	;
		return false;
	}
	
	if(parseInt(m[0],16)&1 == 1) {//m[0]最后一位是否为1{
		alert("MAC地址为组播地址,请重新输入。")	;
		return false;
	}
	return true;
}

function show_hide(el,shownow) {
	document.getElementById(el).style.display = (shownow) ? '' : 'none' ;
}

function tenda_ipMskChk(mn,str,lanip) {
	var m = [],
		lanipi =[],
		t = /^[0-9]{1,}$/,
		f = 0;
	if (mn.length==4) {
		for (i=0;i<4;i++){
			m[i]=mn[i].value;
		}
	} else {
		m=mn.value.split('.');
		if (m.length!=4) { alert(str+" 无效 !!!") ; return 0; }
	}
	for(i=0;i<4;i++) {
		if(!t.test(m[i])) {
			alert(str+"包含非法字符 !!!");
			return 0;
		}
	}
//add end

	var v=(m[0]<<24)|(m[1]<<16)|(m[2]<<8)|(m[3]);
  
   	for (k=0;k<32;k++){
		if ((v>>k)&1){
			f = 1;
		} else if (f==1){
			alert(str+" 无效 !!!") ;
			return 0 ;
		}
	}
	if (f==0) { alert(str+" 无效 !!!") ; return 0; }
//add by stanley
	lanipi = lanip.value.split('.');
	if(lanipi[0]<128) {
		if(m[0]<255){
			alert(str+" 无效, 不支持网络聚集 !");
			return 0;
		}
		if(m[1]>0 || m[2]>0 || m[3]>0){
			alert(str+" 无效, 不支持子网划分 !");
			return 0;
		}	
	}else if(lanipi[0]<192){
		if(m[0]<255 || m[1]< 255 ){
			alert(str+" 无效, 不支持网络聚集 !");
			return 0;
		}
		if( m[2]>0 || m[3]>0){
			alert(str+" 无效, 不支持子网划分 !");
			return 0;
		}	
	}else if(lanipi[0]<224){
		if(m[0]<255 || m[1]< 255 || m[2]< 255){
			alert(str+" 无效, 不支持网络聚集 !");
			return 0;
		}
		if( m[3]>0){
			alert(str+" 无效, 不支持子网划分 !");
			return 0;
		}	
	}
//end
	return 1 ;
}
function  detectCapsLock(event){  
    var e = event||window.event, 
		o = e.target||e.srcElement,  
		oTip = o.nextSibling, 	
		keyCode  =  e.keyCode||e.which; 
	if ( keyCode >= 65 && keyCode <= 90){
		oTip.style.display = '';
		setTimeout(function () {
			document.getElementById("uCL").style.display = 'none';
		},1000);
	} else {
		oTip.style.display = 'none';
	} 
}

function showSaveMassage(){
	var div = document.getElementById("save");
	
	if (div) {
		div.className = "";
	} else {
		div = document.createElement("div");
		document.getElementById("body")[0].appendChild(div);
	}
	div.innerHTML = '设置保存成功！';
	setTimeout(function () {
		var div = document.getElementById("save");
		div.className = "none";
	},1000);
}

function Click(){ 
	window.event.returnValue=false; 
} 
document.oncontextmenu = Click; 

function tbl_tail_str(button){
	var m='<table width="75%" border="0" cellpadding="0" cellspacing="0"><tr><td height="40">&nbsp;';
	if (button!=''){
		m+=button;
	    m+='</td></tr>';
	}
	m+='</table>';
	return m;
}
function tbl_tail(button){
	document.write(tbl_tail_str(button));
}
function tbl_tail_save(f){
	var m = '<div class="btn-group">';
	m += '<button type="button" class="btn" onClick=preSubmit(' + f + ')>确 定</button>';
	m += '<button type="button" class="btn" onClick=init(' + f + ')>取 消</button>';
	m += '</div>';
	document.write(m);
}
function checkSSID(SSID)
{		
	var ascii = /^[ -~]+$/g;
	if(SSID == "" || SSID.length > 32 || !ascii.test(SSID)) {
		return false;
	}
	return true;
}
function decodeSSID(SSID){
    var e = document.createElement("div"),
		deSSID = '';
    e.innerHTML = SSID.replace(/\x20/g,"\xA0");
	if(e.innerText){
		deSSID = e.innerText;
	} else if (e.textContent) {
		deSSID = e.textContent;
	}
    e = null;
    return deSSID.replace(/\xA0/g,"\x20");
}

function checkVerifyIp(ip,mask,msg)
{
	var ips = ip.split(".");
	var msks = mask.split(".");
	var tmp = 0;
	
	tmp = (parseInt(ips[0])&parseInt(msks[0]))*256;
	tmp = (tmp+(parseInt(ips[1])&parseInt(msks[1])))*256;
	tmp = (tmp+(parseInt(ips[2])&parseInt(msks[2])))*256;
	tmp = tmp+(parseInt(ips[3])&parseInt(msks[3]));
	var netaddr = tmp;
	
	tmp = (parseInt(ips[0]))*256;
	tmp = (tmp+(parseInt(ips[1])))*256;
	tmp = (tmp+(parseInt(ips[2])))*256;
	tmp = tmp+(parseInt(ips[3]));
	var ipaddr = tmp;
	
	if (ipaddr <= netaddr) {
		alert(msg + "无效，主机部分为全0！");	
		return false;
	}
	
	tmp = (parseInt(msks[0]))*256;
	tmp = (tmp+(parseInt(msks[1])))*256;
	tmp = (tmp+(parseInt(msks[2])))*256;
	tmp = tmp+(parseInt(msks[3]));
	var mask = tmp;
	tmp = (parseInt(ips[0])&(~parseInt(msks[0])))*256;
	tmp = (tmp+(parseInt(ips[1])&(~parseInt(msks[1]))))*256;
	tmp = (tmp+(parseInt(ips[2])&(~parseInt(msks[2]))))*256;
	tmp = tmp+(parseInt(ips[3])&(~parseInt(msks[3])));
	var revrsnet =  tmp;
	var judgeNum = parseInt(mask+revrsnet);
	if(judgeNum == 4294967295)
	{
		alert(msg + "无效，主机部分为全1！");
		return false;
	}
	return true;
}