var pageHeight = 0,
	reinitIframeNum = 0,
	mainNav;
	
function initMinHeight (iframe, subMenu, helpContent) {
	pageHeight = T.dom.pageHeight();
	iframe.style.minHeight = (pageHeight - 230) + "px";
	subMenu.style.minHeight = (pageHeight - 220) + "px";
	helpContent.style.minHeight = (pageHeight - 230) + "px";
}
	
function reinitIframe(){
	var iframe 	 = T.dom.byId("main_iframe"),
		subMenu  = T.dom.byId("sub_menu"),
		menus = T.dom.byId("sub_menu").getElementsByTagName('ul')[0],
		helpContent = T.dom.byId("help"),
		heightArr = [],
		bHeight;
		
	heightArr.push(menus.offsetHeight);
	
	if (pageHeight === 0) {
		initMinHeight (iframe, subMenu, helpContent)
	}	
	iframe.style.height = "";
	helpContent.style.height = "";
	subMenu.style.height = "";
	try{
		heightArr.push(iframe.contentWindow.document.body.scrollHeight ||
				iframe.contentWindow.document.documentElement.scrollHeight);
		heightArr.push(helpContent.offsetHeight - 92);
		heightArr.push(pageHeight - 230);
		
		bHeight =  Math.max.apply(Math, heightArr);
		iframe.style.height = (bHeight + 7) + "px";
		subMenu.style.height = (bHeight + 19) + "px";
	} catch (ex) {
		if (reinitIframeNum < 10) {
			setTimeout(reinitIframe, 50);
			reinitIframeNum += 1;
		} else {
			reinitIframeNum = 0;
		}
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

function selectSubmenu(index) {
	var selectId = mainNav.subNav.getElementsByTagName('a')[index].id;
	if (selectId !== mainNav.subNav.activeSubNavId) {
		mainNav.changeNav(mainNav.subNav, selectId);
	}
}
(function (window, T) {
"use strict";
var createNav;

function loadedIframe() {
	document.getElementById("loading").style.display = "none";
	document.getElementById("main_iframe").style.display = "";
	reinitIframe();
}
function createIframe(url) {
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

createNav = (function () {

	function Nav(mod, topId, subId) {
		this.mod = mod;
		this.topNav = document.getElementById(topId);
		this.helpInfo = {};
		
		if (subId) {
			this.subNav = document.getElementById(subId);
		} else {
			this.subNav = this.topNav;
		}
		
		function creatNavberNav(id, Class, text) {
			var str = '<li>'+
				'<a id="'+ id +'" class="' + Class +'" href="#">' +
				'<span class="nav-left">&nbsp;</span>' +
				'<span class="nav-text">' + text + '</span>' +
				'<span class="nav-right">&nbsp;</span>' + 
				'</a></li>';
			return str;
		}
		
		this.init = function() {
			var navbarNavHtml = this.topNav.innerHTML,
				myClass = '',
				activeNav,
				navObj,
				name,
				i;
						
			for (name in this.mod) {
				navObj = this.mod[name];
				activeNav = activeNav || name;
				if (name === activeNav) {
					navbarNavHtml += creatNavberNav(name, 'active', navObj.text);
					this.topNav.activeNavId = name;
					
					if (navObj.menus) {
						this.createSubMenu(name);
					}
				} else {
					navbarNavHtml += creatNavberNav(name, '', navObj.text);
				}
			}
			
			this.topNav.innerHTML = navbarNavHtml;
			this.initEvent();
		}
	}
	
	Nav.prototype = {
		constructor: Nav,
		
		clickLeafMenu: function(elem) {
			var iframe = document.getElementById("main_iframe"),
				lin = elem.href;
			
			document.getElementById("loading").style.display = "";
			iframe.style.display = "none";
			iframe.src = lin;
			document.getElementById("help_text").innerHTML = this.helpInfo[elem.id];
		},
		
		createSubMenu: function(submenuId, init) {
			var menuArr = this.mod[submenuId].menus,
				len 	= menuArr.length,
				menuStr = '<ul class="nav-list">',
				i,
				curMenu;
			
			for (i = 0; i < len; i++) {
				curMenu = menuArr[i];
				this.helpInfo[curMenu.id] = curMenu.help;
				
				if (i > 0) {
					menuStr += '<li><a id="' + curMenu.id + '" href="' +
						curMenu.url + '" target="mainFrame"' + '>' +
						curMenu.text + '</a></li>';
				} else {
					menuStr += '<li><a id="' + curMenu.id +
						'" class="active" href="' + curMenu.url +
						'" target="mainFrame"' +'>'
						+ curMenu.text +'</a></li>';
				}
			}
			menuStr += '</ul>';
			this.subNav.innerHTML = menuStr;
			this.subNav.activeSubNavId = menuArr[0].id;
			this.clickLeafMenu(T.dom.byId(menuArr[0].id));
		},
		
		changeNav: function(contentElem, curId) {
			var curElem,
				lastElem,
				ifLeaf;
				
			if (contentElem.activeNavId !== curId) {
				curElem = T.dom.byId(curId);
				ifLeaf = curElem.href.indexOf("#") !== curElem.href.length -1;
				
				if(ifLeaf) {
					lastElem = T.dom.byId(contentElem.activeSubNavId);
					this.clickLeafMenu(curElem);
				} else {
					lastElem = T.dom.byId(contentElem.activeNavId);
					this.createSubMenu(curId);
				}
				
				T.dom.removeClass(lastElem, 'active');
				T.dom.addClass(curElem, 'active');
				
				if(ifLeaf) {
					contentElem.activeSubNavId = curId;
				} else {
					contentElem.activeNavId = curId;
				}
				
			}
		},
		
		initEvent: function() {
			var _this = this;
			
			T.Event.on(this.topNav, 'click', function(e) {
				var e = e || window.event,
					target = e.target || e.srcElement,
					aElm;

				if (target.tagName.toLocaleLowerCase() === "span") {
					aElm = target.parentNode;
					
					// handle href != "#"
					if (aElm.href.indexOf("#") !== aElm.href.length -1) {
						return true;
					}
					
					if (e.preventDefault) { 
						e.preventDefault();
					} else {
						e.returnValue = false;
					}
					_this.changeNav(this, aElm.id);
					return false;
				}
			});
			
			T.Event.on(this.subNav, 'click', function(e) {
				var e = e || window.event,
					target = e.target || e.srcElement;

				if (target.tagName.toLocaleLowerCase() === "a") {
					if (e.preventDefault) { 
						e.preventDefault();
					} else {
						e.returnValue = false;
					}
					
					_this.changeNav(this, target.id);
				}
			});
		}
	}

	return function(mod, topId, subId) {
		var nav = new Nav(mod, topId, subId);
		nav.init();
		
		return nav;
	}
})();

//添加DOM元素加载完，图片还未加载的时事件
T.ready(function(){
	var menuContent = document.getElementById("sub_menu");
	
	createIframe("system_status.asp");
	mainNav = createNav(MOD, 'navbar-nav', 'sub_menu');
});
})(window, T);