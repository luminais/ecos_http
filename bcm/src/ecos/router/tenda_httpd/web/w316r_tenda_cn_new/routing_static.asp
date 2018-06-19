<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8">
<title>Routing | Static Routing</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
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
		m+='<input class="btn btn-small" type=button value="删 除 " onClick=delRte(this,' + i +  ')></td>';
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
	if (!verifyIP2(f.ip,"目的地址 ",1)) return ;
	if (!ipMskChk(f.nm,"子网掩码")) return ;
	if (!verifyIP2(f.gw,"网关")) return ;
	f.ip.value = clearInvalidIpstr(f.ip.value);
	f.nm.value = clearInvalidIpstr(f.nm.value);
	f.gw.value = clearInvalidIpstr(f.gw.value);	
	info = f.ip.value+':'+f.nm.value+':'+f.gw.value+':'+'1';
	if(resList.length >= max+1){
	        window.alert("静态路由最多能配置"+max+"条.");
		 	return;
	}
	
	for(var i=0; i<resList.length; i++) {
		if(resList[i] == info) {
			window.alert("指定的静态路由已经存在.");
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
</head>
<body onLoad="init(document.frmSetup)">
<form name=frmSetup method=POST action="/goform/RouteStatic">
	<input type=hidden name=GO value=routing_static.asp>
	<input type=hidden name=cmd>
	<fieldset>
		<legend>静态路由</legend>
		<table class="table">
			<thead>
				<tr class=item1 align=center height=30>
					<th width="130">目的网络IP</th>
					<th width="130">子网掩码</th> 
					<th width="130">网关</th>
					<th>操作</th>
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
			  <td><input class="btn btn-mini" type="button" name="staticAdd" value="&lt;&lt;添 加"   onClick="addRte(this.form)"></td>
			</tr>
			</tbody>
		</table>
	</fieldset>
    <div id="routeList" style="position:relative;"></div>    
    <script>
        tbl_tail_save("document.frmSetup");
    </script>
</form>
</body>
</html>