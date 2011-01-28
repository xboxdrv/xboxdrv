#!/bin/sh

./xboxdrv --daemon \
    `# regular Xbox360 controller gets flightstick threatment` \
    --device-name mouse.auto="Virtual Mouse" \
    --device-name keyboard.auto="Virtual Keyboard" \
    --mouse \
    --next-controller \
    --mouse \
 "$@"

# EOF #
