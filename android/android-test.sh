#!/usr/bin/env bash
ABIS=`adb $@ shell getprop ro.product.cpu.abilist`

for ABI in $(echo $ABIS | tr "," "\n"); do
    echo "libsndfile: testing ABI [$ABI]"
    TEST_DIR="/data/local/tmp/libsndfile/test/$ABI"
    set -o xtrace

    # remove existing test files
    adb $@ shell rm -rv $TEST_DIR
    adb $@ shell mkdir -pv $TEST_DIR

    # push test files to device
    pushd build/intermediates/cmake/release/obj/$ABI
    adb $@ push ./* $TEST_DIR
    popd

    # run tests
    adb $@ shell "cd $TEST_DIR && find . -type f -executable -exec {} all \;"

    set +o xtrace
done

echo "Tests finished for ABIs: $ABIS"
echo "NOTE: make sure to verify the test results manually. This :task will not fail if tests fail"
