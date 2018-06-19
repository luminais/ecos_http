<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>Tenda 11N Wireless Router</title>
<link rel="stylesheet" href="css/screen.css" />
<script src="lang/b28n.js"></script>
</head>
<body>
<form name="frmSetup" method="POST" action="/goform/SetPowerControl">
  <fieldset>
    <legend>Wireless Signal</legend>
    <div class ="control-group">
      <label class="control-label">Signal Strength Mode</label>
      <div class="controls">
        <label class="radio" for="mode-common">
          <input type="radio" id="mode-common" name="signal-mode" value="middle" />
          Common Mode</label>
        <label class="radio" for="mode-high">
          <input type="radio" id="mode-high" name="signal-mode" value="high" />
          Enhanced Mode</label>
      </div>
    </div>
  </fieldset>
  <div class="btn-group">
    <input type="button" class="btn" value="OK" id="submit" />
    <input type="button" class="btn last" value="Cancel" id="cancel" />
  </div>
</form>
<script>
	var signal = 'middle';
</script> 
<script src="js/libs/reasy-1.0.0.js"></script>
<script src="js/wireless_signal.js"></script>
</body>
</html>
