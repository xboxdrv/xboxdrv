#!/bin/sh

exec ./xboxdrv --daemon \
  --on-connect test/on-connect.sh \
  --on-disconnect test/on-disconnect.sh \
  --next-controller \
  --next-controller \
  --next-controller

# EOF #
