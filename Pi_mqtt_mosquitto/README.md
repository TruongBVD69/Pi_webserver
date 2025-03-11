- Command build this file:
    + protoc-c --c_out=. hmi_messages.proto
    + g++ main.cpp hmi_messages.pb-c.c -lprotobuf-c -lmosquitto -o my_program
