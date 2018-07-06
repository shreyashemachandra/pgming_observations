BEGIN {
	for (i = 0; i < ARGC; i++) 
	{
		printf ("ARGC[%d] = %s\n",i,ARGV[i]);
	}
	print FILENAME
}

# Rules
{
	print FILENAME
}

END {
	print ENVIRON["USER"]
	print FILENAME
}
