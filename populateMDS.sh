#!/bin/sh
#
# -*- mode: bash; -*-
#
# This file is part of the Back End Server component of the
# Hyrax Data Server.
#
# Copyright (c) 2018 OPeNDAP, Inc.
# Author: Nathan David Potter <ndp@opendap.org>, James Gallagher
# <jgallagher@opendap.org>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#
# You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.

BES_CONF=
local_bes_get_dap=./localBesGetDap.sh
build_dmrpp=./modules/dmrpp_module/data/build_dmrpp

show_usage () { 
    echo ""
    echo "OPeNDAP              MAY 20, 2018"

}

OPTIND=1         # Reset in case getopts has been used previously in the shell.

verbose=""

while getopts "h?vc:" opt; do
    case "$opt" in
    h|\?)
        show_usage
        exit 0
        ;;
    v)  verbose="-v"
        ;;
    c)  BES_CONF="-c $OPTARG"
        ;;
    esac
done

shift $((OPTIND-1))

[ "$1" = "--" ] && shift

# Find teh bes.conf file
if [ -z "$BES_CONF" ]
then 
    echo "BES_CONF is not set trying env variable 'prefix'...";  
    if [ ! -z $prefix ] # True if var is set and str length != 0
    then
        test_conf=$prefix/etc/bes/bes.conf
        echo "Env var 'prefix' is set. Checking $test_conf"; 
        if [ -r $test_conf ]
        then
            BES_CONF="-c $test_conf"
            if [ "x$verbose" = "x-v" ]
            then
                echo "BES_CONF: " $BES_CONF
            fi
        fi
    fi
fi   
    
if [ -z "$BES_CONF" ]
then     
    export BES_CONF=/etc/bes/bes.conf
    echo "Last Attempt:  Trying $BES_CONF"; 
    if [ -r $BES_CONF ]
    then
        BES_CONF="-c $BES_CONF"
        if [ "x$verbose" = "x-v" ]
        then
            echo "BES_CONF: " $BES_CONF
        fi
    else
        echo ""
        echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
        echo ""
        echo "Unable to locate a BES configuration file."
        echo "You may identify your BES configurstion in one of three ways: "
        echo ""
        echo "  1. Using the command line parameter '-c'"
        echo ""
        echo "  2. Setting the environment variable $prefix to the install"
        echo "     location of the BES."
        echo ""
        echo "  3. Placing the bes.conf file in the well known location"
        echo "         /etc/bes/bes.conf"
        echo ""
        echo "The command line parameter is the recommended usage."        
        echo ""
        echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
        show_usage
        echo ""
       exit;
    fi
fi

[ -n "$1" ] || { echo "Expected one or more pathnames." ; exit 1; }

# Read a pathname from the next argument
pathname=$1

# Ask the BES to get the DDS and DMR for that file

$local_bes_get_dap $verbose $BES_CONF -i $pathname -d dds > /dev/null

$local_bes_get_dap $verbose $BES_CONF -i $pathname -d dmr > /dev/null

$build_dmrpp $verbose $BES_CONF -f $pathname