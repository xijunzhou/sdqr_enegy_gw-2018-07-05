#! /bin/sh
### BEGIN INIT INFO
# Provides:          gateway 
# Required-Start:    $local_fs $network
# Required-Stop:     $local_fs
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: gateway
### END INIT INFO

#/home/john/gateway/gateway > /var/meter/gateway.log 2>&1 &
#/home/john/monitor/monitor > /var/meter/monitor.log 2>&1 &


mypath="/home/john"


if [ ! -f "$mypath/new_*" ]; then
cp $mypath/new_* $mypath/gateway
chmod a+x $mypath/gateway
rm $mypath/new_*
echo "success"
fi

$mypath/gateway > /var/meter/gateway.log 2>&1 &
$mypath/daemon > /var/meter/daemon.log 2>&1 &

exit 0
