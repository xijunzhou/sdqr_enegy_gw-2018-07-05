#! /bin/sh
### BEGIN INIT INFO
# Provides:          gateway
# Required-Start:    $local_fs $network
# Required-Stop:     $local_fs
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: gateway
### END INIT INFO

mypath="/home/root"
Day=$(date +%d)
ver=$(cat $mypath/version)
cat /var/meter/meter.log > /var/meter/gateway_log_$Day.log
cat /var/meter/daemon.log >> /var/meter/gateway_log_$Day.log
chmod a+x $mypath/meter_$ver
sleep 1
$mypath/meter_$ver > /var/meter/meter.log 2>&1 &
$mypath/daemon > /var/meter/daemon.log 2>&1 &

exit 0

