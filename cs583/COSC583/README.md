# COSC583
Cryptography Project Repository

It's pretty simple. Read the included report for now. Then we'll bustle together to get 
a sizable report done. Chat will be over google hangouts or [Slack](https://slack.com/).
I'll include the info for that later.

## What do I need to make the report
You will need [Pandoc](https://pandoc.org) and LaTeX. 

## How to Make the Report

A makefile is included. Right now the default recipe is for _report.pdf_. So just use that.

## How do I add new sections to the report

Create a file for you new section. Add it to contents.toc in the proper place. Add it 
to the makefile. Then you're done. Pandoc will concatenate your fill appropriaetely 
then generate the report.
