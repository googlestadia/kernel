#!/bin/bash
set -ex

# Usage:
#   fix_permissions.sh --user 1000:1000 -- /some/script --with --args

while [ "$1" != "" ] ; do
  case $1 in
    --user)
      shift
      readonly USER_GROUP="$1"
      shift
      ;;
    --)
      shift
      break
      ;;
    *)
      shift
      ;;
  esac
done

readonly TO_RUN="$1"
shift
${TO_RUN} "$@"
readonly RETCODE=$?

chown -hR ${USER_GROUP} /workspace/artifacts

exit ${RETCODE}
