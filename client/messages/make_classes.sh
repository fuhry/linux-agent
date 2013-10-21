#!/bin/bash
# This is for testing purposes only and should NOT be part of the main build process

cd $(dirname $0)
mkdir -p ../protobuf_classes/
if protoc --proto_path=. *.proto --cpp_out=../protobuf_classes/ ; then
    echo "Success"
    exit 1
else
    echo "Failure"
    exit 0
fi
