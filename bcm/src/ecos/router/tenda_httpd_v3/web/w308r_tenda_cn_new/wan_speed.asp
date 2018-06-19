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
		<h2 class="legend">WAN口速率选择</h2>
		<table id="table1">
			<tr><td width="20%" align="left"><label class="radio"><input type="radio" name="ws" value="0" />自动协商</label></td>
				<td width="20%" align="left"><label class="radio"><input type="radio" name="ws" value="1" />10M半双工</label></td>
				<td width="20%" align="left"><label class="radio"><input type="radio" name="ws" value="2" />10M全双工</label></td>
				<td width="20%" align="left"><label class="radio"><input type="radio" name="ws" value="3" />100M半双工</label></td>
				<td width="20%" align="left"><label class="radio"><input type="radio" name="ws" value="4" />100M全双工</label></td>
			</tr>
		</table>
	</fieldset>
<script>
    tbl_tail_save('document.wanspeed');
</script>
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
</body>
</html>