<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8">
<title>WAN | DNS</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script type="text/javascript" src="js/gozila.js"></script>
<script>
var def_DNS1 = "<%aspTendaGetStatus("wan","dns1");%>",
	def_DNS2 = "<%aspTendaGetStatus("wan","dns2");%>",
	def_dnsen = "<%aspTendaGetStatus("wan","dnsen");%>";
addCfg("DS1",0x34,def_DNS1);
addCfg("DS2",0x35,def_DNS2);
function init(f) {
	cfg2Form(f);
	if(parseInt(def_dnsen) == 1) {
		f.DNSEN.checked = true;
	} else {
		f.DNSEN.checked = false;
	}
	onSel();
}

function preSubmit(f) {
	if(document.frmSetup.DNSEN.checked == true){
	    	if (!verifyIP2(f.DS1,"主DNS地址")) return ;
	    	if((f.DS2.value !="") )
	    	{
	    		if (!verifyIP2(f.DS2,"备用DNS地址")) return ;
	    	}
			f.DSEN.value = "1";
    } else {
		f.DSEN.value = "0";
	}
	form2Cfg(f);
	f.DS1.value = clearInvalidIpstr(f.DS1.value);
	f.DS2.value = clearInvalidIpstr(f.DS2.value);
	f.submit();
	showSaveMassage();
}

function onSel(){
	if(document.frmSetup.DNSEN.checked == false) {
		document.frmSetup.DS1.disabled = document.frmSetup.DS2.disabled = true;
	} else {
		document.frmSetup.DS1.disabled = document.frmSetup.DS2.disabled = false;
	}
}
</script>
</head>
<body onLoad="init(document.frmSetup);">
<form name="frmSetup" method="POST" action="/goform/AdvSetDns">
<input type="hidden" name="GO" value="wan_dns.asp">
<input type="hidden" id="rebootTag" name="rebootTag">
<input type="hidden" name="DSEN">
	<fieldset>
		<h2 class="legend">WAN口DNS设置</h2>
		<div class="control-group">
			<label class="control-label">域名服务设置</label>
			<div class="controls">
				<label class="checkbox"><input type="checkbox" class="fl" name="DNSEN"  onClick="onSel()" /></label>
			</div>
		</div>
		<div  class="control-group"> 
			<label class="control-label">域名服务器(DNS) 地址</label>  
			<div class="controls"><input name=DS1 class=text size=15 maxlength=15 /></div>
		</div>
		<div  class="control-group">
			<label class="control-label">备用DNS地址 (可选)</label> 
			<div class="controls"><input name=DS2 class=text size=15 maxlength=15 /></div>
		</div>    
		<p>注意：设置完成后，需要重启路由器，使设置生效。</p>
	</fieldset>
<script>tbl_tail_save("document.frmSetup");</script>
</form>
<div id="save" class="none"></div>
</body>
</html>