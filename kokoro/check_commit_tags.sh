#!/bin/bash

SRC_DIR="${1}/git/gamelet_kernel"
#SRC_DIR="${1}"
GIT_DIR="${SRC_DIR}/.git"
ARTIFACT_DIR="${1}"

if [ ! -d ${ARTIFACT_DIR} ]; then
        echo "The artifact directory ${ARTIFACT_DIR} doesn't exist"
        exit 1
fi

if [ ! -d ${GIT_DIR} ]; then
        echo "The source directory ${GIT_DIR} doesn't exist"
        exit 1
fi

COMMIT_MSG=$(git --git-dir "${GIT_DIR}" log --pretty=%B -1)

function check_tag() {
  echo "${COMMIT_MSG}" | grep -E $1 && echo "found" || echo ""
}

BUG_TAG_FOUND=$(check_tag "^Google-Bug-Id:|^Bug:")
TESTED_TAG_FOUND=$(check_tag "^Tested:")
NO_TESTING_TAG_FOUND=$(check_tag "^No_Testing:")

SPONGE_XML='<?xml version="1.0" encoding="UTF-8"?>
<testsuites name="" tests="1" failures="0" errors="0" time="0.9">
  <testsuite name="Kernel_Presubmit_Tests" tests="1" failures="0" errors="0" time="0.9">
    <testsuite name="Commit_Message_Tests" tests="2" failures="0" errors="0" time="0.9">'

ERROR=0
if [ -z "${BUG_TAG_FOUND}" ];then
        SPONGE_XML="${SPONGE_XML}$(python3 ${SRC_DIR}/kokoro/generate_sponge_xml.py --generate_test_case=true --name=BUG --message='"Google Bug ID missing. Please add a Google Bug ID to the commit message."')"
        ERROR=1
else
        SPONGE_XML="${SPONGE_XML}$(python3 ${SRC_DIR}/kokoro/generate_sponge_xml.py --generate_test_case=true --passed=true --name=BUG --message='"Google Bug ID tag found"')"
fi

if [ -z "${TESTED_TAG_FOUND}" ];then
        if [ -z "${NO_TESTING_TAG_FOUND}" ];then
                SPONGE_XML="${SPONGE_XML}$(python3 ${SRC_DIR}/kokoro/generate_sponge_xml.py --generate_test_case=true --name=Tested --message='"Tested tag missing. Please specify how this commit was tested. Or add `No_Testing:&lt;reason&gt;` if no testing is required."')"
                ERROR=1
        fi
else
        SPONGE_XML="${SPONGE_XML}$(python3 ${SRC_DIR}/kokoro/generate_sponge_xml.py --generate_test_case=true --passed=true --name=Tested --message='"Tested tag found"')"
fi

SPONGE_XML="${SPONGE_XML}</testsuite> </testsuite></testsuites>"

mkdir "${ARTIFACT_DIR}/Commit_Message"
echo ${SPONGE_XML} > "${ARTIFACT_DIR}/Commit_Message/sponge_log.xml"

if [ "${ERROR}" = "1" ];then
        exit 1
fi
