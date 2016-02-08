#!/usr/bin/perl
#soca-research(c)2014-2016
use strict;
use warnings;
use utf8;
use JSON;
use autodie ':io';
binmode STDOUT, ":utf8";
 
my $json;
{
  local $/; #Enable 'slurp' mode
  open my $fh, "<", "out/output.json";
  $json = <$fh>;
  close $fh;
}
my $data = decode_json($json);
print $data->{'1'}->{'totalRerata'} . "\n";
my $jsondata;
{
 local $/;
 open my $fhdata, "<", "out/output_1.json";
 $jsondata = <$fhdata>;
 close $fhdata;
}
my $newRerataData = {totalRerata=>"$data->{'1'}->{'totalRerata'}", status=>"$data->{'1'}->{'status'}"};
my $olddata = decode_json($jsondata);
push @{ $olddata->{'1'}}, $newRerataData;
open my $fhdata, ">", "out/output_1.json";
print $fhdata encode_json($olddata);
close $fhdata;
my $line;
open my $fhdata, "<", "out/output_1.json";
$line = <$fhdata>;
close $fhdata;
open my $fhdata, ">", "out/html_output_1.json";
substr($line, 0, 5)='';
substr($line,-1)='';
print $fhdata $line;
close $fhdata;
