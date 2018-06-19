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
		<h2 class="legend">UPNP设置</h2>
		<div class="control-group">
			<label for="UpnpStatus" class="control-label">启用UPnP</label>
			<div class="controls">
				<input type="checkbox" name="UpnpStatus" id="UpnpStatus" value="1">
			</div>
		</div>
	</fieldset>
    <script>tbl_tail_save("document.frmSetup");</script>
</form>
</body>
</html>