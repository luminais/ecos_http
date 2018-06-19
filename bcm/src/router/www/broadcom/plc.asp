<!--
Copyright (C) 2009, Broadcom Corporation
All Rights Reserved.

THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.

$Id: plc.asp 245433 2011-03-10 01:49:30Z rnuti $
-->

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
        "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<title>Broadcom Home Gateway Reference Design: Firmware</title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<link rel="stylesheet" type="text/css" href="style.css" media="screen" />
<script language="JavaScript" type="text/javascript" src="overlib.js"></script>
</head>

<body>
  <div id="overDiv" style=
    "position:absolute; visibility:hidden; z-index:1000;"></div>
  <table border="0" cellpadding="0" cellspacing="0" width="100%" bgcolor=
    "#cc0000">
    <% asp_list(); %>
  </table>

  <table border="0" cellpadding="0" cellspacing="0" width="100%">
    <tr>
      <td colspan="2" class="edge"><img border="0" src="blur_new.jpg" alt="" />
      </td>
    </tr>
    <tr>
      <td><img border="0" src="logo_new.gif" alt="" /></td>
      <td width="100%" valign="top">
      	<br />
      	<span class="title">PLC</span><br />
      	<span class="subtitle">This page allows you to configure the Power 
          Line Interface</span>
      </td>
    </tr>
  </table>


  <table border="0" cellpadding="0" cellspacing="0">
    <tr>
      <th width="310" onMouseOver=
        "return overlib('Displays Device MAC Address.', LEFT);" onMouseOut=
        "return nd();">
      	MAC Address:&nbsp;&nbsp;
      </th>
      <td>&nbsp;&nbsp;</td>
      <td><tt><% ggl_plc_get_mac(); %></tt></td>
    </tr>
    <tr>
      <th width="310" onMouseOver=
        "return overlib('Displays the Device Nickname.', LEFT);" onMouseOut=
        "return nd();">
        HFID:&nbsp;&nbsp;
      </th>
      <td>&nbsp;&nbsp;</td>
      <td><% ggl_plc_get_nick(); %></td>
    </tr>
    <tr>
      <th width="310" onMouseOver=
        "return overlib('Displays the Device Role.', LEFT);" onMouseOut=
        "return nd();">
        ROLE:&nbsp;&nbsp;
      </th>
      <td>&nbsp;&nbsp;</td>
      <td><% ggl_plc_get_role(); %></td>
    </tr>
    <tr>
      <th width="310" onMouseOver=
        "return overlib('Displays the Device Uptime.', LEFT);" onMouseOut=
        "return nd();">
        Uptime:&nbsp;&nbsp;
      </th>
      <td>&nbsp;&nbsp;</td>
      <td><% ggl_plc_get_uptime(); %></td>
    </tr>
  </table>

  <p></p>
  <form method="post" action="plc.cgi">
    <table border="0" cellpadding="0" cellspacing="0">
      <tr>	
        <th width="310" onMouseOver=
          "return overlib('Configure PLC Network Password', LEFT);" onMouseOut=
          "return nd();">
        	NPW:&nbsp;&nbsp;
        </th>
        <td>&nbsp;&nbsp;</td>
        <td><input name="plc_npw" value="" size="32" maxlength="64" /> (8 to 64 chars)</td>
      </tr>
      <tr>
        <th width="310" onMouseOver=
          "return overlib('Configure PLC Device Identifier', LEFT);" onMouseOut=
          "return nd();">
          HFID:&nbsp;&nbsp;
        </th>
        <td>&nbsp;&nbsp;</td>
    	  <td><input name="plc_hfid" size="32" maxlength="63" 
          value="<% ggl_plc_get_nick(); %>"/> (max. 63 chars)</td>
      </tr>
      
      <tr>
        <th width="310" onMouseOver=
          "return overlib('Configure PLC Autoconf Root', LEFT);" onMouseOut=
          "return nd();">
          Autoconfig Root&nbsp;&nbsp;
        </th>
        <td>&nbsp;&nbsp;</td>
    	  <td><input name="plc_cfg_root" type="checkbox" 
            <% ggl_plc_get_autoconf_root(); %> /></td>
      </tr>

      <tr>
        <th></th>
        <td></td>
        <td><input type="submit" value="Apply" /> <input type="reset" /></td>
      </tr>
    </table>
  </form>

  <p></p>
  <table border="0" cellpadding="0" cellspacing="0">
    <tr>	
      <th width="310" valign="top" rowspan="1"
  	onMouseOver="return overlib('Restart PLC Switch', LEFT);"
  	onMouseOut="return nd();">
  	Reset &nbsp;&nbsp;
      </th>
      <td>&nbsp;&nbsp;</td>
      <td>
        <form action="plc-restart.cgi">
          <input type="submit" value="Restart Device" />
        </form>
      </td>
    </tr>
  </table>

  <p></p>
  <table border="0" cellpadding="0" cellspacing="0">
    <tr>
      <th width="310" valign="top"
  	onMouseOver="return overlib('PLC stations in current PLC network.', LEFT);"
  	onMouseOut="return nd();">
  	PLC Stations:&nbsp;&nbsp;
      </th>
      <td>&nbsp;&nbsp;</td>
      <td>
        <table border="1" cellpadding="2" cellspacing="0">
          <tr align="center">
            <td class="label">MAC<br />
              Address</td>
            <td class="label">Tx PHY<br />
              Rate (Mbps)</td>
            <td class="label">Rx PHY<br />
              Rate (Mbps)</td>
            <td class="label">Role</td>
<!--
            /* TODO */
            <td class="label">HFID</td>
-->
          </tr>
          <% ggl_plc_get_stas(); %>
        </table>
      </td>
    </tr>
  </table>
  <p></p>
<!--
  <table border="0" cellpadding="0" cellspacing="0">
    <tr>
      <th width="310" valign="top"
  	onMouseOver="return overlib('Visible PLC Networks.', LEFT);"
  	onMouseOut="return nd();">
  	Visible PLC Networks:&nbsp;&nbsp;
      </th>
      <td>&nbsp;&nbsp;</td>
      <td>
        <table border="1" cellpadding="2" cellspacing="0">
          <tr align="center">
            <td class="label">MAC<br>Address</td>
            <td class="label">Network<br>ID</td>
            <td class="label">Number of<br>Stations</td>
          </tr>
          < % ggl_plc_get_nets(); % >
        </table>
      </td>
    </tr>
  </table>
-->
  <p>CGI version: <% ggl_get_version(); %>
  <p class="label">&#169;2001-2010 Broadcom Corporation. All rights reserved.
  </p>
</body>
</html>

