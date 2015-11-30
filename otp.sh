#!/bin/bash
#################################################################################
#CMPT 361 - Assignment 3                                                         
#Group 4: Nick, John, Alex, Kevin
#November 6th, 2015
#Filename: otp.sh 
#Description: Computes OTP and saves it in otp folder with filename of md5 digest  
##################################################################################

# NOTE: In bash sucess is a return 1 and failure is return 0.  Consistency :(

# Put all the sizes in a list.
SIZES="$@"

# Check if no arguments given
if [[ $# -eq 0 ]]; then
	echo "$0 ERROR: Please list sizes (in KiBs) of otps to generate." 
	echo "USAGE: $0 kilobytes..."
	exit 0
fi

# Check each of the arguments given and make sure they are numeric.
# check if file sizes are way too big.
for i in $SIZES; do
	echo "$i" | grep -E "^[0-9]+$" > /dev/null
# Reference: http://stackoverflow.com/questions/806906/how-do-i-test-if-a-variable-is-a-number-in-bash
	if [[ $? -eq 0 ]]; then 
		count=$1
	else 
		echo "$0 ERROR: An argument is not numeric." 
		exit 0
	fi

# Reference: http://stackoverflow.com/questions/1885525/how-do-i-prompt-a-user-for-confirmation-in-bash-script
	if [[ $i -ge 1048576 ]]; then
		GB=$((i / 1048576))
		read -p "$0 WARNING: You will be generating an OTP larger than 1GB (Approx: $GB GiB)  Are you okay with this?" -n 1 -r
		echo # as will take input before hitting return we need to make a new line.
		if [[ $REPLY =~ ^[Nn]$ ]]; then
			exit 0
		fi
	fi
done

# Create an otp folder if one does not exist.
# Ref: http://stackoverflow.com/questions/59838/check-if-a-directory-exists-in-a-shell-script
if [ ! -d "otp" ]; then
    mkdir otp;
fi

# Generate an OTP for each provided 
for i in $SIZES; do
	dd if=/dev/urandom of=gen.dat bs=1K count=$i status=none
	filename=$(cat gen.dat | md5sum)
	filename=${filename:0:32}
	$(mv gen.dat otp/$filename.dat)
	echo "Created: otp/$filename.dat"
done

# We are done.
exit 1
