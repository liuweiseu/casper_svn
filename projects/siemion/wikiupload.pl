#!/usr/bin/perl
# Andrew Siemion siemion@berkeley.edu
# wikiupload.pl 
# syntax is wikiupload.pl <dir or wildcard> <comment>
# example: wikiupload.pl ~/*.png "My home dir pngs"
# note: must include trailing slash w/ directories, as in:
# wikiupload.pl /tmp/ "contents of /tmp"
# NOT: wikiupload.pl /tmp "contents of /tmp"



# This function generates random strings of a given length
###########################################################
# Written by Guy Malachi http://guymal.com
# 18 August, 2002
###########################################################

use File::Basename;
use Getopt::Long;

sub generate_random_string
{
	my $length_of_randomstring=shift;# the length of 
			 # the random string to generate

	my @chars=('a'..'z','A'..'Z','0'..'9','_');
	my $random_string;
	foreach (1..$length_of_randomstring) 
	{
		# rand @chars will generate a random 
		# number between 0 and scalar @chars
		$random_string.=$chars[rand @chars];
	}
	return $random_string;
}


#GetOptions ('p:s' => \$path, 'c:s' => \$comment, 'im:s' => \$comment);


#EDIT THESE FOR YOUR INSTALLATION!

$mvs_command = "/opt/local/bin/mvs";  #location of mvs
$upload_command = "/usr/bin/mwup.pl";  #location of mediawiki bot upload script
$convert_command = "/opt/local/bin/convert"; #location of imagemagick convert script

$myusername = "siemion";
$mypassword = "somegreatpassword";
$mywikipage = "User:Siemion";
$mywiki = "casper.berkeley.edu";
$mywiki_name = "casper"; #defined in mwup.pl



@extensions = ('gz', 'tar', 'ppt', 'mdl', 'bit', 'c', 'zip', 'rtf', 'txt', 'avi', 'mpg', 'mp3', 'doc', 'xls', 'psd');
@images = ('png', 'gif', 'jpg', 'jpeg');
@imconvert = ('eps', 'pdf', 'tif', 'tiff', 'ps');

#extensions = ('.png', '.gif', '.jpg', '.jpeg', '.pdf', '.gz', '.tar', '.ppt', '.mdl', '.bit', '.c', '.zip', '.rtf', '.txt', '.avi', '.mpg', '.mp3', '.doc', '.xls');






$ignore_login_error=0;
$docstring="Read through the code for details.\n";
my $dir=$ARGV[0] or die  "Syntax: perl wikiupload.pl file or wildcard \n$docstring";
my $comment=$ARGV[1] or die "Syntax: perl wikiupload file or wildcard \n$docstring";
$dir =~ s/([ ])/\\$1/g;

(@files) = glob($dir."*");
#print @files;
#print "num: ".$#files."\n";

if($#files < 0) {
	die "no files to process...\n";
}

my $random_string=&generate_random_string(8);

@months = qw(Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec);
@weekDays = qw(Sun Mon Tue Wed Thu Fri Sat Sun);
($second, $minute, $hour, $dayOfMonth, $month, $yearOffset, $dayOfWeek, $dayOfYear, $daylightSavings) = localtime();
$year = 1900 + $yearOffset;
$theTime = "$hour:$minute:$second, $weekDays[$dayOfWeek] $months[$month] $dayOfMonth, $year";
#print $theTime;


system( 'mkdir /tmp/'.$random_string); 
system( 'mkdir /tmp/'.$random_string.'/'.$mywiki); 


system ('echo "<br style=\'clear:both\' />" >> /tmp/'.$random_string.'/wiki.txt');
system ('echo "=='.$theTime.'==" >> /tmp/'.$random_string.'/wiki.txt');

#or die "couldn't mk temp dir";



system ('echo "<br style=\'clear:both\' />" >> /tmp/'.$random_string.'/wiki.txt');	 
$imgcnt = 0;

foreach $file (@files) {
	$file =~ s/([ ])/\\$1/g;
}

foreach $file (@files) {
	
	($name,$path,$suffix) = fileparse($file);
	($iname,$path,$isuffix) = fileparse($file, @images);
	$name =~ s/%/_/g;
	$name =~ s/([ ])/\\$1/g;

	if($isuffix ne ''){
		if($imgcnt == 0) {
			system ('echo "<gallery caption=\'Sample gallery\' widths=\'200px\' heights=\'200px\' perrow=\'5\'>" >> /tmp/'.$random_string.'/wiki.txt');	 			
		}
		system ('cp '.$file.' /tmp/'.$random_string.'/'.$random_string.'_'.$name);
		system ('echo ">'.$random_string.'_'.$name.'" >> /tmp/'.$random_string.'/files.txt');
		system ('echo "'.$comment.'" >> /tmp/'.$random_string.'/files.txt');
		system ('echo "Image:' .$random_string.'_'.$name.'|'.$name.'" >> /tmp/'.$random_string.'/wiki.txt');
		$imgcnt = $imgcnt + 1;
	} 
}

foreach $file (@files) {
	
	($name,$path,$suffix) = fileparse($file);
	($iname,$path,$isuffix) = fileparse($file, @imconvert);
	$name =~ s/%/_/g;
	$name =~ s/([ ])/\\$1/g;
	if($isuffix ne ''){
		if($imgcnt == 0) {
			system ('echo "<gallery caption=\'Image Gallery\' widths=\'200px\' heights=\'200px\' perrow=\'5\'>" >> /tmp/'.$random_string.'/wiki.txt');	 			
		}
		system ('cp '.$file.' /tmp/'.$random_string.'/'.$random_string.'_'.$name);

		system ($convert_command.' /tmp/'.$random_string.'/'.$random_string.'_'.$name.' -append /tmp/'.$random_string.'/'.$random_string.'_'.$name.'.png');

		system ('echo "Image:' .$random_string.'_'.$name.'.png|[[Media:'.$random_string.'_'.$name.'|Vector version of '.$name.']]" >> /tmp/'.$random_string.'/wiki.txt');

		system ('echo ">'.$random_string.'_'.$name.'" >> /tmp/'.$random_string.'/files.txt');
		system ('echo "'.$comment.'" >> /tmp/'.$random_string.'/files.txt');

		system ('echo ">'.$random_string.'_'.$name.'.png" >> /tmp/'.$random_string.'/files.txt');
		system ('echo "'.$comment.'" >> /tmp/'.$random_string.'/files.txt');

		$imgcnt = $imgcnt + 1;
	} 
}



if($imgcnt > 0) {
	system ('echo "</gallery>" >> /tmp/'.$random_string.'/wiki.txt');	 			
}

foreach $file (@files) {

	($name,$path,$suffix) = fileparse($file);
	($ename,$path,$esuffix) = fileparse($file, @extensions);
	$name =~ s/%/_/g;
	$name =~ s/([ ])/\\$1/g;
	
	if($esuffix ne ''){
		system ('cp '.$file.' /tmp/'.$random_string.'/'.$random_string.'_'.$name);
		system ('echo ">'.$random_string.'_'.$name.'" >> /tmp/'.$random_string.'/files.txt');
		system ('echo "'.$comment.'" >> /tmp/'.$random_string.'/files.txt');
		system ('echo "{{bit|'.$random_string.'_'.$name.'|'.$name.'}}" >> /tmp/'.$random_string.'/wiki.txt');
		system ('echo "<br style=\'clear:both\' />" >> /tmp/'.$random_string.'/wiki.txt');	 
	}

}



system ($upload_command.' '.$mywiki_name.' /tmp/'.$random_string);

#>> ./User:Siemion.wiki


system ('cd /tmp/'.$random_string.'/'.$mywiki.'; '.$mvs_command.' login -d '.$mywiki.' -u '.$myusername.' -p \''.$mypassword.'\' -w /w/index.php');
system ('cd /tmp/'.$random_string.'/'.$mywiki.'; '.$mvs_command.' update '.$mywikipage.'.wiki');
system ('cat /tmp/'.$random_string.'/wiki.txt >> /tmp/'.$random_string.'/'.$mywiki.'/'.$mywikipage.'.wiki');
system ('cd /tmp/'.$random_string.'/'.$mywiki.'; '.$mvs_command.' commit -m \'automated edit\' '.$mywikipage.'.wiki');


$totprocessed = $files + 1;
#print "end: processed ".$totprocessed." files\n";

#die "done";
