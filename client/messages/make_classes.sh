#!/bin/bash
cd $(dirname $0)
mkdir -p ../protobuf_classes/
if protoc --proto_path=. *.proto --cpp_out=../protobuf_classes/ ; then
    echo "Success"
    exit 1
else
    echo "Failure"
    exit 0
fi
