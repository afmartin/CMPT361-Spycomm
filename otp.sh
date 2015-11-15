#!/bin/bash
#################################################################################
#CMPT 361 - Assignment 3                                                         
#Group 4: Nick, John, Alex, Kevin
#November 6th, 2015
#Filename: otp.sh 
#Description: Computes OTP and saves it in otp folder with filename of md5 digest  
##################################################################################
if [[ $# -eq 0 ]]; then
	echo "$0 HINT: If you want to generate multiple OTP's provide the count as an argument. Ex: $0 5"
	count=1;
else
	echo "$1" | grep -E "^[0-9]+$" > /dev/null
	if [[ $? -eq 0 ]]; then # Reference: http://stackoverflow.com/questions/806906/how-do-i-test-if-a-variable-is-a-number-in-bash
		count=$1
	else 
		echo "$0 ERROR: Please enter a number.  Exited"
		exit 0
	fi
fi
counter=0

# Ref: http://stackoverflow.com/questions/59838/check-if-a-directory-exists-in-a-shell-script
if [ ! -d "otp" ]; then
    mkdir otp;
fi

while [ $counter -lt  $count ]; do 
	dd if=/dev/urandom of=gen.dat bs=1M count=10 status=none
	filename=$(cat gen.dat | md5sum)
	filename=${filename:0:32}
	$(mv gen.dat otp/$filename.dat)
	echo "Created: otp/$filename.dat"

	counter=$[$counter+1]
done
