BEGIN {
	for (i = 0; i < ARGC; i++) 
	{
		printf ("ARGC[%d] = %s\n",i,ARGV[i]);
	}
}
