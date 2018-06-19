<!DOCTYPE html>
<html style="overflow-y: scroll;">
<head>
<meta charset="utf-8">
<title>Tenda</title>
<link rel="stylesheet" type="text/css" href="css/reasy-1.0.0.css">
<script src="lang/b28n.js"></script>
<script>
var lanip = "<%aspTendaGetStatus("lan","lanip");%>";

var progressStrip = (function(window) {
    function ProgressStrip(url, reboot_time, reboot_msg,  up_time, up_msg) {
        var my_obj = this;
        this.url = url || '';
        this.reboot_timer = null;
        this.reboot_time = reboot_time + 60 || 360;
        this.reboot_msg = reboot_msg || _("Please wait: Device rebooting...");

        this.up_timer = null;
        this.up_msg = up_msg ||
                      _("Upgrading. DO NOT POWER OFF THE DEVICE!");
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
                bodyElem = document.getElementsByTagName('body')[0],
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
                bodyElem.appendChild(gbxOverlay);
            }
            if (!loadContentElem) {
                loadContentElem = document.createElement('div');
                loadContentElem.id = 'loading_div';
                bodyElem.appendChild(loadContentElem);
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

            left = (pageWidth() - loadContentElem.offsetWidth) / 2,
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
    }
}(window));

function init() {
    if(location.href.indexOf('122') != '-1') {
        alert(_('Error! The router will reboot automatically.'))
    }
    var url = "http://" + lanip;
    progressStrip(url, 170, _('Rebooting…please wait…'));
}

</script>
</head>
<body onLoad="init();">
<div class="masthead">
  <div class="container head-inner"> <a class="brand" href="#" title="Tenda"></a>
    <div class="easy-logo"></div>
  </div>
  <div class="footer"></div>
</div>
</body>
</html>