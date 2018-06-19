<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8">
<title>Routing | Static Routing</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
</head>
<body onLoad="init(document.frmSetup)">
<form name=frmSetup method=POST action="/goform/RouteStatic">
	<input type=hidden name=GO value=routing_static.asp>
	<input type=hidden name=cmd>
	<fieldset>
		<legend>Static Routing</legend>
		<table class="table">
			<thead>
				<tr class=item1 align=center height=30>
					<th width="130">Destination Network IP Address</th>
					<th width="130">Subnet Mask</th> 
					<th width="130">Gateway</th>
					<th></th>
				</tr>
			</thead>
			<tbody>
			<tr>
			  <td nowrap>
				<input name=ip size=15 maxlength=15 class="text input-small">         
			  </td>
			  <td nowrap>
				<input name=nm size=15 maxlength=15 class="text input-small">
			  </td>
			  <td nowrap>
				<input name=gw size=15 maxlength=15 class="text input-small">
			  </td>
			  <td><input class="btn btn-mini" type="button" name="staticAdd" value="Add"   onClick="addRte(this.form)"></td>
			</tr>
			</tbody>
		</table>
	</fieldset>
    <div id="routeList" style="position:relative;"></div>    
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
//192.168.5.0:255.255.255.0:192.168.5.1:1 192.168.6.0:255.255.255.0:192.168.6.1:1
var list = "<%aspTendaGetStatus("wan","wan0_route");%>";
var resList;
var max = 5;

function init(f){
	resList = list.split(" ");
	showRT();
}

function showRT() {
	var i;	
	var m='<table class="table" id=showTab>';
	for (i=0;i<resList.length;i++) {
		var rt=resList[i];
		var s=rt.split(":");
		if (s.length!=4) continue;

		m+='<tr align=center>';
		m+='<td width="130">'+s[0];
		m+='</td>';
		m+='<td width="130">'+s[1];
		m+='</td>';
		m+='<td width="130">'+s[2];
		m+='</td>';
		m+='<td>';
		m+='<input class="btn btn-small" type=button value="Delete" onClick=delRte(this,' + i +  ')></td>';
		m+='</tr>';
	}
	m+='</table>';
	document.getElementById("routeList").innerHTML = m;
	
	//reset this iframe height by call parent iframe function
	window.parent.reinitIframe();
}

function delRte(obj,dex) {
	document.getElementById("showTab").deleteRow(dex-1);
	var i=0;
	for(i=dex;i<resList.length;i++) {
		resList[i] = resList[(i+1)];
	}
	resList.length--;
	showRT();
}

function addRte(f) {
	var info;
	if (!verifyIP2(f.ip,_("destination network IP address"),1)) return ;
	if (!ipMskChk(f.nm,_("subnet mask"))) return ;
	if (!verifyIP2(f.gw,_("gateway"))) return ;
	f.ip.value = clearInvalidIpstr(f.ip.value);
	f.nm.value = clearInvalidIpstr(f.nm.value);
	f.gw.value = clearInvalidIpstr(f.gw.value);	
	info = f.ip.value+':'+f.nm.value+':'+f.gw.value+':'+'1';
	if(resList.length >= max+1){
	        window.alert(_("Up to %s static routing entries are supported!",[max]));
		 	return;
	}
	
	for(var i=0; i<resList.length; i++) {
		if(resList[i] == info) {
			window.alert(_("The static routing entry already exists!"));
			return;
		}
	}
	resList[resList.length]=info;
	showRT();
}

function preSubmit(f) {
	var loc = "/goform/RouteStatic?GO=routing_static.asp";
	loc += "&wan0_route=";	
	for(var j=0; j<resList.length; j++){
		loc +=  resList[j] +" ";
	}
	window.location = loc;
}
</script>
</body>
</html>