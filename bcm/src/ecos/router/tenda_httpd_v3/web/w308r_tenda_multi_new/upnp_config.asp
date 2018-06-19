<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8">
<title>UPnP | Settings</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script type="text/javascript" src="js/gozila.js"></script>
<script>
def_UpnpStatus = "<%aspTendaGetStatus("sys","upnpen");%>";//启用UPnP:返回0与空值是未启用；其他：启用
addCfg("UpnpStatus",371,def_UpnpStatus);

function init(f){
	cfg2Form(f);
}

function preSubmit(f) {    
	form2Cfg(f);
	f.submit();
}
</script>
</head>
<body onLoad="init(document.frmSetup);">
<form name=frmSetup id=frmSetup method=POST action="/goform/VirSerUpnp">
	<input type=hidden name=GO id=GO value="upnp_config.asp">
	<fieldset>
		<legend>UPNP Settings</legend>
		<div class="control-group">
			<label for="UpnpStatus" class="control-label">Enable UPnP</label>
			<div class="controls">
				<input type="checkbox" name="UpnpStatus" id="UpnpStatus" value="1">
			</div>
		</div>
	</fieldset>
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
</body>
</html>