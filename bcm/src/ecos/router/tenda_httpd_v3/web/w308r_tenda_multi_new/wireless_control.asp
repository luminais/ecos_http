<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>Tenda 11N Wireless Router</title>
<link rel="stylesheet" href="css/screen.css" />
<script src="lang/b28n.js"></script>
</head>
<body>
<fieldset>
  <legend>Wireless Schedule</legend>
  <div class="control-group">
    <label class="control-label">Wireless Schedule</label>
    <div class="controls">
      <label class="radio" for="disableCtrl">
        <input type="radio" id="disableCtrl" name="switcher" value="0" />
        Disable</label>
      <label class="radio" for="enableCtrl">
        <input type="radio" id="enableCtrl" name="switcher" value="1" />
        Enable </label>
    </div>
  </div>
  <div id="wifi-control" class="none">
    <div class="control-group">
      <label class="control-label">Disable WiFi from</label>
      <div class="controls">
        <div class="clearfix">
          <input id="stime-1" class="input-mic-mini timebox time-hour" required type="text" validchars="0123456789" maxlength="2">
          <span class="text-bridge">:</span>
          <input id="stime-2" class="input-mic-mini timebox time-minute" required type="text" validchars="0123456789" maxlength="2">
          <span class="text-bridge">to</span>
          <input id="etime-1" class="input-mic-mini timebox time-hour" required type="text" validchars="0123456789" maxlength="2">
          <span class="text-bridge">:</span>
          <input id="etime-2" class="input-mic-mini timebox time-minute" required type="text" validchars="0123456789" maxlength="2">
        </div>
      </div>
    </div>
    <div class="controls-group">
      <label class="control-label">Repeat</label>
      <div class="controls">
        <label class="radio">
          <input type="radio" name="repeat-time" id="everyday" value="all">
          Every day</label>
        <label class="radio">
          <input type="radio" name="repeat-time" id="appointday" value="appoint">
          The specified date</label>
      </div>
      <div class="controls">
        <label class="checkbox">
          <input type="checkbox" name="week" value="1">
          Sunday</label>
        <label class="checkbox">
          <input type="checkbox" name="week" value="2">
          Monday</label>
        <label class="checkbox">
          <input type="checkbox" name="week" value="3">
          Tuesday</label>
        <label class="checkbox">
          <input type="checkbox" name="week" value="4">
          Wednesday</label>
        <label class="checkbox">
          <input type="checkbox" name="week" value="5">
          Thursday</label>
        <label class="checkbox">
          <input type="checkbox" name="week" value="6">
          Friday</label>
        <label class="checkbox">
          <input type="checkbox" name="week" value="7">
          Saturday</label>
      </div>
    </div>
  </div>
  <div class="btn-group">
    <input type="button" class="btn" value="OK" id="submit" />
    <input type="button" class="btn last" value="Cancel" id="cancle" />
  </div>
</fieldset>
<script src="js/libs/reasy-1.0.0.js"></script>
<script src="js/libs/reasy-ui-1.0.0.js"></script>
<script src="js/wireless_control.js"></script>
</body>
</html>
