#!/bin/bash

TARGET=$1

(

    cd $TARGET

    git clone https://github.com/deepsea-inria/pbbs-pctl.git

    pbbs-pctl/script/get-dependencies.sh $TARGET
    
)
