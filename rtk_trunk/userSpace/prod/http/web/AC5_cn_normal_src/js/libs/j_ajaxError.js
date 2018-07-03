// "REasy" (reasy.js)
//	Copyright 2013 REasy Foundation, Inc. and other contributors

//	versions 1.0.0
//	$Id: reasy.js 2013-10-28 11:42:30 REasy $
//===========================================

// Extend native to w3c standard.
(function () {
	"use strict";
	var rnative = /^[^{]+\{\s*\[native code/;

	/**
	 * For feature detection
	 * @param {Function} fn The function to test for native support
	 */
	function isNative(fn) {
		return rnative.test(String(fn));
	}

	/**
	 * For obj extend method
	 * @param {Object}		obj		The obj that you will extend
	 * @param {String}		name	The method name.
	 * @param {Function}	func	The method that you will add.
	 * @return {object}		this	This extended Object.
	 */
	function method(obj, name, func) {
		if (!isNative(obj[name])) {
			obj[name] = func;
			return obj;
		}
	}

	/**
	 * Add method to object when prototype[method] not exist
	 * @method method
	 * @static
	 * @param {String}		name	The name of the method that you will add
	 * @param {Function}	func	A reference to class in the module.  This
	 * @return {object}		this	This object .
	 */
	Function.prototype.method = function (name, func) {
		if (!this.prototype[name] && !isNative(this[name])) {
			this.prototype[name] = func;
			return this;
		}
	};

	/*
		Object.method('create', function (o) {

		});*/

	// Extend String prototype
	// ===========================================
	String.method('trim', function () {
		var str = this.replace(/^\s+/, ""),
			end = str.length - 1,
			ws = /\s/;

		while (ws.test(str.charAt(end))) {
			end--;
		}

		return str.slice(0, end + 1);
	});
}());

/* REasy core */
(function (window) {
	"use strict";
	var r_version = "1.0.0",

		// A central reference to the root REasy(document)
		rootREasy,

		// Use the correct document accordingly with window argument (sandbox)
		location = window.location,
		document = window.document,

		// Map over REasy in case of overwrite
		_REasy = window.REasy,

		// Map over the $ in case of overwrite
		_$ = window.$,

		// Selector RegExp
		rsingleTag = /^<(\w+)\s*\/?>(?:<\/\1>|)$/,
		classSelectorRE = /^\.([\w-]+)$/,
		idSelectorRE = /^#([\w-]*)$/,
		tagSelectorRE = /^[\w-]+$/,

		// Save a reference to some core methods
		core_concat = Array.prototype.concat,
		core_push = Array.prototype.push,
		core_slice = Array.prototype.slice,
		core_indexOf = Array.prototype.indexOf,
		core_toString = Object.prototype.toString,
		core_hasOwn = Object.prototype.hasOwnProperty,
		core_trim = String.prototype.trim,
		core_strundefined = typeof undefined,

		// Define a local copy of REasy
		REasy = function (selector, context) {
			return REasy.inst(selector, context);
		},

		isREasy = function (obj) {
			return obj.constructor === REasy;
		},

		// JSON RegExp
		rvalidchars = /^[\],:{}\s]*$/,
		rvalidbraces = /(?:^|:|,)(?:\s*\[)+/g,
		rvalidescape = /\\(?:["\\\/bfnrt]|u[\da-fA-F]{4})/g,
		rvalidtokens = /"[^"\\\r\n]*"|true|false|null|-?(?:\d+\.|)\d+(?:[eE][+-]?\d+|)/g,

		class2type = {},
		klass;

	var nodeNames = "abbr|article|aside|audio|bdi|canvas|data|datalist|details|figcaption|figure|footer|" +
		"header|hgroup|mark|meter|nav|output|progress|section|summary|time|video",
		rnoshimcache = new RegExp("<(?:" + nodeNames + ")[\\s/>]", "i"),
		rnoInnerhtml = /<(?:script|style|link)/i,
		rboolean = /(?:checked|selected|autofocus|autoplay|async|controls|defer|disabled|hidden|loop|multiple|open|readonly|required|scoped)$/i,
		rhtml = /<|&#?\w+;/,
		rtagName = /<([\w:]+)/,
		rtbody = /<tbody/i,
		rleadingWhitespace = /^\s+/,
		rquickExpr = /^(?:(<[\w\W]+>)[^>]*|#([\w-]*))$/,
		rxhtmlTag = /<(?!area|br|col|embed|hr|img|input|link|meta|param)(([\w:]+)[^>]*)\/>/gi,
		safeFragment = createSafeFragment(document),
		fragmentDiv = safeFragment.appendChild(document.createElement("div"));

	function returnTrue() {
		return true;
	}

	function returnFalse() {
		return false;
	}

	function getAll(context, tag) {
		var elems, elem,
			i = 0,
			found = typeof context.getElementsByTagName !== core_strundefined ? context.getElementsByTagName(tag || "*") :
			typeof context.querySelectorAll !== core_strundefined ? context.querySelectorAll(tag || "*") :
			undefined;

		if (!found) {
			for (found = [], elems = context.childNodes || context;
				(elem = elems[i]) != null; i++) {
				if (!tag || REasy.nodeName(elem, tag)) {
					found.push(elem);
				} else {
					REasy.merge(found, getAll(elem, tag));
				}
			}
		}

		return tag === undefined || tag && REasy.nodeName(context, tag) ?
			REasy.merge([context], found) :
			found;
	}

	REasy.extend = function () {
		var target = arguments[0] || {},
			i = 1,
			length = arguments.length,
			copy,
			name,
			options;

		// Handle case when target is a string or something (possible in deep copy)
		if (typeof target !== "object" && !REasy.isFunction(target)) {
			target = {};
		}

		// extend REasy itself if only one argument is passed
		if (length === i) {
			target = this;
			--i;
		}

		for (i; i < length; i++) {
			// Only deal with non-null/undefined values
			if ((options = arguments[i]) != null) {
				// Extend the base object
				for (name in options) {
					copy = options[name];

					// Prevent never-ending loop
					if (target !== copy) {

						if (copy !== undefined) {
							target[name] = copy;
						}
					}
				}
			}
		}

		// Return the modified object
		return target;
	};

	// Extend Object prototype
	// ===========================================
	function createObject(o) {
		function F() {}

		F.prototype = o;
		return new F();
	};

	//与类的操作相关的工具函数
	klass = {
		init: function () {},

		prototype: {
			init: function () {}
		},

		create: function () {
			//var object = Object.create(this);
			var obj = createObject(this);

			object.constructor = this;
			object.init.apply(object, arguments);
			return object;
		},

		inst: function () {
			var instance = createObject(this.prototype);

			instance.constructor = this;
			instance = instance.init.apply(instance, arguments);
			return instance;
		},

		proxy: function (func) {
			var thisObject = this;
			return function () {
				return func.apply(thisObject, arguments);
			};
		},

		include: function (obj) {
			var included = obj.included || obj.setup,
				i;

			for (i in obj) {
				this.fn[i] = obj[i];
			}
			if (included) {
				included(this);
			}
		}
	};

	klass.fn = klass.prototype;
	// 向REasy对象本身添加，与类相关的工具函数
	REasy.extend(klass);

	/*==============================================================================
	 * REasy.include() 是对REasy.prototype对象进行扩展 , 所有REasy实例的原型都指向
	 * REasy.prototype, REasy() 工厂函数t调用的是原型的init函数相关
	 *============================================================================*/
	REasy.include({
		// The current version of REasy being used
		REasy: r_version,

		init: function (selector, context) {
			var match, elem;

			// HANDLE: $(""), $(null), $(undefined), $(false)
			if (!selector) {
				return this;
			}

			// Handle HTML strings
			if (typeof selector === "string") {
				if (selector.charAt(0) === "<" && selector.charAt(selector.length - 1) === ">" && selector.length >= 3) {
					// Assume that strings that start and end with <> are HTML and skip the regex check
					match = [null, selector, null];

				} else {
					match = rquickExpr.exec(selector);
				}

				// Match html or make sure no context is specified for #id
				if (match && (match[1] || !context)) {

					// HANDLE: $(html) -> $(array)
					if (match[1]) {
						context = context instanceof REasy ? context[0] : context;

						// scripts is true for back-compat
						REasy.merge(this, REasy.parseHTML(
							match[1],
							context && context.nodeType ? context.ownerDocument || context : document,
							true
						));

						// HANDLE: $(html, props)
						if (rsingleTag.test(match[1])) {
							for (match in context) {
								// Properties of context are called as methods if possible
								if (REasy.isFunction(this[match])) {
									this[match](context[match]);

									// ...and otherwise set as attributes
								} else {
									this.attr(match, context[match]);
								}
							}
						}

						return this;

						// HANDLE: $(#id)
					} else {
						elem = document.getElementById(match[2]);

						// Check parentNode to catch when Blackberry 4.6 returns
						// nodes that are no longer in the document #6963
						if (elem && elem.parentNode) {
							// Handle the case where IE and Opera return items
							// by name instead of ID
							if (elem.id !== match[2]) {
								return rootREasy.find(selector);
							}

							// Otherwise, we inject the element directly into the jQuery object
							this.length = 1;
							this[0] = elem;
						}

						this.context = document;
						this.selector = selector;
						return this;
					}

					// HANDLE: $(expr) or $(expr, $(...)))
				} else if (!context || isREasy(context)) {
					return (context || rootREasy).find(selector);

					// HANDLE: $(expr, context)
					// (which is just equivalent to: $(context).find(expr)
				} else {
					return this.constructor(context).find(selector);
				}

				// HANDLE: $(DOMElement)
			} else if (selector.nodeType) {
				this.context = this[0] = selector;
				this.length = 1;
				return this;

				// HANDLE: $(function)
				// Shortcut for document ready
			} else if (REasy.isFunction(selector)) {
				return REasy.ready(selector);
			}

			if (selector.selector !== undefined) {
				this.selector = selector.selector;
				this.context = selector.context;
			}

			return REasy.makeArray(selector, this);
		},

		// Start with an empty selector
		selector: "",

		// The default length of a REasy object is 0
		length: 0,

		modules: ['core'],

		toArray: function () {
			return core_slice.call(this);
		},

		// Take an array of elements and push it onto the stack
		// (returning the new matched element set)
		pushStack: function (elems) {

			// Build a new REasy matched element set
			var ret = REasy.merge(this.constructor(), elems);

			// Add the old object onto the stack (as a reference)
			ret.prevObject = this;
			ret.context = this.context;

			// Return the newly-formed element set
			return ret;
		},

		slice: function () {
			return this.pushStack(core_slice.apply(this, arguments));
		},

		first: function () {
			return this.eq(0);
		},

		last: function () {
			return this.eq(-1);
		},

		get: function (i) {
			return this[i];
		},

		eq: function (i) {
			var len = this.length,
				j = +i + (i < 0 ? len : 0);

			return this.pushStack(j >= 0 && j < len ? [this[j]] : []);
		},

		map: function (callback) {
			return REasy.map(this, function (elem, i) {
				callback.call(elem, i);
			});
		},

		each: function (callback) {
			this.map(callback);
			return this;
		},

		end: function () {
			return this.prevObject || this.constructor(null);
		}
	});


	/*==============================================================================
	 * 对REasy对象本身进行扩展 --- REasy工具函数
	 *============================================================================*/
	var div = document.createElement("div");
	div.setAttribute("className", "t");
	div.innerHTML = "  <link/><table></table><a href='/a'>a</a><input type='checkbox'/>";
	REasy.extend({
		modules: ['base'],
		noConflict: function (deep) {
			if (window.$ === REasy) {
				window.$ = _$;
			}

			if (deep && window.REasy === REasy) {
				window.REasy = _REasy;
			}

			return REasy;
		},

		support: {
			// Test setAttribute on camelCase class. If it works, we need attrFixes when doing get/setAttribute (ie6/7)

			// IE strips leading whitespace when .innerHTML is used
			leadingWhitespace: div.firstChild.nodeType === 3,

			// Make sure that tbody elements aren't automatically inserted
			// IE will insert them into empty tables
			tbody: !div.getElementsByTagName("tbody").length,

			// Make sure that link elements get serialized correctly by innerHTML
			// This requires a wrapper element in IE
			htmlSerialize: !!div.getElementsByTagName("link").length,

			html5Clone: document.createElement("nav").cloneNode(true).outerHTML !== "<:nav></:nav>",
			// Get the style information from getAttribute
			// (IE uses .cssText instead)

			deleteExpando: true,
			noCloneEvent: true,
			inlineBlockNeedsLayout: false,
			shrinkWrapBlocks: false,
			reliableMarginRight: true,
			boxSizingReliable: true,
			pixelPosition: false
		},

		isReady: false,

		ready: (function () {
			var funcs = [],
				len,
				i;

			function handler(e) {
				if (REasy.isReady) {
					return;
				}

				if (e.type === 'readystatechange' && document.readyState !== 'complete') {
					return;
				}

				try {
					for (i = 0, len = funcs.length; i < len; i++) {
						funcs[i].call(document);
					}
				} finally {
					REasy.isReady = true;
					funcs = null;
				}

			}

			if (document.addEventListener) {
				// Use the handy event callback
				document.addEventListener("DOMContentLoaded", handler, false);

				// A fallback to window.onload, that will always work
				window.addEventListener("load", handler, false);

				// If IE event model is used
			} else {
				// Ensure firing before onload, maybe late but safe also for iframes
				document.attachEvent("onreadystatechange", handler);

				// A fallback to window.onload, that will always work
				window.attachEvent("onload", handler);
			}

			// return ready() function
			return function (f) {
				if (REasy.isReady) {
					f.call(document);
				} else {
					funcs.push(f);
				}
			};
		}()),

		isFunction: function (obj) {
			return REasy.type(obj) === "function";
		},

		buildFragment: function (elems, context, scripts, selection) {
			var j, elem, contains,
				tmp, tag, tbody, wrap,
				l = elems.length,

				// Ensure a safe fragment
				safe = createSafeFragment(context),

				nodes = [],
				i = 0;
			var wrapMap = {
				option: [1, "<select multiple='multiple'>", "</select>"],
				legend: [1, "<fieldset>", "</fieldset>"],
				area: [1, "<map>", "</map>"],
				param: [1, "<object>", "</object>"],
				thead: [1, "<table>", "</table>"],
				tr: [2, "<table><tbody>", "</tbody></table>"],
				col: [2, "<table><tbody></tbody><colgroup>", "</colgroup></table>"],
				td: [3, "<table><tbody><tr>", "</tr></tbody></table>"],

				// IE6-8 can't serialize link, script, style, or any html5 (NoScope) tags,
				// unless wrapped in a div with non-breaking characters in front of it.
				_default: REasy.support.htmlSerialize ? [0, "", ""] : [1, "X<div>", "</div>"]
			};
			for (; i < l; i++) {
				elem = elems[i];

				if (elem || elem === 0) {

					// Add nodes directly
					if (REasy.type(elem) === "object") {
						REasy.merge(nodes, elem.nodeType ? [elem] : elem);

						// Convert non-html into a text node
					} else if (!rhtml.test(elem)) {
						nodes.push(context.createTextNode(elem));

						// Convert html into DOM nodes
					} else {
						tmp = tmp || safe.appendChild(context.createElement("div"));

						// Deserialize a standard representation
						tag = (rtagName.exec(elem) || ["", ""])[1].toLowerCase();
						wrap = wrapMap[tag] || wrapMap._default;

						tmp.innerHTML = wrap[1] + elem.replace(rxhtmlTag, "<$1></$2>") + wrap[2];

						// Descend through wrappers to the right content
						j = wrap[0];
						while (j--) {
							tmp = tmp.lastChild;
						}

						// Manually add leading whitespace removed by IE
						if (!REasy.support.leadingWhitespace && rleadingWhitespace.test(elem)) {
							nodes.push(context.createTextNode(rleadingWhitespace.exec(elem)[0]));
						}

						// Remove IE's autoinserted <tbody> from table fragments
						if (!REasy.support.tbody) {

							// String was a <table>, *may* have spurious <tbody>
							elem = tag === "table" && !rtbody.test(elem) ?
								tmp.firstChild :

								// String was a bare <thead> or <tfoot>
								wrap[1] === "<table>" && !rtbody.test(elem) ?
								tmp :
								0;

							j = elem && elem.childNodes.length;
							while (j--) {
								if (REasy.nodeName((tbody = elem.childNodes[j]), "tbody") && !tbody.childNodes.length) {
									elem.removeChild(tbody);
								}
							}
						}

						REasy.merge(nodes, tmp.childNodes);

						// Fix #12392 for WebKit and IE > 9
						tmp.textContent = "";

						// Fix #12392 for oldIE
						while (tmp.firstChild) {
							tmp.removeChild(tmp.firstChild);
						}

						// Remember the top-level container for proper cleanup
						tmp = safe.lastChild;
					}
				}
			}

			// Fix #11356: Clear elements from fragment
			if (tmp) {
				safe.removeChild(tmp);
			}

			i = 0;
			while ((elem = nodes[i++])) {


				// #4087 - If origin and destination elements are the same, and this is
				// that element, do not do anything
				if (selection && REasy.inArray(elem, selection) !== -1) {
					continue;
				}

				if (selection && jQuery.inArray(elem, selection) !== -1) {
					continue;
				}

				contains = jQuery.contains(elem.ownerDocument, elem);

				// Append to fragment
				tmp = getAll(safe.appendChild(elem), "script");
			}

			tmp = null;

			return safe;
		},

		isArray: Array.isArray || function (obj) {
			return REasy.type(obj) === "array";
		},

		likeArray: function (arr) {
			var type = REasy.type(arr);

			// The window, strings (and functions) also have 'length'
			// TODO: Tweaked logic slightly to handle Blackberry 4.7 RegExp issues #2
			return typeof arr.length !== 'number' && type !== "string" &&
				type !== "function" && type !== "regexp" || !REasy.isWindow(arr);
		},

		isWindow: function (obj) {
			return obj != null && obj == obj.window;
		},

		isEmptyObject: function (obj) {
			var name;
			for (name in obj) {
				return false;
			}
			return true;
		},
		isNumeric: function (obj) {
			return !isNaN(parseFloat(obj)) && isFinite(obj);
		},

		type: function (obj) {
			return obj == null ?
				String(obj) :
				class2type[core_toString.call(obj)] || "object";
		},

		error: function (msg) {
			throw new Error(msg);
		},

		// data: string of html
		// context (optional): If specified, the fragment will be created in this context, defaults to document
		// keepScripts (optional): If true, will include scripts passed in the html string
		parseHTML: function (data, context, keepScripts) {
			if (!data || typeof data !== "string") {
				return null;
			}
			if (typeof context === "boolean") {
				keepScripts = context;
				context = false;
			}
			context = context || document;

			var parsed = rsingleTag.exec(data),
				scripts = !keepScripts && [];

			// Single tag
			if (parsed) {
				return [context.createElement(parsed[1])];
			}

			parsed = REasy.buildFragment([data], context, scripts);
			if (scripts) {
				REasy(scripts).remove();
			}
			return REasy.merge([], parsed.childNodes);
		},

		parseJSON: function (data) {
			// Attempt to parse using the native JSON parser first
			if (window.JSON && window.JSON.parse) {
				return window.JSON.parse(data);
			}

			if (data === null) {
				return data;
			}

			if (typeof data === "string") {

				// Make sure leading/trailing whitespace is removed (IE can't handle it)
				data = REasy.trim(data);

				if (data) {
					// Make sure the incoming data is actual JSON
					// Logic borrowed from http://json.org/json2.js
					if (rvalidchars.test(data.replace(rvalidescape, "@")
							.replace(rvalidtokens, "]")
							.replace(rvalidbraces, ""))) {

						return (new Function("return " + data))();
					}
				}
			}

			REasy.error("Invalid JSON: " + data);
		},

		// Cross-browser xml parsing
		parseXML: function (data) {
			var xml,
				tmp;
			if (!data || typeof data !== "string") {
				return null;
			}
			try {
				if (window.DOMParser) { // Standard
					tmp = new DOMParser();
					xml = tmp.parseFromString(data, "text/xml");
				} else { // IE
					xml = new ActiveXObject("Microsoft.XMLDOM");
					xml.async = "false";
					xml.loadXML(data);
				}
			} catch (e) {
				xml = undefined;
			}
			if (!xml || !xml.documentElement || xml.getElementsByTagName("parsererror").length) {
				REasy.error("Invalid XML: " + data);
			}
			return xml;
		},

		isElement: function (o) {
			var toString;

			if (!o) {
				return false;
			}
			toString = core_toString.call(o);

			return toString.indexOf('HTML') !== -1 ||
				(toString === '[object Object]' && o.nodeType === 1 &&
					!(o instanceof Object));
		},

		nodeName: function (elem, name) {
			return elem.nodeName && elem.nodeName.toLowerCase() === name.toLowerCase();
		},

		isXML: function (elem) {
			// documentElement is verified for cases where it doesn't yet exist
			// (such as loading iframes in IE - #4833)
			var documentElement = elem && (elem.ownerDocument || elem).documentElement;
			return documentElement ? documentElement.nodeName !== "HTML" : false;
		},
		clone: function (elem, dataAndEvents, deepDataAndEvents) {
			var destElements, node, clone, i, srcElements,
				inPage = REasy.contains(elem.ownerDocument, elem);

			if (REasy.support.html5Clone || REasy.isXML(elem) || !rnoshimcache.test("<" + elem.nodeName + ">")) {
				clone = elem.cloneNode(true);

				// IE<=8 does not properly clone detached, unknown element nodes
			} else {
				fragmentDiv.innerHTML = elem.outerHTML;
				fragmentDiv.removeChild(clone = fragmentDiv.firstChild);
			}

			if ((!jQuery.support.noCloneEvent) &&
				(elem.nodeType === 1 || elem.nodeType === 11)) {

				// We eschew Sizzle here for performance reasons: http://jsperf.com/getall-vs-sizzle/2
				destElements = getAll(clone);
				srcElements = getAll(elem);

			}

			destElements = srcElements = node = null;

			// Return the cloned set
			return clone;
		},

		each: function (obj, callback, args) {
			var len = obj.length,
				isArray = isArraylike(obj),
				key,
				val,
				i;

			if (args) {
				if (isArray) {
					for (i = 0; i < len; i++) {
						val = callback.apply(obj[i], args);

						if (val === false) {
							break;
						}
					}
				} else {
					for (key in obj) {
						val = callback.apply(obj[key], args);

						if (val === false) {
							break;
						}
					}
				}

				// A special, fast, case for the most common use of each
			} else {
				if (isArray) {
					for (i = 0; i < len; i++) {
						val = callback.call(obj[i], i, obj[i]);

						if (val === false) {
							break;
						}
					}
				} else {
					for (key in obj) {
						val = callback.call(obj[key], key, obj[key]);

						if (val === false) {
							break;
						}
					}
				}
			}

			return obj;
		},

		// Use native String.trim function wherever possible
		trim: function (text) {
			return text == null ?
				"" :
				core_trim.call(text);
		},

		// results仅仅是在内部使用
		makeArray: function (arr, results) {
			var ret = results || [];

			if (arr != null) {
				if (isArraylike(Object(arr))) {
					REasy.merge(ret,
						typeof arr === "string" ?
						[arr] : arr
					);
				} else {
					core_push.call(ret, arr);
				}
			}

			return ret;
		},

		inArray: function (elem, arr, i) {
			var len;

			if (arr) {
				if (core_indexOf) {
					return core_indexOf.call(arr, elem, i);
				}

				len = arr.length;
				i = i ? i < 0 ? Math.max(0, len + i) : i : 0;

				for (i; i < len; i++) {
					// Skip accessing in sparse arrays
					if (i in arr && arr[i] === elem) {
						return i;
					}
				}
			}

			return -1;
		},

		merge: function (first, second) {
			var l,
				i = first.length,
				j = 0;
			if (second) {
				l = second.length;
			}
			if (typeof l === "number") {
				for (j; j < l; j++) {
					first[i++] = second[j];
				}
			} else {
				while (second && second[j] !== undefined) {
					first[i++] = second[j++];
				}
			}

			first.length = i;

			return first;
		},

		grep: function (elems, callback, inv) {
			var retVal,
				ret = [],
				length = elems.length,
				i = 0;

			inv = !!inv;

			// Go through the array, only saving the items
			// that pass the validator function
			for (i = 0; i < length; i++) {
				retVal = !!callback(elems[i], i);
				if (inv !== retVal) {
					ret.push(elems[i]);
				}
			}

			return ret;
		},

		map: function (elems, callback) {
			var values = [],
				len = elems.length,
				val,
				i,
				key;

			if (REasy.likeArray(elems)) {
				for (i = 0; i < len; i++) {
					val = callback(elems[i], i);

					if (val != null) {
						values.push(val);
					}
				}
			} else {
				for (key in elems) {
					val = callback(elems[key], key);

					if (val != null) {
						values.push(val);
					}
				}
			}

			// Flatten any nested arrays
			return values.concat.apply([], values);
		}
	});

	// Populate the class2type map
	REasy.each("Boolean Number String Function Array Date RegExp Object".split(" "), function (i, name) {
		class2type["[object " + name + "]"] = name.toLowerCase();
	});

	function isArraylike(obj) {
		var len = obj.length,
			type = REasy.type(obj);

		if (REasy.isWindow(obj)) {
			return false;
		}

		if (obj.nodeType === 1 && len) {
			return true;
		}

		return type === "array" || type !== "function" &&
			(len === 0 ||
				typeof len === "number" && len > 0 && (len - 1) in obj);
	}


	/*==============================================================================
	 * 事件处理模块
	 *============================================================================*/
	REasy.include({

		// 扩展事件处理模块时，立即执行的函数
		included: function (_this) {
			_this.modules.push('event');
		},

		on: function (type, callback) {
			return this.each(function (i) {
				var elem = this;

				if (!elem) {
					return false;
				}

				if (document.attachEvent) {
					elem.attachEvent("on" + type, function () {
						if (REasy.isFunction(callback)) {
							callback.apply(elem, arguments);
						}
					});
				} else if (document.addEventListener) {
					elem.addEventListener(type, function () {
						if (REasy.isFunction(callback)) {
							callback.apply(elem, arguments);
						}
					}, false);
				} else {
					if (REasy.isFunction(callback)) {
						elem["on" + type] = callback;
					}

				}
			});

		},

		delegate: function (target, type, callback) {
			return this.each(function (i) {
				var elem = this,
					targetElem,
					events = [];
				type = type.replace(/(.*)\.(.*$)/g, "$1");

				if (!elem) {
					return false;
				}

				if (REasy(elem).find(target).length) {
					REasy(elem).find(target).on(type, callback);
				} else {
					if (document.attachEvent) {
						if (type === "focus") {
							if (REasy.isFunction(callback)) {
								elem.onfocusin = function (e) {
									e = e || window.event;
									$(target).each(function () {
										targetElem = e.target || e.srcElement;
										if (this == targetElem) {
											callback.apply(targetElem, arguments);
										}
									});
								}
							}

						} else if (type === "blur") {
							if (REasy.isFunction(callback)) {
								//
								if (typeof elem.onfocusout == "function") {
									events.push(elem.onfocusout);
								}
								elem.onfocusout = function (e) {
									e = e || window.event;
									for (var i = 0;
										(i < events.length); i++) {
										events[i].apply(arguments);
									}
									$(target).each(function () {
										targetElem = e.target || e.srcElement;
										if (this == targetElem) {
											callback.apply(targetElem, arguments);
										}
									});
								}
							}
						} else {
							elem.attachEvent("on" + type, function (e) {
								$(target).each(function (i) {
									targetElem = e.target || e.srcElement;
									if (this == targetElem) {
										if (REasy.isFunction(callback)) {
											callback.apply(targetElem, arguments);
										}
									}
								})

							});
						}
					} else if (document.addEventListener) {
						elem.addEventListener(type, function (e) {
							$(target).each(function (i) {
								targetElem = e.target || e.srcElement;
								if (this == targetElem) {
									if (REasy.isFunction(callback)) {
										callback.apply(targetElem, arguments);
									}
								}
							})
						}, true);
					} else {
						if (REasy.isFunction(callback)) {
							elem["on" + type] = callback;
						}

					}
				}

			});
		},

		off: function (type, callback) {
			return this.each(function (i) {
				var elem = this;

				if (!elem) {
					return false;
				}
				if (document.attachEvent) {
					elem.detachEvent("on" + type, callback);
				} else if (document.addEventListener) {
					elem.removeEventListener(type, callback, false);
				} else {
					elem["on" + type] = null;
				}
			});
		}

	});

	/*==============================================================================
	 * DOM元素,CSS样式操作模块
	 *============================================================================*/
	var isSimple = /^.[^:#\[\.,]*$/,
		rnative = /^[^{]+\{\s*\[native code/,
		docElem = document.documentElement,
		cssNormalTransform = {
			letterSpacing: 0,
			fontWeight: 400
		},
		Rquery = null,
		getStyles,
		curCSS;

	rootREasy = REasy(document);

	REasy.extend({
		// Add in style property hooks for overriding the default
		// behavior of getting and setting a style property
		cssHooks: {
			opacity: {
				get: function (elem, computed) {
					if (computed) {
						// We should always get a number back from opacity
						var ret = curCSS(elem, "opacity");
						return ret === "" ? "1" : ret;
					}
				}
			}
		},

		// Exclude the following css properties to add px
		cssNumber: {
			"columnCount": true,
			"fillOpacity": true,
			"fontWeight": true,
			"lineHeight": true,
			"opacity": true,
			"orphans": true,
			"widows": true,
			"zIndex": true,
			"zoom": true
		},

		// Add in properties whose names you wish to fix before
		// setting or getting the value
		cssProps: {
			// normalize float css property
			/* "float": (!!a.style.cssFloat) ? "cssFloat" : "styleFloat" */
		},

		// Get and set the style property on a DOM Node
		style: function (elem, name, value, extra) {
			// Don't set styles on text and comment nodes
			if (!elem || elem.nodeType === 3 || elem.nodeType === 8 || !elem.style) {
				return;
			}

			// Make sure that we're working with the right name
			var ret,
				type,
				hooks,
				style = elem.style;

			name = REasy.cssProps[name] || name;

			// gets hook for the prefixed version
			// followed by the unprefixed version
			hooks = REasy.cssHooks[name];

			// Check if we're setting a value
			if (value !== undefined) {
				type = typeof value;

				// If a hook was provided, use that value, otherwise just set the specified value
				if (!hooks || !("set" in hooks) || (value = hooks.set(elem, value, extra)) !== undefined) {

					// Wrapped to prevent IE from throwing errors when 'invalid' values are provided
					// Fixes bug #5509
					try {
						style[name] = value;
					} catch (e) {}
				}

			} else {
				// If a hook was provided get the non-computed value from there
				if (hooks && "get" in hooks && (ret = hooks.get(elem, false, extra)) !== undefined) {
					return ret;
				}

				// Otherwise just get the value from the style object
				return style[name];
			}
		},

		css: function (elem, name, extra, styles) {
			var num,
				val,
				hooks;

			// Make sure that we're working with the right name

			name = REasy.cssProps[name] || name;

			// gets hook for the prefixed version
			// followed by the unprefixed version
			hooks = REasy.cssHooks[name];

			// If a hook was provided get the computed value from there
			if (hooks && "get" in hooks) {
				val = hooks.get(elem, true, extra);
			}

			// Otherwise, if a way to get the computed value exists, use that
			if (val === undefined) {
				val = curCSS(elem, name, styles);
			}

			//convert "normal" to computed value
			if (val === "normal" && name in cssNormalTransform) {
				val = cssNormalTransform[name];
			}

			// Return, converting to number if forced or a qualifier was provided and val looks numeric
			if (extra === "" || extra) {
				num = parseFloat(val);
				return extra === true || REasy.isNumeric(num) ? num || 0 : val;
			}
			return val;
		}
	});


	// NOTE: we've included the "window" in window.getComputedStyle
	// because jsdom on node.js will break without it.
	if (window.getComputedStyle) {
		getStyles = function (elem) {
			return window.getComputedStyle(elem, null);
		};

		curCSS = function (elem, name, _computed) {
			var width,
				minWidth,
				maxWidth,
				computed = _computed || getStyles(elem),

				ret = computed ? computed.getPropertyValue(name) || computed[name] : undefined,
				style = elem.style;

			if (computed) {
				if (ret === "" && !REasy.contains(elem.ownerDocument, elem)) {
					ret = REasy.style(elem, name);
				}
			}

			return ret;
		};
	} else if (document.documentElement.currentStyle) {
		getStyles = function (elem) {
			return elem.currentStyle;
		};

		curCSS = function (elem, name, _computed) {
			var left,
				rs,
				rsLeft,
				computed = _computed || getStyles(elem),
				ret = computed ? computed[name] : undefined,
				style = elem.style;

			// Avoid setting ret to empty string here
			// so we don't default to auto
			if (ret == null && style && style[name]) {
				ret = style[name];
			}

			return ret === "" ? "auto" : ret;
		};
	}

	// REasy querySelector
	Rquery = (function (window, document) {
		var support = {

			},

			// Local document vars
			setDocument,
			docElem,
			documentIsXML,
			rbuggyQSA,
			rbuggyMatches,
			matches,
			contains,
			sortOrder;

		/**
		 * For feature detection
		 * @param {Function} fn The function to test for native support
		 */
		function isNative(fn) {
			return rnative.test(String(fn));
		}

		function getByClass(selector, context) {
			var elems = [],
				all = (context || document).getElementsByTagName('*'),
				len = all.length,
				i = 0,
				classStr;

			for (i; i < len; i++) {
				classStr = ' ' + all[i].className + ' ';
				if (classStr.indexOf(' ' + selector + ' ') !== -1) {
					elems.push(all[i]);
				}
			}
			return elems;
		}

		function querySelectorAll(selector, context) {
			context = context || document;

			var Expri = {
					TAG: /^((?:\\.|[\w*-]|[^\x00-\xa0])+)/,
					CLASS: /^\.((?:\\.|[\w-]|[^\x00-\xa0])+)/,
					ATTR: /^\[[\x20\t\r\n\f]*((?:\\.|[\w-]|[^\x00-\xa0])+)[\x20\t\r\n\f]*(?:([*^$|~]?=)[\x20\t\r\n\f]*(?:(['"])((?:\\.|[^\\])*?)\3|((?:\\.|[\w#-]|[^\x00-\xa0])+)|)|)[\x20\t\r\n\f]*\]/,
					ID: /^#((?:\\.|[\w-]|[^\x00-\xa0])+)/
				},

				/*> +  ~关系符*/
				relativeReg = /^[\x20\t\r\n\f]*([\x20\t\r\n\f>+~])[\x20\t\r\n\f]*/,
				rtrim = /^[\x20\t\r\n\f]+|((?:^|[^\\])(?:\\.)*)[\x20\t\r\n\f]+$/g,
				relative = {
					">": {
						dir: "parentNode",
						first: true
					},
					" ": {
						dir: "parentNode"
					},
					"+": {
						dir: "previousSibling",
						first: true
					},
					"~": {
						dir: "previousSibling"
					}
				},

				//the attribute for IE7
				Hook = {
					tabindex: 'tabIndex',
					readonly: 'readOnly',
					'for': 'htmlFor',
					'class': 'className',
					maxlength: 'maxLength',
					cellspacing: 'cellSpacing',
					cellpadding: 'cellPadding',
					rowspan: 'rowSpan',
					colspan: 'colSpan',
					usemap: 'useMap',
					frameborder: 'frameBorder',
					contenteditable: 'contentEditable'
				},

				getDomArray = {
					"ID": function (value) {
						var ret = [],
							result = context.getElementById(value);
						if (result == null || (result && result.id != value)) {
							return false;
						}
						ret[0] = result;
						return ret;
					},
					"TAG": function (value) {
						var ret = context.getElementsByTagName(value);
						if (0 === ret.length) {
							return false;
						}
						return ret;
					},
					"CLASS": function (value) {
						var ret = support.byClass ?
							context.getElementsByClassName(value) :
							getByClass(value, context);

						if (0 === ret.length) {
							return false;
						}
						return ret;
					},
					"ATTR": function (value) {
						var ret = [],
							attr = value[0],
							attflag = value[1] === undefined ? "" : value[1],
							attrvalue = value[2],
							elements = context.getElementsByTagName("*"),
							len = elements.length,
							i = 0,
							name = "";

						for (i; i < len; i++) {
							//name = elements[i].getAttribute(attr) || (attr == "class" ? elements[i].getAttribute("className") :  elements[i].getAttribute(attr));
							name = attrHooks[attr] ? attrHooks[attr](elements[i]) : (elements[i].getAttribute(attr) || elements[i][Hook[attr]]);
							//name = name ? name.replace(/^\s+|\s+$/g,"") : "";	
							//replace only support string, but name may object when the attr is style
							//TODO: style attr here is not supported
							name = typeof name == "string" ? name.replace(/^\s+|\s+$/g, "") : "";
							switch (attflag) {
							case "":
								//if (name != "" || name != null) {
								if (name != "") {
									ret.push(elements[i]);
								}
								break;
							case "=":
								if (name == attrvalue) {
									ret.push(elements[i]);
								}
								break;
							case "~=":
								//just only one ,eg: a b c,only get a or b ,if a b, it cannot get
								name = " " + name + " ";
								//attrvalue="a b c", it cannot get elem
								if (attrvalue.indexOf(' ') != -1) {
									break;
								}
								var attrvalue1 = " " + attrvalue + " ";
								if (name.indexOf(attrvalue1) != -1) {
									ret.push(elements[i]);
								}
								break;
							case "^=":
								name = name.slice(0, attrvalue.length);
								if (name == attrvalue) {
									ret.push(elements[i]);
								}
								break;
							case "$=":
								name = name.slice(-attrvalue.length);
								if (name == attrvalue) {
									ret.push(elements[i]);
								}
								break;
							case "*=":
								if (name.indexOf(attrvalue) != -1) {
									ret.push(elements[i]);
								}
								break;
							case "|=":
								name = name.split("-")[0];
								if (name == attrvalue) {
									ret.push(elements[i]);
								}
								break;
							default:
								break;
							}
						}
						return ret;
					}
				},

				Filter = {
					"TAG": function (mached, target) {
						if (target.nodeType == 1) {
							return target.getAttribute("tagName");
						} else {
							return null;
						}
					},
					"CLASS": function (mached, target) {
						if (target.nodeType == 1) {
							return target.getAttribute("className");
						} else {
							return null;
						}
					},
					"ID": function (mached, target) {
						if (target.nodeType == 1) {
							return target.getAttribute("id");
						} else {
							return null;
						}
					},
					"ATTR": function (matched, target) {
						var result;

						if (target.nodeType == 1) {
							/*if (matched[0] == "class" && !target.getAttribute("class")) {
								matched[0] = "className";	
							}*/
							result = attrHooks[matched[0]] ?
								attrHooks[matched[0]](target) :
								(target.getAttribute(matched[0]) || target[Hook[matched[0]]]);
							return typeof result == "string" ? result : "";
						} else {
							return null;
						}
					}
				},

				runescape = /\\([\da-fA-F]{1,6}[\x20\t\r\n\f]?|.)/g,
				funescape = function (_, escaped) {
					var high = "0x" + escaped - 0x10000;
					// NaN means non-codepoint
					return high !== high ?
						escaped :
						// BMP codepoint
						high < 0 ?
						String.fromCharCode(high + 0x10000) :
						// Supplemental Plane codepoint (surrogate pair)
						String.fromCharCode(high >> 10 | 0xD800, high & 0x3FF | 0xDC00);
				},

				select = function (tokens) {
					var types = "",
						nodeType = "",
						value = "",
						len = 0,
						dir = "",
						result = [],
						ret = [],
						first = false,
						token = {},
						//handle:div>table table
						relativation = [],
						relativations = [],
						textNode = [],
						i;

					while (token = tokens.pop()) {
						types = token.type;
						value = types == "ATTR" ?
							token.matches :
							(token.matches ? token.matches[0] : token.value);

						if (token && relative[types]) {
							//handle .div > #div
							//ret = document.getElementById("div");
							dir = relative[types].dir;
							first = relative[types].first;
							len = ret.length;
							token = tokens.pop();
							types = token.type;
							value = types == "ATTR" ? token.matches[2] : (token.matches ? token.matches[0] : token.value);
							textNode = [];
							tmp = "";
							for (i = 0; i < len; i++) {
								//ret[i].parentNode.className == "div", id, class, tag, |attr
								textNode = [];
								// > 选择第一代子节点
								if (dir == "parentNode" && first) {
									//对于relativation这个二维数组，是为了处理$("#testdiv>div #div411")和$(“#testdiv>div+div”)
									//即#div411可能有多个祖先元素，但并不是每个祖先元素都是#testdiv的第一子代
									//因此需要用来记录多个祖先元素或则兄弟元素
									//1,找到#div411; 2,找到#div411的所有祖先元素，并添加到对应的relativation数组中
									//3，对于div即relativation数组中的祖先元素来查找他们的父元素#testdiv
									var relatiArr = [];
									if (relativation[i] && relativation[i][0]) {
										textNode = relativation[i];
									} else {
										textNode[0] = ret[i];
									}
									var node = [];
									// [name=valueP], 查找父元素的name属性， 如果符合条件，择将此放入数组中
									while (node[0] = textNode.pop()) {
										//tmp = Filter[types](token.matches, ret[i][dir]);
										tmp = Filter[types](token.matches, node[0][dir]);
										tmp = tmp ? tmp.toLowerCase().replace(/^\s+|\s+$/g, "") : tmp;
										//if (tmp && ((types == "ATTR" && getAttr(tmp, token.matches)) || tmp == value || tmp.has(value))) {
										if (tmp && ((types == "ATTR" && getAttr(tmp, token.matches)) || tmp == value ||
												(" " + tmp + " ").indexOf(value) === 0)) {
											relatiArr.push(node[0][dir]);
											if ((result.length - 1) != ret[i]) {
												result.push(ret[i]);
											}
										}
									}
									if (relatiArr.length !== 0) {
										relativations.push(relatiArr);
									}
								} else if (dir == "parentNode" && !first) {
									//textNode.push(ret[i].parentNode);
									//" "选择所有后代节点
									var relatiArr = [];
									if (relativation[i] && relativation[i][0]) {
										//textNode[0] = relativation[i][dir];	
										textNode = relativation[i];
									} else {
										textNode[0] = ret[i];
									}
									var node = [];
									while (node[0] = textNode.pop()) {
										//tmp = textNode[0][Filter[types](token.matches)];
										while (node[0][dir]) {
											tmp = Filter[types](token.matches, node[0][dir]);
											tmp = tmp ? tmp.toLowerCase().replace(/^\s+|\s+$/g, "") : tmp;
											//if (tmp && ((types == "ATTR" && getAttr(tmp, token.matches)) || tmp == value || tmp.has(value))) {
											if (tmp && ((types == "ATTR" && getAttr(tmp, token.matches)) || tmp == value ||
													(" " + tmp + " ").indexOf(value) == 0)) {
												relatiArr.push(node[0][dir]);
												if (result[result.length - 1] != ret[i]) {
													result.push(ret[i]);
												}
												//break;
											}
											node[0] = node[0][dir];

										}

									}
									if (relatiArr.length != 0) {
										relativations.push(relatiArr);
									}

									//为空的文本节点
									/*while(textNode[0]) {
										if (textNode[0].nodeType == 3 && (!/\S+/.test(textNode[0].nodeValue))) {
											textNode[0] = texNode[0].previousSibling;	
										} else {
											break;	
										}
									}*/
								} else if (dir == "previousSibling" && !first) {
									// E~F E后面的所有兄弟节点F
									var relatiArr = [];
									if (relativation[i] && relativation[i][0]) {
										//textNode[0] = relativation[i][dir];	
										textNode = relativation[i];
									} else {
										textNode[0] = ret[i];
									}
									var node = [];
									while (node[0] = textNode.pop()) {
										//如果为空白, 则继续寻找F前面的节点
										/*if (textNode[0].nodeType == 3 && (!/\S+/.test(textNode[0].nodeValue))) {
											textNode[0] = textNode[0][dir];	
										} else */
										//tmp = textNode[0][Filter[types](token.matches)];
										while (node[0][dir]) {
											tmp = Filter[types](token.matches, node[0][dir]);
											tmp = tmp ? tmp.toLowerCase().replace(/^\s+|\s+$/g, "") : tmp;
											//if (tmp && ((types == "ATTR" && getAttr(tmp, token.matches)) || tmp == value || tmp.has(value))) {
											if (tmp && ((types == "ATTR" && getAttr(tmp, token.matches)) || tmp == value ||
													(" " + tmp + " ").indexOf(value) === 0)) {
												relatiArr.push(node[0][dir]);

												if (result[result.length - 1] != ret[i]) {
													result.push(ret[i]);
												}
											}
											//为空白节点或F的前一个兄弟不是，则继续寻找再前一个兄弟，直到textNode[0]为空为止
											node[0] = node[0][dir];

										}

									}
									if (relatiArr.length !== 0) {
										relativations.push(relatiArr);
									}
								} else if ((dir == "previousSibling") && first) {
									var relatiArr = [];
									// E+F 紧贴在E后面的F元素 
									if (relativation[i] && relativation[i][0]) {
										textNode = relativation[i];
									} else {
										textNode[0] = ret[i];
									}
									var node = [];
									while (node[0] = textNode.pop()) {
										//如果前一个为空白节点，则继续往前找，直到找到一个为止
										//tmp = textNode[0][Filter[types](token.matches)];
										while (node[0][dir]) {
											tmp = Filter[types](token.matches, node[0][dir]);
											tmp = tmp ? tmp.toLowerCase().replace(/^\s+|\s+$/g, "") : tmp;


											if (node[0][dir].nodeType == 3 && (!/\S+/.test(node[0][dir].nodeValue))) {
												node[0] = node[0][dir];
											} else if (tmp && ((types == "ATTR" && getAttr(tmp, token.matches)) || tmp == value ||
													(" " + tmp + " ").indexOf(value) === 0)) {
												relatiArr.push(node[0][dir]);
												if (result[result.length - 1] != ret[i]) {
													result.push(ret[i]);
												}
												break;
											} else {
												//前一个节点不是空白节点，但不符合要求
												break;
											}

										}
									}
									if (relatiArr.length !== 0) {
										relativations.push(relatiArr);
									}
								}
							}
							ret = result;
							relativation = relativations;
							relativations = [];
							result = [];
						} else if (0 === ret.length) {

							ret = getDomArray[types](value);
						} else {
							//handle #div.div[type='button']
							//ret = document.getElementsByClassName("div");
							len = ret.length;
							var tmp = "";
							for (i = 0; i < len; i++) {
								//tmp = ret[i][Filter[types](token.matches)];
								tmp = Filter[types](token.matches, ret[i]);
								tmp = tmp ? tmp.toLowerCase().replace(/^\s+|\s+$/g, "") : tmp;
								//if(tmp && ((types == "ATTR" && getAttr(tmp, token.matches)) || tmp == value || tmp.has(value))) {
								if (tmp && ((types == "ATTR" && getAttr(tmp, token.matches)) || tmp == value ||
										(" " + tmp + " ").indexOf(value) === 0)) {
									result.push(ret[i]);
								}
							}
							ret = result;
							result = [];
						}
						//当第一次取数据以后，仍然为空，则说明没有要找的元素，则不用再查找
						if (0 === ret.length) {
							break;
						}
						/*} else {
							return [];	
						}*/
					}

					return ret;
				},
				getAttr = function (name, matches) {
					var value = matches[2],
						relative = matches[1],
						flag = -1;
					name = typeof name == "string" ? name.replace(/^\s+|\s+$/g, "") : "";
					switch (relative) {
					case "":

						if (name !== "") {
							flag = 0;
						}
						break;
					case "=":
						if (name == value) {
							flag = 0;
						}
						break;
					case "~=":
						name = " " + name + " ";
						if (value.index(" ") != -1) {
							break;
						}
						var value1 = " " + value + " ";
						if (name.indexOf(value1) != -1) {
							flag = 0;
						}
						break;
					case "^=":
						name = name.slice(0, value.length);
						if (name == value) {
							flag = 0;
						}
						break;
					case "$=":
						name = name.slice(-value.length);
						if (name == value) {
							flag = 0;
						}
						break;
					case "*=":
						if (name.indexOf(value) != -1) {
							flag = 0;
						}
						break;
					case "|=":
						name = name.split("-")[0];
						if (name == value) {
							flag = 0;
						}
						break;
					default:
						break;
					}
					if (flag === 0) {
						return true;
					} else {
						return false;
					}

				};
			//Some attributes require a special call on IE
			var attrHooks = {};
			REasy.each(["href", "src", "width", "height"], function (i, name) {
				attrHooks[name] = function (elem) {
					var ret = elem.getAttribute(name, 2);
					return ret == null ? undefined : ret;
				};
			});
			//IE8 the attribute of ret is upper, but chrome is lower, eg: ret = "width: 102px;", IE is "WIDTH: 102px"
			attrHooks['style'] = function (elem) {
				var ret = elem['style'].cssText.toLowerCase();
				return ret == null ? undefined : ret;
			}
			var soFar = selector,
				match = [],
				tokens = [],
				token = "",
				matched = false,
				type;
			while (soFar != "") {
				if ((match = relativeReg.exec(soFar))) {
					matched = match.shift();
					tokens.push({
						value: matched,
						type: match[0].replace(rtrim, " ")
					});
					soFar = soFar.slice(matched.length);
				}
				for (type in Expri) {
					if (Expri.hasOwnProperty(type)) {
						match = Expri[type].exec(soFar);

						if (match != null) {
							if (type == "ATTR") {
								match[1] = match[1].replace(runescape, funescape);
								match[3] = (match[4] || match[5] || "").replace(runescape, funescape);

								match = match.slice(0, 4);
							}
							matched = match.shift();
							tokens.push({
								matches: match,
								type: type,
								value: matched
							});
							soFar = soFar.slice(matched.length);
							break;
						}
					}
				}
				if (!matched) {
					break;
				}
			}
			return select(tokens);
		}

		docElem = document.documentElement;
		support.qs = isNative(document.querySelectorAll);
		support.qsa = isNative(document.querySelectorAll);
		support.byClass = isNative(document.getElementsByClassName);

		// Extend document native function
		// ===========================================
		if (!support.byClass) {
			document.getElementsByClassName = getByClass;
		}
		if (!support.qsa) {
			document.querySelectorAll = querySelectorAll;
		}
		if (!support.qs) {
			document.querySelector = function (selector) {
				return querySelectorAll(selector)[0];
			}
		}

		contains = isNative(docElem.contains) || docElem.compareDocumentPosition ?
			function (a, b) {
				var adown = a.nodeType === 9 ? a.documentElement : a,
					bup = b && b.parentNode;

				return a === bup || !!(bup && bup.nodeType === 1 && (
					adown.contains ?
					adown.contains(bup) :
					a.compareDocumentPosition && a.compareDocumentPosition(bup) & 16
				));
			} :
			function (a, b) {
				if (b) {
					while ((b = b.parentNode)) {
						if (b === a) {
							return true;
						}
					}
				}
				return false;
			};

		// Export object
		return {
			selectAll: isNative(document.querySelectorAll) ?
				function (selector, context) {
					try {
						return (context || document).querySelectorAll(selector);
					} catch (e) {
						//for :checked or :selected
						if (rboolean.test(selector)) {
							var selectorArry = selector.split(":"),
								arry = (context || document).querySelectorAll(selectorArry[0]),
								i = 0,
								l = arry.length;
							for (; i < l; i++) {
								if (arry[i][selectorArry[1]]) {
									return {
										0: arry[i],
										length: 1,
										context: document
									}
								}
							}
						}
					}
				} : function (selector, context) {
					return querySelectorAll(selector, context || document);
				},

			contains: contains
		}
	})(window, document);

	function sibling(cur, dir) {
		do {
			cur = cur[dir];
		} while (cur && cur.nodeType !== 1);

		return cur;
	}

	REasy.extend({

		find: function (expr, elems, ret) {
			var len = elems.length,
				nodeType,
				i;

			ret = ret || [];

			if (!expr || typeof expr !== "string") {
				return ret;
			}

			nodeType = elems.nodeType || (elems[0] ? elems[0].nodeType : -1);
			if (nodeType !== 1 && nodeType !== 9) {
				return ret;
			}

			// HANDLE: One element 
			if (!len) {
				REasy.merge(ret, Rquery.selectAll(expr, elems));

				// HANDLE: Two or more elements 
			} else {
				for (i = 0; i < len; i++) {
					REasy.merge(ret, Rquery.selectAll(expr, elems[i]));
				}
			}

			return ret;
		},

		contains: Rquery.contains,

		//获取元素位置坐标坐标 
		getPosition: function (e) {
			var offsetY = e.offsetTop,
				offsetX = e.offsetLeft,
				position;

			if (e.offsetParent != null) {
				position = REasy.getPosition(e.offsetParent);
				offsetY += position.top + REasy.css(e, 'borderTopWidth', true);
				offsetX += position.left + REasy.css(e, 'borderLeftWidth', true);
			}
			return {
				top: offsetY,
				left: offsetX
			};
		},

		getValue: function (elem) {
			if (typeof elem.value !== "undefined") {
				return elem.value;
			} else if (REasy.isFunction(elem.val)) {
				return elem.val();
			}
		},

		setValue: function (elem, val) {
			if (typeof val !== "undefined") {
				return false;
			}

			if (typeof elem.value !== "undefined") {
				elem.value = val;
			} else if (REasy.isFunction(elem.val)) {
				elem.val(val);
			}
		}
	});

	function getWindow(elem) {
		return REasy.isWindow(elem) ?
			elem :
			elem.nodeType === 9 ?
			elem.defaultView || elem.parentWindow :
			false;
	}

	// Implement the identical functionality for filter and not
	function winnow(elements, qualifier, keep) {

		// Can't pass null or undefined to indexOf in Firefox 4
		// Set to 0 to skip string check
		qualifier = qualifier || 0;

		if (REasy.isFunction(qualifier)) {
			return REasy.grep(elements, function (elem, i) {
				var retVal = !!qualifier.call(elem, i, elem);
				return retVal === keep;
			});

		} else if (qualifier.nodeType) {
			return REasy.grep(elements, function (elem) {
				return (elem === qualifier) === keep;
			});

		} else if (typeof qualifier === "string") {
			var filtered = REasy.grep(elements, function (elem) {
				return elem.nodeType === 1;
			});

			if (isSimple.test(qualifier)) {
				return REasy.filter(qualifier, filtered, !keep);
			} else {
				qualifier = REasy.filter(qualifier, filtered);
			}
		}

		return REasy.grep(elements, function (elem) {
			return (REasy.inArray(elem, qualifier) >= 0) === keep;
		});
	}

	function createSafeFragment(document) {
		var list = nodeNames.split("|"),
			safeFrag = document.createDocumentFragment();

		if (safeFrag.createElement) {
			while (list.length) {
				safeFrag.createElement(
					list.pop()
				);
			}
		}
		return safeFrag;
	}

	function findOrAppend(elem, tag) {
		return elem.getElementsByTagName(tag)[0] || elem.appendChild(elem.ownerDocument.createElement(tag));
	}

	REasy.include({
		// 扩展事件处理模块时，立即执行的函数
		included: function (_this) {
			_this.modules.push('dom');
		},

		find: function (selector) {
			var len = this.length,
				ret,
				i,
				self;

			if (typeof selector !== "string") {
				self = this;

				return this.pushStack(REasy(selector).filter(function () {
					for (i = 0; i < len; i++) {
						if (REasy.contains(self[i], this)) {
							return true;
						}
					}
				}));
			}

			ret = [];
			REasy.find(selector, this, ret);

			// Needed because $( selector, context ) becomes $( context ).find( selector )
			ret = this.pushStack(ret);
			ret.selector = (this.selector ? this.selector + " " : "") + selector;

			return ret;
		},

		not: function (selector) {
			return this.pushStack(winnow(this, selector, false));
		},

		filter: function (selector) {
			return this.pushStack(winnow(this, selector, true));
		},

		html: function (value) {
			var elem = this[0] || {},
				i = 0,
				l = this.length;
			var wrapMap = {
				option: [1, "<select multiple='multiple'>", "</select>"],
				legend: [1, "<fieldset>", "</fieldset>"],
				area: [1, "<map>", "</map>"],
				param: [1, "<object>", "</object>"],
				thead: [1, "<table>", "</table>"],
				tr: [2, "<table><tbody>", "</tbody></table>"],
				col: [2, "<table><tbody></tbody><colgroup>", "</colgroup></table>"],
				td: [3, "<table><tbody><tr>", "</tr></tbody></table>"],

				// IE6-8 can't serialize link, script, style, or any html5 (NoScope) tags,
				// unless wrapped in a div with non-breaking characters in front of it.
				_default: REasy.support.htmlSerialize ? [0, "", ""] : [1, "X<div>", "</div>"]
			};
			if (value === undefined) {
				return elem.nodeType === 1 ?
					elem.innerHTML : undefined;
			}
			if (typeof value === "string" && !rnoInnerhtml.test(value) &&
				(jQuery.support.htmlSerialize || !rnoshimcache.test(value)) &&
				(jQuery.support.leadingWhitespace || !rleadingWhitespace.test(value)) &&
				!wrapMap[(rtagName.exec(value) || ["", ""])[1].toLowerCase()]) {

				value = value.replace(rxhtmlTag, "<$1></$2>");

				try {
					for (; i < l; i++) {
						// Remove element nodes and prevent memory leaks
						elem = this[i] || {};
						if (elem.nodeType === 1) {
							elem.innerHTML = value;
						}
					}
					elem = 0;

					// If using innerHTML throws an exception, use the fallback method
				} catch (e) {}
			}

			if (elem) {
				this.empty().append(value);
			}
			return this;
		},

		text: function (text) {
			if (typeof text == "undefined") {
				if (typeof this[0].innerText != "undefined") {
					return this[0].innerText;
				} else { //firefox
					return this[0].textContent;
				}
			} else {
				return this.each(function (i) {
					if (typeof this.innerText != "undefined") {
						this.innerText = text;
					} else { //firefox
						this.textContent = text;
					}
				});
			}
			return this;
		},

		attr: function (name, str) {

			if (typeof str == "undefined") {
				return this[0].getAttribute(name);
			} else {
				return this.each(function (i) {
					this.setAttribute(name, str);
				});
			}
		},

		removeAttr: function (name) {
			return this.each(function (i) {
				this.removeAttribute(name);
			});
		},

		focus: function () {
			if (typeof this[0] == "undefined") {
				return this;
			} else {
				//解决IE8 元素隐藏时，不聚焦
				if (!REasy.isHidden(this[0])) {
					this[0].focus();
				}
				return this;
			}
		},

		height: function (height) {
			if (!height) {
				return this[0].offsetHeight;
			} else {
				return this.each(function (i) {
					this.css("height", height + "px");
				});
			}
		},

		domManip: function (args, table, callback) {
			var first, node, hasScripts,
				scripts, doc, fragment,
				i = 0,
				l = this.length,
				set = this,
				iNoClone = l - 1,
				value = args[0],
				isFunction = REasy.isFunction(value);

			if (l) {
				fragment = REasy.buildFragment(args, this[0].ownerDocument, false, this);
				first = fragment.firstChild;

				if (fragment.childNodes.length === 1) {
					fragment = first;
				}

				if (first) {
					table = table && REasy.nodeName(first, "tr");

					// Use the original fragment for the last item instead of the first because it can end up
					// being emptied incorrectly in certain situations (#8070).
					for (; i < l; i++) {
						node = fragment;

						if (i !== iNoClone) {
							node = REasy.clone(node, true, true);
						}

						callback.call(
							table && REasy.nodeName(this[i], "table") ?
							findOrAppend(this[i], "tbody") :
							this[i],
							node,
							i
						);
					}
				}
			}
			return this;
		},

		append: function (node) {
			return this.domManip(arguments, true, function (node) {
				if (this.nodeType === 1 || this.nodeType === 11 || this.nodeType === 9) {
					this.appendChild(node);
				}
			});
		},
		empty: function () {

			var elem,
				i = 0;

			for (;
				(elem = this[i]) != null; i++) {


				// Remove any remaining nodes
				while (elem.firstChild) {
					elem.removeChild(elem.firstChild);
				}

				// If this is a select, ensure that it displays empty (#12336)
				// Support: IE<9
				if (elem.options && REasy.nodeName(elem, "select")) {
					elem.options.length = 0;
				}
			}

			return this;
		},

		appendTo: function (node) {
			REasy(node).append(this[0]);
		},
		prepend: function (node) {
			if (!$.isElement(node)) {
				return this;
			}
			return this.each(function () {
				if (this.nodeType === 1 || this.nodeType === 11 ||
					this.nodeType === 9) {
					this.insertBefore(elem, this.firstChild);
				}
			});
		},

		before: function (node) {
			if (!$.isElement(node)) {
				return this;
			}
			return this.each(function () {
				this.parentNode.insertBefore(node, this);
			});
		},

		after: function (node) {
			if (!$.isElement(node)) {
				return this;
			}
			return this.each(function () {
				var parent = this.parentNode;
				if (parent.lastChild == this) {
					parent.appendChild(node);
				} else {
					parent.insertBefore(node, this.nextSibling);
				}

			});
		},

		remove: function () {
			return this.each(function () {
				var parentElem = this && this.parentNode;
				if ($.isElement(parentElem)) {
					parentElem.removeChild(this);
				}

			});
		},

		css: function (name, value) {

			//TODO 暂时修改,以解决不能赋值问题
			//if (REasy.type(name) === "object" && value === undefined) {
			//for (var prop in name) {
			//	if (name.hasOwnProperty(prop)) {
			//		this.css(prop, name[prop]);
			//	}
			//}

			if (REasy.type(name) === "string" && value === undefined) {
				return this[0].style[name];
			} else {
				this.each(function () {
					if (value !== undefined) {
						try {
							this.style[name] = value;
						} catch (ex) {

						}
					}
				});
			}
			return this;
		},

		val: function (val) {
			if (typeof this[0] == "undefined") {
				return undefined;
			}
			if (typeof val == "undefined") {
				return this[0].value;
			}
			return this.each(function () {
				if (this.value !== "undefined") {
					this.value = val;
				} else if (REasy.isFunction(this.val)) {
					this.val(val);
				}
			});
		},

		offset: function () {
			/* return REasy.getPosition(this[0]); */
			if (arguments.length) {
				return options === undefined ?
					this :
					this.each(function (i) {
						REasy.offset.setOffset(this, options, i);
					});
			}

			var docElem, win,
				box = {
					top: 0,
					left: 0
				},
				elem = this[0],
				doc = elem && elem.ownerDocument;

			if (!doc) {
				return;
			}

			docElem = doc.documentElement;

			// Make sure it's not a disconnected DOM node
			if (!REasy.contains(docElem, elem)) {
				return box;
			}

			// If we don't have gBCR, just use 0,0 rather than error
			// BlackBerry 5, iOS 3 (original iPhone)
			if (typeof elem.getBoundingClientRect !== undefined) {
				box = elem.getBoundingClientRect();
			}
			win = getWindow(doc);
			return {
				top: box.top + (win.pageYOffset || docElem.scrollTop) - (docElem.clientTop || 0),
				left: box.left + (win.pageXOffset || docElem.scrollLeft) - (docElem.clientLeft || 0)
			};
		},

		show: function () {
			return this.each(function (i) {
				REasy(this).css('display', 'block');
			});
		},

		hide: function () {
			return this.each(function (i) {
				REasy(this).css('display', 'none');
			});
		},

		fadeIn: function (time, callback) {
			return this.each(function (i) {
				var elem = this;
				setTimeout(function () {
					REasy(elem).show();
					if (typeof callback == "function") {
						callback.apply();
					}

				}, time)
			})

		},

		fadeOut: function (time, callback) {
			return this.each(function (i) {
				var elem = this;
				setTimeout(function () {
					REasy(elem).hide();
					if (typeof callback == "function") {
						callback.apply();
					}
				}, time)
			})
		},

		hasClass: function (classStr) {
			var elem = this[0],
				str;

			if (typeof elem === "undefined") {
				return false;
			}
			classStr = classStr || "";
			str = " " + elem.className + " ";

			return str.indexOf(" " + classStr.trim() + " ") !== -1;
		},

		addClass: function (classStr) {
			if (REasy.type(classStr) === 'string') {
				return this.each(function (i) {
					if ($(this).hasClass(classStr)) {
						return;
					} else {
						this.className += this.className === '' ?
							classStr : " " + classStr;
					}
				});
			}

			return this;
		},

		removeClass: function (classStr) {

			if (REasy.type(classStr) === 'string') {
				return this.each(function (i) {
					var newName,
						str;

					if ($(this).hasClass(classStr)) {
						str = " " + this.className + " ";
						newName = str.replace(" " + classStr + " ", " ").trim();
						this.className = newName;

						return this;
					}
				});

			}
		},
		load: function (url, callback) {
			var elem = this;
			if (typeof url !== "string") {
				return false;
			}

			function ajaxCallback(responseText) {
				if (typeof callback == "function") {
					REasy(elem).html(responseText);
					callback.apply();
				}
			}


			REasy.ajax({
				url: url,
				type: "get",
				dataType: "html",
				success: ajaxCallback
			})
		},
		resize: function (callback) {
			var elem = this;
			if (typeof callback == "function") {
				window.onresize = function () {
					callback.apply();
				}
			}
		}
	});

	/*================ Traversing ======================*/
	/* == utils == */
	REasy.extend({
		filter: function (expr, elems) {
			// TODO: Not support ":not" selector
			// support in futrue
			/* if (not) {
				expr = ":not(" + expr + ")";
			} */

			return elems.length === 1 ?
				(REasy.find(expr, elems[0]) ? elems[0] : []) :
				REasy.find(expr, elems);
		},

		dir: function (elem, dir, until) {
			var matched = [],
				cur = elem[dir];

			while (cur && cur.nodeType !== 9 &&
				(until === undefined || cur.nodeType !== 1 || cur !== until)) {

				if (cur.nodeType === 1) {
					matched.push(cur);
				}
				cur = cur[dir];
			}
			return matched;
		},

		sibling: function (n, elem) {
			var r = [];

			for (; n; n = n.nextSibling) {
				if (n.nodeType === 1 && n !== elem) {
					r.push(n);
				}
			}

			return r;
		},

		isHidden: function (elem) {
			return REasy.css(elem, "display") === "none" ||
				REasy.css(elem, "visibility") === "hidden" ||
				(elem.offsetHeight == 0 && elem.offsetWidth == 0);
		}
	});

	/* Tree Traversal */
	REasy.each({
		parent: function (elem) {
			var parent = elem.parentNode;
			return parent && parent.nodeType !== 11 ? parent : null;
		},
		parents: function (elem) {
			return REasy.dir(elem, "parentNode");
		},
		next: function (elem) {
			return sibling(elem, "nextSibling");
		},
		prev: function (elem) {
			return sibling(elem, "previousSibling");
		},
		nextAll: function (elem) {
			return REasy.dir(elem, "nextSibling");
		},
		prevAll: function (elem) {
			return REasy.dir(elem, "previousSibling");
		},
		siblings: function (elem) {
			return REasy.sibling((elem.parentNode || {}).firstChild, elem);
		},
		children: function (elem) {
			return REasy.sibling(elem.firstChild);
		},
		contents: function (elem) {
			return REasy.nodeName(elem, "iframe") ?
				elem.contentDocument || elem.contentWindow.document :
				REasy.merge([], elem.childNodes);
		}
	}, function (name, fn) {
		REasy.fn[name] = function () {
			var ret = REasy.map(this, fn);

			return this.pushStack(ret);
		};
	});


	/*==============================================================================
	 * Ajax
	 *============================================================================*/
	var ajaxLocation,
		rnoContent = /^(?:GET|HEAD)$/;

	//创建Ajax对象
	function createRequest() {
		var req = null;

		try {
			req = new XMLHttpRequest();
			if (req.overrideMimeType) {
				req.overrideMimeType("text/xml");
			}
		} catch (trymicrosoft) {
			try {
				req = new ActiveXObject("MSXML2.XMLHTTP");
			} catch (othermicrosoft) {
				try {
					req = new ActiveXObject("Microsoft.XMLHTTP");
				} catch (failed) {
					req = false;
				}
			}
		}
		if (!req) {
			throw new Error("No XHR object available.");
		}
		return req;
	}

	// IE may throw an exception when accessing
	// a field from window.location if document.domain has been set
	try {
		ajaxLocation = window.location.href;
	} catch (e) {
		// Use the href attribute of an A element
		// since IE will modify it given document.location
		ajaxLocation = document.createElement("a");
		ajaxLocation.href = "";
		ajaxLocation = ajaxLocation.href;
	}

	REasy.extend({
		encodeFormData: function (data) {
			var pairs = [],
				name,
				val;

			if (!data) {
				return "";
			}
			for (name in data) {
				if (data.hasOwnProperty(name) && !REasy.isFunction(data[name])) {
					val = data[name].toString();
					name = encodeURIComponent(name.replace("%20", "+"));
					val = encodeURIComponent(val.replace("%20", "+"));
					pairs.push(name + '=' + val);
				}
			}
			return pairs.join('&');
		},

		queryToObj: function (str) {
			var strArr = str.split('&'),
				i = 0,
				len = strArr.length,
				retObj = {},
				strI;

			for (; i < len; i++) {
				strI = strArr[i];
			}
		},

		serializeByClass: function (name, obj) {
			var ret = [],
				retObj = {};

			REasy('.' + name).each(function () {
				var nameVal = this.getAttribute('name');

				if (nameVal) {
					ret.push(encodeURIComponent(nameVal) + "=" +
						encodeURIComponent($.getValue(this)));
					retObj[nameVal] = $.getValue(this);
				}

			});
			if (obj) {
				return retObj;
			}
			return ret.join("&");
		},

		serialize: function (form, byClass, obj) {
			var parts = [],
				field = null,
				i,
				len,
				j,
				optLen,
				option,
				optValue;

			for (i = 0, len = form.elements.length; i < len; i++) {
				field = form.elements[i];

				switch (field.type) {
				case "select-one":
				case "select-multiple":
					if (field.name.length) {
						for (j = 0, optLen = field.options.length; j < optLen; j++) {
							optValue = "";
							if (option.hasAttribute) {
								optValue = (option.hasAttribute("value") ?
									option.value : option.text);
							} else {
								optValue = (option.attributes.value.specified ?
									option.value : option.text);
							}
							parts.push(encodeURIComponent(field.name) + "=" +
								encodeURIComponent(optValue));
						}
					}
					break;

				case undefined:
				case "file":
				case "submit":
				case "reset":
				case "button":
					break;

				case "radio":
				case "checkbox":
					if (!field.checked) {
						break;
					}

				default:
					if (field.name.length) {
						parts.push(encodeURIComponent(field.name) + "=" +
							encodeURIComponent(field.value));
					}
				}
			}

			return byClass ? parts.join("&") + "&" + REasy.serializeByClass(byClass) :
				parts.join("&");
		},

		ajaxSettings: {
			url: ajaxLocation,
			type: "GET",
			global: true,
			processData: true,
			async: true,
			contentType: "application/x-www-form-urlencoded;"
		},
		ajaxErrorFlag: false,
		ajaxStop: function () {
			var req = arguments[0];
			req.abort();
			REasy.ajaxError.apply();
		},
		ajaxError: function (callback) {
			if (typeof callback === "function") {
				REasy.ajaxHandler = callback;
			}
			if (typeof REasy.ajaxHandler == "function" && REasy.ajaxErrorFlag) {
				REasy.ajaxHandler.apply();
				REasy.ajaxErrorFlag = false;
				return;
			}
		},
		ajax: function (options) {
			var s = REasy.extend({}, REasy.ajaxSettings, options),
				req = createRequest();

			//判断是否需要，含有"Content-Type"头，例如“get”方式是没有
			s.hasContent = !rnoContent.test(s.type);

			// Uppercase the type
			s.type = s.type.toUpperCase();
			req.open(s.type, s.url, s.async);

			// Set the correct header, if data is being sent
			if ((s.data && s.hasContent && s.contentType !== false) || options.contentType) {
				req.setRequestHeader("Content-Type", s.contentType);
			}

			req.onreadystatechange = function () {
				if (req.readyState === 4 && req.status === 200 && s.success) {
					if (!s.successed && REasy.type(s.success) === 'function') {
						s.successed = true;
						REasy.ajaxErrorFlag = false;
						s.success.call(req, req.responseText);
					} else {
						REasy.ajaxErrorFlag = true;
					}
				} else {
					REasy.ajaxErrorFlag = true;
				}
			};

			if (REasy.type(s.data) === "object") {
				s.data = $.encodeFormData(s.data);
			}
			req.send(s.data);
			//REasy.ajaxErrorFlag = true;
			if (REasy.ajaxErrorFlag && !s.successed) {
				REasy.ajaxStop.apply(this, [req]);
			}
			s.successed = false;
			return req;
		},

		getJSON: function (url, callback) {
			REasy.get(url, function (req) {
				var dataStr = req,
					jsonObj = {};

				// Delete control characters
				dataStr = dataStr.replace(/[\x00-\x1F\x7F]/g, " ");
				//dataStr = dataStr.replace(/\xEF\xBF\xBF/g, '"');
				//TODO: JSON字串带空格会引发错误，暂时屏蔽
				//dataStr = dataStr.replace(/[^"],"/, '","');
				try {
					jsonObj = $.parseJSON(dataStr);
				} catch (e) {
					REasy.ajaxErrorFlag = true;
				} finally {
					if (typeof callback === "function") {
						if (REasy.isEmptyObject(jsonObj)) {
							REasy.ajaxErrorFlag = true;
							REasy.ajaxStop.apply(this, [req]);
							return;
						}
						callback.call(req, jsonObj);
					}
				}

			});
		}
	});

	REasy.each(["get", "post"], function (i, method) {
		REasy[method] = function (url, data, callback, type) {
			// shift arguments if data argument was omitted
			if (REasy.isFunction(data)) {
				type = type || callback;
				callback = data;
				data = undefined;
			}

			return REasy.ajax({
				url: url,
				type: method,
				dataType: type,
				data: data,
				success: callback
			});
		};
	});


	// Expose REasy to the global object
	window.REasy = window.R = window.$ = window.jQuery = REasy;

})(window);