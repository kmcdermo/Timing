#!/bin/bash

modifier=${1:-""}

root -l -b -q test_macros/plot_counter.C\(\"${modifier}\"\)
