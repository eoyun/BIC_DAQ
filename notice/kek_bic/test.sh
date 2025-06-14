#!/bin/bash

if [ "$#" -ne 2 ]; then
	echo fail
	exit 1
fi



export TEST=$1

if [[ "$TEST" =~ ^[0-9]+$ ]]; then
	echo number 
	exit 1
if

echo $TEST

