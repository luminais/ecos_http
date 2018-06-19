function macsCheck(I,s){
    var m = /[0-9a-fA-F\:]{17}/;
    if(I.length!=17 || !m.test(I)){
   	    alert(s+" 无效 !!!");
   	    I.value = I.defaultValue;
   	    return false
    }
    return true
}
function ill_check(val, ill_array, str)
{
	var i = 0;
	for(; i< ill_array.length; i++)
	{
		if (val.indexOf(ill_array[i]) != -1)
		{
			alert(str+" 有非法字符: "+ill_array[i]);
			return false;
		}
	}
	return true;
}
function validNumCheck(v,m) {
	var t = /^[0-9]{1,}$/;
	if (!t.test(v.value))
	{
		alert(m+" 不是数字 !!!");
		v.value=v.defaultValue;
		return 0;
	}
	return 1 ;
}

function IsNumCheck(v) {
	var t = /^[0-9]{1,}$/;
	if (!t.test(v))
	{
		alert(v +" 不是数字!!!");
		return false;
	}
	return true ;
}

function rangeCheck(v,a,b,s) {
   if (!validNumCheck(v,s)) return 0;          
   if ((v.value<a)||(v.value>b)) {	
      alert(s+" 超出范围 !!!") ;
      v.value=v.defaultValue ;
      return 0 ;
   } else return 1 ;
}
//add by stanley
function ipCheck(srcip,dstip,mask) {
  	var sip = srcip.split('.');
	var dip = dstip.split('.');
	var lanmsk = mask.split('.');
	if( sip[0] == dip[0]){
		if(sip[0]<128 && lanmsk[1] == "0")
			return true;	
	}else{
		return false;
	}
	
	if( sip[1] == dip[1]){
		if(sip[0]<192 && lanmsk[2] == "0")
			return true;
	}else{
		  return false;
	}

	if( sip[2] == dip[2]){
		if(sip[0]<224 && lanmsk[3] == "0")
			return true;
	}else{
		  return false;
	}
}
//end

function strCheck(s,msg) {
	return true;
}

function scCheck(s,msg) {
	var ck=/[\;]/;
	if (ck.test(s.value))
	{
		alert(msg+" 包含无效字符: \;");
		return false;
	}
	return true;
}

function refresh(destination) {
   window.location = destination ;		
}

function decomList(str,len,idx,dot) {  
	var t = str.split(dot);
	return t[idx];
}

function setCheckValue(t) {   	
   if (t.checked) t.value=1 ;
   else t.value=0 ;	
}

function preLogout() {	
   if (confirm('确信要退出吗？')) {                    
     window.top.close();
   }
}	


function combinMAC(m1,m2,m3,m4,m5,m6)
{
    var mac=m1.value+":"+m2.value+":"+m3.value+":"+m4.value+":"+m5.value+":"+m6.value;
    if (mac==":::::")
        mac="";
    return mac;
}

function verifyIP0(ipa,msg)
{
	var ip=combinIP2(ipa);
	if (ip=='' || ip=='0.0.0.0') return true;
	return verifyIP2(ipa,msg);
}
function verifyIP2(ipa,msg,subnet)
{
	var tip = /^[0-9]{1,}$/;
	ip = new Array();
	if (ipa.length==4)
	{
		for (var i=0;i<4;i++)
			ip[i]=ipa[i].value;
	}
	else
		ip=ipa.value.split(".");

	if(ip.length != 4){
		alert(msg+"无效 !!!");
		return false;
	}
	
	for(i=0;i<4;i++)
	{
		if(!tip.test(ip[i]))
		{
			alert(msg+"包含非法字符!!!") ; return 0;
		}
	}
    //if(ip[0] == 0 || ((ip[0] == 127) && (ip[3] == 0))) 	
	if(ip[0] == 0 ||ip[0] == 127)
	{
		alert(msg+"无效 !!!");
		return false;	
	}

	if(ip[1] == "" || ip[2] == "")
	{
			alert(msg+"无效 !!!");
		return false;	
	}
	
	if(ip[1] == " " || ip[2] == " ")
	{
			alert(msg+"无效 !!!");
		return false;	
	}

	if(ip[0] >=224)
	{
		alert(msg+"无效 !!!");
		return false;
	}
 
    for (var i = 0; i < 4; i++)
    {
        d = ip[i];
        if (d < 256 && d >= 0)
        {
			if (i!=3 || subnet==1)
			{
				continue;
			}
			else
			{
 				if (d != 255 && d !=0 )
					continue;
			}
						
        }	
        alert(msg+"无效 !!!");
        return false;
    }
    return true;
}

function verifyPPTPmask(ipa,msg,subnet)
{
	ip = new Array();
	
	if (ipa.length==4)
	{
		for (var i=0;i<4;i++)
			ip[i]=ipa[i].value;
	}
	else
		ip=ipa.value.split(".");
	
	if (ip[0]==0 && ip[1]==0 && ip[2]==0 && ip[3]==0) return true;
		
	if((ip[0] == 127) && (ip[3] == 0))
	{
		alert(msg+"无效 !!!");
		return false;	
	}

    for (var i = 0; i < 4; i++)
    {
        d = ip[i];
        if (d < 256 && d >= 0)
        {
			if (i!=3 || subnet==1)
				continue;
			else
			{
 				if (d != 255 && d !=0 )
					continue;
			}
        }	
        alert(msg+"无效 !!!");
        return false;
    }
    return true;
}

function verifyMAC(ma,s,sp)
{
    var t =/^([0-9a-fA-F][0-9a-fA-F]:){5}([0-9a-fA-F][0-9a-fA-F])$/;

	if (!t.test(ma.value)) 
	{
	 alert(s+" 无效!!!"); 
	 return false;
	}
    return true
}

function decomMAC2(ma,macs,nodef)
{
    var re = /^[0-9a-fA-F]{1,2}:[0-9a-fA-F]{1,2}:[0-9a-fA-F]{1,2}:[0-9a-fA-F]{1,2}:[0-9a-fA-F]{1,2}:[0-9a-fA-F]{1,2}$/;
    if (re.test(macs)||macs=='')
    {
		if (ma.length!=6)
		{
			ma.value=macs;
			return true;
		}
	if (macs!='')
        	var d=macs.split(":");
	else
		var d=['','','','','',''];
        for (i = 0; i < 6; i++)
		{
            ma[i].value=d[i];
			if (!nodef) ma[i].defaultValue=d[i];
		}
        return true;
    }
    return false;
}

function decomIP2(ipa,ips,nodef)
{
    var re = /^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$/;
    if (re.test(ips))
    {
        var d =  ips.split(".");
        for (i = 0; i < 4; i++)
		{
            ipa[i].value=d[i];
			if (!nodef) ipa[i].defaultValue=d[i];
		}
        return true;
    }
    return false;
}

function combinIP2(d)
{
	if (d.length!=4) return d.value;
    var ip=d[0].value+"."+d[1].value+"."+d[2].value+"."+d[3].value;
    if (ip=="...")
        ip="";
    return ip;
}
function combinMAC2(m)
{
	var t = /^[0-9a-fA-F]{1,}$/;
	for( var i=0;i<6;i++ )
	{
		if(!t.test(m[i].value))
		{
			alert("输入的MAC地址包含非法字符或无效字符，请重新输入 !!!") ; return 0;
		}
	}
    var mac=m[0].value.toUpperCase()+":"+m[1].value.toUpperCase()+":"+m[2].value.toUpperCase()+":"+m[3].value.toUpperCase()+":"+m[4].value.toUpperCase()+":"+m[5].value.toUpperCase();
    if (mac==":::::")
        mac="";
    return mac;
}

function ipMskChk(mn,str)
{
	var t = /^[0-9]{1,}$/;
	var m=new Array();
	if (mn.length==4)
		for (i=0;i<4;i++) m[i]=mn[i].value;
	else
	{
		m=mn.value.split('.');
		if (m.length!=4) { alert(str+" 无效 !!!") ; return 0; }
	}
    for(i=0;i<4;i++)
	{
		if(!t.test(m[i]))
		{
			alert(str+"包含非法字符 !!!") ; return 0;
		}
	}
	var v=(m[0]<<24)|(m[1]<<16)|(m[2]<<8)|(m[3]);

   	var f=0 ;	  
   	for (k=0;k<32;k++)
	{
		if ((v>>k)&1) f = 1;
		else if (f==1)
		{
			alert(str+" 无效 !!!") ;
			//for(var i=0; i<4; i++) m[i].value=m[i].defaultValue;
			return 0 ;
		}
	}
	if (f==0) { alert(str+" 无效 !!!") ; return 0; }
	return 1 ;
}

function Cfg(i,n,v)
{
	this.i=i;
    this.n=n;
    this.v=this.o=v;
}

var CA = new Array() ;

function addCfg(n,i,v)
{
	CA.length++;
    CA[CA.length-1]= new Cfg(i,n,v);
}

function idxOfCfg(kk)
{
    if (kk=='undefined') { alert("没有定义"); return -1; }
    for (var i=0; i< CA.length ;i++)
    {
        if ( CA[i].n != 'undefined' && CA[i].n==kk )
            return i;
    }
    return -1;
}

function getCfg(n)
{
	var idx=idxOfCfg(n)
	if ( idx >=0)
		return CA[idx].v ;
	else
		return "";
}

function setCfg(n,v)
{
	var idx=idxOfCfg(n)
	if ( idx >=0)
	{
//debug, if (CA[idx].v != v) alert("setCfg("+n+","+v+")");
		CA[idx].v = v ;
	}
}

//从配置解析到表单
function cfg2Form(f)
{
    for (var i=0;i<CA.length;i++)
    {
        var e=eval('f.'+CA[i].n);
        if ( e )
		{
			if (e.name=='undefined') continue;
			if ( e.length && e[0].type=='text' )
			{
				if (e.length==4) decomIP2(e,CA[i].v);
				else if (e.length==6) decomMAC2(e,CA[i].v);
			}
			else if ( e.length && e[0].type=='radio')
			{
				for (var j=0;j<e.length;j++)
					e[j].checked=e[j].defaultChecked=(e[j].value==CA[i].v);
			}
			else if (e.type=='checkbox')
				e.checked=e.defaultChecked=Number(CA[i].v);
			else if (e.type=='select-one')
			{
				for (var j=0;j<e.options.length;j++)
					 e.options[j].selected=e.options[j].defaultSelected=(e.options[j].value==CA[i].v);
			}
			else
				e.value=getCfg(e.name);
			if (e.defaultValue!='undefined')
				e.defaultValue=e.value;
		}
    }
}

//从表单提交到配置
var frmExtraElm='';
function form2Cfg(f)
{
    for (var i=0;i<CA.length;i++)
    {
        var e=eval('f.'+CA[i].n);
		if ( e )
		{
			if (e.disabled) continue;
			if ( e.length && e[0].type=='text' )
			{
				if (e.length==4) CA[i].v=combinIP2(e);
				else if (e.length==6) CA[i].v=combinMAC2(e);
			}
			else if ( e.length && e[0].type=='radio')
			{
				for (var j=0;j<e.length;j++)
					if (e[j].checked) { CA[i].v=e[j].value; break; }
			}
			else
			if (e.type=='checkbox')
				setCfg(e.name, Number(e.checked) );
			else
				setCfg(e.name, e.value);
		}
    }
}

var OUTF;
function frmHead(na,to,cmd,go)
{
	OUTF="<FORM name="+na+" action="+to+" method=POST>\n"+
	"<INPUT type=hidden name=CMD value="+cmd+">\n"+
	"<INPUT type=hidden name=GO value="+go+">\n";
}

function frmEnd()
{
	OUTF+="</FORM>\n";
}

function frmAdd(n,v)
{
	set1="<input type=hidden name="+n+" value=\"";
	v=v.replace(/\"/g,"&quot;");
	var r=new RegExp(set1+".*\n","g");
	if (OUTF.search(r) >= 0)
		OUTF=OUTF.replace(r,(set1+v+"\">\n"));
	else
		OUTF += (set1+v+"\">\n");
}

function genForm(n,a,d,g)
{
	frmHead(n,a,d,g);
	var sub=0;
    for (var i=0;i<CA.length;i++)
	{
		if (CA[i].v!=CA[i].o)
		{
			frmAdd("SET"+sub,String(CA[i].i)+"="+CA[i].v);
			sub++;
		}
	}
	if (frmExtraElm.length)
		OUTF+=frmExtraElm;
	frmExtraElm=''; //reset
	frmEnd();
	return OUTF;
}

function subForm(f1,a,d,g)
{
	var msg=genForm('OUT',a,d,g);
/*DEMO*/
	if (!confirm(msg)) return;
/*END_DEMO*/

	var newElem = document.createElement("div");
	newElem.innerHTML = msg ;
	f1.parentNode.appendChild(newElem);
	f=document.OUT;
	f.submit();
}

function addFormElm(n,v)
{
	var set1='<input type=hidden name='+n+' value="'+v+'">\n';
	frmExtraElm += set1;
}

function pwdSame(p,p2)
{
   	if (p != p2) { alert("密码不一致!") ; return 0 ; }
   	else return 1 ;
}

function chkPwdUpdate(p,pv,c)
{
	if (c.value=='0') return true;
    // modified
    if (!pwdSame(p.value,pv.value)) return false;
    if (!confirm('改变密码吗？'))
    {
        c.value=0 ;
        p.value=pv.value=p.defaultValue;
        return false;
    }
    return true;
}


function chkPwd1Chr2(po,p,pv,c)
{
   	if (c.value=='0')
   	{
  		po.value=p.value=pv.value=""; // reset to null;
  		c.value='1';
	}
}

function chkStrLen(s,m,M,msg)
{
	var str=s.value;
	if ( str.length < m || str.length > M )
	{
		alert(msg+" 长度不在范围内!!!");
		return false;
	}
    return true;
}

function isIE()
{
	var agt = navigator.userAgent.toLowerCase();
	return (agt.indexOf("msie") != -1); // ie
}

function fit2(n)
{
	var s=String(n+100).substr(1,2);
	return s;
}

function timeStr(t)
{
	if(t < 0)
	{
		str='00:00:00';
		return str;
	}
	var s=t%60;
	var m=parseInt(t/60)%60;
	var h=parseInt(t/3600)%24;
	var d=parseInt(t/86400);

	var str='';
	if (d > 999) { return '永久'; }
	if (d) str+=d+'天 ';
	str+=fit2(h)+':';
	str+=fit2(m)+':';
	str+=fit2(s);
	return str;
}

// auto,NA,IC,ETS,SP,FR,JP
var dmnRng= new Array(16383,2047,2047,8191,1536,7680,16383);

function rmEntry(a,i)
{
	if (a.splice)
		a.splice(i,1);
	else
	{
		if (i>=a.length) return;
		for (var k=i+1;k<=a.length;k++)
			a[k-1]=a[k];
		a.length--;
	}
}


function getStyle(objId) {
	var obj=document.getElementById(objId);
	if (obj) return obj.style;
	else return 0;
}

function setStyle(id, v) {
    var st = getStyle(id);
    if(st) { st.visibility = v; return true; } else return false;
} 

function check_text(str)
{
	var flag = true;
	if(str.length==0) return false;
	for(var i=0;i<str.length&&flag;i++)
  {
		var c = str.charAt(i);
		if((c>='0'&&c<='9')||(c>='A'&&c<='Z')||(c>='a'&&c<='z'))
  		flag=true;
		else 
		{
			flag = false
		}
	}
	return flag;
}

function check_text2(str)
{
	var flag = true;
	if(str.length==0) return true;
	for(var i=0;i<str.length&&flag;i++)
  {
		var c = str.charAt(i);
		if((c>='0'&&c<='9')||(c>='A'&&c<='Z')||(c>='a'&&c<='z'))
  		flag=true;
		else 
		{
			flag = false
		}
	}
	return flag;
}

//判断是否式C类IP地址
function isCIP(str)
{
	
	var re=/^(19[2-9]|2[0-2][0-3])\.(25[0-5]|2[0-4][0-9]|1?[0-9]{1,2})\.(25[0-5]|2[0-4][0-9]|1?[0-9]{1,2})\.(25[0-4]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]?)$/;
	if(re.test(str))
	{
		return true;
	}
	else
		return false;
}

function CheckMAC(mac)
{	
	//huang xiaoli modify  改成MAC地址必须输入两位才有
    //var re = /^[0-9a-fA-F]{1,2}:[0-9a-fA-F]{1,2}:[0-9a-fA-F]{1,2}:[0-9a-fA-F]{1,2}:[0-9a-fA-F]{1,2}:[0-9a-fA-F]{1,2}$/;
	var re = /^[0-9a-fA-F]{2}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}$/;
    if (! re.test(mac))
    {
        return false;
    }
    return true;
}

//mac ip url 过滤日期选择
function onClickAllDay()
{
	if(document.getElementById("alldays").checked)
	{
		for(var i=1;i<=7;i++)
			document.getElementById("date"+i).disabled = true;
	}
	else
	{
		for(var i=1;i<=7;i++)
			document.getElementById("date"+i).disabled = false;
	}
}

///mac ip url 过滤 清除
function onDelDateRemark(f)
{
	f.en.checked = false;
		
	f.remark.value = "";
	f.sh.selectedIndex = 0;
	f.sm.selectedIndex = 0;
	f.eh.selectedIndex = 0;
	f.em.selectedIndex = 0;
	f.alldays.checked = true;
	onClickAllDay();
}

///mac ip url 过滤 初始化
function init2(f)
{
	var n = getCfg("curNum");
	if(n == "")
		n = 1;
		
	if(getCfg("check_en") == 1)
		document.getElementById("check").checked = true;
	else
		document.getElementById("check").checked = false;
	onCheck();

	f.curNum.selectedIndex = n-1;
	onChangeNum(f);
}

//检测ASCII
function checkASCII(str)
{
	var i;
	for (i = 0; i < str.length; i ++)
	{
		if (str.charCodeAt(i)>255)
		{
			alert("不可输入汉字!");
			return false;
		}
	}
	return true;
}

//检测16进制
function isHex(str)
{
	for (var i=0; i<str.length; i++) {
		var ch = str.charAt(i).toLowerCase();
		if (!((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f'))) {
			return false;
		}
	}
	
	if ((str.length % 2) != 0)
		return false;
	
    return true;
}

//异步传输生成对象
function GetReqObj()
{
	var request = false;

try  {
		req = new XMLHttpRequest();
	 } catch (trymicrosoft) 
	 {
		try {
    		req = new ActiveXObject("Msxml2.XMLHTTP");
		} catch (othermicrosoft) {
    		try {
				req = new ActiveXObject("Microsoft.XMLHTTP");
			} catch (failed) {
			req = false;
			}
  		}
	 }
	
	if (! req)
	{
		alert("Error initializing HttpRequest!");
		return false;
	}

	return req;
}

function IsReboot()
{
	if(confirm("重启路由器使设置生效，是否立即重启"))	
		return 1;
	else
		return 0;
}
function IsRebootW()
{
	if(confirm("重启无线模块使设置生效，是否立即重启"))	
		return 1;
	else
		return 0;
}
function IsShow(id,show)
{
	if (document.all)
		document.all(id).style.display = (show) ? "block" : "none";
	else if (document.getElementById)
		document.getElementById(id).style.display = (show) ? "block" : "none";
}


function byID(id)
{
	return document.getElementById(id);
}

function ckRemark(s)
{
	var re = /^\w{0,}$/;
	if(!re.test(remark))
	{
		alert("注释只能由数字、字母、下划线组成");
		return false;
	}	
	return true;
}

function ckMacReserve(mac)
{
	var m = mac.split(":");	
	for(var i=0;i<6;i++)
	{
		m[i] = m[i].toUpperCase();
	}
	var ms = m[0]+":"+m[1]+":"+m[2]+":"+m[3]+":"+m[4]+":"+m[5];
	if(ms =="00:00:00:00:00:00" || ms == "FF:FF:FF:FF:FF:FF")	
	{
		alert("MAC地址为保留地址,请重新输入。")	;
		return false;
	}
	
	/*alert(parseInt(0xB,10).toString(2)); 
	if(m[0]=="01" && m[1]=="00" && m[2]=="5E" && ( Hex2Dec(m[3])<= 127))
	{
		alert("MAC地址为组播地址,请重新输入。")	;
		return false;
	}
	*/
	
	if(parseInt(m[0],16)&1 == 1)//m[0]最后一位是否为1
	{
		alert("MAC地址为组播地址,请重新输入。")	;
		return false;
	}
	
	return true;
}

function Hex2Dec(num) { return eval('0x'+num); }

function orderIP(ip1,ip2)
{
	var mi = ip1.split(".");
	var ma = ip2.split(".");
	if(parseInt(mi[0])>parseInt(ma[0]))
	{
		return false;
	}
	else if(parseInt(mi[0])==parseInt(ma[0]))
	{
		if(parseInt(mi[1])>parseInt(ma[1]))	
			return false;
		else if(parseInt(mi[1])==parseInt(ma[1]))
		{
			if(parseInt(mi[2])>parseInt(ma[2]))	
				return false;
			else if(parseInt(mi[2])==parseInt(ma[2]))
			{
				if(parseInt(mi[3])>parseInt(ma[3]))	
					return false;
			}
		}
	}
	
	return true;
}
function style_display_on()
{
	if (window.ActiveXObject)
	{ // IE
		return "block";
	}
	else if (window.XMLHttpRequest)
	{ // Mozilla, Safari,...
		return "table-row";
	}
}

function show_hide(el,shownow)  // IE & NS6; shownow = true, false
{
//	alert("el = " + el);
    var showit = style_display_on();
    var hideit = "none";
	if (document.all)
		document.all(el).style.display = (shownow) ? showit : hideit ;
	else if (document.getElementById)
		document.getElementById(el).style.display = (shownow) ? showit : hideit ;
}

function tenda_ipMskChk(mn,str,lanip)
{
	var m=new Array();
	var lanipi=new Array();
	if (mn.length==4)
		for (i=0;i<4;i++) m[i]=mn[i].value;
	else
	{
		m=mn.value.split('.');
		if (m.length!=4) { alert(str+" 无效 !!!") ; return 0; }
	}
//huangxiaoli add 2011.11.11	
	var t = /^[0-9]{1,}$/;
	for(i=0;i<4;i++)
	{
		if(!t.test(m[i]))
		{
			alert(str+"包含非法字符 !!!") ; return 0;
		}
	}
//add end

	var v=(m[0]<<24)|(m[1]<<16)|(m[2]<<8)|(m[3]);

   	var f=0 ;	  
   	for (k=0;k<32;k++)
	{
		if ((v>>k)&1) f = 1;
		else if (f==1)
		{
			alert(str+" 无效 !!!") ;
			//for(var i=0; i<4; i++) m[i].value=m[i].defaultValue;
			return 0 ;
		}
	}
	if (f==0) { alert(str+" 无效 !!!") ; return 0; }
//add by stanley
	lanipi = lanip.value.split('.');
	//alert(lanipi[0]);
	if(lanipi[0]<128)
	{
		if(m[0]<255){
			alert(str+" 无效, 不支持网络聚集 !");
			//alert(str+" 无效 !!!") ;
			return 0;
		}
		if(m[1]>0 || m[2]>0 || m[3]>0){
			alert(str+" 无效, 不支持子网划分 !");
			//alert(str+" 无效 !!!") ;
			return 0;
		}	
	}else if(lanipi[0]<192){
		if(m[0]<255 || m[1]< 255 ){
			alert(str+" 无效, 不支持网络聚集 !");
			//alert(str+" 无效 !!!") ;
			return 0;
		}
		if( m[2]>0 || m[3]>0){
			alert(str+" 无效, 不支持子网划分 !");
			//alert(str+" 无效 !!!") ;
			return 0;
		}	
	}else if(lanipi[0]<224){
		if(m[0]<255 || m[1]< 255 || m[2]< 255){
			alert(str+" 无效, 不支持网络聚集 !");
			//alert(str+" 无效 !!!") ;
			return 0;
		}
		if( m[3]>0){
			alert(str+" 无效, 不支持子网划分 !");
			//alert(str+" 无效 !!!") ;
			return 0;
		}	
	}
//end
	return 1 ;
}
function checkSSID(SSID)
{		
	var ascii = /^[ -~]+$/g;
	if(SSID == "" || SSID.length > 32 || !ascii.test(SSID)) {
	//alert("Please specify a valid SSID between 1-32 ASCII characters!")
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