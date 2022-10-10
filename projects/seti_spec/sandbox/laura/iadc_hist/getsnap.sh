#!/bin/bash

timestamp=$(date +%s)
snap_name=${1}
udpexe=tinyshell

#Set WE = 1
$udpexe "regw "$snap_name"_we 1"

#Toggle CTRL register

$udpexe "regw $snap_name/ctrl 1"
$udpexe "regw $snap_name/ctrl 0"

#Get data
$udpexe "bramd $snap_name/bram" | awk '{print $3, $9, $9, $9, $9, $9, $9}' > "$snap_name"_$timestamp.txt

#$udpexe "bramd $snap_name/bram" | awk '{printf("%d %d\n", $3, $9)}' | awk > "$snap_name".txt
