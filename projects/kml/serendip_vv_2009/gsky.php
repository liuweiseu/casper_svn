<?phpfunction convertHoursToHoursMinutesAndSeconds($hours) {	$integralHours = floor($hours);	$remainderHours = $hours - $integralHours;	$minutes = floor(round($remainderHours / 1 * 60, 1));	$remainderMinutes = $remainderHours / 1 * 60 - $minutes;	$seconds = abs(round(($remainderMinutes / 1 * 60), 2));	//$minutes = str_pad($minutes, 2, '0', STR_PAD_LEFT);	//$seconds = str_pad($seconds, 4, '0', STR_PAD_LEFT);	//$seconds = str_pad($seconds, 5, '0', STR_PAD_RIGHT);	$sexag[0] = $integralHours;	$sexag[1] = $minutes;	$sexag[2] = $seconds;	//return "{$integralHours}:{$minutes}:{$seconds}";	return $sexag;}$file_name = "candidates.csv";$fp = fopen($file_name, "r");$sexag = convertHoursToHoursMinutesAndSeconds(40.221);printf("%d%d%.2f\n",$sexag[0],$sexag[1],$sexag[2]);//var_dump($hoursMinutesAndSeconds);print "<kml xmlns='http://www.opengis.net/kml/2.2' hint='target=sky'>\n";print "<Document>\n";while (!feof($fp)){   $row = fgets($fp);   $rows = explode(",", $row);     if(strlen($rows[0]) > 2){					 $rah = (float) $rows[0];		 $dec = (float) $rows[1];		 $ra = ($rah / 24 * 360) - 180;		 	print '		  	<Placemark>			 <name>SETI Candidate</name>			 <description>			   <![CDATA[				SETI@Home Candidate				]]>			 </description>			 <LookAt>			   <longitude>'.$ra.'</longitude>			   <latitude>'.$dec.'</latitude>			   <altitude>0</altitude>			   <range>10000</range>			   <tilt>0</tilt>			   <heading>0</heading>			 </LookAt>			 <Point>			   <coordinates>'.$ra.','.$dec.',0</coordinates>			 </Point>		   </Placemark>';		      }}fclose ($fp);		print "</Document>\n";		print "</kml>\n";?>