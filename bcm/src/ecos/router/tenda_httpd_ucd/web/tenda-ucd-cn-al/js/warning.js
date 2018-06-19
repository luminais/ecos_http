
$(function() {
	$("#close-page").on("click",closePage);	
	$("#continue").on("click",continuePage);
})

function closePage() {

	window.opener=null;
	window.open('','_self','');
	
	if(!window.close()) {
		showErrMsg("warning-msg","当前页面无法关闭，请手动关闭");	
	} else {
		window.close();	
	}
}

function continuePage() {
	var hash = window.location.hash,
		datas = hash.slice(1).split("&"),
		submitData = {},
		submitStr = "flag=1";
		
	for (var i = 0; i < datas.length; i++) {
		var splitData = datas[i].split("=");
		submitData[splitData[0]] = splitData[1];
		submitStr += "&" + splitData[0] + "=" + encodeURIComponent(splitData[1]);
	}
	
	//console.log(submitData.url);
	$.post("goform/InsertWhite", submitStr, function() {
		window.location.href = "http://" + submitData.url + "/";
	});
}