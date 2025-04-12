#!/bin/sh
 command=$1
 if [ -z $command ]
     then echo 'Chua nhap chuc nang'
 fi
 if [ $command = "install" ]
     then
         if [ `whoami` != "root" ]
             then
             echo 'Ban can phai chay lenh sudo de thay doi he thong'
             exit $E_NOTROOT
         fi
         cat input.txt | xargs sudo apt-get -y install
 fi
 if [ $command = "uninstall" ]
     then
         if [ `whoami` != "root" ]
             then
             echo 'Ban can phai chay lenh sudo de thay doi he thong'
             exit $E_NOTROOT
         fi
         cat input.txt | xargs sudo apt-get -y purge
 fi
