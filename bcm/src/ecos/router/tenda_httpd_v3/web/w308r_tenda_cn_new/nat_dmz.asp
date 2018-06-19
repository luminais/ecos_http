<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8">
<title>NAT | DMZ</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script type="text/javascript" src="js/gozila.js"></script>
<script>
var def_LANMASK = "<%aspTendaGetStatus("lan", "lanmask");%>";
var def_LANIP = "<%aspTendaGetStatus("lan", "lanip");%>";
var def_dmzen = "<%aspTendaGetStatus("lan","dmzen");%>";//DMZ主机IP地址是否启用：0：未启用；1:启用
var def_DMZIP = "<%aspTendaGetStatus("lan","dmzip");%>";//MZ主机IP地址
addCfg("LANIP", 0,def_LANIP);
addCfg("en", 1, def_dmzen);
addCfg("DMZ1", 2, def_DMZIP);
var netip=getCfg("LANIP").replace(/\.\d{1,3}$/,".");
function init(f){
	f.dmzip.value=def_DMZIP;
	cfg2Form(f);
}

function preSubmit(f) {
	var m;
	if (f.dmzip.value=='' && (!f.en.checked) ) m='';
	else
	{
		if (f.dmzip.value==getCfg("LANIP")) { alert("不可使用路由器IP地址!"); return; }
		if (!verifyIP2(f.dmzip,"DMZ主机IP地址")) return ;
		f.dmzip.value = clearInvalidIpstr(f.dmzip.value);
		if ( !ipCheck(def_LANIP,f.dmzip.value, def_LANMASK) ) 
		{
			alert(f.dmzip.value+ "与LAN　IP　不同网段！");
			f.dmzip.value="0.0.0.0";
			return ;
		}
		if(f.dmzip.length<8)
			alert("非法IP!");
		if (f.dmzip.value=='' || (!f.en.checked) ) m='';
		else
		{
		  m="1;0;"+f.dmzip.value;
		}
	}
	setCfg("DMZ1",m);
	form2Cfg(f);
	f.submit();
}
</script>
</head>
<body onLoad="init(document.frmSetup);">
<form name=frmSetup method=POST action="/goform/VirSerDMZ">
<input type=hidden name=GO value=nat_dmz.asp >
	<fieldset>
		<h2 class="legend">DMZ主机</h2>
		<p>注意:设置DMZ主机之后，与该IP相关的防火墙设置将不起作用。</p>
		<div class="control-group">
			<label class="control-label" nowrap>DMZ主机IP地址</label>
			<div class="controls">
				<input class="text input" name="dmzip" size=16 maxlength=15 />
			</div>
		</div class="control-group">
		<div class="control-group">
			<label class="control-label" nowrap></label>
			<div class="controls">
				<label class="checkbox"><input type="checkbox" name=en value=1 />启用</label>
			</div>
		</div>
	</div>
    <script>tbl_tail_save("document.frmSetup");</script>
</form>
</body>
</html>