# thanks
the effort is to migrate exist working solution from: https://github.com/xiongyihui/LinkIt_Smart_7688
it seems only mmap mode works in 7688 openwrt alsa driver, need future work on driver level but now mmap mode can do application and testing

# debugging

## to fix shairport startup issue

`
root@mylinkit:/etc/init.d# shairport-sync 
startup
recv setsockopt(IP_ADD_MEMBERSHIP): No such device
unable to create recv socket
tinysvcmdns: mdnsd_start() failed
Could not establish mDNS advertisement!
`

## temporary solution
`
route add -net 224.0.0.0 netmask 224.0.0.0 ra0
`

## reference
http://stackoverflow.com/questions/3187919/error-no-such-device-in-call-setsockopt

