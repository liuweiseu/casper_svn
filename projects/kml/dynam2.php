<?php

$x = time()%100;

$long = -95.44 + $x;
$lat = 40.42;
echo '<kml xmlns="http://www.opengis.net/kml/2.2">'."\n";


if($x >= 0 && $x < 3) {
?>
  <NetworkLinkControl>
    <LookAt>
      <longitude>-122.243944</longitude>
      <latitude>37.880675</latitude>
      <altitude>0</altitude>
      <heading>126.1506691541814</heading>
      <tilt>65.38634344128558</tilt>
      <range>100.1988425024791</range>
    </LookAt>
  </NetworkLinkControl>
<?php
} else if($x >= 25 && $x < 28) {
?>
  <NetworkLinkControl>
    <LookAt>
      <longitude>34.0</longitude>
      <latitude>33.0</latitude>
      <altitude>0</altitude>
      <heading>0.0</heading>
      <tilt>0.0</tilt>
      <range>10000000.1988425024791</range>
    </LookAt>
  </NetworkLinkControl>
<?php
} else if($x >= 50 && $x < 53) {
?>
  <NetworkLinkControl>
    <LookAt>
      <longitude>122.0</longitude>
      <latitude>3.0</latitude>
      <altitude>0</altitude>
      <heading>0.0</heading>
      <tilt>0.0</tilt>
      <range>10000000.1988425024791</range>
    </LookAt>
  </NetworkLinkControl>
<?php
} else if($x >= 75 && $x < 78) {
?>
  <NetworkLinkControl>
    <LookAt>
      <longitude>-100.243944</longitude>
      <latitude>27.880675</latitude>
      <altitude>0</altitude>
      <heading>0.0</heading>
      <tilt>0.0</tilt>
      <range>10000000.1988425024791</range>
    </LookAt>
  </NetworkLinkControl>
<?php

}


echo '</kml>';

?>

