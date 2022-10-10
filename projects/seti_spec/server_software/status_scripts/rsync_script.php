#!/usr/bin/php
<?
$ps_check_string = "ssh -L 3133";
$running_check_string = "rsync_script";
$ping_check_string = "1 received";
$recipient_list = "lspitler@astro.cornell.edu, mwagner@eecs.berkeley.edu, siemion@berkeley.edu";
$subject = "SERENDIP V.5 rsync Report";
$disk_to_check = "/dev/md1";
$mail_message = "SERENDIP V.5 rsync Report\n\n";



//Check to see if this process is running, and check that tunnel is active

$output = shell_exec("/bin/ps aux");

// count how many times this script is running
$runningpieces = explode($running_check_string, $output);


if( count($runningpieces) > 2 ) {
	$mail_message .= "\n\n*****Old rsync_script.php process STILL RUNNING on ".date ("l dS F Y h:i:s A")."!\n";
	$mail_message .= "This could mean a number of things... maybe check on it?\n";
	$send_message = 1;
	$subject = "SERENDIP V.5 RSYNC ERROR";
	if($send_message == 1){
		mail($recipient_list, $subject, $mail_message, "From: serendip@naic.edu\r\n"); 
	}
	echo "ending early...\n";
	exit(0);
} else {
	$mail_message .= "No stale rsync_script processes.. proceeding with prejudice.\n";
}

if( stristr($output, $ps_check_string) ) {
	$mail_message .= "SSH tunnel to lotf is wide open as of ".date ("l dS F Y h:i:s A")."\n";
} else {
	$mail_message .= "\n\n*****SSH tunnel to lotf is **NOT RUNNING** as of ".date ("l dS F Y h:i:s A")." PLEASE RESTART HUMAN!\n";
	$send_message = 1;
	$subject = "SERENDIP V.5 RSYNC ERROR";

}


	$rsyncoutput = shell_exec("/usr/bin/rsync -avz -e 'ssh -p 3133' /home/tfiliba/datarecorder2/diskbuf setispec@localhost:/Volumes/mufongo1/seti_spec_data");
    $send_message = 1;  //go ahead and send every day for now


$output = shell_exec("/bin/df -h");
$mail_message .= "\n\nDisk Usage: \n".$output."\n";

$output = shell_exec("/bin/df -h | grep ".$disk_to_check);
$tokens = explode(" ", $output);
for($i = 0;$i < count($tokens); $i++){
   if(stristr($tokens[$i], "%")){
	   $moretokens = explode("%", $tokens[$i]);
	   if($moretokens[0] > 75) {
		   $mail_message .= "\n*** NOTE *** Disk usage is high on: ".$disk_to_check." - ".$tokens[$i]."\n\n";
   		   $subject = "SERENDIP V.5 RSYNC ERROR";
	   	   $send_message = 1;
	   } else {
	   		   $mail_message .= "Disk usage looks OK on: ".$disk_to_check." - ".$tokens[$i]."\n\n";
	   }
   }
}

$mail_message .= "See rsync output below: \n";
$mail_message .= $rsyncoutput;

$mail_message .= "\nThank you, that is all.\n";


//echo $mail_message;

if($send_message == 1){
 mail($recipient_list, $subject, $mail_message, "From: serendip@naic.edu\r\n"); 
}

//echo "ending...\n";
exit(0);

?>


