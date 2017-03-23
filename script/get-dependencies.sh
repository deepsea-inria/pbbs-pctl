#!/bin/bash

TARGET=$1

(

    cd $TARGET

    git clone https://github.com/deepsea-inria/pctl.git

    pctl/script/get-dependencies.sh $TARGET

    git clone https://github.com/deepsea-inria/pasl.git

    git clone https://github.com/deepsea-inria/pbench.git

    git clone https://github.com/deepsea-inria/pbbs-include.git

    ipfs get QmUvGoyv8hBprTqjFnhD5m4HGkcxqS4FoNteKEbYmyLj9n -o=quickcheck
    
)
