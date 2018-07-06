BEGIN {
	for (i = 0; i < ARGC; i++) 
	{
		printf ("ARGC[%d] = %s\n",i,ARGV[i]);
	}
	print FILENAME
}

# Rules
{
	{print "Marks: "$4 "Extra Marks: " ($4 + 1)}
}

END {
	print ENVIRON["USER"]
	print FILENAME
	print FS
}
