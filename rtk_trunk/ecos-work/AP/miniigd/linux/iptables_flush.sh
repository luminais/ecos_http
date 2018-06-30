#! /bin/sh
# $Id: iptables_flush.sh,v 1.1.1.1 2007-08-06 10:04:43 root Exp $
IPTABLES=iptables

#flush all rules owned by miniupnpd
$IPTABLES -t nat -F MINIUPNPD
$IPTABLES -t filter -F MINIUPNPD

