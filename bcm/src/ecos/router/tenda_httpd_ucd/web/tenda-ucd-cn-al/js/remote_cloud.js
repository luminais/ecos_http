(function (window, document, $) {
    "use strict";
    var G = {};
    function initControler() {
        $("#enableCtrl").on('click', function () {
            $("#wrapper").removeClass("none");
        });
        $("#disableCtrl").on('click', function () {
            $("#wrapper").addClass("none");
        });

        $('#submit').on('click', function () {
            var submitStr = "enable=" + getValueByName("switcher");
            if(getValueByName("switcher") !== G.data.enable) {
                var url = "/goform/SysToolReboot";
                
                if(window.confirm("云管理状态改变会重启路由器，确定重启？")) {
					$.post('/goform/SetCloud?' + Math.random(), submitStr, function (req){

						if(req.responseText.indexOf('<DOCTYPE') !== -1) {
							window.location.reload();
						}
						setTimeout(function () {
							getData(refreshData);
						}, 1000);
					});
                    $.get(url, function (){ });
                    progressStrip(location.href, 120, '设备重启中...');
                }
            } else {
                G.ajaxMsg = $.ajaxMassage('数据保存中，请稍候...');
                $.post('/goform/SetCloud?' + Math.random(), submitStr, function (req){

                    if(req.responseText.indexOf('<DOCTYPE') !== -1) {
                        window.location.reload();
                    }
                    setTimeout(function () {
                        getData(refreshData)
                        G.ajaxMsg.text("数据保存成功！");
                        setTimeout(function () {
                            G.ajaxMsg.hide();
                        }, 500);
                    }, 1000);
                });
            }
        });

        $('#cancel').on('click',function() {
            window.location.reload();
        });
    }

    function initView() {
        setValueByName('switcher', G.data.enable);

        if(+G.data["wifi-power"]) {
             $(".icons-signal").parent().removeClass("none");   
        } else {
             $(".icons-signal").parent().addClass("none");
        }
        
        if ($("#enableCtrl")[0].checked) {
            $("#wrapper").removeClass("none");
        }
    }

    // 初始化页面
    function init(data){
        G.data = data;
        initView();
        initControler();
    }
    
    // 只刷新页面显示数据
    function refreshData(data) {
        G.data = data;
        transformData();
        initView();
    }
    
    function getData(callBack) {
        
        $.getJSON("/goform/GetCloud?" + Math.random(), callBack);
    }
    
    // 文档加载完，图片未加载完时运行
    $(function() {
        getData(init);
    });
}(window, document, $));