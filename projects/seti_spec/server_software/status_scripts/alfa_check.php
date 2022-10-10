#!/usr/bin/php
<?
$recipient_list = "lspitler@astro.cornell.edu, mwagner@eecs.berkeley.edu, tfiliba@eecs.berkeley.edu, siemion@berkeley.edu";

$tcpdumpcmd = "sudo /usr/sbin/tcpdump -A -i eth0 -c 300  udp port 2007 | grep ALFASHM";
$newfilecheckcmd = "sudo /bin/ls -t -x /home/tfiliba/datarecorder2/diskbuf/largefile* | head -1";

$mail_message = "SERENDIP V.v CRITICAL ERROR\n\n";


$alfain=0;

$output = shell_exec($tcpdumpcmd);
echo $output;

if (strlen($output) > 1 ) {
	$alfain = 1;
}

$output = shell_exec($newfilecheckcmd);
$output = shell_exec('/usr/bin/stat -c %Y '.$output);

$ftime = intval($output);
$current = time();

$timediff = $current - $ftime;

if($alfain == 1 && $timediff > 600) {
   $mail_message .= '\n\n';
   $mail_message .= 'Hello Serendipiters, \n\n';
   $mail_message .= 'It appears that the ALFA receiver is in place but we are not collecting new data.\n';
   $mail_message .= 'Please rectify.\n\nThank You, \n\nPattern';
   mail($recipient_list, "SERENDIP V.v CRITICAL ERROR", $mail_message, "From: serendip@naic.edu\r\n");
}

?>