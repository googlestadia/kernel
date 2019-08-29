#!/usr/bin/env python3
"""Generate a file from Bazel workspace status files using a template.

See
https://docs.bazel.build/versions/master/user-manual.html#flag--workspace_status_command
"""

import argparse
import errno
import io
import os
import re
import time

KOKORO_RELEASE_TRAIN_RE = re.compile(r"(\d{4}\.R+\d+)/release")
RAPID_RC_NUMBER_RE = re.compile(r"_RC(\d+)$")
RAPID_RELEASE_TRAIN_RE = re.compile(r"(\d{4}\.R+\d+)_RC\d+$")
SCM_BRANCH_RELEASE_TRAIN_RE = re.compile(r"^release/gamelet/(\d{4}\.R+\d+)")


def main():
  """main."""
  parser = argparse.ArgumentParser(
      "Generates a version header from Bazel workspace status files.")
  parser.add_argument("--output", help="Output file.")
  parser.add_argument(
      "--template",
      help=
      "Template file for the output file using Python's format string syntax. Use 'vars' as the token."
  )
  parser.add_argument(
      "--var_format",
      help=
      "Format string for a buildstamp variable using Python's format string syntax. Use 'k' and 'v' tokens."
  )
  parser.add_argument(
      "status_files",
      metavar='<path>',
      nargs='+',
      help="One or more files containing buildstamp status lines."
  )
  args = parser.parse_args()
  output_dir = os.path.dirname(args.output)
  try:
    os.makedirs(output_dir, mode=0o755)
  except OSError as e:
    if e.errno != errno.EEXIST:
      raise
  with io.open(args.template) as f:
    template = f.read()
  status = ''
  for status_file in args.status_files:
    with io.open(status_file) as f:
      status += f.read()
  vars = {
      "BUILD_EMBED_LABEL": "",
      "BUILD_HOST": "",
      "BUILD_TIMESTAMP": "0",
      "BUILD_USER": "",
      "STABLE_BUILD_SCM_BRANCH": "",
      "STABLE_BUILD_SCM_REVISION": "",
      "STABLE_BUILD_SCM_STATUS": "",
      "STABLE_KOKORO_BUILD_ID": "",
      "STABLE_KOKORO_BUILD_NUMBER": "0",
      "STABLE_KOKORO_JOB_NAME": "",
      "STABLE_KOKORO_JOB_TYPE": "",
      "STABLE_RAPID_CANDIDATE_NAME": "",
      "STABLE_RELEASE_CANDIDATE_NUMBER": "0",
      "STABLE_RELEASE_TRAIN": "0000.0",
  }
  for l in status.splitlines():
    k, v = l.split(" ", 1)
    vars[k] = v
  # Extract information from STABLE_KOKORO_JOB_NAME.
  if vars["STABLE_KOKORO_JOB_NAME"]:
    m = KOKORO_RELEASE_TRAIN_RE.search(vars["STABLE_KOKORO_JOB_NAME"])
    if m:
      vars["STABLE_RELEASE_TRAIN"] = m.group(1)
  # Extract information from STABLE_RAPID_CANDIDATE_NAME.
  if vars["STABLE_RAPID_CANDIDATE_NAME"]:
    m = RAPID_RC_NUMBER_RE.search(vars["STABLE_RAPID_CANDIDATE_NAME"])
    if m:
      vars["STABLE_RELEASE_CANDIDATE_NUMBER"] = m.group(1)
    m = RAPID_RELEASE_TRAIN_RE.search(vars["STABLE_RAPID_CANDIDATE_NAME"])
    if m:
      vars["STABLE_RELEASE_TRAIN"] = m.group(1)
  # Extract information from STABLE_BUILD_SCM_BRANCH.
  if vars["STABLE_BUILD_SCM_BRANCH"]:
    m = SCM_BRANCH_RELEASE_TRAIN_RE.search(vars["STABLE_BUILD_SCM_BRANCH"])
    if m:
      vars["STABLE_RELEASE_TRAIN"] = m.group(1)
  # Format BUILD_TIMESTAMP and store as BUILD_TIMESTAMP_FMT.
  vars["BUILD_TIMESTAMP_FMT"] = time.strftime(
      "%c %Z", time.localtime(int(vars["BUILD_TIMESTAMP"])))
  # Process the template and output the result.
  print("INFO: Formatting buildstamp variables with format: " + args.var_format)
  vars_str = "\n".join(
      args.var_format.format(k, v) for k, v in sorted(vars.items()))
  with io.open(args.output, "w+", encoding="utf-8") as f:
    f.write(template.format(vars=vars_str, **vars))


if __name__ == "__main__":
  main()
