cat /proc/"$(ps -w | grep -v grep | grep systemd | awk '{print $1}')"/status | grep RSS
