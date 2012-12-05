#!/bin/bash

BASE_NAME=ngrt4n-d4n-1.1.0
MACHINE=ubs-1
SOURCE_TARBALL="/opt/dev/ngrt4n-qt/obs/${BASE_NAME}.orig.tar.gz"

set -e 
set -x
scp -p prepare.sh input.dat ${SOURCE_TARBALL} ${MACHINE}:
ssh ${MACHINE} "sh prepare.sh ${BASE_NAME} < input.dat"
scp -rp debian/  ubs-1:${BASE_NAME}/
ssh ${MACHINE} "cd ${BASE_NAME} && debuild -us -uc"
