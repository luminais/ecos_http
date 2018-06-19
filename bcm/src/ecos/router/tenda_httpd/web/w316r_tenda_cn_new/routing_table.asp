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
	<h2 class="legend">路由列表</h2>
	<table class="table">
		<thead>
			<tr align=center>
				<th>目的IP</th>
				<th>子网掩码</th>
				<th>网关</th>
				<th>跳跃数</th>
				<th>接口</th>
			</tr>
		</thead>
		<tbody>
			<script>
				showRTE();
			</script>
		</tbody>
	</table>
	<script>
		var m='<input class="btn" value="刷 新" type=button onclick=refresh("routing_table.asp")>';
		tbl_tail(m);
		
		//reset this iframe height by call parent iframe function
		window.parent.reinitIframe();
	</script>
</body>
</html>