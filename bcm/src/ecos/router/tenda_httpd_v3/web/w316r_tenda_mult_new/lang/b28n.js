// "Butterfat internationalization" (b28n.js)
//  Released under the GNU GPL  by bmuller@buttterfat.net- http://www.gnu.org/licenses/gpl.txt
//  Modified by taijisanfeng@yahoo.com for our need
// 	versions 1.0
//  $Id: b28n.js 2013-2-1 09:42:30 taiji $
/***********************************************************************************************
************************************************************************************************/

(function (window) {
	"use strict";
	Function.prototype.method = function (name, func) {
		if (!this.prototype[name]){
			this.prototype[name] = func;
			return this;
		}
	};
	
	String.method('trim', function () {
		var str = this.replace(/^\s+/, ""),
		end = str.length - 1,
		ws	= /\s/;
		
		while (ws.test(str.charAt(end))) {
			end--;
		}
		return str.slice(0, end + 1);
	});
	function replaceTextNodeValue(valObject, element){  
		element = element || document;
		var cur = element.firstChild,
			typeStr = "submit,reset,button",
			curValue;
		while (cur != null){
			if (cur.nodeType === 3 && /\S/.test(cur.nodeValue)){
				curValue = cur.nodeValue.trim();
				if (valObject[curValue]) {
					cur.nodeValue = valObject[curValue];
				}
			} else if (cur.nodeType === 1){
				replaceTextNodeValue(valObject, cur);
				if (cur.getAttribute("value") &&
						/\S/.test(cur.getAttribute("value"))) {
					curValue = cur.getAttribute("value").trim();
					if (valObject[curValue]) {
						cur.setAttribute("value", valObject[curValue]);
					}
				}
			}
			cur = cur.nextSibling;
		}  
	}
	
	function Butterlation() {
		this.dict = {};
		
		this.getLang = function() {
			var one,
				end;
			if((one=document.cookie.indexOf("language")) === -1) {
				return (navigator.language || navigator.browserLanguage).substring(0, 2);
			}
			end = (document.cookie.indexOf(';', one) !== -1) ?
					document.cookie.indexOf(';', one) :
					document.cookie.length;
			return unescape(document.cookie.substring(one + 9, end));
		};
		
		this.lang = this.getLang();
		
		this.setTextDomain = function(domain, lang) {
			var i,
				domainLen;
			
			//Hander lang is undefined
			//this.lang = lang || this.lang;
			
			//TODO 测试版只支持英文,发布正式版时应改为 this.lang = lang || this.lang;
			this.lang = "en";
			
			if (Object.prototype.toString.call(domain) === "[object Array]") {
				domainLen = domain.length;
				
				for (i = 0; i < domainLen; i = i + 1) {
					this.po = window.location.protocol + "//" 
						+ window.location.host
						+ "/lang/" + this.lang + "/" + domain[i] + ".xml";
					this.initializeDictionary();
				}
			} else if (typeof domain === "string") {
				this.po = window.location.protocol + "//"
					+ window.location.host
					+ "/lang/" + this.lang + "/" + domain + ".xml";
				this.initializeDictionary();
			}
		};

		this.initializeDictionary = function() {
			var request,
				i,
				pos,
				posLen;
				
			try {
				request = new XMLHttpRequest(); 
			} catch(e1) {
				try { 
					request = new ActiveXObject("Msxml2.XMLHTTP"); 
				} catch(e2) {
					try {
						request = new ActiveXObject("Microsoft.XMLHTTP"); 
					} catch(e3) { 
						return; 
					}
				}
			}
			request.open("GET",this.po,false); 
			request.setRequestHeader("If-Modified-Since","1");
			request.send(null);
	
			if(request.status === 200) {
				pos = request.responseXML.documentElement.getElementsByTagName("message");
				posLen = pos.length;
				for(i = 0; i < posLen; i++) {
					this.dict[pos[i].getAttribute("msgid")] = pos[i].getAttribute("msgstr");
				}
			}
		};
		this.gettext = function(key) {
			return this.dict[key] || key; 
		};
		this.getFormatText = function(key,replacements) { 
			var nkey=this.gettext(key),
				index,
				count=0;
			
			if(replacements.length === 0) {
				return nkey;
			}
			while((index = nkey.indexOf('%s')) !== -1) { 
			  nkey = nkey.substring(0,index) + replacements[count]
					+ nkey.substring(index+2,nkey.length);
			  count = ((count + 1) === replacements.length) ? count : (count+1);
			}
			return nkey;
		};
		this.deleteKey = function (key) {
			delete this.dict[key];
		};
		this.translate = function (){
			var titleElem = document.getElementsByTagName("title")[0];
			titleElem.text = this.gettext(titleElem.text.trim());
			replaceTextNodeValue(this.dict);
		}
	}

	window.Butterlate = new Butterlation();
	window.B = window.B || window.Butterlate;
	window._ = function(key, replacements) {
		if (!replacements) {
			return window.Butterlate.gettext(key);
		} else {
			return window.Butterlate.getFormatText(key,replacements);
		}
	};
	window.__ = function(key,replacements) {
		return window.Butterlate.getFormatText(key,replacements);
	};     
}(window));




