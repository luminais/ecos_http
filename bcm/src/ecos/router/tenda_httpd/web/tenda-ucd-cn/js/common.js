//验证两个IP是否在同一网段
function isSameNet(ip_lan, ip_wan, mask_lan, mask_wan) {
	var ip1Arr = ip_lan.split("."),
		ip2Arr = ip_wan.split("."),
		maskArr1 = mask_lan.split("."),
		maskArr2 = mask_wan.split("."),
		i;

	for (i = 0; i < 4; i++) {
		if ((ip1Arr[i] & maskArr1[i]) != (ip2Arr[i] & maskArr2[i])) {
			return false;
		}
	}
	return true;
}

function ipMaskMergeOk (ip, mask) {
	var ipArr = ip.split('.'),
		maskArr = mask.split('.'),
		mergeRslt,
		i,
		msg = "无效的 IP 地址和子网掩码合并，请修改 IP 地址或子网掩码.";
	for(i = 0; i < 4; i++) {
		ipArr[i] = parseInt(ipArr[i], 10);
		maskArr[i] = parseInt(maskArr[i], 10);
	}
	mergeRslt = (ipArr[0] | maskArr[0]) + '.' + (ipArr[1] | maskArr[1]) + '.' + (ipArr[2] | maskArr[2]) + '.' + (ipArr[3] | maskArr[3]);
	if(mergeRslt == mask || mergeRslt == '255.255.255.255') {
		alert(msg);
		return false;
	}
	return true;
}

function getValueByName(name) {
	var elems = document.getElementsByName(name),
		len = elems.length,
		val,
		i;

	if (len === 1) {
		val = elems[0].value;
	} else {
		for (i = 0; i < len; i++) {
			if (elems[i].checked) {
				val = elems[i].value;
				return val;
			}
		}
	}

	return val;
}

function setValueByName(name, val) {
	var elems = document.getElementsByName(name),
		len = elems.length,
		i;

	if (len === 1 && (elems[0].type !== 'radio' || elems[0].type !== 'checkbox')) {
		elems[0].value = val;
		return elems[0];
	} else {
		for (i = 0; i < len; i++) {
			if (elems[i].value == val) {
				elems[i].checked = true;
				return elems[i];
			}
		}
	}

	return null;
}

function decodeSSID(SSID) {
	var e = document.createElement("div"),
		deSSID = '';
		
	if (!SSID) {
		return deSSID;
	}
	e.innerHTML = SSID.replace(/\x20/g, "\xA0");
	if (e.innerText) {
		deSSID = e.innerText;
	} else if (e.textContent) {
		deSSID = e.textContent;
	}
	e = null;
	return deSSID.replace(/\xA0/g, "\x20");
}

var progressStrip = (function(window) {
	function ProgressStrip(url, reboot_time, reboot_msg,  up_time, up_msg) {
		var my_obj = this;
		this.url = url || '';
		this.reboot_timer = null;
		this.reboot_time = reboot_time + 60 || 360;
		this.reboot_msg = reboot_msg || "Please wait: Device rebooting...";
		
		this.up_timer = null;
		this.up_msg = up_msg ||
				"The system is flashing now. DO NOT POWER OFF THE DEVICE!";
		this.up_pc = 0;
		this.reboot_pc = 0;
		this.up_time = up_time + 60 || "0";
		
			
		this._loadding = function () {
			my_obj.reboot_pc += 1;
			
			if (my_obj.reboot_pc == 30) {
				my_obj.reboot_time -= 60; 
			}
			
			if (my_obj.reboot_pc == 70) {
				my_obj.reboot_time -= 60;
			}
			
			if (my_obj.reboot_pc > 100) {
				if (my_obj.url === "") {
					location.pathname = '';
				} else {
					if (my_obj.url.indexOf("http://") !== -1 ||
							my_obj.url.indexOf("https://") !== -1) {
						window.location = my_obj.url;
					} else {
						window.location = "http://" + my_obj.url;
					}

				}
				//document.getElementById('gbx_overlay').style.display = 'none';
				//document.getElementById('loading_div').style.display = 'none';
				
				clearTimeout(my_obj.reboot_timer); 
				return 0;
			}
			
			document.getElementById('load_pc').style.width = my_obj.reboot_pc + "%";
			document.getElementById('load_text').innerHTML = my_obj.reboot_pc + "%";
			
			my_obj.reboot_timer = setTimeout(my_obj._loadding,  my_obj.reboot_time);
		};
		
		this._upgrading = function () {
			my_obj.up_pc += 1;
			
			if (my_obj.up_pc == 30) {
				my_obj.up_time -= 60;
			}
			
			if (my_obj.up_pc == 70) {
				my_obj.up_time -= 60;
			}
			
			if (my_obj.up_pc > 100) {	
				clearTimeout(my_obj.up_timer);
				my_obj._loadding();		
				return 0;
			}
			
			document.getElementById('upgrade_pc').style.width = my_obj.up_pc + "%";
			document.getElementById('upgrade_text').innerHTML = my_obj.up_pc + "%";
			
			my_obj.up_timer = setTimeout(my_obj._upgrading, my_obj.up_time);
		};
	}

	ProgressStrip.prototype = {
		constructor : ProgressStrip,
		_init: function () {
			var gbxOverlay = document.getElementById("gbx_overlay"),
				loadContentElem = document.getElementById("loading_div"),
				loadingElem = document.getElementById("loadding"),
				left,
				top;
			
				function pageWidth() {
					var de = document.documentElement;
					
					return window.innerWidth ||
							(de && de.clientWidth) || document.body.clientWidth;
				}
				
				function pageHeight() {
					var de = document.documentElement;
					
					return window.innerHeight ||
							(de && de.clientHeight) || document.body.clientHeight;
				}
			
			if (!gbxOverlay) {
				gbxOverlay = document.createElement('div');
				gbxOverlay.id = 'gbx_overlay';
				$('body').append(gbxOverlay);
			}
			if (!loadContentElem) {
				loadContentElem = document.createElement('div');
				loadContentElem.id = 'loading_div';
				$('body').append(loadContentElem);
			}
			if (loadContentElem && loadingElem) {
				
				gbxOverlay.style.display = "block";
				loadContentElem.style.display = "block";
			} else {
			
				loadContentElem.innerHTML = '<div id="up_contain">'+
						'<span id="upgrading">' +
						'<span id="upgrade_pc">' + '</span></span><p>' + this.up_msg +
						'<span id="upgrade_text"></span></p></div>' +
						'<span id="loadding"><span id="load_pc"></span>' +
						'</span>' +
						'<p>' + this.reboot_msg + '<span id="load_text"></span></p>';
				loadContentElem = document.getElementById("loading_div");
				loadingElem = document.getElementById("loadding");
				gbxOverlay.style.display = "block";
				loadContentElem.style.display = "block";	
			}
			
			left = (pageWidth() - loadContentElem.offsetWidth) / 2;
			top = (pageHeight() - loadContentElem.offsetWidth) / 2;
			
			loadContentElem.style.left = left + 'px';
			loadContentElem.style.top = top + 'px';
			
			if (this.up_time !== "0") {
				loadContentElem.style.height = 200 + 'px';
				loadContentElem.style.width = 320 + 'px';
				
				loadingElem.style.marginTop = 15 + 'px';
				loadingElem.style.width = 260 + 'px';
				this._upgrading();
			} else {
				document.getElementById('up_contain').style.display = 'none';
				this._loadding();
			}
			return this;
		},
		
		setRebootTime: function (time) {
			this.reboot_time = time;
		},
		
		setUpTime: function (time) {
			this.up_time = time;
		}
	};

	return function (url, reboot_time, reboot_msg,  up_time, up_msg) {
		var obj = new ProgressStrip(url, reboot_time, reboot_msg,  up_time, up_msg);
					
		return obj._init();
	};
}(window));



