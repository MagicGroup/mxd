#!/bin/sh
# This script belongs to RPPPPoEK 0.3, by Christian Nitschkowski

# You can modify it under the same terms as the main-application
# This is the GPL v2 or any later version.
# For further details about the license read the file COPYING in
# in the source-package

# Modify the following lines according to your system.

# Path to ifconfig
IFCONFIG=/sbin/ifconfig

# Path to grep
GREP=/bin/grep

# Path to cut
CUT=/usr/bin/cut

# Path to sudo
SUDO=/usr/bin/sudo

# Path to adsl-start (Roaring Penguin utils)
RPC=/usr/sbin/pppoe-start

# Path to adsl-stop (Roaring Penguin utils)
RPD=/usr/sbin/pppoe-stop

#####################################################################
#  You shouldn't modify anything below this line, if you don't know #
#  what you're doing!                                               #
#####################################################################

ERROR=0

# At first check if the tools are there and executable
if [ -x $IFCONFIG ]; then
    echo >/dev/null
else
    echo "$IFCONFIG isn't executable. Aborting."
    exit -1
fi
if [ -x $GREP ]; then
    echo >/dev/null
else
    echo "$GREP isn't executable. Aborting."
    exit -1
fi
if [ -x $CUT ]; then
    echo >/dev/null
else
    echo "$CUT isn't executable. Aborting."
    exit -1
fi
if [ -x $SUDO ]; then
    echo >/dev/null
else
    echo "$SUDO isn't executable. Aborting."
    exit -1
fi
if [ -x $RPC ]; then
    echo >/dev/null
else
    echo "$RPC isn't executable. Aborting."
    exit -1
fi
if [ -x $RPD ]; then
    echo >/dev/null
else
    echo "$RPD isn't executable. Aborting."
    exit -1
fi

case "$1" in
    connect)
	$SUDO $RPC || ERROR=1
    ;;
    dconnect)
	$SUDO $RPD || ERROR=1
    ;;
    status)
	if [ $2 ]; then
	    $IFCONFIG | $GREP -s -A 1 $2 | $GREP inet | $CUT -f 2 -d : | $CUT -f 1 -d \  
	else
	    echo "Usage: $0 status device"
	fi
    ;;
    *)
	echo "Usage: $0 {connect|dconnect|status device}"
    ;;
esac

exit $ERROR
