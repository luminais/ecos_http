<!DOCTYPE html>
<html> 
<head>
<meta charset="utf-8">
<title>Routing | Routing Table</title>
<link rel="stylesheet" type="text/css" href="css/screen.css">
<script type="text/javascript" src="js/gozila.js"></script>
<script>
var rte=new Array(<%mGetRouteTable("sys","0");%>);//'ip,submask,gateway,metric,inteface',、、、、、、、、、、、、、、
function showRTE(){
	for (var i=0; i<rte.length;i++){
		var s = rte[i].split(",");
		if (s.length!=5){
			break;
		}
		var  m='<tr>';
			m+='<td>'+s[0]+'</td>';
			m+='<td>'+s[1]+'</td>';
			m+='<td>'+s[2]+'</td>';
			m+='<td>'+s[3]+'</td>';
			m+='<td>'+s[4]+'</td>';
			m+='</tr>';
		document.write(m);
	}
}
</script>
</head>
<body>
	<h2 class="legend">Routing Table</h2>
	<table class="table">
		<thead>
			<tr align=center>
				<th>Destination IP</th>
				<th>Subnet Mask</th>
				<th>Gateway</th>
				<th>Hops</th>
				<th>Interface</th>
			</tr>
		</thead>
		<tbody>
			<script>
				showRTE();
			</script>
		</tbody>
	</table>
	<script>
		var m='<input class="btn" value="Refresh" type=button onclick=refresh("routing_table.asp")>';
		tbl_tail(m);
		
		//reset this iframe height by call parent iframe function
		window.parent.reinitIframe();
	</script>
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