#!/bin/bash

ps -eLf|grep -E "multi_thr|UID" | grep -v grep
