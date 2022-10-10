<?php
echo '<kml xmlns="http://www.opengis.net/kml/2.2"  xmlns:gx="http://www.google.com/kml/ext/2.2">'."\n";
echo '<Document>'; 
echo '<open>1</open>';
?>


<?php
echo '<Style id="linestylePurple">';
echo '<LineStyle>';
echo '<color>7fE666FF</color>';
echo '<width>3</width>';
echo '</LineStyle>';
echo ' </Style>';
echo '<Style id="linestyleGreen">';
echo '<LineStyle>';
echo '<color>7f7FFF66</color>';
echo '<width>3</width>';
echo '</LineStyle>';
echo ' </Style>';
echo '<Folder>';
for($i = 0; $i < 40; $i++){
$long = rand(2,358) - 180;
$lat = rand(2,178) - 90;
$color = rand(1,2);
echo '  <Placemark>'."\n";
echo '    <name>Tessellated</name>'."\n";
if($color==1) {
echo '<styleUrl>#linestylePurple</styleUrl>'."\n";
} else {
echo '<styleUrl>#linestyleGreen</styleUrl>'."\n";
}
echo '    <visibility>1</visibility>'."\n";
echo '    <LineString>'."\n";
echo '      <tessellate>1</tessellate>'."\n";
echo '      <coordinates>'."\n";
echo "-122.243944,37.880675,0\n";
echo $long.",".$lat.",0\n";
echo '      </coordinates>'."\n";
echo '    </LineString>'."\n";
echo '  </Placemark>'."\n";
}



echo '</Folder>';
echo '</Document>';


echo '</kml>';

?>

