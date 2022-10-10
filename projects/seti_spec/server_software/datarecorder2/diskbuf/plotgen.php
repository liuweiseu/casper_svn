#!/usr/bin/php

<?php
$convert_command = "/usr/bin/convert";
$montage_command = "/usr/bin/montage";

$gencsv_command = "/home/siemion/server_software/datarecorder2/diskbuf/quicklook";

$gnuplot_command = "/usr/bin/gnuplot /home/siemion/server_software/datarecorder2/diskbuf/gnuplot.script";

$pdf_args = "";
$specstart=0;

if(!file_exists($argv[1])){
   print $argv[1]." not found!\n";
   exit;
}

if(file_exists($argv[2]) || strlen($argv[2]) < 4){
   print $argv[2]." exists or is invalid!\n";
   exit;
}


while(!stristr($output, "EOF")) {

$shellcommand = $gencsv_command." ".$argv[1]." ".$specstart." ".($specstart + 500)." gnuplot.txt"; 
echo "executing... ".$shellcommand."\n";
$output = shell_exec($shellcommand);
echo $output;
$shellcommand = $gnuplot_command; 
echo "executing... ".$shellcommand."\n";
shell_exec($shellcommand);

$shellcommand = $montage_command." -geometry 720x192 -tile 1x4 filter.png time.png corner.png";
echo "executing... ".$shellcommand."\n";
shell_exec($shellcommand);


$shellcommand = $montage_command." -geometry 720x504 -tile 3x2 histogram.png histogram_sigma.png time.png 2dscatter.png scatter.png filter.png scatter_".$specstart.".pdf";
echo "executing... ".$shellcommand."\n";
shell_exec($shellcommand);


$pdf_args = $pdf_args."scatter_".$specstart.".pdf ";


$specstart = $specstart + 500;
}

echo "Reached end of file: ".$output."\n";



shell_exec("pdftk ".$pdf_args." cat output ".$argv[2]);
shell_exec("rm ".$pdf_args);

?>