#!/usr/bin/python
#
# file: pls_parse.py
# auth: Billy Mallard
# mail: wjm@berkeley.edu
# date: 02-Sep-2008

import MySQLdb
import os, sys

# NOTE: Do the following before running this script.
#
# 1) Open ssh tunnel to MySQL server.
#   ssh -L 9909:localhost:3306 flyeye@sun.berkeley.edu
#
# 2) Create ~/my.cnf with the following information.
#   [client]
#   host     = 127.0.0.1
#   port     = 9909
#   user     = [username]
#   password = [password]
#   database = flyseye

BATCH_SIZE = 10
HOSTNAME = os.uname()[1]

def get_next_pls_batch () :
	update_query = \
	"""
	UPDATE pls_file_parse_status
	  SET proc_server = "%s"
	  WHERE proc_server IS NULL
	  LIMIT %s
	""" % (HOSTNAME, BATCH_SIZE)

	select_query = \
	"""
	SELECT pls_file_map.id,
	       images_redux_deux.pls_rfi_removed,
		   pls_file_parse_status.parse_started
	  FROM images_redux_deux
	    LEFT JOIN pls_file_map
	      ON images_redux_deux.data_file_id = pls_file_map.data_file_id
	        AND images_redux_deux.antenna = pls_file_map.antenna
	        AND images_redux_deux.start_time = pls_file_map.start_time
	    LEFT JOIN pls_file_parse_status
	      ON pls_file_map.id = pls_file_parse_status.id
	  WHERE pls_file_parse_status.proc_server = "%s"
	    AND pls_file_parse_status.parse_finished = FALSE
	  LIMIT %s
	""" % (HOSTNAME, BATCH_SIZE)

	num_rows = cursor.execute(update_query)
	if num_rows == 0 :
		print "Nothing left to process!"
	cursor.execute(select_query)
	pls_file_tuples = cursor.fetchall()

	return pls_file_tuples

def parse_pls_file (pls_id, pls_file) :
	pls_rows = pls_file.splitlines()
	query = """INSERT INTO detections VALUES (%s, %s, %s, %s, %s, %s, %s)"""
	for pls_row in pls_rows :
		pls_cols = pls_row.split()
		pls_data = (pls_id,) + tuple(pls_cols)
		cursor.execute(query % pls_data)

def clear_incomplete_data (pls_id) :
	query = """DELETE FROM detections WHERE id = %s"""
	cursor.execute(query % pls_id)

def start_parsing (pls_id) :
    query = """UPDATE pls_file_parse_status SET parse_started = TRUE WHERE id = %s"""
    cursor.execute(query % pls_id)

def finish_parsing (pls_id) :
    query = """UPDATE pls_file_parse_status SET parse_finished = TRUE WHERE id = %s"""
    cursor.execute(query % pls_id)

#
# MAIN SCRIPT BODY
#

db = MySQLdb.connect(read_default_file="~/my.cnf")
cursor = db.cursor()

pls_file_count = -1
while pls_file_count != 0 :

	pls_file_tuples = get_next_pls_batch()
	pls_file_count = len(pls_file_tuples)

	for pls_file_tuple in pls_file_tuples :

		pls_id = pls_file_tuple[0]
		pls_file = pls_file_tuple[1]
		pls_unfinished = pls_file_tuple[2]
		print "Processing: ID %s." % pls_id

		if pls_unfinished == 1 :
			print "Clearing incomplete data. (previous parse interrupted?)"
			clear_incomplete_data(pls_id)

		start_parsing(pls_id)
		parse_pls_file(pls_id, pls_file)
		finish_parsing(pls_id)

