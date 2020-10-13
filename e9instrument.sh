#!/bin/bash
#
# Copyright (C) 2020 National University of Singapore
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

if [ -t 1 ]
then
    RED="\033[31m"
    GREEN="\033[32m"
    YELLOW="\033[33m"
    BOLD="\033[1m"
    OFF="\033[0m"
else
    RED=
    GREEN=
    YELLOW=
    BOLD=
    OFF=
fi

usage()
{
    echo -e "${RED}usage${OFF}: $0 [OPTIONS]" >&2
    echo >&2
    echo "OPTIONS:" >&2
    echo "    -b FILE" >&2
    echo >&2
    exit 1
}

while getopts "b:" OPTION
do
    case "$OPTION"
    in
        b)
            BINARY=$OPTARG
            ;;
        *)
            usage
            ;;
    esac
done

if [ $# -eq 0 ]
then
    usage
fi

NAME=`basename "$BINARY"`

set -e

if [ ! -x ./e9patch ]
then
    echo -e "${RED}error${OFF}: e9patch is not installed (run ./build.sh first) " >&2
fi

echo -e "${GREEN}$0${OFF}: building instrument lib..."

./e9compile.sh e9fuzzer.c -O2 -Wno-error \
    -include tiny-lib.c \
    -include tiny-printf.c \
    -I "$PWD"

mv e9fuzzer e9instrument

echo -e "${GREEN}$0${OFF}: patching the target ($BINARY)..."
echo "./e9tool --match 'asm=j.*' --action 'call e9_maybe_log(rip, next)@'"e9instrument" "$BINARY" -o "$NAME-patched""

./e9tool \
	--match 'asm=j.*' \
	--action 'call e9_maybe_log(rip, next, base)@'"e9instrument" \
	"$BINARY" -o "$NAME-patched" | tee "e9patch-$NAME.log" 

echo -e "${GREEN}$0${OFF}: done!"
echo
echo

if grep -e 'num_patched .*(100.00%)' "e9patch-$NAME.log" > /dev/null
then
    echo -e "${GREEN}SUCCESS${OFF}!"
    echo
    echo -e "Successfully built the patched binary ${YELLOW}$NAME-patched${OFF}."
else
    echo -e "${RED}WARNING${OFF}!"
    echo
    echo -e "Built the patched binary ${YELLOW}$BINARY-patched${OFF}, however 100% coverage was not"
    echo "achieved.  This means that some instructions will not be intercepted."
fi

echo -ne "__AFL_SHM_ID\0" >> "$NAME-patched"
rm e9instrument
