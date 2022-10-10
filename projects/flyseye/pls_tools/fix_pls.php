#!/usr/bin/php
<?
ini_set("memory_limit","120M");
$scratch_dir = "/dev/shm/";



include("./mysql.inc");

$dbname = "flyseye";
$hostname = "localhost";

$handle = dbconnect($dbname, $hostname) or die("dbconnect failed\n");
print "connected to database, looking for 100% complete records\n";

$query = "SELECT COUNT( * ) AS `Rows` , `data_file_id` FROM `analysis` GROUP BY `data_file_id` ORDER BY `data_file_id`";
$result = mysql_query($query, $handle);
print "done.";



//$sql =  "SELECT date_analyzed FROM analysis";
//$sql = 'SELECT pls_file, data_file_id, start_time, end_time, antenna, date_analyzed FROM analysis where analysis_notes NOT LIKE \'%plsfixed%\'';

//echo "attempting sql select operation...\n";
//$result = mysql_query($sql, $handle);
//echo "done...\n";

while($row = mysql_fetch_array($result, MYSQL_NUM) ){

    if($row[0] == 264) {     //100% processed    
        $data_file_id = $row[1];
        print "Found complete data file at id: ".$data_file_id."\n";

        for($recordcnt = 0;$recordcnt < 264;$recordcnt = $recordcnt + 11){

             $sql = 'SELECT pls_file, data_file_id, start_time, end_time, antenna, date_analyzed 
                     FROM analysis where data_file_id = '.$data_file_id.' 
                     AND analysis_notes NOT LIKE \'%plsfixed%\' LIMIT 11';
             print "runnning: ".$sql."\n";
             $current_rows = mysql_query($sql, $handle);

 
             while($rows=mysql_fetch_array($current_rows)){
                                 
                 //pull .pls file from analysis table
                 $data_file_id =  $rows['data_file_id'];
                 $antenna =  $rows['antenna'];
                 $start_time =  $rows['start_time'];
                 $end_time =  $rows['end_time'];
                 $date_analyzed =  $rows['date_analyzed'];
                 $pls_file = $rows['pls_file'];
                 print "fixing data_file_id: " . $data_file_id . " antenna: " . $antenna . " start_time: " . $start_time . " end_time: " . $end_time . " date_analyzed: " . $date_analyzed . "\n";
                 $new_pls_file = "";
                 if($pls_file != ""){
                       file_put_contents($scratch_dir."tmppls1.pls", $pls_file);
                       exec("grep -v tsamp ".$scratch_dir."tmppls1.pls > ".$scratch_dir."tmppls.pls");
                       exec("/dev/shm/fix_pls ".$scratch_dir."tmppls.pls ".$scratch_dir."tmppls_fixed.pls");                 
                       $new_pls_file = file_get_contents($scratch_dir."tmppls_fixed.pls");
                       print "Here's a new pls file...\n";
                 } else {
                    print 'empty pls...\n';
                 }
                 
                 
                 
                 //sleep(3);
                 
                 //echo $new_pls_file;	
                //print $new_pls_file;
             $new_pls_file = mysql_real_escape_string($new_pls_file);
             	print 'inserting...\n';
             	$sql_update = 'INSERT INTO corrected_pls_files 
             	(data_file_id, antenna, start_time, end_time, date_analyzed, pls_corrected)
             	VALUES (\''.$data_file_id.'\', \''.$antenna.'\', \''.$start_time.'\', \''.$end_time.
             	'\', \''.$date_analyzed.'\', \''.$new_pls_file.'\')';
             	
                         
             $result_update = mysql_query($sql_update);

              $sql_update = 'UPDATE `flyseye`.`analysis` 
                            SET `analysis_notes` = \'plsfixed\' 
                            WHERE `data_file_id` = ' . $data_file_id
                            . ' AND `antenna` = '. $antenna
                            . ' AND `start_time` = '. $start_time
                            . ' AND `date_analyzed` = '. $date_analyzed
                            . '';

            $result_update = mysql_query($sql_update);


   
             }
        }
    }
}

dbclose($handle);

?>
