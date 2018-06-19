var GO = 0,
	pageHeight = 0;
	
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
		iframe.style.height = (bHeight + 4) + "px";
		subMenu.style.height = (bHeight + 10) + "px";
		//helpContent.style.height = bHeight + "px";
		if (GO < 3) {
			setTimeout(reinitIframe,50);
			GO++;
		} else {
			GO = 0;
			return 0;
		}
	} catch (ex){ 
		setTimeout(reinitIframe,50);
	}
}

(function (window) {
var lastModuleId, lastSubmenu;

function clickSubMenu(eve){
	var lin 	= eve.href,
		str 	= lin.match(/\w+\.asp/g),
		hLen	= helpinfos.length,
		i;
	document.getElementById("main_iframe").src = lin;
	for(i = 0; i < hLen; i += 2) {
		if(helpinfos[i].indexOf(str) >= 0) {
			document.getElementById("help_text").innerHTML = helpinfos[i + 1];
		}
	}
	reinitIframe();
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
			target = e.target || e.srcElement;
		if (target.tagName.toLocaleLowerCase() === "a") {
			if (target.href.indexOf("index.asp") !== -1) {
				return true;
			}
			changeMenu(target.parentNode);
			return false;
		}
	}
}

window.onload = function(){
	var menuContent = document.getElementById("sub_menu");
	
	initEvent(menuContent);
	lastModuleId = "advance";
	lastSubmenu = menuContent.getElementsByTagName("a")[0];
	reinitIframe();
};
})(window);