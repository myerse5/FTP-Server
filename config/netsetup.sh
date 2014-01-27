#!/bin/bash

#------------------------------------------------------------------------------
# File: netsetup.sh
# Date: January, 2014
# Author: Evan Myers
#
# This script will auto generate the network settings found in the 'ftp.conf'
# file. No other files will be modified. An internet connection will be
# required for a proper configuration when using the "nat" argument to this
# script.
#
# Arguments:
#   loopback - Useful for testing the server when not connected to a network.
#        The server may only be accessed on the server.
#
#   default - A server who is not using NAT, or a server that is to be used for
#        a local network should use this setting. The interface chosen may be
#        specified by the the [-i interface] option, or else the first
#        interface with an IPv4 address will be chosen from the output of
#        ifconfig.
#
#   nat - The external IPv4 address will be detected and displayed on the
#        server console as the reachable IP address. The external IPv4 address
#        will not be used for any other purpose. This address will likely be
#        the IP address of the router, and will differ from the IP address used
#        to create the sockets used by the FTP server. All other settings will
#        be equivelant to the default setup.
#
#   reset - The default port is set to 21, and all other network settings are
#        cleared. Suitable to use when pushing code to a public repository.
#------------------------------------------------------------------------------


#The port argument must be either DEF_PORT, or in the range MIN_PORT-MAX_PORT.
MAX_PORT=65535
MIN_PORT=1024
DEF_PORT=21

#Interface constants.
LOOPBACK="lo"


#------------------------------------------------------------------------------
#
#------------------------------------------------------------------------------
function helpMesg() {
    echo "use $0 -h for more information"
}


#------------------------------------------------------------------------------
# Print the usage message to standard out.
#------------------------------------------------------------------------------
function usageMesg() {
    echo "usage: $0 [-i interface] [-p port] mode"

    echo -ne "\nmode:"
    echo -e "\tloopback - For local computer access, no network necessary."
    echo -e "\tdefault - Use any available interface. See [-i interface]."
    echo -e "\tnat - For global access when behind a NAT router.\n"

    echo -ne "options:"
    echo -e "[-i interface] Specify an interface to be used by the server."
    echo -e "\t[-p port] Specify a default port to be used instead of port 21."
}


#------------------------------------------------------------------------------
# Set the interface to the loopback interface. Remove the NAT_GLOBAL_IP value
# if present.
#------------------------------------------------------------------------------
function loSetup() {
    sed -i "/INTERFACE_CONFIG/c\INTERFACE_CONFIG $LOOPBACK" ftp.conf
    sed -i "/NAT_GLOBAL_IP /c\NAT_GLOBAL_IP " ftp.conf
}


#------------------------------------------------------------------------------
# Set the interface value and remove the NAT_GLOBAL_IP value if present. The
# first interface found will be used unless a specific interface was chosen as
# an interface argument (-i) by the user on the command line.
#------------------------------------------------------------------------------
function defaultSetup() {
    for iface in $@; do
	#Skip the loopback interface.
	if [[ "$iface" == "$LOOPBACK" ]]; then
	    continue
	fi
	
	#Set the command line interface option if it is found, or set the first
	#interface returned from ifconfig.
	if [[ -n "$userIface" ]]; then
	    if [[ "$userIface" == "$iface" ]]; then
		match=$iface
		break
	    fi
	else
	    match=$iface
	    break
	fi
    done

    if [[ -z "$match" ]] && [[ -n "$userIface" ]]; then
	echo "Error: Interface '$userIface' does not exist" >&2
	helpMesg
	exit 1
    elif [[ -z "$match" ]] && [[ -z "userIface" ]]; then
	echo "Error: No interfaces found. Try using mode 'loopback'"
	helpMesg
	exit 1
    fi
	
    #Update the interface in the config file, remove any NAT IP values.
    sed -i "/INTERFACE_CONFIG/c\INTERFACE_CONFIG $match" ftp.conf
    sed -i "/NAT_GLOBAL_IP/c\NAT_GLOBAL_IP" ftp.conf	
}


#------------------------------------------------------------------------------
# Equivelant to the default setup, but also retrieves the public IPv4
# address to be displayed on the server console.
#
# This setup should only be called when the server has a NAT router as the
# gateway to the internet.
#------------------------------------------------------------------------------
function natSetup() {
    defaultSetup "$@"
    sed -i "/NAT_GLOBAL_IP/c\NAT_GLOBAL_IP $(wget -qO- http://ipecho.net/plain; echo)" ftp.conf
}


#Acknowledgement: http://wiki.bash-hackers.org/howto/getopts_tutorial
while getopts ":i:p:h" opt; do
    case $opt in
	p)
	    port=$OPTARG
	    shift $((OPTIND-1))
	    ;;
	i)
	    userIface="$OPTARG"
	    shift $((OPTIND-1))
	    ;;
	h)
	    usageMesg
	    exit 0
	    ;;
	\?)
	    echo "Error: Invalid option: -$OPTARG" >&2
	    helpMesg
	    exit 1
	    ;;
	:)
	    echo "Error: Option -$OPTARG requires an argument." >&2
	    helpMesg
	    exit 1
	    ;;
    esac
    shift $((OPTIND-1))
done

if [[ -n "$userIface" ]] && [[ "$userIface" == "$LOOPBACK" ]]; then
    echo "Error: Interface '$LOOPBACK' may only be used as a mode"
    helpMesg
    exit 1
fi

#Check the script argument count (that are left after the options). 
if [[ $# -lt 1 ]]; then
    echo "Error: No mode selected"
    helpMesg
    exit 1
fi

#Collect all available interface names.
interfaces=$(ifconfig | grep -B2 "inet addr" | awk '{print $1}' |\
             grep -v -e 'inet\|--')

#Redirect according to script argument.
if [[ "$1" == "loopback" ]]; then
    loSetup
elif [[ "$1" == "default" ]]; then
    defaultSetup $interfaces
elif [[ "$1" == "nat" ]]; then
    natSetup $interfaces
elif [[ "$1" == "clear" ]]; then
    sed -i "/INTERFACE_CONFIG/c\INTERFACE_CONFIG " ftp.conf
    sed -i "/NAT_GLOBAL_IP/c\NAT_GLOBAL_IP" ftp.conf
    sed -i "/DEFAULT_PORT_CONFIG/c\DEFAULT_PORT_CONFIG $DEF_PORT" ftp.conf
    exit 0
fi

#Set the default port if the option was selected.
if [[ -n "$port" ]]; then
    (($port+0)) >/dev/null
    if [[ $? -ne 0 ]]; then
	echo "Error: Port argument must be a posotive integer"
	helpMesg
	exit 1
    fi

    if [[ $port -gt $MAX_PORT ]]; then
	echo "Error: Port must be $DEF_PORT or in range $MIN_PORT-$MAX_PORT" >&2
	helpMesg
	exit 1
    elif [[ $port -lt $MIN_PORT ]] && [[ $port -ne $DEF_PORT ]]; then
	echo "Error: Port must be $DEF_PORT or in range $MIN_PORT-$MAX_PORT" >&2
	helpMesg
	exit 1
    fi

    sed -i "/DEFAULT_PORT_CONFIG/c\DEFAULT_PORT_CONFIG $port" ftp.conf
else
    sed -i "/DEFAULT_PORT_CONFIG/c\DEFAULT_PORT_CONFIG $DEF_PORT" ftp.conf
fi
    