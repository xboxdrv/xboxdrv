#!/bin/bash

# Bash script for using two (or more) wireless controllers 
# ########################################################
#
# Running xboxdrv regularly only supports a single controller, using
# two or more controllers is however possible by launching multiple
# instances of xbdroxv. When launching two instances the build-in
# command execution of xboxdrv no longer can be used, so keeping track
# and killing the xboxdrv processes has to be done manually.

# tell bash to exit the script when something goes wrong
set -e 

# launch xboxdrv for the first wireless controller
xboxdrv \
  --wid 0 \
  --deadzone 6000 --trigger-as-button -s --dpad-as-button \
  --ui-buttonmap back=KEY_ESC,start=KEY_ENTER,dd=KEY_DOWN,du=KEY_UP,dr=KEY_RIGHT,dl=KEY_LEFT &
XBOXPID1=$!

# launch xboxdrv for the second wireless controller
xboxdrv \
  --wid 1 \
  --deadzone 6000 --trigger-as-button -s --dpad-as-button \
  --ui-buttonmap back=KEY_ESC,start=KEY_ENTER,dd=KEY_DOWN,du=KEY_UP,dr=KEY_RIGHT,dl=KEY_LEFT &
XBOXPID2=$!

# launch your game
/usr/games/you_game

# after the game has exited, kill the started xboxdrv processes and
# wait for them to shut down
kill $XBOXPID1
kill $XBOXPID2
wait $XBOXPID1
wait $XBOXPID2

# EOF #
