##!/bin/sh
#
#command=$1
#
#if [ -z $command ]
#    then echo "Chua nhap chuc nang"
#fi
#
#
#if [ $command = "list" ]
#    then
#        crontab -l
#fi
#
#if [ $command = "create" ]
#    then
#        echo "Xin hay nhap doan lenh ban muon lap lich"
#        read order
#        echo "Gio ban se nhap gio va phut de chay."
#        echo "Hay nhap gio"
#        read hour
#        echo "Hay nhap phut"
#        read min
#        echo "Gio ban can nhap ngay tac vu chay hang thang."
#        echo "Neu ban muon chay tac vu hang ngay, hay nhap *"
#        echo "Neu ban muon chon ngay rieng le, hay nhap cac ngay ban mong muon, phan cach boi dau \",\""
#        read day
#        echo "Gio ban can nhap thang tac vu ma tac vu chay."
#        echo "Neu ban muon tac vu chay hang thang, hay nhap *"
#        echo "Neu ban muon chon thang rieng le, hay nhap cac thang ban mong muon, phan cach boi dau \",\""
#        read month
#        echo "Gio ban can nhap ngay tac vu chay hang tuan."
#        echo "Neu ban muon chay tac vu hang ngay, hay nhap *"
#        echo "Neu ban muon chon thu rieng le, hay nhap cac thu ban mong muon, phan cach boi dau \",\""
#        echo "Xin nho rang gia tri 0 la Chu nhat"
#        read dow
#        echo "Ban co chac la ban muon thuc hien thao tac nay khong?"
#        read confirmation
#        if [ $confirmation = 'Y' ]
#                then
#                        echo "$min $hour $day $month $dow $order" >> "input.txt"
#                        crontab -l -u $LOGNAME | cat - "input.txt" | crontab -u $LOGNAME -
##                        rm -f "input.txt"
#        else echo "Huy bo\n"
#        fi
#fi
#
#if [ $command = "remove-all" ]
#    then
#        crontab -r
#fi
#
#if [ $command = "edit" ]
#    then
#        crontab -e
#fi
#
#!/bin/bash

# Liet ke tat ca cac tac vu
function list_tasks {
  crontab -l
}

# Tao mot tac vu moi
function create_task {
  read -p "Nhap vao cau lenh muon thuc hien: " command
  read -p "Nhap vao thoi gian lich trinh (theo dinh dang * * * * *): " schedule
  (crontab -l ; echo "$schedule $command") | crontab -
}

# Sua mot tac vu
function edit_task {
  read -p "Nhap vao so thu tu cua tac vu muon sua: " task_number
  read -p "Nhap vao cau lenh muon thuc hien: " command
  read -p "Nhap vao thoi gian lich trinh (theo dinh dang * * * * *): " schedule
  (crontab -l | sed -e "${task_number}s/.*/$schedule $command/") | crontab -
}

# Xoa mot tac vu
function delete_task {
  read -p "Nhap vao so thu tu cua tac vu muon xoa: " task_number
  (crontab -l | sed -e "${task_number}d") | crontab -
}

# Menu chuc nang
while true
do
  echo "1. Liet ke tat ca cac tac vu"
  echo "2. Tao mot tac vu moi"
  echo "3. Sua mot tac vu"
  echo "4. Xoa mot tac vu"
  echo "5. Thoat"
  read -p "Nhap vao lua chon cua ban: " choice

  case $choice in
    1)
      list_tasks
      ;;
    2)
      create_task
      ;;
    3)
      edit_task
      ;;
    4)
      delete_task
      ;;
    5)
      break
      ;;
    *)
      echo "Lua chon khong hop le"
      ;;
  esac
done
