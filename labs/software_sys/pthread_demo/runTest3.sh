#!/bin/bash

ps -eLf|grep -E "cond2|UID" | grep -v grep
