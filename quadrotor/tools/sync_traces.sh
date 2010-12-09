#!/bin/bash

# sync with traces on beta
rsync -avz 192.168.1.3:/home/traces/ .
rsync -avz . 192.168.1.3:/home/traces/
