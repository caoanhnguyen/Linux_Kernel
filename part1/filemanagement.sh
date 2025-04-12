
# Function to display the menu
show_menu() {
    echo "Menu:"
    echo "-------------------------"
    echo "1. List files"
    echo "2. Create file"
    echo "3. Remove file"
    echo "4. Show file"
    echo "5. Edit file"
    echo "6. Rename file"
    echo "7. Copy file"
    echo "-------------------------"
    echo "0. Exit"
    echo "-------------------------"
}

# Function to get user input
get_input() {
    echo "Enter your choice:"
    read choice
}

# Loop to display the menu and get user input
while true
do
    show_menu
    get_input

    case $choice in
        1) command="list" ;;
        2) command="create-file" ;;
        3) command="remove-file" ;;
        4) command="show-file" ;;
        5) command="edit-file" ;;
        6) command="rename-file" ;;
        7) command="copy-file" ;;
        0) exit ;;
        *) echo "Invalid choice" ;;
    esac

    # Execute the command based on user choice
    if [ -n "$command" ]
    then
        if [ "$command" = "list" ]
        then
            ls -la
        else
            echo "Enter file name:"
            read filename

            if [ -z "$filename" ]
            then
                echo "Missing file name"
            else
                case "$command" in
                    "create-file") touch "$filename" ;;
                    "remove-file") rm -rf "$filename" ;;
                    "show-file") less "$filename" ;;
                    "edit-file") vim "$filename" ;;
                    "rename-file")
                        echo "Enter new file name:"
                        read new_filename
                        if [ -z "$new_filename" ]
                        then
                            echo "Missing new file name"
                        else
                            mv "$filename" "$new_filename"
                        fi
                        ;;
                    "copy-file")
                        echo "Enter destination file name:"
                        read destination_filename
                        if [ -z "$destination_filename" ]
                        then
                            echo "Missing destination file name"
                        else
                            cp "$filename" "$destination_filename"
                        fi
                        ;;
                esac
            fi
            command=""
        fi
    fi
done
