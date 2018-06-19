(function () {
	var G = {};
	
	function initView() {
		setValueByName('signal-mode', G.data['signal-mode']);
	}
	
	function initControl() {
		$('#submit').on('click', function(req) {
			var data = "signal-mode=" + getValueByName('signal-mode');
			
			G.ajaxMsg = $.ajaxMassage(_('Saving…please wait…'));
			$.post('/goform/SetPowerControl?' + Math.random(), data, function (req){

				if(req.responseText.indexOf('<DOCTYPE') !== -1) {
					window.location.reload();
				}
				setTimeout(function () {
					G.ajaxMsg.text(_('Saved successfully!'));
					setTimeout(function () {
						G.ajaxMsg.hide();
					}, 500);
				}, 1000);
			});
		});
	}
	
	function init(obj) {
		G.data = obj;
		initView();
		initControl();
	}
	$(function () {
		$.getJSON('/goform/GetPowerControl?' + Math.random(), init);
	});
	
})();