<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8" />
<title>WAN SPEED</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script language=JavaScript src="js/gozila.js"></script>
</head>

<body>
<form name="wanspeed" method=post action="/goform/setWanSpeed">
	<fieldset>
		<legend>Choose The WAN Speed</legend>
		<table>
			<tr>
				<td width="50"></td>
				<td><label class="radio"><input type="radio" name="ws" value="0" />AUTO</label></td>
			</tr>
			<tr>
				<td></td>
				<td><label class="radio"><input type="radio" name="ws" value="1" />10M HALF-duplex</label></td>
			</tr>
			<tr>
				<td></td>
				<td><label class="radio"><input type="radio" name="ws" value="2" />10M FULL-duplex</label></td>
			</tr>
			<tr>
				<td></td>
				<td><label class="radio"><input type="radio" name="ws" value="3" />100M HALF-duplex</label></td>
			</tr>
			<tr>
				<td></td>
				<td><label class="radio"><input type="radio" name="ws" value="4" />100M FULL-duplex</label></td>
			</tr>
		</table>
	</fieldset>
	<div class="btn-group">
		<input type="button" class="btn" value="OK" onClick="preSubmit(document.wanspeed)" />
		<input type="button" class="btn last" value="Cancel" onClick="init(document.wanspeed)" />
	</div>
</form>
<script>
var WANSPEED = "<%aspTendaGetStatus("wan","wanspeed");%>";
function init(){
	document.wanspeed.ws[WANSPEED].checked = true;
}
function preSubmit(f){
	f.submit();
}
window.onload = init;
</script>
<script src="lang/b28n.js" type="text/javascript"></script>
<script>
//handle translate
(function() {
	B.setTextDomain(["base", "advanced"]);
	B.translate();
})();
</script>
</body>
</html>