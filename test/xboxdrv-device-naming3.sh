#!/bin/sh

./xboxdrv --daemon \
    `# regular Xbox360 controller gets flightstick threatment` \
    --next-controller \
    --next-controller \
 "$@"

# EOF #
