#!/bin/sh

./xboxdrv --daemon \
    `# regular Xbox360 controller gets flightstick threatment` \
    --device-name auto.0="Slot1 Controller" \
    --device-name auto.1="Slot2 Controller" \
    --device-name auto.auto="Any Controller" \
    --device-name 0.0="Special Name" \
    --next-controller \
    --next-controller \
 "$@"

# EOF #
