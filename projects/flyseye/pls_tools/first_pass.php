#!/usr/bin/php

<?php
ini_set("memory_limit","120M");


include("/home/casper/flys_eye_grid/php_scripts/mysql.inc");
$dbname = "flyseye";
$hostname = "localhost";
$scratch_dir = "/dev/shm/";
$billycommand = $scratch_dir."pls_skim ".$scratch_dir."plslook.pls";
$handle = dbconnect($dbname, $hostname) or die("dbconnect failed\n");


//Get a list of the data file ids for each of the completely corrected files
$sql = "SELECT COUNT( * ) AS `Rows` , `data_file_id` FROM `images_redux_deux` GROUP BY `data_file_id` ORDER BY `data_file_id`";
echo $sql;
$result = mysql_query($sql, $handle) or die('could not run query 1');
$i = 0;

while($row = mysql_fetch_array($result)) {
   $file_ids[$i] = $row[1];
   $i++;
}

//var_dump($file_ids);
$id_max = $i;
$id_cnt = 0;
$i = 0;

$edited_count = 0;

while(1)  {

    //select records 10 at a time


    $sql = 'SELECT data_file_id, antenna, start_time, date_analyzed, pls_rfi_removed FROM images_redux_deux where data_file_id = '.$file_ids[$id_cnt].' LIMIT '.$i.', 6';
    //echo $sql;
    $result = mysql_query($sql, $handle) or die('could not run query 2');
    $sqlcnt = 0;     
 
    //process up to six records
    while($row = mysql_fetch_array($result)) {

                //process records
                $data_file_id =  $row['data_file_id'];
                $antenna =  $row['antenna'];
                $start_time =  $row['start_time'];
                $date_analyzed =  $row['date_analyzed'];
                $pls_file = $row['pls_rfi_removed'];   
       
               if(   (strlen($pls_file) > 0)  ){            
                     file_put_contents($scratch_dir."plslook.pls", $pls_file);
                     $analyzed_cnt++;
                     //run billy's codes
                     unset($output);
                     exec($billycommand, $output);
                    if (sizeof($output) > 1) {
                        echo "file_analyzed\n"; 
			$numfound = sizeof($output) - 1;
			echo "\nfound: ".$numfound." events in: ";
                         echo "size of pls file: ".strlen($pls_file)."\n";   
                         
                         echo "data_file_id: ".$data_file_id." antenna: ".$antenna." start_time: ".$start_time."\n"; 
                         
                         for($k = 1;$k < sizeof($output);$k++) print $output[$k]."\n";
                     } else if (sizeof($output) == 1) {
			echo "file_analyzed\n";
		     }	
                }
       
            $sqlcnt = $sqlcnt + 1;    
      }
              
       
       
        if($sqlcnt == 6) {
            $i = $i + 6;
        } else {

             $id_cnt = $id_cnt + 1;
             $edited_count = $edited_count + 1;
             echo "Edited count is: ".$edited_count." and i = ".$i."\n";
             $i = 0;
             if($id_cnt == $id_max){
                 echo "Nothing more to do!\n";
                 dbclose($handle);
                 exit();
             }
       }

}





dbclose($handle);
?>
