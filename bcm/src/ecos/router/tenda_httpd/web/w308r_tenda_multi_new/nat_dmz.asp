<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8">
<title>NAT | DMZ</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">

</head>
<body onLoad="init(document.frmSetup);">
<form name=frmSetup method=POST action="/goform/VirSerDMZ">
<input type=hidden name=GO value=nat_dmz.asp >
	<fieldset>
		<legend>DMZ Host</legend>
		<p>DMZ_Note</p>
		<div class="control-group">
			<label class="control-label" nowrap>DMZ Host IP Address</label>
			<div class="controls">
				<input class="text input" name="dmzip" size=16 maxlength=15 />
			</div>
		</div>
		<div class="control-group">
			<label class="control-label" nowrap></label>
			<div class="controls">
				<label class="checkbox"><input type="checkbox" name=en value=1 />Enable</label>
			</div>
		</div>
	</div>
    <div class="btn-group">
		<input type="button" class="btn" value="OK" onClick="preSubmit(document.frmSetup)" />
		<input type="button" class="btn last" value="Cancel" onClick="init(document.frmSetup)" />
	</div>
</form>
<script src="lang/b28n.js" type="text/javascript"></script>
<script>
//handle translate
(function() {
	B.setTextDomain(["base", "applications"]);
	B.translate();
})();
</script>
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
		if (f.dmzip.value==getCfg("LANIP")) { alert(_("Can not use the router's IP address!")); return; }
		if (!verifyIP2(f.dmzip,_("DMZ host IP address"))) return ;
		f.dmzip.value = clearInvalidIpstr(f.dmzip.value);
		if ( !ipCheck(def_LANIP,f.dmzip.value, def_LANMASK) ) 
		{
			alert(_("Please enter a valid IP address. DMZ Host IP address should be on the same IP net segment as the device's LAN IP and they should be different!"));
			f.dmzip.value="0.0.0.0";
			return ;
		}
		if(f.dmzip.length<8)
			alert(_("illegal IP!"));
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
</body>
</html>