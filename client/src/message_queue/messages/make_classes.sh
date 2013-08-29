#!/bin/bash
protoc --proto_path=. request.proto --cpp_out=output
