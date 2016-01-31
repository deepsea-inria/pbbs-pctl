#!/bin/bash

TARGET=$1

$TARGET/pctl/script/print-include-paths.sh $TARGET

echo "$TARGET/quickcheck/quickcheck/"

echo "$TARGET/pbbs-pctl/include/"

echo "$TARGET/pbbs-pctl/example/include"

echo "$TARGET/pbbs-pctl/test/include"

echo "$TARGET/pbbs-pctl/bench/include"

