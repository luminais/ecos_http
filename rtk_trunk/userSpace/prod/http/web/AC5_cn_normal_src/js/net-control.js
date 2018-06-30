define(function(require, exports, module) {
    var pageModule = new PageLogic({
        getUrl: "goform/getQos",
        modules: "localhost,onlineList,macFilter,guestList,wifiBasicCfg,wifiRelay,wifiGuest",
        setUrl: "goform/setQos"
    });
    pageModule.modules = [];
    module.exports = pageModule;

    /********Attached Devices and Blocked Devices******************/

    //获取黑名单个数
    function getBlackLength() {
        var index = 0,
            i = 0,
            $listTable = $("#qosList").children(),
            length = $listTable.length,
            $blackTable = $("#qosListAccess").children(),
            blackLength = $blackTable.length,
            $tr;

        //access
        if (length == 1 && $listTable.eq(0).children().length < 2) {

        } else {
            for (i = 0; i < length; i++) {
                $tr = $listTable.eq(i);

                //判断是否关闭 或者上传或下载限制为0
                if ($tr.find(".internet-ctl").children().hasClass("toggle-off-icon") || $tr.find(".qosListUpLimitTd").find(".input-append")[0].val() == "0" || $tr.find(".qosListDownLimitTd").find(".input-append")[0].val() == "0") {
                    index++;
                }
            }
        }

        //black
        for (i = 0; i < blackLength; i++) {
            //存在未解除限制的条目
            if ($blackTable.eq(i).find(".deviceName").html()) {
                index++;
            }
        }
        return index;
    }

    function shapingSpeed(value) {
        var val = parseFloat(value);

        if (val > 1024) {
            return (val / 1024).toFixed(2) + "MB/s";
        } else {
            return val.toFixed(0) + "KB/s";
        }
    }

    pageModule.beforeSubmit = function() {
        if (getBlackLength() > 10) {
            top.mainLogic.showModuleMsg(_("A maximum of %s devices can be added to the blacklist.", [10]));
            return false;
        }

        //限速用户不能超过20个（可以接入网络的情况下）
        var $listTable = $("#qosList").children(),
            length = $listTable.length,
            count = 0,
            $td, upLimit, downLimit, i;


        for (i = 0; i < length; i++) {
            $td = $listTable.eq(i).children();

            //在没有设备的情况下，调出循环
            if ($td.hasClass("no-device")) {
                break;
            }

            internetFobid = $td.eq(5).children().hasClass("toggle-off-icon");

            //在可以连网情况下，如果上传或下载设置了限速，则count++
            if (!internetFobid) {
                upLimit = $td.eq(4).find(".input-append")[0].val();
                downLimit = $td.eq(3).find(".input-append")[0].val();
                if (downLimit != "No Limit" || upLimit != "No Limit") {
                    count++;
                }
            }
        }

        if (count > 20) {
            top.mainLogic.showModuleMsg(_("The number of items cannot exceed %s.", [20]));
            return false;
        }
        return true;
    }

    var netCrlModule = new AttachedModule();
    pageModule.modules.push(netCrlModule);

    function AttachedModule() {
        var timeFlag, refreshDataFlag, dataChanged;
        var that = this;

        this.data = {};

        this.moduleName = "qosList";

        this.init = function() {
            refreshDataFlag = true;
            dataChanged = false;
            this.initEvent();
        }
        this.initEvent = function() {
            $("#qosList").delegate(".icon-edit", "click", editDeviceName);

            // $("#qosList").delegate(".icon-toggle-on, .icon-toggle-off", "click", clickAccessInternet);
            $("#qosList").delegate(".switch", "click", clickAccessInternet);

            $("#qosList").delegate(".edit-old", "blur", function() {
                $(this).parent().prev().attr("title", $(this).val());
                $(this).parent().prev().text($(this).val());
                $(this).parent().hide();
                $(this).parent().prev().show();
                $(this).parent().next().show();
            });

            $("#qosListAccess").delegate(".del", "click.dd", function(evnet) {
                var e = evnet || window.event;
                $(this).parent().parent().remove();
                dataChanged = true;
                //return;
            });

            $("#qosList").delegate(".dropdown input[type='text']", "blur.refresh", function() {
                var $access = $(this).parent().parent().parent().children().eq(5); //接入控制选项
                refreshDataFlag = true;
                //如果当前值为无限制，那么手动修改value为 'No Limit'
                if (this.value.replace(/[ ]+$/, "") == _("No limit")) {
                    this.value = _("No Limit");
                    $(this).next().val("No Limit");
                    return;
                }

                if (isNaN(parseInt(this.value, 10))) {
                    //为空或非数字时，默认为不限制
                    this.value = _("No limit");
                } else {
                    //含有小数位时取整数
                    if (this.value.indexOf(".") !== -1) {
                        this.value = this.value.split(".")[0];
                    }

                    if (parseInt(this.value) > 38400) { //大于38400则表示无限制
                        this.value = _("No limit");
                    } else if (parseInt(this.value) <= 0) {

                        //当为本机时，不可限制速度为0KB/s
                        if ($access.children().text() == _("Local")) {
                            this.value = _("No limit");
                        } else {
                            this.value = "0" + "KB/s";
                        }

                    } else {
                        this.value = parseInt(this.value, 10) + "KB/s";
                    }
                }

                timeFlag = setTimeout(function() {
                    refreshTableList();
                }, 5000);
            });

            $("#qosList").delegate(".setDeviceName", "keyup", function() {
                var deviceVal = this.value,
                    len = deviceVal.length, //输入总字符数
                    totalByte = getStrByteNum(deviceVal); //输入总字节数

                if (totalByte > 63) {
                    for (var i = len - 1; i > 0; i--) {
                        totalByte = totalByte - getStrByteNum(deviceVal[i]); //每循环一次，总字节数就减去最后一个字符的字节数，

                        if (totalByte <= 63) { //直到总字节数小于等于63，i值就是边界值的下标
                            this.value = deviceVal.slice(0, i);
                            break;
                        }
                    }
                }
            });
        };
        this.initValue = function() {
            this.data = pageModule.data;

            timeFlag = setTimeout(function() {
                refreshTableList();
            }, 5000);

            $("#qosList").html(""); //
            $("#qosListAccess").html(""); //受限制table
            createOnlineList(this.data);

            //路由模式且无线开启且访客网络开启时，才有访客网络列表
            if (this.data.wifiRelay.wifiRelayType == "disabled" && (this.data.wifiBasicCfg.wifiEn == "true" || this.data.wifiBasicCfg.wifiEn_5G == "true") && this.data.wifiGuest.guestEn == "true") {
                $("#guestListWrap").removeClass("none");

                //清除访客网络列表中显示的数据，重新初始化
                $("#guestList").html("");
                createGuestList(this.data.guestList);
            }
        };

        /**
         * @method createOnlineList [创建在线及离线显示列表]
         * @param  {Object} obj [对象中包含本机IP地址， 在线列表，访客网络，黑名单列表]
         */
        function createOnlineList(obj) {
            var i = 0,
                k = 0,
                str = "",
                limitStr = "",
                onlineListlen, blackListLen,
                prop, divElem, tdElem, trElem, upLimit, downLimit,
                localhostIP, deviceName, ipStr;

            var selectDownObj = {
                    "initVal": "21",
                    "editable": "1",
                    "size": "small",
                    "seeAsTrans": true,
                    "options": [{
                        "No Limit": _("No limit")
                    }, {
                        "128": _("128 KB/s (Web)")
                    }, {
                        "256": _("256 KB/s (SD Videos)")
                    }, {
                        "512": _("512 KB/s (HD Videos)")
                    }, {
                        ".divider": ".divider"
                    }, {
                        ".hand-set": _("Manual (unit: KB/s)")
                    }]
                },
                selectUpObj = {
                    "initVal": "21",
                    "editable": "1",
                    "size": "small",
                    "seeAsTrans": true,
                    "options": [{
                        "No Limit": _("No limit")
                    }, {
                        "32": "32" + "KB/s"
                    }, {
                        "64": "64" + "KB/s"
                    }, {
                        "128": "128" + "KB/s"
                    }, {
                        ".divider": ".divider"
                    }, {
                        ".hand-set": _("Manual (unit: KB/s)")
                    }]
                };


            localhostIP = obj.localhost.localhost;
            obj.onlineList = reCreateObj(obj.onlineList, "qosListIP", "up");

            //将本机添加到数组首位
            for (var i = 0; i < obj.onlineList.length; i++) {
                if (obj.onlineList[i].qosListIP == localhostIP) {
                    var local = obj.onlineList[i];
                    obj.onlineList.splice(i, 1);
                    obj.onlineList.unshift(local);
                    break;
                }
            }

            obj.macFilter.macFilterList = reCreateObj(obj.macFilter.macFilterList, "mac", "up");
            onlineListlen = obj.onlineList.length;
            macFilterListLen = obj.macFilter.macFilterList.length;

            //初始化在线列表
            for (i = 0; i < onlineListlen; i++) {
                str = "<tr class='addListTag'>"; //类只为初始化与赋值，用完就删除
                for (prop in obj.onlineList[i]) {
                    if (obj.onlineList[i].qosListRemark != "") {
                        deviceName = obj.onlineList[i].qosListRemark;
                    } else {
                        deviceName = obj.onlineList[i].qosListHostname;
                    }

                    ipStr = obj.onlineList[i].qosListIP;

                    var manufacturer = translateManufacturer(obj.onlineList[i].qosListManufacturer);

                    switch (prop) {
                        case "qosListHostname":
                            str += '<td class="qosListHostnameTd">';
                            str += "<div class='col-xs-3 col-sm-3 col-md-3 col-lg-3'>" + manufacturer + "</div>"
                            str += '<div class="col-xs-8 col-sm-9 col-md-9 col-lg-9"><div class="col-xs-11 span-fixed deviceName" style="height:24px;"></div>';
                            str += '<div class="col-xs-11 none">';
                            str += ' <input type="text" class="form-control setDeviceName" style="height: 24px;padding: 3px 12px;" value="" maxLength="63">';
                            str += '</div>';
                            str += '<div class="col-xs-1 row"> <span class="ico-small icon-edit" title="' + _("Edit") + '"></span> </div>';
                            str += '<div class="col-xs-12 help-inline"><span style="color:#ccc; font-size:12px;">' + ipStr + '</span> </div></div>';
                            str += '</td>';
                            break;
                        case "qosListUpSpeed":
                        case "qosListDownSpeed":
                            //在此处增加width:10%是为了规避由于上、下载数值较小时头部文字出现换行；
                            str += '<td class="span-fixed" style="width:10%;">';
                            if (prop == "qosListUpSpeed") {
                                str += '<span class="text-warning">&uarr;</span> <span>' + shapingSpeed(obj.onlineList[i][prop]) + '</span>';
                            } else {
                                str += '<span class="text-success">&darr;</span> <span>' + shapingSpeed(obj.onlineList[i][prop]) + '</span>';
                            }
                            str += '</td>';
                            break;
                        case "qosListUpLimit":
                            str += '<td class="qosListUpLimitTd">';

                            str += '<span class="dropdown ' + prop + ' input-medium validatebox" required="required" maxLength="5"></span>';

                            str += '</td>';
                            upLimit = obj.onlineList[i][prop] + "KB/s";
                            if (+obj.onlineList[i][prop] >= 38401) {
                                upLimit = ("No Limit");
                            }
                            break;

                        case "qosListDownLimit":
                            str += '<td class="qosListDownLimitTd">';

                            str += '<span class="dropdown ' + prop + ' input-medium validatebox" required="required" maxLength="5"></span>';

                            str += '</td>';

                            downLimit = obj.onlineList[i][prop] + "KB/s";
                            if (+obj.onlineList[i][prop] >= 38401) {
                                downLimit = ("No Limit");
                            }

                            break;
                        case "qosListAccess":

                            if (localhostIP == obj.onlineList[i].qosListIP) {
                                str += '<td class="internet-ctl" style="text-align:center;">';
                                str += "<div class='native-device'>" + _("Local") + "</div>";
                            } else {
                                str += '<td class="internet-ctl" style="text-align:center;">';
                                // str += "<div class='switch icon-toggle-on'></div>";
                                str += "<div class='switch toggle-on-icon'></div>";
                            }
                            str += '</td>';
                            break;
                    }
                }

                str += '</tr>';
                $("#qosList").append(str);
                $("#qosList .addListTag").find(".deviceName").text(deviceName); //主机名赋值
                $("#qosList .addListTag").find(".deviceName").attr("title", deviceName);
                $("#qosList .addListTag").find(".setDeviceName").val(deviceName);
                $("#qosList .addListTag").find(".setDeviceName").attr("data-mark", obj.onlineList[i].qosListHostname); //绑定主机名
                $("#qosList .addListTag").find(".setDeviceName").attr("alt", obj.onlineList[i].qosListMac.toUpperCase()); //绑定mac
                selectUpObj.initVal = upLimit;
                $("#qosList .addListTag").find(".qosListUpLimit").toSelect(selectUpObj);
                if (upLimit == "No Limit") {
                    $("#qosList .addListTag").find(".qosListUpLimit").find("input[type='text']").val(_("No limit"));
                }
                //初始化下拉框
                selectDownObj.initVal = downLimit;
                $("#qosList .addListTag").find(".qosListDownLimit").toSelect(selectDownObj);
                if (downLimit == "No Limit") {
                    $("#qosList .addListTag").find(".qosListDownLimit").find("input[type='text']").val(_("No limit"));
                }
                //初始化下拉框
                $("#qosList .addListTag").find(".input-box").attr("maxLength", "5");
                //增加根据屏幕宽度隐藏部分数据信息
                $("#qosList").find(".addListTag").children().eq(1).addClass("hidden-max-sm");
                $("#qosList").find(".addListTag").children().eq(2).addClass("hidden-max-md");
                $("#qosList").find(".addListTag").find(".qosListDownLimitTd").addClass("hidden-max-xs");
                $("#qosList").find(".addListTag").find(".qosListUpLimitTd").addClass("hidden-max-md");

                $("#qosList").find(".addListTag").removeClass("addListTag");

            } //end


            //初始化黑名单列表
            for (k = 0; k < macFilterListLen; k++) {
                if (obj.macFilter.macFilterList[k].filterMode == "pass") {
                    continue;
                }
                str = "<tr class='addListTag'>";
                str += "<td class='deviceName'><div class='col-xs-11 span-fixed'>";
                str += "</div></td>";
                str += "<td class='hidden-max-xs denyMac'>" + obj.macFilter.macFilterList[k].mac.toUpperCase() + "</td>";

                str += "<td>";
                str += '<input type="button" class="del btn" value="' + _("Unlimit") + '">';
                str += "</td>";
                str += "</tr>";
                $("#qosListAccess").append(str);
                if (obj.macFilter.macFilterList[k].remark != "") {
                    deviceName = obj.macFilter.macFilterList[k].remark;
                } else {
                    deviceName = (obj.macFilter.macFilterList[k].hostname == "Unknown"?_("Unknown"):obj.macFilter.macFilterList[k].hostname);
                }

                $("#qosListAccess .addListTag").find(".deviceName div").text(deviceName);
                $("#qosListAccess .addListTag").find(".deviceName").attr("data-hostname", obj.macFilter.macFilterList[k].hostname);
                $("#qosListAccess .addListTag").find(".deviceName").attr("data-remark", obj.macFilter.macFilterList[k].remark);
                $("#qosListAccess").find(".addListTag").removeClass("addListTag");
            } //end
            $("#qosDeviceCount").html("(" + $("#qosList").children().length + ")");
            if ($("#qosList").children().length == 0) {
                str = "<tr><td colspan='2' class='no-device'>" + _("No device") + "</td></tr>";
                $("#qosList").append(str);
            }

            //$("#blockedDeviceCount").html("(" + $("#qosListAccess").children().length + ")");
            if ($("#qosListAccess").children().length == 0) {
                str = "<tr><td colspan='2'>" + _("No device") + "</td></tr>";
                $("#qosListAccess").append(str);
            }

            //白名单模式下没有互联网访问控制及黑名单功能。
            if (pageModule.data.macFilter.curFilterMode == "pass") {
                $(".internet-ctl").addClass("none");
                $("#accessCtrl").css("display", "none");
                $("#blockedDevices").addClass("none");
            } else {
                $(".internet-ctl").removeClass("none");
                $("#accessCtrl").css("display", "");
                $("#blockedDevices").removeClass("none");
            }
            top.mainLogic.initModuleHeight();
        }

        this.checkData = function() { //数据验证
            var deviceName = "",
                $listTable = $("#qosList").children(),
                length = $listTable.length,
                $td,
                upLimit,
                downLimit,
                i = 0;
            if (length == 1 && $listTable.eq(0).children().length < 2) {
                return;
            }
            for (i = 0; i < length; i++) {
                $tr = $listTable.eq(i);
                deviceName = $tr.find(".setDeviceName").val(); //device name
                upLimit = ($tr.find(".qosListUpLimit")[0].val()); //up limit
                downLimit = ($tr.find(".qosListDownLimit")[0].val()); //down limit

                if (deviceName.replace(/[ ]/g, "") == "") {
                    $tr.find(".setDeviceName").eq(0).focus();
                    return _("No space is allowed in a device name.");
                }

                //如果手动输入时
                if (upLimit == _("No limit")) {
                    upLimit = "No Limit";
                }

                if (downLimit == _("No limit")) {
                    downLimit = "No Limit";
                }

                if (isNaN(parseFloat(upLimit)) && upLimit != ("No Limit")) {
                    $tr.find(".qosListUpLimitTd").find(".dropdown .input-box").focus();
                    return _("Please enter a valid number.")
                }

                if (isNaN(parseFloat(downLimit)) && downLimit != ("No Limit")) {
                    $tr.find(".qosListDownLimitTd").find(".dropdown .input-box").focus();
                    return _("Please enter a valid number.")
                }
            }
            return;
        };

        this.getSubmitData = function() { //获取提交数据
            var listArray = getBlockedList().concat(getAttacheList()),
                onlineListStr = departList("true"),
                offlineListStr = departList("false"),
                onlineObj = {},
                offlineObj = {};


            onlineObj = {
                module1: "onlineList",
                onlineList: onlineListStr
            };

            offlineObj = {
                module2: "macFilter",
                macFilterList: offlineListStr

            };
            if (that.data.macFilter.curFilterMode == "pass") {
                return objToString(onlineObj);
            } else {
                return objToString(onlineObj) + "&" + objToString(offlineObj);
            }

            function departList(type) {
                var i = 0,
                    tmpList = [],
                    tmpStr = "";
                for (i = 0; i < listArray.length; i++) {
                    if (listArray[i].access == type) {
                        tmpStr += listArray[i].hostname + "\t";
                        tmpStr += listArray[i].remark + "\t";
                        tmpStr += listArray[i].mac + "\t";
                        tmpStr += listArray[i].upLimit + "\t";
                        tmpStr += listArray[i].downLimit + "\t";
                        tmpStr += listArray[i].access + "\n";
                    }
                }
                return tmpStr.replace(/[\n]$/, "");
            }

        };

        function refreshTableList() {
            $.get("goform/getQos?" + getRandom() + encodeURIComponent("&modules=localhost,onlineList,macFilter,guestList,wifiBasicCfg,wifiRelay,wifiGuest"), updateTable);

            if (!refreshDataFlag || dataChanged) {
                clearTimeout(timeFlag);
                return;
            }
            clearTimeout(timeFlag);
            timeFlag = setTimeout(function() {
                refreshTableList();
            }, 5000);
            if (!pageModule.pageRunning) {
                clearTimeout(timeFlag);
            }
        }

        /*
         *
         * @method updateTable 处理获取到的在线列表数据：
         *    1、如果用户手动修改了当前在线列表数据，则保持不变；如果当前在线列表数据在CGI有更新，则针对该数据进行更新。
         *    2、如果CGI有新增的在线条目，则append至在线列表里面
         *    3、如果CGI有删除的在线条目，则将在线列表里面的数据删除
         * @param {Object} 对象中包含本机，在线列表，黑名单列表数据
         * @return {Array} 由本机IP、在线列表、黑名单列表组合成的数组
         *
         */
        function updateTable(obj) {
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
                return;
            }

            if (!pageModule.pageRunning || dataChanged) {
                return;
            }

            obj.onlineList = reCreateObj(obj.onlineList, "qosListIP", "up");

            //将本机添加到数组首位
            for (var i = 0; i < obj.onlineList.length; i++) {
                if (obj.onlineList[i].qosListIP == obj.localhost.localhost) {
                    var local = obj.onlineList[i];
                    obj.onlineList.splice(i, 1);
                    obj.onlineList.unshift(local);
                    break;
                }
            }

            var getOnlineList = obj.onlineList;

            var $onlineTbodyList = $("#qosList").children(),
                onlineTbodyLen = $onlineTbodyList.length,
                getOnlineLen = getOnlineList.length,
                j = 0,
                i = 0,
                oldMac, newMac;

            var rowData = new Array(onlineTbodyLen);
            var refreshObj = new Array(getOnlineLen);
            var newDataArray = [];

            for (i = 0; i < getOnlineLen; i++) {
                newMac = getOnlineList[i].qosListMac.toUpperCase();
                refreshObj[i] = {};
                for (j = 0; j < onlineTbodyLen; j++) {
                    //TODO : ReasyJS不能对空对象进行操作, 如： 全部被拦截上网的情况
                    var $input = $onlineTbodyList.eq(j).children().eq(0).find("input")
                    if ($input[0]) {
                        oldMac = $input.eq(0).attr("alt").toUpperCase();
                    } else {
                        oldMac = '';
                    }

                    if (oldMac == newMac) { //存在
                        rowData[j] = {};
                        //在线或本机
                        if (!$onlineTbodyList.eq(j).children().find('.internet-ctl').children().hasClass("toggle-off-icon")) { //当前为允许时
                            $onlineTbodyList.eq(j).children().eq(2).find("span").eq(1).html(shapingSpeed(getOnlineList[i].qosListUpSpeed));

                            $onlineTbodyList.eq(j).children().eq(1).find("span").eq(1).html(shapingSpeed(getOnlineList[i].qosListDownSpeed));
                        }
                        rowData[j].refresh = true; //
                        refreshObj[i].exist = true;
                    }
                    if ($onlineTbodyList.eq(j).children().eq(0).find("input").eq(0).hasClass("edit-old")) { //已编辑过的不能删除
                        rowData[j] = {};
                        rowData[j].refresh = true;
                    }
                }
            }

            for (i = 0; i < getOnlineLen; i++) {
                if (!refreshObj[i].exist) {
                    newDataArray.push(getOnlineList[i]); //新增
                }
            }

            for (j = 0; j < onlineTbodyLen; j++) {
                if (!rowData[j] || !rowData[j].refresh) {
                    //将在线列表中当前已经不在线的设备删除
                    $onlineTbodyList.eq(j).remove();
                }
            }

            //清除黑名单列表中显示的数据，重新初始化
            $("#qosListAccess").html("");
            obj.onlineList = newDataArray;
            createOnlineList(obj);

            if (obj.wifiRelay.wifiRelayType == "disabled" && (obj.wifiBasicCfg.wifiEn == "true" || obj.wifiBasicCfg.wifiEn_5G == "true") && obj.wifiGuest.guestEn == "true") {
                //清除访客网络列表中显示的数据，重新初始化
                $("#guestList").html("");
                createGuestList(obj.guestList);
            }

        };

        function editDeviceName() {
            var deviceName = $(this).parent().prev().prev().text(),
                reMarkMaxLength = "";
            $(this).parent().prev().prev().hide(); //隐藏用户名
            $(this).parent().hide(); //隐藏编辑
            $(this).parent().prev().show();
            $(this).parent().prev().find("input").addClass("edit-old"); //编辑时给编辑元素增加类标志
            reMarkMaxLength = $(this).parent().prev().find("input").attr("maxLength");
            $(this).parent().prev().find("input").val(deviceName.substring(0, reMarkMaxLength));
            $(this).parent().prev().find("input").focus();
        }

        function clickAccessInternet() {
            var className = this.className || "switch toggle-on-icon";
            if (className == "switch toggle-on-icon") {

                if (getBlackLength() >= 10) {
                    top.mainLogic.showModuleMsg(_("A maximum of %s devices can be added to the blacklist.", [10]));
                    return;
                }
                this.className = "switch toggle-off-icon";
            } else {
                this.className = "switch toggle-on-icon";
            }
        }

        //获取允许列表数据
        function getAttacheList() {
            var str = "",
                $listTable = $("#qosList").children(),
                length = $listTable.length,
                $td,
                hostname,
                internetAccess,
                isNativeDevice,
                tmpList = [],
                i = 0;

            if (length == 1 && $listTable.eq(0).children().length < 2) {
                return tmpList;
            }

            for (i = 0; i < length; i++) {
                var tmpObj = {},
                    device, hostname, remark, upLimit, downLimit;

                $tr = $listTable.eq(i);
                //允许
                internetAccess = $tr.find(".internet-ctl").children().hasClass("toggle-on-icon");
                //本机
                isNativeDevice = $tr.find(".internet-ctl").children().hasClass("native-device");

                //设备名称栏
                device = $tr.find(".qosListHostnameTd").find("input").eq(0);
                //用户名
                hostname = device.attr("data-mark");
                tmpObj.hostname = hostname;

                //remark;
                remark = device.val();
                if (remark == hostname) { //主机名与备注一样
                    tmpObj.remark = "";
                } else {
                    tmpObj.remark = remark;
                }

                //mac
                tmpObj.mac = device.attr("alt");

                //下载限速
                downLimit = $tr.find(".qosListDownLimit")[0].val();
                downLimit = transLimit(internetAccess, isNativeDevice, downLimit);

                tmpObj.downLimit = downLimit;

                //上传限速
                upLimit = $tr.find(".qosListUpLimit")[0].val();
                upLimit = transLimit(internetAccess, isNativeDevice, upLimit);
                tmpObj.upLimit = upLimit;

                //是否允许接入
                tmpObj.access = (internetAccess || isNativeDevice) ? "true" : "false";

                tmpList.push(tmpObj);

            }

            return tmpList;

            function transLimit(internetAccess, isNativeDevice, limit) {
                //当为普通接入端时，且被禁止上网时，限速为0
                if (!internetAccess && !isNativeDevice) {
                    limit = "0";
                } else if (isNativeDevice) {
                    if (+limit < 1 || limit == ("No Limit")) {
                        limit = "38528";
                    } else {
                        limit = parseInt(limit, 10);
                    }
                } else {
                    //值小于0时，向后台传0值
                    if (+limit < 0) {
                        limit = "0";
                    } else if (limit == "No Limit") {
                        limit = "38528";
                    } else {
                        limit = parseInt(limit, 10);
                    }
                }

                if (+limit >= 38528) {
                    limit = 38528;
                }

                return limit;
            }
        }

        //获取禁止列表数据 
        function getBlockedList() {
            var $listTable = $("#qosListAccess").children(),
                length = $listTable.length,
                i = 0,
                tmpList = [];

            //暂无设备时
            if (length == 1 && $listTable.eq(0).children().length < 2) {

                return tmpList;
            }
            for (i = 0; i < length; i++) {
                var tmpObj = {};

                //拒绝访问
                tmpObj.hostname = $listTable.eq(i).children().eq(0).attr("data-hostname");
                tmpObj.remark = $listTable.eq(i).children().eq(0).attr("data-remark");

                tmpObj.mac = $listTable.eq(i).find(".denyMac").text();
                tmpObj.upLimit = "0";
                tmpObj.downLimit = "0";
                tmpObj.access = "false";
                tmpList.push(tmpObj);

            }
            return tmpList;
        }
    }

    /********END Attached Devices and Blocked Devices ************************/



    /************************ Guest Devices ************************/
    function createGuestList(obj) {
        var i = 0,
            str = "",
            guestListlen,
            prop,
            deviceName,
            ipStr,
            manufacturer;

        guestListlen = obj.length;

        obj = reCreateObj(obj, "qosListIP", "up");

        //初始化在线列表
        for (; i < guestListlen; i++) {
            str = "<tr class='addListTag'>"; //类只为初始化与赋值，用完就删除
            for (prop in obj[i]) {
                deviceName = obj[i].qosListHostname;

                ipStr = obj[i].qosListIP;

                manufacturer = translateManufacturer(obj[i].qosListManufacturer);

                switch (prop) {
                    case "qosListHostname":
                        str += '<td>';
                        str += "<div class='col-xs-3 col-sm-3 col-md-3 col-lg-3 guest-device'>" + manufacturer + "</div>";
                        str += '<div class="col-xs-8 col-sm-9 col-md-9 col-lg-9"><div class="col-xs-12 span-fixed deviceName" style="height:38px;padding-top:10px;"></div>';
                        str += '</td>';
                        break;
                    case "qosListIP":
                        str += '<td>';
                        str += '<div class="col-xs-12"><span>' + ipStr + '</span> </div></div>';
                        str += '</td>';
                        break;
                    case "qosListUpSpeed":
                    case "qosListDownSpeed":
                        //在此处增加width:10%是为了规避由于上、下载数值较小时头部文字出现换行；
                        str += '<td class="hidden-max-sm span-fixed" style="width:10%;">';
                        if (prop == "qosListUpSpeed") {
                            str += '<span class="text-warning">&uarr;</span> <span>' + shapingSpeed(obj[i][prop]) + '</span>';
                        } else {
                            str += '<span class="text-success">&darr;</span> <span>' + shapingSpeed(obj[i][prop]) + '</span>';
                        }
                        str += '</td>';
                        break;
                }
            }

            str += '</tr>';
            $("#guestList").append(str);
            $("#guestList .addListTag").find(".deviceName").text(deviceName); //主机名赋值
            $("#guestList .addListTag").find(".deviceName").attr("title", deviceName);

            $("#guestList").find(".addListTag").removeClass("addListTag");

        } //end


        $("#guestDeviceCount").html("(" + $("#guestList").children().length + ")");
        if ($("#guestList").children().length == 0) {
            str = "<tr><td colspan='2' class='no-device'>" + _("No device") + "</td></tr>";
            $("#guestList").append(str);
        }

        top.mainLogic.initModuleHeight();
    }

    /**
     * 显示访客网络列表
     * @type {GuestModule}
     */
    var guestCrlModule = new GuestModule();
    pageModule.modules.push(guestCrlModule);

    function GuestModule() {
        var _this = this,
            refreshTime;

        this.moduleName = "guestList";

        this.init = function() {
            this.initEvent();
        }
        this.initEvent = function() {

        };
        this.initValue = function(obj) {
            $("#guestList").html("");

            createGuestList(obj);
        };
    }
    /************************ END Guest Devices *******************/
})