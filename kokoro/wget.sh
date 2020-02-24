#!/bin/bash
set -xe

# Downloads a file and verifies its authenticity using a checksum.
function download_wget() {
  local -r url="$1"
  local dst_path="$2"
  local -r sha256sum="$3"

  if [[ -d "${dst_path}" ]]; then
    dst_path="${dst_path%/}/$(basename "${url}")"
  fi

  if [[ -f "${dst_path}" ]]; then
    echo "${sha256sum}  ${dst_path}" | sha256sum --quiet -c
    if [[ $? -eq 0 ]]; then
      echo "${dst_path}"
      return
    fi
  fi

  wget -q -O "${dst_path}" "${url}"
  if [[ $? -ne 0 ]]; then
    return 1
  fi
  echo "${sha256sum}  ${dst_path}" | sha256sum --quiet -c
  if [[ $? -ne 0 ]]; then
    return 1
  fi

  echo "${dst_path}"
}
