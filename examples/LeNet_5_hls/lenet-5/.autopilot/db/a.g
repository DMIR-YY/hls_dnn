#!/bin/sh
lli=${LLVMINTERP-lli}
exec $lli \
    /home/yaochen/work/NN/NN_hls/tiny_dnn_LeNet5/examples/LeNet_5_hls/lenet-5/.autopilot/db/a.g.bc ${1+"$@"}
