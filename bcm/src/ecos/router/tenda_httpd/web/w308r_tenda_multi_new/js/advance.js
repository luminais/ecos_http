var pageHeight = 0;
	
function initMinHeight (iframe, subMenu, helpContent) {
	pageHeight = T.dom.pageHeight();
	if ((pageHeight - 232) > 320) {
		iframe.style.minHeight = (pageHeight - 232) + "px";
		subMenu.style.minHeight = (pageHeight - 226) + "px";
		helpContent.style.minHeight = (pageHeight - 232) + "px";
	}
}
	
function reinitIframe(){
	var iframe 	 = T.dom.byId("main_iframe"),
		subMenu  = T.dom.byId("sub_menu"),
		helpContent = T.dom.byId("help"),
		bHeight;
		
	if (pageHeight === 0) {
		initMinHeight (iframe, subMenu, helpContent)
	}	
	iframe.style.height = "";
	helpContent.style.height = "";
	subMenu.style.height = "";
	try{
		bHeight = iframe.contentWindow.document.body.scrollHeight ||
				iframe.contentWindow.document.documentElement.scrollHeight;
		bHeight = bHeight > (helpContent.offsetHeight - 90) ? bHeight :
				(helpContent.offsetHeight - 90);
		bHeight = (pageHeight - 236) > bHeight ? (pageHeight - 236) : bHeight;
		
		if (bHeight < 320) {
			bHeight = 320;
		}
		iframe.style.height = (bHeight + 5) + "px";
		subMenu.style.height = (bHeight + 10) + "px";
	} catch (ex){ 
		setTimeout(reinitIframe,50);
	}
}

function selectHelp(index) {
	var helpP = document.getElementById("help_text").getElementsByTagName("p"),
		i = 0,
		len = helpP.length,
		idArr = ["help-static", "help-dhcp", "help-pppoe", "help-pptp", "help-l2tp"];
		
	for ( ; i < len; i++) {
		if (i !== len - 1) {
			T.dom.addClass(helpP[i], "none");
		}
	}
	T.dom.removeClass(idArr[index], "none");
}

(function (window) {
var lastModuleId, lastSubmenu;

function loadedIframe() {
	document.getElementById("loading").style.display = "none";
	document.getElementById("main_iframe").style.display = "";
	reinitIframe();
}
function renewIframe(url) {
	var iframe = document.createElement("iframe"),
		iframeContainer = document.getElementById("container_main");
	
	iframe.setAttribute("id", "main_iframe");
	iframe.setAttribute("name", "mainFrame");
	iframe.setAttribute("frameborder", "0");
	
	//IE7下不认”frameborder“ 必须写“frameBorder” 第三个参数表0示区分大小写
	iframe.setAttribute("frameBorder", "0", 0);
	iframe.setAttribute("scrolling", "auto");
	iframe.setAttribute("framespacing", "0");
	iframe.setAttribute("marginwidth", "0");
	iframe.setAttribute("marginheight", "0");
	iframe.setAttribute("src", url);
	iframe.setAttribute("style", "display:none");

	if (iframe.attachEvent){
		iframe.attachEvent("onload", function(){
			loadedIframe();
		});
	} else {
		iframe.onload = function(){
			loadedIframe();
		};
	}
	iframeContainer.innerHTML = "";
	iframeContainer.appendChild(iframe);
}

function clickSubMenu(elem){
	var i, iframe,
		lin 	= elem.href,
		str 	= lin.match(/\w+\.asp/g),
		hLen	= helpinfos.length,
		subMenu  = T.dom.byId("sub_menu"),
		mainContentHeight  = T.dom.byId("main_content").offsetHeight;
	
	document.getElementById("main_iframe").style.display = "";
	document.getElementById("loading").style.display = "";
	
	renewIframe(lin);
	
	iframe 	 = T.dom.byId("main_iframe")
	iframe.style.height = (mainContentHeight - 84) + "px";
	subMenu.style.height = (mainContentHeight - 84) + "px";
	for(i = 0; i < hLen; i += 2) {
		if(helpinfos[i].indexOf(str) >= 0) {
			document.getElementById("help_text").innerHTML = helpinfos[i + 1];
		}
	}
}

function changeMenu(obj) {
	var curModuleId = obj.id,
		menuContent	= document.getElementById("sub_menu"),
		lastModuleLink = T.dom.byId(lastModuleId);
			
	if (curModuleId !== menuContent.lastModuleId) {
		T.dom.removeClass(lastModuleLink, "active");
		T.dom.addClass(obj, "active");
		changeSubMenu(menuContent, curModuleId);
		lastModuleId = curModuleId;
	}
	//menuContent.getElementsByTagName("a")[0].click();
	lastSubmenu = document.getElementById("sub_menu").getElementsByTagName("a")[0];
	clickSubMenu(lastSubmenu);
}

function changeSubMenu(menuContent, submenuId) {
	var menuArr = menuObj[submenuId],
		len 	= menuArr.length,
		i		= 0,
		menuStr = '<ul class="nav-list">',
		curMenu;
	
	for (; i < len; i++) {
		curMenu = menuArr[i].split(";");
		if (i > 0) {
			menuStr += '<li><a href="' + curMenu[0] + '" target="mainFrame"' +
				'>' + curMenu[1] +'</a></li>';
		} else {
			menuStr += '<li><a class="active" href="' + curMenu[0] + '" target="mainFrame"' +
				'>' + curMenu[1] +'</a></li>';
		}
	}
	menuStr += '</ul>';
	menuContent.innerHTML = menuStr;
}
	
function initEvent(menuContent) {
	menuContent.onclick = function (e) {
		var e = e || window.event,
			target = e.target || e.srcElement;
				
		if (target.tagName.toLocaleLowerCase() === "a") {
			if (e.preventDefault) { 
				e.preventDefault();
			} else {
				e.returnValue = false;
			}
			if (!lastSubmenu) {
				lastSubmenu = target;
			}
			
			lastSubmenu.className = "";
			target.className = "active";
			clickSubMenu(target);
			lastSubmenu = target;
		}
	}
	
	document.getElementById("menu_ul").onclick = function (e) {
		var e = e || window.event,
			target = e.target || e.srcElement,
			aElm;

		if (target.tagName.toLocaleLowerCase() === "span") {
			aElm = target.parentNode;
			if (aElm.href.indexOf("index.asp") !== -1) {
				return true;
			}
			if (e.preventDefault) { 
				e.preventDefault();
			} else {
				e.returnValue = false;
			}
			changeMenu(aElm);
			return false;
		}
	}
}

window.onload = function(){
	var menuContent = document.getElementById("sub_menu");
	
	initEvent(menuContent);
	lastModuleId = "advance";
	lastSubmenu = menuContent.getElementsByTagName("a")[0];
	
	renewIframe("system_status.asp");
};
})(window);