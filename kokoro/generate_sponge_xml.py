""" Script used to generate the sponge xml file

Sponge accepts an xml format as a result of test suite. It can contain
multiple test cases with properties, logs, status etc. This is a helper script
used to generate the sponge xml with support of fail/pass state with test
ouput message.
"""

import sys
import argparse
from string import Template

def generate_test_case(name, passed, message):
        tcase = Template('<testcase name="$name" status="run" time="0.5"> $test_case_body </testcase>')

        tfail = Template('<failure message=$message/>')
        tpass = Template('<properties> <property name="test_output" value=$message/> </properties>')
        test_case_body=None
        if passed:
                test_case_tag = tcase.substitute(test_case_body=tpass.substitute(message=message), name=name)
        else:
                test_case_tag = tcase.substitute(test_case_body=tfail.substitute(message=message), name=name)

        print(test_case_tag)

def main(args):
        if args.generate_test_case:
                generate_test_case(args.name, args.passed, args.message)

if __name__ == '__main__':
        parser = argparse.ArgumentParser()
        parser.add_argument('--generate_test_case', type=bool,
                    help='Generate test case tag')
        parser.add_argument('--name',
                    help='Test case name')
        parser.add_argument('--passed', type=bool,
                    help='Whether test passed')
        parser.add_argument('--message',
                    help='Test case message')
        if len(sys.argv) == 1:
                parset.print_help()
        args = parser.parse_args(sys.argv[1:])
        main(args)
