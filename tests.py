from colorama import Fore, Style
from glob import glob
import os
import json
import sys
from time import time
import feedbackarcset as fas
import numpy as np

FAILURE = Fore.RED + "FAILURE" + Style.RESET_ALL
SUCCESS = Fore.GREEN + "SUCCESS" + Style.RESET_ALL


TEST_CASES = sys.argv[1:] or glob(
    os.path.abspath(os.path.dirname(__file__)) + "/testcases/*.data"
)
TEST_CASES.sort()


def main():
    quality_failures = []
    runtime_failures = []
    correctness_failures = []
    losses = []
    runtimes = []
    failed = False

    for index, test in enumerate(TEST_CASES):
        best_score = 0
        best_run = []

        score_file = test.replace(".data", ".json")

        if os.path.exists(score_file):
            with open(score_file) as sf:
                data = json.load(sf)
            best_score = data["score"]
            best_run = data["ordering"]

        start = time()
        test_name = os.path.basename(test).replace(".data", "")
        print test_name
        start = time()
        ft = fas.Tournament.load(test).optimise()
        runtime = time() - start
        score = ft.score

        ordering_error = None
        values_found = sorted(list(ft.ordering))

        for i, x in enumerate(values_found):
            if x != i:
                ordering_error = "Found wrong value #{x} at index #{i}"
                break

        correctness_failed = ordering_error

        if score > best_score:
            best_score = ft.score
            best_run = map(int, ft.ordering)

        if not correctness_failed:
            with open(score_file, "w") as o:
                data = {
                    'score': best_score,
                    'ordering': best_run,
                }
                json.dump(
                    data, o, indent=2, separators=(',', ': ')
                )

        quality_lost = (1 - score / best_score) * 100

        losses.append(quality_lost)
        runtimes.append(runtime)

        quality_failed = quality_lost > 3
        runtime_failed = runtime > 60

        failed = failed or quality_failed or runtime_failed

        if correctness_failed:
            correctness_failures.append(test_name)
        if quality_failed:
            quality_failures.append(test_name)
        if runtime_failed:
            runtime_failures.append(test_name)

        print "  Loss:     %.2f %s" % (
            quality_lost, FAILURE if quality_failed else SUCCESS)
        print "  Runtime:  %.2f %s" % (
            runtime, FAILURE if runtime_failed else SUCCESS)
        print "  Correctness:   %s" % (
            FAILURE if correctness_failed else SUCCESS,)

    def report_failures(name, failures):
        if failures:
            print "  %s: %s" % (name, ', '.join(failures))

    print "Runtime:"
    print "  mean   : %.2f" % np.mean(runtimes)
    print "  median : %.2f" % np.median(runtimes)
    print "  max    : %.2f" % max(runtimes)
    print "Loss:"
    print "  mean   : %.2f" % np.mean(losses)
    print "  median : %.2f" % np.median(losses)
    print "  max    : %.2f" % max(losses)

    if failed:
        print "Failures:"
        report_failures("Correctness", correctness_failures)
        report_failures("Performance", runtime_failures)
        report_failures("Quality", quality_failures)
        sys.exit(1)

if __name__ == '__main__':
    main()
