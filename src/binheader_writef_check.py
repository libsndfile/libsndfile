#!/usr/bin/python

import re, string, sys

_whitespace_re = re.compile ("\s+", re.MULTILINE)

def find_binheader_writefs (data):
	lst = re.findall ('psf_binheader_writef\s*\(\s*[a-zA-Z_]+\s*,\s*\"[^;]+;', data, re.MULTILINE)
	return [_whitespace_re.sub (" ", x) for x in lst]

def find_format_string (s):
	fmt = re.search ('"([^"]+)"', s)
	if not fmt:
		print "Bad format in :\n\n\t%s\n\n" % s
		sys.exit (1)
	fmt = fmt.groups ()
	if len (fmt) != 1:
		print "Bad format in :\n\n\t%s\n\n" % s
		sys.exit (1)
	return _whitespace_re.sub ("", fmt [0])

def get_param_list (data):
	dlist = re.search ("\((.+)\)\s*;", data)
	dlist = dlist.groups ()[0]
	dlist = string.split (dlist, ",")
	dlist = [string.strip (x) for x in dlist]
	return dlist [2:]

def handle_file (fname):
	errors = 0
	data = open (fname, "r").read ()

	writefs = find_binheader_writefs (data)
	for item in writefs:
		fmt = find_format_string (item)
		params = get_param_list (item)
		param_index = 0

		# print item

		for ch in fmt:
			if ch in 'Eet ':
				continue

			# print "    param [%d] %c : %s" % (param_index, ch, params [param_index])

			if ch != 'b':
				param_index += 1
				continue

			# print item
			# print "    param [%d] %c : %s <-> %s" % (param_index, ch, params [param_index], params [param_index + 1])

			if string.find (params [param_index + 1], "sizeof") < 0 \
						and string.find (params [param_index + 1], "make_size_t") < 0 \
						and string.find (params [param_index + 1], "strlen") < 0:
				if errors == 0: print
				print "\n%s :" % fname
				print "    param [%d] %c : %s <-> %s" % (param_index, ch, params [param_index], params [param_index + 1])
				print "    %s" % item
				errors += 1
			param_index += 2
			
	return errors

#===============================================================================

if len (sys.argv) > 1:
	print "%s\n    binheader_writef_check   : " % sys.argv [0],
	sys.stdout.flush ()
	errors = 0
	for fname in sys.argv [1:]:
		errors += handle_file (fname)
	if errors > 0:
		print "\nErrors : %d\n" % errors
		sys.exit (1)

print "ok"

# Do not edit or modify anything in this comment block.
# The following line is a file identity tag for the GNU Arch
# revision control system.
#
# arch-tag: 4ed34789-925a-4135-af90-2e51523ca1ce
