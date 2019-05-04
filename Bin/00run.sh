#!/bin/sh

# Start these only if they are not running
ps auxw | grep abdata | grep -v grep > /dev/null
if [ $? != 0 ]
then
        ./abdata &
fi
sleep 2

ps auxw | grep abmsgs | grep -v grep > /dev/null
if [ $? != 0 ]
then
        ./abmsgs &
fi
sleep 2
ps auxw | grep abfile | grep -v grep > /dev/null
if [ $? != 0 ]
then
        ./abfile &
fi
ps auxw | grep ablogin | grep -v grep > /dev/null
if [ $? != 0 ]
then
        ./ablogin &
fi

./abserv &
# start abserv -conf abserv2.lua
# start abserv -conf abserv3.lua
