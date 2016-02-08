#!/usr/bin/perl
#teangan(c)2014-2016
use warnings;
use strict;
use utf8;

system("echo \"0\" > out/status");
my $id=0;
my $file=shift;
my $infile=shift;
my $output= "{\n";
open(FILE,$infile) or die;
while (<FILE>)
{
        chomp;
        my @list = split /\t/,$_;
        my $url = `curl --silent '$list[1]' | grep mp4 | sed -e 's/.mp4\\".*/.mp4/' -e 's/^.*\\"//'`;
        $url =~ s/^\s+//;
        $url =~ s/\s+$//;
        my $filename = $url;
        $filename =~ s/^.*\///;
        if ($url =~ m/^http/)
        {
			$id++;
			$output .= "\"$id\":\n{\n\"loc\":\"$list[0]\",\n";
			system("mkdir -p out/$list[0]/");
			system("wget --quiet --output-document=out/$list[0]/latest.mp4 $url");
			system("mkdir -p out/$list[0]/tmp/");
			my $temp = `/home/aqmar/work/cardetection/CarDetection out/$list[0]/latest.mp4 out/$list[0]/tmp/`;
			$temp =~ s/,\s*/,\n/g;
			$output .= "$temp\n},\n";
        }
}
close(FILE);
$output =~ s/,\s+$/\n/;
$output .= "}\n";
system("echo \"1\" > out/status");
open(FILE,">$file") or die;
print FILE $output;
close(FILE);
