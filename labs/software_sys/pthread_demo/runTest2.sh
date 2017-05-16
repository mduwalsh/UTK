#!/bin/bash

ps -eLf|grep -E "sem1|UID" | grep -v grep
