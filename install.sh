#!/bin/sh

set -e

PREFIX="/usr/local/"

while [ $# -gt 0 ]; do
    case "$1" in
        -h|--help) 
            echo "Usage: install.sh [OPTION]"
            echo ""
            echo "  --prefix   Prefix of the install location"
            echo "  --bindir   Install location for executables"
            echo "  --mandir   Install location for manpages"
            echo ""
            exit 0
            ;;
        --prefix) 
            PREFIX="$2"
            shift 2
            ;;
        --bindir) 
            USERBINDIR="$2"
            shift 2
            ;;
        --mandir) 
            USERMANDIR="$2"
            shift 2
            ;;
        *)
            echo "$0: invalid option: $1"
            echo "Use '$0 --help' to get a list of options."
            exit 1 
            ;;
    esac
done

if [ -z "$USERMANDIR" ]; then
    MANDIR="${PREFIX}/share/man/"
else
    MANDIR="$USERMANDIR"
fi

if [ -z "$USERBINDIR" ]; then
    BINDIR="${PREFIX}/bin/"
else
    BINDIR="$USERBINDIR"
fi

if [ -z "$DRYRUN" ]; then
    install -v -d "${BINDIR}"
    install -v -d "${MANDIR}/man1"
    install -v xboxdrv "${BINDIR}"
    install -v xboxdrv-daemon "${BINDIR}"
    install -v doc/xboxdrv.1 "${MANDIR}/man1/"
    install -v doc/xboxdrv-daemon.1 "${MANDIR}/man1/"
    echo ""
    echo "xboxdrv install complete"
fi

# EOF #
