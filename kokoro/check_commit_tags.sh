#!/bin/bash

GIT_DIR=$1
COMMIT_MSG=$(git --git-dir "${GIT_DIR}" log --pretty=%B -1)

function check_tag() {
  echo "${COMMIT_MSG}" | grep -E $1 && echo "found" || echo ""
}

BUG_TAG_FOUND=$(check_tag "^Google-Bug-Id:|^Bug:")
TESTED_TAG_FOUND=$(check_tag "^Tested:")
NO_TESTING_TAG_FOUND=$(check_tag "^No_Testing:")

if [ -z "${BUG_TAG_FOUND}" ];then
  echo "Google Bug ID missing. Please add a Google Bug ID to the commit message."
  exit 1
fi

if [ -z "${TESTED_TAG_FOUND}" ];then
  if [ -z "${NO_TESTING_TAG_FOUND}" ];then
    echo "'Tested:' tag missing. Please specify how this commit was tested."
    echo "Or add 'No_Testing:<reason>' if not testing is required."
    exit 1
  fi
fi

