
## use on 8 of total 32MB flash for fast boot after upgrade firmware (2016/4/22)

# df print on 32MB configuration
root@mylinkit:/# df -h                                                          
Filesystem                Size      Used Available Use% Mounted on              
rootfs                   26.8M    844.0K     26.0M   3% /                       
/dev/root                 3.8M      3.8M         0 100% /rom                    
tmpfs                    61.7M     80.0K     61.7M   0% /tmp                    
/dev/mtdblock6           26.8M    844.0K     26.0M   3% /overlay                
overlayfs:/overlay       26.8M    844.0K     26.0M   3% /                       
tmpfs                   512.0K         0    512.0K   0% /dev                    
root@mylinkit:/# 

# df print on 16MB configuration
root@mylinkit:/# df -h                                                          
Filesystem                Size      Used Available Use% Mounted on              
rootfs                   11.6M    472.0K     11.1M   4% /                       
/dev/root                 3.0M      3.0M         0 100% /rom                    
tmpfs                    61.7M     72.0K     61.7M   0% /tmp                    
/dev/mtdblock6           11.6M    472.0K     11.1M   4% /overlay                
overlayfs:/overlay       11.6M    472.0K     11.1M   4% /                       
tmpfs                   512.0K         0    512.0K   0% /dev                    
root@mylinkit:/# 

# df print on 8 MB configuration
root@mylinkit:/# df -h
Filesystem                Size      Used Available Use% Mounted on
rootfs                    3.6M    340.0K      3.2M   9% /
/dev/root                 3.0M      3.0M         0 100% /rom
tmpfs                    61.7M     72.0K     61.7M   0% /tmp
/dev/mtdblock6            3.6M    340.0K      3.2M   9% /overlay
overlayfs:/overlay        3.6M    340.0K      3.2M   9% /
tmpfs                   512.0K         0    512.0K   0% /dev
root@mylinkit:/# 
