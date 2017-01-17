#!/bin/bash
RED='\033[01;31m'
RESET='\033[0m'
GREEN='\033[01;32m'
_version=`cat ./version.txt`
clear
echo -e "$GREEN*************************************************************$RESET"
echo -e "          Sona  Installation Wizard  Version $GREEN $_version $RESET"
echo -e "       Copyright (c) 2015 https://www.facebook.com/phatsovathana"
echo -e "   					Software Development Services  "
echo -e "$GREEN*************************************************************$RESET"
echo " "
echo " "
export who=`whoami`
lpid=$$
echo "">/var/log/ffmpeginstall.$_version.log.$lpid
echo " All operations are loged to /var/log/ffmpeginstaller.$_version.log.$lpid "
echo -e "$RED"
echo "*************************************************************************"
echo -e "$RESET"
#Don't edit or move the following credits without permissin
echo -e "\033[1mLooking for a developer ?$RESET"
echo ""
echo -e "Skype  me @ \033[1m \033[4m ph.sovathana$RESET   for more infomation."
echo ""
echo "Thank you !!!"
echo ""
echo -e "$RED"
echo "**************************************************************************"
echo -e "$RESET"
echo  "If you need to cancell  the installation press Ctrl+C  ...."
echo -n  "Press ENTER to start the installation  ...."
read option
if [[ $who == "root" ]];then
       sh start.sh | tee /var/log/ffmpeginstaller.$_version.log.$lpid
else
        echo "                  Sorry  Buddy, you are not a root user. This is not for yours."
        echo "                  You need admin privilege for installing this applications"
fi
