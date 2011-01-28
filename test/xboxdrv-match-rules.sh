#!/bin/sh

echo "Use flightstick config for regular Xbox360 controller and fighting game config for arcade stick"
echo

./xboxdrv --daemon \
    `# regular Xbox360 controller gets flightstick threatment` \
    --name "Fightstick" \
    --match-group usbserial=13FEF2D \
    --trigger-as-zaxis --square-axis \
    --relative-axis y2=64000 \
    --axismap -y2=x2,x2=y2 \
  --next-controller \
    `# arcade stick gets fighting config` \
    --name "Xbox360 Controller" \
    --match-group usbserial=89E88EEF \
    --dpad-only \
    --trigger-as-button \
    --buttonmap lb=1,x=2,y=3,lt=4,a=5,b=6 \
    --buttonmap rb=1,rb=2,rb=3 \
    --buttonmap rt=4,rt=5,rt=6 \
  "$@"

# EOF #
