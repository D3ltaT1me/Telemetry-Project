# Project 2

Hello! This is my submission for project 2 for the telemetry position at E-agle.

# Usage
When running the program, make sure to use the -f argument followed by the can file, otherwise it will fail.

Like this:
```text
project_2.exe -f candump.log
```

There are two other command line arguments implemented. The first is --debug, which simply prints the ID_map every time the csv file is created (My implementation uses a map with key ''string'' and value ''vector(long long)'' to store all of the timestamps and then uses that compute them). The second is --overwrite, which tells the program to overwrite existing session files from before the program was run.