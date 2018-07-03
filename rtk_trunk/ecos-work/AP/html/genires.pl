#!/usr/bin/perl
# This generate the .C for data webpage and FILE.mak
# read file name
$FILEMAKE='FILE.mak';
$PREFIX="../html/";
$FILEIRESENTRY="../html/cyg_ires_table.c";

open(WEBLIST,"web_files") or die $!;
open(MAKEFILE,">",$FILEMAKE) or die $!;
print MAKEFILE "#Auto Generet Makefile";
print MAKEFILE "SRCS=\n";
while($line = <WEBLIST>){
	chomp $line;
	next if $line eq '';
	next if $line eq "./";
	open(FILE,$line) or die $!;
	print $line;
	$filename=substr($line,1,length($line)-1);
	print $filename;
	$data_filename="$filename";
	$data_filename=~s/\./_/g;
	$data_filename=~s/\//_/g;
	$data_filename=~s/\-/_/g;
	print $data_filename;
	$c_suffix='.c';	
	$C_filename="$data_filename"."$c_suffix" ;
	print $C_filename;
	print MAKEFILE "SRCS+=$PREFIX$C_filename\n";
	open(OUT,">",$C_filename);
	
	# Create an array of unsigned chars from input
	#@array = <FILE>;
	#@chars = unpack "C*", $array;

	print OUT "#include <pkgconf/hal.h>\n";
	print OUT "#include <pkgconf/kernel.h>\n";
	print OUT "#include <cyg/hal/hal_tables.h>\n";
	print OUT "#include <cyg/fileio/fileio.h>\n";
	print OUT "#include <dirent.h>\n";
	print OUT "#include <network.h>\n";
	print OUT "#include <string.h>\n";
	print OUT "#include <stdio.h>\n"; 
	print OUT "#include \"http.h\"\n";
	print OUT "#include \"socket.h\"\n";
	print OUT "#include \"handler.h\"\n";
	
		
	print OUT "unsigned char $data_filename\[\] = {\n  ";	
	
	$i=0;
 	while ($line = <FILE>) {
		#$j=0;
 		@chars = unpack("C*", $line);
        	foreach $char (@chars) {
                        printf OUT (($i % 13) ? ", " : ",\n  ") if $i > 0;
                	printf OUT "0x%02x", $char;
			$i++;
                #        last if $j == $#chars;
                }
	}
        print OUT "};\n";
	$CYG_HTTP_PREFIX="cyg_httpd_ires";
	$CYG_HTTP_PREFIX="$CYG_HTTP_PREFIX"."$data_filename";
	print OUT "CYG_HTTPD_IRES_TABLE_ENTRY($CYG_HTTP_PREFIX, \"$filename\", $data_filename, sizeof($data_filename));";
	open(ENTRY_FILE,">>",$FILEIRESENTRY);
	print ENTRY_FILE "#include \"$PREFIX$C_filename\"\n";
	close(OUT);
	close(FILE);

}
close(MAKEFILE);
close(WEBLIST);
