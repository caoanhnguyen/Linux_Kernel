#!/bin/bash

while true
do
    echo "========================="
    echo "MENU"
    echo "1. Show current time"
    echo "2. Set hour"
    echo "3. Set date"
    echo "4. Automatic time update"
    echo "5. Exit"
    echo "========================="
    read choice

    case $choice in
        1) TZ="Asia/Ho_Chi_Minh" date;;
        2)
            if [ `whoami` != "root" ]
            then
                echo 'You need to run this command as root to set the time.'
            else
                echo "Enter the time in the format hh:mm:ss:"
                read new_time
                date -s "${new_time}"
            fi;;
        3)
            if [ `whoami` != "root" ]
            then
                echo 'You need to run this command as root to set the date.'
            else
                echo "Enter the date in the format yyyy-mm-dd:"
                read new_time
                date  -s "${new_time}"
            fi;;
        4)
            if [ `whoami` != "root" ]
            then
                echo 'You need to run this command as root to update the time.'
            else
                echo "Updating time from Microsoft server..."
                sntp -sS time.windows.com
            fi;;
        5) echo "Exiting..."; exit;;
        *) echo "Invalid choice";;
    esac
done
