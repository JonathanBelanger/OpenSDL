#! /bin/bash

#
# This script must be located in the top level directory for the project it
# will be building.  It takes one argument, which indicates that it should
# delete the build directory and recreate it, running cmake to start a fully
# clean build (this is more than just a 'make clean'.
#
# When executed, this script performs the following steps:
#
#    1. Saves the current directory (we'll return here when done)
#    2. Gets the directory where the script is located (project directory)
#    3. If the '-r|--refresh' qualifier was specified, deletes the 'build'
#       directory
#    4. If the 'build' directory does not exist, it creates one.
#    5. Changes the current directory to the 'build' directory.
#    6. If the 'build' was created in step 4, run 'cmake ..'
#    7. If the 'build' was not created in step 4, run 'make clean'
#    8. Changes the current directory back to the one saved in step 1
#
# Usage: build.sh [-h|--help]|[-d|--debug] [-r|--refresh]
#       [-d|--debug]    Run cmake for debug (release by default)
#       [-h|--help]     Display the usage inforamtion
#       [-r|--refresh]  Delete the 'build' directory, if it exists
#
SCRIPT_NAME=$0
#
# Usage function
#   Called when a parameter error was detected or the user asked for help.
#
function usage
{
    echo "Usage: $SCRIPT_NAME [-h|--help]|[-d|--debug] [-r|--refresh]"
    echo "  -d|--debug      instruct cmake to build for debug"
    echo "  -h|--help       display this help text"
    echo "  -r|--refresh    build from scratch"
    exit 0
}

#
# Get the current directory and the directory where this script is located.
#
CUR_DIR=$(pwd)
SCRIPT_LOC="${BASH_SOURCE[0]}"
while [ -h "$SCRIPT_LOC" ]; do
    DIR="$( cd -P "$( dirname "$SCRIPT_LOC" )" >/dev/null 2>&1 && pwd )"
    SCRIPT_LOC="$(readlink "$SCRIPT_LOC")"
    [[ $SCRIPT_LOC != /* ]] && SCRIPT_LOC="$DIR/$SCRIPT_LOC"
done
PROJ_DIR="$( cd -P "$( dirname "$SCRIPT_LOC" )" >/dev/null 2>&1 && pwd )"

#
# Put us into the project directory.
#
cd $PROJ_DIR

#
# Parse out the command-line options
#
refresh=false
build_type="Release"
while [ "$#" -gt 0 ]; do
    case "$1" in
        -d)         build_type="Debug"; shift 1;;
        -h)         usage;;
        -r)         refresh=true; shift 1;;
        -dr)        refresh=true; build_type="Debug"; shift 1;;
        -rd)        refresh=true; build_type="Debug"; shift 1;;
        --debug)    build_type="Debug"; shift 1;;
        --help)     usage;;
        --refresh)  refresh=true; shift 1;;
        -*) echo "unknown option: $1" >&2; usage;;
        *) handle_argument "$1"; shift 1;;
    esac
done

#
# If refresh was set, then if the 'build' directory exists, get rid of it.
#
if [[ refresh ]]; then
    [[ -d build ]] && rm -rf build/
fi

#
# If the 'build' directory does not exist, create it.
#
if [[ -d build ]]; then
    make_opt="clean"
else
    mkdir build
    make_opt=""
fi

#
# OK, the environment is set up, so change the current directory into the
# 'build' directory, then either run 'cmake ..' followed by 'make' or  just
# 'make clean'.
#
cd build
if [[ $make_opt = "" ]]; then
    cmake -DCMAKE_BUILD_TYPE=$build_type ..
fi
make $make_opt

#
# We are all done.  Reset the current directory and get out of here.
#
cd $CUR_DIR
exit 0
