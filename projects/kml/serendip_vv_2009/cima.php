<?phpfunction convertHoursToHoursMinutesAndSeconds($hours) {	$integralHours = floor($hours);	$remainderHours = $hours - $integralHours;	$minutes = floor(round($remainderHours / 1 * 60, 1));	$remainderMinutes = $remainderHours / 1 * 60 - $minutes;	$seconds = abs(round(($remainderMinutes / 1 * 60), 2));	//$minutes = str_pad($minutes, 2, '0', STR_PAD_LEFT);	//$seconds = str_pad($seconds, 4, '0', STR_PAD_LEFT);	//$seconds = str_pad($seconds, 5, '0', STR_PAD_RIGHT);	$sexag[0] = $integralHours;	$sexag[1] = $minutes;	$sexag[2] = $seconds;	//return "{$integralHours}:{$minutes}:{$seconds}";	return $sexag;}$file_name = "candidates2.csv";$fp = fopen($file_name, "r");//printf("%2f%2f%.2f\n",$sexag[0],$sexag[1],$sexag[2]);//var_dump($hoursMinutesAndSeconds);//print "<kml xmlns='http://www.opengis.net/kml/2.2' hint='target=sky'>\n";//print "<Document>\n";$cnt = 0;while (!feof($fp)){   $cnt++;   $row = fgets($fp);   $rows = explode(",", $row);     if(strlen($rows[0]) > 2){					 $ra = (float) $rows[0];		 $dec = (float) $rows[1];				 		$ras = convertHoursToHoursMinutesAndSeconds($ra);		$decs = convertHoursToHoursMinutesAndSeconds($dec);	    printf("ET-%s_%s   %s%s%s  +%s%s%s j 0 T zo # score: %s star: %s gaussians: %s pulses: %s triplets: %s spikes: %s comment: %s qpix: %s\n",str_pad($rows[0], 8, '0', STR_PAD_LEFT), str_pad($rows[1], 8, '0', STR_PAD_LEFT), str_pad($ras[0], 2, '0', STR_PAD_LEFT), str_pad($ras[1], 2, '0', STR_PAD_LEFT),str_pad(sprintf("%.2f",$ras[2]), 5, '0', STR_PAD_LEFT),str_pad($decs[0], 2, '0', STR_PAD_LEFT), str_pad($decs[1], 2, '0', STR_PAD_LEFT),str_pad(sprintf("%.2f",$decs[2]), 5, '0', STR_PAD_LEFT) , $rows[2], $rows[3], $rows[4], $rows[5], $rows[6], $rows[7], $rows[8], $rows[9]);	      }}fclose ($fp);?>