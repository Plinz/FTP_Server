
kill -9 ps -ef | grep FTPserver | grep -v grep | awk '{print $2}'
