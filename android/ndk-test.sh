#!/usr/bin/env bash
SCRIPT_DIR=$(dirname $0)

LIB_NAME="sndfile"
TEST_DIR="/data/local/tmp/lib${LIB_NAME}/test"

# remove existing test files
adb $@ shell "rm -r $TEST_DIR" > /dev/null
adb $@ shell "mkdir -p $TEST_DIR" > /dev/null

ABIS=`adb $@ shell getprop ro.product.cpu.abilist`

print_message() {
  echo "[==========================================================]"
  echo "| [lib${LIB_NAME}]: $1"
  echo "[==========================================================]"
}

for ABI in $(echo $ABIS | tr "," "\n"); do
    if [ $ABI == "armeabi" ]; then
        print_message "skipping deprecated ABI: [$ABI]"; echo
        continue
    fi
    print_message "testing ABI [$ABI]"

    # create test abi directory
    TEST_ABI_DIR="$TEST_DIR/$ABI"
    adb $@ shell mkdir -p $TEST_ABI_DIR > /dev/null

    # push test files to device
    pushd "$SCRIPT_DIR/build/intermediates/cmake/release/obj/$ABI" > /dev/null
    adb $@ push * $TEST_ABI_DIR > /dev/null
    popd > /dev/null

    # run tests
    adb $@ shell -t "cd $TEST_ABI_DIR && export LD_LIBRARY_PATH=. && find . -type f -not -name '*.so' -executable -exec {} all \;"
    echo
done

print_message "tests finished for ABIS: [$ABIS]"; echo
echo "NOTE: make sure to verify the test results manually. This task will not fail if tests fail"
