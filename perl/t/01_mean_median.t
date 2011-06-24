
my ($last_test,$loaded);

######################### We start with some black magic to print on failure.
use lib '../blib/lib','../blib/arch';

BEGIN { $last_test = 11; $| = 1; print "1..$last_test\n"; }
END   { print "not ok 1  Can't load Algorithm::Cluster\n" unless $loaded; }

use Algorithm::Cluster;
no  warnings 'Algorithm::Cluster';

$loaded = 1;
print "ok 1\n";

######################### End of black magic.

sub test;  # Predeclare the test function (defined below)

my $tcounter = 1;
my $want     = '';


#------------------------------------------------------
# Tests
# 
my ($meanval, $medianval);
my $dummy;
my $dummy_ref = \$dummy;
my $dummy_sub_ref = sub {};

my $data1 = [ 34.3, 3, 2 ];
my $data2 = [ 5, 10 ,15, 20 ];
my $data3 = [ 1, 2, 3, 5, 7, 11, 13, 17, $dummy_sub_ref ];
my $data4 = [ 100, 19, 3, 1.5, 1.4, 1, 1, 1, $dummy_ref ];
my $data5 = [ 2.0, 21, 1, 1, 1, 4.0, 5.0, 'not a number' ];

$want = '13.1000';  test q(sprintf "%7.4f", Algorithm::Cluster::mean($data1));
$want = '12.5000';  test q(sprintf "%7.4f", Algorithm::Cluster::mean($data2));
$want = 'undef';  test q(sprintf "%7.4f", Algorithm::Cluster::mean($data3));
$want = 'undef';  test q(sprintf "%7.4f", Algorithm::Cluster::mean($data4));
$want = 'undef';  test q(sprintf "%7.4f", Algorithm::Cluster::mean($data5));

$want = ' 3.0000';  test q(sprintf "%7.4f", Algorithm::Cluster::median($data1));
$want = '12.5000';  test q(sprintf "%7.4f", Algorithm::Cluster::median($data2));
$want = 'undef';  test q(sprintf "%7.4f", Algorithm::Cluster::median($data3));
$want = 'undef';  test q(sprintf "%7.4f", Algorithm::Cluster::median($data4));
$want = 'undef';  test q(sprintf "%7.4f", Algorithm::Cluster::median($data5));


#------------------------------------------------------
# Test function
# 
sub test {
	$tcounter++;

	my $string = shift;
	my $ret = eval $string;
	$ret = 'undef' if not defined $ret;

        if ("$ret" =~ /^$want$/sm) {

		print "ok $tcounter\n";

	} else {
		print "not ok $tcounter\n",
		"   -- '$string' returned '$ret'\n", 
		"   -- expected =~ /$want/\n"
	}
}

