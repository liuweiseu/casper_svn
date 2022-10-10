#!/usr/bin/php
<?php
exec("grep data_file_id $argv[1]", $output);
$contents = file_get_contents($argv[1]);

for($i = 1;$i < sizeof($output);$i++){
  $exploded = explode(" ", $output[$i]);
  echo "<h3><a href='http://cancer.berkeley.edu/viewing_pages/view_details.php?data_file_id=".$exploded[1]."&start_time=".$exploded[5]."&antenna=".$exploded[3]."'>Hit # ".$i." data_file_id=".$exploded[1]." start_time=".$exploded[5]." antenna=".$exploded[3]." </a><br/> </h3>\n";
  $explodeA = explode($output[$i], $contents);
  $explodeB = explode("file_analyzed", $explodeA[1]);
  echo "<pre>".$explodeB[0]."</pre><hr>";
}







?>

