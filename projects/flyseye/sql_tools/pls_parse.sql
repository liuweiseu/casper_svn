-- file: pls_parse.sql
-- auth: Billy Mallard
-- mail: wjm@berkeley.edu
-- date: 29-Aug-2008

use flyseye;

CREATE TABLE pls_file_map
(
  id INT UNSIGNED NOT NULL AUTO_INCREMENT,
  data_file_id INT(11) UNSIGNED NOT NULL,
  antenna MEDIUMINT(8) UNSIGNED NOT NULL,
  start_time INT(10) UNSIGNED NOT NULL,
  PRIMARY KEY (id)
);

INSERT INTO pls_file_map (data_file_id, antenna, start_time) SELECT data_file_id, antenna, start_time FROM images_redux_deux ORDER BY data_file_id, antenna, start_time;

CREATE TABLE pls_file_parse_status
(
  id INT UNSIGNED NOT NULL,
  parse_started BOOLEAN NOT NULL DEFAULT FALSE,
  parse_finished BOOLEAN NOT NULL DEFAULT FALSE,
  proc_server VARCHAR(64),
  proc_time TIMESTAMP,
  PRIMARY KEY (id)
);

INSERT INTO pls_file_parse_status (id) SELECT pls_file_map.id FROM pls_file_map LEFT JOIN pls_file_parse_status ON pls_file_map.id = pls_file_parse_status.id WHERE pls_file_parse_status.id IS NULL ORDER BY id;

CREATE TABLE detections
(
  id BIGINT UNSIGNED NOT NULL,
  dispersion_measure SMALLINT UNSIGNED NOT NULL,
  smoothing_level TINYINT UNSIGNED NOT NULL,
  timestamp INT UNSIGNED NOT NULL,
  sigma FLOAT NOT NULL,
  mystery_field TINYINT UNSIGNED NOT NULL,
  rfi_tag FLOAT NOT NULL,
  INDEX (id)
);

