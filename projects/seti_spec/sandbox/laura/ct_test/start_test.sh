#!/bin/bash

./restart_ct.sh

sleep 1
echo "TVG set"

./autosnap.sh &
echo "Auto snap begun."
