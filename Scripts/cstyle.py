#!/usr/bin/python
#
# Copyright (C) 2005-2012 Erik de Castro Lopo <erikd@mega-nerd.com>
#
# Released under the 2 clause BSD license.

# This program checks C code for compliance to coding standards used in
# libsndfile and other projects I run.

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

class comment_stripper:
	# Strip C and C++ style comments from a series of lines.
	def __init__ (self):
		self.comment_nest = 0
		self.leading_space_re = re.compile ('^(\t+| )')
		self.trailing_space_re = re.compile ('(\t+| )$')

	def comment_nesting (self):
		return self.comment_nest

	def strip (self, line):
		orig_line = line
		# Strip C++ style comments.
		if self.comment_nest == 0:
			line = re.sub ("\t*//.*", '', line)

		# Strip C style comments.
		open_comment = line.find ('/*')
		close_comment = line.find ('*/')

		if self.comment_nest > 0 and close_comment < 0:
			# Inside a comment block that does not close on this line.
			return ""

		if open_comment >= 0 and close_comment < 0:
			# A comment begins on this line but doesn't close on this line.
			self.comment_nest += 1
			return self.trailing_space_re.sub ('', line [:open_comment])

		if open_comment < 0 and close_comment >= 0:
			# Currently open comment ends on this line.
			self.comment_nest -= 1
			return self.trailing_space_re.sub ('', line [close_comment + 2:])

		if open_comment >= 0 and close_comment > 0 and self.comment_nest == 0:
			# Comment begins and ends on this line. Replace it with 'comment'
			# so we don't need to check whitespace before and after the comment
			# we're removing.
			newline = line [:open_comment] + "comment" + line [close_comment + 2:]
			return self.strip (newline)

		return line

class cstyle_checker:
	def __init__ (self, debug):
		self.debug = debug
		self.error_count = 0
		self.orig_line = ''
		self.trailing_newline_re = re.compile ('[\r\n]+$')

	def get_error_count (self):
		return self.error_count

	def check_files (self, files):
		for filename in files:
			self.check_file (filename)

	def check_file (self, filename):
		self.filename = filename
		self.file = open (filename, "r")

		self.line_num = 1

		stripper = comment_stripper ()
		while 1:
			line = self.file.readline ()
			if not line:
				break

			line = self.trailing_newline_re.sub ('', line)

			self.orig_line = line

			line = "".join (split_string_on_quoted_str (line))
			line = stripper.strip (line)

			self.line_checks (line)
			self.line_num += 1

		self.file.close ()
		self.filename = None

		# Check for errors finding comments.
		if stripper.comment_nesting () != 0:
			print ("Weird")
			sys.exit (1)

		return

	def line_checks (self, line):
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

		if re.search ("[a-zA-Z0-9][<>!=^/&\|]{1,2}[a-zA-Z0-9]", line):
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
		if self.debug:
			print ("'" + self.orig_line + "'")
		self.error_count += 1

#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

if len (sys.argv) < 1:
	print ("Usage : yada yada")
	sys.exit (1)

# Create a new cstyle_checker object
if sys.argv [1] == '-d' or sys.argv [1] == '--debug':
	cstyle = cstyle_checker (True)
	cstyle.check_files (sys.argv [2:])
else:
	cstyle = cstyle_checker (False)
	cstyle.check_files (sys.argv [1:])


if cstyle.get_error_count ():
	sys.exit (1)

sys.exit (0)

