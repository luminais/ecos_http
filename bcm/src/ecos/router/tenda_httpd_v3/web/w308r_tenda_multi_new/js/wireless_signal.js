(function () {
	var G = {};
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
	
	function initView() {
		setValueByName('signal-mode', G.data['signal-mode']);
	}
	
	function initControl() {
		$('#submit').on('click', function(req) {
			var data = "signal-mode=" + getValueByName('signal-mode');
			
			$.post('/goform/SetPowerControl?' + Math.random(), data, function (req){
				window.location.reload();
			});
		});
		$('#cancel').on('click',function() {
			window.location.reload();
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