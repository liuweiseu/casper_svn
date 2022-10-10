#!/usr/bin/php
<?
$ps_check_string = "setispec_dr";
$disk_to_check = "/dev/md1";
$ping_check_string = "1 received";
$recipient_list = "danw@ssl.berkeley.edu, lspitler@astro.cornell.edu, mwagner@eecs.berkeley.edu, pmcmahon@stanford.edu, siemion@berkeley.edu";

$mail_message = "SERENDIP V.5 CRITICAL ERROR REPORT\n\n";

echo "pinging bee\n";
$output = shell_exec("/bin/ping -c 1 beecourageous");

if( stristr($output, $ping_check_string) ) {
        $mail_message .= "The BEE2 is responding to pings:\n".$output;
} else {
        echo "bee is dead\n";
        $mail_message .= "The BEE2 is **NON-RESPONSIVE**:\n".$output;
        $mail_message .= "Attempting to restart the BEE... \n";
        $output = shell_exec("/usr/bin/perl /home/siemion/UU.pl 192.168.100.65 admin:ibmthinkpad 5off");
        $mail_message .= "\n Power off command output: ".$output."\n";
	sleep(5);        
        $output = shell_exec("/usr/bin/perl /home/siemion/UU.pl 192.168.100.65 admin:ibmthinkpad 5on");
        $mail_message .= "\n Power on command output: ".$output."\n";
	sleep(2);	
	$output = shell_exec("/usr/bin/perl /home/siemion/UU.pl 192.168.100.65 admin:ibmthinkpad status");
        $mail_message .= "\n Current Status: ".$output."\n";
        sleep(60);
        $mail_message .= "Sleeping 60 seconds to wait for BEE2 to come back up...\n";
        $mail_message .= "\n";

        $output = shell_exec("/bin/ping -c 1 beecourageous");
                if( stristr($output, $ping_check_string) ) {
                        $mail_message .= "!!Success!! The BEE2 lives!\n";
                        $mail_message .= "Sleeping 5 minutes to let the BEE2 boot...\n";
                        sleep(300);
                        $mail_message .= "Attempting to restart the whole shebang...\n";
                        $output = shell_exec("sudo /home/siemion/dorun2.sh > /tmp/blah  2>&1 &");
                        do {
                                sleep(2);
                                $output = shell_exec("cat /tmp/blah");
                        } while(!stristr($output, "imdonenow"));

                        $mail_message .= "Check output below: \n".$output;

                } else {
                        $mail_message .= "Ugh... Looks like the BEE2 is still dead.  Better take a look at this.\n";
                }
        $send_message = 1;
}


$output = shell_exec("/bin/ps aux");

if( stristr($output, $ps_check_string) ) {
	$mail_message .= "The data collection process is running as of ".date ("l dS F Y h:i:s A")."\n";
} else {
	$mail_message .= "\n\n*****The data collection process is **NOT RUNNING** as of ".date ("l dS F Y h:i:s A")."\n";
	$output = shell_exec("/usr/bin/perl /home/siemion/UU.pl 192.168.100.65 admin:ibmthinkpad status");
	$mail_message .= "\n Current Status: ".$output."\n";
			$mail_message .= "Attempting to restart the whole shebang...\n";
			$output = shell_exec("sudo rm -rf /tmp/blah");
			$output = shell_exec("sudo /home/siemion/dorun2.sh > /tmp/blah  2>&1 &");
                        do {
			sleep(2);
			$output = shell_exec("cat /tmp/blah");
			} while(!stristr($output, "imdonenow"));			
			$mail_message .= "Check output below: \n".$output;			
	$send_message = 1;
}

//echo $mail_message;

$output = shell_exec("/bin/df -h");
$mail_message .= "\n\nDisk Usage: \n".$output."\n";

$output = shell_exec("/bin/df -h | grep ".$disk_to_check);

$tokens = explode(" ", $output);
for($i = 0;$i < count($tokens); $i++){
   if(stristr($tokens[$i], "%")){
	   $moretokens = explode("%", $tokens[$i]);
	   if($moretokens[0] > 95) {
		   $mail_message .= "*** WARNING *** WARNING *** DISK USAGE CRITICAL ON DISK: ".$disk_to_check." - ".$tokens[$i]."\n\n";
	   	   $send_message = 1;
	   }
   }
}


if($send_message == 1){
mail($recipient_list, "SERENDIP V.5 Critical Error", $mail_message, "From: serendip@naic.edu\r\n"); 
}
?>

