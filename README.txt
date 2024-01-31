Sean Sponsler

Usage: "./schedule quantum [prog 1 [args] [: prog 2 [args] [: prog3 [args] [: … ]]]]"

Uses execv() instead of execvp() to satisfy program requirements in regards to Piazza posts

Tested on unix1.csc.calpoly.edu
(/home/ssponsle/csc453)

Program should fulfill all requirements, rarely with larger quantums it may not start and run indefinitely,
cannot reproduce consistently

