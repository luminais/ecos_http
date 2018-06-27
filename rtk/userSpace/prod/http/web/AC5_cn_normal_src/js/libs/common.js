/********获取随机数**************/
function getRandom() {
    return "?random=" + Math.random();
}

var DeviceType = {
    "samsung": [_("Samsung"), "#4b7bc2","-88px -308px"],
    "apple": [_("Apple"), "#909eb0"],
    "huawei": [_("Huawei"), "#f05c57"],
    "xiaomi": [_("XiaoMi"), "#f68330"],
    "vivo": [_("vivo"), "#51b3e7"],
    "oppo": [_("OPPO"), "#61d237"],
    "meizu": [_("Meizu"), "#42c4ea"],
    "leshi": [_("LeTV"), "#f60000"],
    "sony": [_("Sony"), "#676767"],
    "acer": [_("Acer"), "#61d237"],
    "nokia": [_("Nokia"), "#239bdd","0 -396px"],
    "asus": [_("Asus"),  "#458aee","0 0"],
    "lenovo": [_("Lenovo"), "#499ee3"],
    "moto": [_("Moto"), "#00c2d8"],
    "amoi": [_("Amoi"), "#1886d4"],
    "zte": [_("ZTE"), "#4b7bc2"],
    "google": [_("Google"), "#245bf1"],
    "hisense": [_("Hisense"), "#00897a"],
    "gionee": [_("Gionee"), "#e40600"],
    "tenda": [_("Tenda"), "#ff6600"],
    "amazon": [_("Amazon"), "#ffb709"],
    "smartisan": [_("Smartisan"), "#e53d3c"],
    "oneplus": [_("OnePlus"), "#e63434"],
    "lg": [_("LG"), "#ef528c"],
    "htc": [_("HTC"), "#83cd29"],
    "tcl": [_("TCL"), "#f05c57"],
    "other": [_("?"), "#92979A"]
};
/**
 * @method [将后台传过来的关于设备厂商类型的数据转化为图片显示]
 * @param  {[type]} type [description]
 * @return {[type]}      [description]
 */
function translateManufacturer(type) {
    var name = "";
    var color = "";
    var str = "";
    var fontStyle;
    /*
     * edit by xc 换成企业图标
     */ 
    // for (var prop in DeviceType) {
    //     if (prop == type) {
    //         name = DeviceType[prop][0];
    //         color = DeviceType[prop][1];
    //         break;
    //     }
    // }

    // var device = DeviceType[type];
    // name = device[0];
    // color = device[1];

    str += "<span class='hostlogo logo-" + type + "'></span>";

    // if (type == "other") {
    //     fontStyle = "color:#fff;position:absolute; font-size:15px; top:12px;width: 40px;text-align: center;left:0;";

    // } else {
    //     fontStyle = "color:#fff;position:absolute; font-size:12px; top:12px;width: 40px;text-align: center;left:0;"
    // }
    // str += "<span style='" + fontStyle + "'>" + name + "</span>";

    return str;
}


/*
    第一位表示工作模式(0表示AP,1表示WISP,2表示APClient)
    第二位表示WAN口类型(1表示static IP,2表示DHCP,3表示PPPOE)
    第三位和第四位表示错误代码编号
*/
var statusMsg = {
    /*AP模式*/

    //STATIC
    "0101": _("WAN port disconnected. Please connect an Ethernet cable with Internet connectivity to the port."),
    "0102": _("Disconnected"),
    "0103": _("Connecting... Detecting the Internet..."),
    "0104": _("Connected... Accessing the Internet..."),
    "0105": _("Internet is inaccessible. Please contact your ISP."),
    "0106": _("Connected. You can access the internet."),

    //DHCP
    "0201": _("WAN port disconnected. Please connect an Ethernet cable with Internet connectivity to the port."),
    "0202": _("Disconnected"),
    "0203": _("Connecting... Detecting the Internet..."),
    "0204": _("Connected... Accessing the Internet..."),
    "0205": _("The router has obtained a valid IP address but cannot access the Internet. Please try the solutions below one by one.") + "<br/>" + "1. <a id='cloneMac' style='text-decoration:underline;' class='text-success' href='javascript:void(0)'>" + _("Clone MAC Address") + "</a>" + _("(MAC cloning will take effect in 30 seconds.)") +
        "<br/>" + _("2. Reconfigure the router on another computer.") + "<br/>" + _("3. Please make sure that you have subscribed to a valid internet service. For details, consult your ISP."),
    "0206": _("Connected. You can access the internet."),
    "0207": _("IP conflict. Please change the LAN IP address."),
    "0208": _("Error: No response from the remote server. Please contact your ISP."),

    //PPPoE
    "0301": _("WAN port disconnected. Please connect an Ethernet cable with Internet connectivity to the port."),
    "0302": _("Disconnected"),
    "0303": _("Checking the user name and password... Please wait 1-5 minutes."),
    "0304": _("Dial-up connection succeeded. Accessing the Internet..."),
    "0305": _("Dial-up connection succeeded but the internet is inaccessible. Please contact your ISP."),
    "0306": _("Connected. You can access the internet."),
    "0307": _("Failed. Please confirm your user name and password and try again."),
    "0308": _("Error: No response from the remote server. Please contact your ISP."),

    /************WISP**************/
    //STATIC 
    "1102": _("No hotspot"),
    "1103": _("Bridging in WISP mode..."),
    "1104": _("Bridged successfully in WISP mode. Accessing the internet..."),
    "1105": _("Internet is inaccessible. Please contact your ISP."),
    "1106": _("Connected. You can access the internet."),
    //无线桥接用户名、密码错误@add by windy
    "1107": _("The WiFi password of the upstream device is incorrect."),
    //end

    //DHCP 
    "1202": _("No hotspot"),
    "1203": _("Bridging in WISP mode..."),
    "1204": _("Bridged successfully in WISP mode. Accessing the internet..."),
    "1205": _("An IP address is obtained but the internet is inaccessible. Please contact your ISP."),
    "1206": _("Connected. You can access the internet."),
    "1207": _("IP conflict. Please change the LAN IP address."),
    "1208": _("Error: No response from the remote server. Please contact your ISP."),
    //无线桥接用户名、密码错误@add by windy
    "1209": _("The WiFi password of the upstream device is incorrect."),
    //end

    //PPPoE 
    "1302": _("No hotspot"),
    "1303": _("Checking the user name and password... Please wait 1-5 minutes."),
    "1304": _("Dial-up connection succeeded. Accessing the Internet..."),
    "1305": _("Dial-up connection succeeded but the internet is inaccessible. Please contact your ISP."),
    "1306": _("Connected. You can access the internet."),
    "1307": _("Failed. Please confirm your user name and password and try again."),
    "1308": _("Error: No response from the remote server. Please contact your ISP."),
    //add by windy
    "1309": _("The WiFi password of the upstream device is incorrect."),

    //APClinet
    "2102": _("No signal"),
    "2103": _("Bridging in Universal Repeater mode..."),
    "2104": _("Bridged in Universal Repeater mode"),
    "2202": _("No signal"),
    "2203": _("Bridging in Universal Repeater mode..."),
    "2204": _("Bridged in Universal Repeater mode"),
    "2302": _("No signal"),
    "2303": _("Bridging in Universal Repeater mode..."),
    "2304": _("Bridged in Universal Repeater mode"),
    "2107": _("The WiFi password of the upstream device is incorrect."),
    "2209": _("The WiFi password of the upstream device is incorrect."),
    "2309": _("The WiFi password of the upstream device is incorrect."),

    //AP
    "4102": _("Connection failure"),
    "4104": _("Connection success"),
    "4202": _("Connection failure"),
    "4204": _("Connection success"),
    "4302": _("Connection failure"),
    "4304": _("Connection success")
};

/**
 *  @method [updateWanConnectStatus] [用于处理wan口的连接状态,如果IP冲突，则提示用户并重启。否则在页面上显示wan口状态]
 *  @param {Object} [obj] [wan口状态代码,表示internetStauts模块]
 *  @param {String} [elemId] [用于显示wan口状态信息的元素id]
 *  @return {无}
 */

var lanWanIPConflict = false;

function updateInternetConnectStatus(obj, elemID) {

    if (!lanWanIPConflict) {
        if (obj.lanWanIPConflict == "true") {
            lanWanIPConflict = true;
            alert(_('IP conflict! The login IP address will be changed into %s automatically.Please log in again using %s.', [obj.newLanIP, obj.newLanIP]));
            dynamicProgressLogic.init("reboot", "", 450, obj.newLanIP);

        } else {
            showWanInternetStatus(obj.wanConnectStatus, elemID);

        }
    }

    /**
     * @method [showWanInternetStatus] [用于处理wan口连接状态的函数]
     * @param  {string} statusCode [约定了用于表示wan连接状态的一串数据]
     * @param  {string} elemID     [显示连接状态信息的元素的ID]
     * @return {booleans}             [执行完成后返回true]
     */
    function showWanInternetStatus(statusCode, elemID) {
        var str = "",
            connectStatus, connectMsg;

        if (!statusCode) {
            return false;
        }
        connectStatus = statusCode.slice(1, 2);
        connectMsg = statusCode.slice(3, 7);

        if (connectStatus == "3") {
            str = "text-success";
        } else if (connectStatus == "2") {
            str = "text-primary";
        } else {
            str = "text-danger";
        }

        $("#" + elemID).html(statusMsg[connectMsg]).attr("class", str);
        return true;
    }
}



/**
 * @method [checkIpInSameSegment] [用于判断两个IP地址是否同网段]
 * @param  {string} ip_lan   [lan口IP地址]
 * @param  {string} mask_lan [lan口子网掩码]
 * @param  {string} ip_wan   [wan口IP地址]
 * @param  {string} mask_wan [wan口子网掩码]
 * @return {booleans}        [若在同一网段，则返回true, 不在同一网段，返回false]
 */
function checkIpInSameSegment(ip_lan, mask_lan, ip_wan, mask_wan) {
    if (ip_lan === '' || ip_wan === '') {
        return false;
    }
    var ip1Arr = ip_lan.split("."),
        ip2Arr = ip_wan.split("."),
        maskArr1 = mask_lan.split("."),
        maskArr2 = mask_wan.split("."),
        maskArr = maskArr1,
        i;
    for (i = 0; i < 4; i++) {
        if (maskArr1[i] != maskArr2[i]) {
            if ((maskArr1[i] & maskArr2[i]) == maskArr1[i]) {
                maskArr = maskArr1;
            } else {
                maskArr = maskArr2;
            }
            break;
        }
    }
    for (i = 0; i < 4; i++) {
        if ((ip1Arr[i] & maskArr[i]) != (ip2Arr[i] & maskArr[i])) {
            return false;
        }
    }
    return true;
}

/**
 * @method [检查IP地址是否为网段或广播IP合法性]
 * @param  {string} ip   [IP地址]
 * @param  {string} mask [子网掩码]
 * @param  {string} str  [提示信息，用于表示ip是什么地址]
 * @return {string}      [若检查有错，则返回报错提示语。否则，返回为空]
 */
function checkIsVoildIpMask(ip, mask, str) {
    var ipArry,
        maskArry,
        len,
        maskArry2 = [],
        netIndex = 0,
        netIndex1 = 0,
        broadIndex = 0,
        i = 0;
    str = str || _("IP Address");
    ipArry = ip.split("."),
    maskArry = mask.split("."),
    len = ipArry.length;

    for (i = 0; i < len; i++) {
        maskArry2[i] = 255 - Number(maskArry[i]);
    }

    for (var k = 0; k < 4; k++) { // ip & mask
        if ((ipArry[k] & maskArry[k]) == 0) {
            netIndex1 += 0;
        } else {
            netIndex1 += 1;
        }
    }
    for (var k = 0; k < 4; k++) { // ip & 255 - mask
        if ((ipArry[k] & maskArry2[k]) == 0) {
            netIndex += 0;
        } else {
            netIndex += 1;
        }
    }

    if (netIndex == 0 || netIndex1 == 0) {
        return _("%s cannot be a network segment.", [str]);
    }

    for (var j = 0; j < 4; j++) {
        if ((ipArry[j] | maskArry[j]) == 255) {
            broadIndex += 0;
        } else {
            broadIndex += 1;
        }
    }

    if (broadIndex == 0) {
        return _("%s cannot be a broadcast address.", [str]);
    }

    return;
}


/*********对象转换成字符串****************/
function objToString(obj) {
    var str = "",
        prop;
    for (prop in obj) {
        str += prop + "=" + encodeURIComponent(obj[prop]) + "&";
    }
    str = str.replace(/[&]$/, "");
    return str;
}

/**
 * @method [为表单元素赋值或为元素显示文本]
 * @param  {[type]}   obj      [obj里面放置的是值]
 * @param  {Function} callback [执行完成赋值后执行的回调函数]
 * @return {无}
 */
function inputValue(obj, callback) {
    var prop,
        tagName;

    for (prop in obj) {
        if (prop && $("#" + prop).length > 0) {
            tagName = document.getElementById(prop).tagName.toLowerCase();
            switch (tagName) {
            case "input":
            case "select":
                if (document.getElementById(prop).type == "checkbox") {
                    if (obj[prop] == "true") {
                        document.getElementById(prop).checked = true;
                    } else {
                        document.getElementById(prop).checked = false;
                    }
                } else {
                    $("#" + prop).val(obj[prop]);
                }
                break;
            default:
                if ($("#" + prop).hasClass("textboxs") || $("#" + prop).hasClass("input-append")) {
                    $("#" + prop)[0].val(obj[prop]);
                } else {
                    $("#" + prop).text(obj[prop]);
                }
                break;
            }
        } else if (prop && $("[name='" + prop + "']").length > 1) {
            tagName = document.getElementsByName(prop)[0].tagName.toLowerCase();
            if (tagName === "input") {
                if ($("[name='" + prop + "'][value='" + obj[prop] + "']").length > 0) {
                    $("[name='" + prop + "'][value='" + obj[prop] + "']")[0].checked = true;
                }
            }
        }
    };
    if (typeof callback == "function") {
        callback.apply();
    }
}

//处理时间变成 天 时分秒
function formatSeconds(value) {
    var theTime = parseInt(value); // 秒 
    var theTime1 = 0; // 分 
    var theTime2 = 0; // 小时
    var theTime3 = 0; // 天
    // alert(theTime); 
    if (theTime > 60) {
        theTime1 = parseInt(theTime / 60);
        theTime = parseInt(theTime % 60);
        // alert(theTime1+"-"+theTime); 
        if (theTime1 > 60) {
            theTime2 = parseInt(theTime1 / 60);
            theTime1 = parseInt(theTime1 % 60);
            if (theTime2 > 24) {
                theTime3 = parseInt(theTime2 / 24);
                theTime2 = parseInt(theTime2 % 24);
            }
        }
    }
    var result = "" + parseInt(theTime) + _("s");
    if (theTime1 > 0) {
        result = "" + parseInt(theTime1) + _("m") + " " + result;
    }
    if (theTime2 > 0) {
        result = "" + parseInt(theTime2) + _("h") + " " + result;
    }
    if (theTime3 > 0) {
        result = "" + parseInt(theTime3) + _("d") + " " + result;
    }
    return result;
}

//base64加密
function Encode() {
    var base64EncodeChars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';

    function utf16to8(str) {
        var out,
            i,
            len,
            c;

        out = "";
        len = str.length;
        for (i = 0; i < len; i++) {
            c = str.charCodeAt(i);
            if ((c >= 0x0001) && (c <= 0x007F)) {
                out += str.charAt(i);
            } else if (c > 0x07FF) {
                out += String.fromCharCode(0xE0 | ((c >> 12) & 0x0F));
                out += String.fromCharCode(0x80 | ((c >> 6) & 0x3F));
                out += String.fromCharCode(0x80 | ((c >> 0) & 0x3F));
            } else {
                out += String.fromCharCode(0xC0 | ((c >> 6) & 0x1F));
                out += String.fromCharCode(0x80 | ((c >> 0) & 0x3F));
            }
        }
        return out;
    }

    function base64encode(str) {
        var out,
            i,
            len;
        var c1,
            c2,
            c3;

        len = str.length;
        i = 0;
        out = "";
        while (i < len) {
            c1 = str.charCodeAt(i++) & 0xff;
            if (i == len) {
                out += base64EncodeChars.charAt(c1 >> 2);
                out += base64EncodeChars.charAt((c1 & 0x3) << 4);
                out += "==";
                break;
            }
            c2 = str.charCodeAt(i++);
            if (i == len) {
                out += base64EncodeChars.charAt(c1 >> 2);
                out += base64EncodeChars.charAt(((c1 & 0x3) << 4) | ((c2 & 0xF0) >> 4));
                out += base64EncodeChars.charAt((c2 & 0xF) << 2);
                out += '=';
                break;
            }
            c3 = str.charCodeAt(i++);
            out += base64EncodeChars.charAt(c1 >> 2);
            out += base64EncodeChars.charAt(((c1 & 0x3) << 4) | ((c2 & 0xF0) >> 4));
            out += base64EncodeChars.charAt(((c2 & 0xF) << 2) | ((c3 & 0xC0) >> 6));
            out += base64EncodeChars.charAt(c3 & 0x3F);
        }
        return out;
    }
    return function (s) {
        return base64encode(utf16to8(s));
    }
}


/**
 * [GetSetData description]
 * @type {Object}
 */

if (window.JSON) {
    $.parseJSON = JSON.parse;
}

//检查是否超时
function checkIsTimeOut(str) {
    if (str.indexOf("<!DOCTYPE") != -1) {
        return true;
    }
    return false;
}

function addOverLay(time) {
    time = time || 0;
    var str = "<div class='save-overlay'></div>";
    $("body").append(str);
    setTimeout(function () {
        $(".save-overlay").remove();
    }, time)
}

function isEmptyObject(obj) {
    var name;
    for (name in obj) {
        return false;
    }
    return true;
};
//page 逻辑对象
//function PageLogic(url, setUrl) {
function PageLogic(urlObj) {
    var moduleArry;
    var _this = this;
    //初始化入口
    this.init = function () {
        moduleArry = this.modules;
        this.pageRunning = true;
        this.initEvent();
        for (var i = 0; i < moduleArry.length; i++) {
            if (typeof moduleArry[i].init == "function") {
                moduleArry[i].init();
            }
        }
        this.addValidate(); //添加数据验证
        this.getValue(urlObj.getUrl, urlObj.modules); //获取数据
    };

    //page 事件绑定
    this.initEvent = function () {

    };

    //取消时重新初始化
    this.reCancel = function () {
        _this.initValue(_this.data);
    }
    this.updateTimer = null;
    this.update = function (modules, time, callback) {
        time = time || 0;
        if (!_this.pageRunning || time == 0) {
            clearTimeout(this.updateTimer);
            return;
        };

        clearTimeout(this.updateTimer);
        if (time != 0) {
            this.updateTimer = setTimeout(function () {
                _this.update(modules, time, callback);
            }, time);
        }

        if (typeof urlObj.getUrl == "string") {
            $.get(urlObj.getUrl + "?" + Math.random() + "&modules=" + encodeURIComponent(modules), function (obj) {

                if (checkIsTimeOut(obj)) {
                    top.location.reload(true);
                }

                try {
                    obj = $.parseJSON(obj);
                } catch (e) {
                    obj = {};
                }

                if (isEmptyObject(obj)) {
                    top.location.reload(true);
                }

                if (!_this.pageRunning) {
                    return;
                }

                if (typeof callback == "function") {
                    callback.apply(this, [obj]);
                }

            });
        }
    }

    //page 获取数据
    this.getValue = function (getUrl, modules) {
        var obj = {};
        if (!this.pageRunning) {
            return;
        }

        if (typeof getUrl == "string") {
            var tmpObj = {},
                data = "";

            //存在参数时；
            if (modules) {
                data = $.encodeFormData({
                    "random": Math.random(),
                    "modules": modules
                });

            } else {
                data = Math.random();
            }

            $.get(getUrl + "?" + data, _this.initValue);
        }
    }

    //初始化数据
    this.initValue = function (obj) {
        var moduleName, i;

        //在页面获取数据，而非点击cancel时
        if (typeof obj == "string") {

            //检查返回的字符串里面是否包含!DOCUTYEP;
            if (checkIsTimeOut(obj)) {
                top.location.reload(true);
            }

            try {
                obj = $.parseJSON(obj);
            } catch (e) {
                obj = {};
            }
        }

        if (isEmptyObject(obj)) {
            top.location.reload(true);
        }

        _this.data = obj;

        if (!_this.pageRunning) {
            return;
        }

        for (i = 0; i < moduleArry.length; i++) {
            moduleName = moduleArry[i].moduleName;
            if (typeof moduleArry[i].initValue == "function") {
                moduleArry[i].initValue(_this.data[moduleName]);
            }
        }

        if (mainLogic && typeof mainLogic == "object") {
            mainLogic.initModuleHeight();
        }
    };

    //获取提交数据
    this.getSubmitData = function () {
        var data = "";
        var moduleData = "";
        for (var i = 0; i < moduleArry.length; i++) {
            //判断提交函数是否存在;
            if (typeof moduleArry[i].getSubmitData == "function") {
                //如果返回值为"",则说明该模块值不需要保存;
                moduleData = moduleArry[i].getSubmitData();
                if (moduleData != "") {
                    data += moduleData + "&";
                }
            }
        }

        data = data.replace(/[&]$/, "");
        return data;
    };

    //数据验证初始化
    this.addValidate = function () {
        this.validate = $.validate({
            custom: function () {
                var msg;
                //执行模块内部的数据验证
                for (var i = 0; i < moduleArry.length; i++) {
                    if (typeof moduleArry[i].checkData == "function") {
                        msg = moduleArry[i].checkData.apply();
                    }
                    if (msg) {
                        return msg;
                    }
                }
            },

            success: function () {

                _this.preSubmit();
            },

            error: function (msg) {
                if (msg) {
                    _this.showPageMsg(msg);
                }
            }
        });
    };

    //提交数据
    this.preSubmit = function () {
        if (typeof _this.beforeSubmit == "function") {
            if (!_this.beforeSubmit()) {
                return;
            }
        }
        var data = _this.getSubmitData();

        addOverLay(1000);

        mainLogic.showModuleMsg(_("Saving..."));
        $.ajax({
            url: urlObj.setUrl,
            type: "POST",
            data: data,
            success: _this.successCallback,
            error: _this.ajaxErrMsg
        });

    };

    //提交成功
    this.successCallback = function (msg) {
        if (checkIsTimeOut(msg)) {
            top.location.reload(true);
            return;
        }

        var num = $.parseJSON(msg).errCode || "-1";
        var ip = _this.rebootIP || "";

        if (+num == 0) { //success
            mainLogic.showModuleMsg(_("Saved successfully."));
            _this.getValue(urlObj.getUrl, urlObj.modules);
        } else if (num == "2") {
            mainLogic.showModuleMsg(_("Failed to change the password. Old Password is incorrect."));
        } else if (num == "100") {
            mainLogic.hideModuleMsg();
            dynamicProgressLogic.init("reboot", "", 450, ip);
        } else if (num == "101") {
            window.location = "./login.html";
        }
        $("#submit").removeAttr("disabled");
    }

    //ajax提交数据失败
    this.ajaxErrMsg = function () {
        _this.showPageMsg(_("Failed to upload the data."));
    }

    //显示用户提示信息
    this.showPageMsg = function (msg) {
        mainLogic.showModuleMsg(msg);
        return;
    }
}

$.getData = function (getUrl, modules, callback) {
    var obj = {};

    if (typeof getUrl == "string") {
        var tmpObj = {},
            data = "";

        //存在参数时；
        if (modules) {
            data = $.encodeFormData({
                "random": Math.random(),
                "modules": modules
            });

        } else {
            data = Math.random();
        }

        $.get(getUrl + "?" + data, function (dt) {

            var obj = {};
            try {
                obj = $.parseJSON(dt);
            } catch (e) {
                obj = {};
            }
            if (isEmptyObject(obj)) {
                top.location.reload(true);
            }

            if (typeof callback == "function") {
                callback.apply(this, [obj]);
            }
        });
    }
}


function isAllNumber(arry) {
    var i = 0,
        len = arry.length || 0;
    for (i = 0; i < len; i++) {
        if (isNaN(Number(arry[i]))) {
            return false;
        }
    }
    return true;
}

function isAllIp(arry) {
    var i = 0,
        len = arry.length || 0;
    for (i = 0; i < len; i++) {
        if (!(/^([1-9]|[1-9]\d|1\d\d|2[0-1]\d|22[0-3])\.(([0-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.){2}([0-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])$/).test(arry[i])) {
            return false;
        }
    }
    return true;
}

function isAllMac(arry) {
    var i = 0,
        len = arry.length || 0;
    for (i = 0; i < len; i++) {
        if (!(/^([0-9a-fA-F]{2}:){5}[0-9a-fA-F]{2}$/).test(arry[i])) {
            return false;
        }
    }
    return true;
}


//排序
function reCreateObj(obj, prop, sortTag) { //数据  排序对象 排序方式
    var newObj = [], //存放最后的排序结果
        len = obj.length || 0,
        i = 0,
        j = 0,
        newArry = [], //存放所有 prop 元素的数组
        arry_prop = [],
        temporaryObj = [], //临时存放数据
        numberFlag = false,
        ipFlag = false,
        macFlag = false;
    for (var k = 0; k < len; k++) {
        temporaryObj[k] = obj[k];
    }

    for (i = 0; i < len; i++) {
        arry_prop[i] = temporaryObj[i][prop]; //将需要排序的元素放在一个数组里；
    }
    numberFlag = isAllNumber(arry_prop);
    ipFlag = isAllIp(arry_prop);
    macFlag = isAllMac(arry_prop);

    function sortNumber(a, b) {
        var c = parseInt(a.replace(/[.:]/g, ""), 16);
        var d = parseInt(b.replace(/[.:]/g, ""), 16);
        //if (d > c)
        return c - d
    };

    if (numberFlag || ipFlag || macFlag) { //如果数组全为数字
        newArry = arry_prop.sort(sortNumber);
    } else {
        newArry = arry_prop.sort();
    }

    for (i = 0; i < len; i++) {
        for (j = 0; j < temporaryObj.length; j++) {
            if (newArry[i] == temporaryObj[j][prop]) { // 排序好的元素中寻找原obj元素
                newObj.push(temporaryObj[j]); //重新排序后的obj
                temporaryObj.splice(j, 1); //  去掉已经找到的；
                break;
            }
        }
    }
    if (sortTag == "up") {
        return newObj;
    } else {
        return newObj.reverse(); ///
    }
}

$.include({
    removeValidateTipError: function (valid) {
        return this.each(function () {
            var $tipElem = $("#" + this.validateTipId);

            if (!$tipElem) {
                return;
            }
            $("#" + this.validateTipId).parent().removeClass("has-error").removeClass("has-feedback");
            $("#" + this.validateTipId).remove();
            this.validateTipId = '';
        });
    }
});

function Dialog() {
    var _this = this;

    this.obj = {
        fistInit: true,
        data: {}
    };

    this.open = function (obj) {
        this.obj.data = obj;

        if (!$('#progress-overlay').hasClass('in')) {
            $('#progress-overlay').addClass('in');
        }

        $("#" + this.obj.data.Id).removeClass("none");
        $("#" + this.obj.data.Id).css("height", this.obj.data.height)

        if (this.obj.fistInit) {
            $(".dialog").delegate(".dialog-close, .cancel", "click", function () {
                _this.close();
            });

            this.obj.fistInit = false;
        }
        if (typeof this.obj.data.callback == "function") {
            this.obj.data.callback.apply();
        }
    };

    this.close = function () {
        $("#" + this.obj.data.Id).addClass("none");
        $('#progress-overlay').removeClass('in');
    }

}


/**
 * @method 判断一个字符串中的字节个数（无论是什么字符）
 * @param  {String} str [待判断字节数的字符串]
 * @return {Number}     [字节长度]
 */
function getStrByteNum(str) {
    var totalLength = 0,
        charCode;

    for (var i = str.length - 1; i >= 0; i--) {
        charCode = str.charCodeAt(i);
        if (charCode <= 0x007f) {
            totalLength++;
        } else if ((charCode >= 0x0080) && (charCode <= 0x07ff)) {
            totalLength += 2;
        } else if ((charCode >= 0x0800) && (charCode <= 0xffff)) {
            totalLength += 3;
        } else {
            totalLength += 4;
        }
    }
    return totalLength;
}

/**
 * 参数解析
 * title：    string 消息框标题
 * content：  string/$对象，消息体内容(html代码或者iframe框)
 * 
 * -----------以下为非必填项
 * 
 * height：   num 高度
 * width：    num 宽
 * autoClose：bool 是否自动关闭
 * timeout：  bool autoClose为true时多少秒后自动关闭
 * buttons：  array [{}] 操作按钮
 * -----text：按钮文本值
 * ----theme：按钮主题
 * -autoHide: 点击按钮后是否关闭弹出窗，true为关闭，false为不关闭，默认值：true
 * -callback：点击按钮的回调，参数event会传入
 */
(function($, window, undefined){
    var _DEFAULT ={
        title:"消息提醒",
        content:"消息体",
        isIframe: false,
        height:0,
        width:0,
        buttons:[],//[{text:'确定',theme:'ok',autoHide:false,callback:function(e){}}]
        autoClose:false,
        timeout:60
    }

    function ModalDialog(opt){
        this.opt = $.extend({},_DEFAULT, opt);
        var html = '<div class="md-modal-wrap">' +
                        '<div class="md-modal">' +
                            '<div class="md-content">' +
                                '<h3 class="md-header"></h3>' +
                                '<button class="md-close">×</button>' +
                                '<div class="md-con-body"></div>' +
                                '<div class="md-toolbar"></div>'+
                            '</div>' +
                        '</div>' +
                    '</div>';
        this.$mask = $(html);       
        this.$overlay = $('<div class="md-overlay"></div>');
        this.$title = this.$mask.find(".md-header");
        this.$close = this.$mask.find('.md-close');
        this.$content = this.$mask.find(".md-con-body");
        this.$footer = this.$mask.find(".md-toolbar");
        this.namespace = "Modal_" + new Date().getTime();

        //added by dapei, 点击蒙版层可关闭弹框
        this.$overlay_close = this.$mask;
        this.$overlay_close_ie = this.$overlay;

        this.TIMEOUT = null;
        $("body").append(this.$overlay).append(this.$mask);
        this.init();
        this.bindEvets();
    }

    ModalDialog.prototype = {
        init:function(){
            this.$title.text(this.opt.title);
            var _this = this;

            this.$content.html();
            if(this.opt.isIframe){
                
                this.iframeName = "Modal_Iframe";
                this.iframe = '<iframe src="'+ this.opt.content +'" name="'+ this.iframeName +'" />';
                this.$content.html(_this.iframe);
            }
            else{
                $(this.opt.content).show();
                this.$content.append(this.opt.content);
            }

            this.reSize();
            
            this.opt.width > 0 && this.$content.css("width", this.opt.width + "px");
            this.opt.height > 0 && this.$content.css("height", this.opt.height + "px");

            this.setButtons(this.opt.buttons);
            this.show();
        },
        getSize:function(){
            this.opt.MAX_Height = (window.innerHeight || (document.documentElement && document.documentElement.clientHeight) || document.body.clientHeight) * 0.8 - 40;
            this.opt.MAX_Width = (window.innerWidth || (document.documentElement && document.documentElement.clientWidth) || document.body.clientWidth) * 0.8;
            this.opt.buttons.length > 0 && (this.opt.WIN_Height -=40);
        },
        setButtons:function(btns){
            if(!btns || Object.prototype.toString.call(btns) != "[object Array]" || btns.length < 1) {
                this.$footer.remove();
                return;
            }
            var $footer  = this.$footer.show(), 
                _this = this;
            $footer.html("");
            for(var i=0,l = btns.length;i<l;i++){
                (function(i){
                    var btn = btns[i], $btn = $('<button class="md-btn '+ btn.theme +'">'+ btn.text +'</button>');
                    $btn.on("click",function(e){
                        isFunction(btn.callback) && btn.callback(e);
                        if(btn.autoHide === false){
                            return false;
                        }
                    });
                    $footer.append($btn);
                }(i));
            } 
            _this.opt.buttons = btns;
        },
        show:function(callback){
            this.$overlay.addClass('md-show');
            this.$mask.addClass('md-show');

            isFunction(callback) && callback();
        },
        hide:function(callback){
            this.$overlay.removeClass('md-show');
            this.$mask.removeClass('md-show');

            isFunction(callback) && callback();
        },
        toggle:function(callback){
            this.$overlay.toggleClass('md-show');
            this.$mask.toggleClass('md-show');
                    
            isFunction(callback) && callback();
        },
        reSize:function(opt){
            this.getSize();

            this.$content.css({
                "max-height": this.opt.MAX_Height + "px",
                "max-width": this.opt.MAX_Width + "px"
            });
        },
        setTitle:function(text){
            if(text){
                this.$title.text(text);
                this.opt.title = text;
            }
        },
        refresh:function(){

        },
        bindEvets:function(){
            var _this = this;
            // $(window).off('resize.'+_this.namespace).on('resize.'+_this.namespace, function(event) {
            $(window).on('resize', function(event) {
                if(_this.TIMEOUT){
                    window.clearTimeout(_this.TIMEOUT);
                    _this.TIMEOUT = null;
                }
                _this.TIMEOUT = window.setTimeout(function(){
                    _this.reSize.call(_this);
                },300);
            });
            
            // _this.$close.unbind('click.' + _this.namespace).bind('click.'+_this.namespace,function(e){
            _this.$close.on('click',function(e){
                _this.hide();
                return false;
            });

            // _this.$footer.off("click." + _this.namespace).on("click." + _this.namespace, ".md-btn", function(e){
            //     _this.hide();
            //     return false;
            // });
            _this.$footer.on("click", function(e){
                if($(e.target).hasClass("md-btn")){
                    _this.hide();
                }
                return false;
            });

            //点击蒙版时，可以关闭弹框
            _this.$overlay_close.on("click", function(e) {
                if(_this.$content.hasClass("no_close_flag")) {
                    return;
                }

                var event = e || window.event;
                var target = event.target || event.srcElement || null;
                if (!target) {
                  return;
                }

                if(target.childNodes.length !== 0 && target.childNodes[0].className == "md-modal") {
                    _this.hide();
                    return false;
                }
            });

            _this.$overlay_close_ie.on("click", function(e) {
                if(_this.$content.hasClass("no_close_flag")) {
                    return;
                }

                var event = e || window.event;
                var target = event.target || event.srcElement || null;
                if (!target) {
                  return;
                }

                _this.hide();
                return false;
            });

        }
    }
    
    /**
     * 是否是函数
     * @param  {[type]}  func 函数
     * @return {Boolean}      [true：是]
     */
    function isFunction(func){
        if(func && typeof func === "function"){
            return true;
        }
        return false;
    }

    
    $.fn.modalDialog = function(opt){
        this.ModalDialog = new ModalDialog(opt);
        return this;
    } 

    $.modalDialog = function(opt){
        return new ModalDialog(opt);
    }
}(jQuery, window));