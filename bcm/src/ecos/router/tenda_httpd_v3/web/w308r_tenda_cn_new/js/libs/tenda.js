// "tenda" (tenda.js)
//  Released under the GNU GPL
//  Build by Tendasanfeng@yahoo.com
//	versions 1.0
//  $Id: tenda.js 2013-5-5 09:42:30 Tenda $
/***********************************************************************************************
************************************************************************************************/

(function (window) {
"use strict";

var document = window.document,
	coreTostring = Object.prototype.toString,
	coreSlice = Array.prototype.slice,
	rootTenda,
	Tenda = {};

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

Tenda.isElement= function(o) {
		var toString;
	
		if (!o) {
			return false;
		}
		toString = coreTostring.call(o);
		return toString.indexOf('HTML') !== -1 ||
				(toString === '[object Object]' &&o.nodeType === 1 &&
				!(o instanceof Object));
};

Tenda.isWindow = function( obj ) {
		return obj != null && obj == obj.window;
}
Tenda.hasCapital = function(value, pos) {
		var pattern = /[A-Z]/g,
			myPos = pos || value.length;
			
		return pattern.test(value.charAt(myPos - 1));
};

Tenda.Event = {
	getEvent: function (event) {
		return event || window.event;
	},
	
	getTarget: function (event) {
		event = this.getEvent(event);
		return   event.target || event.srcElement;
	},
	
	getRelatedTarget: function (event) {
		if (event.relatedTarget) {
			return event.relatedTarget;
		} 
		if (event.toElement) {
			return event.toElement;
		}
		if (event.fromElement) {
			return event.fromElement;
		}
		
		return null;
	},
	
	//Prevent the default event occurs
	preventDefault: function (event) {
		if (event.preventDefault) { 
			event.preventDefault();
		} else {
			event.returnValue = false;
		}
	},
	
	//Stop event propagation
	stopPropagation: function (event) {
		if (event.stopPropagation) { 
			event.stopPropagation();
		} else {
			event.cancelBubble = true;
		}
	},
	
	//Bind events to DOM elements
	on: function (node, eventType, handler, scope) {
		var elem = Tenda.dom.toElement(node);

		if (!elem) {
			return false;
		}
		scope = scope || elem;
		
		if (document.attachEvent) {
			elem.attachEvent("on" + eventType, function(){
				handler.apply(scope,arguments);
			});
		} else if (document.addEventListener){
			elem.addEventListener(eventType, function(){
				handler.apply(scope,arguments);
			}, false);
		} else {
			elem["on" + eventType] = handler;
		}
	},
	
	//Remove bind events on DOM elements
	off: function (node, eventType, handler) {
		var elem = Tenda.dom.toElement(node);

		if (!elem) {
			return false;
		}
		if (document.attachEvent) {
			elem.detachEvent("on" + eventType, handler);
		} else if (document.addEventListener){
			elem.removeEventListener(eventType, handler, false);
		} else {
			elem["on" + eventType] = null;
		}
	}
};

Tenda.dom = {
	pageWidth: function() {
		var de = document.documentElement;
		
		return window.innerWidth ||
				(de && de.clientWidth) ||
				document.body.clientWidth;
	},
	pageHeight: function() {
		var de = document.documentElement;
		
		return window.innerHeight ||
				(de && de.clientHeight) ||
				document.body.clientHeight;
	},
	byId: function (id) {
		return document.getElementById(id);
	},
	
	//change id or node to element
	toElement: function (node) {
		var elem = null;
		
		if (Tenda.isWindow(node) || node === window.document) {
			return node;
		}
		if (typeof node === "string") {
			elem = Tenda.dom.byId(node);
		} else if (Tenda.isElement(node)) {
			elem = node;
		}
		return elem;
	},
	
	addClass: function (node, className) {
		var elem = Tenda.dom.toElement(node),
			str;
		if (elem === null) {
			return 0;
		}
		str = " " + elem.className + " ";
		if (str.indexOf(" " + className + " ") === -1) {
			elem.className = elem.className + " " + className;
		}
	},
	
	removeClass: function (node, className) {
		var elem = Tenda.dom.toElement(node),
			str,
			newName;
		if (elem === null) {
			return 0;
		}
		str = " " + elem.className + " ";
		newName = str.replace(" " + className + " ", " ").trim();
		elem.className = newName;
	},
	
	hide: function(node) {
		var elem = Tenda.dom.toElement(node);
		
		if (elem === null) {
			return 0;
		}
		elem.style.display = "none";
	},
	
	show: function(node) {
		var elem = Tenda.dom.toElement(node);
		
		if (elem === null) {
			return 0;
		}
		elem.style.display = "";
	},
	
	createFragment: function () {
	},
	 
	getCursorPos: function (ctrl) {
		var Sel,
			CaretPos = 0;
		//IE Support
		if (document.selection) {
			ctrl.focus ();
			Sel = document.selection.createRange();
			Sel.moveStart ('character', -ctrl.value.length);
			CaretPos = Sel.text.length; 
		} else if (ctrl.selectionStart || parseInt(ctrl.selectionStart, 10) === 0){
			CaretPos = ctrl.selectionStart;
		}
		return (CaretPos); 
	},
	
	setCursorPos: function (ctrl, pos){
		var range;
		
		if(ctrl.setSelectionRange){
			ctrl.focus();
			ctrl.setSelectionRange(pos,pos);
		} else if (ctrl.createTextRange) {
			range = ctrl.createTextRange();
			range.collapse(true);
			range.moveEnd('character', pos);
			range.moveStart('character', pos);
			range.select();
		}
	},
	
	addCapTip: function(newField, pasElem) {
		//add capital tip 
		Tenda.Event.on(newField, "keyup", function (e) {
			var massageElm, repeat, pos,
			msgId = this.id + "caps";
			e = Tenda.Event.getEvent(e);
			if (!this.capDetected) {
				massageElm = document.createElement('span');
				massageElm.className = "help-inline text-info";
				massageElm.id = msgId;
				massageElm.innerHTML = '你输的是大写字母！';
				if (pasElem) {
					this.parentNode.insertBefore(massageElm, pasElem.nextSibling);
				} else {
					this.parentNode.insertBefore(massageElm, newField.nextSibling);
				}
				
				this.capDetected = true;
			} else {
				massageElm = Tenda.dom.byId(msgId);
			}
			pos = Tenda.dom.getCursorPos(this);
			if (Tenda.hasCapital(this.value, pos)) {
				Tenda.dom.show(massageElm);
				repeat = "Tenda.dom.hide('" + msgId +"')";
				window.setTimeout(repeat, 1000);
			} else {
				Tenda.dom.hide(massageElm);
			}
		});
	},
	
	toTextType: function (elem) {
		var newField;
		
		newField = document.createElement('input');
		newField.setAttribute("type", "text");
		newField.setAttribute("maxLength", elem.getAttribute("maxLength"));
		newField.setAttribute("id", elem.id + "_");
		newField.className = elem.className;
		newField.setAttribute("placeholder", elem.getAttribute("placeholder") || "");
		elem.parentNode.insertBefore(newField, elem);
		
		Tenda.Event.on(elem, "focus", function () {
			elem.style.display = "none";
			newField.style.display = "";
			if (elem.value !== "") {
				newField.value = elem.value;
			} else {
				newField.value = "";	
			}
			newField.focus();
		});
		Tenda.Event.on(newField, "blur", function () {
			if ((newField.value.trim() !== newField.getAttribute("placeholder")) &&
					newField.value !== "") {
				newField.style.display = "none";
				elem.style.display = "";
				elem.value = newField.value;
			} else {
				elem.value = "";
			}
		});
		
		if (elem.value !== "") {
			newField.style.display = "none";
			newField.value = elem.value;
		} else {
			elem.style.display = "none";
			newField.style.display = "";
		}
		elem.textChanged = true;
		return newField;
	},
	
	addPlaceholder: function (id, text, capTip, formElm) {
		var elem = Tenda.dom.toElement(id),
			textElem;
			
		formElm = formElm || document.forms[0];
		if (elem !== null) {
			textElem = elem;
		} else {
			return 0;
		}
		
		function hasPlaceholder() {
			var i = document.createElement('input');
			return 'placeholder' in i;
		}
		
		function isPlaceholderVal(node) {
			return (node.value === node.getAttribute("placeholder"));
		}
		
		function showPlaceholder(node, reload) {
			if (node.value === "" || (reload && isPlaceholderVal(node))) {
				node.value = node.getAttribute("placeholder");
				Tenda.dom.addClass(node.id, "placeholder-text");
			}
		}
		
		if (text) {
			elem.setAttribute("placeholder", text);
		}
		
		//Detect capital
		if (elem.type === "password" && !elem.textChanged) {
			textElem = Tenda.dom.toTextType(elem, capTip);
			if (capTip) {
				Tenda.dom.addCapTip(textElem, elem);
			}
		} else if (elem.type === "text" && capTip) {
			Tenda.dom.addCapTip(textElem);
		}
		
		
		if (!hasPlaceholder()) {
			Tenda.Event.on(textElem, "blur", function () {
				showPlaceholder(this, false);
			});
			Tenda.Event.on(textElem, "focus", function () {
				Tenda.dom.setCursorPos(this, this.value.length);
				Tenda.dom.removeClass(this.id, "placeholder-text");
				if (isPlaceholderVal(this)) {
					this.value = "";
				}
			});
			Tenda.Event.on(formElm, "submit", function () {
				if (textElem.value.trim() === textElem.getAttribute("placeholder")) {
					textElem.value = "";
				}
			});
			showPlaceholder(textElem, true);
		} else {
			Tenda.Event.on(textElem, "blur", function () {
				if (isPlaceholderVal(this)) {
					this.value = "";
				}
				if (this.value === "") {
					Tenda.dom.addClass(this.id, "placeholder-text");
				}
			});
			
			Tenda.Event.on(textElem, "keyup", function () {
				if (this.value !== "") {
					Tenda.dom.removeClass(this.id, "placeholder-text");
				}
			});
			
			if (textElem.value === "") {
				Tenda.dom.addClass(textElem, "placeholder-text");
			}
		}
		
		return textElem;
	},
	
	inputPassword: function (id, text, capTip, hide) {
		if (document.getElementById(id).value === "") {
			Tenda.dom.addPlaceholder(id, text, capTip);
		} else {
			if (hide) {
				Tenda.Event.on(id, "keyup", function () {
					if (this.value === "") {
						Tenda.dom.addPlaceholder(id, text, capTip).focus();
					}
				});
			} else {
				Tenda.dom.addPlaceholder(id, text, capTip);
			}
		}
	}
};

Tenda.ready = (function() {
	var funcs = [],
		ready = false,
		len,
		i;
		
	function handler(e) {
		e = e || window.event; 
		if (ready) {
			return;
		}
		
		if (e.type === 'readystatechange' && document.readyState !== 'complete') {
			return;
		}
		
		for (i = 0, len = funcs.length; i < len; i++) {
			funcs[i].call(document);
		}
		
		ready = true;
		funcs = null;
	}
	
	//
	if (document.addEventListener) {
		Tenda.Event.on(document, 'DOMContentLoaded', handler);
		
	} /* else if (document.attachEvent) {
		window.attachEvent('onload', handler)
		document.attachEvent('onreadystatechange', handler);
	} */
	Tenda.Event.on(document, 'readystatechange', handler);
	Tenda.Event.on(window, 'load', handler);
	
	// return ready() function
	return function ready(f) {
		if (ready) {
			f.call(document);
		} else {
			funcs.push(f);
		}
	};
}());

window.T = Tenda;
window.Tenda = Tenda;
}(window));