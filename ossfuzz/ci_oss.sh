#!/bin/bash

set -ex

PROJECT_NAME=libsndfile

# Clone the oss-fuzz repository
git clone https://github.com/google/oss-fuzz.git /tmp/ossfuzz

# TODO: Verify that the GITHUB variables below are correct for a PR
env | grep "GITHUB_"

if [[ ! -d /tmp/ossfuzz/projects/${PROJECT_NAME} ]]
then
    echo "Could not find the ${PROJECT_NAME} project in ossfuzz"

    # Exit with a success code while the libsndfile project is not expected to exist
    # on oss-fuzz.
    exit 0
fi

# Work out which repo to clone from, inside Docker
if [[ -n ${GITHUB_BASE_REF} ]]
then
    # Pull-request branch
    REPO=${GITHUB_REPOSITORY}
    BRANCH=${GITHUB_BASE_REF}
else
    # Push build.
    REPO=${GITHUB_REPOSITORY}
    BRANCH=${GITHUB_REF}
fi

# Modify the oss-fuzz Dockerfile so that we're checking out the current branch on travis.
sed -i "s@https://github.com/erikd/libsndfile.git@-b ${BRANCH} https://github.com/${REPO}.git@" /tmp/ossfuzz/projects/${PROJECT_NAME}/Dockerfile

# Try and build the fuzzers
pushd /tmp/ossfuzz
python infra/helper.py build_image --pull ${PROJECT_NAME}
python infra/helper.py build_fuzzers ${PROJECT_NAME}
popd
