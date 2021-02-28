#!/bin/sh

cd src;

gmake clean;

gmake dep;

bear gmake V=1;

mv compile_commands.json ..;
