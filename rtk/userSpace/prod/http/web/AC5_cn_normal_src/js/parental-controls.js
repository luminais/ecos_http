define(function(require, exports, module) {

    var pageModule = new PageLogic({
        getUrl: "goform/getParentControl",
        modules: "parentCtrlList,parentAccessCtrl",
        setUrl: "goform/setParentControl"
    });
    pageModule.modules = [];
    module.exports = pageModule;

    pageModule.beforeSubmit = function() {
        var urlInputVal = $("#urlFilterAllow").val(),
            msg = CheckUrlVolidate();

        if (msg && urlInputVal !== "") { //输入框有错误，且不是为空时，提示用户错误信息
            top.mainLogic.showModuleMsg(msg);
            return false;
        }

        return true;
    };
    /**
     * [attachedModule 显示及操作家长控制在线列表]
     */
    var attachedModule = new AttachedModule();
    pageModule.modules.push(attachedModule);

    function AttachedModule() {
        var timeFlag;

        this.moduleName = "parentCtrlList";
        this.init = function() {
            this.initEvent();
        };
        this.initEvent = function() {
            // $("#onlineList").delegate(".icon-toggle-off, .icon-toggle-on", "click", changeDeviceManage);
            $("#onlineList").delegate(".switch", "click", changeDeviceManage);

            $("#onlineList").delegate(".icon-edit", "click", editDeviceName);

            $("#onlineList").delegate(".form-control", "blur", function() {
                //若没有修改过，则去掉css类edit-old
                if ($(this).val() == $(this).next().attr("data-remark")) {
                    $(this).removeClass("edit-old");
                }

                $(this).next().attr("title", $(this).val());
                $(this).next().text($(this).val());
                $(this).next().show(); //显示设备名称
                $(this).next().next().show(); //显示编辑按钮
                $(this).hide(); //隐藏自身
            });
        };
        this.initValue = function(onlineArr) {
            //生成在线列表
            $("#onlineList").html("");
            creatOnlineTable(onlineArr);

            //每5秒刷新在线设备列表
            timeFlag = setTimeout(function() {
                refreshTableList();
            }, 5000);

            this.adjustWidth();
        };

        this.adjustWidth = function() {
            if (window.innerWidth < 375) {
                $(".span-fixed").css("width", "90px");
            }
        };

        this.checkData = function() {
            var deviceName = "",
                $listTable = $("#onlineList").children(),
                length = $listTable.length,
                $td,
                i = 0;

            for (i = 0; i < length; i++) {
                $td = $listTable.eq(i).children();
                deviceName = $td.find("input").eq(0).val(); //device name
                if (deviceName.replace(/[ ]/g, "") == "") {
                    $td.find("input").eq(0).focus();
                    return _("No space is allowed in a device name.");
                }
            }
            return;
        };
        this.getSubmitData = function() {
            var data = {
                module1: this.moduleName,
                onlineList: getOnlineListData()
            }
            return objToString(data);
        };



        function getOnlineListData() {
            var str = "",
                i = 0,
                listArry = $("#onlineList").children(),
                len = listArry.length,
                hostname;
            for (i = 0; i < len; i++) {
                hostname = $(listArry).eq(i).children().find("div").eq(0).attr("data-hostName");
                str += hostname + "\t"; //主机名
                if (hostname == $(listArry).eq(i).children().find("input").val()) {
                    str += "\t";
                } else {
                    str += $(listArry).eq(i).children().find("input").val() + "\t"; //备注
                }
                str += $(listArry).eq(i).children().find("div").eq(0).attr("alt") + "\t"; //mac
                str += $(listArry).eq(i).children().eq(1).html() + "\t"; //ip
                str += $(listArry).eq(i).children().eq(3).find("div").hasClass("toggle-on-icon") + "\n";
            }
            str = str.replace(/[\n]$/, "");
            return str;
        }

        function editDeviceName() {
            var deviceName = $(this).parent().prev("div").text();
            $(this).parent().parent().find("div").hide();
            $(this).parent().parent().find("input").show().addClass("edit-old"); //编辑时给编辑元素增加类标志
            $(this).parent().parent().find("input").val(deviceName);
            $(this).parent().parent().find("input").focus();
        }

        function getEnablelist() {
            var index = 0,
                i = 0,
                $listArry = $("#onlineList").children(),
                length = $listArry.length;
            for (i = 0; i < length; i++) {
                if ($listArry.eq(i).children().eq(3).find("div").hasClass("toggle-on-icon")) {
                    index++;
                }
            }
            return index;
        }

        function changeDeviceManage() {
            var className = this.className || "switch toggle-on-icon";
            if (className == "switch toggle-on-icon") {
                this.className = "switch toggle-off-icon";
            } else {
                if (getEnablelist() >= 10) {
                    top.mainLogic.showModuleMsg(_("A maximum of %s entries can be added.", [10]));
                    return;
                }
                this.className = "switch toggle-on-icon";
            }
        }

        function refreshTableList() {
            $.get("goform/getParentControl" + getRandom() + "&modules=parentCtrlList", updateTable);

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
         * @param {Object} 对象中包含parentCtrlList数组的数据
         * @return {Array} 
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

            if (!pageModule.pageRunning) {
                return;
            }

            var getOnlineList = obj.parentCtrlList;

            var $onlineTbodyList = $("#onlineList").children(),
                onlineTbodyLen = $onlineTbodyList.length,
                getOnlineLen = getOnlineList.length,
                j = 0,
                i = 0,
                oldMac, newMac;

            var rowData = new Array(onlineTbodyLen); //记录当前列表项是否已帅新的标记数组
            var refreshObj = new Array(getOnlineLen); //标记当前设备是否已存在列表中的标记数组
            var newDataArray = []; //更新后的数据

            for (i = 0; i < getOnlineLen; i++) {
                newMac = getOnlineList[i].parentCtrlMAC.toUpperCase(); //以mac地址为对比参照

                refreshObj[i] = {}; //默认不存在当前设备

                for (j = 0; j < onlineTbodyLen; j++) {
                    var $nameDom = $onlineTbodyList.eq(j).children().eq(0).find(".device-name-show");

                    if ($nameDom[0]) {
                        oldMac = $nameDom.eq(0).attr("alt").toUpperCase();
                    } else {
                        oldMac = '';
                    }

                    //当前列表中存在此设备，则更新在线时长
                    if (oldMac == newMac) {
                        //先将本行的数据清空
                        rowData[j] = {};

                        //方法一：
                        $onlineTbodyList.eq(j).children().eq(2).html(formatSeconds(getOnlineList[i].parentCtrlConnectTime));
                        //方法二：手动更新时间 
                        /*var time_old = +$onlineTbodyList.eq(j).children().eq(2).attr("data-onlinetime") + 5,
                            time_new = formatSeconds(time_old);

                        $onlineTbodyList.eq(j).children().eq(2).html(time_new);
                        $onlineTbodyList.eq(j).children().eq(2).attr("data-onlinetime", time_old);*/
                        //更新时间end

                        rowData[j].refresh = true; //标记本行已刷新

                        refreshObj[i].exist = true; //标记当前设备已在列表中
                    }

                    //当前列表中不存在此设备，有可能是新增的，也有可能是已下线的
                    if ($onlineTbodyList.eq(j).children().eq(0).find("input").eq(0).hasClass("edit-old")) { //已编辑过的不能删除，因为还要提交
                        //先将本行的数据清空
                        rowData[j] = {};

                        rowData[j].refresh = true;
                    }
                }
            }

            for (i = 0; i < getOnlineLen; i++) {
                if (!refreshObj[i].exist) {
                    newDataArray.push(getOnlineList[i]); //重组新的数据
                }
            }

            for (j = 0; j < onlineTbodyLen; j++) {
                if (!rowData[j] || !rowData[j].refresh) {
                    //将在线列表中当前已经不在线的设备删除
                    $onlineTbodyList.eq(j).remove();
                }
            }

            //重新生成在线设备列表
            if (newDataArray.length !== 0) {
                creatOnlineTable(newDataArray);
            }
        }

        function creatOnlineTable(obj) {
            var len = obj.length,
                i = 0,
                str = "",
                prop,
                hostname,
                divElem,
                divElem1,
                trElem,
                tdElem;

            for (i = 0; i < len; i++) {

                trElem = document.createElement("tr");

                //td内容的初始化
                var tdStr = "";
                for (prop in obj[i]) {

                    if (obj[i].parentCtrlRemark != "") {
                        hostname = obj[i].parentCtrlRemark;
                    } else {
                        hostname = obj[i].parentCtrlHostname;
                    }
                    //设备名
                    if (prop == "parentCtrlHostname") {
                        tdStr += '<td><input type="text" class="form-control none device-name" style="width:66%;" value="" maxLength="63" />';
                        tdStr += '<div class="col-xs-8 span-fixed device-name-show"></div>';
                        tdStr += '<div class="col-xs-2 editDiv"><span class="ico-small icon-edit" title="' + _("Edit") + '">&nbsp;</span></div></td>';

                        //IP地址
                    } else if (prop == "parentCtrlIP") {
                        tdStr += '<td class="hidden-xs">' + obj[i][prop] + '</td>';

                        //连接时间
                    } else if (prop == "parentCtrlConnectTime") {
                        tdStr += '<td class="hidden-xs" data-onlinetime="' + obj[i][prop] + '">' + formatSeconds(obj[i][prop]) + '</td>';

                        //管理
                    } else if (prop == "parentCtrlEn") {
                        if (obj[i][prop] == "true") {
                            // tdStr += "<td><div class='switch icon-toggle-on'></div></td>";
                            tdStr += "<td class='internet-ctl' style='text-align:center;'><div class='switch toggle-on-icon'></div></td>";
                        } else {
                            // tdStr += "<td><div class='switch icon-toggle-off'></div></td>";
                            tdStr += "<td class='internet-ctl' style='text-align:center;'><div class='switch toggle-off-icon'></div></td>";
                        }
                    }
                }
                //将td附加到tr上
                $(trElem).html(tdStr);

                $(trElem).find(".device-name")[0].value = hostname;

                var $deviceNameShow = $(trElem).find(".device-name-show");

                $deviceNameShow.attr("title", hostname);
                $deviceNameShow.attr("alt", obj[i].parentCtrlMAC);
                $deviceNameShow.attr("data-hostName", obj[i].parentCtrlHostname);

                //added by dapei
                $deviceNameShow.attr("data-remark", hostname);

                if (typeof $deviceNameShow.text() != "undefined") {
                    $deviceNameShow[0].innerText = hostname;
                } else { //firefox
                    $deviceNameShow[0].textContent = hostname;
                }

                //将tr附加到tbody上
                $("#onlineList").append($(trElem));
            }

            if ($("#onlineList").children().length == 0) {
                str = "<tr><td colspan='2' class='no-device'>" + _("No device") + "</td></tr>";
                $("#onlineList").append(str);
            }

            top.mainLogic.initModuleHeight();
        }

    }
    /*************END Attached Devices************************/

    /*******handle URL*****/
    function CheckUrlVolidate() {
        /*var strRegex = "^((https|http|ftp|rtsp|mms)?://)" //
            + "?(([0-9a-z_!~*'().= $%-] : )?[0-9a-z_!~*'().= $%-] @)?" //ftp的user@
            + "(([0-9]{1,3}\.){3}[0-9]{1,3}" // IP形式的URL- 199.194.52.184
            + "|" // 允许IP和DOMAIN（域名）
            + "([0-9a-z_!~*'()-] \.)*" // 域名- www.
            + "([0-9a-z][0-9a-z-]{0,61})?[0-9a-z]\." // 二级域名
            + "[a-z]{2,6})" // first level domain- .com or .museum
            + "(:[0-9]{1,4})?" // 端口- :80
            + "((/?)|" // a slash isn't required if there is no file name
            + "(/[0-9a-z_!~*'().;?:@= $,%#-] ) /?)$";
        var re = new RegExp(strRegex);*/
        // var re = /^([A-Za-z]+:\/\/)?[A-Za-z0-9-_]{2,}(\.[A-Za-z0-9-_%&?\/.=])?/;
        //(/^([A-Za-z]+:\/\/)?[A-Za-z0-9-_]+(\.[A-Za-z0-9-_%&?\/.=])?/).test("https://wwwcom:8080")
        //re.test()

        var url = $('#urlFilterAllow').val(),
            len = $("#urlList").children().length,
            i = 0;

        if (url == "") {
            $('#urlFilterAllow').focus();
            return (_("Please input a key word of domain name!"));
        }

        /*if (!re.test(url)) {
            $('#urlFilterAllow').focus();
            return (_("Please input a key word of domain name!"));
        }*/

        /**
         * 开头不能是.
         * 不能连着两个点，比如baidu..com
         * 不能输入协议名称（没意义）
         * 不能有中文标点符号
         * 特殊符号仅支持~#%&_-|\/?=+!*()
         * 特殊符号不支持@`$^<>,空格{}[]"':;（注：不支持的字符，不管是在第几位输入的，都应该报错）
         * 支持中文网址
         */
        if (/^[-_~\#%&\|\\\/\?=+!*\.()0-9a-zA-Z\u4e00-\u9fa5]+$/ig.test(url)) {
            //开头不能有.
            if ((/^(\.)(.+)?$/ig).test(url)) {
                $('#urlFilterAllow').focus();
                return (_("Please input a key word of domain name!"));
            }

            //不能连着两个.
            if (url.indexOf("..") !== -1) {
                $('#urlFilterAllow').focus();
                return (_("Please input a key word of domain name!"));
            }

            //长度范围：2~128字节  
            var ret = $.validate.valid.byteLen(url, 2, 128);
            if (ret) {
                $('#urlFilterAllow').focus();
                return ret;
            }
        } else {
            $('#urlFilterAllow').focus();
            return (_("Please input a key word of domain name!"));
        }

        var trList = $("#urlList").children();
        for (i = 0; i < len; i++) {
            if (url == trList.eq(i).children().eq(1).find("div").text()) {
                $('#urlFilterAllow').focus();
                return (_("This website is used. Please try another."));
            }
        }

        if (len >= 32) {
            return (_("A maximum of %s entries can be added.", [32]));
        }
        return;
    }
    /*******handle URL end*****/

    /**
     * [restrictionModule 接入限制]
     * @type {RestrictionModule}
     */
    var restrictionModule = new RestrictionModule();
    pageModule.modules.push(restrictionModule);

    function RestrictionModule() {
        this.moduleName = "parentAccessCtrl";
        this.init = function() {
            this.initHtml();
            this.initEvent();
        };

        this.initHtml = function() {
            var hourStr = "",
                minStr = "",
                i = 0;
            for (i = 0; i < 24; i++) {
                hourStr += "<option value='" + ("100" + i).slice(-2) + "'>" + ("100" + i).slice(-2) + "</option>";
            }

            $("#startHour, #endHour").html(hourStr);

            for (i = 0; i < 60; i++) {
                if (i % 5 === 0) {
                    minStr += "<option value='" + ("100" + i).slice(-2) + "'>" + ("100" + i).slice(-2) + "</option>";
                }
            }
            $("#startMin, #endMin").html(minStr);
        };

        this.initEvent = function() {
            $("[id^=day]").on("click", clickTimeDay);
            $("#addUrl").on("click", addUrlList);
            // $("#urlList").delegate(".ico", "click", deUrlList);
            $("#urlList").delegate(".deleteUrl", "click", deUrlList);
            $("#parentCtrlURLFilterMode").on("change", changeUrlMode);
            $("#onlineList").delegate(".device-name", "keyup", function() {
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

        /**
         *
         * @param  {Object} obj [初始化接入限制的参数]
         */
        this.initValue = function(obj) {
            //值重置
            $("#urlFilterAllow").val("");
            $("#urlFilterAllow").addPlaceholder(_("Enter website"));

            translateDate(obj.parentCtrlOnlineDate);

            oldDate = obj.parentCtrlOnlineDate;
            var time = obj.parentCtrlOnlineTime.split("-");
            $("#startHour").val(time[0].split(":")[0]);
            $("#startMin").val(time[0].split(":")[1]);
            $("#endHour").val(time[1].split(":")[0]);
            $("#endMin").val(time[1].split(":")[1]);

            $("#parentCtrlURLFilterMode").val(obj.parentCtrlURLFilterMode);
            createUrlList(obj.parentCtrlURL); //生成URLlist
            changeUrlMode();
        };
        this.checkData = function() {
            var date = getScheduleDate();
            if (date == "00000000") {
                return _("Select at least one day.");
            }
            return;
        };
        this.getSubmitData = function() {
            var time = time = $("#startHour").val() + ":" + $("#startMin").val() + "-" +
                $("#endHour").val() + ":" + $("#endMin").val();

            var data = {
                module2: this.moduleName,
                parentCtrlOnlineTime: time,
                parentCtrlOnlineDate: getScheduleDate(),
                parentCtrlURLFilterMode: $("#parentCtrlURLFilterMode").val(),
                urlList: getUrlListData()
            }

            return objToString(data);
        };

        /*********获取定时重启日期字符串***********/
        function getScheduleDate() {
            var i = 0,
                len = 8,
                str = "";
            for (i = 0; i < len; i++) {
                if ($("#day" + i)[0].checked) {
                    str += "1";
                } else {
                    str += "0";
                }
            }
            return str;
        }

        var oldDate; /******保存初始化日期******/

        /**********点击everyday**********/
        function clickTimeDay() {
            var dataStr = getScheduleDate();
            if (this.id == "day0") { //点击everyday
                if (this.checked) {
                    translateDate("11111111");
                } else {
                    translateDate("00000000");
                }
            } else {
                if (dataStr.slice(1) == "1111111") {
                    translateDate("11111111");
                } else {
                    translateDate("0" + dataStr.slice(1));
                }
            }
        }

        /*******根据字符串改变日期的选择*******/
        function translateDate(str) {
            var dayArry = str.split(""),
                len = dayArry.length,
                i = 0;
            for (i = 0; i < len; i++) {
                $("#day" + i)[0].checked = dayArry[i] == 1;
            }
        }


        function changeUrlMode() {
            var urlMode = $("#parentCtrlURLFilterMode").val();
            if (urlMode != "disable") {
                $("#urlFilterWrap").show();

                if (urlMode == "permit") {
                    $("#websiteLabel").html(_("Unblocked Websites"));
                } else {
                    $("#websiteLabel").html(_("Blocked Websites"));
                }
            } else {
                $("#urlFilterWrap").hide();
            }

            mainLogic.initModuleHeight();
        }

        function addUrlList() {
            var url = $('#urlFilterAllow').val(),
                len = $("#urlList").children().length,
                i = 0;
            var msg = CheckUrlVolidate();

            if (msg) {
                top.mainLogic.showModuleMsg(msg);
                return;
            }

            var str = "";
            str += "<tr>";
            str += "<td align='center'>" + (len + 1) + "</td>";
            str += "<td><div class='span-fixed' style='width:200px;' title='" + url + "'>" + url + "</div></td>";
            // str += '<td align="center"><div class="ico icon-minus-circled text-primary"></div></td>';
            str += '<td align="center"><span class="picture pic-del deleteUrl"></span></td>';
            $("#urlList").append(str);
            $('#urlFilterAllow').val('');
            top.mainLogic.initModuleHeight();

        }

        function deUrlList() { //删除URLlist
            var nextTr = $(this).parent().parent().nextAll(), //待删条目之后的所有条目
                len = nextTr.length; //待删条目之后的条目总数


            for (var i = 0; i < len; i++) {
                nextTr[i].children[0].innerHTML = parseInt(parseInt(nextTr[i].children[0].innerHTML)) - 1;
            }

            $(this).parent().parent().remove();
            top.mainLogic.initModuleHeight();
        }

        function getUrlListData() { //获取URL提交数据
            var str = "",
                i = 0,
                listArry = $("#urlList").children(),
                len = listArry.length;

            var urlInputVal = $('#urlFilterAllow').val();

            for (i = 0; i < len; i++) {
                str += $(listArry).eq(i).children().eq(1).find("div").text() + "\n";
            }
            str = str.replace(/[\n]$/, "");

            // var msg = CheckUrlVolidate();

            // if (!msg) {
            if (urlInputVal !== "") {
                if (str != "") {
                    str += "\n" + $('#urlFilterAllow').val();
                } else {
                    str += $('#urlFilterAllow').val();
                }
                // $('#urlFilterAllow').val('');
            // } else {
                // $('#urlFilterAllow')[0].blur();
            }

            return str;
        }

        function createUrlList(arry) { //生成URL 列表
            var i = 0,
                len = arry.length,
                str = "";
            for (i = 0; i < len; i++) {
                str += "<tr>";
                str += "<td align='center'>" + (i + 1) + "</td>";
                str += "<td><div class='span-fixed' style='width:200px;' title='" + arry[i] + "'>" + arry[i] + "</div></td>";
                // str += '<td align="center"><div class="ico icon-minus-circled text-primary"></div></td>';
                str += '<td align="center"><span class="picture pic-del deleteUrl"></span></td>';
            }
            $("#urlList").html(str);
        }
    }
    /**************EDN Access Restrictions***************************/
})