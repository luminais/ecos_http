<!DOCTYPE html>
<html lang="en" class="index-html">
<head>
<meta charset="utf-8">
<title>TENDA 11N Wireless Router</title>
<script src="lang/b28n.js"></script>
<link href="css/reasy-1.0.0.css" rel="stylesheet">
<script>
function isFileType(name, types) {
    var fileTypeArr = name.split('.'),
        fileType = fileTypeArr[fileTypeArr.length - 1],
        len = types.length,
        i;

    for (i = 0; i < len; i++) {
        if (fileType === types[i]) {
            return true;
        }
    }

    return false;
}

function goUpgrade() {

  var upgradefile = document.getElementById('upgradeFile').value,
      upform = document.upgradefrm;

  if (upgradefile == null || upgradefile == "") {
    return;

  // 判断文件类型
  } //else {//if (!isFileType(upgradefile, ['bin', 'trx'])) {
   // alert(_('Please select the file with a suffix: 'trx' or 'bin'.'));
  //  upform.reset();
 //   return;
//  }

  if(confirm(_('Are you sure to upgrade?'))){
    upform.submit();
    parent.showUpgrading();
  } else {
    upform.reset();
  }
}
</script>
</head>
<body>
<form name="upgradefrm" method="POST" action="/cgi-bin/upgrade" enctype="multipart/form-data">
  <div class="control-group">
    <label class="control-label">Firmware Upgrade</label>
    <div class="controls" style="position: relative;">
      <input type="file" class="fileUpLoadBtn" name="upgradeFile" id="upgradeFile"  onchange="goUpgrade()"/>
      <input type="button" name="upgrade" id="upgrade"  class="btn" value="Browse…" >
      <span class="help-inline softVer" id="softVer"><span>Current version:</span> 
      <%aspTendaGetStatus("sys","sysver");%>
      </span> </div>
  </div>
</form>
</body>
</html>