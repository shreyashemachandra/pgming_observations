# Note: No Space must be given to on either side of equal sign(=)
# ie, no = 10 or no =10 or no= 10 --> is an error
#
# Defining NULL variable ex and ex1;

no=10;
hs=90;
str=bus;
ex=""
ex1=
echo "No has: $no"
echo "Str has: $str"
echo $ex
echo $ex1
echo "Heloo - `expr $no + $hs`"
