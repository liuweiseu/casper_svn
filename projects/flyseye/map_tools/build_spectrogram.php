#!/usr/bin/php
<?
$google_maps_script = "/home/griffin/flyseye/map/gdal2tiles.py";
$map_base_url = "http://cancer.berkeley.edu/spectrograms/";
$map_base_dir = "/var/www/html/spectrograms/";
$image_extractor_script = "/home/griffin/flyseye/map/image_extractor";
$image_gen_script = "/home/griffin/flyseye/map/pulse.py";
$eq_script = "/home/griffin/flyseye/map/fil_shroom";
$script_scratch_dir = "/home/griffin/flyseye/map/";

//$ld_call = "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib;";

/*
*****1. get list in holding pattern dir on tape bot
*****2. check list against completed mysql entries
---for new entries
*****3. download data file
*****3b. equalize
*****3b. generate ignore file
*****4. convert to text file
*****5. convert to png
*****7. generate google map
*****8. store png, data file, url to complete sql entry
*****9. send out email notification
*****10. clean up
*/

//setup MySQL Connection
ini_set("memory_limit","220M");
include("mysql.inc");
$dbname = "flyseye";
$hostname = "127.0.0.1:24000";
$handle = dbconnect($dbname, $hostname) or die("dbconnect failed\n");

//Get the list of .fil files on the tapebot
$fil_on_tapebot = array();
$listcommand = "rsync -avP -e \"ssh -i /home/griffin/.ssh/id_rsa.flyeye-rsync\" joeri@blackhole.westgrid.ca\:data/flyeye/zoom_plots/";
exec($listcommand, $output);
for($j=0;$j<sizeof($output);$j++){
	if (substr($output[$j], -8) == "ibob.fil") {
		$exploded_str = explode(" ",$output[$j]);
	    array_push($fil_on_tapebot, $exploded_str[sizeof($exploded_str)-1]);
	}
}
unset($output);
//print_r($fil_on_tapebot);

//get the list of spectrograms in the db that have not been processed
$fil_in_db = array();
$sql = 'SELECT * FROM `spectrograms` WHERE `map_generated` != 1';
//$sql = "SELECT COUNT( * ) AS `Rows` , `spectrogram_id` FROM `spectrograms` GROUP BY `spectrogram_id` ORDER BY `spectrogram_id`";
$result = mysql_query($sql, $handle) or die('could not run query 1');
while($row = mysql_fetch_array($result)) {
   array_push($fil_in_db, $row);
}
//print_r($fil_in_db);

//find the entry for the .fil files on the tapebot
for ($i=0; $i<sizeof($fil_on_tapebot); $i++) {
	list($spectrogram_id, $tail) = split("ibob.fil", $fil_on_tapebot[$i]);
	for ($j=0; $j<sizeof($fil_in_db); $j++) {
		if($fil_in_db[$j]['spectrogram_id'] == $spectrogram_id) {
			
			//download .fil file from the tapebot
			print "Pulling ".$fil_on_tapebot[$i]." from the tapebot...\n";
			$listcommand = "rsync -avP -e \"ssh -i /home/griffin/.ssh/id_rsa.flyeye-rsync\" joeri@blackhole.westgrid.ca\:data/flyeye/zoom_plots/".$fil_on_tapebot[$i]." .";
			exec($listcommand, $output);
			$fil_file = $fil_on_tapebot[$i];
			
			//equalize code
			print "Equalizing...\n";
			exec($eq_script . " " . $fil_file . " " . $fil_file . ".eq temp.ignore 5.0 1.4 0 > fil_shroom.log");
			$raw_fil_file = $fil_file;
			$fil_file = $fil_file . ".eq";
			
			//get ignore file for 11 min data
			print "Pulling ignore file from db...\n";
			$sql = 'SELECT elog_file FROM analysis where 
				data_file_id = \''.$fil_in_db[$j]['data_file_id'].'\' AND 
				antenna = \''.$fil_in_db[$j]['antenna'].'\' AND 
				start_time = \''.$fil_in_db[$j]['start_time'].'\' AND 
				date_analyzed = \''.$fil_in_db[$j]['date_analyzed'].'\' LIMIT 1'; 

			$result = mysql_query($sql, $handle) or die('could not run query 2');
			$row = mysql_fetch_array($result);
			file_put_contents($script_scratch_dir."tmplog.log", $row['elog_file']);
			unset($output);
			exec("grep -i ignored ".$script_scratch_dir."tmplog.log", $output);
			//var_dump($output);

			$ignorefile = explode("IGNORED: ", $output[0]);
			//var_dump($ignorefile);
			unset($output);
			file_put_contents($script_scratch_dir."tmp.ignore", $ignorefile[1]);
			$ignore_file = $script_scratch_dir."tmp.ignore";
			
			//convert to text file
			print "Converting to text file...\n";
			$txt_file = $fil_file . ".txt";
			exec($image_extractor_script . " " . $fil_file . " " . $txt_file);
			
			//convert to png
			print "Converting to png...\n";
			exec($image_gen_script . " " . $txt_file . " -i " . $ignore_file . " -s " . $fil_file . " -F");
			$png_image = $fil_file . ".png";
			
			//generate google map
			
			/* ./gdal2tiles.py -noopenlayers -nokml -title "Test Map" -publishurl http://cancer.berkeley.edu/map_test/
			PREFIXibob03_1000.000000_10.000000.fil.temp.png 
			/var/www/html/map_test/PREFIXibob03_1000.000000_10.000000 */
			print "Generating map...\n";
			//$map_title = split(".fil",$fil_file);
			//$map_title = $map_title[0];
			$map_title = "spectrogram_id: " . $fil_in_db[$j]['spectrogram_id'] . " data_file_id: " . $fil_in_db[$j]['data_file_id']. " antenna: " . $fil_in_db[$j]['antenna'];
			exec($google_maps_script . " -noopenlayers -nokml -title \"" . $map_title . "\" -publishurl " . 
			$map_base_url . " " . $png_image . " " . $map_base_dir . $fil_in_db[$j]['spectrogram_id']);
			
			//store png, data file, url to complete sql entry
			print "Storing results to db...\n";
			$raw_spectrogram = mysql_real_escape_string(file_get_contents($script_scratch_dir.$raw_fil_file));
			$png_spectrogram = mysql_real_escape_string(file_get_contents($script_scratch_dir.$png_image));
			$map_url = $map_base_url . $fil_in_db[$j]['spectrogram_id'] . "/spectrogram.html";
			$sql = "update spectrograms set raw_spectrogram = \"".$raw_spectrogram."\", png_spectrogram=\"".$png_spectrogram."\", map_url=\"".$map_url."\", map_generated=\"1\" WHERE spectrogram_id = ".$spectrogram_id;
			$result = mysql_query($sql, $handle) or die('could not run query 3');
			
			//send out email notification
			$mail_list = split("," ,$fil_in_db[$j]['email']);
			
			$mail_subject = "Spectrogram " . $fil_in_db[$j]['spectrogram_id'] . " Complete";
			$mail_body = "Spectrogram " . $fil_in_db[$j]['spectrogram_id'] . " has be successfully generated, the spectrogram can be viewed at " .
			$map_url;

			for ($k=0; $k<sizeof($mail_list); $k++) {
				if(mail($mail_list[$k], $mail_subject, $mail_body)) {
					echo "Mail to " . $mail_list[$k] . " successfully sent\n";
				} else {
					echo "ERROR: mail to " . $mail_list[$k] . " not sent\n";
				}
			}

			//cleanup
			exec("rm -f " . $fil_file);
			exec("rm -f " . $raw_fil_file);
			exec("rm -f " . $txt_file);
			exec("rm -f " . $png_image);
			exec("rm -f tmp.ignore");
			exec("rm -f tmplog.log");
		}
	}
}

//Close MySQL connection
dbclose($handle);

?>
