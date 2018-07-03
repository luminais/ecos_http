define(function(require, exports, module) {
    var pageModule = new PageLogic({
        getUrl: "goform/getPowerSave",
        modules: "LEDControl,wifiTime,wifiBasicCfg",
        setUrl: "goform/setPowerSave"
    });
    pageModule.modules = [];
    module.exports = pageModule;

    /**
     * [LEDControlModule LED灯控制模块]
     */
    var ledControlModule = new LEDControlModule();
    pageModule.modules.push(ledControlModule);

    function LEDControlModule() {
        this.moduleName = "LEDControl";
        var that = this;
        this.init = function() {
            this.initEvent();
        }
        this.initEvent = function() {
            $("input[name=LED-control]").on("click", changeSchedule);
        };
        this.initValue = function(obj) {
            $("input[name=LED-control]")[obj.LEDStatus].checked = true;
            var initObj = {};
            if (obj.LEDCloseTime != "") {
                initObj = {
                    "startTimeHour": obj.LEDCloseTime.split("-")[0].split(":")[0],
                    "startTimeMin": obj.LEDCloseTime.split("-")[0].split(":")[1],
                    "endTimeHour": obj.LEDCloseTime.split("-")[1].split(":")[0],
                    "endTimeMin": obj.LEDCloseTime.split("-")[1].split(":")[1]
                };
            } else {
                initObj = {
                    "startTimeHour": "0",
                    "startTimeMin": "0",
                    "endTimeHour": "7",
                    "endTimeMin": "0"
                };
            }

            for (var key in initObj) {
                $("#" + key).val(initObj[key]);
            }

            changeSchedule();
        };

        this.checkData = function() {
            //起始的时间和结束时间不能相同
            if ($("[name='LED-control']")[2].checked) {
                if ($("#startTimeHour").val() + ":" + $("#startTimeMin").val() == $("#endTimeHour").val() + ":" + $("#endTimeMin").val()) {
                    $("#startTimeHour").focus();
                    return _("The end time and start time cannot be the same.");
                }
            }
            return;
        };

        this.getSubmitData = function() {
            var startTimeHour = $("#startTimeHour").val(),
                startTimeMin = $("#startTimeMin").val(),
                endTimeHour = $("#endTimeHour").val(),
                endTimeMin = $("#endTimeMin").val();
            var subObj = {
                "module1": "LEDControl",
                "LEDStatus": "",
                "LEDCloseTime": ""
            }
            for (var i = 0; i < 3; i++) {
                if ($("input[name=LED-control]")[i].checked == true) {
                    subObj.LEDStatus = i;
                }
            }
            if (subObj.LEDStatus == "2") {
                subObj.LEDCloseTime = startTimeHour + ":" + startTimeMin + "-" + endTimeHour + ":" + endTimeMin;
            }
            return objToString(subObj);
        };

        function changeSchedule() {
            if ($("input[name=LED-control]")[2].checked == true) {
                $("#open-regularly").removeClass("none");
            } else {
                $("#open-regularly").addClass("none");
            }
        }

    }
    /*************END LED Control Module************************/



    /*
     * 无线定时开关模块，显示无线时间设置
     * @method wifiTimeModule
     * @param {Object} wifiTime 从后台获取的关于无线时间信息
     * @return {无}
     */

    var wifiTimeModule = new WifiTimeModule();
    pageModule.modules.push(wifiTimeModule);

    function WifiTimeModule() {
        var oldDate; /******保存初始化日期******/
        this.moduleName = "wifiTime";

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
            $("input[name='wifiTimeEn']").on("click", changeWifiTimeEn);
            $("[id^=day]").on("click", clickTimeDay);
        };

        /**
         * [initValue 用于初始化显示的数据]
         * @param  {object} obj [wifiTime]
         */
        this.initValue = function(obj) {
            //仅在无线功能开启，且工作模式为路由模式或AP模式时，才显示无线定时开关
            var wifiEn = pageModule.data.wifiBasicCfg;
            if((wifiEn.wifiEn == "true" || wifiEn.wifiEn_5G == "true") && (obj.wifiRelayType == "disabled" || obj.wifiRelayType == "ap")) {
                $("#wifiScheduleWrap").removeClass("none");
            }else{
                return;
            }

            top.mainLogic.initModuleHeight();

            inputValue(obj);
            translateDate(obj.wifiTimeDate);
            oldDate = obj.wifiTimeDate;
            var time = obj.wifiTimeClose.split("-");
            $("#startHour").val(time[0].split(":")[0]);
            $("#startMin").val(time[0].split(":")[1]);
            $("#endHour").val(time[1].split(":")[0]);
            $("#endMin").val(time[1].split(":")[1]);
            changeWifiTimeEn();
        };
        this.checkData = function() {
            var wifiEn = pageModule.data.wifiBasicCfg;
            //在无线功能关闭，或者工作模式为wisp模式或apclient模式时，不需要传该模块值;
            if ((wifiEn.wifiEn == "false" && wifiEn.wifiEn_5G == "false") || (pageModule.data.wifiTime.wifiRelayType == "wisp" || pageModule.data.wifiTime.wifiRelayType == "client+ap")) {
                return;
            }

            if ($("[name='wifiTimeEn']")[0].checked) {
                var date = getScheduleDate();
                if (date == "00000000") {
                    return _("Select at least one day.");
                }

                if ($("#startHour").val() + ":" + $("#startMin").val() == $("#endHour").val() + ":" + $("#endMin").val()) {
                    $("#startHour").focus();
                    return _("The end time and start time cannot be the same.");
                }
            }
            return;
        };
        this.getSubmitData = function() {
            var wifiEn = pageModule.data.wifiBasicCfg;
            //在无线功能关闭，或者工作模式为wisp模式或apclient模式时，不需要传该模块值;
            if ((wifiEn.wifiEn == "false" && wifiEn.wifiEn_5G == "false") || (pageModule.data.wifiTime.wifiRelayType == "wisp" || pageModule.data.wifiTime.wifiRelayType == "client+ap")) {
                return "";
            }

            var time = $("#startHour").val() + ":" + $("#startMin").val() + "-" +
                $("#endHour").val() + ":" + $("#endMin").val();
            var data = {
                module2: this.moduleName,
                wifiTimeEn: $("input[name='wifiTimeEn']:checked").val() || "false",
                wifiTimeClose: time,
                wifiTimeDate: getScheduleDate()
            };
            return objToString(data);
        };

        /*******启用或禁用定时开关********/
        function changeWifiTimeEn() {
            if ($("input[name='wifiTimeEn']")[0].checked) {
                $("#tendaApp,#wifiScheduleCfg").show();
            } else {
                $("#tendaApp,#wifiScheduleCfg").hide();
            }
            top.mainLogic.initModuleHeight();
        }
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

    }
    /*************END WiFi Schedule *****************************/



})