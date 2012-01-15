#!/usr/bin/python
#
# Copyright (C) 2005-2012 Erik de Castro Lopo <erikd@mega-nerd.com>
#
# Released under the 2 clause BSD license.

import re, string, sys

# Take a line and split it ont the double quoted strings so we can test the
# non double quotes strings for double spaces.
def split_string_on_quoted_str (line):
	for k in range (0, len (line)):
		if line [k] == '"':
			start = k
			for k in range (start + 1, len (line)):
				if line [k] == '"' and line [k - 1] != '\\':
					return [line [:start + 1]] + split_string_on_quoted_str (line [k:])
	return [line]

class cstyle_checker:
	def __init__ (self):
		self.error_count = 0

	def get_error_count (self):
		return self.error_count

	def check_file (self, filename):
		self.filename = filename
		self.file = open (filename, "r")
		self.line_num = 0
		comment_nest = 0

		self.line_num = 1
		while 1:
			line = self.file.readline ()
			if not line:
				break

			orig_line = line.rstrip (line)
			# Delete C++ style comments.
			line = re.sub ("([ ]{1}|\t*)//.*", '', "".join (split_string_on_quoted_str (line)))

			# Handle comments.
			open_comment = line.find ('/*')
			close_comment = line.find ('*/')
			if comment_nest > 0 and close_comment < 0:
				line = ""
			elif open_comment >= 0 and close_comment < 0:
				line = line [:open_comment]
				comment_nest += 1
			elif open_comment < 0 and close_comment >= 0:
				line = line [close_comment:]
				comment_nest -= 1
			elif open_comment >= 0 and close_comment > 0:
				line = line [:open_comment] + line [close_comment:]

			# Check for errors finding comments.
			if comment_nest < 0 or comment_nest > 1:
				print ("Weird")
				sys.exit (1)
			elif comment_nest == 0:
				# If wer're not inside comments, check the line.
				self.line_checks (line, orig_line)
			self.line_num += 1

		self.file.close ()
		self.filename = None
		return

	def line_checks (self, line, orig_line):
		# print (line, end = "")

		# Check for error which occur in comments abd code.
		if re.search ("[ \t]+$", line):
			self.error ("contains trailing whitespace")

		if line.find ("  ") >= 0:
			self.error ("multiple space instead of tab")

		if re.search ("[^ ];", line):
			self.error ("missing space before semi-colon")

		# The C #define requires that there be no spaces in the
		# first argument, remove first part of line.
		if re.search ("\s*#\s*define\s+", line):
			line = re.sub ("\s*#\s*define\s+[^\s]+", '', line)

		# Open and close parenthesis.
		if re.search ("[^\s\(\[\*&']\(", line):
			self.error ("missing space before open parenthesis")
		if re.search ("\)-[^>]", line) or re.search ("\)[^,'\s\n\)\]-]", line):
			self.error ("missing space after close parenthesis")

		# Open and close square brace.
		if re.search ("[^\s\(\]]\[", line):
			self.error ("missing space before open square brace")
		if re.search ("\][^,\)\]\[\s\.-]", line):
			self.error ("missing space after close square brace")

		if re.search ("[^\s][\*/%+-][=][^\s]", line):
			self.error ("missing space around [*/%+-][=]")

		if re.search ("[^\s][<>!=^/][=]{1,2}[^\s]", line):
			self.error ("missing space around comparison")

		if re.search ("[a-zA-Z0-9][<>!=^/]{1,2}[a-zA-Z0-9]", line):
			if not re.search (".*#include.*[a-zA-Z0-9]/[a-zA-Z]", line):
				self.error ("missing space around operator")

		if re.search (";[a-zA-Z0-9]", line):
			self.error ("missing space after semi-colon")

		# Space after comma
		if re.search (",[^\s\n]", line):
			self.error ("missing space after comma")

		if re.search ("\s(do|for|if|when)\s.*{$", line):
			self.error ("trailing open parenthesis should be on the next line")

		return

	def error (self, msg):
		print ("%s (%d) : %s" % (self.filename, self.line_num, msg))
		self.error_count += 1

#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

if len (sys.argv) < 1:
	print ("Usage : yada yada")
	sys.exit (1)

# Create a ner cstyle_checker object
cstyle = cstyle_checker ()

error_count = 0
for filename in sys.argv [1:]:
	cstyle.check_file (filename)

if cstyle.get_error_count ():
	sys.exit (1)

sys.exit (0)

