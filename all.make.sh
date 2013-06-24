C_FILES=$(python ./files.py)
C_FLAGS="$C_FLAGS -g -Wall -Wextra -Werror"
gcc $C_FLAGS $C_FILES -o logcat
