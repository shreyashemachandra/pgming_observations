BEGIN {
	for (i = 0; i < ARGC; i++) 
	{
		printf ("ARGC[%d] = %s\n",i,ARGV[i]);
	}
	print FILENAME
}

# Rules
{
	if (match( $3, "s" )) {print "RLENGTH: " RLENGTH " RSTART:" RSTART}
	{print FILENAME ": NR" NR ": FNR" FNR}
}

END {
	print ENVIRON["USER"]
	print FILENAME
	print FS
}
